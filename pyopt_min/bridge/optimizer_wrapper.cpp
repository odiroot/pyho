#include <cstdlib>
#include <math.h>
#include <fstream>
#include "optimizer_wrapper.h"
#include "ptreal.h"
#include "latticegrid.h"
#include "paramcoil.h"
#include "evalcoil.h"
#include "redirecter.h"
#include "mathconst.h"
#include "opt_utils.h"
using namespace std;

void c_print(const char* text) {
    cout << text << endl;
}

/** WRAPPING OPTIMIZER **/
// General purpose variables
ofstream logFile;
// Coil parameters
static real* myMin = 0;
static real* myMax = 0;
static int noDesignVars = 0;
static ParamCoil paramCoil;
static LatticeGrid* grid = 0;
static Pt_real midP;
// The objectives
int nCntrComps;
int cntrComps[3];
Pt_real Bwanted;
double wantedNorm;

BlockCoil bestC;
int nx, ny, nz;
real bestRslt;
Pt_real Bmean;
real stdDev;


void redirect_log(const char* path) {
    logFile.open(path);
    //redirecter* redir =
    new redirecter(logFile, cout);
}

void get_coil(const char* coil_path) {
    ifstream cstream(coil_path);
    cstream >> paramCoil;
    cstream.close();

    if(paramCoil.getNoSect() > 0) {
        noDesignVars = paramCoil.getNoSect() + 1;
        real min, max;
        paramCoil.getConstr4Lgt(min, max);
        if(max > min) {
            noDesignVars++;
        }
        paramCoil.getConstr4Dpth(min, max);
        if(max > min) {
            noDesignVars++;
        }
        c_print("Read parametric description of the coil.");
        cout << paramCoil.getNoSect() << " sections => ";
        cout << noDesignVars << " design variables" << endl;

    } else {
        c_print("ERROR: Can't find parametric description of the coil.");
        c_print("Exiting abnormally\n");
        exit(1);
    }
}

void prepare_constraints() {
    myMin = new real[noDesignVars];
    myMax = new real[noDesignVars];
    int nSect = paramCoil.getNoSect();
    for(int i= 0; i < nSect; i++) {
        paramCoil.getConstr4Blck(i, myMin[i], myMax[i]);
    }
    paramCoil.getConstr4Side(myMin[nSect], myMax[nSect]);
    if(noDesignVars > nSect+1) {
        paramCoil.getConstr4Lgt(myMin[nSect+1], myMax[nSect+1]);
    }
    if(noDesignVars > nSect+2) {
        paramCoil.getConstr4Dpth(myMin[nSect+2], myMax[nSect+2]);
    }
}

void get_grid(const char* grid_path) {
    Pt_real llc, urc;
    nx = 0, ny = 0, nz = 0;
    ifstream cstream(grid_path);
    cstream >> llc >> urc;
    cstream >> nx >> ny >> nz;
    cstream.close();

    grid = new LatticeGrid(llc, urc, nx, ny, nz);
    cout << "Grid: " << nx*ny*nz << " points in";
    cout << " BB=(" << llc << ")-(" << urc << ")" << endl;

    if(grid->size() <= 0) {
        c_print("ERROR: Can't find grid.");
        c_print("Exiting abnormally\n");
        exit(1);
    }
}

void cost_function(float Bx, float By, float Bz, int flag) {
    nCntrComps = 0;
    wantedNorm = 0;
    if(flag & ARG_BX) {
        cntrComps[nCntrComps++] = 1;
        Bwanted(1) = Bx;
        wantedNorm += pow(Bwanted(1), 2);
    }
    if(flag & ARG_BY) {
        cntrComps[nCntrComps++] = 2;
        Bwanted(2) = By;
        wantedNorm += pow(Bwanted(2), 2);
    }
    if(flag & ARG_BZ) {
        cntrComps[nCntrComps++] = 3;
        Bwanted(3) = Bz;
        wantedNorm += pow(Bwanted(3), 2);
    }
    wantedNorm = sqrt(wantedNorm);
}

void prepare_reference_value() {
    bestC = paramCoil.shape(0, noDesignVars);
    midP = paramCoil.midPt();

    stdDev = evalCoilStdDev(bestC, *grid, Bmean);
    if(nCntrComps == 0) {
        bestRslt = stdDev;
    } else {
        bestRslt = evalCoilGlobalUniv(bestC, *grid, nCntrComps,
            cntrComps, Bwanted, Bmean);
        for(int j = 0; j < nCntrComps; j++) {
            int cc = cntrComps[j];
            if(Bmean(cc)*Bwanted(cc) < 0) {
                Bwanted(cc) = -Bwanted(cc);
            }
        }
    }
    cout << "Reference coil: B_mean=(" << Bmean << "),";
    cout << " stdDev=" << stdDev << ", err= " << bestRslt << endl;

    if(nCntrComps == 0) {
        c_print("Trying to obtain possibly uniform field fit to mean value.");
    } else {
        cout << "Trying to obtain uniform B=[ ";
        for(int j = 1; j < 4; j++) {
            int k;
            for(k = 0; k < nCntrComps; k++) {
                if(cntrComps[k] == j) {
                    break;
                }
            }
            if(k == nCntrComps) {
                cout << "* ";
            } else {
                cout << Bwanted(cntrComps[k]) << " ";
            }
        }
        cout << "]" << endl;
    }
}

int get_no_design_vars() {
    return noDesignVars;
}

real* get_my_min() {
    return myMin;
}

real* get_my_max() {
    return myMax;
}

float bFun(real t[]) {
    BlockCoil bc = paramCoil.shape(t, noDesignVars);

    float obj;

    if(nCntrComps == 0) {
        real std = evalCoilStdDev(bc, *grid, Bmean);
        obj = std / Bmean.norm();
    } else {
        obj = evalCoilGlobalUniv(bc, *grid, nCntrComps, cntrComps, Bwanted, Bmean);
        obj /= wantedNorm;
    }
    
    return obj;
}

void print_best_coil(real t[]) {   
    bestC = paramCoil.shape(t, noDesignVars);
    stdDev = evalCoilStdDev(bestC, *grid, Bmean);
    if(nCntrComps == 0) {
        bestRslt = stdDev;
    } else {
        Pt_real tmpB;
        bestRslt = evalCoilGlobalUniv(bestC, *grid, nCntrComps, cntrComps,
            Bwanted, tmpB);
    }
    
    cout << "Optimized coil: B_mean= (" << Bmean;
    cout << "), stdDev= " << stdDev;
    cout << ", err= " << bestRslt << endl;
}

void output_cblock(const char* path) {
    ofstream cblockOut(path);
    cblockOut << bestC << endl;
    cblockOut.close();
}

void rebuild_grid(int fine) {
    grid->rebuild(fine);
}

void output_xml(char* path, real t[]) {
    writeXML(path, noDesignVars, t, nCntrComps, cntrComps, Bwanted,
        grid, nx, ny, nz, paramCoil, bestC, bestRslt, Bmean, stdDev);
}

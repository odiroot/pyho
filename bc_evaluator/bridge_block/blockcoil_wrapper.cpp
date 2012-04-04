#include <cstdlib>
#include <math.h>
#include <fstream>
#include "blockcoil_wrapper.h"
#include "ptreal.h"
#include "latticegrid.h"
#include "paramcoil.h"
#include "evalcoil.h"
#include "redirecter.h"
#include "mathconst.h"
#include "opt_utils.h"


using namespace std;


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
int nx, ny, nz;


BlockCoil basic_eval(real t[], float& stddev, float& result, Pt_real& bmean) {
    BlockCoil bc = paramCoil.shape(t, noDesignVars);

    stddev = evalCoilStdDev(bc, *grid, bmean);
    if(nCntrComps == 0) {
        result = stddev;
    } else {
        result = evalCoilGlobalUniv(bc, *grid, nCntrComps, cntrComps,
            Bwanted, bmean);
    }
    return bc;
}

int get_coil(const char* coil_path, int& nsect, int& nvars) {
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
        nsect = paramCoil.getNoSect();
        nvars = noDesignVars;
        return 0;
    } else {
        return -1;
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

int get_grid(const char* grid_path, int& npoints) {
    Pt_real llc, urc;
    nx = 0, ny = 0, nz = 0;
    ifstream cstream(grid_path);
    cstream >> llc >> urc;
    cstream >> nx >> ny >> nz;
    cstream.close();

    grid = new LatticeGrid(llc, urc, nx, ny, nz);
    npoints = nx * ny * nz;
    // TODO
    // cout << " BB=(" << llc << ")-(" << urc << ")" << endl;
    if(grid->size() <= 0) {
        return -1;
    } else {
        return 0;
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

void prepare_reference_value(float& stddev, float& err) {
    Pt_real bmean;
    midP = paramCoil.midPt();

    basic_eval(0, stddev, err, bmean);
    // TODO:
    //cout << "Reference coil: B_mean=(" << bmean << "),";

    if(nCntrComps == 0) {
        // TODO:
        // cout << "Trying to obtain possibly uniform field fit to mean value.";
    } else {
        for(int j = 0; j < nCntrComps; j++) {
            int cc = cntrComps[j];
            if(bmean(cc)*Bwanted(cc) < 0) {
                Bwanted(cc) = -Bwanted(cc);
            }
        }
        // TODO:
        // cout << "Trying to obtain uniform B=[ ";
        // for(int j = 1; j < 4; j++) {
        //     int k;
        //     for(k = 0; k < nCntrComps; k++) {
        //         if(cntrComps[k] == j) {
        //             break;
        //         }
        //     }
        //     if(k == nCntrComps) {
        //         cout << "* ";
        //     } else {
        //         cout << Bwanted(cntrComps[k]) << " ";
        //     }
        // }
        // cout << "]" << endl;
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
    float stddev, result, obj;
    Pt_real bmean;

    basic_eval(t, stddev, result, bmean);

    if(nCntrComps == 0) {
        obj = stddev / bmean.norm();
    } else {
        obj = result / wantedNorm;
    }
    return obj;
}

void print_coil(real t[]) {
    float stddev, result;
    Pt_real bmean;

    basic_eval(t, stddev, result, bmean);
    // TODO: Transfer to Cython
    cout << "Optimized coil: B_mean= (" << bmean;
    cout << "), std. dev.= " << stddev;
    cout << ", err= " << result << endl;
}

void output_cblock(const char* path, real t[]) {
    BlockCoil tmp = paramCoil.shape(t, noDesignVars);
    ofstream cblockOut(path);
    cblockOut << tmp << endl;
    cblockOut.close();
}

void rebuild_grid(int fine) {
    grid->rebuild(fine);
}

void output_xml(char* path, real t[]) {
    BlockCoil tmp;
    float stddev, result;
    Pt_real bmean;

    tmp = basic_eval(t, stddev, result, bmean);

    writeXML(path, noDesignVars, t, nCntrComps, cntrComps, Bwanted,
        grid, nx, ny, nz, paramCoil, tmp, result, bmean, stddev);
}

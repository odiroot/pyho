#include <fstream>
#include <cstdlib>
#include <cstring>
#include "latticegrid.h"
#include "paramcoil.h"
#include "evalcoil.h"
#include "stopper.h"

using namespace std;

// a simple tool for checking command line switches
int findOption( const char *opt, int argc, char **argv ) {
    for( int i= argc-1; i > 0; i-- )  // last option is the most important one
        if( strcmp(opt,argv[i]) == 0 )
            return i;
    return 0;
}

int
        main(int argc, char** argv) {
    cout << "# Electromagnetic flow meter - coil shape evaluator\n";
    if( argc == 1 || strcmp( argv[1], "-h" ) == 0 ) {
        cout << "Usage:\n";
        cout << argv[0] << " -coil <parmetric> -shape <shape> [ <other options> ]\n\n";
        cout << "\tWhere: <parmetric> and <grid> are file names.\n";
        cout << "\t\t <shape> may be file name or list of values (in the latter case -shape must be the last option).\n";
        cout << "\t\t <other options> include: -grid <grid> -coilfile, -gridfile, -bboxfile, -Bfile\n";
        cout << "\t\t otput file name must follow any of <other options>.\n\n";
        return 0;
    }

    string coilOut(""), gridOut(""), bboxOut(""), BOut("");

    for( int i= 1; i < argc; i++ )
        if( strcmp( argv[i] , "-coilfile" ) == 0 )
            coilOut = argv[++i];
    else if( strcmp( argv[i], "-gridfile" ) == 0 )
        gridOut = argv[++i];
    else if( strcmp( argv[i], "-bboxfile" ) == 0 )
        bboxOut = argv[++i];
    else if( strcmp( argv[i], "-Bfile" ) == 0 )
        BOut = argv[++i];

    int optind= 0;  // used to get ret. value from findOption

		int nCntrComps;    // # of controlled components: 0 - 3 (0 means "optimize to mean")
		int cntrComps[3];  // controlled components
		Pt_real Bwanted;   // values of controlled components (some components may be not used and not initialized!)
		real wantedNorm;   // L2 norm of controlled components
		Pt_real Bmean;

    nCntrComps= 0;
    wantedNorm= 0;
    if( (optind= findOption( "-Bx", argc, argv)) ) {
      cntrComps[nCntrComps++]= 1;
      Bwanted(1)= atof( argv[optind+1] );
      wantedNorm += Bwanted(1)*Bwanted(1);
    }
    if( (optind= findOption( "-By", argc, argv)) ) {
      cntrComps[nCntrComps++]= 2;
      Bwanted(2)= atof( argv[optind+1] );
      wantedNorm += Bwanted(2)*Bwanted(2);
    }
    if( (optind= findOption( "-Bz", argc, argv)) ) {
      cntrComps[nCntrComps++]= 3;
      Bwanted(3)= atof( argv[optind+1] );
      wantedNorm += Bwanted(3)*Bwanted(3);
    }
    wantedNorm= sqrt( wantedNorm );

    int noDesignVars= 0;
    ParamCoil paramCoil;
    LatticeGrid *grid= 0;

    // Get coil
    for( int i= 1; i < argc; i++ )
        if( strcmp(argv[i],"-coil") == 0 ) {
        ifstream cstream( argv[++i] );
        cstream >> paramCoil;
        cstream.close();
        break;
    }
    if( paramCoil.getNoSect() > 0 ) {
        noDesignVars = paramCoil.getNoSect() + 1;
        real min, max;
        paramCoil.getConstr4Lgt( min, max );
        if( max > min )
            noDesignVars++;
        paramCoil.getConstr4Dpth( min, max );
        if( max > min )
            noDesignVars++;
        cout << "# Read parametric description of the coil.\n";
        cout << "# " << paramCoil.getNoSect() << " sections gives " << noDesignVars << " design variables\n";
    } else {
        cout << "ERROR: Can't find parametric description of the coil.\n";
        cout << "Exiting abnormally\n\n";
        return 1;
    }
    // Prepare for shape
    real *shape = 0;
    for( int i= 1; i < argc; i++ )
        if( strcmp(argv[i],"-shape") == 0 ) {
        ifstream cstream( argv[++i] );
        if( cstream.good() ) {
            shape = new real[noDesignVars];
            for( int j= 0; j < noDesignVars; j++ )
                cstream >> shape[j];
            cstream.close();
        } else if( argc-i == noDesignVars ) {
            shape = new real[noDesignVars];
            for( int j= 0; j < noDesignVars; j++ )
                shape[j] = atof( argv[i+j] );
        }
        break;
    }

    // Get grid
    Pt_real llc, urc;
    int nx, ny, nz;
    for( int i= 1; i < argc; i++ )
        if( strcmp(argv[i],"-grid") == 0 ) {
        ifstream cstream( argv[++i] );
        if( cstream.good() ) {
            cstream >> llc >> urc;
            cstream >> nx >> ny >> nz;
            cstream.close();
            grid = new LatticeGrid( llc, urc, nx, ny, nz );
            cout << "# Grid: (" << llc << ")-(" << urc << ")\n";
            if( bboxOut != "" ) {
                ofstream oc( bboxOut.c_str() );
                oc << "> 1\n";
                oc << C_Block( llc+Pt_real(0,0,0.5*(urc(3)-llc(3))), Pt_real( urc(1)-llc(1), 0, 0 ), Pt_real( 0, urc(2)-llc(2), 0 ), urc(3)-llc(3), 0, 0 );
                oc.close();
            }
        }
        break;
    }

    BlockCoil shapedC = paramCoil.shape( shape, noDesignVars );
    real shapedRslt = 0;
    real stdDev = 0;
    if( grid != 0 ) {
        stopper s;
        s.start();
        stdDev = evalCoilStdDev( shapedC, *grid, Bmean );
        if( nCntrComps == 0 ) {
        	shapedRslt = stdDev;
        } else {
          shapedRslt = evalCoilGlobalUniv( shapedC, *grid, nCntrComps, cntrComps, Bwanted, Bmean );
        }

        cout << "# Field over " << grid->getNx() << 'x' << grid->getNy() << 'x' << grid->getNz() << " grid evaluated in " << s.stop() << " s.\n";
        cout << "# Mean B=[" << Bmean.getX() << "," << Bmean.getY() << "," << Bmean.getZ() << "]\n";
        cout << "# Std. dev.=" << stdDev << endl;
    }
    if( coilOut == "" ) {
        //cout << "# BlockCoil: vol=" << shapedC.volume() << endl;
        //cout << "# err for B= " << shapedRslt << endl;
        //cout << "# shape:\n";
        cout << shapedC << endl;
    } else {
        ofstream oc( coilOut.c_str() );
        //oc << "# BlockCoil: vol=" << shapedC.volume() << endl;
        //oc << "# err for B= " << shapedRslt << endl;
        //oc << "# shape:\n";
        oc << shapedC << endl;
        oc.close();
    }

    if( (optind= findOption( "-xml", argc, argv)) ) {
        ofstream xmlOut( argv[optind+1] );
		    cout << "# Generating evaluation XML" << endl;
        xmlOut << "<optimization>" << endl;
        xmlOut << "\t<coil_evaluation criteriaField=\"B\">" << endl;

        paramCoil.printXML4Shape( xmlOut, shape, noDesignVars, "" );

        xmlOut << "\t\t<vector_field name=\"B\" dim=\"3\" nVals=\""<< (*grid).size() << "\" >" << endl;
        xmlOut << "\t\t\t<mean x=\"" << Bmean.getX() << "\" y=\"" << Bmean.getY() << "\" z=\"" << Bmean.getZ() << "\" />" << endl;

        xmlOut << "\t\t\t<eval>" << endl;
        xmlOut << "\t\t\t\t<std_dev value=\"" << stdDev << "\" />" << endl;
        xmlOut << "\t\t\t</eval>" << endl;
        xmlOut << "\t\t\t<grid>" << endl;
        xmlOut << "\t\t\t\t<density x=\"" << nx << "\" y=\"" << ny << "\" z=\"" << nz << "\" />" << endl;
        xmlOut << "\t\t\t\t<vertex x=\"" << llc.getX() << "\" y=\"" << llc.getY() << "\" z=\"" << llc.getZ() << "\" />" << endl;
        xmlOut << "\t\t\t\t<vertex x=\"" << urc.getX() << "\" y=\"" << urc.getY() << "\" z=\"" << urc.getZ() << "\" />" << endl;
        xmlOut << "\t\t\t</grid>" << endl;

        for (int p=0; p < (*grid).size(); p++) {
                Pt_real vecB = shapedC.B( (*grid)(p) );
                xmlOut << "\t\t\t<val ind=\"" << p << "\" "
                       << "vx=\"" << vecB.getX() << "\" "
                       << "vy=\"" << vecB.getY() << "\" "
                       << "vz=\"" << vecB.getZ() << "\" />"
                           << endl;
        }

        xmlOut << "\t\t</vector_field>" << endl;

        xmlOut << "\t\t<grid>" << endl;
        xmlOut << "\t\t\t<density x=\"" << nx << "\" y=\"" << ny << "\" z=\"" << nz << "\" />" << endl;
        xmlOut << "\t\t\t<vertex x=\"" << llc.getX() << "\" y=\"" << llc.getY() << "\" z=\"" << llc.getZ() << "\" />" << endl;
        xmlOut << "\t\t\t<vertex x=\"" << urc.getX() << "\" y=\"" << urc.getY() << "\" z=\"" << urc.getZ() << "\" />" << endl;
        xmlOut << "\t\t</grid>" << endl;

        xmlOut << "\t</coil_evaluation>" << endl;
        xmlOut << "</optimization>" << endl;
        xmlOut.close();
    } else {
        cout << "Skipping generation of evalution XMl file" << endl;
    }

    if( grid == 0 )
        return 0;

    if( BOut != "" ) {
        ofstream oc( BOut.c_str() );
        oc << "B = [\n";
        for( int p= 0; p < (*grid).size(); p++ )
            oc << shapedC.B( (*grid)(p) ) << endl;
        oc << "]\n";
        oc.close();
    }

    if( gridOut != "" ) {
        ofstream oc( gridOut.c_str() );
        oc << (*grid);
        oc.close();
    }

    return 0;
}

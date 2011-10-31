#include "evalcoil.h"

// Calculates local errors for every point of the Grid g.
// Input:
//   c - coil
//   g - grid
//   Bwanted - wanted value of B (same for each node)
// Output:
//   Bmean - mean value of B (from coil) on the grid
//   err   - errors in each node: err(i) = (B(i) - Bwanted)^2
void evalCoilLocalErr( BlockCoil& c, Grid& g, Pt_real& Bwanted, Pt_real& Bmean, real* err )
{
    Pt_real *B = new Pt_real[g.size()];
    if( err == 0 ) {
        err = new real[g.size()];
    }
    Bmean.fill(0.0);
    for( int i= 0; i < g.size(); i++ ) {
        B[i] = c.B( g(i) );
        err[i] = (B[i]-Bwanted)*(B[i]-Bwanted);
        Bmean += B[i];
    }
    Bmean /= g.size();

    delete [] B;

    return;
}

// Calculates local deviations for every point of the Grid g.
//  Deviations are calculated with respect to the mean value of B.
// Input:
//   c - coil
//   g - grid
// Output:
//   Bmean - mean value of B (from coil) on the grid
//   err   - errors in each node: err(i) = (Bmean - Bwanted)^2
void evalCoilLocalErr( BlockCoil& c, Grid& g, Pt_real& Bmean, real* err )
{
    Pt_real *B = new Pt_real[g.size()];
    if( err == 0 ) {
        err = new real[g.size()];
    }
    Bmean.fill(0.0);
    for( int i= 0; i < g.size(); i++ ) {
        B[i] = c.B( g(i) );
        Bmean += B[i];
    }
    Bmean /= g.size();

    for( int i= 0; i < g.size(); i++ )
        err[i] = (B[i]-Bmean)*(B[i]-Bmean);

    delete [] B;

    return;
}

// Calculates global err of B with respect to expected value over the Grid g.
// Input:
//   c - coil
//   g - grid
//   Bwanted - wanted value of B (same for each node)
// Output:
//   Bmean - mean value of B (from coil) on the grid
//   return value - sqrt( sum[ (B(i)-Bwanted)^2 ] / N )
real evalCoilGlobalErr( BlockCoil& c, Grid& g, Pt_real& Bwanted, Pt_real& Bmean )
{
    real maxDiff= 0;
    int maxDiffAt = -1;
    Pt_real *B = new Pt_real[g.size()];
    Bmean.fill(0.0);
    real stdDev= 0.0;
    for( int i= 0; i < g.size(); i++ ) {
        B[i] = c.B( g(i) );
        real diff2 = (B[i]-Bwanted).norm2();
#if B_DEBUG > 1
        std::cout << "P=(" << g(i) << ") \tB=[" << B[i] << "] ";
        std::cout << "\t|e|=" << sqrt(diff2) << std::endl;
#endif
        if( diff2 > maxDiff ) {
            maxDiff= diff2;
            maxDiffAt= i;
        }
        stdDev += diff2;
        Bmean += B[i];
    }

#ifdef B_DEBUG
    std::cout << "evalCoilStdDev: Bmean = [ " << Bmean << " ], StdDev=" << sqrt(stdDev) << ", Bs= [ " << Bwanted << " ], ||maxDiff||= " << sqrt(maxDiff) << " @ (" << g(maxDiffAt) << "), B= [" << B[maxDiffAt] << " ] " << std::endl;
#endif

    delete [] B;

    Bmean /= g.size();     
    return sqrt( stdDev /g.size() );
}

// Calculates standard deviation of B with respect to mean value over the Grid g.
// Input:
//   c - coil
//   g - grid
// Output:
//   Bmean - mean value of B (from coil) on the grid
//   return value - sqrt( sum[ (B(i)-Bmean)^2 ] / N )
real evalCoilStdDev( BlockCoil& c, Grid& g, Pt_real& Bmean )
{
    Pt_real *B = new Pt_real[g.size()];
    Pt_real tmp;
    Bmean.fill(0.0);
    for( int i= 0; i < g.size(); i++ ) {
        B[i] = c.B( g(i) );
        Bmean += B[i];
    }
    Bmean /= g.size();

    real stdDev= 0;
    for( int i= 0; i < g.size(); i++ ) {
        tmp= B[i]-Bmean;
        stdDev += tmp.norm2();
    }

    delete [] B;

#ifdef B_DEBUG
std::cout << " - stdDev=" << sqrt( stdDev / g.size() ) << " - " << std::endl;
#endif
    return sqrt( stdDev / g.size() );
}

real evalCoilGlobalMeanNorm( BlockCoil& c, Grid& g, Pt_real& Bref )
{
    real result= 0;
    for( int i= 0; i < g.size(); i++ ) {
        Pt_real B = c.B( g(i) ) - Bref;
        result += B.norm();
    }
    return result / g.size();
}

real evalCoilGlobalLInfty( BlockCoil& c, Grid& g, Pt_real& Bref )
{
    real result= 0;
    for( int i= 0; i < g.size(); i++ ) {
        Pt_real B = c.B( g(i) ) - Bref;
        real test = B.norm();
        if( test > result )
            result = test;
    }
    return result;
}

real evalCoilGlobalUniv( BlockCoil& c, Grid& g, int nCntrComps, int *cntrComps, Pt_real& Bwanted, Pt_real& Bmean ) {
/* Universal evaluator: controls from 1 to 3 components of B */
    Pt_real B;
    Bmean.fill(0.0);
    real err2= 0.0;
    for( int i= 0; i < g.size(); i++ ) {
        B = c.B( g(i) );
        for( int j= 0; j < nCntrComps; j++ )
					err2 += (B(cntrComps[j])-Bwanted(cntrComps[j]))*(B(cntrComps[j])-Bwanted(cntrComps[j]));
        
        Bmean += B;
    }
    Bmean /= g.size();

    return sqrt( err2 /g.size() );
}

#ifndef _OPT_UTILS_H_IS_INCLUDED_
#define _OPT_UTILS_H_IS_INCLUDED_

#include "latticegrid.h"
#include "paramcoil.h"

void writeXML( char *filename, 
               int noDesignVars, real *parameters,
               int nCntrComps, int * cntrComps, Pt_real& Bwanted,
               LatticeGrid *grid, int onx, int ony, int onz,
               ParamCoil& paramCoil,
               BlockCoil& bestC, real bestRslt, Pt_real& Bmean, real stdDev
             );

#endif

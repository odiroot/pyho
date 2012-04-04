#ifndef _EVALCOIL_H_IS_INCLUDED_
#define _EVALCOIL_H_IS_INCLUDED_

#include "grid.h"
#include "blockcoil.h"

void evalCoilLocalErr( BlockCoil& c, Grid& g, Pt_real& Bwanted, Pt_real& Bmean, real* err );

void evalCoilLocalErr( BlockCoil& c, Grid& g, Pt_real& Bmean, real* err );

real evalCoilStdDev( BlockCoil& c, Grid& g, Pt_real& Bmean );

real evalCoilGlobalErr( BlockCoil& c, Grid& g, Pt_real& Bwanted, Pt_real& Bmean );

real evalCoilStdDev( BlockCoil& c, Grid& g, int wantedComp, real wantedValue, Pt_real& Bmean );

real evalCoilGlobalUniv( BlockCoil& c, Grid& g, int nCntrComps, int *cntrComps, Pt_real& Bwanted, Pt_real& Bmean );

real evalCoilGlobalMeanNorm( BlockCoil& c, Grid& g, Pt_real& Bref );

real evalCoilGlobalLInfty( BlockCoil& c, Grid& g, Pt_real& Bref );

#endif

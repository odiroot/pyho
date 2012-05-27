#ifndef _MATCONST_H_IS_INCLUDED_
#define _MATCONST_H_IS_INCLUDED_

#undef _SP_CBLOCK_
#undef _DP_CBLOCK_

#ifdef SINGLE_PRECISION_CODE
#define _SP_CBLOCK_
typedef float real;
static const real comparison_tolerance = 1e-9;
#else
#define _DP_CBLOCK_
typedef double real;
static const real comparison_tolerance = 1e-13;
#endif


#endif

#ifndef paramcoil_h_is_included
#define paramcoil_h_is_included

#include <iostream>
#include "mathconst.h"
#include "ptreal.h"
#include "blockcoil.h"

typedef real sectParm_t[4];

// ParamCoil: Parametric description of the BlockCoil set.
//             ^y
//             |
//             | w/2
//             +--+----> x
//            /   |    | C_Block[0]
//           /    |  |   C_Block[1]
//          /     | |    C_Block[2]
//         z      ....
//                |   |  C_Block[nSect-1]
//
//   nSect     - no. of sections
//   sectParm  - sections' parameters: depth, start width, min width, max width
//   w         - coil width (inside, OX direction)
//   l         - coil length (OZ direction)
//   lgtParm   - length parameters: not-used, nominal, min length, max length
//   dpthParm  - depth parameters: not-used, nominal, min, max
//   dblCnct   - true if side-to-side connection go at the top and at the bottom
//   				  		false if only to side-to-side connections are present
//   varH		 	 - which dimension of side-to-side is the variable one
//   					  	true means that the height is variable
//   sideParm  - dimension of side-to-side connection:
//   					  	first elem not used (it is calculated to fit cross-section)
//   						  then go: start, min, max for the variable Cdimension
//   (jr,ji)   - complex current density (ji not used yet)


class ParamCoil {
private:
    Pt_real pos;
    int nSect;
    sectParm_t *sectParm;
    real w;
    real l;
    sectParm_t lgtParm;
    sectParm_t dpthParm;
    bool dblCnct;
    bool varH;
    sectParm_t sideParm;
    real jr, ji;
    long ngen;

    void freeMem( void );
    int countBlck(void);
    BlockCoil shapeSingle( real *actshape, bool optSide, bool optLgt, bool optDpth );
    BlockCoil shapeDouble( real *actshape, bool optSide, bool optLgt, bool optDpth );


public:
    ParamCoil( );
    ParamCoil( const ParamCoil& c );
    ~ParamCoil() { freeMem(); }

    int getNoSect( void ) { return nSect; }

    bool getConstr4Blck( int i, real& min, real& max );
    bool getConstr4Side( real& min, real& max );
    bool getConstr4Lgt ( real& min, real& max );
    bool getConstr4Dpth( real& min, real& max );
    real getDflt4Blck( int i );
    real getDflt4Side();
    real getDflt4Lgt();
    real getDflt4Dpth();

    void scan( std::istream& is );
    void print( std::ostream& os ) const;

    void setJ( const real, const real );
    void setPos( const Pt_real newpos );
    void setPos( const real x, const real y= 0, const real z= 0 );

    friend std::ostream& operator <<( std::ostream& os, const ParamCoil& c );
    friend std::istream& operator >>( std::istream& is, ParamCoil& c );

    BlockCoil shape( real *actshape, int nDV );
    BlockCoil shapeRel( real *actshape, int NDV );

    void printXML4Shape( std::ostream& os, real *actshape, int nDV, std::string additionalInfo );

    Pt_real midPt( void );
};
#endif

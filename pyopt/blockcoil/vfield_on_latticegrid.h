#ifndef _VFIELD_ON_LATTICEGRID_IS_INCLUDED_
#define _VFIELD_ON_LATTICEGRID_IS_INCLUDED_

#include "latticegrid.h"

class VFieldOnLatticeGrid  {
        private:
                Pt_real *v;
                LatticeGrid *g;

        public:
                VFieldOnLatticeGrid( LatticeGrid &g );
                ~VFieldOnLatticeGrid();

                int size() const;

                Pt_real operator()( const int i ) const;

                Pt_real derivativeAt( int i, int c ) const;  // derivatives of F_c at i

                LatticeGrid& grid();

                void computeAllValues( Pt_real (*f)( Pt_real ) );

                void setAllValues( Pt_real& v );

                void setValueAt( int i, Pt_real v );
};

#include <iostream>

std::ostream& operator <<( std::ostream& os, const VFieldOnLatticeGrid& f );

#endif

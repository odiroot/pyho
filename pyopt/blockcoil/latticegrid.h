#ifndef _LATTICEGRID_IS_INCLUDED_
#define _LATTICEGRID_IS_INCLUDED_

#include "grid.h"

class LatticeGrid : public Grid {
	private:
		Pt_real *p;
		int _size;
		Pt_real llc;
		Pt_real urc;
		int nx, ny, nz;
                real hx, hy, hz;

	public:
		LatticeGrid( Pt_real& llc, Pt_real& urc, int nx, int ny= 0, int nz= 0 );
		virtual ~LatticeGrid();

		virtual int size() const;
		virtual Pt_real operator()( const int i ) const;

                //rebuild
                void rebuild( int nx, int ny= 0, int nz= 0 );

                // get grid parameters
		int getNx();
		int getNy();
		int getNz();
                real gethx();
                real gethy();
                real gethz();
                void getdh( int i, real &dx, real &dy, real &dz );
                real getminX();
                real getminY();
                real getminZ();
                real getmaxX();
                real getmaxY();
                real getmaxZ();

		// calculate x,y and z layer for given node number
		int xlayer( int i );
		int ylayer( int i );
		int zlayer( int i );

                // get nodes relative to i: XM is ( x - hx ) XP is ( x + hx ) and so on
                int getXM( int i );
                int getXP( int i );
                int getYM( int i );
                int getYP( int i );
                int getZM( int i );
                int getZP( int i );
};

#include <iostream>

std::ostream& operator <<( std::ostream& os, const LatticeGrid& g );

#endif

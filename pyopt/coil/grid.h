#ifndef _GRID_H_IS_INCLUDED_
#define _GRID_H_IS_INCLUDED_

#include "ptreal.h"

class Grid {  // interface to grids
	public:
		Grid() {};
		virtual ~Grid() {};
		virtual int size() const = 0;
		virtual Pt_real operator()( const int i ) const = 0;
};
#endif

#include "vfield_on_latticegrid.h"

VFieldOnLatticeGrid::VFieldOnLatticeGrid( LatticeGrid &_g ) {
    this->g = &_g;
    v = new Pt_real[g->size()];
}

VFieldOnLatticeGrid::~VFieldOnLatticeGrid() {
}

int VFieldOnLatticeGrid::size() const {
    return g->size();
}

Pt_real VFieldOnLatticeGrid::operator()( const int i ) const {
    Pt_real tmp;
    if( i > 0 && i < g->size() )
        tmp= v[i];
    return tmp;
}

Pt_real VFieldOnLatticeGrid::derivativeAt( int i, int c ) const {
    Pt_real tmp;
    if( i > 0 && i < g->size() ) {
        int nn;
        // d F_c / dx
        if( (nn= g->getXP(i)) >= 0 ) { // forward diff
            tmp(1) = ( v[nn](c) - v[i](c) ) / g->gethx();
        } else { // backward diff
            nn= g->getXM(i);
            tmp(1) = ( v[i](c) - v[nn](c) ) / g->gethx();
        }
        // d F_c / dy
        if( (nn= g->getYP(i)) >= 0 ) { // forward diff
            tmp(2) = ( v[nn](c) - v[i](c) ) / g->gethy();
        } else { // backward diff
            nn= g->getYM(i);
            tmp(2) = ( v[i](c) - v[nn](c) ) / g->gethy();
        }
        // d F_c / dz
        if( (nn= g->getZP(i)) >= 0 ) { // forward diff
            tmp(3) = ( v[nn](c) - v[i](c) ) / g->gethz();
        } else { // backward diff
            nn= g->getZM(i);
            tmp(3) = ( v[i](c) - v[nn](c) ) / g->gethz();
        }
    }
    return tmp;
}

LatticeGrid& VFieldOnLatticeGrid::grid() {
    return *g;
}

void VFieldOnLatticeGrid::computeAllValues( Pt_real (*f)(Pt_real ))
{
    for( int i= 0; i < g->size(); i++ )
        v[i] = f( (*g)(i) );
}

void VFieldOnLatticeGrid::setAllValues( Pt_real& _v ) {
    for( int i= 0; i < g->size(); i++ )
        v[i] = _v;
}

void VFieldOnLatticeGrid::setValueAt( int i, Pt_real _v ) {
    if( i > 0 && i < g->size() )
        v[i] = _v;
}

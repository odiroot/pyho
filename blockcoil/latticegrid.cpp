#include "latticegrid.h"

using namespace std;

LatticeGrid::LatticeGrid( Pt_real& _llc, Pt_real& _urc, int _nx, int _ny, int _nz ) 
{
    _size= 0;
    p= 0;
    if( _nx < 1 )
        return;

    llc = _llc;
    urc = _urc;
    rebuild( _nx, _ny, _nz );
}

void LatticeGrid::rebuild(int _nx, int _ny, int _nz) {

    if( p != 0 ) delete [] p;

    nx = _nx;
    ny = _ny > 0 ? _ny : nx;
    nz = _nz > 0 ? _nz : ny;

    hx= ( urc(1) - llc(1) ) / ( nx - 1 );
    hy= ( urc(2) - llc(2) ) / ( ny - 1 );
    hz= ( urc(3) - llc(3) ) / ( nz - 1 );

    p = new Pt_real[nx*ny*nz];
    int n= 0;

    for( int i= 0; i < nz; i++ )
        for( int j= 0; j < ny; j++ )
            for( int k= 0; k < nx; k++ ) {
        p[n++] = llc + Pt_real( k*hx, j*hy, i*hz );
    }

    _size = n;
}

LatticeGrid::~LatticeGrid()
{
    //std::cerr << "LatticeGrid destructor\n";
    if( p != 0 )
        delete [] p;
    //std::cerr << "LatticeGrid destructor finished\n";
}

int LatticeGrid::size() const
{
    return _size;
}

Pt_real LatticeGrid::operator()( const int i ) const
{
    return p[i];
}

int LatticeGrid::getNx()
{
    return nx;
}

int LatticeGrid::getNy()
{
    return ny;
}

int LatticeGrid::getNz()
{
    return nz;
}

real LatticeGrid::gethx() {
    return hx;
}

real LatticeGrid::gethy() {
    return hy;
}

real LatticeGrid::gethz() {
    return hz;
}

void LatticeGrid::getdh(int i, real &dx, real &dy, real &dz) {
    // dx dy dz for point no. i
    dx= hx;
    dy= hy;
    dz= hz;
    int layer;
    layer= xlayer(i);
    if( layer == 0 || layer == nx-1 )
        dx /= 2;
    layer= ylayer(i);
    if( layer == 0 || layer == ny-1 )
        dy /= 2;
    layer= zlayer(i);
    if( layer == 0 || layer == nz-1 )
        dz /= 2;
}

real LatticeGrid::getminX() {
    return p[0].getX();
}

real LatticeGrid::getminY() {
    return p[0].getY();
}

real LatticeGrid::getminZ() {
    return p[0].getZ();
}

real LatticeGrid::getmaxX() {
    return p[_size-1].getX();
}

real LatticeGrid::getmaxY() {
    return p[_size-1].getY();
}

real LatticeGrid::getmaxZ() {
    return p[_size-1].getZ();
}

int LatticeGrid::xlayer( int i ) {
    return i % nx;
}

int LatticeGrid::ylayer( int i ) {
    i %= nx*ny;
    return i / nx;
}

int LatticeGrid::zlayer( int i ) {
    return i / (nx*ny);
}

int LatticeGrid::getXM( int i ) {
    if( xlayer(i) == 0 )
    	return -1;
    else
        return i-1;
}

int LatticeGrid::getXP( int i ) {
    if( xlayer( i ) == nx-1 )
        return -1;
    else
        return i+1;
}

int LatticeGrid::getYM( int i ) {
    if( ylayer( i ) == 0 )
        return -1;
    else
        return i-nx;
}

int LatticeGrid::getYP( int i ){
    if( ylayer( i ) == ny-1 )
        return -1;
    else
        return i+nx;
}

int LatticeGrid::getZM( int i ) {
    if( zlayer( i ) == 0 )
        return -1;
    else
        return i-nx*ny;
}

int LatticeGrid::getZP( int i ){
    if( zlayer( i ) == nz-1 )
        return -1;
    else
        return i+nx*ny;
}

ostream& operator <<( std::ostream& os, const LatticeGrid& g )
{
    os << "# ( " << g.size() << " ) grid = [\n";
    for( int i= 0; i < g.size(); i++ )
        os << g(i) << endl;
    os << "#]\n";
    return os;
}

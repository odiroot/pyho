#ifndef pt_real_h_is_included
#define pt_real_h_is_included

#include <math.h>
#include "mathconst.h"
#include <iostream>

class Pt_real {
private:
    real x[3];

public:
    Pt_real();
    Pt_real( real _xyz );
    Pt_real( real _x, real _y, real _z );
    Pt_real( const Pt_real& C );
    void fill( real v );
    void set( real v1, real v2, real v3 );
    ~Pt_real();

    real norm( void ) const;
    real norm2( void ) const;

    void mult( real v );

    void add( real a, Pt_real& p, real b, Pt_real& q );

    void scan( std::istream is );
    void print( std::ostream os );

    real getX() { return x[0]; };
    real getY() { return x[1]; };
    real getZ() { return x[2]; };

    real& operator()( const int i );
    Pt_real& operator=( const Pt_real& c );
    Pt_real& operator+=( const Pt_real& o );

    Pt_real& operator/=( const real& o );
    Pt_real& operator*=( const real& o );

    Pt_real operator+( const Pt_real& o ) const;
    Pt_real operator-( const Pt_real& o ) const;

    Pt_real operator*( const real& s ) const;
    Pt_real operator/( const real& s ) const;

    Pt_real cross( const Pt_real& o ) const;

    friend std::ostream& operator <<( std::ostream& os, const Pt_real& c );
    friend std::istream& operator >>( std::istream& is, Pt_real& c );

    friend Pt_real operator*( const real& s, const Pt_real& p );
    friend real operator*( const Pt_real& p1, const Pt_real& p2 );

};
#endif

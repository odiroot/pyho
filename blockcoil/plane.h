#ifndef _PLANE_H_IS_INCLUDED_
#define _PLANE_H_IS_INCLUDED_

#include "ptreal.h"
#include <math.h>
#include <iostream>

class Plane {
   private:
      real a, b, c, d;  // Plane: a*x+b*y+c*z = d
      real out;         // sign of "outer" direction

   public:
      Plane( ) { a= b= c= 1; d= 0; out= 1; }
      Plane( const Plane& P ) { a= P.a; b= P.b; c= P.c; d= P.d; out= P.out; }
      Plane( real A, real B, real C, real D, real O= 1 ) { a= A; b= B; c= C; d= D; out= O; }
      Plane( Pt_real p, Pt_real n );
      ~Plane() {}

      int ok( ) { return out != 0.0; }

      void def( real A, real B, real C, real D, real O= 1 ) { a= A; b= B; c= C; d= D; out= O; }
      void def( Pt_real p, Pt_real n );

      void scan( std::istream is ) { is >> a >> b >> c >> d >> out; }
      void print( std::ostream os ) { os << a << ' ' << b << ' ' << c << ' ' << d << ' ' << out << '\n'; }

      Plane& operator=( const Plane& x );

      friend std::ostream& operator <<( std::ostream& os, const Plane& x );
      friend std::istream& operator >>( std::istream& is, Plane& x ) { is >> x.a >> x.b >> x.c >> x.d >> x.out; return is; }

      int outside( Pt_real p ) { return (a*p(1)+b*p(2)+c*p(3)-d)*out > 0; }

};
#endif

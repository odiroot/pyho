#include "plane.h"
#include <iostream>

Plane& Plane::operator=( const Plane& x ) {
   a= x.a;
   b= x.b;
   c= x.c;
   d= x.d;
   out= x.out;
   return *this;
}

Plane::Plane( Pt_real p, Pt_real n ) {
   if( n.norm() < comparison_tolerance ) {
      a= b= c= 1;
      d= 0;
      out= 1;
   } else {
      a= n(1);
      b= n(2);
      c= n(3);
      d= p(1)*n(1)+p(2)*n(2)+p(3)*n(3);
      out= a*(p(1)+n(1))+b*(p(2)+n(2))+c*(p(3)+n(3))-d > 0 ? 1 : -1;
   }
}

void Plane::def( Pt_real p, Pt_real n ) {
   if( n.norm() < comparison_tolerance ) {
      a= b= c= 1;
      d= 0;
      out= 1;
   } else {
      a= n(1);
      b= n(2);
      c= n(3);
      d= p(1)*n(1)+p(2)*n(2)+p(3)*n(3);
      out= a*(p(1)+n(1))+b*(p(2)+n(2))+c*(p(3)+n(3))-d > 0 ? 1 : -1;
   }
}

std::ostream& operator <<( std::ostream& os, const Plane& x ) {
   os << x.a << ' ' << x.b << ' ' << x.c << ' ' << x.d << ' ' << x.out << '\n';
   return os;
}

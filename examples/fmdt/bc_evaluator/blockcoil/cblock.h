#ifndef _CBLOCK_H_IS_INCLUDED_
#define _CBLOCK_H_IS_INCLUDED_

#include <math.h>
#include <iostream>
#include "ptreal.h"
#include "plane.h"

class C_Block {
   private:
      Pt_real p, x, y, z;       // spaning vectors
      Pt_real nx, ny, nz, ony;  // normal vectors  (ony is nx x ny for rhomboidal block)
      real sin, cos;            // sin and cos of the x,y angle
      real cross;               // cross-sections
      real xl, yl, zl;          // sides' lengths
      real bbr;                 // bounding radius
      real jr, ji;              // current density

   public:
      C_Block( ) { p.fill(0); x.fill(0.); y.fill(0.); z.fill(0); jr= 0.; ji= 0.; }
      C_Block( const C_Block& C ) { p= C.p; x= C.x; y= C.y; z= C.z; jr= C.jr; ji= C.ji; this->norm(); }
      C_Block( Pt_real P, Pt_real X, Pt_real Y, Pt_real Z, real Jr, real Ji ) { p= P; x= X; y= Y; z= Z; jr= Jr; ji= Ji ; this->norm(); }
      C_Block( Pt_real P, Pt_real X, Pt_real Y, real L, real Jr, real Ji);
      C_Block( real Cross, Pt_real P, Pt_real Z, Pt_real X, real Jr, real Ji );
      ~C_Block() {};

   private:
      inline real scalar( Pt_real, Pt_real );
      Pt_real vector( Pt_real, Pt_real );

      void norm( void );

   public:
      void set( Pt_real P, Pt_real X, Pt_real Y, Pt_real Z, real Jr, real Ji ) { p= P; x= X; y= Y; z= Z; jr= Jr; ji= Ji; this->norm(); }
      void set( Pt_real P, Pt_real X, Pt_real Y, real L, real Jr, real Ji);
      void set( real Cross, Pt_real P, Pt_real Z, Pt_real X, real Jr, real Ji );

      void setJ( real Jr, real Ji = 0);
      void setL( real L );

      void scan( std::istream& is );
      void print( std::ostream& os ) const;

      void print_VRML( std::ostream& os );

      C_Block& operator=( const C_Block& c );

      bool is_inside( Pt_real );

      Pt_real B( Pt_real x0 );
      Pt_real A( Pt_real x0 );

			real volume();
      
      friend std::ostream& operator <<( std::ostream& os, const C_Block& c );
      friend std::istream& operator >>( std::istream& is, C_Block& c );

};
#endif

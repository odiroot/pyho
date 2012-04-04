#ifndef _BLOCKCOIL_H_IS_INCLUDED_
#define _BLOCKCOIL_H_IS_INCLUDED_

#include <string>
#include "cblock.h"

class BlockCoil {
  private:
    int nBlck;
    C_Block *blcks;
    int blcksSize;  // not yet used

  public:
    BlockCoil();
    BlockCoil( int n, C_Block *t );
    ~BlockCoil();

    bool is_ok() { return nBlck > 0; }

    BlockCoil& operator=( const BlockCoil& c );

    void scan( std::istream& is );
    void setJ( const real jr, const real ji= 0 );
    void print( std::ostream& os ) const;

    bool is_inside( Pt_real p );
    Pt_real B( Pt_real x0 );
    Pt_real A( Pt_real x0 );
    real volume();
};

#include <iostream>

std::ostream& operator <<( std::ostream& os, const BlockCoil& c );
std::istream& operator >>( std::istream& is, BlockCoil& c );

#endif

#include "blockcoil.h"
#include <assert.h>

using namespace std;

BlockCoil::BlockCoil( )
{
	nBlck = 0;
	blcks = 0;
	blcksSize= 0;
}

BlockCoil::BlockCoil( int n, C_Block *t )
{
	nBlck= n;
	blcksSize= n;
	blcks = t;
}

BlockCoil::~BlockCoil()
{
	//std::cerr << "BlockCoil::destructor\n";
	if( blcks != 0 ) {
	  //std::cerr << "BlockCoil::destructor: n=" << nBlck << ", blcks=" << blcks << "\n";
		delete [] blcks;
	}
	//std::cerr << "BlockCoil::finished destructor\n";
}

BlockCoil& BlockCoil::operator=( const BlockCoil& c ) {
	nBlck = c.nBlck;
	blcksSize= c.blcksSize;
	blcks = new C_Block[blcksSize];
	for( int i= 0; i < nBlck; i++ ) {
	 	blcks[i] = c.blcks[i];
	}
    return *this;
}

void BlockCoil::scan( std::istream& is )
{
	if( blcks != 0 ) {
		delete [] blcks;
		blcks = 0;
	}
	nBlck = 0;
	blcksSize = 0;
	if( !is.good() ) {
		return;
    }
	string s;
	do {
		is >> s;
	} while( !is.eof() && s[s.size()-1] != '>' );
	if( is.eof() ) {
		return;
    }
	is >> nBlck;
	blcksSize= nBlck;
	blcks = new C_Block[nBlck];
	for( int i= 0; i < nBlck; i++ ) {
		is >> blcks[i];
    }
}

void BlockCoil::setJ( const real jr, const real ji )
{
	for( int i= 0; i < nBlck; i++ ) {
		blcks[i].setJ( jr, ji );
    }
}

void BlockCoil::print( std::ostream& os ) const
{
	os << "> " << nBlck << endl;
	for( int i= 0; i < nBlck; i++ ) {
		os << blcks[i] << endl;
    }
}

bool BlockCoil::is_inside( Pt_real p )
{
	for( int i= 0; i < nBlck; i++ ) {
		if( blcks[i].is_inside( p ) ) {
			return true;
        }
    }
	return false;
}

Pt_real BlockCoil::B( Pt_real x0 )
{
	Pt_real B( 0, 0, 0 );
	for( int i= 0; i < nBlck; i++ ) {
		B += blcks[i].B( x0 );
    }
	return B;
}

Pt_real BlockCoil::A( Pt_real x0 )
{
	Pt_real A( 0, 0, 0 );
	for( int i= 0; i < nBlck; i++ ) {
		A += blcks[i].A( x0 );
    }
	return A;
}

real BlockCoil::volume() 
{
	real v= 0;
	for( int i= 0; i < nBlck; i++ ) {
		v += blcks[i].volume();
    }
	return v;
}

std::ostream& operator <<( std::ostream& os, const BlockCoil& c )
{
	c.print( os );
	return os;
}

std::istream& operator >>( std::istream& is, BlockCoil& c )
{
	c.scan( is );
	return is;
}

#include "ptreal.h"

Pt_real::Pt_real()
{
	x[0]= x[1]= x[2]= 0;
}

Pt_real::~Pt_real()
{
};

Pt_real::Pt_real( real _xyz )
{
	x[0]= x[1]= x[2]= _xyz;
}

Pt_real::Pt_real( real _x, real _y, real _z )
{
	x[0]= _x;
	x[1]= _y;
	x[2] = _z;
}

Pt_real::Pt_real( const Pt_real& C )
{
	x[0]= C.x[0];
	x[1]= C.x[1];
	x[2]= C.x[2];
}

void Pt_real::fill( real v )
{
	x[0]= x[1]= x[2]= v;
}

void Pt_real::set( real v1, real v2, real v3 )
{
	x[0]= v1;
	x[1]= v2;
	x[2]= v3;
}

real Pt_real::norm( void ) const
{
	return sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
}

real Pt_real::norm2( void ) const
{
    return  x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
}

void Pt_real::mult( real v )
{
	x[0] *= v;
	x[1] *= v;
	x[2] *= v;
}

void Pt_real::add(real a, Pt_real & p, real b, Pt_real & q) 
{
    for (int i = 0; i < 3; i++)
        x[i] = a * p.x[i] + b * q.x[i];
}

real & Pt_real::operator()(const int i)  {
    return x[i - 1];
}

Pt_real & Pt_real::operator=(const Pt_real & c)
{
    x[0] = c.x[0];
    x[1] = c.x[1];
    x[2] = c.x[2];
    return *this;
}

Pt_real & Pt_real::operator+=(const Pt_real & o)
{
    for (int i = 0; i < 3; i++)
        x[i] += o.x[i];
    return *this;
}

Pt_real & Pt_real::operator/=(const real & o)
{
    for (int i = 0; i < 3; i++)
        x[i] /= o;
    return *this;
}

Pt_real & Pt_real::operator*=(const real & o)
{
    for (int i = 0; i < 3; i++)
        x[i] *= o;
    return *this;
}

Pt_real Pt_real::operator+(const Pt_real & o) const
{
    Pt_real tmp;
    for (int i = 0; i < 3; i++)
        tmp.x[i] = x[i] + o.x[i];
    return tmp;
}
               
Pt_real Pt_real::operator-(const Pt_real & o) const
{
    Pt_real tmp;
    for (int i = 0; i < 3; i++)
        tmp.x[i] = x[i] - o.x[i];
    return tmp;
} 
               
Pt_real Pt_real::operator*(const real & s) const
{
    Pt_real tmp;
    for (int i = 0; i < 3; i++)
        tmp.x[i] = s * x[i];
    return tmp;
}
               
Pt_real Pt_real::operator/(const real & s) const
{
    Pt_real tmp;
    for (int i = 0; i < 3; i++)
        tmp.x[i] = x[i] / s;
    return tmp;
}

Pt_real Pt_real::cross( const Pt_real& o ) const
{
	Pt_real tmp;
	tmp.x[0] = x[1]*o.x[2]-x[2]*o.x[1];
	tmp.x[1] = x[2]*o.x[0]-x[0]*o.x[2];
	tmp.x[2] = x[0]*o.x[1]-x[1]*o.x[0];
	return tmp;
}

void Pt_real::scan( std::istream is )
{
	is >> x[0] >> x[1] >> x[2];
}

void Pt_real::print( std::ostream os )
{
	os << x[0] << " " << x[1] << " " << x[2];
}
               
std::ostream& operator <<(std::ostream & os, const Pt_real & c)
{
    os << c.x[0] << " " << c.x[1] << " " << c.x[2];
    return os;
}

std::istream & operator >>(std::istream & is, Pt_real & c)
{
    is >> c.x[0] >> c.x[1] >> c.x[2];
    return is;
}

Pt_real operator*(const real & s, const Pt_real & p)
{
    Pt_real tmp;
    for (int i = 0; i < 3; i++)
        tmp.x[i] = s * p.x[i];
    return tmp;
}

real operator*(const Pt_real & p1, const Pt_real & p2)
{
    real tmp= 0;
    for (int i = 0; i < 3; i++)
        tmp += p1.x[i] * p2.x[i];
    return tmp;
}

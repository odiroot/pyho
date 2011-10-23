#include "cblock.h"
#include "errhandler.h"
#include <stdlib.h>

C_Block& C_Block::operator=( const C_Block& c )
                           // copy constructor
                           {
    p= c.p;
    x= c.x;
    y= c.y;
    z= c.z;
    jr= c.jr;
    ji= c.ji;
    this->norm();
    return *this;
}

C_Block::C_Block( Pt_real P, Pt_real X, Pt_real Y, real L, real Jr, real Ji )
        // P - origin, X,Y spanning vectors, L length in XxY direction
{
    this->set( P, X, Y, L, Jr, Ji );
}

C_Block::C_Block( real Cross, Pt_real P, Pt_real Z, Pt_real X, real Jr, real Ji )
        // Cross - cross-section
        // P - origin, Z - length and direction,
        // X - one of the spanning vectors,
{
    this->set( Cross, P, Z, X, Jr, Ji );
}

void C_Block::set( Pt_real P, Pt_real X, Pt_real Y, real L, real Jr, real Ji ) {
    Pt_real Z;
    Z(1)= X(2)*Y(3)-X(3)*Y(2);  // X x Y
    Z(2)= X(3)*Y(1)-X(1)*Y(3);
    Z(3)= X(1)*Y(2)-X(2)*Y(1);
    real n= Z.norm();
    if( n <= 0 ) {
        std::cerr << "P=" << P <<" X=" << X << " Y=" << Y << " Z=" << Z << std::endl;
        fatalerror("C_Block::set", " defining vectors can not be parallel!" );
    }
    L /= n;
    Z.mult( L );
    p= P; x= X; y= Y; z= Z; jr= Jr; ji= Ji;
    this->norm();
}

void C_Block::set( real Cross, Pt_real P, Pt_real Z, Pt_real X, real Jr, real Ji )
        // Cross - cross-section
        // P - origin, Z - length and direction,
        // X - one of the spanning vectors,
{
    P += Z/2;
    Pt_real Y;  // the other spanning vector
    Y(1)= -X(2)*Z(3)+X(3)*Z(2);  // X x -Z
    Y(2)= -X(3)*Z(1)+X(1)*Z(3);
    Y(3)= -X(1)*Z(2)+X(2)*Z(1);
    real n= Y.norm();
    if( n <= 0 )  {
        std::cerr << "P=" << P <<" Z=" << Z << " X=" << X << " Y=" << Y << std::endl;
        fatalerror("C_Block::set", " defining vectors can not be paralel!" );
    }
    // Y.norm()*X.norm() == Cross
    real Yl = Cross / X.norm();
    Yl /= n;
    Y.mult(Yl);
    if( Z.norm() == 0 )
        fatalerror("C_Block::constructor", "can not create block with zero length!" );
    p= P; x= X; y= Y; z= Z; jr= Jr, ji= Ji;
    this->norm();
}

void C_Block::setL( real L ) {
    L /= z.norm();
    z.mult( L );
    this->norm();
}

void C_Block::setJ( real Jr, real Ji ) {
    jr= Jr;
    ji= Ji;
}

void C_Block:: norm( void ) {
    xl= x.norm();
    nx= x / xl;
    yl= y.norm();
    ny= y / yl;
    zl= z.norm();
    nz= z/ zl;
    bbr = sqrt( xl*xl + yl*yl );
    cos= scalar( nx, ny );
    sin= sqrt( 1 - cos*cos );
    cross= xl*yl*sin;
    if( cross < 0 )
        fatalerror("C_Block::norm", "  negative cross-section !?!" );
    if( fabs(cos) < comparison_tolerance ) { // rectangular
        sin= 1;
        cos= 0;
        ony= ny;
    } else // rhomboid
        ony= vector( nz, nx );
    //s_e << '\n';
    //s_e << "cos= " << cos << " sin= " << sin << " nx= " << nx << '\n';
    //s_e << "ny = " << ny << '\n';
    //s_e << "ony= " << ony << '\n';
}

void C_Block::scan( std::istream& is ) {
    Pt_real P, X, Y;
    real L, Jr, Ji;
    is >> P >> X >> Y >> L >> Jr >> Ji;
    this->set( P, X, Y, L, Jr, Ji );
}

inline void C_Block::print( std::ostream& os ) const { os << p << ' ' << x << ' ' << y << ' ' << z.norm() << ' ' << jr << ' ' << ji; }

void C_Block::print_VRML( std::ostream& os ) {
    Pt_real p0= p + z*0.5;
    os << "   Shape{  \n";
    os << "           appearance Appearance { material Material { }}\n";
    os << "           geometry IndexedFaceSet {\n";
    os << "                   coord Coordinate { point [\n";
    os << "                      " << p0 << "\n";
    os << "                      " << p0+x << "\n";
    os << "                      " << p0+x+y << "\n";
    os << "                      " << p0+y << "\n";
    p0= p - z*0.5;
    os << "                      " << p0 << "\n";
    os << "                      " << p0+x << "\n";
    os << "                      " << p0+x+y << "\n";
    os << "                      " << p0+y << "\n";
    os << "                   ] }\n";
    os << "                   \n";
    os << "                   coordIndex [0 1 2 3 -1, 4 7 6 5 -1,\n"
            "                               1 5 6 2 -1, 2 6 7 3 -1,\n"
            "                               3 7 4 0 -1, 0 4 5 1 ]\n";
    os << "                   color Color { color [0.5 0.5 0.5]}\n";
    os << "                   colorIndex [ 0 0 0 0 0 0 ]\n";
    os << "                   colorPerVertex FALSE\n";
    os << "                   creaseAngle 0\n";
    os << "                   solid TRUE\n";
    os << "                   ccw TRUE\n";
    os << "                   convex TRUE\n";
    os << "           }\n";
    os << "   }\n";
}

std::ostream& operator <<( std::ostream& os, const C_Block& c ) {
    Pt_real nz = c.z;
    os << c.p << ' ' << c.x << ' ' << c.y << ' ' << nz.norm() << ' ' << c.jr << ' ' << c.ji;
    return os;
}

std::istream& operator >>( std::istream& is, C_Block& c ) {
    Pt_real P, X, Y;
    real L, Jr, Ji;
    is >> P >> X >> Y >> L >> Jr >> Ji;
    c.set( P, X, Y, L, Jr, Ji );
    //s_e << "\nBLOK: " << c.p << c.x << c.y << c.z << c.jr << c.ji;

    return is;
}

inline real C_Block:: scalar( Pt_real a, Pt_real b ) {
    return a(1)*b(1)+a(2)*b(2)+a(3)*b(3);  // 3D vectors assumed !!!
}

Pt_real C_Block:: vector( Pt_real a, Pt_real b ) {
    return Pt_real ( a(2)*b(3)-a(3)*b(2), a(3)*b(1)-a(1)*b(3), a(1)*b(2)-a(2)*b(1) );
}

bool C_Block:: is_inside( Pt_real x0 ) {
    // Local system: assume that 0X points right, 0Y points up, 0Z is right-oriented
    // check if x0 is "left" of the Block
    Plane s( p, -1*nx );
    if( s.outside( x0 ) )
        return false;
    // check if x0 is "below" of the Block
    s.def( p, -1*ony );
    if( s.outside( x0 ) )
        return false;
    // check if x0 is "right" of the Block
    s.def( p+x, nx );
    if( s.outside( x0 ) )
        return false;
    // check if x0 is "above" of the Block
    s.def( p+y, ony );
    if( s.outside( x0 ) )
        return false;
    // check if it is "in front of
    Pt_real z2= z / 2;
    s.def( p+z2, nz );
    if( s.outside( x0 ) )
        return false;
    // and finally - if it is "behind"
    s.def( p-z2, -1*nz );
    if( s.outside( x0 ) )
        return false;
    //
    return true;
}

// numerical integration
real xip[] = { -0.861136311594053, -0.339981043584856, 0.339981043584856, 0.861136311594053}; // coord-s of integration points
real wip[] = {0.347854845137454, 0.652145154862546, 0.652145154862546, 0.347854845137454 }; // weights corresp. to xip
int lpc= sizeof xip / sizeof xip[0];

Pt_real C_Block::B( Pt_real x0 ) {

    Pt_real Br, Bi;

    if( is_inside( x0 ) ) {
        warning( "C_Block::B", format( (char*)"Point (%g,%g,%g) is inside of the C_Block - can't calculate B!!!", x0(1), x0(2), x0(3) ) );
        Br.fill(0.0);
        return Br;
    }

    // transform x0 to local system of this C_Block
    Pt_real tx0= x0 - p;  // translate 000 to p
    Pt_real lx0( scalar( nx, tx0 ), scalar( ony, tx0 ), scalar( nz, tx0 ) );  // rotate
    // lx0 is in local (to CBlock) coordinate system.

    // check relation of the 2D point position to the size of the block
    real rx0 = sqrt(lx0(1)*lx0(1)+lx0(2)*lx0(2));
    // block will be divided into smaller blocks if the distance is smaller
    // than BIGR x <size of the block>
    const double BIGR = 5;
    int n_dx = rx0 > BIGR*xl ? 1 : (int) (BIGR*xl / rx0);
    int n_dy = rx0 > BIGR*yl ? 1 : (int) (BIGR*yl / rx0);

#if 0 // only for debugging
		std::cerr << "O=(" << p << ") xl=" << xl << " yl=" << yl;
		std::cerr << " p=(" << x0 << ") r=" << rx0;
		std::cerr << " n_dx=" << n_dx << " n_dy=" << n_dy << std::endl;
#endif

    // prepare for numerical integration
    real Bx= 0, By= 0;
    real a2= xl / 2 / n_dx;
    real b2= yl / 2 / n_dy;
    real l2= zl / 2;
    real lpz= l2 + lx0(3); real lpz2= lpz*lpz;
    real lmz= l2 - lx0(3); real lmz2= lmz*lmz;
    for( int idx= 0; idx < n_dx; idx++ ) {
        for( int idy= 0; idy < n_dy; idy++ ) {
            if( cos == 0 ) { // rectangle
                real xbase = (idx * xl) / n_dx;
                real ybase = (idy * yl) / n_dy;
                // numerical quadrature is defined for the [-1;1]x[-1:1] square
                for( int ix= 0; ix < lpc; ix++ ) { // numerical integration along ox
                    real xc= (1+xip[ix])*a2 + xbase;
                    real x_x0= lx0(1) - xc;
                    real x_x02= x_x0*x_x0;
                    for( int iy= 0; iy < lpc; iy++ ) { // numerical integration along oy
                        real yc= (1+xip[iy])*b2 + ybase;
                        real y_y0= lx0(2) - yc;
                        real y_y02= y_y0*y_y0;
                        real r2= x_x02 + y_y02;
                        real sqrtp= sqrt( r2 + lpz2 );
                        real sqrtm= sqrt( r2 + lmz2 );
#if 1
// production code ;-)
                        real Bphi_r= 1 / r2 * ( lmz / sqrtm + lpz / sqrtp ) * wip[ix]*wip[iy];
                        Bx -= y_y0 * Bphi_r;
                        By += x_x0 * Bphi_r;
#else
// testing code
                        real Bphi_r = 1 / sqrt(r2) * ( lmz / sqrtm + lpz / sqrtp ) * wip[ix]*wip[iy];
                        Pt_real rv( x_x0, y_y0, 0 );
                        Pt_real z( 0, 0, 1 );
                        rv /= rv.norm();
                        Pt_real bd = z.cross( rv );
                        Bx += Bphi_r * bd.getX();
                        By += Bphi_r * bd.getY();
#endif
                    }
                }
            } else { // rhomboid (x and y are not perpendicular)
                std::cout << "\nRHOMBOID NOT IMPLEMENTED YET!!!\n";
                exit( 13 );
                for( int ix= 0; ix < lpc; ix++ ) { // numerical integration along ox
                    for( int iy= 0; iy < lpc; iy++ ) { // numerical integration along oy
                        real yc= (1+xip[iy])*b2;
                        real xc= (1+xip[ix])*a2;
                        xc += cos*yc;  // here we have to add some of dy to dx
                        yc *= sin;     // and modify dy
                        real x_x0= lx0(1) - xc;
                        real x_x02= x_x0*x_x0;
                        real y_y0= lx0(2) - yc;
                        real y_y02= y_y0*y_y0;
                        real sum2= x_x02 + y_y02;
                        real sqrtp= sqrt( sum2 + lpz2 );
                        real sqrtm= sqrt( sum2 + lmz2 );
                        real cxy= 1 / sum2 * ( lmz / sqrtm + lpz / sqrtp ) * wip[ix]*wip[iy];
                        Bx -= y_y0 * cxy;
                        By += x_x0 * cxy;
                    }
                }
            }
        }
    }
    real areaWeight = cross / 4.0 / (n_dx*n_dy ); // real_area / area_of_[-1;1]^2 / no_of_divisions
    real Brx = Bx * 1e-7 * jr * areaWeight;  // \miu_0 / (4*pi) gives 1e-7
    real Bry = By * 1e-7 * jr * areaWeight;
    real Bix = Bx * 1e-7 * ji * areaWeight;
    real Biy = By * 1e-7 * ji * areaWeight;

#ifdef DEBUG
    std::cerr << "Bloc=(" << Brx << "," << Bry << ")\n";
#endif

    // transform B to the global coordinate system
    Br(1)=  Brx*nx(1)+Bry*ny(1);
    Br(2)=  Brx*nx(2)+Bry*ny(2);
    Br(3)=  Brx*nx(3)+Bry*ny(3);
    Bi(1)=  Bix*nx(1)+Biy*ny(1);
    Bi(2)=  Bix*nx(2)+Biy*ny(2);
    Bi(3)=  Bix*nx(3)+Biy*ny(3);

    return Br;

}

// metoda zwracajaca A w dowolnym punkcie poza C_Blockiem
Pt_real C_Block::A( Pt_real x0 ) {
    Pt_real Ar, Ai;

    if ( is_inside( x ) ) {
        warning( "C_Block::A", format( (char*)"Point (%g,%g,%g) is inside of the C_Block - can't calculate A!!!", x(1) , x(2) , x(3) ) );
        Ar.fill (0.0);
        return Ar;
    }

    Pt_real tx0 = x0 - p ;  //przesuniecie punktu x w uklad lokalny
    Pt_real lx0( scalar(nx , tx0) , scalar (ony , tx0) , scalar (nz, tx0) ); //gotowe polozenie punku x w ukladzie lokalnym

    Ar.fill(0);
    Ai.fill(0);
    real a2 = xl/2; //polowy dlugosci cewki
    real b2 = yl/2;
    real l2 = zl/2;
    //  real lpz = l2 + lx0(3); real lpz2 = lpz * lpz;
    //  real lmz = l2 - lx0(3); real lmz2 = lmz * lmz;


    real Apomoc=0 , distan=0 ;

    for (int ix = 0 ;  ix < lpc ; ix++){
        for ( int iy = 0 ; iy < lpc ; iy++){
            for ( int iz = 0 ; iz < lpc ; iz++){
                real yc = (1 + xip[iy]) * b2;	//dobrze do wspolrzednych w lokalnym
                real xc = (1 + xip[ix]) * a2;	//tez
                real zc = (1 + xip[iz]) * l2;	//tez

                distan = sqrt( (lx0(1) - xc) * (lx0(1) - xc) + (lx0(2) - yc) * (lx0(2) -yc) + (lx0(3)-zc) * (lx0(3)-zc) );
                Apomoc += wip[ix] * wip[iy] * wip[iz] /distan;  //pole 4 znormalizowanego kwadratu
                //do poprawienia po Z i scalkowac po trzech wymiarach na razie po dwoch zbyteczne
                //	Arx += z(1) * jr * xip[ix] * wip[iy] * 1e-7; // p.distance(lx0); /// proba dobra
            }
        }
    }
    real cuss=sqrt(  z(1)*z(1)+z(2)*z(2)+z(3)*z(3)  );    	//dlugosc wektora
    real cus=xl*yl*zl/8;   //stosunek objetosci rzeczywistego do jednostkowego
    Ar(1) = z(1) * jr * 1e-7 * Apomoc*cus/cuss;
    Ar(2) = z(2) * jr * 1e-7 * Apomoc*cus/cuss;
    Ar(3) = z(3) * jr * 1e-7 * Apomoc*cus/cuss;
    Ai(1) = z(1) * ji * 1e-7 * Apomoc*cus/cuss;
    Ai(2) = z(2) * ji * 1e-7 * Apomoc*cus/cuss;
    Ai(3) = z(3) * ji * 1e-7 * Apomoc*cus/cuss;
    //cout <<' zet '<< z(1)<<' aaaa ' << A(1) <<' jjjjj '<<jr<<endl;
    //cout <<z(1) << ' ' << z(2) << '  '<<'   ' << z(3)   <<endl;
    //cout << xl <<'  ' << yl << '   ' <<  zl <<endl;
    //cerr << L<<endl;
    return Ar;
}

// objetosc
real C_Block::volume() {
    return cross*zl;
}

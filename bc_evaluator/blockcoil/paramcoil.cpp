#include "paramcoil.h"
#include <assert.h>

using namespace std;

ParamCoil::ParamCoil( )
{
    ngen = 0;
    pos.set( 0, 0, 0 );
    nSect= 0;
    sectParm= 0;
    l= w= 0;
    lgtParm[0] = lgtParm[1]= lgtParm[2]= lgtParm[3]= 0;
    dpthParm[0] = dpthParm[1]= dpthParm[2]= dpthParm[3]= 0;
    dblCnct= false;
    varH= false;
    jr= 0.; ji= 0.;
}

ParamCoil::ParamCoil( const ParamCoil& c )
{
    ngen = 0;
    pos = c.pos;
    nSect= c.nSect;
    sectParm = new sectParm_t[nSect];
    for( int i= 0; i < nSect; i++ )
        for( int j= 0; j < 4; j++ )
            sectParm[i][j] = c.sectParm[i][j];
    w= c.w;
    l= c.l;
    dblCnct= c.dblCnct;
    varH= c.varH;
    for( int i= 0; i < 4; i++ ) {
        sideParm[i] = c.sideParm[i];
        lgtParm[i] = c.lgtParm[i];
        dpthParm[i] = c.dpthParm[i];
    }
    jr= c.jr;
    ji= c.ji;
}

void ParamCoil::scan( std::istream& is )
{
    freeMem();
    is >> pos;
    is >> nSect;
    is >> dpthParm[1];
    is >> dpthParm[2];
    is >> dpthParm[3];
    sectParm = new sectParm_t[nSect];
    for( int i = 0; i < nSect; i++ )
        for( int j = 0; j < 4; j++ )
            is >> sectParm[i][j];
    is >> l;
    lgtParm[1] = l;
    is >> lgtParm[2];
    is >> lgtParm[3];
    is >> w;
    string tmp;
    is >> tmp;
    if( tmp == "double" )
        dblCnct = true;
    else
        dblCnct = false;
    is >> tmp;
    if( tmp == "h" )
        varH = true;
    else
        varH = false;
    for( int i = 1; i < 4; i++ )
        is >> sideParm[i];
    is >> jr >> ji;
}

void ParamCoil::setJ( const real _jr, const real _ji )
{
    jr = _jr;
    ji = _ji;
}

void ParamCoil::setPos( const Pt_real newPos )
{
    pos = newPos;
}

void ParamCoil::setPos( const real x, const real y, const real z )
{
    pos.set( x, y, z );
}

bool ParamCoil::getConstr4Blck( int i, real& min, real& max )
{
    if( i >= 0 && i < nSect ) {
        min= sectParm[i][2];
        max= sectParm[i][3];
        return true;
    } else {
        min= max= 0;
        return false;
    }
}

real ParamCoil::getDflt4Blck( int i ) 
{
    if( i >= 0 && i < nSect ) {
        return sectParm[i][1];
    } else {
        return 0;
    }
}

bool ParamCoil::getConstr4Side( real& min, real& max )
{
    if( nSect > 0 ) {
        min= sideParm[2];
        max= sideParm[3];
        return true;
    } else {
        min= max= 0;
        return false;
    }
}

real ParamCoil::getDflt4Side() 
{
    return sideParm[1];
}

bool ParamCoil::getConstr4Lgt( real& min, real& max )
{
    if( nSect > 0 ) {
        min= lgtParm[2];
        max= lgtParm[3];
        return true;
    } else {
        min= max= 0;
        return false;
    }
}

real ParamCoil::getDflt4Lgt() 
{
    return lgtParm[1];
}

bool ParamCoil::getConstr4Dpth( real& min, real& max )
{
    if( nSect > 0 ) {
        min= dpthParm[2];
        max= dpthParm[3];
        return true;
    } else {
        min= max= 0;
        return false;
    }
}

real ParamCoil::getDflt4Dpth()
{
    return dpthParm[1];
}

void ParamCoil::print( std::ostream& os ) const
{
    os << pos << endl;
    os << nSect << endl;
    for( int i = 1; i < 4; i++ )
        os << ' ' << dpthParm[i];
    for( int i = 0; i < nSect; i++ ) {
        for( int j = 0; j < 4; j++ )
            os << ' ' << sectParm[i][j];
        os << endl;
    }
    for( int i = 1; i < 4; i++ )
        os << ' ' << lgtParm[i];
    os << w << endl;
    if( dblCnct )
        os << "double\n";
    else
        os << "single\n";
    if( varH )
        os << "h\n";
    else
        os << "w\n";
    for( int i = 1; i < 4; i++ )
        os << ' ' << sideParm[i];
    os << endl;
    os << jr << ' ' << ji << endl;
}

static real htr( real cx, real b )
        // Wysokosc trojkata prostokatnego opuszczona
        // na przeciwprostokatna.
        // b - jedna z przyprostokatnych
        // cx - kawalek przeciwprostokatnej pomiędzy
        //      wierzcholkiem naprzeciw b i rzutem wysokosci
{
    real cx2 = cx*cx;
    real delta = cx2*cx2+4*b*b*cx2;
    real t = 0.5*(sqrt(delta)-cx2);
    return sqrt(t);
}

void ParamCoil::printXML4Shape( ostream& os, real *shape, int nDV, string additionalInfo ) {
    // shape:
    // 				0..(nSect-1)  - sections' widths
    //        nSect         - side-to-side connection optimal dimension (v or h)
    //        nSect+1       - length
    //        nSect+2       - sections' depth (one for all)
    os << "<coil";
    os<<  " internalWidth=\"" << w << "\"";
    os << " totalHeight=\"" << nSect*(shape != 0 && nDV > nSect+2 ? shape[nSect+2] : dpthParm[1]) << "\"";
    os << " sectionHeight=\"" << (shape != 0 && nDV > nSect+2 ? shape[nSect+2] : dpthParm[1]) << "\"";
    os << " sectionHeight_Min=\"" << dpthParm[2] << "\"";
    os << " sectionHeight_Max=\"" << dpthParm[3] << "\"";
    os << " fixedDim=\"" << (varH ? "h" : "v" ) << "\"";
    os << " description=\"The Coil\"";
    os << " J=\"" << jr << "\"";
    os << " fixedValue=\"" << (shape != 0 && nDV > nSect ? shape[nSect] : sideParm[1] ) << "\"";
    os << " fixedValue_Min=\"" << sideParm[2] << "\"";
    os << " fixedValue_Max=\"" << sideParm[3] << "\"";
    os << " length=\"" << (shape != 0 && nDV > nSect+1 ? shape[nSect+1] : lgtParm[1] ) << "\"";
    os << " length_min=\"" << lgtParm[2] << "\"";
    os << " length_max=\"" << lgtParm[3] << "\"";
    os << " numberOfSections=\"" << nSect << "\"";
    os << " connectionType=\"" << ( dblCnct ? "Double" : "Single" ) << "\" >" << endl;
    os << "\t<location x=\"" << pos.getX() << "\" y=\"" << pos.getY() << "\" z=\"" << pos.getZ() << "\" />" << endl;

		if( shape == 0 ) {
    	for( int i= 0; i < nSect; i++ )
        os << "\t<section dx=\"" << sectParm[i][1] << "\" dy=\"" << dpthParm[1] << "\" maxx=\"" << sectParm[i][3] << "\" minx=\"" << sectParm[i][2] << "\" number=\"" << i << "\" />" << endl;
		} else {
    	for( int i= 0; i < nSect; i++ )
        os << "\t<section dx=\"" << shape[i] << "\" dy=\"" << (nDV > nSect+2 ? shape[nSect+2] : dpthParm[1]) << "\" maxx=\"" << sectParm[i][3] << "\" minx=\"" << sectParm[i][2] << "\" number=\"" << i << "\" />" << endl;
		}

    os << additionalInfo;
    os << "</coil>" << endl;
}

BlockCoil ParamCoil::shape( real *actshape, int nDV )
        // actshape:
        // 				0..(nSect-1)  - sections' widths
        //        nSect         - side-to-side connection optimal dimension (v or h)
        //        nSect+1       - length
        //        nSect+2       - sections' depth (one for all)
{
    // check if optimize side connections, length, depth
    bool optimizeSide= false;
    bool optimizeLength= false;
    bool optimizeDepth= false;
    if( actshape != 0 ) {
        if( nDV > nSect && actshape[nSect] >= sideParm[2] && actshape[nSect] <= sideParm[3] )
            optimizeSide = true;
        if( nDV > nSect+1 && actshape[nSect+1] >= lgtParm[2] && actshape[nSect+1] <= lgtParm[3] )
            optimizeLength = true;
        if( nDV > nSect+2 && actshape[nSect+2] >= dpthParm[2] && actshape[nSect+2] <= dpthParm[3] )
            optimizeDepth = true;
    }
    if( nDV < nSect ) {
        return BlockCoil();
    } else {
        if( dblCnct )
            return shapeDouble( actshape, optimizeSide, optimizeLength, optimizeDepth );
        else
            return shapeSingle( actshape, optimizeSide, optimizeLength, optimizeDepth );
    }
}

BlockCoil ParamCoil::shapeSingle( real *actshape, bool optimizeSide, bool optimizeLength, bool optimizeDepth )
{
    real cross= 0;
    real x= pos(1);
    real y0= pos(2);
    real y= pos(2);
    real z= pos(3);
    real g= 0;
    real h= 0;
    int nB= countBlck();
    C_Block *tmp = new C_Block[nB];
    int n= 0;


    if( optimizeLength )
        l= actshape[nSect+1];
    else
        l= lgtParm[1];

    // dimensions of side-to-side connection
    // first: calculate cross-section of all the wires
    for( int i= 0; i < nSect; i++ ) {
        // check if depth is optimized
        if( optimizeDepth )
            h= actshape[nSect+2];
        else
            h = sectParm[i][0];
        g = actshape == 0 || actshape[i] == 0.0 ? sectParm[i][1] : actshape[i];
        cross += h*g; // cumulate cross-section
    }
    real s2sVar = optimizeSide ? actshape[nSect] : sideParm[1];
    real s2sFixd = cross / s2sVar; // side-to-side cross-sect must agree
    real sideZspan= 0;
    real sideYspan= 0;
    // top connections
    if( varH ) { // height (Y-direction) varies
        sideYspan = s2sFixd;
        sideZspan = s2sVar;
    } else { // width (Z-direction) varies
        sideYspan = s2sVar;
        sideZspan = s2sFixd;
    }

    // prepare for vertical connections
    if( optimizeDepth )
        h= actshape[nSect+2];
    else
        h = sectParm[0][0];
    real z0 = htr( h+sideYspan/2, h );
    // the working part - both sides at once
    for( int i= 0; i < nSect; i++ ) {
        // check if depth is optimized
        if( optimizeDepth )
            h= actshape[nSect+2];
        else
            h = sectParm[i][0];
        g = actshape == 0 || actshape[i] == 0.0 ? sectParm[i][1] : actshape[i];
        //if( actshape != 0 )std::cerr << actshape[i] << "->";
        //std::cerr << "h,g: " << h << ',' << g << ' ';
        cross += h*g; // cumulate cross-section
        y -= h;
        // horizontal part
        // x+
        tmp[n++].set(      Pt_real( w/2+x, y, z ),     Pt_real(g,0,0),  Pt_real( 0, h, 0 ), l, jr, ji );
        tmp[n++].set( h*g, Pt_real( w/2+x, y, z+l/2 ), Pt_real(0, y0-y+sideYspan/2, z0+i*(sideZspan-z0)/nSect), Pt_real(g,0,0), jr, ji );
        tmp[n++].set( h*g, Pt_real( w/2+g+x, y, z-l/2 ), Pt_real(0, y0-y+sideYspan/2, -z0-i*(sideZspan-z0)/nSect), Pt_real(-g,0,0), -jr, -ji );
        // x-
        tmp[n++].set(      Pt_real( -w/2+x, y, z ), Pt_real(-g,0,0), Pt_real( 0, h, 0 ), l, jr, ji );
        tmp[n++].set( h*g, Pt_real( -w/2-g+x, y, z+l/2 ), Pt_real(0, y0-y+sideYspan/2, z0+i*(sideZspan-z0)/nSect), Pt_real(g,0,0), -jr, -ji );
        tmp[n++].set( h*g, Pt_real( -w/2+x, y, z-l/2 ), Pt_real(0, y0-y+sideYspan/2, -z0-i*(sideZspan-z0)/nSect), Pt_real(-g,0,0), jr, ji );
    }
    // side-to-side connection(s):
    // top connections only
    // front
    tmp[n++].set( Pt_real( x, y0, z+l/2 ), Pt_real( 0, 0, sideZspan ), Pt_real( 0, sideYspan, 0 ), w, jr, ji );
    // back
    tmp[n++].set( Pt_real( x, y0, z-l/2 ), Pt_real( 0, 0, -sideZspan ), Pt_real( 0, sideYspan, 0 ), w, jr, ji );
    assert( n == nB );
    return BlockCoil( nB, tmp);
}

BlockCoil ParamCoil::shapeDouble( real *actshape, bool optimizeSide, bool optimizeLength, bool optimizeDepth )
{
    real cross= 0;
    real x= pos(1);
    real y0= pos(2);
    real y= pos(2);
    real z= pos(3);
    real g= 0;
    real h= 0;
    int nB= countBlck();
    C_Block *tmp = new C_Block[nB];
    int n= 0;
    // check if optimize length
    if( optimizeLength )
        l= actshape[nSect+1];
    else
        l= lgtParm[1];

    // dimensions of side-to-side connection
    // first: calculate cross-section of all the wires
    //        and total height of the coil
    real hTot= 0;
    for( int i= 0; i < nSect; i++ ) {
        // check if depth is optimized
        if( optimizeDepth )
            h= actshape[nSect+2];
        else
            h = sectParm[i][0];
        g = actshape == 0 || actshape[i] == 0.0 ? sectParm[i][1] : actshape[i];
        cross += h*g; // cumulate cross-section
        hTot += h;    // and height
    }
    real s2sVar = optimizeSide? actshape[nSect] : sideParm[1];
    real s2sFixd = cross / s2sVar / 2 ; // total side-to-side cross-sect must agree
    real sideZspan= 0;
    real sideYspan= 0;
    // top connections
    if( varH ) { // height (Y-direction) varies
        sideYspan = s2sFixd;
        sideZspan = s2sVar;
    } else { // width (Z-direction) varies
        sideYspan = s2sVar;
        sideZspan = s2sFixd;
    }

    // prepare for vertical connections
    if( optimizeDepth )
        h= actshape[nSect+2];
    else
        h = sectParm[0][0];

    // the working part - top - both sides at once
    for( int i= 0; i < nSect/2; i++ ) {
        // check if depth is optimized
        if( optimizeDepth )
            h= actshape[nSect+2];
        else
            h = sectParm[i][0];
        g = actshape == 0 || actshape[i] == 0.0 ? sectParm[i][1] : actshape[i];
        //if( actshape != 0 )std::cerr << actshape[i] << "->";
        //std::cerr << "h,g: " << h << ',' << g << ' ';
        cross += h*g; // cumulate cross-section
        y -= h;
        // x+
        // working
        tmp[n++].set(      Pt_real( w/2+x, y, z ),     Pt_real(g,0,0),  Pt_real( 0, h, 0 ), l, jr, ji );
        // prepare for vertical
        real vert_dx = 0;
        real vert_dy = y0+sideYspan/2-(y+h/2);
        real vert_dz = (i+1)*2*sideZspan/nSect;
        real sinal = vert_dy/sqrt(vert_dy*vert_dy+vert_dz*vert_dz);
        real cosal = vert_dz/sqrt(vert_dy*vert_dy+vert_dz*vert_dz);
        real deltay = h/2*(1-cosal);
        real deltaz = h/2*sinal;
        // vert (z+)
        tmp[n++].set( h*g, Pt_real( w/2+x, y+deltay, z+l/2+deltaz ), Pt_real( vert_dx, vert_dy, vert_dz ), Pt_real(g,0,0), jr, ji );
        // vert (z-)
        tmp[n++].set( h*g, Pt_real( w/2+g+x, y+deltay, z-l/2-deltaz ), Pt_real( vert_dx, vert_dy, -vert_dz ), Pt_real(-g,0,0), -jr, -ji );
        // x-
        // working
        tmp[n++].set(      Pt_real( -w/2+x, y, z ), Pt_real(-g,0,0), Pt_real( 0, h, 0 ), l, jr, ji );
        // vert (z+)
        tmp[n++].set( h*g, Pt_real( -w/2-g+x, y+deltay, z+l/2+deltaz ), Pt_real( vert_dx, vert_dy, vert_dz ), Pt_real(g,0,0), -jr, -ji );
        // vert (z-)
        tmp[n++].set( h*g, Pt_real( -w/2+x, y+deltay, z-l/2-deltaz ), Pt_real( vert_dx, vert_dy, -vert_dz ), Pt_real(-g,0,0), jr, ji );
        //std::cerr << "First loop: " << i << endl;
    }
    // the working part - bottom - both sides at once
    for( int i= nSect/2, j= 0; i < nSect; i++,j++ ) {
        // check if depth is optimized
        if( optimizeDepth )
            h= actshape[nSect+2];
        else
            h = sectParm[i][0];
        g = actshape == 0 || actshape[i] == 0.0 ? sectParm[i][1] : actshape[i];
        //if( actshape != 0 )std::cerr << actshape[i] << "->";
        //std::cerr << "h,g: " << h << ',' << g << ' ';
        cross += h*g; // cumulate cross-section
        y -= h;
        // x+
        // working
        tmp[n++].set(      Pt_real( w/2+x, y, z ),     Pt_real(g,0,0),  Pt_real( 0, h, 0 ), l, jr, ji );
        // prepare for vertical
        real vert_dx = 0;
        real vert_dy = -(hTot+y+h/2+sideYspan/2);
        real vert_dz = (nSect-i)*2*sideZspan/nSect;
        real sinal = vert_dy/sqrt(vert_dy*vert_dy+vert_dz*vert_dz);
        real cosal = vert_dz/sqrt(vert_dy*vert_dy+vert_dz*vert_dz);
        real deltay = -h/2*(1-cosal);
        real deltaz = -h/2*sinal;
        // vert (z+)
        tmp[n++].set( h*g, Pt_real( w/2+g+x, y+h+deltay, z+l/2+deltaz ), Pt_real( vert_dx, vert_dy, vert_dz ), Pt_real(-g,0,0), jr, ji );
        // vert (z-)
        tmp[n++].set( h*g, Pt_real( w/2+x, y+h+deltay, z-l/2-deltaz ), Pt_real( vert_dx, vert_dy, -vert_dz ), Pt_real(g,0,0), -jr, -ji );
        // x-
        // working
        tmp[n++].set(      Pt_real( -w/2+x, y, z ), Pt_real(-g,0,0), Pt_real( 0, h, 0 ), l, jr, ji );
        // vert (z+)
        tmp[n++].set( h*g, Pt_real( -w/2+x, y+h+deltay, z+l/2+deltaz ), Pt_real( vert_dx, vert_dy, vert_dz ), Pt_real(-g,0,0), -jr, -ji );
        // vert (z-)
        tmp[n++].set( h*g, Pt_real( -w/2-g+x, y+h+deltay, z-l/2-deltaz ), Pt_real( vert_dx, vert_dy, -vert_dz ), Pt_real(g,0,0), jr, ji );
        //std::cerr << "Second loop: " << i << endl;
    }
    // side-to-side connection(s):
    // top connections
    // front
    tmp[n++].set( Pt_real( x, y0, z+l/2 ), Pt_real( 0, 0, sideZspan ), Pt_real( 0, sideYspan, 0 ), w, jr, ji );
    // back
    tmp[n++].set( Pt_real( x, y0, z-l/2 ), Pt_real( 0, 0, -sideZspan ), Pt_real( 0, sideYspan, 0 ), w, jr, ji );
    // bottom connections
    // front
    tmp[n++].set( Pt_real( x, y, z+l/2 ), Pt_real( 0, -sideYspan, 0 ), Pt_real( 0, 0, sideZspan ), w, jr, ji );
    // back
    tmp[n++].set( Pt_real( x, y, z-l/2 ), Pt_real( 0, -sideYspan, 0 ), Pt_real( 0, 0, -sideZspan ), w, jr, ji );
    //std::cerr << "n= " << n << " nB=" << nB << std::endl;
    assert( n == nB );
    return BlockCoil( nB, tmp);
}

BlockCoil ParamCoil::shapeRel( real *actshapeRelative, int nDV )
{
    real *actshape = 0;
    if( actshapeRelative != 0 ) {
        actshape = new real[nSect+1];
        for( int i = 0; i < nSect; i++ )
            if( actshapeRelative[i] < 0 )
                actshape[i] = 0;
        else
            actshape[i] = sectParm[i][2] + actshapeRelative[i]*( sectParm[i][3] - sectParm[i][2] );

        if( nDV > nSect && actshapeRelative[nSect] < 0 )
            actshape[nSect] = 0;
        else
            actshape[nSect] = sideParm[2] + actshapeRelative[nSect]*(sideParm[3] - sideParm[2] );

        if( nDV > nSect+1 && actshapeRelative[nSect+1] < 0 )
            actshape[nSect+1] = 0;
        else
            actshape[nSect+1] = lgtParm[2] + actshapeRelative[nSect+1]*(lgtParm[3] - lgtParm[2] );

        if( nDV > nSect+2 && actshapeRelative[nSect+2] < 0 )
            actshape[nSect+2] = 0;
        else
            actshape[nSect+2] = dpthParm[2] + actshapeRelative[nSect+2]*(dpthParm[3] - dpthParm[2] );
    }
    BlockCoil result = shape( actshape, nDV );
    if( actshape != 0 )
        delete [] actshape;
    return result;
}

Pt_real ParamCoil::midPt( void )
{
    real d= 0;
    for( int i= 0; i < nSect; i++ )
        d += sectParm[i][0];
    d /= 2;
    return Pt_real( pos(1), pos(2)-d, pos(3) );
}

void ParamCoil::freeMem( void )
{
    //std::cerr << "ParamCoil::freeMem\n";
    if( sectParm != 0 ) {
        delete [] sectParm;
        //std::cerr << "ParamCoil::freeMem: sectParm deleted\n";
    }
    nSect= 0;
    sectParm= 0;
    //std::cerr << "ParamCoil::freeMem: finished\n";
}

int ParamCoil::countBlck(void)
{
    int nBlck= 0;
    if( nSect > 0 ) {
        nBlck= 6*nSect + 2; // both sides + top connect.
        if( dblCnct )  // add connect. at the bottom
            nBlck += 2;
    }
    return nBlck;
}

std::ostream& operator <<( std::ostream& os, const ParamCoil& c )
{
    c.print( os );
    return os;
}

std::istream& operator >>( std::istream& is, ParamCoil& c )
{
    c.scan( is );
    return is;
}

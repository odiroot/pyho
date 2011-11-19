#include <fstream>
#include <cstring>

#include "opt_utils.h"

using namespace std;

void writeXML( char *filename, 
                                                         int noDesignVars, real *parameters,
               int nCntrComps, int * cntrComps, Pt_real& Bwanted,
							 LatticeGrid *grid, int onx, int ony, int onz, 
               ParamCoil& paramCoil,
               BlockCoil& bestC, real bestRslt, Pt_real& Bmean, real stdDev
             )
{
  ofstream xmlOut( filename );
     
  xmlOut << "<optimization>" << endl;
  xmlOut << "\t<coil_evaluation criteriaField=\"B\">" << endl;

  paramCoil.printXML4Shape( xmlOut, parameters, noDesignVars, "" );

  xmlOut << "\t\t<vector_field name=\"B\" dim=\"3\" nVals=\""<< (*grid).size() << "\" >" << endl;
  xmlOut << "\t\t\t<mean x=\"" << Bmean.getX() << "\" y=\"" << Bmean.getY() << "\" z=\"" << Bmean.getZ() << "\" />" << endl;

  if( nCntrComps != 0 ) {
    xmlOut << "\t\t\t<eval typeEvaluation=\"toValue\" result=\"" << bestRslt << "\">" << endl;
    xmlOut << "\t\t\t\t<std_dev value=\"" << stdDev << "\" />" << endl;
    int used_comp[3];
    for( int j= 0; j < 3; j++ )
			used_comp[j] = 0;
		for( int j= 0; j < nCntrComps; j++ ) {
			int c= cntrComps[j];
			used_comp[c-1]= 1;
      xmlOut << "\t\t\t\t<objective component=\"" << c << "\" controlled=\"true\" value=\""<< Bwanted(c) << "\" />" << endl;
		}
		for( int j= 0; j < 3; j++ ) {
      if( used_comp[j] == 0 )
        xmlOut << "\t\t\t\t<objective component=\"" << j+1 << "\" controlled=\"false\" />" << endl;
    }
    xmlOut << "\t\t\t</eval>" << endl;
  } else {
    xmlOut << "\t\t\t<eval typeEvaluation=\"toMean\" result=\"" << bestRslt << "\">" << endl;
    xmlOut << "\t\t\t\t<std_dev value=\"" << stdDev << "\" />" << endl;
    xmlOut << "\t\t\t</eval>" << endl;
  }
  xmlOut << "\t\t\t<grid>" << endl;
  xmlOut << "\t\t\t<densityEval x=\"" << grid->getNx() << "\" y=\"" << grid->getNy() << "\" z=\"" << grid->getNz() << "\" />" << endl;
  xmlOut << "\t\t\t<densityOpt x=\"" << onx << "\" y=\"" << ony << "\" z=\"" << onz << "\" />" << endl;
  xmlOut << "\t\t\t<vertex x=\"" << grid->getminX() << "\" y=\"" << grid->getminY() << "\" z=\"" << grid->getminZ() << "\" />" << endl;
  xmlOut << "\t\t\t<vertex x=\"" << grid->getmaxX() << "\" y=\"" << grid->getmaxY() << "\" z=\"" << grid->getmaxZ() << "\" />" << endl;
  xmlOut << "\t\t\t</grid>" << endl;

  Pt_real mean;
  mean.fill(0.0);	  
	for (int p=0; p < (*grid).size(); p++) {
	 	Pt_real vecB = bestC.B( (*grid)(p) );
    mean += vecB;
		xmlOut << "\t\t\t<val ind=\"" << p << "\" "
		       << "vx=\"" << vecB.getX() << "\" "
		       << "vy=\"" << vecB.getY() << "\" "
		       << "vz=\"" << vecB.getZ() << "\" />"
	  		   << endl;
	}
  mean /= (*grid).size();
  // For testing cout << "Mean=" << mean << endl;

  xmlOut << "\t\t</vector_field>" << endl;


  xmlOut << "\t\t<grid>" << endl;
  xmlOut << "\t\t\t<densityEval x=\"" << grid->getNx() << "\" y=\"" << grid->getNy() << "\" z=\"" << grid->getNz() << "\" />" << endl;
  xmlOut << "\t\t\t<densityOpt x=\"" << onx << "\" y=\"" << ony << "\" z=\"" << onz << "\" />" << endl;
 // A FUJ!: tak mialo nie byc! xmlOut << "\t\t\t<density x=\"" << grid->getNx() << "\" y=\"" << grid->getNy() << "\" z=\"" << grid->getNz() << "\" />" << endl;
  xmlOut << "\t\t\t<vertex x=\"" << grid->getminX() << "\" y=\"" << grid->getminY() << "\" z=\"" << grid->getminZ() << "\" />" << endl;
  xmlOut << "\t\t\t<vertex x=\"" << grid->getmaxX() << "\" y=\"" << grid->getmaxY() << "\" z=\"" << grid->getmaxZ() << "\" />" << endl;
  xmlOut << "\t\t</grid>" << endl;

  xmlOut << "\t</coil_evaluation>" << endl;
	xmlOut << "</optimization>" << endl;
	xmlOut.close();
}

#include "errhandler.h"
#include <iostream>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

using namespace std;

void fatalerror( string where, string msg )
{
        cout << endl << "FATAL ERROR: " << where << " " << msg << endl;
  exit(1);
}
	
void warning( string where, string msg )
{
        cout << "WARNING: " << where << " " << msg << endl;
}

std::string format( char *fmt, ...) 
{ 
   string retStr("");
   char buffer[1024];

   if (fmt != NULL)
   {
      va_list marker;

      // initalize variable arguments

      va_start(marker, fmt); 
      
      // Get formatted string length adding one for the NULL

      int nWritten = vsnprintf(buffer, 1000, fmt, marker);    

      if (nWritten > 0)
      {
         retStr = &buffer[0];
      }
            
      // Reset variable arguments

      va_end(marker); 
   }

   return retStr; 
}

#ifndef _errhandler_is_included_
#define _errhandler_is_included_

#include <string>

void fatalerror( std::string where, std::string msg );
void warning( std::string where, std::string msg );
std::string format( char *fmt, ... );

#endif

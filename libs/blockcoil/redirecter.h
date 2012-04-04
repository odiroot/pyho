#include <iostream>

// redirected can be used to redirect cout to the logfile
class redirecter
{
public:
    redirecter(std::ostream & dst, std::ostream & src) : src(src), sbuf(src.rdbuf(dst.rdbuf())) {}
    ~redirecter() { src.rdbuf(sbuf); }
private:
    std::ostream & src;
    std::streambuf * const sbuf;
};

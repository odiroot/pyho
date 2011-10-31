#ifndef _STOPPER_H_IS_INCLUDED_
#define _STOPPER_H_IS_INCLUDED_

#include <sys/time.h>

class stopper {
  private:
    struct timeval t1, t2, t1lap;

  public:
		stopper() { start(); }
	  ~stopper() {}

		void start() { gettimeofday(&t1, 0); t1lap= t1; }
		double stop() { gettimeofday(&t2, 0); return (1000000.0*(t2.tv_sec-t1.tv_sec) + t2.tv_usec-t1.tv_usec)/1000000.0; }
		double lap() { gettimeofday(&t2, 0); double tmp= (1000000.0*(t2.tv_sec-t1lap.tv_sec) + t2.tv_usec-t1lap.tv_usec)/1000000.0; t1lap= t2; return tmp; }
};
#endif

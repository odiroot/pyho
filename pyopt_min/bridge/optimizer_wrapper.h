#include "mathconst.h"

#ifndef optimizer_wrapper_h
#define optimizer_wrapper_h
extern "C" {

int get_coil(const char* coil_path, int& nsect, int& nvars);
void prepare_constraints();
int get_grid(const char* grid_path, int& npoints);
void cost_function(float Bx, float By, float Bz, int flag);
void prepare_reference_value(float& stddev, float& err);

int get_no_design_vars();
real* get_my_min();
real* get_my_max();

float bFun(real t[]);
void print_coil(real t[]);
void output_cblock(const char* path, real t[]);
void rebuild_grid(int fine);
void output_xml(char* path, real t[]);

const int ARG_BX = 1;
const int ARG_BY = 2;
const int ARG_BZ = 4;

}

#endif

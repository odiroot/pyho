#ifndef optimizer_wrapper_h
#define optimizer_wrapper_h
extern "C" {

void c_print(const char* text);
void redirect_log(const char* path);
void get_coil(const char* coil_path);
void prepare_constraints();
void get_grid(const char* grid_path);
void cost_function(float Bx, float By, float Bz, int flag);
void prepare_reference_value();

int get_no_design_vars();
float* get_my_min();
float* get_my_max();

float bFun(float t[]);

const int ARG_BX = 1;
const int ARG_BY = 2;
const int ARG_BZ = 4;

int eval_server(const char* address);

}
#endif

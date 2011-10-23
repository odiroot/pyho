cimport cython
import time
import sys
cdef extern from "optimizer_wrapper.h":
    void c_print(char* text)
    void redirect_log(char* path)
    void get_coil(char* coil_path)
    void prepare_constraints()
    void get_grid(char* grid_path)
    void cost_function(float Bx, float By, float Bz, int flag)
    void prepare_reference_value()

    int get_no_design_vars()
    float* get_my_min()
    float* get_my_max()

    float bFun(float t[])

    int ARG_BX
    int ARG_BY
    int ARG_BZ

    int eval_server(char* address)

cdef extern from "stdlib.h":
    void free(void* ptr)
    void* malloc(size_t size)
    void* realloc(void* ptr, size_t size)


def cprint(object string):
    string = unicode(string)
    c_print(string)


def prepare(object args):
    # Initialize log file.
    if args.logfile:
        redirect_log(args.logfile)

    cprint("PyHO :: Started the GA coil optimizer at: %s\n" % time.asctime())
    # Prepare param coil.
    get_coil(args.coil)
    prepare_constraints()
    # Get grid.
    get_grid(args.grid)
    # Cost function.
    cdef int flag  # Testing for arguments.
    flag = args.Bx and ARG_BX or 0
    flag |= args.By and ARG_BY or 0
    flag |= args.Bz and ARG_BZ or 0
    cost_function(args.Bx or 0., args.By or 0., args.Bz or 0., flag)
    # Prepare the reference value.
    prepare_reference_value()


def get_optimization_params():
    no_vars = get_no_design_vars()
    cdef float* c_my_min = get_my_min()
    cdef float* c_my_max = get_my_max()

    my_min = []
    my_max = []
    for num in c_my_min[:no_vars]:
        my_min.append(round(num, 9))
    for num in c_my_max[:no_vars]:
        my_max.append(round(num, 9))

    return no_vars, my_min, my_max

def bfun(object genome):
    cdef int size = get_no_design_vars()
    cdef float* t = <float*>malloc(size * sizeof(float))
    # Rewrite Python list to C floats.
    for i in range(size):
        t[i] = genome[i]
    cdef float res = bFun(t)
    free(t)
    return res

def start_eval_server(address_string):
    cdef int res
    res = eval_server(address_string)

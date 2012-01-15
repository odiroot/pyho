cimport cython
import time
import sys

cdef extern from *:
    ctypedef float real

cdef extern from "optimizer_wrapper.h":
    void redirect_log(char* path)
    void get_coil(char* coil_path, int& nsect, int& nvars)
    void prepare_constraints()
    void get_grid(char* grid_path)
    void cost_function(float Bx, float By, float Bz, int flag)
    void prepare_reference_value()

    int get_no_design_vars()
    real* get_my_min()
    real* get_my_max()

    float bFun(real t[])
    void print_best_coil(real t[])
    void output_cblock(char* path)
    void rebuild_grid(int fine)
    void output_xml(char* path, real t[])

    int ARG_BX
    int ARG_BY
    int ARG_BZ

cdef extern from "stdlib.h":
    void free(void* ptr)
    void* malloc(size_t size)
    void* realloc(void* ptr, size_t size)


def prepare(object args):
    # Initialize log file.
    if args.logfile:
        redirect_log(args.logfile)

    print "PyHO :: Started the block coil optimizer evaluation module",
    print "at: %s\n" % time.asctime()

    # Prepare param coil.
    cdef int nsect, nvars
    get_coil(args.coil, nsect, nvars)
    print "Read parametric description of the coil.",
    print "%d sections => %d design variables" % (nsect, nvars)
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
    cdef real* c_my_min = get_my_min()
    cdef real* c_my_max = get_my_max()

    my_min = []
    my_max = []
    for num in c_my_min[:no_vars]:
        my_min.append(round(num, 9))
    for num in c_my_max[:no_vars]:
        my_max.append(round(num, 9))

    return no_vars, my_min, my_max

cdef real* list_to_real(object genome):
    cdef int size = get_no_design_vars()
    cdef real* t = <real*>malloc(size * sizeof(real))
    # Rewrite Python list to C floats.
    for i in range(size):
        t[i] = genome[i]
    return t


def bfun(object genome):
    cdef real* t = list_to_real(genome)
    cdef float res = bFun(t)
    free(t)
    return res

def print_best(object genome):
    cdef real* t = list_to_real(genome)
    print_best_coil(t)
    print "PyHO :: coil design finished at: %s\n" % time.asctime()
    free(t)

def save_cblock(object path):
    output_cblock(path)

def rebuild(int fine):
    rebuild_grid(fine)

def save_xml(object path, object genome):
    cdef real* t = list_to_real(genome)
    output_xml(path, t)
    free(t)

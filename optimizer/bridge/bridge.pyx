cimport cython
import time
import sys

cdef extern from *:
    ctypedef float real

cdef extern from "optimizer_wrapper.h":
    void redirect_log(char* path)
    int get_coil(char* coil_path, int& nsect, int& nvars)
    void prepare_constraints()
    int get_grid(char* grid_path, int& npoints)
    void cost_function(float Bx, float By, float Bz, int flag)
    void prepare_reference_value(float& stddev, float& err)

    int get_no_design_vars()
    real* get_my_min()
    real* get_my_max()

    float bFun(real t[])
    void print_coil(real t[])
    void output_cblock(char* path, real t[])
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
    cdef int res
    # XXX: What to do with the log file?
    print ("PyHO :: Started the block coil optimizer evaluation module"
        " at: %s\n" % time.asctime())

    # Prepare param coil.
    cdef int nsect, nvars
    res = get_coil(args.coil, nsect, nvars)
    if res == 0:
        print "Read parametric description of the coil.",
        print "%d sections => %d design variables" % (nsect, nvars)
    else:
        raise RuntimeError("Can't find parametric description of the coil.")

    prepare_constraints()

    # Get grid.
    cdef int npoints
    res = get_grid(args.grid, npoints)
    if res == 0:
        print "Working on a grid of %d points" % npoints
    else:
        raise RuntimeError("Can't find grid.")

    # Cost function.
    cdef int flag  # Testing for arguments.
    flag = args.Bx and ARG_BX or 0
    flag |= args.By and ARG_BY or 0
    flag |= args.Bz and ARG_BZ or 0
    cost_function(args.Bx or 0., args.By or 0., args.Bz or 0., flag)

    # Prepare the reference coil and evaluate.
    cdef float std_dev, err
    prepare_reference_value(std_dev, err)
    print ("Starting with reference coil: std. dev. = %g, error = %g" %
        (std_dev, err))


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

def coil_to_print(object genome):
    cdef real* t = list_to_real(genome)
    print_coil(t)
    free(t)

def save_cblock(object path, object genome):
    cdef real* t = list_to_real(genome)
    output_cblock(path, t)
    free(t)

def save_xml(object path, object genome, int fine=0):
    # Rebuild grid with new density if fine is specified.
    if fine > 0:
        rebuild_grid(fine)
    cdef real* t = list_to_real(genome)
    output_xml(path, t)
    free(t)

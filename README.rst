======================================
PyHO - Hybrid coil optimizer in Python
======================================

This project at its current state is based on the original C++ optimizer module 
from `FMDT` (Flow Meter Design Tool) project.

Current status
--------------
- Reading original input file formats (Coil and Grid definitions) and execution arguments
- Coil optimization with a (simple) genetic algorithm engine
    - Support for original (customized) genetic operators
    - Support for `PyEvolve` specific `allele` operators
- Optimization speedup using memoization
- Parallel processing using network or local worker processes
    - `Local mode` with automatic worker management (``-local-workers`` option, one worker by default)
    - `Network mode` with arbitrary number of worker nodes
- Currently optimizer and worker processes need access to the same file system (passing files through the network channel is not implemented nor designed)
- Optimizer can be successfully launched by the `FMDT` GUI.


Dependencies
------------

**PyHO has some hard dependencies**:

* ``zmq`` Python package (called ``python-zmq`` in Debian based systems) - version 2.1.9 or newer is highly recommended
* The `ZeroMQ` library (or ``libzmq1``) with sources (``libzmq-dev``) - again the 2.1.9+ version is preferred.

If you want to use hybrid optimization mode make sure you also have:

* Source distribution of BLAS (``libblas-dev``) and LAPACK (``liblapack-dev``) libraries
* The `NumPy` Python library (try ``python-numpy``)


PyHO also uses following Python packages:

* ``Cython``
* ``pyximport``
* ``PyEvolve``
* ``levmar`` (modified version supporting user initiated breaks)

All these libraries are included in the ``libs`` directory for convenience.
When present in the standard Python path, system installed versions take
precedence.

**A note of warning**: This project uses custom ``levmar`` package, officially available version cannot be used, as it will break during hybrid optimization.


Quick start
-----------

You can start the optimizer directly by launching ``run_optimizer`` Python
script from the root project directory. Running it with ``--help`` option
will show all available options.

You can use example evaluator based on FMDT project by using ``run_bc_evaluator``
shell script. There are example coil/grid definition files located in ``assets`` 
directory under ``examples/fmdt`` path.

There are 32-bit Linux and 64-bit Linux precompiled binaries of `FMDT` GUI application packaged
in a ``GUI`` directory. Run it by calling ``Kanal_v1`` shell script.

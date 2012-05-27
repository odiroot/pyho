======================================
PyHO - Hybrid coil optimizer in Python
======================================

PyHO is a prototype of an universal two-stage optimizer written in Python.

This project is based on the original C++ optimizer module from `FMDT`
(Flow Meter Design Tool) project.

Current status
--------------
- Objective function optimization with a genetic algorithm engine
    - Support for customized genetic operators
    - Support for `PyEvolve` specific `allele` operators
- Second stage optimization with Levenberg-Marquardt algorithm
- Optimization speedup using memoization
- Parallel processing using network or local worker processes
    - `Local mode` with automatic worker management
    - `Network mode` with arbitrary number of worker nodes
- Currently optimizer and worker processes need access to the same file system
  (passing files through the network channel is not implemented nor designed)
- Reading original FMDT input file formats (coil and grid definitions)
- Optimizer can be successfully launched by the `FMDT` GUI.


Dependencies
------------

**PyHO has some hard dependencies**:

* ``zmq`` Python package (called ``python-zmq`` in Debian based systems) -
  version 2.1.9 or newer is highly recommended
* The `ZeroMQ` library (or ``libzmq1``) with sources (``libzmq-dev``) - again
  the 2.1.9+ version is preferred.

If you want to use hybrid optimization mode make sure you also have:

* Source distribution of BLAS (``libblas-dev``) and LAPACK (``liblapack-dev``) libraries
* The `NumPy` Python library (try ``python-numpy``)


PyHO also uses following Python packages:

* ``Cython`` (version at least 0.16)
* ``pyximport``
* ``PyEvolve``
* ``levmar`` (modified version supporting user initiated breaks)

All these libraries are included in the ``libs`` directory for convenience.
When present in the standard Python path, system installed versions take
precedence.

**A note of warning**: This project uses custom ``levmar`` package, officially
available version cannot be used, as it will break during hybrid optimization.


Quick start
-----------

You can start the optimizer directly by launching ``run_optimizer`` Python
script from the root project directory. Running it with ``--help`` option
will show all available options.

You can use example evaluator based on FMDT project by using ``run_bc_evaluator``
shell script. There are example coil/grid definition files located in ``assets``
directory under ``examples/fmdt/`` path.

There are 32-bit Linux and 64-bit Linux precompiled binaries of `FMDT` GUI
application packaged in a ``GUI`` directory. Run it by calling ``Kanal_v1``
shell script.

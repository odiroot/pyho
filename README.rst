======================================
PyHO - Hybrid coil optimizer in Python
======================================

This project at its current state is a Python port (and a wrapper) of the
original C++ optimizer module from `FMDT` (Flow Meter Design Tool) project.

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
- Optimizer can be successfully launched by the `FMDT` GUI. Optimization progress is being displayed but no output files are saved yet (to be done)


Dependencies
------------
**PyHO requires a working installation of** `PyZMQ` **Python package.**
This usually means you have to install ``libzmq1`` and ``python-zmq`` packages. For example in Debian-like systems:

    ``$ sudo apt-get install libzm1 python-zmq``

PyHO also uses following Python packages:

* ``Cython``
* ``pyximport``
* ``PyEvolve``

All these libraries are included in the ``libs`` directory for convenience.
When present in the standard Python path, system installed versions take
precedence.

Quick start
-----------
Running ``launch_pyopt_min.sh`` script from the main directory can quickly
tell whether the optimizer is running at all. This script uses input data from
``assets`` directory.

You can start the optimizer directly by launching ``optimizer_ga.py`` Python
module from the ``pyopt_min`` directory. Running it with ``--help`` option
will show all available options.

There is a 32-bit Linux precompiled version of `FMDT` GUI application packaged
in a ``GUI/linux_x86`` directory. Run it by calling ``Kanal_v1`` shell script.

======================================
PyHO - Hybrid coil optimizer in Python
======================================

This project at its current state is a Python port (and a wrapper) of the 
original C++ optimizer module from `FMDT` (Flow Meter Design Tool) project.

Dependencies
------------
PyHO uses following Python packages:

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


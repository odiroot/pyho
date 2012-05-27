"Setup for modified levmar library."
import os
from distutils.core import setup
from distutils.extension import Extension


THIS_DIR = os.path.dirname(os.path.abspath(__file__))
PY_DIR = os.path.join(THIS_DIR, "levmar_mod")
C_DIR = os.path.join(PY_DIR, "levmar-2.5")
CORE_SOURCE = os.path.join(PY_DIR, "_core.c")


def get_library_sources():
    src = ('lm.c', 'Axb.c', 'misc.c', 'lmlec.c', 'lmbc.c', 'lmblec.c',
           'lmbleic.c',)
    src = [os.path.join(C_DIR, f) for f in src]
    return src


setup(
    ext_modules=[Extension(
        name="levmar_mod._core",
        sources=[CORE_SOURCE] + get_library_sources(),
        include_dirs=[PY_DIR, C_DIR],
        depends=[CORE_SOURCE] + get_library_sources(),
        libraries=['lapack', 'blas'],
    )]
)

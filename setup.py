import sys
from setuptools import setup, find_packages
from os.path import join, dirname, abspath
import pyho


def __setup_levmar():
    this_dir = dirname(abspath(__file__))
    libs_dir = join(this_dir, "pyho", "libs")
    # Temporarily add levmar_mod's setup file to path.
    sys.path.append(libs_dir)
    from setup_levmar import main
    # Run levmar_mod package setup.
    main()
    # Remove custom library path from system path.
    sys.path.remove(libs_dir)


# Modified levmar dependency handling.
try:
    import levmar_mod
    dir(levmar_mod)
except ImportError:
    __setup_levmar()


setup(
    name="pyho",
    author="Michal Odnous",
    author_email="odnousm@iem.pw.edu.pl",
    description="Hybrid optimization system.",
    version=pyho.__version__,
    url="https://bitbucket.org/odiroot/pyho",
    license="BSD (optimizer code only)",

    dependency_links=[
        "http://pyevolve.sourceforge.net/distribution/0_6rc1/"
            "Pyevolve-0.6rc1.tar.gz",
    ],

    install_requires=[
        "pyzmq>=2.1.9",
        "numpy",
        "PyEvolve>=0.6rc1",
    ],

    packages=find_packages(),

    long_description=open(join(dirname(__file__), "README.txt")).read(),
    entry_points={
        "console_scripts": ["run_pyho = pyho.run:main"]
    },
)

from setuptools import setup, find_packages
from os.path import join, dirname
import pyho


setup(
    name="pyho",
    author="Michal Odnous",
    author_email="odnousm@iem.pw.edu.pl",
    description="Hybrid optimization system.",
    version=pyho.__version__,

    dependency_links=[
        "http://pyevolve.sourceforge.net/distribution/0_6rc1/Pyevolve-0.6rc1.tar.gz"
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

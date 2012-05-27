from setuptools import setup, find_packages
from os.path import join, dirname
import pyho


setup(
    name="pyho",
    version=pyho.__version__,
    packages=find_packages(),
    long_description=open(join(dirname(__file__), "README.txt")).read(),
    entry_points={
        "console_scripts": ["run_pyho = pyho.run:main"]
    }
)

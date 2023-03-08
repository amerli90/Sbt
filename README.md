# Sbt
Sbt is a software designed to analyse data taken in a "test beam" setup, were charged-particle tracks are reconstructed using several planes of tracking detectors, typically silicon detectors.

## Requirements

This package is intended to be compiled as a shared library, to be used by an external executable.
Requirements:

- cmake 3.11 (or above): https://cmake.org/
- yaml-cpp 0.60 (or above): https://github.com/jbeder/yaml-cpp
- ROOT 6.14/06 (or above): https://root.cern.ch/

Note: yaml-cpp must be compiled as a shared library. This is achieved by adding the option `-DBUILD_SHARED_LIBS=ON` when running `cmake` for yaml-cpp.

Sbt has been tested with the following OS, compilers and ROOT:

- CentOS 7 (CERN lxplus.cern.ch) with gcc 8.2.1 and ROOT 6.16/00;
- macOS 10.14 Mojave with clang 10 (default version shipped with macOS) and ROOT 6.14/06 [ONLY OLD VERSIONS, not updated];
- Ubuntu 18.10 with gcc 8.2.0 and ROOT 6.16/00

Other versions of Linux/macOS and ROOT 6 may be compatible, but have not been tested. ROOT 6.10/08 is known to be incompatible.

The current version of this package does NOT support gcc 4. The reason is a lack of full support of C++11 features with this version of the compiler. Must use gcc 6 at least, version 8 is recommended.

## Installation

In the following it is assumed that the requirements, as listed in the above section, have been installed properly. In particular, cmake and ROOT should be installed in a location pointed by the `$PATH` environment variable.

1. Create a build directory: `mkdir build; cd build`
2. Configure: `cmake ../Sbt_src -DCMAKE_INSTALL_PREFIX=../inst -DYAML=/path/to/yaml-cpp`, where `/path/to/yaml-cpp` is the location where yaml-cpp was installed
3. Compile and install: `make install`

If you want to easily compile your executables against the Sbt library you should include the Sbt library path (`inst/lib`) in your `$LD_LIBRARY_PATH` environment variable.

## To Do List

### Simulation

1. Simulate partial crystal channeling
2. Simulate large-angle scatterings when traversing matter
3. Simulate crystal torsion
4. Fix the simulation of the detector response: currently the calculation of the strips hit by the particle does not properly take into account the angle at which the particle enters and exits the detector

# PEPM19-poster-cpp

## Automatic Build with TravisCI
[![Build Status](https://travis-ci.org/tdauth/cpp-futures-promises.svg?branch=master)](https://travis-ci.org/tdauth/cpp-futures-promises)

## Manual Build
To compile the project run the Bash script [buildrelease.sh](./buildrelease.sh) on Linux.

The dependencies will be downloaded and compiled automatically.
Therefore, you need Internet access when building for the first time.

## Dependencies
The project requires the GCC with C++17 support and CMake to be built.

The project requires the following libraries:
* [Boost](http://www.boost.org/)
* [Folly](https://github.com/facebook/folly)

It will download and compile these libraries automatically when being compiled.
The versions of the libraries are specified in the [CMakeLists.txt](./CMakeLists.txt) file in the top level directory of the project.

Furthermore, the project requires the following packages on Fedora 27:
* cmake
* gcc-c++
* libatomic
* bash
* which
* rpm-build
* valgrind
* lcov
* cppcheck
* cppcheck-htmlreport

Folly dependencies on Fedora 27:
* glog-devel
* gflags-devel
* autoconf
* automake
* autoconf
* automake
* libtool
* zlib-devel
* lzma-devel
* snappy-devel
* double-conversion-devel
* openssl-devel
* libevent-devel

These dependencies can be installed with the script [install_fedora_dependencies.sh](./install_fedora_dependencies.sh).
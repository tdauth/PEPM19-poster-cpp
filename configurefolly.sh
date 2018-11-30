#!/bin/bash
CC="/usr/bin/gcc"
CXX="/usr/bin/g++"

echo "Using Boost root for Folly: $3"

cmake "$1" -DBUILD_SHARED_LIBS=OFF -DBOOST_ROOT="$3" -DBOOST_INCLUDEDIR="$3/include" -DBOOST_LIBRARYDIR="$3/lib" -DBoost_NO_SYSTEM_PATHS="TRUE" -DBoost_NO_BOOST_CMAKE="TRUE" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX="$2" -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS"
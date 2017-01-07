#!/usr/bin/env bash

export PATH="/usr/local/bin:$PATH"
export PATH="/usr/lib/llvm-3.8/bin:$PATH"

export CMAKE_C_COMPILER=$(which gcc)
export CC=$(which gcc)

rm -Rf build
mkdir build
cd build
export PATH=$PATH:$PWD
cmake -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_BUILD_TYPE=Debug ..
make -j 4
make axon_coverage

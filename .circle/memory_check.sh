#!/usr/bin/env bash

export PATH="/usr/local/bin:$PATH"
export PATH="/usr/lib/llvm-3.8/bin:$PATH"

export CMAKE_C_COMPILER=$(which clang-3.8)
export CC=$(which clang-3.8)

echo "clang path: $(which clang)"

rm -Rf build
mkdir build
cd build
cmake -DCMAKE_C_COMPILER=$(which clang-3.8) -DCMAKE_CXX_COMPILER=$(which clang++-3.8) -DCMAKE_BUILD_TYPE=Debug ..
make -j 4
./koro_test

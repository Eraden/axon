#!/usr/bin/env bash

echo "$(lsb_release -a)"

sudo apt-get --yes update &> /dev/null
sudo apt-get --yes install language-pack-pl
sudo apt-get --yes install check
sudo apt-get --yes remove clang
sudo apt-get --yes remove cmake
sudo apt-get --yes install clang-3.8
sudo apt-get --yes install libyajl-dev
sudo apt-get --yes install libyajl2
sudo apt-get --yes install libpq-dev
sudo apt-get --yes install libpq-dev postgresql-server-dev-all
sudo apt-get --yes install lcov
sudo apt-get --yes install libyaml-dev

export CPP=$(which cpp)
export CC=$(which cc)

root="$PWD"
echo "$(cmake --version)"

if [[ ! -f ${root}/build ]];
then
    mkdir -p ${root}/build
fi

if [[ ! -f ${root}/vendor ]];
then
    mkdir -p ${root}/vendor
fi

if [[ ! -f ${root}/tmp ]];
then
    mkdir -p ${root}/tmp
fi

if [[ "$(cmake --version | grep -E '[0-9]+.[0-9]+.[0-9]+' | sed 's/[a-z ]//gi')" == "3.6.1" ]]
then
  echo "Valid cmake"
else
  if [[ ! -d ${root}/vendor/cmake-3.6.1 ]]
  then
    cd vendor
    wget https://cmake.org/files/v3.6/cmake-3.6.1.tar.gz
    tar -xvf cmake-3.6.1.tar.gz &> /dev/null
    cd ${root}/vendor/cmake-3.6.1
    echo "Configure cmake..."
    ./configure &> /dev/null
    echo "Building cmake..."
    make -j 4 &> /dev/null
    cd ${root}
  fi
  cd ${root}/vendor/cmake-3.6.1
  echo "Installing cmake..."
  sudo make install &> /dev/null
  echo "  done"
  cd ${root}
fi

export PATH="/usr/local/bin:/usr/bin:$PATH"
export PATH="/usr/lib/llvm-3.8/bin:$PATH"

version="$(cmake --version | grep -E '[0-9]+.[0-9]+.[0-9]+' | sed 's/[a-z ]//gi')"
echo "cmake version: '$version'"

if [[ "$version" == "3.6.1" ]]
then
  echo "CMake is valid"
else
  echo "Failed to install cmake"
  exit 1
fi

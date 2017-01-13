#!/usr/bin/env bash

echo "Working directory: $PWD"

export PATH=$PWD/..:$PATH
export C_INCLUDE_PATH=$PWD/../includes:${C_INCLUDE_PATH}
export LD_LIBRARY_PATH=$PWD/../lib:${LD_LIBRARY_PATH}
export KORE_ENV=test

rm -Rf db
rm -Rf ./conf/database.yml
rm -Rf ./conf/triggers.yml

psql postgres -c 'DROP DATABASE cmake_axon_test' &> /dev/null

echo "test:
  name: cmake_axon_test
  host: localhost
  port: 5432
" >> ./conf/database.yml

echo "
libs: axonutils axonconfig
flags: -L../lib -I../includes -Wall -g -fsanitize=address -fno-omit-frame-pointer
" >> ./conf/triggers.yml

axon db init
axon db create
axon db drop

axon-migrator create
axon-migrator drop
axon-migrator migrate
axon-migrator setup
axon-migrator seed

axon-creator new table accounts id login pass timestamps
axon-creator new table posts id author_id:accounts\(id\) timestamps
axon-compiler triggers
axon-migrator migrate

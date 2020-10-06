#! /usr/bin/env bash

git clone https://github.com/Garcia6l20/cppcoro.git
cd cppcoro ... || exit
mkdir build
cd build ... || exit
cmake .. -DCMAKE_BUILD_TYPE
make -j4

cp libcppcoro.a /usr/local/lib
cp -r ../include/* /usr/local/include

cd ~ ... || exit

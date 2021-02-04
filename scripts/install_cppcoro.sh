#! /usr/bin/env bash

cd /tmp || exit 1
git clone https://github.com/Garcia6l20/cppcoro.git
cd cppcoro || exit 1
mkdir build
cd build || exit 1
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

cp ./lib/libcppcoro.a /usr/local/lib
cp -r ../include/* /usr/local/include

cd ~ || exit 1

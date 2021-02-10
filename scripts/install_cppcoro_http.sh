#! /usr/bin/env bash

cd /tmp || exit 1
git clone https://github.com/Garcia6l20/cppcoro-http.git -b feature/router-v2
cd cppcoro-http || exit 1
mkdir build
cd build
cmake -CMAKE_BUILD_TYPE=Release ..
#make install -j8
#rm -rf /tmp/cppcoro-http

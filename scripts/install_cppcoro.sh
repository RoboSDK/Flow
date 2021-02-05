#! /usr/bin/env bash

cd /tmp || exit 1
git clone https://github.com/Garcia6l20/cppcoro.git
mkdir cppcoro/build && cd cppcoro/build
cmake -DCMAKE_BUILD_TYPE=Release -DCPPCORO_USE_IO_RING=ON ..
make -j && make install

#! /usr/bin/env bash

git clone https://github.com/axboe/liburing.git
cd liburing ... || exit
./configure
make -j4
make install

#! /usr/bin/env bash

cd /tmp || exit
git clone https://github.com/axboe/liburing.git
cd liburing ... || exit
./configure
make -j4
make install

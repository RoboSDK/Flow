#! /usr/bin/env bash

cd /tmp || exit 1
git clone https://github.com/axboe/liburing.git
cd liburing || exit 1
./configure
make -j4
make install
rm -rf /tmp/liburing

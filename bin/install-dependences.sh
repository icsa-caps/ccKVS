#!/usr/bin/env bash
sudo apt-get --yes --force-yes install \
   git make gcc numactl libnuma-dev \
   libmemcached-dev zlib1g-dev memcached \
   libmemcached-dev libmemcached-tools libpapi-dev 

wget https://github.com/ivmai/libatomic_ops/releases/download/v7.4.6/libatomic_ops-7.4.6.tar.gz

tar xzvf libatomic_ops-7.4.6.tar.gz

cd libatomic_ops-7.4.6

./configure

make

sudo make install

cd ..

rm -rf libatomic_ops-7.4.6*

#!/bin/sh
cd ..
sudo make clean
chmod 777 -R *
sudo apt-get update
sudo apt-get -y upgrade
cd `pwd`/depends
sudo make -j2 HOST=aarch64-linux-gnu
cd ..
sudo ./autogen.sh
mkdir `pwd`/db4
wget -c 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
tar -xzvf db-4.8.30.NC.tar.gz
cd `pwd`/db-4.8.30.NC/build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=`pwd`/db4
sudo make install
cd ../../
sudo ./autogen.sh
./configure LDFLAGS="-L`pwd`/db4/lib/" CPPFLAGS="-I`pwd`/db4/include/" --prefix=`pwd`/depends/aarch64-linux-gnu --enable-glibc-back-compat --enable-reduce-exports LDFLAGS=-static-libstdc++
sudo make -j2
echo "Remember to strip the daemon, cli, and tx files!"
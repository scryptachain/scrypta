#!/bin/sh
cd ..
sudo make clean
chmod 777 -R *
sudo apt-get update
sudo apt-get -y upgrade
PATH=$(echo "$PATH" | sed -e 's/:\/mnt.*//g')
cd `pwd`/depends
mkdir SDKs
cd SDKs
wget -c https://github.com/phracker/MacOSX-SDKs/releases/download/MacOSX10.11.sdk/MacOSX10.11.sdk.tar.xz
tar -xf MacOSX10.11.sdk.tar.xz
cd ..
sudo make -j2 HOST=x86_64-apple-darwin11
cd ..
sudo ./autogen.sh
mkdir db4
wget -c 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
tar -xzvf db-4.8.30.NC.tar.gz
cd `pwd`/db-4.8.30.NC/build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=`pwd`/db4
sudo make install
cd ../../
sudo ./autogen.sh
./configure --prefix=`pwd`/depends/x86_64-apple-darwin11
sudo make -j2
sudo make deploy
echo "No strip required for this file! DMG file is located in the same folder as this script."
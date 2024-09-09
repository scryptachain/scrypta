#!/bin/bash

# Set the root directory for LYRA
LYRA_ROOT=$(pwd)

# Create a directory for Berkeley DB
BDB_PREFIX="${LYRA_ROOT}/db4"
mkdir -p $BDB_PREFIX

# Update package list and install dependencies
sudo apt-get update
sudo apt-get install -y autoconf automake libtool libboost-all-dev libminiupnpc-dev libssl-dev pkg-config protobuf-compiler qt5-default wget

# Fetch and verify Berkeley DB source
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c

# Extract and build Berkeley DB
tar -xzvf db-4.8.30.NC.tar.gz
cd db-4.8.30.NC/build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
make install

# Return to the LYRA root directory
cd $LYRA_ROOT

# Run autogen script
./autogen.sh

# Configure the build with Berkeley DB
./configure LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"

# Build LYRA
make

# Optionally install LYRA
make install
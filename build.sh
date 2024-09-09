#!/bin/bash

# Ensure the correct version of libtool is installed
sudo apt-get update
sudo apt-get install -y libtool

# Recreate aclocal.m4 with macros from the correct version of libtool
aclocal

# Run autoconf to regenerate the configure script
autoconf

# Run autogen script
./autogen.sh

# Ensure configure script is generated
if [ ! -f ./configure ]; then
    echo "Error: configure script not found. Ensure autogen.sh ran successfully."
    exit 1
fi

# Continue with the rest of the build process
./configure LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"
make
make install
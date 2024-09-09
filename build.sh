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
cd db-4.8.30.NC

# Apply fix for __atomic_compare_exchange conflict
sed -i '/__atomic_compare_exchange/d' dbinc/atomic.h

# Apply fix for syntax errors in atomic.h
sed -i '147s/^/static inline int __atomic_inc(db_atomic_t *p) { return __sync_add_and_fetch(p, 1); }\n/' dbinc/atomic.h
sed -i '178s/^/static inline int __atomic_compare_exchange(db_atomic_t *p, atomic_value_t oldval, atomic_value_t newval) { return __sync_bool_compare_and_swap(p, oldval, newval); }\n/' dbinc/atomic.h

cd build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
make install

# Return to the LYRA root directory
cd $LYRA_ROOT

# Update configure.ac to replace obsolete AC_HELP_STRING
sed -i 's/AC_HELP_STRING/AS_HELP_STRING/g' configure.ac

# Ensure ltmain.sh is present
if [ ! -f ./ltmain.sh ]; then
    libtoolize
fi

# Run autoupdate to update configure.ac
autoupdate

# Run autogen script
./autogen.sh

# Ensure configure script is generated
if [ ! -f ./configure ]; then
    echo "Error: configure script not found. Ensure autogen.sh ran successfully."
    exit 1
fi

# Configure the build with Berkeley DB
./configure LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"

# Build LYRA
make

# Optionally install LYRA
make install

# Create install script from install.sh and make it executable
cat install.sh > install
chmod a+x install
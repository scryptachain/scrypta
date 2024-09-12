#!/bin/bash

# Install bzip2 and Python development packages
sudo apt-get install -y libbz2-dev python-dev

# Download the boost tarball
curl -L -o boost_1_59_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.gz/download

# Verify the file type
file_type=$(file --mime-type -b boost_1_59_0.tar.gz)
if [ "$file_type" != "application/gzip" ]; then
  echo "Error: The downloaded file is not a gzip file."
  exit 1
fi

# Extract the tarball to /usr/local
sudo tar xzvf boost_1_59_0.tar.gz -C /usr/local

# Proceed with the installation
cd /usr/local/boost_1_59_0
chmod 777 bootstrap.sh
chmod 777 b2
./bootstrap.sh
./b2 -q
sudo ./b2 install -q

# Clean up
cd ..
sudo rm -rf boost_1_59_0.tar.gz
# This script installs the right version of boost
#/bin/bash
curl http://sourceforge.net/projects/boost/files/boost/1.59.0/boost_1_59_0.tar.gz/download -o boost_1_59_0.tar.gz
tar xzvf boost_1_59_0.tar.gz
cd boost_1_59_0
./bootstrap.sh
./b2
sudo ./b2 install
cd ..
cd ..
rm -rf boost_1_59_0
rm -rf boost_1_59_0.tar.gz
# End of snippet
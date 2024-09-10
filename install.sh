#/bin/bash

# Check if user is root
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi
sudo apt-get install -y software-properties-common python-software-properties
sudo apt-get update
sudo apt-get install -y openssl1.0
sudo apt-get install -y build-essential autoconf automake libboost1.59-all-dev libleveldb-dev libgmp-dev libgmp3-dev libssl-dev libcurl4-openssl-dev libcrypto++-dev libqrencode-dev libminiupnpc-dev autogen libtool git libevent-dev libprotobuf-dev
sudo apt-get install -y curl g++ git-core pkg-config libtool faketime bsdmainutils mingw-w64 g++-mingw-w64 nsis zip ca-certificates python
sudo apt-get install -y libzmq3-dev
sudo apt-get install -y libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler
sudo apt-get install -y libqrencode-dev 
sudo apt-get install -y libssl1.0-dev
# Check if scrypta directory exists
if [ ! -d "scrypta" ]; then
    echo "Cloning LYRA repository..."
    git clone https://github.com/Romoli-Gabriele/scrypta
    cd scrypta
else
    echo "LYRA directory already exists. Skipping clone."
    cd scrypta
    git pull
fi
./autogen.sh
./configure
sudo make
mkdir /root/.lyra
touch /root/.lyra/lyra.conf
echo "rpcuser=lyrarpc\nrpcpassword=SomeSuperStrongPassword\nrpcallowip=127.0.0.1\nlisten=1\nserver=1\ndaemon=1\nindex=1\ntxindex=1\nlogtimestamps=1\nstaking=0\naddnode=142.93.224.34\ndatacarriersize=8000" > /root/.lyra/lyra.conf
cd src
./lyrad &
cd ..
sudo apt update -y
sudo apt upgrade -y
sudo apt install build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git -y
sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler -y

sudo apt-get install software-properties-common -y
sudo add-apt-repository ppa:bitcoin/bitcoin -y
sudo apt-get update -y
sudo apt-get install libdb4.8-dev libdb4.8++-dev -y
sudo apt-get install libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev -y
sudo apt-get install libzmq3-dev -y
sudo apt-get install libminiupnpc-dev -y
sudo apt-get install libgmp3-dev libevent-dev bsdmainutils libboost-all-dev openssl -y
sudo apt-get install libssl1.0-dev -y

# Install libtool
sudo apt-get install libtool -y

# Install Libcurl
sudo apt-get install libcurl4-openssl-dev -y

# Install openssl
apt-get install openssl -y
sudo apt-get install -y autoconf g++ make openssl libssl-dev libcurl4-openssl-dev -y
sudo apt-get install -y libcurl4-openssl-dev pkg-config -y
sudo apt-get install -y libsasl2-dev -y

sudo apt install g++-mingw-w64-x86-64 -y
sudo update-alternatives --config x86_64-w64-mingw32-g++ -y # Set the default mingw32 g++ compiler option to posix. or choose 0 for deb8
PATH=$(echo "$PATH" | sed -e 's/:\/mnt.*//g') # strip out problematic Windows %PATH% imported var
cd depends
sudo make HOST=x86_64-w64-mingw32 -j4

cd ..
sudo ./autogen.sh # not required when building from tarball
sudo CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site ./configure --prefix=/
sudo make

echo "Remember to strip the QT file!"
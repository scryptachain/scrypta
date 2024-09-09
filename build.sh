#!/bin/bash

# Check if running as root
if [ "$(id -u)" != "0" ]; then
   echo "This script must be run as root" 1>&2
   exit 1
fi

# Define the repository URL
REPO_URL="https://github.com/Romoli-Gabriele/scrypta"
LYRA_DIR="scrypta"

# Update package list and install basic build tools
echo "Updating package list and installing build tools..."
sudo apt-get update
sudo apt-get install -y build-essential libtool autotools-dev automake pkg-config bsdmainutils bison python3 git

# Clone LYRA repository
if [ ! -d "$LYRA_DIR" ]; then
    echo "Cloning LYRA repository..."
    git clone "$REPO_URL"
else
    echo "LYRA directory already exists. Skipping clone."
fi

cd "$LYRA_DIR" || { echo "Failed to enter LYRA directory"; exit 1; }

# Install required dependencies
echo "Installing required dependencies..."
sudo apt-get install -y libevent-dev libboost-dev libsqlite3-dev libzmq3-dev

# Install optional dependencies (miniupnpc, NAT-PMP, USDT tracing)
echo "Installing optional dependencies..."
sudo apt-get install -y libminiupnpc-dev libnatpmp-dev systemtap-sdt-dev

# Install GMP development package
echo "Installing GMP development package..."
sudo apt-get install -y libgmp-dev

# Install Berkeley DB for wallet support (optional)
echo "Do you want to install Berkeley DB 4.8 for wallet support? (y/n)"
read -r install_bdb
if [ "$install_bdb" == "y" ]; then
    echo "Building and installing Berkeley DB 4.8..."
    BDB_PREFIX="$(pwd)/db4"
    mkdir -p $BDB_PREFIX
    wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
    echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c
    tar -xzvf db-4.8.30.NC.tar.gz
    cd db-4.8.30.NC/build_unix/
    ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
    make install
    cd ../..
else
    echo "Skipping Berkeley DB installation."
fi

# Install optional GUI dependencies (Qt 5 and libqrencode for lyra-qt)
echo "Do you want to install GUI dependencies for lyra-qt? (y/n)"
read -r install_gui
if [ "$install_gui" == "y" ]; then
    echo "Installing GUI dependencies..."
    sudo apt-get install -y libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools qtwayland5 libqrencode-dev
else
    echo "Skipping GUI installation."
fi

# Build LYRA
echo "Starting LYRA build..."
./autogen.sh
if [ "$install_gui" == "y" ]; then
    ./configure LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"
else
    ./configure --without-gui LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/"
fi
make -j "$(nproc)"

# Optional: Install LYRA
echo "Do you want to install LYRA globally? (y/n)"
read -r install_lyra
if [ "$install_lyra" == "y" ]; then
    sudo make install
    echo "LYRA installed successfully."
else
    echo "Build completed, LYRA not installed."
fi

echo "LYRA build process finished."
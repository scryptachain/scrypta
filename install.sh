#/bin/bash

# Se non hai berkleydb installato, esegui berkley.sh prima di questo script

# Se non hai boost installato, esegui boost.sh prima di questo script



# Controlla che l'utente sia dentro root
if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi
sudo apt-get update # Aggiorna i pacchetti

# Installa le dipendenze
sudo apt-get install -y software-properties-common python-software-properties
sudo apt-get install -y build-essential autoconf automake libleveldb-dev libgmp-dev libgmp3-dev libcrypto++-dev libqrencode-dev libminiupnpc-dev autogen libtool git libevent-dev libprotobuf-dev
sudo apt-get install -y curl g++ git-core pkg-config libtool faketime bsdmainutils mingw-w64 g++-mingw-w64 nsis zip ca-certificates python
sudo apt-get install -y libzmq3-dev
sudo apt-get install -y libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler
sudo apt-get install -y libqrencode-dev
sudo apt-get install -y sudo apt-get install libssl1.0-dev --no-remove

# Controlla se esiste la cartella scrypta
if [ ! -d "scrypta" ]; then
    # Se non esiste, clona il repository
    echo "Cloning LYRA repository..."
    git clone https://github.com/Romoli-Gabriele/scrypta
    cd scrypta
    chmod 777 ltmain.sh
else
    # Se esiste, aggiorna il repository
    echo "LYRA directory already exists. Skipping clone."
    cd scrypta
    git pull
fi
# Genera i file di configurazione
./autogen.sh
# Configura il makefile con il path per boost
./configure --with-boost=/usr/local

if [ $? -eq 0 ]; then
    # Compila il software solo se la configurazione è andata a buon fine
    sudo make install
    mkdir /root/.lyra
    touch /root/.lyra/lyra.conf
    # Riempi il file di configurazione con i dati necessari
    echo "fill in the lyra.conf file then run: lyrad &"
fi
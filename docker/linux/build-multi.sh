#!/bin/bash

cd multi
mkdir build
docker build -t scrypta:multi .
docker run -d --name multi scrypta:multi
docker cp multi:/opt/scrypta/src/lyrad  ./build/lyrad
docker cp multi:/opt/scrypta/src/lyra-cli  ./build/lyra-cli
docker cp multi:/opt/scrypta/src/qt/lyra-qt  ./build/lyra-qt

rm -rf scrypta-wallet
rm scrypta-wallet.deb
mkdir scrypta-wallet
cd scrypta-wallet

mkdir DEBIAN
cd DEBIAN
touch control
echo "Package: scrypta-wallet
Version: 2.0.1
Section: custom
Priority: optional
Architecture: all
Essential: no
Installed-Size: 1024
Maintainer: Scrypta Foundation
Description: FullNode Wallet for Scrypta Blockchain" >> control

cd ..
mkdir usr
mkdir usr/bin/
cd ..

cp ./build/lyra-cli ./scrypta-wallet/usr/bin/lyra-cli
cp ./build/lyrad ./scrypta-wallet/usr/bin/lyrad
cp ./build/lyra-qt ./scrypta-wallet/usr/bin/lyra-qt

sudo dpkg-deb --build ./scrypta-wallet
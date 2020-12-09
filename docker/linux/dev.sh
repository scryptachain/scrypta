#!/bin/bash

rm -rf dev
mkdir dev
cd dev

docker exec -it -w /opt/scrypta/ multi git pull
docker exec -it -w /opt/scrypta/ multi make clean
docker exec -it -w /opt/scrypta/ multi make -j16

docker cp multi:/opt/scrypta/src/lyrad  ./lyrad
docker cp multi:/opt/scrypta/src/lyra-cli  ./lyra-cli
docker cp multi:/opt/scrypta/src/qt/lyra-qt  ./lyra-qt

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

cp ./lyra-cli ./scrypta-wallet/usr/bin/lyra-cli
cp ./lyrad ./scrypta-wallet/usr/bin/lyrad
cp ./lyra-qt ./scrypta-wallet/usr/bin/lyra-qt

sudo dpkg-deb --build ./scrypta-wallet

sleep 2
./lyra-qt
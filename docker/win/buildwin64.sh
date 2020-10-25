#!/bin/bash

cd win64
mkdir build
docker build -t scrypta:win64 .
docker run -d --name win64 -p 42222:42222 -p 42223:42223 scrypta:win64
docker cp win64:/opt/scrypta/src/lyrad.exe  ./build/lyrad.exe
docker cp win64:/opt/scrypta/src/lyra-cli.exe  ./build/lyra-cli.exe
docker cp win64:/opt/scrypta/src/qt/lyra-qt.exe  ./build/lyra-qt.exe
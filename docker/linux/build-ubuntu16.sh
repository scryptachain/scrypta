#!/bin/bash

cd ubuntu-16
mkdir build
docker build -t scrypta:ubuntu16 .
docker run -d --name ubuntu16 -p 42222:42222 -p 42223:42223 scrypta:ubuntu16
docker cp ubuntu16:/opt/scrypta/src/lyrad  ./build/lyrad
docker cp ubuntu16:/opt/scrypta/src/lyra-cli  ./build/lyra-cli
docker cp ubuntu16:/opt/scrypta/src/qt/lyra-qt  ./build/lyra-qt
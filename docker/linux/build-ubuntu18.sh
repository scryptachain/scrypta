#!/bin/bash

cd ubuntu-18
mkdir build
docker build -t scrypta:ubuntu18 .
docker run -d --name ubuntu18 scrypta:ubuntu18
docker cp ubuntu18:/opt/scrypta/src/lyrad  ./build/lyrad
docker cp ubuntu18:/opt/scrypta/src/lyra-cli  ./build/lyra-cli
docker cp ubuntu18:/opt/scrypta/src/qt/lyra-qt  ./build/lyra-qt
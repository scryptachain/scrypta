#!/bin/bash

cd ubuntu-20
mkdir build
docker build -t scrypta:ubuntu20 .
docker run -d --name ubuntu20 scrypta:ubuntu20
docker cp ubuntu20:/opt/scrypta/src/lyrad  ./build/lyrad
docker cp ubuntu20:/opt/scrypta/src/lyra-cli  ./build/lyra-cli
docker cp ubuntu20:/opt/scrypta/src/qt/lyra-qt  ./build/lyra-qt
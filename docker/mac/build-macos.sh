#!/bin/bash

cd macos
mkdir build
docker build -t scrypta:macos .
docker run -d --name macos -p 42222:42222 -p 42223:42223 scrypta:macos
docker cp macos:/opt/scrypta/src/lyrad  ./build/lyrad
docker cp macos:/opt/scrypta/src/lyra-cli  ./build/lyra-cli
docker cp macos:/opt/scrypta/src/qt/lyra-qt  ./build/lyra-qt
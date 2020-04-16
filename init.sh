#!/bin/bash

datadir=$1

if [ $datadir ]
then

    rpcuser=$(( ((RANDOM<<15)|RANDOM) % 999999999999 ))
    rpcpassword=$(( ((RANDOM<<15)|RANDOM) % 999999999999 ))
    echo "rpcuser="$rpcuser >> $datadir'lyra.conf'
    echo "rpcpassword="$rpcpassword >> $datadir'lyra.conf'
    echo "rpcallowip=172.17.0.0/16" >> $datadir'lyra.conf' 
    echo "rpcbind=0.0.0.0" >> $datadir'lyra.conf'
    echo "rpcallowip=127.0.0.1" >> $datadir'lyra.conf'
    echo "listen=1" >> $datadir'lyra.conf'
    echo "server=1" >> $datadir'lyra.conf'
    echo "daemon=1" >> $datadir'lyra.conf'
    echo "index=1" >> $datadir'lyra.conf'
    echo "txindex=1" >> $datadir'lyra.conf'
    echo "logtimestamps=1" >> $datadir'lyra.conf'
    echo "staking=0" >> $datadir'lyra.conf'
    echo "addnode=142.93.224.34" >> $datadir'lyra.conf'
    echo "datacarriersize=8000" >> $datadir'lyra.conf'
    echo "maxconnections=64" >> $datadir'lyra.conf'
else
    echo 'PLEASE GIVE THE WORKING FOLDER'
fi
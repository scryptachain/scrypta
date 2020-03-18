FROM ubuntu:16.04 

RUN apt-get update

RUN apt-get install -y software-properties-common 

RUN add-apt-repository -y ppa:bitcoin/bitcoin

RUN apt-get update

RUN apt-get install -y libdb4.8 libdb4.8++ wget libboost1.58-all libminiupnpc10

WORKDIR /root/.lyra

RUN touch lyra.conf

RUN echo "rpcuser=lyrarpc\nrpcpassword=G1GbXYVU2UcwxAtHDr1L5eGqc2dQhZLSLKno4P9CiCic\nrpcallowip=172.17.0.0/16\nrpcbind=0.0.0.0\nrpcallowip=127.0.0.1\nlisten=1\nserver=1\ndaemon=1\nindex=1\ntxindex=1\nlogtimestamps=1\nstaking=0\naddnode=142.93.224.34\ndatacarriersize=8000" > lyra.conf

WORKDIR /opt

RUN wget https://github.com/scryptachain/scrypta/releases/download/v1.0.0/lyra-1.0.0-x86_64-linux-gnu.tar.gz

RUN tar -xvzf lyra-1.0.0-x86_64-linux-gnu.tar.gz

RUN mv lyra-1.0.0-x86_64-linux-gnu /opt/scrypta 

WORKDIR /opt/scrypta

EXPOSE 42222 42223

CMD tail -f /dev/null
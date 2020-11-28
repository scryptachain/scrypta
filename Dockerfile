FROM ubuntu:16.04 

RUN apt-get update

RUN apt-get install -y software-properties-common 

RUN add-apt-repository -y ppa:bitcoin/bitcoin

RUN apt-get update

RUN apt-get install -y libdb4.8 libdb4.8++ wget libboost1.58-all libminiupnpc10

WORKDIR /root/.lyra

RUN wget https://raw.githubusercontent.com/scryptachain/scrypta/master/init.sh

RUN chmod 777 init.sh

RUN ./init.sh ./

WORKDIR /opt

RUN wget https://github.com/scryptachain/scrypta/releases/download/v1.0.0/lyra-1.0.0-x86_64-linux-gnu.tar.gz

RUN tar -xvzf lyra-1.0.0-x86_64-linux-gnu.tar.gz

RUN mv lyra-1.0.0-x86_64-linux-gnu /opt/scrypta 

WORKDIR /opt/scrypta

EXPOSE 42222 42223

CMD tail -f /dev/null
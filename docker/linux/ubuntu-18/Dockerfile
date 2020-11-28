FROM ubuntu:18.04 
RUN apt-get update
RUN apt-get install -y software-properties-common 
RUN add-apt-repository -y ppa:bitcoin/bitcoin
RUN apt-get update
RUN apt install -y build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git
RUN apt-get install -y libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler
RUN apt-get install -y software-properties-common
RUN apt-get install -y libdb4.8-dev libdb4.8++-dev wget libminiupnpc10
RUN apt-get install -y libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev
RUN apt-get install -y libzmq3-dev
RUN apt-get install -y libminiupnpc-dev
RUN apt-get install -y libtool
RUN apt-get install -y libcurl4-openssl-dev
RUN apt-get install -y libssl1.0-dev

# CLONING REPO
WORKDIR /opt
RUN git clone https://github.com/scryptachain/scrypta

# BUILDING LINUX
WORKDIR /opt/scrypta
RUN ./autogen.sh
RUN ./configure
RUN make

EXPOSE 42222 42223
CMD tail -f /dev/null
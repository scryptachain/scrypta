FROM ubuntu:20.04 
RUN apt-get update
RUN apt-get install -y software-properties-common 
RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y keyboard-configuration
RUN apt install -y build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git
RUN apt-get install -y libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler
RUN apt-get install -y libdb4.8 libdb4.8++
RUN apt-get install -y libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev
RUN apt-get install -y libzmq3-dev
RUN apt-get install -y libminiupnpc-dev
RUN apt-get install -y libtool
RUN apt-get install -y libcurl4-openssl-dev
RUN apt-get install libgmp3-dev libevent-dev bsdmainutils libboost-all-dev openssl -y

# CLONING REPO
WORKDIR /opt
RUN git clone https://github.com/scryptachain/scrypta

# BUILDING LINUX
WORKDIR /opt/scrypta/depends
RUN make
WORKDIR /opt/scrypta
RUN ./autogen.sh 
RUN ./configure --prefix=$(pwd)/depends/x86_64-pc-linux-gnu
RUN make

EXPOSE 42222 42223
CMD tail -f /dev/null
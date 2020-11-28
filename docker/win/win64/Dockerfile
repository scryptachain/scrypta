FROM ubuntu:18.04 
RUN apt update -y
RUN apt upgrade -y
RUN apt install build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git -y
RUN apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler -y
RUN apt-get install software-properties-common -y
RUN add-apt-repository ppa:bitcoin/bitcoin -y
RUN apt-get update -y
RUN apt-get install libdb4.8-dev libdb4.8++-dev -y
RUN apt-get install wget libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev -y
RUN apt-get install libzmq3-dev -y
RUN apt-get install libminiupnpc-dev -y
RUN apt-get install libgmp3-dev libevent-dev bsdmainutils libboost-all-dev openssl -y
RUN apt-get install libssl1.0-dev -y
RUN apt-get install libcurl4-openssl-dev -y
RUN apt-get install -y autoconf g++ make openssl libssl-dev libcurl4-openssl-dev -y
RUN apt-get install -y libcurl4-openssl-dev pkg-config -y
RUN apt-get install -y libsasl2-dev -y
RUN apt install g++-mingw-w64-x86-64 -y
RUN PATH=$(echo "$PATH" | sed -e 's/:\/mnt.*//g')

# CLONING REPO
WORKDIR /opt
RUN git clone https://github.com/scryptachain/scrypta

# BUILDING WINDOWS
WORKDIR /opt/scrypta
WORKDIR /opt/scrypta/depends
RUN make HOST=x86_64-w64-mingw32
WORKDIR /opt/scrypta
RUN ./autogen.sh
RUN ./configure -prefix=$(pwd)/depends/x86_64-w64-mingw32 
RUN make
WORKDIR /opt/scrypta/src/qt
RUN strip lyra-qt

EXPOSE 42222 42223
CMD tail -f /dev/null
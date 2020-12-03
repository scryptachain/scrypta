FROM ubuntu:18.04 
RUN apt-get -qq update
RUN apt-get install -y software-properties-common 
RUN add-apt-repository -y ppa:bitcoin/bitcoin
RUN apt-get -qq update
RUN apt-get install libdb4.8-dev libdb4.8++-dev -y
RUN apt-get install -y libssl1.0-dev
RUN apt-get -y install build-essential autoconf automake libboost-all-dev libleveldb-dev libgmp-dev libgmp3-dev libcrypto++-dev libqrencode-dev libminiupnpc-dev autogen libtool git libevent-dev libprotobuf-dev
RUN apt-get install -y curl g++ git-core pkg-config libtool faketime bsdmainutils nsis zip ca-certificates
RUN apt-get install -y libzmq3-dev
RUN apt-get install -y libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler
RUN apt-get install -y libqrencode-dev
RUN apt-get install -y libcurl4-gnutls-dev

# CLONING REPO
WORKDIR /opt
RUN git clone https://github.com/turinglabsorg/scrypta

# BUILDING LINUX
WORKDIR /opt/scrypta/depends
RUN make HOST=x86_64-linux-gnu -j16
WORKDIR /opt/scrypta
RUN ./autogen.sh; exit 0
RUN libtoolize --automake --copy --debug --force
RUN autoreconf -i
RUN ./autogen.sh
RUN ./configure --enable-glibc-back-compat --prefix=$(pwd)/depends/x86_64-linux-gnu LDFLAGS="-static-libstdc++"
RUN make -j16
WORKDIR /opt/scrypta/src
RUN strip lyrad lyra-cli
WORKDIR /opt/scrypta/src/qt
RUN strip lyra-qt

EXPOSE 42222 42223
CMD tail -f /dev/null
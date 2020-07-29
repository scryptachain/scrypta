cd ..
./autogen.sh # not required when building from tarball
CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site ./configure --prefix=/
make
cd src
cd qt
strip bamboo-qt.exe

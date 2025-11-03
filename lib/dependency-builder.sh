#!/usr/bin/bash

#printf "Current working dir should be inside lib, confirm it: \033[0;31m$(pwd)\n\033[0mIf it's not, use ctrl+c to stop script:\n"
#read -p "Enter to continue or ctrl+c to stop:    "			#npm ignores this

INSTALL_PREFIX="$(pwd)"

[ ! -e sources/ ] || rm -rf sources/
[ ! -e include/ ] || rm -rf include/
[ ! -e libtkrzw.a ] || rm -f libtkrzw.a


mkdir sources
cd sources
cp $INSTALL_PREFIX/tkrzw-1.0.32.tar.gz ./
tar xvf tkrzw-1.0.32.tar.gz
rm -f tkrzw-1.0.32.tar.gz

#clear									#this is bad

if [[ "$EUID" -ne 0 ]]; then
    printf "\033[0;31mYou are not root user. We need to install dependencies. Please run this with root user or install them manually.\nDependencies:\n\t- cmake\n\t- ninja-build\n\t- build-essential\n\t- liblz4-dev\n\033[0m" >&2
else
    apt install -y cmake ninja-build build-essential liblz4-dev
fi

printf "\033[0;31mWe are going to build Tkrzw library, it can take several minutes...\033[0m\n"
#read -p "Enter to start building or ctrl+c to stop:    "		#npm ignores this

cd tkrzw-1.0.32
./configure --enable-opt-native --enable-most-features --enable-lz4 --disable-zlib --disable-shared --enable-static --prefix=$INSTALL_PREFIX
make
make install

cd $INSTALL_PREFIX
mv lib/libtkrzw.a ./
rm -rf sources
rm -rf lib
#rm -rf bin

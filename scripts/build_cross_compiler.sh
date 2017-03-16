#!/bin/bash

TARGET=x86_64-elf
PREFIX=$(pwd)/compiler/$TARGET
PATH=$PATH:$PREFIX/bin
CFLAGS=" -g -O2"
# if we're on a mac, try our damndest to use real GCC not clang
if [ "$(uname)" == "Darwin" ]; then
	export CC=gcc-6
	export CXX=g++-6
fi

echo 'Nuking ./compiler/build and ./compiler/arm and recreating directories'
rm -rf compiler/build
rm -rf compiler/$TARGET
mkdir -p compiler/build
mkdir -p compiler/$TARGET

if [[ ! -d compiler/src ]]; then
	mkdir -p compiler/src
	cd compiler/src

	echo 'Downloading sources'
	echo $(pwd)
	wget http://ftp.gnu.org/gnu/binutils/binutils-2.26.tar.gz
	wget http://ftp.gnu.org/gnu/gcc/gcc-6.1.0/gcc-6.1.0.tar.gz
	wget http://ftp.gnu.org/gnu/gmp/gmp-6.1.0.tar.xz
	wget http://ftp.gnu.org/gnu/mpfr/mpfr-3.1.4.tar.xz
	wget http://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz
	if [ "$(uname)" == "Darwin" ]; then
		wget http://www.bastoul.net/cloog/pages/download/count.php3?url=./cloog-parma-0.16.1.tar.gz
		wget http://isl.gforge.inria.fr/isl-0.17.tar.xz
	fi

	echo 'Extracting archives'
	echo $(pwd)
	tar xzf binutils-2.26.tar.gz
	tar xzf gcc-6.1.0.tar.gz
	tar xzf mpc-1.0.3.tar.gz
	tar xJf gmp-6.1.0.tar.xz
	tar xJf mpfr-3.1.4.tar.xz
	mv gmp-6.1.0 gcc-6.1.0/gmp
	mv mpfr-3.1.4 gcc-6.1.0/mpfr
	mv mpc-1.0.3 gcc-6.1.0/mpc
	if [ "$(uname)" == "Darwin" ]; then
		tar xzf cloog-parma-0.16.1.tar.gz
		tar xJf isl-0.17.tar.xz
		mv cloog-parma-0.16.1 gcc-6.1.0/cloog
		mv isl-0.17 gcc-6.1.0/isl
	fi
	cd -
fi

echo 'Building binutils'
echo $(pwd)
mkdir compiler/build/binutils
cd compiler/build/binutils
../../src/binutils-2.26/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j8
make install
cd -

echo 'Building gcc'
echo $(pwd)
mkdir compiler/build/gcc
cd compiler/build/gcc
../../src/gcc-6.1.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j8
make all-target-libgcc -j8
make install-gcc
make install-target-libgcc
cd -

echo 'Done!'

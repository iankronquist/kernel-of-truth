#!/bin/sh

TARGET=i686-elf
PREFIX=$(pwd)/compiler/$TARGET
PATH=$PATH:$PREFIX/bin
CFLAGS=" -g -O2"
# if we're on a mac, try our damndest to use real GCC not clang
if [ "$(uname)" == "Darwin" ]; then
	export CC=gcc
	export CXX=g++
	alias CC=gcc
	alias CXX=g++
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
	wget http://ftp.gnu.org/gnu/binutils/binutils-2.24.tar.gz 
	wget ftp://ftp.gnu.org/gnu/gcc/gcc-4.9.1/gcc-4.9.1.tar.gz
	wget https://gmplib.org/download/gmp/gmp-6.0.0a.tar.xz
	wget http://www.mpfr.org/mpfr-current/mpfr-3.1.2.tar.xz
	wget ftp://ftp.gnu.org/gnu/mpc/mpc-1.0.2.tar.gz
	if [ "$(uname)" == "Darwin" ]; then
		wget http://www.bastoul.net/cloog/pages/download/count.php3?url=./cloog-parma-0.16.1.tar.gz
		wget http://isl.gforge.inria.fr/isl-0.13.tar.xz
	fi

	echo 'Extracting archives'
	echo $(pwd)
	tar xzf binutils-2.24.tar.gz
	tar xzf gcc-4.9.1.tar.gz
	tar xzf mpc-1.0.2.tar.gz
	tar xJf gmp-6.0.0a.tar.xz
	tar xJf mpfr-3.1.2.tar.xz
	mv gmp-6.0.0a gcc-4.9.1/gmp
	mv mpfr-3.1.2 gcc-4.9.1/mpfr
	mv mpc-1.0.2 gcc-4.9.1/mpc
	if [ "$(uname)" == "Darwin" ]; then
		tar xzf cloog-parma-0.16.1.tar.gz
		tar xJf isl-0.13.tar.xz
		mv cloog-parma-0.16.1 gcc-4.9.1/cloog
		mv isl-0.13 gcc-4.9.1/isl
	fi
	cd -
fi

echo 'Building binutils'
echo $(pwd)
mkdir compiler/build/binutils
cd compiler/build/binutils
../../src/binutils-2.24/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
cd -

echo 'Building gcc'
echo $(pwd)
mkdir compiler/build/gcc
cd compiler/build/gcc
../../src/gcc-4.9.1/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc
cd -

echo 'Done!'

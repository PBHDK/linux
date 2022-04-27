#!/bin/sh

makeFlags="ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu- SHELL=bash KCFLAGS=-g
KCFLAGS=-fsanitize=lkmm-dep-checker"

makeFlagsLLVM="CC=$CC LD=ld.lld AR=$AR NM=$NM STRIP=$STRIP OBJCOPY=$OBJCOPY OBJDUMP=$OBJDUMP READELF=llvm-readelf HOSTCC=$CC_FOR_BUILD HOSTCXX=$CXX_FOR_BUILD HOSTAR=$AR_FOR_BUILD HOSTLD=$LD_FOR_BUILD LLVM_IAS=1 ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu- SHELL=bash KCFLAGS=-g KCFLAGS=-fsanitize=lkmm-dep-checker KCFLAGS=--target=aarch64"

make mrproper 

case $1 in
	"fast")	
		make $makeFlags -j$NIX_BUILD_CORES
		;;
	"precise")
		make $makeFlags -j1
		;;
 	"llvm-fast") 
		make $makeFlagsLLVM defconfig
 		make $makeFlagsLLVM -j$NIX_BUILD_CORES #V=1
 		;;
 	"llvm-precise")
		make $makeFlagsLLVM defconfig
 		make $makeFlagsLLVM -j1
 		;;
	*)
		echo "Invalid command line argument"
		exit -1
		;;
esac

/scratch/paul/src/linux/scripts/clang-tools/gen_compile_commands.py


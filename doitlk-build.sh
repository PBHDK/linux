#!/bin/sh

makeFlagsVanilla="CC=$CC ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu-"

#makeFlags="CC=clang ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu- HOSTCC=$CC_FOR_BUILD HOSTCXX=$CXX_FOR_BUILD HOSTAR=$AR_FOR_BUILD HOSTLD=$LD_FOR_BUILD KCFLAGS=-g KCFLAGS=-fsanitize=lkmm-dep-checker"
makeFlags="CC=clang ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu- HOSTCC=$CC_FOR_BUILD HOSTCXX=$CXX_FOR_BUILD HOSTAR=$AR_FOR_BUILD HOSTLD=$LD_FOR_BUILD KCFLAGS=-g"
#makeFlags="ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu-"

#makeFlagsLLVM="CC=$CC LD=ld.lld AR=$AR NM=$NM STRIP=$STRIP OBJCOPY=$OBJCOPY OBJDUMP=$OBJDUMP READELF=llvm-readelf HOSTCC=$CC_FOR_BUILD HOSTCXX=$CXX_FOR_BUILD HOSTAR=$AR_FOR_BUILD HOSTLD=$LD_FOR_BUILD LLVM_IAS=1 ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu- SHELL=bash KCFLAGS=-g KCFLAGS=-fsanitize=lkmm-dep-checker"
makeFlagsLLVM="LLVM=1 ARCH=arm64 KCFLAGS="-g" KCFLAGS="-fsanitize=lkmm-dep-checker""


case $1 in
	"clean")
		make mrproper 
		;;
	"vanilla-fast")
		make $makeFlagsVanilla defconfig
		make $makeFlagsVanilla -j$NIX_BUILD_CORES 2> build_output.ll
		;;
	"vanilla-precise")
		make $makeFlagsVanilla defconfig
		make $makeFlagsVanilla -j1 2> build_output.ll
		;;
	"fast")	
		make $makeFlags defconfig
		#make $makeFlags -j$NIX_BUILD_CORES SHELL='sh -x' #2> build_output.ll
		make $makeFlags -j1 2> build_output.ll
		;;
	"precise")
		make $makeFlags -j1
		;;
 	"llvm-fast") 
		make $makeFlagsLLVM defconfig
 		make $makeFlagsLLVM -s -j$NIX_BUILD_CORES #V=1
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


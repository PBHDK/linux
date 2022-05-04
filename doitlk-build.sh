#!/bin/sh

makeFlags="CC=$CC ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu-
KCFLAGS="-fsanitize=lkmm-dep-checker""

case $1 in
	"mrproper")
		make mrproper 
		;;
	"clean")
		make clean
		;;
	"defconfig")
		make $makeFlags defconfig
		;;
	"fast")	
		make $makeFlags -j$(nproc) 2> build_output.ll
		;;
	"precise")
		make $makeFlags -j1 2> build_output.ll
		;;
	*)
		echo "Invalid command line argument"
		exit -1
		;;
esac

./scripts/clang-tools/gen_compile_commands.py


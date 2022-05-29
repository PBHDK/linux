#!/bin/sh

ENABLE_DEP_CHEKER="-fsanitize=lkmm-dep-checker"
ENABLE_TESTS="-mllvm -lkmm-enable-tests"
PRINT_MODULES="-mllvm -debug-only=lkmm-print-modules"
CROSS="ARCH=arm64 CROSS_COMPILE=aarch64-unknown-linux-gnu-"

MAKEFLAGS="CC=clang $CROSS KCFLAGS=\"$ENABLE_DEP_CHEKER\""
MAKEFLAGSTEST="CC=clang $CROSS KCFLAGS=\"$ENABLE_DEP_CHEKER $ENABLE_TESTS\"" 
MAKEFLAGS_DEBUG_MODULES="CC=clang $CROSS KCFLAGS=\"$ENABLE_DEP_CHEKER $PRINT_MODULES\""

case $1 in
	"mrproper")
		make mrproper 
		;;
	"clean")
		make clean
		;;
	"defconfig")
		make $MAKEFLAGS defconfig
		./scripts/config --disable CONFIG_DEBUG_INFO_NONE
		./scripts/config --enable CONFIG_DEBUG_INFO
		./scripts/config --enable CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT 
		./scripts/config --disable CONFIG_DEBUG_INFO_REDUCED
		./scripts/config --disable CONFIG_DEBUG_INFO_SPLIT
		./scripts/config --disable CONFIG_DEBUG_INFO_BTF
		./scripts/config --enable CONFIG_PAHOLE_HAS_SPLIT_BTF 
		./scripts/config --enable CONFIG_PAHOLE_HAS_BTF_TAG
		./scripts/config --disable CONFIG_GDB_SCRIPTS
		./scripts/config --disable CONFIG_DEBUG_EFI
		;;
	"fast")	
		make $MAKEFLAGS -j$(nproc) $2 2> build_output.ll
		;;
	"precise")
		make $MAKEFLAGS -j1 $2 2> build_output.ll
		;;
	"tests")
		make $MAKEFLAGSTEST -j1 lib/modules/dep_chain_tests.o 2> test_output.ll
		;;
	*)
		echo "Invalid command line argument"
		exit -1
		;;
esac

./scripts/clang-tools/gen_compile_commands.py


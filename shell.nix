# { pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/953909341ea62291a4c4a9483214f9a12d58ec99.tar.gz") {} }:
with import <nixpkgs> {};
let
  aarch64 = pkgs.pkgsCross.aarch64-multiplatform;
  binutils-unwrapped = aarch64.buildPackages.binutils-unwrapped.overrideAttrs (old: {
    name = "binutils-2.37";
    src = aarch64.fetchurl {
      url = "https://ftp.gnu.org/gnu/binutils/binutils-2.37.tar.xz";
      sha256 = "sha256-gg2XJPAgo+acszeJOgtjwtsWHa3LDgb8Edwp6x6Eoyw=";
    };
    patches = [];
  });
  myclang = aarch64.buildPackages.callPackage ./impure-clang.nix {};
  #cc = wrapCCWith rec {
  #  cc = myclang;
  #  bintools = wrapBintoolsWith {
  #    bintools = binutils-unwrapped';
  #    libc = glibc;
  #  };
  #};
  # cc = aarch64.buildPackages.wrapCCWith rec {
  #   cc = aarch64.buildPackages.llvmPackages_13.clang-unwrapped;
  #   # bintools = aarch64.buildPackages.wrapBintoolsWith {
  #   #   bintools = binutils-unwrapped;
  #   #   libc = aarch64.glibc;
  #   # };
  # };
  mystdenv = overrideCC aarch64.stdenv myclang;
in mystdenv.mkDerivation {
  name = "env";
#(aarch64.linux.override {
#  # To configure:
#  # make $makeFlags defconfig
#  # To build:
#  # make $makeFlags -j$(nproc)
#  stdenv = overridecc aarch64.stdenv myclang;
#  #stdenv = overrideCC aarch64.clang13Stdenv cc;
#}).overrideAttrs (old: {
  #stdenv = overrideCC aarch64.clang13Stdenv cc;
  nativeBuildInputs = aarch64.linux.nativeBuildInputs ++ [
#    pkgs.llvmPackages_13.clang
    #pkgs.llvmPackages_13.bintools
    #pkgs.binutils
    #pkgs.llvmPackages_13.libcxx
    #pkgs.llvmPackages_13.libcxxabi
    pkgs.ncurses
    pkgs.perl
    pkgs.gdb
    pkgs.lldb
  ];
  inherit (aarch64.linux) buildInputs;

  ASAN_SYMBOLIZER_PATH="/scratch/paul/src/llvm-project/build/bin/llvm-symbolizer";
  MSAN_SYMBOLIZER_PATH="/scratch/paul/src/llvm-project/build/bin/llvm-symbolizer";

  depsBuildBuild = [ 
    aarch64.buildPackages.stdenv.cc 
  ];
  
# export PATH="/scratch/paul/src/llvm-project/build/bin:$PATH"
# export C_INCLUDE_PATH="${stdenv.cc.libc.dev}/include"
# export C_PLUS_INCLUDE_PATH="${pkgs.llvmPackages_14.libclang}/include";
# export CFLAGS="-B${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version} -B ${stdenv.cc.libc}/lib"

# export CPATH=$CPATH:${pkgs.llvmPackages_13.libcxx}/include/c++/v1:${stdenv.glibc.dev}/include
# export CPLUS_INCLUDE_PATH=$CPATH
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${pkgs.llvmPackages_13.libcxx}/lib:${pkgs.llvmPackages_13.libcxxabi}/lib
# export NIX_LDFLAGS=$NIX_LDFLAGS"-L${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version}"
# export CFLAGS="-B${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version} -B ${stdenv.cc.libc}/lib"

  #shellHook = ''
  #  export CPATH="$CPATH:${pkgs.llvmPackages_13.libcxx}/include/c++/v1:${stdenv.glibc.dev}/include"
  #  export NIX_LDFLAGS="$NIX_LDFLAGS -L${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version} -L${gccForLibs}/lib/gcc/${hostPlatform.config}/${gccForLibs.version}"
  #  export CFLAGS="-B${gccForLibs}/lib/gcc/${targetPlatform.config}/${gccForLibs.version} -B${stdenv.cc.libc}/lib"
  #'';

#   makeFlags = ["CC=/scratch/paul/src/llvm-project/build/bin/clang" "ARCH=arm64" "CROSS_COMPILE=aarch64-unknown-linux-gnu-" "SHELL=bash" "KCFLAGS=-g" "KCFLAGS=-fsanitize=lkmm-dep-checker" "-j$NIX_BUILD_CORES"];
#   makeFlags1 = ["CC=/scratch/paul/src/llvm-rpoject/build/bin/clang" "ARCH=arm64" "CROSS_COMPILE=aarch64-unknown-linux-gnu-" "SHELL=bash" "KCFLAGS=-g" "KCFLAGS=-fsanitize=lkmm-dep-checker" "-j1"];
#   makeFlagsLLVM = ["CC=/scratch/paul/src/llvm-project/build/bin/clang" "LD=/scratch/paul/src/llvm-project/build/bin/ld.lld" "AR=/scratch/paul/src/llvm-project/build/bin/llvm-ar" "NM=/scratch/paul/src/llvm-project/build/bin/llvm-nm" "STRIP=/scratch/paul/src/llvm-project/build/bin/llvm-strip" "OBJCOPY=/scratch/paul/src/llvm-project/build/bin/llvm-objcopy" "OBJDUMP=/scratch/paul/src/llvm-project/build/bin/llvm-objdump" "READELF=/scratch/paul/src/llvm-project/build/bin/llvm-readelf" "HOSTCC=clang" "HOSTCXX=clang++" "HOSTAR=llvm-ar" "HOSTLD=ld.lld" "LLVM_IAS=1" "ARCH=arm64" "CROSS_COMPILE=aarch64-unknown-linux-gnu-" "SHELL=bash" "KCFLAGS=-g" "KCFLAGS=-fsanitize=lkmm-dep-checker" "-j$NIX_BUILD_CORES"];
}

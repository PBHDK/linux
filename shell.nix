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
  cc = aarch64.buildPackages.wrapCCWith rec {
    cc = aarch64.buildPackages.llvmPackages_11.clang-unwrapped;
    bintools = aarch64.buildPackages.wrapBintoolsWith {
      bintools = binutils-unwrapped;
      libc = aarch64.glibc;
    };
  };
in
(aarch64.linux.override {
  # To configure:
  # make $makeFlags defconfig
  # To build:
  # make $makeFlags -j$(nproc)
  stdenv = overrideCC aarch64.clang11Stdenv cc;
}).overrideAttrs (old: {
  nativeBuildInputs = old.nativeBuildInputs ++ [
    pkgs.gllvm
    pkgs.llvmPackages_13.clang
    pkgs.llvmPackages_13.bintools
    pkgs.ncurses
    pkgs.perl
  ];
  ASAN_SYMBOLIZER_PATH="~/src/DoitLK-llvm/build/bin/llvm-symbolizer";
  MSAN_SYMBOLIZER_PATH="~/src/DoitLK-llvm/build/bin/llvm-symbolizer";
  makeFlags = ["CC=/home/paul/src/DoitLK-llvm/build/bin/clang" "ARCH=arm64" "CROSS_COMPILE=aarch64-unknown-linux-gnu-" "SHELL=bash" "-j$NIX_BUILD_CORES"];
})
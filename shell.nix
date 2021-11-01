# { pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/953909341ea62291a4c4a9483214f9a12d58ec99.tar.gz") {} }:
with import <nixpkgs> {};
let
  aarch64 = pkgs.pkgsCross.aarch64-multiplatform;
in
(aarch64.linux.override {
  # To configure:
  # make $makeFlags defconfig
  # To build:
  # make $makeFlags -j$(nproc)
  stdenv = aarch64.clang11Stdenv;
}).overrideAttrs (old: {
  nativeBuildInputs = old.nativeBuildInputs ++ [
    pkgs.gllvm
    pkgs.ncurses
    pkgs.perl
  ];
  ASAN_SYMBOLIZER_PATH="~/src/DoitLK-llvm/build/bin/llvm-symbolizer";
  MSAN_SYMBOLIZER_PATH="~/src/DoitLK-llvm/build/bin/llvm-symbolizer";
  makeFlags = ["CC=/home/paul/src/DoitLK-llvm/build/bin/clang" "ARCH=arm64" "CROSS_COMPILE=aarch64-unknown-linux-gnu-" "SHELL=bash" "-j$NIX_BUILD_CORES"];
})

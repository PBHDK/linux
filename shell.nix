{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/953909341ea62291a4c4a9483214f9a12d58ec99.tar.gz") {} }:
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
  ];
  ASAN_SYMBOLIZER_PATH="~/src/DoitLK-llvm/build/bin/llvm-symbolizer";
  MSAN_SYMBOLIZER_PATH="~/src/DoitLK-llvm/build/bin/llvm-symbolizer";
})

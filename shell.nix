with import <nixpkgs> {
  # crossSystem = {
  #   config = "aarch64-unknown-linux-gnu";
  # };
};
mkShell {
  nativeBuildInputs = [
    #pkgsCross.aarch64-multiplatform.binutils
    pkgsCross.aarch64-multiplatform.buildPackages.gcc
    getopt
    flex
    bison
    binutils
    #llvmPackages_11.llvm
    clang_11
    gnumake
    bc
    perl
    hostname
    cpio
  ] ++ map lib.getDev [
    elfutils
    ncurses
    openssl
    zlib
  ];
  buildInputs = [
  
  ];
}
# bc
# perl
# binutils
# bison
# clang_11
# cpio
# flex
# getopt
# gnumake
# git
# hostname
# elfutils
# ncurses
# openssl
# zlib
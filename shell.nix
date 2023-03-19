# to update get a commit from here https://status.n#ixos.org/
{ pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/b38db2c901b30a37d345751f4f6418d416e7e46e.tar.gz") { } }:
with pkgs;
let
  aarch64 = pkgsCross.aarch64-multiplatform;
  clangPath = toString ../llvm-project/build;
  myclang = aarch64.buildPackages.wrapCCWith {
    gccForLibs = null;
    inherit (aarch64.buildPackages.targetPackages.llvmPackages_13.libraries) libcxx;
    cc = (stdenv.mkDerivation {
      name = "impure-clang";
      dontUnpack = true;
      installPhase = ''
        mkdir -p $out/bin
        for bin in ${toString (builtins.attrNames (builtins.readDir /scratch/paul/src/llvm-project/build/bin))}; do
          cat > $out/bin/$bin <<EOF
        #!${runtimeShell}
        exec "${clangPath}/bin/$bin" "\$@"
        EOF
          chmod +x $out/bin/$bin
        done
      '';
      passthru.isClang = true;
    });
  };
in
(aarch64.buildPackages.overrideCC aarch64.stdenv myclang).mkDerivation {
  name = "env";
  nativeBuildInputs = aarch64.linux.nativeBuildInputs;
  depsBuildBuild = with aarch64.buildPackages; [ python3.pkgs.autopep8 ncurses stdenv.cc openssl ];
  buildInputs = [
    aarch64.zlib
    aarch64.openssl
  ];
  #NIX_CFLAGS_COMPILE = "-isystem ${clangPath}/build/tools/clang/lib/Headers";
  NIX_LDFLAGS = "-L${aarch64.buildPackages.targetPackages.llvmPackages_13.libraries.libcxxabi}/lib";
  hardeningDisable = [ "all" ];
  PATH_TO_CLANG = "${myclang.cc}/bin/clang";
  PATH_TO_CLANGD = "${myclang.cc}/bin/clangd";
}

with import <nixpkgs> {};
let
	binutils-unwrapped' = binutils-unwrapped.overrideAttrs (old: {
		  name = "binutils-2.38";
   		  src = pkgs.fetchurl {
   		    url = "https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz";
   		    sha256 =
		    "e316477a914f567eccc34d5d29785b8b0f5a10208d36bbacedcc39048ecfe024";
   		  };
   		  patches = [];
	});
	cc = wrapCCWith rec {
  	    cc = (callPackage ./impure-clang.nix {});
  	    bintools = wrapBintoolsWith {
  	      bintools = binutils-unwrapped';
  	      libc = glibc;
  	    };
	    isClang = true;
	};
in
	(overrideCC stdenv cc).mkDerivation {
		name = "kernel-hacking";
		buildInputs = [ 
			bc
		  	perl
		  	bison
		  	cpio
		  	flex
		  	getopt
		  	gnumake
		  	hostname
		  	elfutils
		  	ncurses
		  	openssl
			zlib
		];
}


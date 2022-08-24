#!/usr/bin/env python3

import sys
import subprocess
import os
import re

_arm64_CROSS=["ARCH=arm64", "CROSS_COMPILE=aarch64-unknown-linux-gnu-"]

_ENABLE_DEP_CHEKER=["KCFLAGS=-fsanitize=lkmm-dep-checker"]

_MAKEFLAGS=["CC=clang"] + _arm64_CROSS+ _ENABLE_DEP_CHEKER

def update_config():
    print("Updating config for dep checker support")
    subprocess.run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_NONE"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_LTO_NONE"])
    subprocess.run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_REDUCED"])
    subprocess.run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_SPLIT"])
    subprocess.run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_BTF"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_PAHOLE_HAS_SPLIT_BTF"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_PAHOLE_HAS_BTF_TAG"])
    subprocess.run(["./scripts/config", "--disable", "CONFIG_GDB_SCRIPTS"])
    subprocess.run(["./scripts/config", "--disable", "CONFIG_DEBUG_EFI"])

def configure_kernel(config):
  subprocess.run(["make"] + _MAKEFLAGS + [config], check=True)
  update_config()

def build_kernel(threads=os.environ["NIX_BUILD_CORES"], ModulePath=""):
  JStr = "-j" + threads
  with open("build_output.ll", "w+") as f:
    if ModulePath:
      subprocess.run(["make"] + _MAKEFLAGS + [JStr] + [ModulePath], stderr=f, check=True)
    else:
      subprocess.run(["make"] + _MAKEFLAGS + [JStr], stderr=f, check=True)

  subprocess.run(["./scripts/clang-tools/gen_compile_commands.py"], check=True)

def debug_kernel(ModulePath: str):
  # Build required module to obtain compile command
  build_kernel("1", ModulePath)
  ModulePathPartition = ModulePath.rpartition("/")

  # Get path to compile commands for module
  CompileCmdsPath = ModulePathPartition[0] + ModulePathPartition[1] + "." + ModulePathPartition[2] + ".cmd"

  # Grab compile command
  with open(CompileCmdsPath) as f:
    CompileCmdsStr = f.readline()
    R = re.search(r'(?<=:=)[\s\S]*', CompileCmdsStr)
    if not R:
      print("Couldn't find compile command in " + CompileCmdsStr)
      exit(-1)
    CompileCmd: str = R.group()

  # Make compile command emit LLVM IR
  CompileCmd = CompileCmd.replace("-c -o", "-emit-llvm -o - -S")

  # Compile with -O2
  with open(ModulePathPartition[2] + "2.ll", "w+") as f:
    print("Generating IR -O2")
    # subprocess.run(CompileCmd.split(), stdout=f, check=True)
    subprocess.run(CompileCmd, stdout=f, stderr=subprocess.DEVNULL, shell=True, check=True)

  # Update compile command to use -O0
  CompileCmd = CompileCmd.replace("-O2", "-O0", 1)

  # Compile with -O0
  with open(ModulePathPartition[2] + "0.ll", "w+") as f:
    print("Generating IR -O0")
    # subprocess.run(CompileCmd.split(" "), stdout=f, check=True)
    subprocess.run(CompileCmd, stdout=f, stderr=subprocess.DEVNULL, shell=True, check=True)

if __name__ == "__main__":
  match sys.argv[1]:
    case "mrproper":
      subprocess.run(["make", "mrproper"], check=True)
    case "clean":
      subprocess.run(["make", "clean"], check=True)
    case "config":
      if sys.argv[2]:
        configure_kernel(sys.argv[2])
      else:
        print("Config argument missing")
    case "fast":
      build_kernel()
    case "module":
      build_kernel("1", sys.argv[2])
    case "precise":
      build_kernel("1")
    case "tests":
      build_kernel("1", "lib/modules/dep_chain_tests.o")
    case "debug":
      debug_kernel(sys.argv[2])
    case _:
      print("invalid argument")

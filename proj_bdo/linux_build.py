#!/usr/bin/env python3

import sys
import subprocess
import os
import re

_arm64_CROSS = ["ARCH=arm64", "CROSS_COMPILE=aarch64-unknown-linux-gnu-"]

_PROJ_BDO_FLAGS = ["KCFLAGS=-fsanitize=lkmm-dep-checker"]
_PROJ_BDO_TESTS = [
    "KCFLAGS=-fsanitize=lkmm-dep-checker -mllvm -lkmm-enable-tests"]

_MAKEFLAGS = ["HOSTCC=gcc", "CC=clang"] + _arm64_CROSS + _PROJ_BDO_FLAGS
_MAKEFLAGS_TESTS = ["HOSTCC=gcc", "CC=clang"] + _arm64_CROSS + _PROJ_BDO_TESTS


def run(args, stderr=None, stdout=None, shell=False, executable=None):
    print("[ " + " ".join(args) + " ]\n")

    subprocess.run(args=args, stderr=stderr, stdout=stdout,
                   shell=shell, executable=executable)


def add_dep_checker_support_to_config():
    print("\nUpdating config for dep checker support:\n")
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_NONE"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run(["./scripts/config", "--enable",
         "CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT"])
    run(["./scripts/config", "--enable", "CONFIG_LTO_NONE"])
    run(["./scripts/config", "--disable",
         "CONFIG_DEBUG_INFO_REDUCED"])
    run(["./scripts/config", "--disable",
         "CONFIG_DEBUG_INFO_SPLIT"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_BTF"])
    run(["./scripts/config", "--enable",
         "CONFIG_PAHOLE_HAS_SPLIT_BTF"])
    run(["./scripts/config", "--enable",
         "CONFIG_PAHOLE_HAS_BTF_TAG"])
    run(["./scripts/config", "--disable", "CONFIG_GDB_SCRIPTS"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_EFI"])


def add_syzkaller_support_to_config():
    # Suggested by:
    # https://docs.kernel.org/dev-tools/gdb-kernel-debugging.html
    run(["./scripts/config", "--enable", "CONFIG_GDB_SCRIPTS"])
    build_kernel(ModulePath="scripts_gdb", output_file="/dev/null")

    # Suggested by:
    # https://github.com/google/syzkaller/blob/master/docs/linux/setup_linux-host_qemu-vm_arm64-kernel.md
    run(["./scripts/config", "--enable", "CONFIG_KCOV"])
    run(["./scripts/config", "--enable", "CONFIG_KASAN"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run(["./scripts/config", "--set-str", "CONFIG_CMDLINE", "console=ttyAMA0"])
    run(["./scripts/config", "--enable", "CONFIG_KCOV_INSTRUMENT_ALL"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_FS"])
    run(["./scripts/config", "--enable", "CONFIG_NET_9P"])
    run(["./scripts/config", "--enable", "CONFIG_NET_9P_VIRTIO"])
    run(["./scripts/config", "--set-str",
        "CONFIG_CROSS_COMPILE", "aarch64-linux-gnu-"])


def configure_kernel(config):
    run(["make"] + _MAKEFLAGS + [config])
    add_dep_checker_support_to_config()


def build_kernel(
        threads=os.getenv("NIX_BUILD_CORES", "128"),
        ModulePath="", output_file="build_output.ll"):
    JStr = "-j" + threads
    # FIXME: make this readable
    with open(output_file, "w+") as f:
        if ModulePath:
            if (os.path.exists(ModulePath)):
                run(["rm"] + [ModulePath], stderr=f)

            run(["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"] + (_MAKEFLAGS_TESTS if output_file == "test_output.ll"
                                                                        else _MAKEFLAGS) + [JStr] + [ModulePath], stderr=f)
        else:
            run(["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"] +
                _MAKEFLAGS + [JStr], stderr=f)

    print("\nGenerating compilation database:\n")
    run(["./scripts/clang-tools/gen_compile_commands.py"])


def debug_kernel(ObjPath: str):
    # Build required object to obtain compile command
    if ObjPath == "proj_bdo/dep_chain_tests.o":
        build_kernel("1", "proj_bdo/dep_chain_tests.o", "test_output.ll")
    else:
        build_kernel("1", ObjPath, "obj_output.ll")

    ModulePathPartition = ObjPath.rpartition("/")

    # Get path to compile commands for object
    CompileCmdsPath = ModulePathPartition[0] + \
        ModulePathPartition[1] + "." + ModulePathPartition[2] + ".cmd"

    # Grab compile command
    with open(CompileCmdsPath) as f:
        CompileCmdsStr = f.readline()
        R = re.search(r'(?<=:=)[\s\S]*', CompileCmdsStr)
        if not R:
            print("\nCouldn't find compile command in " + CompileCmdsStr + "\n")
            exit(-1)
        CompileCmd: str = R.group()

    # Make compile command emit LLVM IR after verifier
    # FIXME: Doesn't work as intended because LKMM passes are module passes.
    CompileCmd = CompileCmd.replace(
        "-c -o", "-emit-llvm -mllvm -print-after=lkmm-verify-deps -o - -S")

    # Compile with -O2
    with open(ModulePathPartition[2] + "2.ll", "w+") as f:
        print("\nGenerating IR -O2:\n")
        run(CompileCmd.split(), stdout=f, stderr=subprocess.DEVNULL, shell=True)

    # Update compile command to use -O0
    CompileCmd = CompileCmd.replace("-O2", "-O0", 1)

    # Print IR after annotator
    CompileCmd = CompileCmd.replace(
        "-print-after=lkmm-verify-deps", "-print-after=lkmm-annotate-deps", 1)

    # Compile with -O0
    with open(ModulePathPartition[2] + "0.ll", "w+") as f:
        print("\nGenerating IR -O0:\n")
        run(CompileCmd.split(), stdout=f, stderr=subprocess.DEVNULL, shell=True)


if __name__ == "__main__":
    match sys.argv[1]:
        case "mrproper":
            run(["make", "mrproper"])
        case "clean":
            run(["make", "clean"])
        case "config":
            if sys.argv[2]:
                configure_kernel(sys.argv[2])
                if sys.argv[3] == "syzkaller":
                    add_syzkaller_support_to_config()
            else:
                print("\nConfig argument missing\n")
        case "fast":
            build_kernel()
        case "object":
            build_kernel("1", sys.argv[2], "obj_output.ll")
        case "precise":
            build_kernel("1")
        case "tests":
            debug_kernel("proj_bdo/dep_chain_tests.o")
        case "debug":
            debug_kernel(sys.argv[2])
        case _:
            print("invalid argument")

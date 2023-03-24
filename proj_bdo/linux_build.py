#!/usr/bin/env python3

import sys
import subprocess
import re
from common import utils


def debug_kernel(ObjPath: str):
    # Build required object to obtain compile command
    if ObjPath == "proj_bdo/dep_chain_tests.o":
        utils.build_kernel("1", "proj_bdo/dep_chain_tests.o", "test_output.ll")
    else:
        utils.build_kernel("1", ObjPath, "obj_output.ll")

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
        utils.run(CompileCmd.split(), stdout=f,
                  stderr=subprocess.DEVNULL, shell=True)

    # Update compile command to use -O0
    CompileCmd = CompileCmd.replace("-O2", "-O0", 1)

    # Print IR after annotator
    CompileCmd = CompileCmd.replace(
        "-print-after=lkmm-verify-deps", "-print-after=lkmm-annotate-deps", 1)

    # Compile with -O0
    with open(ModulePathPartition[2] + "0.ll", "w+") as f:
        print("\nGenerating IR -O0:\n")
        utils.run(CompileCmd.split(), stdout=f,
                  stderr=subprocess.DEVNULL, shell=True)


if __name__ == "__main__":
    match sys.argv[1]:
        case "mrproper":
            utils.run(["make", "mrproper"])
        case "clean":
            utils.run(["make", "clean"])
        case "config":
            if sys.argv[2]:
                utils.configure_kernel(sys.argv[2])
                if sys.argv[3] == "syzkaller":
                    add_syzkaller_support_to_config()
            else:
                print("\nConfig argument missing\n")
        case "fast":
            with open("proj_bdo/build_output.ll", "w+") as f:
                utils.build_kernel(stderr=f)
        case "object":
            with open("proj_bdo/obj_output.ll", "w+") as f:
                utils.build_kernel(
                    threads="1", module_path=sys.argv[2], stderr=f)
        case "precise":
            with open("build_output.ll", "w+") as f:
                utils.build_kernel("1", stderr=f)
        case "tests":
            debug_kernel("proj_bdo/dep_chain_tests.o")
        case "debug":
            debug_kernel(sys.argv[2])
        case _:
            print("invalid argument")

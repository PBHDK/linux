#!/usr/bin/env python3

import sys
import re
from common import utils


def debug_kernel(ObjPath: str, arch: str, add_args: list[str] = []):
    # Build required object to obtain compile command
    if ObjPath == "proj_bdo/dep_chain_tests.o":
        res = utils.clang_build_kernel(
            threads="1",
            ObjPath=ObjPath,
            stderr="test_output.err",
            add_args=add_args,
            arch=arch
        )
        with open("test_output.err") as f:
            bds = utils.count_str_file(s="dependency with ID", f=f)
        if (res.returncode != 0):
            exit("Clang crashed when building the tests.")
    else:
        utils.clang_build_kernel(threads="1",
                                 ObjPath=ObjPath,
                                 stderr="obj_output.err",
                                 add_args=add_args, arch=arch)

    ModulePathPartition = ObjPath.rpartition("/")

    # Get path to compile commands for object
    CompileCmdsPath = ModulePathPartition[0] + \
        ModulePathPartition[1] + "." + ModulePathPartition[2] + ".cmd"

    # Grab compile command
    with open(CompileCmdsPath) as f:
        CompileCmdsStr = f.readline()
        R = re.search(r'(?<=:= )[\s\S]*', CompileCmdsStr)
        if not R:
            print("\nCouldn't find compile command in " + CompileCmdsStr + "\n")
            exit(-1)
        CompileCmd: str = R.group()

    # Make compile command emit LLVM IR after verifier
    # FIXME: Doesn't work as intended because LKMM passes are module passes.
    CompileCmd = CompileCmd.replace(
        "-c -o ", "-Qunused-arguments -emit-llvm -mllvm -print-after=lkmm-verify-deps -o - -S ")

    # Compile with -O2
    with open(ModulePathPartition[2].rstrip(".o") + "2.ll", "w+") as f:
        print("\nGenerating IR -O2:\n")
        utils.run([CompileCmd], stdout=f, shell=True)

    # Update compile command to use -O0
    CompileCmd = CompileCmd.replace("-O2", "-O0", 1)

    # Print IR after annotator
    CompileCmd = CompileCmd.replace(
        "-print-after=lkmm-verify-deps", "-print-after=lkmm-annotate-deps", 1)

    # Compile with -O0
    with open(ModulePathPartition[2].rstrip(".o") + "0.ll", "w+") as f:
        print("\nGenerating IR -O0:\n")
        utils.run([CompileCmd], stdout=f, shell=True)

    if ObjPath == "proj_bdo/dep_chain_tests.o":
        print("{} test cases passed.".format(str(bds)))


if __name__ == "__main__":
    match sys.argv[1]:
        case "mrproper":
            utils.run(["make", "mrproper"])
        case "clean":
            utils.run(["make", "clean"])
        case "config":
            arch = sys.argv[3]
            utils.configure_kernel(config=sys.argv[2], arch=arch)
            if len(sys.argv) > 4 and sys.argv[4] == "syzkaller":
                utils.add_syzkaller_support_to_config(arch=arch)
        case "fast":
            utils.clang_build_kernel(arch=sys.argv[2])
        case "object":
            with open("proj_bdo/obj_output.err", "w+") as f:
                utils.clang_build_kernel(
                    threads="1", module_path=sys.argv[2], stderr=f)
        case "precise":
            with open("build_output.err", "w+") as f:
                utils.clang_build_kernel(
                    threads="1", stderr=f)
        case "tests":
            arch = sys.argv[2]
            if len(sys.argv) > 3 and sys.argv[3] == "relaxed":
                pass
                # TODO:
                # utils.run(["./scripts/config", "--enable", ""])
                # debug_kernel("proj_bdo/dep_chain_tests.o", arch=arch)
            else:
                utils.run(
                    ["./scripts/config", "--enable", "CONFIG_LKMMDC_TEST"])
                debug_kernel("proj_bdo/dep_chain_tests.o",  arch=arch)
        case "debug":
            debug_kernel(sys.argv[2])
        case _:
            print("invalid argument")

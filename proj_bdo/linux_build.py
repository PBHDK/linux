#!/usr/bin/env python3

import sys
import re
from common import utils


<<<<<<< HEAD
def debug_kernel(ObjPath: str, add_args=list[str]):
=======
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
    print("\nBuilding GDB Scripts:\n")
    run(["./scripts/config", "--enable", "CONFIG_GDB_SCRIPTS"])
    build_kernel(ModulePath="scripts_gdb", output_file="/dev/null")

    # Suggested by:
    # https://github.com/google/syzkaller/blob/master/docs/linux/setup_linux-host_qemu-vm_arm64-kernel.md
    print("\nUpdating config for syzkaller support:\n")
    # FIXME: why is KCOV making builds fail?
    # run(["./scripts/config", "--enable", "CONFIG_KCOV"])
    run(["./scripts/config", "--enable", "CONFIG_KASAN"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run(["./scripts/config", "--set-str", "CONFIG_CMDLINE", "console=ttyAMA0"])
    # run(["./scripts/config", "--enable", "CONFIG_KCOV_INSTRUMENT_ALL"])
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
>>>>>>> ac969125b8e3 (build: add output for add_syzkaller_support_to_config())
    # Build required object to obtain compile command
    if ObjPath == "proj_bdo/dep_chain_tests.o":
        res = utils.build_clang_arm64_kernel(
            threads="1",
            ObjPath=ObjPath,
            stderr="test_output.err",
            add_args=add_args
        )
        with open("test_output.err") as f:
            bds = utils.count_str_file(s="dependency with ID", f=f)
        if (res.returncode != 0):
            exit("Clang crashed when building the tests.")
    else:
        utils.build_clang_arm64_kernel(threads="1",
                                       ObjPath=ObjPath,
                                       stderr="obj_output.err",
                                       add_args=add_args)

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
    add_args = ["KCFLAGS={}".format(utils.DC_FLAGS)]
    match sys.argv[1]:
        case "mrproper":
            utils.run(["make", "mrproper"])
        case "clean":
            utils.run(["make", "clean"])
        case "config":
            if sys.argv[2]:
                utils.configure_kernel(config=sys.argv[2], add_args=add_args)
                if sys.argv[3] == "syzkaller":
                    add_syzkaller_support_to_config()
            else:
                print("\nConfig argument missing\n")
        case "fast":
            with open("proj_bdo/build_output.err", "w+") as f:
                utils.build_clang_arm64_kernel(stderr=f, add_args=add_args)
        case "object":
            with open("proj_bdo/obj_output.err", "w+") as f:
                utils.build_clang_arm64_kernel(
                    threads="1", module_path=sys.argv[2],
                    stderr=f, add_args=add_args)
        case "precise":
            with open("build_output.err", "w+") as f:
                utils.build_clang_arm64_kernel(
                    threads="1", stderr=f, add_args=add_args)
        case "tests":
            add_args[0] += " {}".format(utils.TEST_FLAGS)
            if len(sys.argv) > 2 and sys.argv[2] == "relaxed":
                add_args[0] += " {}".format(utils.REL_FLAGS)
                debug_kernel("proj_bdo/dep_chain_tests.o",
                             add_args=add_args)
            else:
                debug_kernel("proj_bdo/dep_chain_tests.o", add_args=add_args)
        case "debug":
            debug_kernel(sys.argv[2], add_args=add_args)
        case _:
            print("invalid argument")

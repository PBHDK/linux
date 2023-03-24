import subprocess
import os

_ARM64_CROSS_FLAGS = ["ARCH=arm64", "CROSS_COMPILE=aarch64-unknown-linux-gnu-"]

_PROJ_BDO_FLAGS = ["KCFLAGS=-fsanitize=lkmm-dep-checker"]
_PROJ_BDO_TEST_FLAGS = [
    "KCFLAGS=-fsanitize=lkmm-dep-checker -mllvm -lkmm-enable-tests"]

_MAKEFLAGS = ["HOSTCC=gcc", "CC=clang"] + _ARM64_CROSS_FLAGS + _PROJ_BDO_FLAGS
_MAKEFLAGS_TESTS = ["HOSTCC=gcc", "CC=clang"] + \
    _ARM64_CROSS_FLAGS + _PROJ_BDO_TEST_FLAGS


def run(args, stderr=None, stdout=None, shell=False, executable=None, text=False):
    print("[ " + " ".join(args) + " ]\n")

    return subprocess.run(args=args, stderr=stderr, stdout=stdout, shell=shell, executable=executable)


def get_kernel_version():
    return run(args=["make", "kernelversion"], stdout=subprocess.PIPE, text=True).stdout.strip()


def add_dep_checker_support_to_current_config():
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_NONE"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_REDUCED"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_SPLIT"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_BTF"])

    run(["./scripts/config", "--enable", "CONFIG_PAHOLE_HAS_SPLIT_BTF"])
    run(["./scripts/config", "--enable", "CONFIG_PAHOLE_HAS_BTF_TAG"])

    run(["./scripts/config", "--disable", "CONFIG_GDB_SCRIPTS"])

    run(["./scripts/config", "--disable", "CONFIG_DEBUG_EFI"])


def configure_kernel(config):
    run(["make"] + _MAKEFLAGS + [config])
    add_dep_checker_support_to_current_config()


def build_kernel(add_args=list(), threads=os.getenv("NIX_BUILD_CORES", "128"), ModulePath="", stderr="build_output.ll", text=False):
    JStr = "-j" + threads
    # with open(stderr, "w+") as f:
    if ModulePath:
        if (os.path.exists(ModulePath)):
            run(["rm"] + [ModulePath], stderr=stderr)
        res = run(["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"] +
                  _MAKEFLAGS + add_args + [JStr] + [ModulePath], stderr=stderr, text=text)
    else:
        res = run(["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"] +
                  _MAKEFLAGS + add_args + [JStr], stderr=stderr, text=text)

    print("\nGenerating compilation database:\n")
    run(["./scripts/clang-tools/gen_compile_commands.py"])

    return res

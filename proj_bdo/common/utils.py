"""Various utiliteis required for the dep checker."""
import subprocess
import os

from typing import Optional

_ARM64_CROSS_FLAGS = ["ARCH=arm64", "CROSS_COMPILE=aarch64-unknown-linux-gnu-"]

_PROJ_BDO_FLAGS = ["KCFLAGS=-fsanitize=lkmm-dep-checker"]
_PROJ_BDO_TEST_FLAGS = [
    "KCFLAGS=-fsanitize=lkmm-dep-checker -mllvm -lkmm-enable-tests"
]

_MAKEFLAGS = ["HOSTCC=gcc", "CC=clang"] + _ARM64_CROSS_FLAGS + _PROJ_BDO_FLAGS
_MAKEFLAGS_TESTS = ["HOSTCC=gcc", "CC=clang"] + \
    _ARM64_CROSS_FLAGS + _PROJ_BDO_TEST_FLAGS


def run(args: list[str],
        stderr: int = None,
        stdout: int = None,
        shell: bool = False,
        text: bool = True,
        cwd: Optional[str] = None):
    """
    Run a shell command and print it to stdout.

    args -- the shell command to run, including its arguments.
    stderr -- for capturing stderr.
    stdout -- for capturing stdout.
    shell -- specifies whether the command should be executed in a subshell.
    text -- if true, captures stdout/stderr as a string, not a byte sequence.
    cwd -- string to a different cwd where the command should be run.

    returns: return value of subprocess.run()
    """
    print("[ " + " ".join(args) + " ]\n")

    return subprocess.run(args=args,
                          stderr=stderr,
                          stdout=stdout,
                          shell=shell,
                          text=text,
                          cwd=cwd)


def clean_kernel():
    """Clean the kernel repo."""
    run(["make", "clean", "-s"])


def mrproper_kernel():
    """Deep clean the kernel repo."""
    run(["make", "mrproper", "-s"])


def get_kernel_version():
    """Get the kernel version of HEAD in the kernel repo."""
    return run(args=["make", "kernelversion"],
               stdout=subprocess.PIPE,
               text=True).stdout.strip()


def get_dep_checker_ver() -> str:
    """
    Return the dep checker version as a string based on the current branch \
            in $PROJECTS/llvm-project: "branch-hash-description".

    returnss: the dep checker version as a string.
    """
    branch: str = run(["git", "rev-parse", "--abbrev-ref", "HEAD"],
                      cwd=os.getenv("PROJECTS") + "/llvm-project",
                      stdout=subprocess.PIPE).stdout.rstrip("\n")

    head: str = run(
        ["git", "--no-pager", "show", "HEAD", "--oneline", "-s"],
        cwd=os.getenv("PROJECTS") + "/llvm-project",
        stdout=subprocess.PIPE).stdout.rstrip("\n")

    return (branch + "-" + head).replace(" ", "-")


def add_dep_checker_support_to_current_config():
    """Set all options the dep checker requires in a kernel config."""
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_NONE"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run([
        "./scripts/config", "--enable",
        "CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT"
    ])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_REDUCED"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_SPLIT"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_BTF"])

    run(["./scripts/config", "--enable", "CONFIG_PAHOLE_HAS_SPLIT_BTF"])
    run(["./scripts/config", "--enable", "CONFIG_PAHOLE_HAS_BTF_TAG"])

    run(["./scripts/config", "--disable", "CONFIG_GDB_SCRIPTS"])

    run(["./scripts/config", "--disable", "CONFIG_DEBUG_EFI"])


def configure_kernel(config: str):
    """
    Generate a kernel config.

    config -- the type of config to generate.
    """
    run(["make"] + _MAKEFLAGS + [config])
    add_dep_checker_support_to_current_config()


def build_kernel(add_args: list[str] = list(),
                 threads=os.getenv("NIX_BUILD_CORES", "128"),
                 ObjPath="",
                 stderr="build_output.ll"):
    """
    Build a Linux kernel.

    add_args -- additional arguments.
    threads -- the number of threads to use.
    ObjPath -- if specified, will only build the passed object.
    stderr -- the name of the file where stderr should be captured.
    """
    JStr = "-j" + threads
    if ObjPath:
        if (os.path.exists(ObjPath)):
            run(["rm"] + [ObjPath], stderr=stderr)
        res = run(["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"] +
                  _MAKEFLAGS + add_args + [JStr] + [ObjPath], stderr=stderr)
    else:
        res = run(["/usr/bin/time", "-v", "-o", "/dev/stdout",
                  "make"] + _MAKEFLAGS + add_args + [JStr], stderr=stderr)

    print("\nGenerating compilation database:\n")
    run(["./scripts/clang-tools/gen_compile_commands.py"])

    return res

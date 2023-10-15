"""Various utiliteis required for the dep checker."""
import os
import subprocess
from datetime import datetime
from typing import Optional, TextIO

_CLANG_FLAGS = [
    "HOSTCC=gcc",
    "CC=clang",
]

_CLANG_ARM64_ENV = _CLANG_FLAGS + [
    "ARCH=arm64",
    "CROSS_COMPILE=aarch64-unknown-linux-gnu-",
]
_CLANG_X86_64_ENV = _CLANG_FLAGS + ["ARCH=x86_64"]


def count_str_file(s: str, f: TextIO):
    """
    Counts how often a string occurs in a file.

    s: the string to find.
    f: the alreaddy opened file.

    returns: how often the string occurs in the file.
    """
    fstr = f.read()
    return fstr.count(s)


def run(
    args: list[str],
    stderr: TextIO = None,
    stdout: TextIO = None,
    shell: bool = False,
    text: bool = True,
    cwd: Optional[str] = None,
):
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

    return subprocess.run(
        args=args, stderr=stderr, stdout=stdout, shell=shell, text=text, cwd=cwd
    )


def clean_kernel():
    """Clean the kernel repo."""
    run(["make", "clean", "-s"])


def mrproper_kernel():
    """Deep clean the kernel repo."""
    run(["make", "mrproper", "-s"])


def get_kernel_version():
    """Get the kernel version of HEAD in the kernel repo."""
    return run(
        args=["make", "kernelversion"], stdout=subprocess.PIPE, text=True
    ).stdout.strip()


def get_dep_checker_ver() -> str:
    """
    Return the dep checker version as a string based on the current branch \
            in $PROJECTS/llvm-project: "branch-hash-description".

    returnss: the dep checker version as a string.
    """
    branch: str = run(
        ["git", "rev-parse", "--abbrev-ref", "HEAD"],
        cwd=os.getenv("PROJECTS") + "/llvm-project",
        stdout=subprocess.PIPE,
    ).stdout.rstrip("\n")

    head: str = run(
        ["git", "--no-pager", "show", "HEAD", "--oneline", "-s"],
        cwd=os.getenv("PROJECTS") + "/llvm-project",
        stdout=subprocess.PIPE,
    ).stdout.rstrip("\n")

    return (branch + "-" + head).replace(" ", "-")


def add_dep_checker_support_to_current_config():
    """Set all options the dep checker requires in a kernel config."""
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO_DWARF_TOOLCHAIN_DEFAULT"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_REDUCED"])
    run(["./scripts/config", "--enable", "DEBUG_INFO_COMPRESSED_NONE"])
    run(["./scripts/config", "--disable", "DEBUG_INFO_COMPRESSED_ZLIB"])
    run(["./scripts/config", "--disable", "CONFIG_DEBUG_INFO_SPLIT"])
    run(["./scripts/config", "--enable", "CONFIG_GDB_SCRIPTS"])
    run(["./scripts/config", "--enable", "DEBUG_KERNEL"])
    run(["./scripts/config", "--disable", "DEBUG_INFO_BTF"])
    run(["./scripts/config", "--enable", "LKMMDC"])
    run(["./scripts/config", "--disable", "LKMMDC_TEST"])


def add_syzkaller_support_to_config(arch: str, add_args: list[str] = []):
    return
    # Suggested by:
    # https://docs.kernel.org/dev-tools/gdb-kernel-debugging.html
    print("\nBuilding GDB Scripts:\n")
    run(["./scripts/config", "--enable", "CONFIG_GDB_SCRIPTS"])
    clang_build_kernel(
        ObjPath="scripts_gdb", add_args=add_args, stderr="/dev/null", arch=arch
    )

    # Suggested by:
    # https://github.com/google/syzkaller/blob/master/docs/linux/setup_linux-host_qemu-vm_arm64-kernel.md
    print("\nUpdating config for syzkaller support:\n")
    run(["./scripts/config", "--enable", "CONFIG_KCOV"])
    run(["./scripts/config", "--enable", "CONFIG_KASAN"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    run(["./scripts/config", "--set-str", "CONFIG_CMDLINE", "console=ttyAMA0"])
    run(["./scripts/config", "--enable", "CONFIG_KCOV_INSTRUMENT_ALL"])
    run(["./scripts/config", "--enable", "CONFIG_DEBUG_FS"])
    run(["./scripts/config", "--enable", "CONFIG_NET_9P"])
    run(["./scripts/config", "--enable", "CONFIG_NET_9P_VIRTIO"])
    run(["./scripts/config", "--set-str", "CONFIG_CROSS_COMPILE", "aarch64-linux-gnu-"])


def configure_kernel(config: str, add_args: list[str] = [], arch: str = "arm64"):
    """
    Generate a kernel config.

    config -- the type of config to generate.
    add_args -- additional arguments passed to the make invocation.
    arch -- the architecture to configure. Either arm64 (default) or x86_64.
    """
    env = _CLANG_ARM64_ENV
    if arch == "x86_64":
        env = _CLANG_X86_64_ENV
    run(["make"] + env + add_args + [config])
    add_dep_checker_support_to_current_config()


def clang_build_kernel(
    add_args: list[str] = list(),
    threads=os.getenv("NIX_BUILD_CORES", "128"),
    ObjPath="",
    stderr="build_output.err",
    arch="arm64",
):
    """
    Build an arm64 Linux kernel with the DepChecker support enabled.

    add_args -- additional arguments.
    threads -- the number of threads to use.
    ObjPath -- if specified, will only build the passed object.
    stderr -- the name of the file where stderr should be captured.
    arch -- the architecture to build for. Can be x86_64 or arm64
    """
    env: list[str] = _CLANG_ARM64_ENV
    if arch == "x86_64":
        env = _CLANG_X86_64_ENV

    dt_now = datetime.now()
    current_time = dt_now.strftime("%H:%M:%S")
    print("Starting build at: {}".format(current_time))

    with open(stderr, "w+") as SE:
        JStr = "-j" + threads
        res = None

        if ObjPath:
            if os.path.exists(ObjPath):
                run(["rm"] + [ObjPath], stderr=SE)
            if ObjPath == "proj_bdo/dep_chain_tests.o":
                res = run(
                    ["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"]
                    + env
                    + add_args
                    + [JStr, ObjPath, "-s"],
                    stderr=SE,
                )
            else:
                res = run(
                    ["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"]
                    + env
                    + add_args
                    + [JStr, ObjPath, "-s"],
                    stderr=SE,
                )
        else:
            res = run(
                ["/usr/bin/time", "-v", "-o", "/dev/stdout", "make"]
                + env
                + add_args
                + [JStr, "-s"],
                stderr=SE,
            )

    print("\nGenerating compilation database:\n")
    run(["./scripts/clang-tools/gen_compile_commands.py"])

    return res

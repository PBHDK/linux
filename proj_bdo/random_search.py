#!/usr/bin/env python3
"""
Script for searching for broken dependencies in randomly generated Linux
kernel configs

Collects trophies in --output-folder/trophies.txt and the most recent log in
--output-folder/logs/<dep_checker_version.txt>
"""

import subprocess
import re
import argparse

from typing import Optional, TextIO
from common import utils

_RAND_SEARCH_CONFIG_ARGS = [
    "KCONFIG_ALLCONFIG=proj_bdo/random_search_all.config"
]
_RAND_SEARCH_KC_ARGS = utils.DC_FLAGS + "-Qunused-arguments"

_SEED_PATT = r"(?<=^KCONFIG_SEED=).*"

_BRKN_DEP_PATT = r'//===-{26}Broken Dependency-{26}===//.*?//===-{69}===//$'
_BRKN_DEP_ID_PATT = \
    r'(?:Address|Control) dependency with ID: (.*?)\n\nDependency Beginning'

seed_matcher = re.compile(_SEED_PATT, re.MULTILINE)
bd_matcher = re.compile(_BRKN_DEP_PATT, re.MULTILINE | re.DOTALL)
bd_id_matcher = re.compile(_BRKN_DEP_ID_PATT, re.MULTILINE | re.DOTALL)

kv: str = utils.get_kernel_version()
dcv: str = utils.get_dep_checker_ver()
v_str: str = "DepChecker Version: {}\nKernel Version: {}".format(dcv, kv)
trophy_ids: set[str] = set()


def _pick_up_broken_deps_from_file(bdf: TextIO) -> set[str]:
    """
    Restore the state of the random search from .

    bdf -- broken deps file.

    returns: the set of broken deps/broken dep ids.
    """
    print("Restoring broken deps ...")
    res: set = set()

    bdf.seek(0)

    # Holds all of our discovered broken dependencies
    bds_str: str = bdf.read()

    bds: list[str] = bd_matcher.findall(bds_str)

    for bd in bds:
        bd_id_match: Optional[re.Match[str]] = bd_id_matcher.search(bd)

        if not bd_id_match:
            exit("Couldn't recover broken ID from:\n" + bd)

        bd_id: str = bd_id_match.group(1)

        res.add(bd_id)

    print("Restored " + str(len(res)) + " broken dep(s)")

    return res


def _generated_and_build_config(config_target: str,
                                lf: TextIO,
                                bdf: TextIO):
    """
    Generate a config of type config target and build it.

    config_target -- the config target passed to make
    lf -- the log file
    bdf -- the file containing the DepChecker's trophies

    returns: amount of new trophies.
    """
    seed: Optional[int] = None

    utils.mrproper_kernel()

    print("Generating {}".format(config_target))

    try:
        config_output = utils.run(
            args=["make", config_target] +
            utils._CLANG_ARM64_ENV + ["KCFLAGS={}".format(_RAND_SEARCH_KC_ARGS)] +
            _RAND_SEARCH_CONFIG_ARGS,
            stdout=subprocess.PIPE
        )
    except Exception:
        lf.writelines(config_output.stderr)
        exit("Couldn't generate config")

    if config_target == "randconfig":
        seed_match = seed_matcher.search(config_output.stdout)

        if seed_match:
            seed = seed_match.group()
        else:
            exit("Couldn't find seed")

        print("Seed: " + seed)

    # Buiid randconfig
    print("Building ...")
    try:
        build_result = utils.clang_build_kernel(
            add_args=["KCFLAGS={}".format(_RAND_SEARCH_KC_ARGS)], stderr=subprocess.PIPE)
    except Exception as e:
        print(e)
        exit("Couldn't build kernel.")

    if config_target == "randconfig":
        lf.writelines("## " + str(seed) + "\n\n" +
                      build_result.stderr + "\n\n")
    else:
        lf.writelines("## " + config_target + "\n\n" +
                      build_result.stderr + "\n\n")

    return _commit_new_broken_deps(build_result.stderr, bdf, seed)


def _commit_new_broken_deps(res: str, bdf: TextIO,
                            seed: Optional[str] = None) -> set[str]:
    """
    Process the build results and commit broken dependencies to the broken dep
    file if new ones were found.

    res -- the stderr output of the build.
    bd_file -- the broken deps file.
    curr_seed -- the seed used for generating the randconfig.

    returns: amount of new trophies
    """
    # Gather results
    new_bds: list[str] = bd_matcher.findall(res)
    new_trophies: int = 0

    # Check if we actually found a new broken dep
    for new_bd in new_bds:
        new_id_match: Optional[re.Match[str]] = bd_id_matcher.search(new_bd)

        if not new_id_match:
            exit("Couldn't find an ID in:\n" + new_bd)

        new_bd_id: str = new_id_match.group(1)

        if new_bd_id not in trophy_ids:
            last_newline: int = new_bd.rfind("\n")
            bdf.write(new_bd[:last_newline])
            if (seed):
                bdf.write("\n\nFound with seed: " + seed)
            bdf.write(v_str)
            bdf.write(new_bd[last_newline:] + "\n\n")

            trophy_ids.add(new_bd_id)

            print(new_bd + "\n\n")
            ++new_trophies

    print("This build produced {} new trophies.".format(str(new_trophies)))

    return new_trophies


def _run_random_testing(num_runs: int, bd_path: str, defconfig: bool):
    """
    Run random testing.

    num_runs -- number of random configs to test.
    bd_path -- path to the folder where the results should be stored.
    defconfig -- if set, will only build a defconfig and gather the results.
    """
    log_file_path: str = bd_path + "/logs/" + dcv + ".log"
    bd_file_path = bd_path + "/trophies.txt"

    with open(bd_file_path, 'a+') as bdf, open(log_file_path, "w+") as lf:
        trophy_ids.update(_pick_up_broken_deps_from_file(bdf))

        if defconfig:
            _generated_and_build_config("defconfig", lf, bdf)
        else:
            nts = 0
            for _ in range(num_runs):
                nts += _generated_and_build_config("randconfig", lf, bdf)
            print("All runs produced {} new trophies in total.".format(nts))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    """
    It seemed the easiest to just provide the --defconfig argument vs having
    to identify whether a defconfig hat already been searched for the current
    DepChecker version.
    """
    parser.add_argument("--defconfig",
                        action="store_true",
                        help="Set this option to true if you want to run a \
                                defconfig search for the current dep checker \
                                version.")
    parser.add_argument("-r",
                        "--runs",
                        type=int,
                        default=1,
                        help="The number of randconfigs to generate and build")
    parser.add_argument("-o",
                        "--output-folder",
                        type=str,
                        default="../broken_deps/random_testing/.",
                        help="path to folder where the results should be "
                        "stored.")

    args = parser.parse_args()

    _run_random_testing(num_runs=args.runs,
                        bd_path=args.output_folder, defconfig=args.defconfig)

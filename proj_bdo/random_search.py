#!/usr/bin/env python3

import subprocess
import os
import re
import argparse
import time
from common import utils

_RANDOM = ["randconfig"]

_MAKEFLAGS_RANDOM_CONFIG = utils._MAKEFLAGS + _RANDOM

_SEED_PATTERN = r"(?<=^KCONFIG_SEED=).*"

_BROKEN_DEP_PATTERN = r'//===-{26}Broken Dependency-{27}===//.*?//===-{70}===//$'
_BROKEN_DEP_ID_PATTERN = r'(?<=^(Address|Control) dependency with ID: ).*'

# Matchers
seed_matcher = re.compile(_SEED_PATTERN, re.MULTILINE)

bd_matcher = re.compile(_BROKEN_DEP_PATTERN, re.MULTILINE | re.DOTALL)
bd_id_matcher = re.compile(_BROKEN_DEP_ID_PATTERN, re.MULTILINE)


def pick_up_broken_deps_from_file(bd_file, only_ids=False):
    res = set()

    # Holds all of our discovered broken dependencies
    bds_str = bd_file.read()

    bds = bd_matcher.findall(bds_str)

    for bd in bds:
        if only_ids:
            bd_id = bd_id_matcher.search(bd)

            if not bd_id:
                print("Couldn't recover broken ID from:\n" + bd)
                continue

            res.add(bd_id.group())
        else:
            res.add(bd)

    return res


def update_config():
    utils.add_dep_checker_support_to_current_config()

    # Options which cause unnecessary computing overhead
    # TODO: disable sanitizers

    # Handle LTO
    utils.run(["./scripts/config", "--enable", "CONFIG_LTO_NONE"])


def commit_new_broken_deps(bds_str, bd_file, curr_seed):
    # Gather results
    new_bds = bd_matcher.findall(bds_str)

    print("This build produced " + str(len(new_bds)) + " broken dep(s)")

    # Check if we actually found a new broken dep
    for new_bd in new_bds:
        id_match = bd_id_matcher.search(new_bd)

        if not id_match:
            print("Couldn't find an ID in:\n" + new_bd)
            continue

        id = id_match.group()

        if id in bd_ids:
            # Add seed
            last_newline = new_bd.rfind("\n")
            new_bd = new_bd[:last_newline] + "\n\nFound with seed: " + \
                curr_seed + "\n" + new_bd[last_newline:]

            bd_file.write(new_bd + "\n\n")
            bd_ids.add(id)

            print(new_bd + "\n\n")


def initialise_random_search_for_current_version(dc_version, bd_file, log):
    print("Creating new broken deps record for dep checker version " +
          dc_version + " ...")

    kernel_version = utils.get_kernel_version()

    bd_file.write("# Random Testing kernel version " + kernel_version +
                  "with DepChecker version " + dc_version + "\n\n")
    bd_file.write("## Defconfig results\n\n")

    utils.configure_kernel()
    defconfig_res = utils.build_kernel(args, stderr=subprocess.PIPE, text=True)
    bds_from_file = pick_up_broken_deps_from_file(bd_file)

    print("defconfig yielded " + str(len(bds_from_file)) + " broken dep(s)")
    log.writelines("## " + "defconfig" + "\n\n" +
                   defconfig_res.stderr.decode("utf-8") + "\n\n")


def run_random_testing(num_runs, bd_file_name, dc_version):
    bds = set()

    with open(bd_file_name, 'w+') as bd_file, open("proj_bdo/broken_deps/logs/" + dc_version + time.strftime("--%Y-%m-%d--%H-%M-%S") + ".log", "x") as log:
        # If this is the first run of the script for this dc_version, we first populate bd_file with the results of a current defconfig build
        if not os.stat(bd_file_name).st_size == 0:
            initialise_random_search_for_current_version(
                dc_version, bd_file, log)
        else:
            print("Restoring broken deps from" + bd_file_name + "...")
            bds.update(pick_up_broken_deps_from_file(bd_file, only_ids=True))
            print("Restored " + str(len(bds)) + " broken dep(s)")

        print("Starting LKMM random testing for broken dependencies ...")
        for _ in range(num_runs):
            print("Cleaning ...")
            utils.run(["make", "mrproper", "-s"])

            print("Generating random config ...")
            config_output = utils.run(
                ["make"] + _MAKEFLAGS_RANDOM_CONFIG, stdout=subprocess.PIPE, text=True)

            # TODO save seed of config
            seed_match = seed_matcher.search(
                config_output.stdout.decode("utf-8"))
            curr_seed = "42"
            if seed_match:
                curr_seed = seed_match.group()
            else:
                exit("Couldn't find seed")

            print("Seed: " + curr_seed)

            utils.add_dep_checker_support_to_current_config()

            # Buiid randconfig
            print("Building ...")
            try:
                build_result = utils.build_kernel(
                    add_args=["-s"], stderr=subprocess.PIPE, text=True)
            except Exception as e:
                log.writelines("## " + "Failed build\n\n" +
                               build_result.stderr.decode("utf-8") + "\n\n")

            # Log this run's stderr output
            log.writelines("## " + curr_seed + "\n\n" +
                           build_result.stderr.decode("utf-8") + "\n\n")

            commit_new_broken_deps(
                build_result.stderr.decode("utf-8"), bd_file, curr_seed)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-r", "--runs", type=int, default=1,
                        help="The number of randconfigs to generate and build")
    parser.add_argument("--dep-checker-version", type=str,
                        help="The version of the dependency checker being used for random testing. The version should be a tag in the dependency checker's git repo.")
    parser.add_argument("-o", "--output", type=str, default="proj_bdo/broken_deps",
                        help="path to folder where the results should be stored.")

    args = parser.parse_args()

    run_random_testing(num_runs=args.runs, bd_file_name=args.output + "/" + args.dep_checker_version + ".txt",
                       dc_version=args.dep_checker_version)

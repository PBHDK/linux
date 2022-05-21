#!/usr/bin/env python3

import subprocess
import sys
import re

_BROKEN_DEPS_FILE_PATH="/scratch/paul/src/broken_deps/broken_deps"

_CROSS=["ARCH=arm64", "CROSS_COMPILE=aarch64-unknown-linux-gnu-"]
_ENABLE_DEP_CHEKER=["KCFLAGS=-fsanitize=lkmm-dep-checker"]
_RANDOM=["randconfig"]

_MAKEFLAGS=["make", "CC=clang"] + _CROSS + _ENABLE_DEP_CHEKER
_MAKEFLAGS_RANDOM_CONFIG=_MAKEFLAGS + _RANDOM
_MAKEFLAGS_BUILD=_MAKEFLAGS + ["-j32", "-s"]

_SEED_PATTERN=r"(?<=^KCONFIG_SEED=).*"

_BROKEN_DEP_PATTERN=r'//===-{26}Broken Dependency-{27}===//.*?//===-{70}===//$'
_BROKEN_DEP_ID_PATTERN=r'(?<=^(Address|Control) dependency with ID: ).*'

# Matchers
seedMatcher = re.compile(_SEED_PATTERN, re.MULTILINE)

brokenDepMatcher = re.compile(_BROKEN_DEP_PATTERN, re.MULTILINE | re.DOTALL)
brokenIDMatcher = re.compile(_BROKEN_DEP_ID_PATTERN, re.MULTILINE)

# Set of IDs of broken deps for faster access 
setOfBrokenIDs = set()

# Store previously discovered broken deps into set
def restoreBrokenDeps():
    # Holds all of our discovered broken dependencies
    with open(_BROKEN_DEPS_FILE_PATH) as brokenDepsFile:
        brokenDepsStr = brokenDepsFile.read();

        prevIDedBrokenDeps = brokenDepMatcher.findall(brokenDepsStr)

        for prevIDedBrokenDep in prevIDedBrokenDeps:
            brokenIDMatch = brokenIDMatcher.search(prevIDedBrokenDep)

            if not brokenIDMatch:
                print("Couldn't recover broken ID from:\n" + prevIDedBrokenDep)
                continue

            setOfBrokenIDs.add(brokenIDMatch.group())

        print("Restored " + str(len(prevIDedBrokenDeps)) + " broken dep(s)")

def updateConfig():
    subprocess.run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_DEBUG_INFO"])
    subprocess.run(["./scripts/config", "--enable", "CONFIG_LTO_NONE"])

def main(runs=1):
    print("Restoring broken deps ...")

    restoreBrokenDeps()

    print("Starting LKMM random testing for broken dependencies")

    with open(_BROKEN_DEPS_FILE_PATH, 'a') as brokenDepsFile:
        for _ in range(runs):
            print("Cleaning ...")
            subprocess.run(["make", "mrproper", "-s"])

            print("Generating random config ...")
            configResult = subprocess.run(_MAKEFLAGS_RANDOM_CONFIG,
                                          stdout=subprocess.PIPE, text=True)

            # TODO save seed of config
            seedMatch = seedMatcher.search(configResult.stdout)
            currSeed = "42"
            if seedMatch:
                currSeed = seedMatch.group()
            else:
                print("Couldn't find seed")

            print("Seed: " + currSeed)

            # Adapt random config to requirements of LKMM dep checker
            # updateConfig()

            # Build config
            print("Building ...")
            buildResult = subprocess.run(_MAKEFLAGS_BUILD, stderr=subprocess.PIPE, text=True)

            # Gather results
            newDeps = brokenDepMatcher.findall(buildResult.stderr)

            print("This build produced " + str(len(newDeps)) + " broken dep(s)")
            print("Check if we have seen them before ...")

            # Check if we actually found a new broken dep
            for newDep in newDeps:
                idMatch = brokenIDMatcher.search(newDep)

                if not idMatch:
                    print("Couldn't find an ID in:\n" + newDep)
                    continue

                id = idMatch.group()

                if id in setOfBrokenIDs:
                    continue

                # Add seed 
                lastNewline = newDep.rfind("\n")
                newDep = newDep[:lastNewline] + "\n\nFound with seed: " + currSeed + "\n" + newDep[lastNewline:]

                brokenDepsFile.write(newDep + "\n\n")
                setOfBrokenIDs.add(id)

                print(newDep + "\n\n")

if __name__ == '__main__':
    if len(sys.argv) == 2:
        main(int(sys.argv[1]))
    else:
        main()

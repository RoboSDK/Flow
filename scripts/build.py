#! /usr/bin/env python3

import argparse
import os
import subprocess

"""
Build the flow project
"""


def execute_command(command: list):
    """
    @brief execute the following command

    @param command A comand to build or configure a build
    """
    try:
        process = subprocess.run(command, check=True, universal_newlines=True)

    except FileNotFoundError:
        print(f"Attempted to use the following command: {command[0]}, but it looks like it's not installed!")


def clear_cmake_cache(build_directory: str):
    """
    @brief Clears the cmake cache for the current build path

    Clearing the CMakeCache.txt file will force cmake to reconfigure the project

    @param build_directory The full path to the build directory
    """
    cmake_cache = os.path.join(build_directory, "CMakeCache.txt")
    if not os.path.exists(cmake_cache):
        return

    os.remove(cmake_cache)


def configure(build_directory: str, build_type: str, enable_testing: bool):
    """
    Configure the project using cmake

    @param build_directory:  The full path to the build directory
    @param build_type: Debug, Release, RelWithDebInfo
    @param enable_testing: Whether or not to build tests
    """
    clear_cmake_cache(build_directory)
    os.chdir(build_directory)

    if enable_testing:
        enable_testing_option = "-DENABLE_TESTING=ON"
    else:
        enable_testing_option = "-DENABLE_TESTING=OFF"

    command = ["cmake",
               f"-DCMAKE_BUILD_TYPE={build_type}",
               enable_testing_option,
               ".."]

    execute_command(command)


def build(build_directory: str, num_threads: int, target: str):
    """
    Call make within the build directory and start the build process

    if a target is passed in then, then only that target and its dependencies will be built

    @param build_directory: The full path to the build directory
    @param num_threads: The number of threads to use for the build
    @param target: A specific cmake target to build such as an executable or library (e.g. msdblib or operator_ui)
    """
    os.chdir(build_directory)

    command = ["make"]
    if target is not None:
        command.append(target)

    command.append(f"-j{num_threads}")

    execute_command(command)


def partially_configured(build_directory: str) -> bool:
    """
    Determine if project is not configured enough to begin the build process
    @param build_directory: The full path to the build directory
    @return: Whether or not this needs to be configured from scratch
    """
    cmake_cache = os.path.join(build_directory, "CMakeCache.txt")
    return not os.path.exists(cmake_cache)


def run_tests(build_directory: str, num_threads: int):
    """
    Run ctest command in build directory. Runs tests in random order.

    @param build_directory: The full path to the build
    @param num_threads: The number of tests to execute concurrently
    """
    os.chdir(build_directory)
    command = ["ctest", f"-j{num_threads}", "--schedule-random"]
    execute_command(command)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()

    BUILD_TYPES = ["Debug", "Release", "RelWithDebInfo"]
    DEFAULT_BUILD_TYPE = "Debug"
    parser.add_argument(
        "-bt",
        "--build-type",
        choices=BUILD_TYPES,
        default=DEFAULT_BUILD_TYPE,
        help=f"Set the build type. default: {DEFAULT_BUILD_TYPE}"
    )

    DEFAULT_NUM_THREADS = 8
    parser.add_argument(
        "-j",
        "--num-threads",
        default=DEFAULT_NUM_THREADS,
        help=f"The number of threads to use when building the project. default: {DEFAULT_NUM_THREADS}"
    )

    DEFAULT_BUILD_DIR = "build"
    parser.add_argument(
        "-d",
        "--build-dir",
        default=DEFAULT_BUILD_DIR,
        help="The build directory name where flow is being built. (e.g. build_debug, where build is the name) "
             f"default: {DEFAULT_BUILD_DIR}"
    )

    parser.add_argument(
        "-c",
        "--clear-cache",
        action="store_true",
        default=False,
        help="Force configure the build directory. Note: Make sure to set the proper options, otherwise defaults will "
             "be used "
    )

    parser.add_argument(
        "-e",
        "--enable-testing",
        action="store_true",
        default=False,
        help=f"Build flow with testing enabled.\nIn "
             f"the build directory type in 'ctest -j{DEFAULT_NUM_THREADS}'",
    )

    parser.add_argument(
        "-t",
        "--target",
        default=None,
        help="Build a specific target (e.g. msdblib or operator_ui) Default: all targets"
    )

    options = parser.parse_args()
    options.build_dir = f"{options.build_dir}_{options.build_type}".lower()

    project_path = os.path.dirname(os.path.abspath(os.path.join(__file__, "..")))

    build_path = os.path.join(project_path, options.build_dir)

    if not os.path.exists(build_path):
        os.mkdir(build_path)
        configure(build_directory=build_path, build_type=options.build_type, enable_testing=options.enable_testing)
    elif options.clear_cache or partially_configured(build_path):
        configure(build_directory=build_path, build_type=options.build_type, enable_testing=options.enable_testing)

    build(build_directory=build_path, num_threads=options.num_threads, target=options.target)

    if options.enable_testing:
        run_tests(build_directory=build_path, num_threads=options.num_threads)

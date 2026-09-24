#!/usr/bin/env python3

import concurrent.futures
import os
import platform
import shutil
import subprocess
import sys
import argparse

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("--cx", help="path to cx compiler executable", default="cx")
arg_parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4,
                        help="number of examples to build in parallel (each build spends "
                             "most of its time waiting on compiler/linker subprocesses, "
                             "so matching the core count does not oversubscribe; override as needed)")
args, cx_args = arg_parser.parse_known_args()

os.chdir(os.path.dirname(__file__))
ignored_dirs = ["inputs"]
# The embedding example's entry point is called from the C++ host, so unused
# declarations are expected there; all other warnings are still errors.
no_unused_dirs = ["embedding"]

# The cpp-interop example links a small C++ static library; build it first
# so `cx build` finds libcpp-interop-math.a via build.cx.
def build_cpp_interop_lib():
    # The example is skipped on Windows (see the loop below), so its
    # library is not needed there either.
    if platform.system() == "Windows":
        return
    directory = "cpp-interop"
    if not os.path.isdir(directory):
        return
    cxx = shutil.which("c++") or shutil.which("clang++") or shutil.which("g++")
    if not cxx:
        print("warning: no C++ compiler found, skipping cpp-interop C++ library build")
        return
    for src, obj in [("mathlib.cpp", "mathlib.o"), ("wrapper.cpp", "wrapper.o")]:
        result = subprocess.call([cxx, "-std=c++20", "-O2", "-c", src, "-o", obj], cwd=directory)
        if result != 0:
            sys.exit(result)
    result = subprocess.call(["ar", "rcs", "libcpp-interop-math.a", "mathlib.o", "wrapper.o"], cwd=directory)
    if result != 0:
        sys.exit(result)


build_cpp_interop_lib()

is_windows = platform.system() == "Windows"


def build_example(file):
    # Returns the file on failure, None on success. Each example builds in
    # its own directory with its own output files, so examples are
    # independent and can build in parallel worker threads.
    if is_windows and file in ["tree.cx", "asteroids", "opengl", "voxel-game", "cpp-interop"]:
        return None

    if file.endswith(".cx"):
        output = os.path.splitext(file)[0] + (".exe" if is_windows else "")
        exit_status = subprocess.call([args.cx, file, "-o", output, "-Werror"] + cx_args)
        try:
            os.remove(output)
        except FileNotFoundError:
            pass
        # macOS builds also emit a .dSYM bundle next to the binary.
        shutil.rmtree(output + ".dSYM", ignore_errors=True)
    elif file.endswith(".dSYM"):
        return None
    elif file not in ignored_dirs and os.path.isdir(file):
        extra_args = ["-Wno-unused"] if file in no_unused_dirs else []
        before = set(os.listdir(file))
        exit_status = subprocess.call([args.cx, "build", "-Werror"] + extra_args + cx_args, cwd=file)
        for entry in set(os.listdir(file)) - before:
            path = os.path.join(file, entry)
            if os.path.isdir(path) and not os.path.islink(path):
                shutil.rmtree(path)
            else:
                os.remove(path)
    else:
        return None

    return file if exit_status != 0 else None


with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
    failures = [failure for failure in executor.map(build_example, os.listdir(".")) if failure is not None]

if failures:
    print(f"failed to build: {', '.join(sorted(failures))}")
    sys.exit(1)

# Clean the intermediate C++ objects for cpp-interop (built before the loop,
# so the per-directory before/after cleanup above does not see them).
for artifact in ["mathlib.o", "wrapper.o", "libcpp-interop-math.a"]:
    try:
        os.remove(os.path.join("cpp-interop", artifact))
    except FileNotFoundError:
        pass

print("All examples built successfully.")

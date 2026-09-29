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
# Demoted containers carry their own @test suites; run them with `cx test`
# instead of building them as standalone programs. OrderedSet is
# implemented on top of OrderedMap, so its tests need both files.
container_tests = {
    "OrderedMap.cx": [],
    "OrderedSet.cx": ["OrderedMap.cx"],
    "Queue.cx": [],
}
# The embedding example's entry point is called from the C++ host, so unused
# declarations are expected there; all other warnings are still errors.
no_unused_dirs = ["embedding"]

# Bound every child process: a hung compiler must fail the suite, not block
# CTest forever (this suite has no per-step timeout otherwise).
def _call(cmd, **kwargs):
    try:
        return subprocess.call(cmd, timeout=600, **kwargs)
    except subprocess.TimeoutExpired:
        print(f"timed out: {' '.join(cmd)}")
        return 1


def _find_compiler(compilers):
    for name in compilers:
        path = shutil.which(name)
        if path:
            return path
    return None


# The interop examples link small static libraries; build them first
# so `cx build` finds them via build.cx.
def build_interop_lib(directory, lib, sources, compilers, flags):
    # The examples are skipped on Windows (see the loop below), so their
    # libraries are not needed there either.
    if platform.system() == "Windows":
        return
    if not os.path.isdir(directory):
        return
    cc = _find_compiler(compilers)
    if not cc:
        print(f"warning: no compiler found ({', '.join(compilers)}), skipping {directory} library build")
        return
    objects = []
    for src in sources:
        obj = os.path.splitext(src)[0] + ".o"
        if _call([cc, *flags, "-c", src, "-o", obj], cwd=directory) != 0:
            sys.exit(1)
        objects.append(obj)
    if _call(["ar", "rcs", lib, *objects], cwd=directory) != 0:
        sys.exit(1)


# The interop examples also export cx functions; compile cxlib.cx to an
# object file, link it into the caller's main file, and check the output.
def check_calls_cx(directory, caller, binary, expected, compilers, gen_flags, link_flags, link_suffix=[]):
    if platform.system() == "Windows":
        return
    if not os.path.isdir(directory):
        return
    cc = _find_compiler(compilers)
    if not cc:
        print(f"warning: no compiler found ({', '.join(compilers)}), skipping {directory} caller check")
        return
    if "--backend=c" in cx_args:
        # The C backend's -c output is C source, so compile it to an object
        # instead of linking it directly.
        gen = subprocess.run([args.cx, "cxlib.cx", "--backend=c", "--print-c", "-Werror"],
                             cwd=directory, capture_output=True, text=True, timeout=600)
        if gen.returncode != 0:
            print(gen.stdout)
            print(gen.stderr)
            sys.exit(1)
        with open(os.path.join(directory, "cxlib_gen.c"), "w") as file:
            file.write(gen.stdout)
        steps = [[cc, *gen_flags, "-c", "cxlib_gen.c", "-o", "cxlib.o"]]
    else:
        steps = [[args.cx, "cxlib.cx", "-c", "-o", "cxlib.o", "-Werror"]]
    steps.append([cc, *link_flags, caller, "cxlib.o", "-o", binary, *link_suffix])
    for cmd in steps:
        if _call(cmd, cwd=directory) != 0:
            sys.exit(1)
    try:
        run = subprocess.run([f"./{binary}"], cwd=directory, capture_output=True, text=True, timeout=600)
    except subprocess.TimeoutExpired:
        print(f"timed out: ./{binary}")
        sys.exit(1)
    if run.returncode != 0 or run.stdout != expected:
        print(f"{directory} caller failed: exit {run.returncode}, output {run.stdout!r}")
        sys.exit(1)


C_COMPILERS = ["cc", "clang", "gcc"]
CXX_COMPILERS = ["c++", "clang++", "g++"]

build_interop_lib("c-interop", "libc-interop-math.a", ["mathlib.c"], C_COMPILERS, ["-O2"])
build_interop_lib("cxx-interop", "libcxx-interop-vec.a", ["veclib.cpp"], CXX_COMPILERS, ["-std=c++17", "-O2"])

is_windows = platform.system() == "Windows"


def build_example(file):
    # Returns the file on failure, None on success. Each example builds in
    # its own directory with its own output files, so examples are
    # independent and can build in parallel worker threads.
    if is_windows and file in ["tree.cx", "asteroids", "opengl", "voxel-game", "c-interop", "cxx-interop", "fractal", "boids"]:
        return None

    if file in container_tests:
        # The container's own @test suite runs with any containers it
        # is implemented on top of.
        exit_status = _call([args.cx, "test", *container_tests[file], file, "-Werror"] + cx_args)
    elif file.endswith(".cx"):
        output = os.path.splitext(file)[0] + (".exe" if is_windows else "")
        exit_status = _call([args.cx, file, "-o", output, "-Werror"] + cx_args)
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
        exit_status = _call([args.cx, "build", "-Werror"] + extra_args + cx_args, cwd=file)
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

check_calls_cx("c-interop", "main.c", "c-caller", "1\n5.000000\n6\n", C_COMPILERS, ["-O2"], [], ["-lm"])
check_calls_cx("cxx-interop", "main.cpp", "cxx-caller", "1\n0\n60\n", CXX_COMPILERS, ["-O2", "-x", "c"], ["-std=c++17"])

# Clean the intermediate objects for the interop examples (built before/after
# the loop, so the per-directory before/after cleanup above does not see them).
for directory, artifacts in [
    ("c-interop", ["mathlib.o", "libc-interop-math.a", "cxlib.o", "cxlib_gen.c", "c-caller"]),
    ("cxx-interop", ["veclib.o", "libcxx-interop-vec.a", "cxlib.o", "cxlib_gen.c", "cxx-caller"]),
]:
    for artifact in artifacts:
        try:
            os.remove(os.path.join(directory, artifact))
        except FileNotFoundError:
            pass

print("All examples built successfully.")

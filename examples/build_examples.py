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

# Bound every child process: a hung compiler must fail the suite, not block
# CTest forever (this suite has no per-step timeout otherwise).
def _call(cmd, **kwargs):
    try:
        return subprocess.call(cmd, timeout=600, **kwargs)
    except subprocess.TimeoutExpired:
        print(f"timed out: {' '.join(cmd)}")
        return 1


# The c-interop example links a small C static library; build it first
# so `cx build` finds libc-interop-math.a via build.cx.
def build_c_interop_lib():
    # The example is skipped on Windows (see the loop below), so its
    # library is not needed there either.
    if platform.system() == "Windows":
        return
    directory = "c-interop"
    if not os.path.isdir(directory):
        return
    cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not cc:
        print("warning: no C compiler found, skipping c-interop C library build")
        return
    result = _call([cc, "-O2", "-c", "mathlib.c", "-o", "mathlib.o"], cwd=directory)
    if result != 0:
        sys.exit(result)
    result = _call(["ar", "rcs", "libc-interop-math.a", "mathlib.o"], cwd=directory)
    if result != 0:
        sys.exit(result)


# The c-interop example also exports cx functions to C; compile cxlib.cx to
# an object file, link it into main.c, and check the output.
def check_c_calls_cx():
    if platform.system() == "Windows":
        return
    directory = "c-interop"
    if not os.path.isdir(directory):
        return
    cc = shutil.which("cc") or shutil.which("clang") or shutil.which("gcc")
    if not cc:
        print("warning: no C compiler found, skipping c-interop C caller check")
        return
    if "--backend=c" in cx_args:
        # The C backend's -c output is C source, so compile it to an object
        # with the C compiler instead of linking it directly.
        gen = subprocess.run([args.cx, "cxlib.cx", "--backend=c", "--print-c", "-Werror"],
                             cwd=directory, capture_output=True, text=True, timeout=600)
        if gen.returncode != 0:
            print(gen.stdout)
            print(gen.stderr)
            sys.exit(1)
        with open(os.path.join(directory, "cxlib_gen.c"), "w") as file:
            file.write(gen.stdout)
        steps = [[cc, "-O2", "-c", "cxlib_gen.c", "-o", "cxlib.o"]]
    else:
        steps = [[args.cx, "cxlib.cx", "-c", "-o", "cxlib.o", "-Werror"]]
    steps.append([cc, "main.c", "cxlib.o", "-o", "c-caller", "-lm"])
    for cmd in steps:
        if _call(cmd, cwd=directory) != 0:
            sys.exit(1)
    try:
        run = subprocess.run(["./c-caller"], cwd=directory, capture_output=True, text=True, timeout=600)
    except subprocess.TimeoutExpired:
        print("timed out: ./c-caller")
        sys.exit(1)
    if run.returncode != 0 or run.stdout != "1\n5.000000\n":
        print(f"c-interop C caller failed: exit {run.returncode}, output {run.stdout!r}")
        sys.exit(1)


build_c_interop_lib()

is_windows = platform.system() == "Windows"


def build_example(file):
    # Returns the file on failure, None on success. Each example builds in
    # its own directory with its own output files, so examples are
    # independent and can build in parallel worker threads.
    if is_windows and file in ["tree.cx", "asteroids", "opengl", "voxel-game", "c-interop"]:
        return None

    if file.endswith(".cx"):
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

check_c_calls_cx()

# Clean the intermediate objects for c-interop (built before/after the loop,
# so the per-directory before/after cleanup above does not see them).
for artifact in ["mathlib.o", "libc-interop-math.a", "cxlib.o", "cxlib_gen.c", "c-caller"]:
    try:
        os.remove(os.path.join("c-interop", artifact))
    except FileNotFoundError:
        pass

print("All examples built successfully.")

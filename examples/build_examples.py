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

is_windows = platform.system() == "Windows"

# Built binaries are smoke-tested and deleted: skip debug-info work
# (macOS dsymutil collection, cc -g) via the driver's harness opt-out.
# Subprocesses inherit this environment.
os.environ["CX_SKIP_DSYMUTIL"] = "1"

os.chdir(os.path.dirname(__file__))
ignored_dirs = ["inputs", "__pycache__"]
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
# GUI examples link third-party libraries on Windows (the vendor build files
# name the import libraries; pkg-config is not used there). An example is
# skipped with a note when any of its libraries is not found, like lit's
# REQUIRES.
windows_lib_requirements = {
    "asteroids": ["SDL3.lib"],
    "boids": ["SDL3.lib"],
    "opengl": ["glfw3dll.lib"],
    "voxel-game": ["glfw3dll.lib"],
    "fractal": ["glfw3dll.lib"],
}

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


def _windows_lib_search_dirs():
    dirs = []
    # Explicit -L flags, joined or separate. They are passed through to cx
    # as part of cx_args, so finding the library here also links it.
    for index, arg in enumerate(cx_args):
        if arg.startswith("-L") and len(arg) > 2:
            dirs.append(arg[2:])
        elif arg == "-L" and index + 1 < len(cx_args):
            dirs.append(cx_args[index + 1])
    for var in ("VCPKG_INSTALLATION_ROOT", "VCPKG_ROOT"):
        root = os.environ.get(var)
        if root:
            dirs.append(os.path.join(root, "installed", "x64-windows", "lib"))
    # Documented in asteroids/README.md for manual SDL3 installs.
    sdl_dir = os.environ.get("SDLDIR")
    if sdl_dir:
        dirs.append(os.path.join(sdl_dir, "lib", "x64"))
    lib_env = os.environ.get("LIB")
    if lib_env:
        dirs.extend(lib_env.split(os.pathsep))
    return dirs


def _find_windows_lib(filename):
    for directory in _windows_lib_search_dirs():
        if directory and os.path.isfile(os.path.join(directory, filename)):
            return directory
    return None


# GNU->MSVC translations for the interop builds below; None drops the flag
# (-lm has no Windows counterpart, the C runtime already provides it).
_MSVC_FLAG_MAP = {"-O2": "/O2", "-std=c++17": "/std:c++17", "-lm": None}


def _msvc_flags(flags):
    mapped = []
    skip_next = False
    for flag in flags:
        if skip_next:
            skip_next = False
            continue
        if flag == "-x":
            # Language selector; cl picks the language from the file
            # extension, so the flag and its argument are dropped.
            skip_next = True
            continue
        mapped_flag = _MSVC_FLAG_MAP.get(flag, flag)
        if mapped_flag is not None:
            mapped.append(mapped_flag)
    return mapped


# The interop examples link small static libraries; build them first
# so `cx build` finds them via build.cx.
def build_interop_lib(directory, lib, sources, compilers, flags):
    if not os.path.isdir(directory):
        return
    if is_windows:
        # build.cx names the bare library, which the driver maps to
        # <name>.lib on Windows, so build that instead of lib<name>.a.
        # cl compiles C and C++ by file extension; lib archives the objects.
        if lib.startswith("lib") and lib.endswith(".a"):
            lib = lib[len("lib"):-len(".a")] + ".lib"
        cc = _find_compiler(["cl"])
        lib_exe = shutil.which("lib")
        if not cc or not lib_exe:
            print(f"warning: no MSVC compiler found (cl, lib), skipping {directory} library build")
            return
        objects = []
        for src in sources:
            obj = os.path.splitext(src)[0] + ".obj"
            if _call([cc, "/nologo", "/c", src, "/Fo" + obj, *_msvc_flags(flags)], cwd=directory) != 0:
                sys.exit(1)
            objects.append(obj)
        if _call([lib_exe, "/nologo", "/OUT:" + lib, *objects], cwd=directory) != 0:
            sys.exit(1)
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
    if not os.path.isdir(directory):
        return
    if is_windows:
        cc = _find_compiler(["cl"])
        if not cc:
            print(f"warning: no MSVC compiler found (cl), skipping {directory} caller check")
            return
        cxlib_obj = "cxlib.obj"
        exe = binary + ".exe"
        if "--backend=c" in cx_args:
            # The C backend's -c output is C source, so compile it to an
            # object instead of linking it directly.
            gen = subprocess.run([args.cx, "cxlib.cx", "--backend=c", "--print-c", "-Werror"],
                                 cwd=directory, capture_output=True, text=True, timeout=600)
            if gen.returncode != 0:
                print(gen.stdout)
                print(gen.stderr)
                sys.exit(1)
            with open(os.path.join(directory, "cxlib_gen.c"), "w") as file:
                file.write(gen.stdout)
            steps = [[cc, "/nologo", "/c", "cxlib_gen.c", "/Fo" + cxlib_obj, *_msvc_flags(gen_flags)]]
        else:
            steps = [[args.cx, "cxlib.cx", "-c", "-o", cxlib_obj, "-Werror"]]
        steps.append([cc, "/nologo", caller, cxlib_obj, "/Fe:" + exe, *_msvc_flags(link_flags), *_msvc_flags(link_suffix)])
        for cmd in steps:
            if _call(cmd, cwd=directory) != 0:
                sys.exit(1)
        # Absolute path: CreateProcess resolves relative executable paths
        # (including ./-prefixed ones) against the parent's directory,
        # not against cwd=.
        exe_path = os.path.abspath(os.path.join(directory, exe))
        try:
            run = subprocess.run([exe_path], cwd=directory, capture_output=True, text=True, timeout=600)
        except subprocess.TimeoutExpired:
            print(f"timed out: {exe}")
            sys.exit(1)
        if run.returncode != 0 or run.stdout != expected:
            print(f"{directory} caller failed: exit {run.returncode}, output {run.stdout!r}")
            sys.exit(1)
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


def build_example(file):
    # Returns the file on failure, None on success. Each example builds in
    # its own directory with its own output files, so examples are
    # independent and can build in parallel worker threads.
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
        if is_windows and file in windows_lib_requirements:
            lib_dirs = []
            for lib in windows_lib_requirements[file]:
                lib_dir = _find_windows_lib(lib)
                if lib_dir is None:
                    print(f"skipping {file} on Windows "
                          f"({lib} not found; install SDL3/GLFW or pass -L<dir>)")
                    return None
                if lib_dir not in lib_dirs:
                    lib_dirs.append(lib_dir)
            extra_args = extra_args + ["-L" + lib_dir for lib_dir in lib_dirs]
        if is_windows and "--backend=c" in cx_args and file == "cxx-interop":
            # The C backend emits the imported C++ function under its
            # MSVC-mangled name (?accum_add@@...), which is not valid C,
            # so this configuration cannot build on Windows.
            print("skipping cxx-interop --backend=c build on Windows "
                  "(the C backend cannot name MSVC-mangled C++ symbols)")
            return None
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
if is_windows:
    # extern "C++" uses the Itanium ABI, which is not supported on MSVC
    # targets, so cxlib.cx exports nothing there (see its #if); the
    # C++-from-cx direction above is still built and checked.
    print("skipping cxx-interop caller check on Windows (extern \"C++\" is not supported on MSVC targets)")
else:
    check_calls_cx("cxx-interop", "main.cpp", "cxx-caller", "1\n0\n", CXX_COMPILERS, ["-O2", "-x", "c"], ["-std=c++17"])

# Clean the intermediate objects for the interop examples (built before/after
# the loop, so the per-directory before/after cleanup above does not see them).
if is_windows:
    # cl leaves the caller's object next to the binary; the link may also
    # leave an incremental-link database.
    interop_artifacts = [
        ("c-interop", ["mathlib.obj", "c-interop-math.lib", "cxlib.obj", "cxlib_gen.c", "c-caller.exe", "c-caller.ilk", "main.obj"]),
        ("cxx-interop", ["veclib.obj", "cxx-interop-vec.lib", "cxlib.obj", "cxlib_gen.c", "cxx-caller.exe", "cxx-caller.ilk", "main.obj"]),
    ]
else:
    interop_artifacts = [
        ("c-interop", ["mathlib.o", "libc-interop-math.a", "cxlib.o", "cxlib_gen.c", "c-caller"]),
        ("cxx-interop", ["veclib.o", "libcxx-interop-vec.a", "cxlib.o", "cxlib_gen.c", "cxx-caller"]),
    ]
for directory, artifacts in interop_artifacts:
    for artifact in artifacts:
        try:
            os.remove(os.path.join(directory, artifact))
        except FileNotFoundError:
            pass

print("All examples built successfully.")

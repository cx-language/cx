#!/usr/bin/env python3

import os
import platform
import shutil
import subprocess
import sys
import argparse

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("--cx", help="path to cx compiler executable", default="cx")
args, cx_args = arg_parser.parse_known_args()

os.chdir(os.path.dirname(__file__))
ignored_dirs = ["inputs"]
# The embedding example's entry point is called from the C++ host, so unused
# declarations are expected there; all other warnings are still errors.
no_unused_dirs = ["embedding"]

for file in os.listdir("."):
    if platform.system() == "Windows" and file in ["tree.cx", "asteroids", "opengl", "voxel-game"]:
        continue

    if file.endswith(".cx"):
        output = os.path.splitext(file)[0] + (".exe" if platform.system() == "Windows" else "")
        exit_status = subprocess.call([args.cx, file, "-o", output, "-Werror"] + cx_args)
        os.remove(output)
        # macOS builds also emit a .dSYM bundle next to the binary.
        shutil.rmtree(output + ".dSYM", ignore_errors=True)
    elif file.endswith(".dSYM"):
        continue
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
        continue

    if exit_status != 0:
        sys.exit(1)

print("All examples built successfully.")

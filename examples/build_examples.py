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
ignored_dirs = ["inputs", "embedding"]  # TODO: Enable 'embedding' example once it works.

for file in os.listdir("."):
    if platform.system() == "Windows" and file in ["tree.cx", "asteroids", "opengl"]:
        continue

    if file.endswith(".cx"):
        output = os.path.splitext(file)[0] + (".exe" if platform.system() == "Windows" else "")
        exit_status = subprocess.call([args.cx, file, "-o", output, "-Werror"] + cx_args)
        os.remove(output)
    elif file not in ignored_dirs and os.path.isdir(file):
        exit_status = subprocess.call([args.cx, "build"] + cx_args, cwd=file)
        bin_dir = os.path.join(file, "bin")
        if os.path.isdir(bin_dir):
            shutil.rmtree(bin_dir)
    else:
        continue

    if exit_status != 0:
        sys.exit(1)

print("All examples built successfully.")

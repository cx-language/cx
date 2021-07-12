#!/usr/bin/env python3

import os
import platform
import subprocess
import sys

cx_path = sys.argv[1] if len(sys.argv) > 1 else "cx"

os.chdir(os.path.dirname(__file__))

for file in os.listdir("."):
    if platform.system() == "Windows" and file in ["tree.cx", "asteroids", "opengl"]:
        continue

    if file.endswith(".cx"):
        output = os.path.splitext(file)[0] + (".exe" if platform.system() == "Windows" else "")
        exit_status = subprocess.call([cx_path, file, "-o", output, "-Werror"])
        os.remove(output)
    elif file != "inputs" and os.path.isdir(file):
        env = os.environ.copy()
        env["PATH"] = os.path.dirname(cx_path) + ":" + env["PATH"]
        exit_status = subprocess.call(["make", "-C", file], env=env)
    else:
        continue

    if exit_status != 0:
        sys.exit(1)

print("All examples built successfully.")

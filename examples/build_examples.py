#!/usr/bin/env python

import glob
import os
import platform
import subprocess
import sys

cx_path = sys.argv[1] if len(sys.argv) > 1 else "cx"

os.chdir(os.path.dirname(__file__))

for file in glob.glob("*.cx"):
    if platform.system() == "Windows" and file == "tree.cx":
        continue

    extension = ".out" if platform.system() != "Windows" else ".exe"
    output = os.path.splitext(file)[0] + extension
    exit_status = subprocess.call([cx_path, file, "-o", output])
    if exit_status != 0:
        sys.exit(1)

print("All examples built successfully.")

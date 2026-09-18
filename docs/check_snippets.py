#!/usr/bin/env python3

import argparse
import os
import platform
import re
import subprocess
import sys
import tempfile

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("--cx", help="path to cx compiler executable", default="cx")
args, cx_args = arg_parser.parse_known_args()

book_dir = os.path.join(os.path.dirname(__file__), "book")
failures = []


def check_snippet(path, index, code):
    name = f"{os.path.splitext(os.path.basename(path))[0]}-{index}"
    with tempfile.TemporaryDirectory(prefix="cx-snippet-") as directory:
        with open(os.path.join(directory, "main.cx"), "w") as file:
            file.write(code)

        output = "main" + (".exe" if platform.system() == "Windows" else "")
        compile = subprocess.run([args.cx, "main.cx", "-o", output, "-Werror"] + cx_args,
                                 capture_output=True, text=True, timeout=180, cwd=directory)
        if compile.returncode != 0 or not os.path.exists(os.path.join(directory, output)):
            failures.append(name)
            print(f"FAIL: {name} does not compile warning-free:")
            print(compile.stderr or compile.stdout)
            return

        try:
            run = subprocess.run(os.path.join(".", output), capture_output=True, text=True, timeout=30, cwd=directory)
        except subprocess.TimeoutExpired:
            failures.append(name)
            print(f"FAIL: {name} timed out")
            return

        if run.returncode != 0:
            failures.append(name)
            print(f"FAIL: {name} exited with status {run.returncode}:")
            print(run.stdout)
            print(run.stderr)


for filename in sorted(os.listdir(book_dir)):
    if not filename.endswith(".md"):
        continue

    with open(os.path.join(book_dir, filename)) as file:
        for index, code in enumerate(re.findall(r"^```cs\n(.*?)^```", file.read(), re.M | re.S)):
            check_snippet(filename, index, code)

if failures:
    sys.exit(1)

print("All documentation snippets compiled and ran successfully.")

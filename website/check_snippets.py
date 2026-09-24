#!/usr/bin/env python3

import argparse
import concurrent.futures
import os
import platform
import re
import subprocess
import sys
import tempfile
import threading

arg_parser = argparse.ArgumentParser()
arg_parser.add_argument("--cx", help="path to cx compiler executable", default="cx")
arg_parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4,
                        help="number of snippets to compile and run in parallel")
args, cx_args = arg_parser.parse_known_args()

docs_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "docs"))
failures = []
failures_lock = threading.Lock()
print_lock = threading.Lock()


def check_snippet(path, index, code, reference_only):
    name = f"{os.path.splitext(os.path.basename(path))[0]}-{index}"
    if reference_only:
        code += "\nvoid main() {}\n"
    # Each snippet compiles in a private temp directory, so snippets are
    # independent and can run in parallel worker threads.
    with tempfile.TemporaryDirectory(prefix="cx-snippet-") as directory:
        with open(os.path.join(directory, "main.cx"), "w") as file:
            file.write(code)

        output = "main" + (".exe" if platform.system() == "Windows" else "")
        compile = subprocess.run([args.cx, "main.cx", "-o", output, "-Werror"] + cx_args,
                                 capture_output=True, text=True, timeout=180, cwd=directory)
        if compile.returncode != 0 or not os.path.exists(os.path.join(directory, output)):
            with failures_lock:
                failures.append(name)
            with print_lock:
                print(f"FAIL: {name} does not compile warning-free:")
                print(compile.stderr or compile.stdout)
            return

        try:
            # Absolute path: on Windows the executable resolves against the
            # parent's directory, not cwd, so ./output is not found.
            run = subprocess.run([os.path.join(directory, output)], capture_output=True, text=True, timeout=30, cwd=directory)
        except subprocess.TimeoutExpired:
            with failures_lock:
                failures.append(name)
            with print_lock:
                print(f"FAIL: {name} timed out")
            return

        if run.returncode != 0:
            with failures_lock:
                failures.append(name)
            with print_lock:
                print(f"FAIL: {name} exited with status {run.returncode}:")
                print(run.stdout)
                print(run.stderr)


snippets = []
for filename in sorted(os.listdir(docs_dir)):
    if not filename.endswith(".md"):
        continue

    with open(os.path.join(docs_dir, filename)) as file:
        blocks = re.findall(r"^```cs( \{\.noRun\})?\n(.*?)^```", file.read(), re.M | re.S)
        for index, (marker, code) in enumerate(blocks):
            snippets.append((filename, index, code, bool(marker)))

with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
    list(executor.map(lambda snippet: check_snippet(*snippet), snippets))

if failures:
    sys.exit(1)

print("All documentation snippets compiled and ran successfully.")

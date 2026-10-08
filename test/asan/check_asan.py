#!/usr/bin/env python3
"""AddressSanitizer gate for the compiler and language server.

Runs the ASan-instrumented cx driver over an AST-node-heavy fixture (which
implicitly imports std) and repeated LSP analyses with arena resets
(cx-lsp --leak-check), failing on any sanitizer error. Leak detection stays
off: the LSan gate (check_lsan.py) owns the arena no-malloc contract; this
gate owns memory errors.

Usage:
    test/asan/check_asan.py --cx <path> --cx-lsp <path>

Requires an ASan-instrumented build: cmake -DCX_SANITIZE_ADDRESS=ON with a
Clang that supports -fsanitize=address.
"""

import argparse
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
FIXTURE = ROOT / "test" / "lsp" / "inputs" / "lsan-fixture.cx"


def run(label, command, cwd, strict_exit, must_contain=None):
    """Run one gate step; return an error message or None."""
    existing = os.environ.get("ASAN_OPTIONS", "")
    asan_options = f"{existing}:detect_leaks=0" if existing else "detect_leaks=0"
    proc = subprocess.run(
        [str(c) for c in command],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=600,
        cwd=cwd,
        env=os.environ | {"ASAN_OPTIONS": asan_options},
    )
    output = proc.stdout.decode("utf-8", errors="replace")
    if "AddressSanitizer" in output:
        return f"{label} reported a sanitizer error:\n{output[-3000:]}"
    if proc.returncode < 0:
        return f"{label} crashed with signal {-proc.returncode}:\n{output[-3000:]}"
    if "error:" in output:
        return f"{label} reported errors:\n{output[-3000:]}"
    if must_contain is not None and must_contain not in output:
        return f"{label} ran but produced no expected output:\n{output[-3000:]}"
    # The fixture's exit status is the compiled program's own (nonzero), so
    # only tool steps require a zero exit.
    if strict_exit and proc.returncode != 0:
        return f"{label} exited with status {proc.returncode}:\n{output[-3000:]}"
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cx", required=True)
    parser.add_argument("--cx-lsp", required=True)
    args = parser.parse_args()

    # Steps run with cwd set to the temp dir, so resolve the (possibly
    # relative) tool paths up front; otherwise the child never starts.
    cx = pathlib.Path(args.cx).resolve()
    cx_lsp = pathlib.Path(args.cx_lsp).resolve()
    for label, path in (("cx", cx), ("cx-lsp", cx_lsp)):
        if not path.is_file():
            print(f"error: no such binary for --{label}: {path}", file=sys.stderr)
            return 1

    failures = []
    with tempfile.TemporaryDirectory() as directory:
        fixture = pathlib.Path(directory) / "asan-fixture.cx"
        shutil.copy(FIXTURE, fixture)
        # The marker proves compile+run happened (a no-op driver must not pass).
        error = run("cx run", [cx, "run", fixture], directory, False, "hello world")
        if error:
            failures.append(error)
        # Five iterations exercise repeated arena resets; LSan's 64 measures growth.
        error = run("cx-lsp --leak-check", [cx_lsp, "--leak-check", fixture, "5"], directory, True)
        if error:
            failures.append(error)

    for failure in failures:
        print(f"FAIL: {failure}\n", file=sys.stderr)
    if failures:
        return 1
    print("ASan gate passed: no sanitizer errors in driver or LSP analyses.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

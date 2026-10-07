#!/usr/bin/env python3
"""LeakSanitizer gate for the AST arena no-malloc contract (src/ast/arena.h).

AST nodes live in the arena and their destructors never run, so no node may
own malloc'd memory. This runs the 1-vs-N `--leak-check` comparison: repeated
LSP analyses of one fixture through a single session, dropped before exit.
Per-reset leaks (node-owned malloc with no one to free it) grow with N;
one-time allocations cancel out in the diff.

Usage:
    test/lsp/check_lsan.py --cx-lsp <path> [--count N]
        [--max-growth-bytes B] [--max-growth-allocs A]

Requires an LSan-instrumented build: cmake -DCX_SANITIZE_LEAK=ON with a Clang
that supports -fsanitize=leak (Apple Clang lacks it; use LLVM Clang or Linux).
"""

import argparse
import json
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
FIXTURE = pathlib.Path(__file__).resolve().parent / "inputs" / "lsan-fixture.cx"

SUMMARY_RE = re.compile(r"(\d+) bytes?(?:\(s\))? leaked in (\d+) allocation")


def run_leak_check(cx_lsp, path, count):
    """Return (stdout, stderr, returncode) for one --leak-check run."""
    proc = subprocess.run(
        [str(cx_lsp), "--leak-check", str(path), str(count)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=600,
    )
    return proc.stdout.decode(), proc.stderr.decode(), proc.returncode


def parse_summary(stderr):
    """Return (bytes, allocs) from the LSan summary, or (0, 0) when clean."""
    match = SUMMARY_RE.search(stderr)
    if match is None:
        return 0, 0
    return int(match.group(1)), int(match.group(2))


def check_fixture_clean(cx_lsp, path):
    """Fail when the fixture itself reports diagnostics.

    A rotting fixture could take error paths that leak (or skip work and
    silently weaken the gate), so it must analyze cleanly.
    """
    query = {
        "method": "check",
        "file": str(path),
        "content": path.read_text(),
        "openDocs": {},
        "workspaceFolders": [],
        "importSearchPaths": [],
        "defines": [],
    }
    proc = subprocess.run(
        [str(cx_lsp), "--query"],
        input=json.dumps(query).encode(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=600,
    )
    envelope = json.loads(proc.stdout.decode())
    if not envelope.get("ok"):
        print(f"fixture query failed: {envelope.get('error')}")
        return False
    diagnostics = envelope.get("result", {}).get("diagnostics", [])
    if diagnostics:
        print(f"fixture {path.name} reports diagnostics:")
        for diag in diagnostics[:5]:
            print(f"  {diag.get('message', diag)}")
        return False
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--cx-lsp", required=True, help="LSan-instrumented cx-lsp binary")
    parser.add_argument("--count", type=int, default=64, help="analyses in the N run (default: 64)")
    parser.add_argument("--max-growth-bytes", type=int, default=1024, help="allowed N-vs-1 byte growth")
    parser.add_argument("--max-growth-allocs", type=int, default=16, help="allowed N-vs-1 allocation growth")
    args = parser.parse_args()
    if args.count < 2:
        parser.error("--count must be at least 2 (the gate compares 1-vs-N runs)")

    cx_lsp = pathlib.Path(args.cx_lsp)
    if not cx_lsp.is_file():
        print(f"error: no such binary: {cx_lsp}")
        return 1

    # Isolate the fixture in a temp dir: sibling .cx files or build.cx files
    # next to it would join the analysis and make totals layout-dependent.
    with tempfile.TemporaryDirectory(prefix="cx-lsan-") as tmp:
        path = pathlib.Path(tmp) / FIXTURE.name
        shutil.copy(FIXTURE, path)

        if not check_fixture_clean(cx_lsp, path):
            return 1

        results = {}
        for count in (1, args.count):
            out, err, code = run_leak_check(cx_lsp, path, count)
            if out:
                print(f"--leak-check {count} failed:\n{out}")
                return 1
            if code not in (0, 23):  # 23 is LSan's leaks-reported exit code.
                print(f"--leak-check {count} exited with code {code}")
                print(err[-2000:])
                return 1
            results[count] = parse_summary(err)

    bytes1, allocs1 = results[1]
    bytesN, allocsN = results[args.count]
    growth_bytes = bytesN - bytes1
    growth_allocs = allocsN - allocs1
    print(f"1 run:  {bytes1} bytes in {allocs1} allocations")
    print(f"{args.count} runs: {bytesN} bytes in {allocsN} allocations")
    print(f"growth: {growth_bytes} bytes in {growth_allocs} allocations")
    if growth_bytes > args.max_growth_bytes or growth_allocs > args.max_growth_allocs:
        print(
            f"FAIL: per-reset growth exceeds limits "
            f"({args.max_growth_bytes} bytes, {args.max_growth_allocs} allocations); "
            f"AST nodes likely own malloc'd memory (see src/ast/arena.h)"
        )
        return 1
    print("PASS: no per-reset growth")
    return 0


if __name__ == "__main__":
    sys.exit(main())

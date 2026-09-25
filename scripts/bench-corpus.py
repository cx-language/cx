#!/usr/bin/env python3
"""Compile and run the bench corpus, emit one JSON metrics record.

Usage:
    scripts/bench-corpus.py --cx build/cx [--runs 5] [--compile-runs 3]
        [--build-seconds F] [--check-seconds F] [--sha SHA] [--output bench.json]

Each corpus program is compiled --compile-runs times (median kept) and run
--runs times (median kept). All runs of a program must print identical
output, otherwise the benchmark is meaningless and this exits nonzero.
"""

import argparse
import datetime
import json
import os
import platform
import statistics
import subprocess
import sys
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CORPUS = ["sieve", "mandelbrot", "fib", "wordcount", "mapfilter", "jsonparse"]
TIMEOUT = 600


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cx", required=True, help="path to cx compiler executable")
    parser.add_argument("--runs", type=int, default=5, help="runs per program (median kept)")
    parser.add_argument("--compile-runs", type=int, default=3, help="compiles per program (median kept)")
    parser.add_argument("--build-seconds", type=float, default=None, help="C++ build wall time from CI")
    parser.add_argument("--check-seconds", type=float, default=None, help="test suite wall time from CI")
    parser.add_argument("--sha", default=None, help="commit sha (defaults to GITHUB_SHA or git HEAD)")
    parser.add_argument("--output", default="bench.json", help="where to write the JSON record")
    return parser.parse_args()


def run_timed(cmd, **kwargs):
    start = time.perf_counter()
    try:
        result = subprocess.run(cmd, timeout=TIMEOUT, **kwargs)
    except subprocess.TimeoutExpired:
        print(f"timed out: {' '.join(cmd)}")
        sys.exit(1)
    return time.perf_counter() - start, result


def resolve_sha(explicit):
    if explicit:
        return explicit
    if os.environ.get("GITHUB_SHA"):
        return os.environ["GITHUB_SHA"]
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True, check=False
    )
    return result.stdout.strip() or "unknown"


def main():
    args = parse_args()
    args.runs = max(1, args.runs)
    args.compile_runs = max(1, args.compile_runs)
    suffix = ".exe" if platform.system() == "Windows" else ""
    compile_s, run_s, bench_bytes = {}, {}, {}

    with tempfile.TemporaryDirectory(prefix="cx-bench-") as workdir:
        for name in CORPUS:
            src = os.path.join(ROOT, "bench", name + ".cx")
            binary = os.path.join(workdir, name + suffix)
            compile_cmd = [args.cx, src, "-o", binary, "--release", "-Werror"]
            times = []
            for _ in range(args.compile_runs):
                elapsed, result = run_timed(compile_cmd, capture_output=True, text=True)
                if result.returncode != 0:
                    print(f"failed to compile {src}:\n{result.stderr}")
                    sys.exit(1)
                times.append(elapsed)
            compile_s[name] = statistics.median(times)
            bench_bytes[name] = os.path.getsize(binary)

            times, outputs = [], set()
            for _ in range(args.runs):
                elapsed, result = run_timed([binary], capture_output=True)
                if result.returncode != 0:
                    print(f"{name} exited with status {result.returncode}")
                    sys.exit(1)
                times.append(elapsed)
                outputs.add(result.stdout)
            if len(outputs) != 1:
                print(f"{name} printed {len(outputs)} distinct outputs across runs, benchmark invalid")
                sys.exit(1)
            run_s[name] = statistics.median(times)
            print(f"{name}: compile {compile_s[name]:.3f}s run {run_s[name]:.3f}s")

    record = {
        "sha": resolve_sha(args.sha),
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "metrics": {
            "cxx_build_s": args.build_seconds,
            "check_s": args.check_seconds,
            "compile_s": compile_s,
            "run_s": run_s,
            "cx_bytes": os.path.getsize(args.cx),
            "bench_bytes": bench_bytes,
        },
    }
    with open(args.output, "w") as file:
        json.dump(record, file)
        file.write("\n")
    print(f"wrote {args.output}")


if __name__ == "__main__":
    main()

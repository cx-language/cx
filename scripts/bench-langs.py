#!/usr/bin/env python3
"""Run-time benchmark: cx vs C, C++, Rust and Go on the bench corpus subset.

Usage:
    scripts/bench-langs.py --cx build/cx [--runs 3] [--programs fib,sieve]
        [--languages cx,c] [--output bench/langs/results.json]
        [--html bench/langs/report.html]

Builds each program for each language with release optimizations, runs each
binary --runs times (median kept), and writes a JSON record plus a
self-contained HTML report with graphs (open it straight from disk).

fib/sieve/wordcount/mapfilter must print EXPECTED exactly in every language
that implements them; mandelbrot checks self-consistency only (float-to-int
conversion of huge values is platform-defined, so its checksum legitimately
differs). mapfilter is omitted for C and Go, which have no lambdas.
"""

import argparse
import datetime
import html
import json
import os
import platform
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROGRAMS = ["fib", "sieve", "mandelbrot", "wordcount", "mapfilter"]
EXPECTED = {
    "fib": "102334155",
    "sieve": "5761455",
    "wordcount": "1028015628923350",
    "mapfilter": "164571298857200",
}
TIMEOUT = 600

LANGS = {
    "cx": {"label": "cx", "color": "var(--cx-bar)", "tool": None, "ext": ".cx", "dir": "bench"},
    "c": {"label": "C", "color": "#555555", "tool": "cc", "ext": ".c", "dir": "bench/langs/c"},
    "cxx": {"label": "C++", "color": "#f34b7d", "tool": "c++", "ext": ".cpp", "dir": "bench/langs/cxx"},
    "rust": {"label": "Rust", "color": "#dea584", "tool": "rustc", "ext": ".rs", "dir": "bench/langs/rust"},
    "go": {"label": "Go", "color": "#00ADD8", "tool": "go", "ext": ".go", "dir": "bench/langs/go"},
}


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cx", required=True, help="path to cx compiler executable")
    parser.add_argument("--runs", type=int, default=3, help="runs per binary (median kept)")
    parser.add_argument("--programs", default=",".join(PROGRAMS), help="comma-separated subset")
    parser.add_argument("--languages", default=",".join(LANGS), help="comma-separated subset")
    parser.add_argument("--output", default="bench/langs/results.json", help="where to write the JSON record")
    parser.add_argument("--html", default="bench/langs/report.html", help="where to write the HTML report")
    return parser.parse_args()


def run_timed(cmd, **kwargs):
    start = time.perf_counter()
    try:
        result = subprocess.run(cmd, timeout=TIMEOUT, **kwargs)
    except subprocess.TimeoutExpired:
        print(f"timed out: {' '.join(cmd)}")
        sys.exit(1)
    return time.perf_counter() - start, result


def build_command(lang, cx, src, binary):
    if lang == "cx":
        return [cx, src, "-o", binary, "--release", "-Werror"]
    if lang == "c":
        return ["cc", "-O3", "-fwrapv", "-std=c11", "-o", binary, src]
    if lang == "cxx":
        return ["c++", "-O3", "-fwrapv", "-std=c++20", "-o", binary, src]
    if lang == "rust":
        return ["rustc", "--edition=2021", "-C", "opt-level=3", "-o", binary, src]
    if lang == "go":
        return ["go", "build", "-o", binary, src]
    raise AssertionError("unknown language " + lang)


def tool_version(tool):
    cmd = [tool, "version"] if tool == "go" else [tool, "--version"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
        lines = result.stdout.splitlines()
        return lines[0].strip() if result.returncode == 0 and lines else "unknown"
    except (OSError, subprocess.TimeoutExpired):
        return "missing"


def cx_sha(cx):
    try:
        result = subprocess.run([cx, "--version"], capture_output=True, text=True, timeout=60)
        for line in result.stdout.splitlines():
            if line.startswith("cx commit:"):
                return line.split(":", 1)[1].strip()
    except (OSError, subprocess.TimeoutExpired):
        pass
    fallback = subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True).stdout.strip()
    return fallback or "unknown"


def format_seconds(seconds):
    if seconds < 1:
        return f"{seconds * 1000:.0f} ms"
    return f"{seconds:.3f} s"


def chart_svg(medians):
    # Horizontal bars, fastest first. medians: {lang: seconds}.
    ordered = sorted(medians.items(), key=lambda item: item[1])
    fastest = ordered[0][1]
    row_h, label_w, value_w, width = 30, 64, 150, 760
    bar_w = width - label_w - value_w
    rows = []
    for i, (lang, seconds) in enumerate(ordered):
        y = i * row_h
        length = max(2, seconds / max(medians.values()) * bar_w)
        color = LANGS[lang]["color"]
        rows.append(
            f'<text x="0" y="{y + 20}" class="lang">{LANGS[lang]["label"]}</text>'
            f'<rect x="{label_w}" y="{y + 6}" width="{length:.1f}" height="18" style="fill:{color}"/>'
            f'<text x="{label_w + length + 8:.1f}" y="{y + 20}" class="value">{format_seconds(seconds)}'
            f" ({seconds / fastest:.2f}x)</text>"
        )
    height = len(ordered) * row_h + 6
    return f'<svg viewBox="0 0 {width} {height}" width="100%" role="img">{"".join(rows)}</svg>'


def render_html(record):
    programs = []
    for program in record["program_order"]:
        medians = {lang: data["median_s"] for lang, data in record["programs"][program].items()}
        if not medians:
            programs.append(f"<h2>{program}</h2>\n<p>no successful runs</p>")
            continue
        note = (
            ' <span class="note">checksums differ by design here (float-to-int conversion is '
            "platform-defined); times remain comparable.</span>"
            if program == "mandelbrot"
            else ""
        )
        programs.append(f"<h2>{program}{note}</h2>\n{chart_svg(medians)}")
    tools = " · ".join(f"{lang}: {html.escape(version)}" for lang, version in record["tools"].items())
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="color-scheme" content="light dark">
<title>cx vs C, C++, Rust, Go: run-time comparison</title>
<style>
:root {{ color-scheme: light dark; --bg: #ffffff; --fg: #000000; --muted: #444444; --pre-bg: #f4f4f4; --cx-bar: #000000; }}
@media (prefers-color-scheme: dark) {{
  :root {{ --bg: #1a1a1a; --fg: #e8e8e8; --muted: #aaaaaa; --pre-bg: #2a2a2a; --cx-bar: #ffffff; }}
}}
body {{ font-family: system-ui, -apple-system, sans-serif; max-width: 900px; margin: 2rem auto; padding: 0 1rem; background: var(--bg); color: var(--fg); }}
.meta {{ color: var(--muted); }}
h2 {{ margin-top: 2rem; font-size: 1.2rem; }}
.note {{ font-weight: normal; font-size: 0.85rem; color: var(--muted); }}
svg text {{ font-size: 14px; fill: var(--fg); }}
svg text.value {{ font-variant-numeric: tabular-nums; }}
details {{ margin-top: 2rem; }}
pre {{ background: var(--pre-bg); padding: 1rem; overflow-x: auto; }}
</style>
</head>
<body>
<h1>cx vs C, C++, Rust, Go: run-time comparison</h1>
<p class="meta">{html.escape(record["timestamp"])} · {html.escape(record["platform"])} · median of {record["runs"]} runs<br>{tools}<br>cx at {html.escape(record["cx_sha"])}</p>
{"".join(programs)}
<details><summary>Raw data</summary><pre>{html.escape(json.dumps(record, indent=2))}</pre></details>
</body>
</html>
"""


def main():
    args = parse_args()
    args.runs = max(1, args.runs)
    programs = [p.strip() for p in args.programs.split(",") if p.strip() in PROGRAMS]
    languages = [lang.strip() for lang in args.languages.split(",") if lang.strip() in LANGS]
    if not programs or not languages:
        print("no programs or languages selected")
        sys.exit(1)
    suffix = ".exe" if platform.system() == "Windows" else ""
    missing = [lang for lang in languages if LANGS[lang]["tool"] and not shutil.which(LANGS[lang]["tool"])]
    for lang in missing:
        print(f"warning: {LANGS[lang]['tool']} not found, skipping {lang}")
    languages = [lang for lang in languages if lang not in missing]

    results = {}
    failures = []
    with tempfile.TemporaryDirectory(prefix="cx-langs-") as workdir:
        for program in programs:
            results[program] = {}
            for lang in languages:
                src = os.path.join(ROOT, LANGS[lang]["dir"], program + LANGS[lang]["ext"])
                if not os.path.isfile(src):
                    continue
                binary = os.path.join(workdir, f"{program}-{lang}{suffix}")
                compile_cmd = build_command(lang, args.cx, src, binary)
                _, completed = run_timed(compile_cmd, capture_output=True, text=True)
                if completed.returncode != 0:
                    failures.append(f"{lang}/{program}: compile failed:\n{completed.stderr}")
                    continue
                times, outputs = [], set()
                for _ in range(args.runs):
                    elapsed, completed = run_timed([binary], capture_output=True, text=True)
                    if completed.returncode != 0:
                        failures.append(f"{lang}/{program} exited with status {completed.returncode}")
                        break
                    times.append(elapsed)
                    outputs.add(completed.stdout.strip())
                else:
                    if len(outputs) != 1:
                        failures.append(f"{lang}/{program} printed {len(outputs)} distinct outputs, benchmark invalid")
                        continue
                    output = outputs.pop()
                    if program in EXPECTED and output != EXPECTED[program]:
                        failures.append(f"{lang}/{program} printed {output}, expected {EXPECTED[program]}")
                        continue
                    median = statistics.median(times)
                    results[program][lang] = {"median_s": median, "runs_s": times, "output": output}
                    print(f"{program}/{lang}: {format_seconds(median)} (output {output})")

    record = {
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "platform": platform.platform(),
        "cx_sha": cx_sha(args.cx),
        "runs": args.runs,
        "tools": {lang: tool_version(LANGS[lang]["tool"]) for lang in languages if LANGS[lang]["tool"]},
        "program_order": programs,
        "programs": results,
    }
    for path, content in [(args.output, json.dumps(record, indent=2) + "\n"), (args.html, render_html(record))]:
        with open(path, "w") as file:
            file.write(content)
        print(f"wrote {path}")
    if failures:
        print("\n".join(failures))
        sys.exit(1)


if __name__ == "__main__":
    main()

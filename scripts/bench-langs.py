#!/usr/bin/env python3
"""Run-time benchmark: cx vs C, C++, Rust, Go, Odin, Zig and Swift on the bench corpus subset.

Usage:
    scripts/bench-langs.py --cx build/cx [--runs 3] [--compile-runs 3]
        [--programs fib,sieve] [--languages cx,c] [--modes release,debug]
        [--metrics run,compile] [--output bench/results.json]
        [--html bench/report.html]

Builds each program for each language twice: an optimized release build and
an unoptimized debug build (the configuration used during development). Runs
each binary --runs times (median kept). Debug builds are also timed
--compile-runs times (median kept) after one untimed warmup. --metrics
selects which of those to record: run, compile, or both. Writes a JSON
record plus a self-contained HTML report with graphs (open it straight from
disk).

fib/sieve/wordcount/mapfilter/jsonparse must print EXPECTED exactly in every
language that implements them; mandelbrot checks self-consistency only
(float-to-int conversion of huge values is platform-defined, so its checksum
legitimately differs). mapfilter is omitted for C, Go, Odin, and Zig, which have
no capturing lambdas. jsonparse is omitted for C, C++, and Rust, which have
no JSON parser in the standard library.
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
PROGRAMS = ["fib", "sieve", "mandelbrot", "wordcount", "mapfilter", "jsonparse"]
EXPECTED = {
    "fib": "102334155",
    "sieve": "5761455",
    "wordcount": "1028015628923350",
    "mapfilter": "164571298857200",
    "jsonparse": "939699517700",
}
# Languages with no source for a program, and why that program cannot be ported.
# The report states the reason; a missing file alone does not.
OMIT_REASON = {
    "mapfilter": {
        "langs": ("c", "go", "odin", "zig"),
        "because": "that benchmark times a capturing-lambda pipeline, which {who} cannot express",
    },
    "jsonparse": {
        "langs": ("c", "cxx", "rust"),
        "because": "that benchmark times the standard-library JSON parser, which {who} {do} not have",
    },
}
TIMEOUT = 600

LANGS = {
    "cx": {"label": "cx", "color": "var(--cx-bar)", "tool": None, "ext": ".cx"},
    "c": {"label": "C", "color": "#555555", "tool": "cc", "ext": ".c"},
    "cxx": {"label": "C++", "color": "#f34b7d", "tool": "c++", "ext": ".cpp"},
    "rust": {"label": "Rust", "color": "#dea584", "tool": "rustc", "ext": ".rs"},
    "go": {"label": "Go", "color": "#00ADD8", "tool": "go", "ext": ".go"},
    "odin": {"label": "Odin", "color": "#60AFFE", "tool": "odin", "ext": ".odin"},
    "zig": {"label": "Zig", "color": "#ec915c", "tool": "zig", "ext": ".zig"},
    "swift": {"label": "Swift", "color": "#F05138", "tool": "swiftc", "ext": ".swift"},
}
MODES = ["release", "debug"]
METRICS = ["run", "compile"]
MODE_LABEL = {"release": "optimized", "debug": "unoptimized debug"}
# cx debug keeps safety checks, including the leak detector.
DEBUG_NOTE = (
    "Unoptimized debug is the development build. "
    "cx omits --release, so safety checks stay on and both the LLVM IR pipeline and codegen optimizations are skipped. "
    "C and C++ use -O0 -g. "
    "Rust uses opt-level 0 with debug assertions, overflow checks, and full debug info. "
    "Go disables optimizations and inlining with -gcflags=all=-N -l, including the standard library. "
    "Odin uses -debug, which selects -o:none. "
    "Zig uses -ODebug; its release build is -OReleaseFast. "
    "Swift uses -Onone -g."
)
# Iteration time, not a from-scratch toolchain build. The untimed warmup fills
# the standard-library cache; each sample is a distinct copy of the program.
COMPILE_NOTE = (
    "Debug compile time is a rebuild of that development build. "
    "One untimed compile warms the standard-library cache first. "
    "Each sample is a separate copy of the source, so an unchanged-file cache hit is not what gets timed. "
    "ccache is disabled."
)


def lang_source(program, lang):
    return os.path.join(ROOT, "bench", program, program + LANGS[lang]["ext"])


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cx", required=True, help="path to cx compiler executable")
    parser.add_argument("--runs", type=int, default=3, help="runs per binary (median kept)")
    parser.add_argument("--compile-runs", type=int, default=3, help="debug compiles per program (median kept)")
    parser.add_argument("--programs", default=",".join(PROGRAMS), help="comma-separated subset")
    parser.add_argument("--languages", default=",".join(LANGS), help="comma-separated subset")
    parser.add_argument("--modes", default=",".join(MODES), help="comma-separated: release, debug")
    parser.add_argument("--metrics", default=",".join(METRICS), help="comma-separated: run, compile")
    parser.add_argument("--output", default="bench/results.json", help="where to write the JSON record")
    parser.add_argument("--html", default="bench/report.html", help="where to write the HTML report")
    return parser.parse_args()


def run_timed(cmd, **kwargs):
    start = time.perf_counter()
    try:
        result = subprocess.run(cmd, timeout=TIMEOUT, **kwargs)
    except subprocess.TimeoutExpired:
        print(f"timed out: {' '.join(cmd)}")
        sys.exit(1)
    return time.perf_counter() - start, result


def build_command(lang, mode, cx, src, binary):
    release = mode == "release"
    if lang == "cx":
        cmd = [cx, src, "-o", binary]
        if release:
            cmd.append("--release")
        cmd.append("-Werror")
        return cmd
    if lang == "c":
        opt = ["-O3"] if release else ["-O0", "-g"]
        return ["cc", *opt, "-std=c17", "-o", binary, src]
    if lang == "cxx":
        opt = ["-O3"] if release else ["-O0", "-g"]
        return ["c++", *opt, "-std=c++23", "-o", binary, src]
    if lang == "rust":
        if release:
            return ["rustc", "--edition=2024", "-C", "opt-level=3", "-o", binary, src]
        return [
            "rustc", "--edition=2024", "-C", "opt-level=0", "-C", "debug-assertions=yes",
            "-C", "overflow-checks=yes", "-C", "debuginfo=2", "-o", binary, src,
        ]
    if lang == "go":
        cmd = ["go", "build"]
        if not release:
            cmd.append("-gcflags=all=-N -l")
        cmd.extend(["-o", binary, src])
        return cmd
    if lang == "odin":
        cmd = ["odin", "build", src, "-file"]
        cmd.append("-o:speed" if release else "-debug")
        if not release:
            cmd.append("-o:none")
        cmd.append("-out:" + binary)
        return cmd
    if lang == "zig":
        # Local cache stays next to the binary so a run from the repo does not
        # leave .zig-cache behind. The global cache still holds the stdlib.
        return [
            "zig", "build-exe", src, "--cache-dir", binary + ".cache",
            "-femit-bin=" + binary, "-OReleaseFast" if release else "-ODebug",
        ]
    if lang == "swift":
        opt = ["-O"] if release else ["-Onone", "-g"]
        return ["swiftc", *opt, "-o", binary, src]
    raise AssertionError("unknown language " + lang)


def toolchain_env(workdir):
    env = os.environ.copy()
    # ccache would turn a repeated C or C++ compile into a cache hit.
    env["CCACHE_DISABLE"] = "1"
    # Private caches, so this bench does not depend on or rewrite the user's.
    # The untimed warmup fills them; timed samples reuse the standard library.
    env["GOCACHE"] = os.path.join(workdir, "gocache")
    env["ZIG_GLOBAL_CACHE_DIR"] = os.path.join(workdir, "zig-global")
    return env


def varied_source(workdir, lang, program, tag, src):
    dest_dir = os.path.join(workdir, "src")
    os.makedirs(dest_dir, exist_ok=True)
    dest = os.path.join(dest_dir, f"{program}-{lang}-{tag}{LANGS[lang]['ext']}")
    with open(src, "rb") as infile, open(dest, "wb") as outfile:
        outfile.write(infile.read())
        # A distinct comment makes content-addressed caches recompile the
        # program. The program's behavior does not change.
        outfile.write(f"\n// bench compile {tag}\n".encode())
    return dest


def describe_build(lang, mode):
    # Flags only, so the report can show the configuration without paths.
    cmd = build_command(lang, mode, "cx", "SRC", "BIN")
    shown = []
    skip_next = False
    for arg in cmd:
        if skip_next:
            skip_next = False
            continue
        if arg in ("SRC", "BIN") or arg.startswith("-out:") or arg.startswith("-femit-bin="):
            continue
        if arg in ("-o", "--cache-dir"):
            skip_next = True
            continue
        shown.append(arg)
    return " ".join(shown)


def tool_version(tool):
    cmd = [tool, "version"] if tool in ("go", "odin", "zig") else [tool, "--version"]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", timeout=60)
        lines = result.stdout.splitlines()
        return lines[0].strip() if result.returncode == 0 and lines else "unknown"
    except (OSError, subprocess.TimeoutExpired):
        return "missing"


def cx_sha(cx):
    try:
        result = subprocess.run([cx, "--version"], capture_output=True, text=True, encoding="utf-8", timeout=60)
        for line in result.stdout.splitlines():
            if line.startswith("cx commit:"):
                return line.split(":", 1)[1].strip()
    except (OSError, subprocess.TimeoutExpired):
        pass
    fallback = subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, capture_output=True, text=True, encoding="utf-8").stdout.strip()
    return fallback or "unknown"


def join_names(labels):
    if len(labels) == 1:
        return labels[0]
    if len(labels) == 2:
        return f"{labels[0]} and {labels[1]}"
    return ", ".join(labels[:-1]) + ", and " + labels[-1]


def omission_note(program, omitted):
    if not omitted:
        return ""
    reason = OMIT_REASON.get(program)
    explained = [lang for lang in omitted if reason and lang in reason["langs"]]
    unknown = [lang for lang in omitted if lang not in explained]
    sentences = []
    if explained:
        labels = [LANGS[lang]["label"] for lang in explained]
        has = "has" if len(labels) == 1 else "have"
        who = "it" if len(labels) == 1 else "they"
        do = "does" if len(labels) == 1 else "do"
        because = reason["because"].format(who=who, do=do)
        sentences.append(f"{join_names(labels)} {has} no {program} source because {because}.")
    if unknown:
        labels = [LANGS[lang]["label"] for lang in unknown]
        has = "has" if len(labels) == 1 else "have"
        sentences.append(f"{join_names(labels)} {has} no {program} source.")
    return " ".join(sentences)


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
        by_mode = record["programs"][program]
        note = (
            ' <span class="note">checksums differ by design here (float-to-int conversion is '
            "platform-defined); times remain comparable.</span>"
            if program == "mandelbrot"
            else ""
        )
        omitted = omission_note(program, record.get("omissions", {}).get(program, []))
        omitted_html = f'\n<p class="note">{html.escape(omitted)}</p>' if omitted else ""
        charts = []
        for mode in record["mode_order"]:
            entries = by_mode.get(mode, {})
            run_medians = {lang: data["median_s"] for lang, data in entries.items() if "median_s" in data}
            if run_medians:
                charts.append(f"<h3>{MODE_LABEL[mode]} run</h3>\n{chart_svg(run_medians)}")
            if mode == "debug":
                compile_medians = {
                    lang: data["compile"]["median_s"]
                    for lang, data in entries.items()
                    if "compile" in data
                }
                if compile_medians:
                    charts.append(f"<h3>{MODE_LABEL[mode]} compile</h3>\n{chart_svg(compile_medians)}")
        if not charts:
            programs.append(f"<h2>{program}</h2>{omitted_html}\n<p>no successful measurements</p>")
            continue
        programs.append(f"<h2>{program}{note}</h2>{omitted_html}\n{''.join(charts)}")
    tools = " · ".join(f"{lang}: {html.escape(version)}" for lang, version in record["tools"].items())
    summary_parts = []
    if "run" in record["metrics"]:
        summary_parts.append(f"median of {record['runs']} runs")
    if "compile" in record["metrics"]:
        summary_parts.append(f"median of {record['compile_runs']} debug compiles")
    summary = " · ".join(summary_parts)
    has_compile = "compile" in record["metrics"]
    builds = []
    for mode in record["mode_order"]:
        items = "".join(
            f"<li>{html.escape(LANGS[lang]['label'])}: <code>{html.escape(command)}</code></li>"
            for lang, command in record["builds"][mode].items()
        )
        builds.append(f"<h3>{MODE_LABEL[mode]}</h3><ul>{items}</ul>")
    return f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="color-scheme" content="light dark">
<title>cx vs C, C++, Rust, Go, Odin, Zig, Swift</title>
<style>
:root {{ color-scheme: light dark; --bg: #ffffff; --fg: #000000; --muted: #444444; --pre-bg: #f4f4f4; --cx-bar: #000000; }}
@media (prefers-color-scheme: dark) {{
  :root {{ --bg: #1a1a1a; --fg: #e8e8e8; --muted: #aaaaaa; --pre-bg: #2a2a2a; --cx-bar: #ffffff; }}
}}
body {{ font-family: system-ui, -apple-system, sans-serif; max-width: 900px; margin: 2rem auto; padding: 0 1rem; background: var(--bg); color: var(--fg); }}
.meta {{ color: var(--muted); }}
h2 {{ margin-top: 2rem; font-size: 1.2rem; }}
h3 {{ margin: 1rem 0 0.3rem; font-size: 1rem; }}
.note {{ font-weight: normal; font-size: 0.85rem; color: var(--muted); }}
p.note {{ margin: 0.2rem 0 0.6rem; }}
code {{ font-size: 0.9em; }}
svg text {{ font-size: 14px; fill: var(--fg); }}
svg text.value {{ font-variant-numeric: tabular-nums; }}
details {{ margin-top: 2rem; }}
pre {{ background: var(--pre-bg); padding: 1rem; overflow-x: auto; }}
</style>
</head>
<body>
<h1>cx vs C, C++, Rust, Go, Odin, Zig, Swift</h1>
<p class="meta">{html.escape(record["timestamp"])} · {html.escape(record["platform"])} · {html.escape(summary)}<br>{tools}<br>cx at {html.escape(record["cx_sha"])}</p>
<p>Each chart is one measurement. The ratio on each bar is against the fastest language in that chart.</p>
<p class="note">{html.escape(DEBUG_NOTE)}</p>
{"" if not has_compile else f'<p class="note">{html.escape(COMPILE_NOTE)}</p>'}
{"".join(programs)}
<details><summary>Build configurations</summary>{"".join(builds)}</details>
<details><summary>Raw data</summary><pre>{html.escape(json.dumps(record, indent=2))}</pre></details>
</body>
</html>
"""


def main():
    args = parse_args()
    args.runs = max(1, args.runs)
    args.compile_runs = max(1, args.compile_runs)
    programs = [p.strip() for p in args.programs.split(",") if p.strip() in PROGRAMS]
    languages = [lang.strip() for lang in args.languages.split(",") if lang.strip() in LANGS]
    modes = [mode.strip() for mode in args.modes.split(",") if mode.strip() in MODES]
    metrics = [metric.strip() for metric in args.metrics.split(",") if metric.strip() in METRICS]
    if not programs or not languages or not modes or not metrics:
        print("no programs, languages, modes, or metrics selected")
        sys.exit(1)
    if "compile" in metrics and "debug" not in modes:
        print("debug compile times require --modes to include debug")
        sys.exit(1)
    measure_run = "run" in metrics
    measure_compile = "compile" in metrics
    suffix = ".exe" if platform.system() == "Windows" else ""
    missing = [lang for lang in languages if LANGS[lang]["tool"] and not shutil.which(LANGS[lang]["tool"])]
    for lang in missing:
        print(f"warning: {LANGS[lang]['tool']} not found, skipping {lang}")
    languages = [lang for lang in languages if lang not in missing]

    results = {}
    omissions = {}
    failures = []
    with tempfile.TemporaryDirectory(prefix="cx-langs-") as workdir:
        for program in programs:
            results[program] = {}
            omissions[program] = []
            for lang in languages:
                src = lang_source(program, lang)
                if not os.path.isfile(src):
                    omissions[program].append(lang)
                    continue
                for mode in modes:
                    if mode == "release" and not measure_run:
                        continue
                    binary = os.path.join(workdir, f"{program}-{lang}-{mode}{suffix}")
                    compile_info = None
                    if mode == "debug" and measure_compile:
                        env = toolchain_env(workdir)
                        warmup = varied_source(workdir, lang, program, "warmup", src)
                        _, completed = run_timed(
                            build_command(lang, mode, args.cx, warmup, binary),
                            capture_output=True, text=True, encoding="utf-8", env=env,
                        )
                        if completed.returncode != 0:
                            failures.append(f"{lang}/{program}/{mode}: warmup compile failed:\n{completed.stderr}")
                            continue
                        compile_times = []
                        for i in range(args.compile_runs):
                            sample = varied_source(workdir, lang, program, str(i), src)
                            elapsed, completed = run_timed(
                                build_command(lang, mode, args.cx, sample, binary),
                                capture_output=True, text=True, encoding="utf-8", env=env,
                            )
                            if completed.returncode != 0:
                                failures.append(f"{lang}/{program}/{mode}: compile failed:\n{completed.stderr}")
                                break
                            compile_times.append(elapsed)
                        else:
                            compile_info = {
                                "median_s": statistics.median(compile_times),
                                "runs_s": compile_times,
                            }
                        if compile_info is None:
                            continue
                        if not measure_run:
                            results[program].setdefault(mode, {})[lang] = {"compile": compile_info}
                            print(f"{program}/{lang}/{mode}: compile {format_seconds(compile_info['median_s'])}")
                            continue
                    else:
                        _, completed = run_timed(
                            build_command(lang, mode, args.cx, src, binary),
                            capture_output=True, text=True, encoding="utf-8",
                        )
                        if completed.returncode != 0:
                            failures.append(f"{lang}/{program}/{mode}: compile failed:\n{completed.stderr}")
                            continue
                    times, outputs = [], set()
                    for _ in range(args.runs):
                        elapsed, completed = run_timed([binary], capture_output=True, text=True, encoding="utf-8")
                        if completed.returncode != 0:
                            detail = completed.stderr.strip()
                            suffix_note = f"\n{detail}" if detail else ""
                            failures.append(f"{lang}/{program}/{mode} exited with status {completed.returncode}{suffix_note}")
                            break
                        times.append(elapsed)
                        outputs.add(completed.stdout.strip())
                    else:
                        if len(outputs) != 1:
                            failures.append(f"{lang}/{program}/{mode} printed {len(outputs)} distinct outputs, benchmark invalid")
                            continue
                        output = outputs.pop()
                        if program in EXPECTED and output != EXPECTED[program]:
                            failures.append(f"{lang}/{program}/{mode} printed {output}, expected {EXPECTED[program]}")
                            continue
                        median = statistics.median(times)
                        entry = {"median_s": median, "runs_s": times, "output": output}
                        compile_note = ""
                        if compile_info is not None:
                            entry["compile"] = compile_info
                            compile_note = f", compile {format_seconds(compile_info['median_s'])}"
                        results[program].setdefault(mode, {})[lang] = entry
                        print(f"{program}/{lang}/{mode}: run {format_seconds(median)}{compile_note} (output {output})")

    record = {
        "timestamp": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "platform": platform.platform(),
        "cx_sha": cx_sha(args.cx),
        "runs": args.runs,
        "compile_runs": args.compile_runs,
        "metrics": metrics,
        "tools": {lang: tool_version(LANGS[lang]["tool"]) for lang in languages if LANGS[lang]["tool"]},
        "mode_order": modes,
        "builds": {mode: {lang: describe_build(lang, mode) for lang in languages} for mode in modes},
        "program_order": programs,
        "omissions": {program: langs for program, langs in omissions.items() if langs},
        "omission_notes": {
            program: omission_note(program, langs) for program, langs in omissions.items() if langs
        },
        "programs": results,
    }
    for path, content in [(args.output, json.dumps(record, indent=2) + "\n"), (args.html, render_html(record))]:
        with open(path, "w", encoding="utf-8") as file:
            file.write(content)
        print(f"wrote {path}")
    if failures:
        print("\n".join(failures))
        sys.exit(1)


if __name__ == "__main__":
    main()

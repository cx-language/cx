// Regression tests for bench.js. Runs the chart code against stub DOM +
// canvas with synthetic history, since the repo has no browser harness.
// Run: node website/test-bench.mjs
import fs from "node:fs";
import { fileURLToPath } from "node:url";
import path from "node:path";

const SITE = path.dirname(fileURLToPath(import.meta.url));
const SRC = fs.readFileSync(path.join(SITE, "bench.js"), "utf8");

let failures = 0;
function check(name, cond, detail = "") {
    if (!cond) {
        failures++;
        console.log(`FAIL ${name} ${detail}`);
    }
}

function makeCtx() {
    return new Proxy(
        {},
        {
            get(t, p) {
                if (p in t) return t[p];
                return () => {};
            },
            set(t, p, v) {
                t[p] = v;
                return true;
            },
        }
    );
}

function makeEl() {
    return {
        children: [],
        handlers: {},
        style: {},
        innerHTML: "",
        hidden: true,
        offsetWidth: 100,
        clientWidth: 800,
        parentNode: null,
        appendChild(c) {
            this.children.push(c);
            c.parentNode = this;
        },
        addEventListener(ev, fn) {
            this.handlers[ev] = fn;
        },
        getContext() {
            return makeCtx();
        },
        getBoundingClientRect() {
            return { left: 0, top: 0 };
        },
    };
}

function sha(i) {
    return (i.toString(16).padStart(7, "0") + "deadbeefcafebabe").slice(0, 40);
}

function records(n, { gapAt = -1, nullBuild = false } = {}) {
    const out = [];
    for (let i = 0; i < n; i++) {
        out.push({
            sha: sha(i),
            timestamp: `2026-09-${String(10 + i).padStart(2, "0")}T12:00:00+00:00`,
            metrics: {
                cxx_build_s: nullBuild ? null : 120 + i * 2,
                check_s: nullBuild || i === gapAt ? null : 1500 - i * 5,
                compile_s: { sieve: 0.2 + i * 0.001, mandelbrot: 0.18, fib: 0.17 },
                run_s: { sieve: 0.9, mandelbrot: 0.85 + i * 0.002, fib: 0.3 },
                iterate_s: { sieve: 1.4, mandelbrot: 1.2, fib: 0.5 + i * 0.001 },
                cx_bytes: 33000000 + i * 1000,
                bench_bytes: { sieve: 40264, mandelbrot: 39672, fib: 38696 },
                sloc: {
                    total: 200000 + i * 100,
                    compiler: 100000 + i * 60,
                    stdlib: 20000 + i * 10,
                    vendor: 4000,
                    docs: 12000,
                    examples: 14000 + i * 10,
                    tests: 50000 + i * 20,
                },
            },
        });
    }
    return out;
}

function lateRecords() {
    const out = records(12);
    for (let i = 6; i < 12; i++) {
        out[i].metrics.compile_s.wordcount = 0.25;
        out[i].metrics.run_s.wordcount = 0.5;
        out[i].metrics.iterate_s.wordcount = 0.8;
        out[i].metrics.bench_bytes.wordcount = 41000;
    }
    return out;
}

// Old bench-data history: early records predate SLOC, one no-git run has null.
function mixedSlocRecords() {
    const out = records(12);
    for (let i = 0; i < 5; i++) delete out[i].metrics.sloc;
    out[5].metrics.sloc = null;
    return out;
}

function langsRecord() {
    // Mirrors scripts/bench-langs.py output: release+debug runs, debug compiles and iterations.
    const entry = (run, compile = null, iterate = null) => {
        const e = { median_s: run, runs_s: [run, run, run], output: "102334155" };
        if (compile !== null) e.compile = { median_s: compile, runs_s: [compile, compile, compile] };
        if (iterate !== null) e.iterate = { median_s: iterate, runs_s: [iterate, iterate, iterate] };
        return e;
    };
    return {
        timestamp: "2026-09-15T12:00:00+00:00",
        platform: "Linux-6.8-x86_64",
        cx_sha: sha(42),
        runs: 3,
        compile_runs: 3,
        metrics: ["run", "compile", "iterate"],
        tools: { c: "cc 13.2", rust: "rustc 1.98", go: "go1.27", odin: "dev-2026-09", zig: "0.16.0" },
        mode_order: ["release", "debug"],
        builds: {
            release: { cx: "cx --release -Werror", c: "cc -O3 -std=c17", rust: "rustc -C opt-level=3" },
            debug: { cx: "cx -Werror", c: "cc -O0 -g", rust: "rustc -C opt-level=0" },
        },
        iterate_builds: { cx: "cx run -Werror", c: "cc -O0 -g + run", rust: "rustc -C opt-level=0 + run" },
        program_order: ["fib", "mandelbrot", "mapfilter"],
        omissions: { mapfilter: ["c", "go", "odin", "zig"] },
        omission_notes: {
            mapfilter: "C, Go, Odin, and Zig have no mapfilter source because that benchmark times a capturing-lambda pipeline, which they cannot express.",
        },
        programs: {
            fib: {
                release: { cx: entry(0.9), c: entry(0.8), rust: entry(1.1) },
                debug: { cx: entry(1.5, 0.25, 1.7), c: entry(1.2, 0.1, 1.3), rust: entry(2.0, 0.4, 2.4) },
            },
            mandelbrot: {
                release: { cx: entry(0.85), c: entry(0.95) },
                debug: { cx: entry(1.1, 0.22, 1.3), c: entry(1.0, 0.09, 1.1) },
            },
            mapfilter: {
                release: { cx: entry(0.5), rust: entry(0.6) },
                debug: { cx: entry(0.9, 0.3, 1.2), rust: entry(1.0, 0.5, 1.5) },
            },
        },
    };
}

function langsSparseRecord() {
    // Minimal shape: no metrics, mode_order, tools, builds, or cx_sha.
    return {
        timestamp: "2026-09-15T12:00:00+00:00",
        platform: "Linux",
        program_order: ["fib"],
    };
}

function langsHostileRecord() {
    // Escaping plus single-language, compile-only, and empty-program shapes.
    return {
        timestamp: "2026-09-15T12:00:00+00:00",
        platform: "<img src=x>",
        cx_sha: 'abc"><img src=x onerror=alert(1)>',
        runs: 1,
        compile_runs: 1,
        metrics: ["run", "compile"],
        tools: { c: 'cc <img src=x onerror="alert(1)">' },
        mode_order: ["release", "debug"],
        builds: {
            release: { cx: 'cx "--quoted"' },
            debug: { cx: "cx" },
        },
        program_order: ["fib", "solo", "empty"],
        omission_notes: { empty: 'note with "quotes" & <tags>' },
        programs: {
            fib: {
                release: { cx: { median_s: 1.5, runs_s: [1.5], output: "1" } },
                debug: { cx: { compile: { median_s: 0.2, runs_s: [0.2] } } },
            },
            solo: {
                release: { cx: { median_s: 0.5, runs_s: [0.5], output: "1" } },
            },
            empty: {},
        },
    };
}

function verifySparse(name, els) {
    const charts = els["langs-charts"].innerHTML;
    check(name, charts.includes("fib") && charts.includes("no successful measurements"), "sparse renders");
    check(name, els["langs-meta"].innerHTML.includes("2026-09-15"), "sparse meta");
}

function verifyHostile(name, els) {
    const all = els["langs-meta"].innerHTML + els["langs-charts"].innerHTML + els["langs-builds"].innerHTML;
    check(name, !all.includes("<img"), "hostile escaped");
    check(name, all.includes("&lt;img"), "hostile entities");
    check(name, all.includes("&quot;") && all.includes("&amp;"), "hostile quotes");
    const charts = els["langs-charts"].innerHTML;
    check(name, charts.includes("no successful measurements"), "hostile empty program");
    // fib release run + fib debug compile-only + solo single-language run.
    check(name, (charts.match(/<svg/g) || []).length === 3, "hostile svg count");
}

async function scenario(
    name,
    payload,
    {
        ok = true,
        wantIndex = 5,
        hoverX = 400,
        wantAbsent = null,
        wantCounts = "2,3,3,3,1,3,7",
        wantLate = null,
        slocLate = false,
        langs = langsRecord(),
        langsOk = true,
        langsVerify = null,
    } = {}
) {
    const els = {};
    for (const id of [
        "legend-build",
        "legend-compile",
        "legend-run",
        "legend-iterate",
        "legend-cxsize",
        "legend-benchsize",
        "legend-sloc",
        "bench-tip",
        "bench-charts",
        "langs-meta",
        "langs-charts",
        "langs-builds",
    ]) {
        els[id] = makeEl();
        els[id].parentNode = makeEl();
    }
    let opened = null;
    globalThis.document = {
        getElementById: (id) => els[id] || null,
        createElement: () => makeEl(),
        createTextNode: (text) => ({ text }),
        documentElement: {},
    };
    globalThis.window = {
        devicePixelRatio: 2,
        innerWidth: 1200,
        addEventListener: (ev, fn) => (globalThis.__resize = fn),
        open: (url) => (opened = url),
    };
    globalThis.getComputedStyle = () => ({ getPropertyValue: () => "#111" });
    globalThis.fetch = async (url) => {
        if (String(url).includes("langs")) return { ok: langsOk, json: async () => langs };
        return { ok, json: async () => payload };
    };
    eval(SRC);
    await new Promise((r) => setTimeout(r, 20));

    const checkLangs = () => {
        if (langsVerify) {
            langsVerify(name, els);
            return;
        }
        if (!langsOk || !langs || !langs.program_order || !langs.program_order.length) {
            check(name, els["langs-charts"].innerHTML.includes("No language comparison"), "langs stub missing");
            return;
        }
        const meta = els["langs-meta"].innerHTML;
        check(name, meta.includes("2026-09-15") && meta.includes("Linux-6.8"), "langs meta");
        check(name, meta.includes("median of 3 runs") && meta.includes("median of 3 debug compiles"), "langs summary");
        check(name, meta.includes("median of 3 dev iterations"), "langs iterate summary");
        check(name, meta.includes(sha(42).slice(0, 7)), "langs sha");
        const charts = els["langs-charts"].innerHTML;
        check(name, charts.includes("fib") && charts.includes("mapfilter"), "langs programs");
        check(name, charts.includes("optimized run") && charts.includes("unoptimized debug run"), "langs run charts");
        check(name, charts.includes("unoptimized debug compile"), "langs compile chart");
        check(name, charts.includes("dev iteration"), "langs iterate chart");
        check(name, charts.includes("(1.00x)"), "langs fastest ratio");
        check(name, charts.includes("capturing-lambda"), "langs omission note");
        check(name, charts.includes("platform-defined"), "langs mandelbrot note");
        // Fastest bar sorts first: fib release C (0.8s) precedes cx (0.9s).
        check(name, charts.indexOf(">C<") < charts.indexOf(">cx<"), "langs sort");
        check(name, els["langs-builds"].innerHTML.includes("Build configurations"), "langs builds shown");
        check(name, els["langs-builds"].innerHTML.includes("cc -O0 -g"), "langs builds body");
        check(name, els["langs-builds"].innerHTML.includes("cx run -Werror"), "langs iterate builds");
    };

    if (!ok || !payload || !payload.length) {
        check(name, els["bench-charts"].innerHTML.includes("No benchmark data"), "empty stub missing");
        checkLangs();
        console.log(`ok ${name}`);
        return;
    }
    const legends = ["legend-build", "legend-compile", "legend-run", "legend-iterate", "legend-cxsize", "legend-benchsize", "legend-sloc"];
    const counts = legends.map((id) => els[id].children.length);
    check(name, String(counts) === wantCounts, `legends [${counts}]`);
    const canvases = legends.map(
        (id) => els[id].parentNode.children.find((c) => c.handlers.mousemove || "width" in c) || {}
    );
    // Unsized backing stores render stretched; every chart must size its canvas.
    canvases.forEach((c, i) => {
        check(name, c.width === 1600 && c.height === 520, `canvas ${i} ${c.width}x${c.height}`);
    });
    const canvas = canvases[0];
    if (!canvas.handlers || !canvas.handlers.mousemove) {
        checkLangs();
        console.log(`ok ${name} (no-data chart)`);
        return;
    }
    canvas.handlers.mousemove({ clientX: hoverX, clientY: 100 });
    const tip = els["bench-tip"];
    check(
        name,
        !tip.hidden && tip.innerHTML.includes(sha(wantIndex).slice(0, 7)) && tip.innerHTML.includes("C++ build"),
        "tooltip"
    );
    if (wantAbsent) check(name, !tip.innerHTML.includes(wantAbsent), `tooltip lacks ${wantAbsent}`);
    if (wantLate) {
        const compile = canvases[1];
        compile.handlers.mousemove({ clientX: 784, clientY: 100 });
        check(name, tip.innerHTML.includes(wantLate), `late tooltip has ${wantLate}`);
        compile.handlers.mousemove({ clientX: 64, clientY: 100 });
        check(name, !tip.innerHTML.includes(wantLate), `early tooltip lacks ${wantLate}`);
    }
    canvas.handlers.mouseleave();
    check(name, tip.hidden, "tooltip hides");
    canvas.handlers.mousemove({ clientX: hoverX, clientY: 100 });
    canvas.handlers.click();
    const wantUrl =
        wantIndex === 0
            ? "https://github.com/cx-language/cx/commit/" + sha(0)
            : "https://github.com/cx-language/cx/compare/" + sha(wantIndex - 1) + "..." + sha(wantIndex);
    check(name, opened === wantUrl, `click ${opened}`);
    // The first point of a multi-record chart has no previous record.
    canvas.handlers.mousemove({ clientX: 64, clientY: 100 });
    canvas.handlers.click();
    check(name, opened === "https://github.com/cx-language/cx/commit/" + sha(0), `first click ${opened}`);
    // Resize must repaint at new geometry, not stretch the old store.
    canvas.clientWidth = 400;
    globalThis.__resize();
    check(name, canvas.width === 800, `resized ${canvas.width}`);
    // The SLOC chart tooltips thousands-separated counts.
    const sloc = canvases[6];
    if (slocLate) {
        sloc.handlers.mousemove({ clientX: 457, clientY: 100 });
        check(name, tip.innerHTML.includes("total: 200,"), "late sloc tooltip");
        sloc.handlers.mousemove({ clientX: 64, clientY: 100 });
        check(name, !tip.innerHTML.includes("total:"), "early sloc tooltip lacks total");
    } else {
        sloc.handlers.mousemove({ clientX: hoverX, clientY: 100 });
        check(name, tip.innerHTML.includes("total: 200,"), "sloc tooltip");
    }
    sloc.handlers.mouseleave();
    checkLangs();
    console.log(`ok ${name}`);
}

await scenario("records", records(12));
await scenario("late series", lateRecords(), { wantCounts: "2,4,4,4,1,4,7", wantLate: "wordcount" });
await scenario("late sloc", mixedSlocRecords(), { slocLate: true });
await scenario("gap", records(12, { gapAt: 6 }), { wantIndex: 6, hoverX: 457, wantAbsent: "test suite" });
await scenario("null build metrics", records(4, { nullBuild: true }));
await scenario("single", records(1), { wantIndex: 0 });
await scenario("empty", []);
await scenario("fetch-fail", null, { ok: false });
await scenario("langs fetch-fail", records(4), { wantIndex: 1, langsOk: false, langs: null });
await scenario("langs empty", records(4), { wantIndex: 1, langs: { program_order: [] } });
await scenario("langs sparse", records(4), { wantIndex: 1, langs: langsSparseRecord(), langsVerify: verifySparse });
await scenario("langs hostile", records(4), { wantIndex: 1, langs: langsHostileRecord(), langsVerify: verifyHostile });

if (failures) {
    console.log(`${failures} FAILURES`);
    process.exit(1);
}
console.log("ALL PASS");

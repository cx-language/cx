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
                cx_bytes: 33000000 + i * 1000,
                bench_bytes: { sieve: 40264, mandelbrot: 39672, fib: 38696 },
            },
        });
    }
    return out;
}

async function scenario(name, payload, { ok = true, wantIndex = 5, hoverX = 400, wantAbsent = null } = {}) {
    const els = {};
    for (const id of ["legend-build", "legend-compile", "legend-run", "legend-size", "bench-tip", "bench-charts"]) {
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
    globalThis.fetch = async () => ({ ok, json: async () => payload });
    eval(SRC);
    await new Promise((r) => setTimeout(r, 20));

    if (!ok || !payload || !payload.length) {
        check(name, els["bench-charts"].innerHTML.includes("No benchmark data"), "empty stub missing");
        console.log(`ok ${name}`);
        return;
    }
    const counts = ["legend-build", "legend-compile", "legend-run", "legend-size"].map(
        (id) => els[id].children.length
    );
    check(name, String(counts) === "2,3,3,4", `legends [${counts}]`);
    const canvases = ["legend-build", "legend-compile", "legend-run", "legend-size"].map(
        (id) => els[id].parentNode.children.find((c) => c.handlers.mousemove || "width" in c) || {}
    );
    // Unsized backing stores render stretched; every chart must size its canvas.
    canvases.forEach((c, i) => {
        check(name, c.width === 1600 && c.height === 520, `canvas ${i} ${c.width}x${c.height}`);
    });
    const canvas = canvases[0];
    if (!canvas.handlers || !canvas.handlers.mousemove) {
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
    canvas.handlers.mouseleave();
    check(name, tip.hidden, "tooltip hides");
    canvas.handlers.mousemove({ clientX: hoverX, clientY: 100 });
    canvas.handlers.click();
    check(name, opened === "https://github.com/cx-language/cx/commit/" + sha(wantIndex), `click ${opened}`);
    // Resize must repaint at new geometry, not stretch the old store.
    canvas.clientWidth = 400;
    globalThis.__resize();
    check(name, canvas.width === 800, `resized ${canvas.width}`);
    console.log(`ok ${name}`);
}

await scenario("records", records(12));
await scenario("gap", records(12, { gapAt: 6 }), { wantIndex: 6, hoverX: 457, wantAbsent: "test suite" });
await scenario("null build metrics", records(4, { nullBuild: true }));
await scenario("single", records(1), { wantIndex: 0 });
await scenario("empty", []);
await scenario("fetch-fail", null, { ok: false });

if (failures) {
    console.log(`${failures} FAILURES`);
    process.exit(1);
}
console.log("ALL PASS");

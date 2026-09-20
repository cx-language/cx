// Tests docs/playground-pipeline.js by running the full browser pipeline with
// a mocked cx frontend (returning the committed fixture) and the real WASI
// shim + C toolchain. Usage:
//   node wasm/test-pipeline.mjs <toolchain-dir>
// See test-wasi-shim.mjs for the <toolchain-dir> layouts.

import { readFile, stat } from "node:fs/promises";
import path from "node:path";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);
const repoRoot = new URL("..", import.meta.url).pathname;
const CxWasi = require(repoRoot + "docs/wasi-shim.js");
const CxPipeline = require(repoRoot + "docs/playground-pipeline.js");
const fflate = require(repoRoot + "docs/lib/fflate.js");

const toolchainDir = process.argv[2];
if (!toolchainDir) {
    console.error("usage: node wasm/test-pipeline.mjs <toolchain-dir>");
    process.exit(2);
}

let failures = 0;
function check(condition, message) {
    if (!condition) {
        console.error("FAIL: " + message);
        failures++;
    } else {
        console.log("PASS: " + message);
    }
}

async function main() {
    // Mocked cx-wasm factory: stage 1 returns the fixture like the real
    // frontend would (the frontend itself is covered by test/compile-api).
    // Like the real factory, it creates a fresh module object per call.
    const fixture = await readFile(new URL("./fixtures/smoke.c", import.meta.url), "utf8");
    const CxWasmFactory = async (config) => {
        check(typeof config.print === "function" && typeof config.printErr === "function", "factory receives diagnostic hooks");
        return {
            FS: {
                writeFile: (filePath, content) => {
                    check(filePath === "/main.cx", "source is staged for diagnostics");
                    check(content.includes("fib"), "staged source matches");
                },
            },
            cxCompileToC: (source, importSearchPath) => {
                check(source.includes("fib"), "frontend receives the cx source");
                check(importSearchPath === "/cx", "frontend receives the std search path");
                return JSON.stringify({ status: 0, cCode: fixture });
            },
        };
    };

    // Stage 1: cx to C.
    const compiled = await CxPipeline.compileCxToC(CxWasmFactory, "void main() { fib(); }\n");
    check(compiled.status === 0, "stage 1 succeeds");
    check(compiled.cCode.includes("int main(void)"), "stage 1 returns generated C");

    // Stage 2+3 setup: toolchain files like the browser loads them.
    const wccFiles = {};
    const zipPath = path.join(toolchainDir, "wcc-files.zip");
    if ((await stat(zipPath).catch(() => null))?.isFile()) {
        const unzipped = fflate.unzipSync(await readFile(zipPath));
        for (const name of Object.keys(unzipped)) {
            if (!name.endsWith("/")) wccFiles["/" + name] = unzipped[name];
        }
        check(Object.keys(wccFiles).length > 10, "toolchain archive unpacks (" + Object.keys(wccFiles).length + " files)");
    } else {
        throw new Error("this test requires a distribution directory, run wasm/build-xcc-dist.sh first");
    }
    const ccWasmBytes = await readFile(path.join(toolchainDir, "cc.wasm"));

    const fs = CxPipeline.createCompilerFs(CxWasi, wccFiles);
    check(fs.readFile("/usr/include/stdio.h") !== null, "headers are staged");
    check(fs.readFile("/usr/lib/wlibc.a") !== null, "libraries are staged");

    const cResult = await CxPipeline.compileCToWasm(CxWasi, fs, ccWasmBytes, compiled.cCode);
    check(cResult.exitCode === 0, "stage 2 (C to WebAssembly) succeeds, stderr: " + JSON.stringify(cResult.stderr.slice(-200)));
    check(cResult.wasmBytes && cResult.wasmBytes.length > 0, "stage 2 produces a module");

    const runResult = await CxPipeline.runWasm(CxWasi, fs, cResult.wasmBytes);
    check(runResult.stdout === "45\n6765\n25\n120\n", "stage 3 runs with expected output, got: " + JSON.stringify(runResult.stdout));

    // A failing C program surfaces compiler diagnostics instead of crashing.
    const fs2 = CxPipeline.createCompilerFs(CxWasi, wccFiles);
    const badResult = await CxPipeline.compileCToWasm(CxWasi, fs2, ccWasmBytes, "int main(void) { this is not C; }\n");
    check(badResult.exitCode !== 0, "stage 2 reports C errors");
    check(!badResult.wasmBytes, "stage 2 produces no module on failure");

    if (failures > 0) {
        console.error(failures + " test(s) failed");
        process.exit(1);
    }
    console.log("All pipeline tests passed.");
}

await main();

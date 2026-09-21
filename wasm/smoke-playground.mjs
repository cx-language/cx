// Full playground smoke test with real builds. Loads the Emscripten-built
// cx frontend and the xcc distribution from <dist-dir>, compiles a cx
// program end to end, and checks its output. Usage:
//   node wasm/smoke-playground.mjs <dist-dir>

import { readFile } from "node:fs/promises";
import path from "node:path";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);
const repoRoot = new URL("..", import.meta.url).pathname;
const CxWasi = require(repoRoot + "docs/wasi-shim.js");
const CxPipeline = require(repoRoot + "docs/playground-pipeline.js");
const fflate = require(repoRoot + "docs/lib/fflate.js");

const distDir = process.argv[2];
if (!distDir) {
    console.error("usage: node wasm/smoke-playground.mjs <dist-dir>");
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

const source = 'void main() {\n    println("Hello from the playground");\n}\n';

async function main() {
    // Stage 1 with the real Emscripten module (UMD build loads in Node.js).
    // A fresh instance is created, matching how the worker uses the factory.
    // locateFile points Emscripten at the dist directory for cx-wasm.wasm
    // and cx-wasm.data (it defaults to the working directory under Node.js).
    const distDirAbs = path.resolve(distDir);
    const CxWasm = require(path.join(distDirAbs, "cx-wasm.js"));
    const createModule = (config) => CxWasm({ ...config, locateFile: (file) => path.join(distDirAbs, file) });
    const compiled = await CxPipeline.compileCxToC(createModule, source);
    if (compiled.status !== 0) {
        console.error("diagnostics:\n" + compiled.diagnostics.stderr);
    }
    check(compiled.status === 0, "stage 1 (cx to C) succeeds");
    check(compiled.cCode.includes("Hello from the playground"), "stage 1 generates C");

    // Stages 2+3 with the real toolchain distribution.
    const unzipped = fflate.unzipSync(await readFile(path.join(distDir, "wcc-files.zip")));
    const wccFiles = {};
    for (const name of Object.keys(unzipped)) {
        if (!name.endsWith("/")) wccFiles["/" + name] = unzipped[name];
    }
    const ccWasmBytes = await readFile(path.join(distDir, "cc.wasm"));
    const fs = CxPipeline.createCompilerFs(CxWasi, wccFiles);

    const cResult = await CxPipeline.compileCToWasm(CxWasi, fs, ccWasmBytes, compiled.cCode);
    check(cResult.exitCode === 0, "stage 2 (C to WebAssembly) succeeds, stderr: " + JSON.stringify(cResult.stderr.slice(-2000)));
    check(!!cResult.wasmBytes, "stage 2 produces a module");

    const runResult = await CxPipeline.runWasm(CxWasi, fs, cResult.wasmBytes);
    check(runResult.stdout === "Hello from the playground\n", "stage 3 output matches, got: " + JSON.stringify(runResult.stdout));

    // Error diagnostics still work end to end.
    const bad = await CxPipeline.compileCxToC(createModule, "void main() {\n    nope;\n}\n");
    check(bad.status !== 0, "erroneous input fails stage 1");

    if (failures > 0) {
        console.error(failures + " test(s) failed");
        process.exit(1);
    }
    console.log("Playground smoke test passed.");
}

await main();

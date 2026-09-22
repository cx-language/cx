// End-to-end test of website/wasi-shim.js using the real in-browser C compiler
// (cc.wasm, built from xcc) and real cx output. Mirrors exactly what the
// browser playground does: compile C to WebAssembly with cc.wasm, then run
// the result.
//
// Usage: node wasm/test-wasi-shim.mjs <toolchain-dir>
//
// <toolchain-dir> is either an xcc checkout (with cc.wasm, include/ and
// lib/wcrt0.a + lib/wlibc.a) or a distribution directory as produced by
// wasm/build-xcc-dist.sh (with cc.wasm and wcc-files.zip).

import { readFile, readdir, stat } from "node:fs/promises";
import path from "node:path";
import { createRequire } from "node:module";

const require = createRequire(import.meta.url);
const repoRoot = new URL("..", import.meta.url).pathname;
const CxWasi = require(repoRoot + "website/wasi-shim.js");
const fflate = require(repoRoot + "website/lib/fflate.js");

const xccDir = process.argv[2];
if (!xccDir) {
    console.error("usage: node wasm/test-wasi-shim.mjs <xcc-dir>");
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

async function addDirToFs(fs, hostDir, guestDir) {
    for (const entry of await readdir(hostDir, { withFileTypes: true })) {
        const hostPath = path.join(hostDir, entry.name);
        const guestPath = guestDir + "/" + entry.name;
        if (entry.isDirectory()) {
            fs.mkdirs(guestPath);
            await addDirToFs(fs, hostPath, guestPath);
        } else {
            fs.writeFile(guestPath, await readFile(hostPath));
        }
    }
}

async function main() {
    const fs = new CxWasi.FileSystem();
    fs.mkdirs("/tmp");
    fs.mkdirs("/work");

    let ccWasm;
    const zipPath = path.join(xccDir, "wcc-files.zip");
    if ((await stat(zipPath).catch(() => null))?.isFile()) {
        // Distribution layout: unpack the toolchain archive like the browser does.
        const unzipped = fflate.unzipSync(await readFile(zipPath));
        for (const name of Object.keys(unzipped)) {
            if (name.endsWith("/")) continue;
            const guestPath = "/" + name;
            fs.mkdirs(guestPath.substring(0, guestPath.lastIndexOf("/")) || "/");
            fs.writeFile(guestPath, unzipped[name]);
        }
        ccWasm = await readFile(path.join(xccDir, "cc.wasm"));
    } else {
        // Raw xcc checkout layout.
        await addDirToFs(fs, path.join(xccDir, "include"), "/usr/include");
        fs.mkdirs("/usr/lib");
        fs.writeFile("/usr/lib/wcrt0.a", await readFile(path.join(xccDir, "lib/wcrt0.a")));
        fs.writeFile("/usr/lib/wlibc.a", await readFile(path.join(xccDir, "lib/wlibc.a")));
        ccWasm = await readFile(path.join(xccDir, "cc.wasm"));
    }

    // Compile a C program (representative cx output, see wasm/fixtures/) with
    // cc.wasm running on the shim.
    const source = await readFile(new URL("./fixtures/smoke.c", import.meta.url), "utf8");
    fs.writeFile("/work/main.c", source);

    const compiler = new CxWasi.WasiProcess(fs, { args: ["cc", "/work/main.c", "-o", "/work/main.wasm"] });
    const compileResult = await compiler.run(ccWasm);
    check(compileResult.exitCode === 0, "cc.wasm compiles the generated C (exit " + compileResult.exitCode + ")");
    if (compileResult.stderr) console.log("[cc stderr]\n" + compileResult.stderr);

    const programWasm = fs.readFile("/work/main.wasm");
    check(programWasm && programWasm.length > 0, "compiled WebAssembly module was produced");

    if (programWasm) {
        const runner = new CxWasi.WasiProcess(fs, { args: [] });
        const runResult = await runner.run(programWasm);
        check(runResult.exitCode === 0, "compiled program runs (exit " + runResult.exitCode + ")");
        check(runResult.stdout === "45\n6765\n25\n120\n", "compiled program prints the expected output, got: " + JSON.stringify(runResult.stdout));
    }

    if (failures > 0) {
        console.error(failures + " test(s) failed");
        process.exit(1);
    }
    console.log("All WASI shim tests passed.");
}

await main();

// Web Worker hosting the cx playground pipeline.
//
// The worker loads the Emscripten-built cx frontend (cx-wasm.js), the WASI
// shim, the shared pipeline, and the C toolchain files, then answers
// {"action": "run", "id", "code"} messages with
// {"id", "stdout", "stderr"}.
//
// Heavy artifacts are loaded lazily on the first run and cached in the worker
// for subsequent runs. Note that only downloaded bytes are cached: every Run
// instantiates a fresh compiler module (matching the compiler's
// start-compile-exit design), so no compiler state ever carries over between
// runs. Posting {"action": "ping", "id"} answers {"id",
// "ready": true} once all artifacts are loaded (used to warm up).
// {"action": "check", "id", "code"} answers {"id", "stdout", "stderr"} with
// the stage-1 diagnostics only, without compiling to WebAssembly or running.

/* global importScripts, CxWasm, CxWasi, CxPipeline, fflate, postMessage, onmessage, fetch */

importScripts("cx-wasm.js", "wasi-shim.js", "playground-pipeline.js", "lib/fflate.js");

var ccWasmBytesPromise = null;
var wccFilesPromise = null;

function warmUpCxWasm() {
    // Instantiate once and drop the instance: this warms the download and
    // WebAssembly compile caches so that the first real run is fast. Actual
    // compilations always use fresh instances (see compileCxToC).
    return CxWasm({
        print: function () {},
        printErr: function () {},
    }).then(function () {});
}

function loadCcWasm() {
    if (!ccWasmBytesPromise) {
        ccWasmBytesPromise = fetch("cc.wasm").then(function (response) {
            if (!response.ok) throw new Error("failed to load cc.wasm: " + response.status);
            return response.arrayBuffer();
        }).then(function (buffer) {
            return new Uint8Array(buffer);
        });
    }
    return ccWasmBytesPromise;
}

function loadWccFiles() {
    if (!wccFilesPromise) {
        wccFilesPromise = fetch("wcc-files.zip").then(function (response) {
            if (!response.ok) throw new Error("failed to load wcc-files.zip: " + response.status);
            return response.arrayBuffer();
        }).then(function (buffer) {
            return unzipFiles(new Uint8Array(buffer));
        });
    }
    return wccFilesPromise;
}

// Unpacks the C toolchain archive (stored as usr/... paths in the ZIP)
// into absolute file system paths.
function unzipFiles(data) {
    var unzipped = fflate.unzipSync(data);
    var files = {};
    Object.keys(unzipped).forEach(function (name) {
        if (name[name.length - 1] === "/") return;
        files["/" + name] = unzipped[name];
    });
    return files;
}

// Stage 1 of runCode, shared with the "check" action: compile cx to C,
// reporting compiler-internal failures via stderr.
async function compileCxStage(code) {
    var compiled;
    try {
        compiled = await CxPipeline.compileCxToC(CxWasm, code);
    } catch (error) {
        return { status: -1, cCode: undefined, stdout: "", stderr: "error: the cx compiler failed to run (" + (error && error.message ? error.message : error) + ")\n" };
    }
    var compileStderr = compiled.diagnostics.stderr;
    if (compiled.status !== 0 && compiled.internalError) {
        compileStderr += "internal compiler error: " + compiled.internalError + "\n";
    }
    return { status: compiled.status, cCode: compiled.cCode, stdout: compiled.diagnostics.stdout, stderr: compileStderr };
}

async function runCode(code) {
    var compiled = await compileCxStage(code);
    if (compiled.status !== 0) {
        return { stdout: compiled.stdout, stderr: compiled.stderr };
    }
    var ccWasmBytes = await loadCcWasm();
    var wccFiles = await loadWccFiles();
    var fs = CxPipeline.createCompilerFs(CxWasi, wccFiles);
    var cCompiled = await CxPipeline.compileCToWasm(CxWasi, fs, ccWasmBytes, compiled.cCode);
    if (cCompiled.exitCode !== 0 || !cCompiled.wasmBytes) {
        return { stdout: cCompiled.stdout, stderr: "C compilation failed:\n" + cCompiled.stderr };
    }
    var executed = await CxPipeline.runWasm(CxWasi, fs, cCompiled.wasmBytes);
    return { stdout: executed.stdout, stderr: executed.stderr };
}

onmessage = function (event) {
    var message = event.data;
    if (message.action === "ping") {
        Promise.all([warmUpCxWasm(), loadCcWasm(), loadWccFiles()]).then(function () {
            postMessage({ id: message.id, ready: true });
        }, function (error) {
            postMessage({ id: message.id, ready: false, error: String(error) });
        });
        return;
    }
    if (message.action === "run") {
        runCode(message.code).then(function (result) {
            postMessage({ id: message.id, stdout: result.stdout, stderr: result.stderr });
        }, function (error) {
            postMessage({ id: message.id, stdout: "", stderr: "error: " + String((error && error.stack) || error) + "\n" });
        });
    }
    if (message.action === "check") {
        compileCxStage(message.code).then(function (compiled) {
            postMessage({ id: message.id, stdout: compiled.stdout, stderr: compiled.stderr });
        }, function (error) {
            postMessage({ id: message.id, stdout: "", stderr: "error: " + String((error && error.stack) || error) + "\n" });
        });
    }
};

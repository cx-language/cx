// Shared playground pipeline: C* source -> C -> WebAssembly -> output.
//
// This file contains no environment-specific code (no Worker, fetch, or file
// system access beyond the injected objects), so it can be loaded in browsers
// (via <script> or importScripts), in Web Workers, and in Node.js (for tests).
//
// The pipeline stages:
//   1. cxCompileToC: the C* frontend compiled to WebAssembly (cx-wasm) turns
//      C* source into C code plus diagnostics.
//   2. compileCToWasm: the in-browser C compiler (cc.wasm, built from xcc's
//      wcc) compiles the C code to a WebAssembly module, running on CxWasi.
//   3. runWasm: the compiled module is executed on CxWasi, capturing stdout.

(function (global) {
    "use strict";

    // Fresh file system for one run, pre-populated with the C toolchain files
    // (wccFiles maps absolute paths like "/usr/include/stdio.h" to Uint8Array
    // contents) and the C* standard library location expected by cx-wasm.
    function createCompilerFs(CxWasi, wccFiles) {
        var fs = new CxWasi.FileSystem();
        fs.mkdirs("/tmp");
        fs.mkdirs("/work");
        Object.keys(wccFiles).forEach(function (path) {
            fs.mkdirs(path.substring(0, path.lastIndexOf("/")) || "/");
            fs.writeFile(path, wccFiles[path]);
        });
        return fs;
    }

    // Stage 1: compile C* source to C. createModule instantiates a FRESH
    // compiler module for every compilation and is never reused afterwards:
    // like the command-line compiler (start, compile, exit), each compilation
    // gets pristine global state, and dropping the instance afterwards means
    // leaked memory is simply reclaimed. Only downloaded bytes are cached,
    // never module instances.
    // Returns {status, cCode, diagnostics} where cCode is present on success
    // and diagnostics is {stdout, stderr} captured from the compiler.
    async function compileCxToC(createModule, source) {
        var diagnostics = { stdout: "", stderr: "" };
        var cxWasmModule = await createModule({
            print: function (text) {
                diagnostics.stdout += text + "\n";
            },
            printErr: function (text) {
                diagnostics.stderr += text + "\n";
            },
        });
        // The source file exists so that diagnostics can show the surrounding
        // source lines.
        cxWasmModule.FS.writeFile("/main.cx", source);
        var result = JSON.parse(cxWasmModule.cxCompileToC(source, "/cx"));
        result.diagnostics = diagnostics;
        return result;
    }

    // Stage 2: compile C code to a WebAssembly module using cc.wasm.
    // Returns {exitCode, stdout, stderr, wasmBytes} (wasmBytes undefined on failure).
    async function compileCToWasm(CxWasi, fs, ccWasmBytes, cCode) {
        fs.writeFile("/work/main.c", cCode);
        // Remove leftovers from previous runs in the same file system.
        fs.unlink("/work/main.wasm");
        var compiler = new CxWasi.WasiProcess(fs, { args: ["cc", "/work/main.c", "-o", "/work/main.wasm"] });
        var result = await compiler.run(ccWasmBytes);
        var output = {
            exitCode: result.exitCode,
            stdout: result.stdout,
            stderr: result.stderr,
            wasmBytes: undefined,
        };
        if (result.exitCode === 0) {
            var wasmBytes = fs.readFile("/work/main.wasm");
            if (wasmBytes) {
                output.wasmBytes = wasmBytes;
            } else {
                output.exitCode = 1;
                output.stderr += "error: the C compiler produced no output file\n";
            }
        }
        // Best-effort cleanup of compiler temporaries.
        var temporaries = fs.readdir("/tmp") || [];
        temporaries.forEach(function (name) {
            fs.unlink("/tmp/" + name);
        });
        return output;
    }

    // Stage 3: run a compiled WebAssembly module, capturing its output.
    // Returns {exitCode, stdout, stderr}.
    async function runWasm(CxWasi, fs, wasmBytes) {
        var runner = new CxWasi.WasiProcess(fs, { args: [] });
        return await runner.run(wasmBytes);
    }

    var api = {
        createCompilerFs: createCompilerFs,
        compileCxToC: compileCxToC,
        compileCToWasm: compileCToWasm,
        runWasm: runWasm,
    };

    if (typeof module !== "undefined" && module.exports) {
        module.exports = api;
    } else {
        global.CxPipeline = api;
    }
})(typeof globalThis !== "undefined" ? globalThis : this);

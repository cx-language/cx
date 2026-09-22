# WebAssembly playground

This directory contains everything needed to run the cx online playground
without a server: the compiler frontend compiled to WebAssembly, a small
in-browser C toolchain, and the tests for the browser runtime.

## Architecture

```
cx source --(1)--> C code --(2)--> WebAssembly module --(3)--> output
```

1. **cx to cx* (`src/wasm/api.cpp` compiled with Emscripten to
   `cx-wasm.js`): runs the cx frontend (parsing, type checking, IR
   generation, C code generation) in the browser. Always generates goto-free
   dispatch code (`CompileToCOptions::dispatchMode`), and rejects C header
   imports (`CX_NO_C_IMPORT`). Diagnostics go to stdout/stderr, which the
   worker captures.
2. **C to WebAssembly** (`cc.wasm` + `wcc-files.zip`, built from
   [xcc](https://github.com/tyfkda/xcc)): xcc's single-file C compiler,
   self-hosted to WebAssembly, compiles the generated C to a WASI module.
   It runs on the minimal WASI implementation in `website/wasi-shim.js`.
3. **Run** (`website/wasi-shim.js`): the compiled module is executed with
   captured stdout/stderr.

The browser glue lives in `website/`: `playground.js` (main-thread API),
`playground-worker.js` (runs the pipeline off the main thread with a
timeout), `playground-pipeline.js` (shared, testable pipeline stages), and
`wasi-shim.js` (minimal WASI preview1 + in-memory file system).

Like the command-line compiler, the playground compilers run once and are
then discarded: every Run instantiates a fresh `cx-wasm` module (a pristine
address space, just like a new process, so no compiler state can leak
between runs) and a fresh `cc.wasm` instance on a fresh file system. Only
downloaded bytes are cached, never module instances.

## Building

Prerequisites: Emscripten SDK (`emcc`), CMake, Ninja or Make, Python 3,
`llvm-ar`, Node.js 20+.

```sh
./wasm/build.sh [output-dir]   # everything: cx-wasm.js + .wasm + .data, cc.wasm + wcc-files.zip
```

`build.sh` runs the two steps below; run them individually to rebuild only
one half:

```sh
./wasm/build-frontend.sh [output-dir]   # cx-wasm.js + .wasm + .data
./wasm/build-xcc-dist.sh [output-dir]   # cc.wasm + wcc-files.zip
```

`build-frontend.sh` builds LLVM's `LLVMSupport` for WebAssembly from source
(matching `LLVM_VERSION`, default 23.1.1) and links it with the embeddable
frontend translation units globbed in the script. The cx standard library
(`std/`) is preloaded into the module's file system at `/cx/std`.

`build-xcc-dist.sh` clones xcc at the pinned revision (`XCC_REF`), builds
`wcc` natively, self-hosts it to `cc.wasm` (`make wcc-gen2`), and packs its
headers and libraries into `wcc-files.zip`.

Both scripts default to writing into `wasm/dist/`, which `website/build-website.sh`
copies into the generated site. Build products are git-ignored; the website
CI builds them (see `.github/workflows/build.yml`).

To update the xcc pin, set `XCC_REF` to a newer commit, rebuild, and run the
tests below. xcc is MIT licensed; see its repository for attribution.

## Testing

```sh
# Needs an xcc checkout or a dist directory (stages the real cc.wasm):
node wasm/test-wasi-shim.mjs <xcc-dir-or-dist>
node wasm/test-pipeline.mjs <dist-dir>

# Full pipeline with a freshly built frontend (runs in the wasm CI job):
node wasm/smoke-playground.mjs wasm/dist
```

`test-wasi-shim.mjs` compiles the committed fixture (`wasm/fixtures/smoke.c`,
representative cx output) with the real `cc.wasm` on the real shim and runs
the result. `test-pipeline.mjs` additionally covers the shared pipeline
stages with a mocked frontend. `smoke-playground.mjs` covers stage 1 with the
real Emscripten build.

The compiler side is covered natively: `cmake --build build --target check`
runs the lit suite (including `--c-dispatch` mode) and
`test/compile-api/test-compile-api.cpp` covers the in-memory entry points.

## Limitations

Compared to the native compiler, the playground:

- rejects C header imports (`import "foo.h"`) with an error,
- rejects programs using `float80`, 128-bit integers, and other types the
  small C toolchain can't represent (with the toolchain's error message),
- has no file system access besides the standard streams, so programs
  reading files fail at runtime,
- kills programs running longer than 60 seconds (possible infinite loop).

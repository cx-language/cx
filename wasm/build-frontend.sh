#!/bin/sh
# Builds the C* frontend (parser, type checker, IR, C backend) to WebAssembly
# with Emscripten for the online playground. See wasm/README.md.
#
# Prerequisites: emcc, cmake, ninja or make, python3, llvm-ar (any version).
# Usage: ./wasm/build-frontend.sh [output-dir]
# Output: <output-dir>/cx-wasm.js, cx-wasm.wasm, cx-wasm.data
#
# Environment:
#   LLVM_VERSION  LLVM release to build LLVMSupport from (default 23.1.1).
#                 Must match the LLVM version the native build uses.
#   LLVM_JOBS     Parallel jobs for the LLVM build (default: nproc).

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:-"$ROOT/wasm/dist"}
LLVM_VERSION=${LLVM_VERSION:-23.1.1}
WORK="$ROOT/wasm/build"

cd "$ROOT"

# Resolve the output directory while the working directory is known.
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)

if ! command -v emcc >/dev/null; then
    echo "error: emcc not found, install the Emscripten SDK first (see wasm/README.md)" >&2
    exit 1
fi

NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
LLVM_JOBS=${LLVM_JOBS:-$NPROC}

LLVM_SRC="$WORK/llvm-project"
LLVM_BUILD="$WORK/llvm-build"

if [ ! -f "$LLVM_SRC/llvm/CMakeLists.txt" ]; then
    echo "Fetching LLVM $LLVM_VERSION source..."
    rm -rf "$LLVM_SRC"
    git clone --depth 1 --branch "llvmorg-$LLVM_VERSION" https://github.com/llvm/llvm-project.git "$LLVM_SRC"
fi

if [ ! -f "$LLVM_BUILD/lib/libLLVMSupport.a" ]; then
    echo "Building LLVMSupport for WebAssembly (this takes a while)..."
    emcmake cmake -S "$LLVM_SRC/llvm" -B "$LLVM_BUILD" -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DLLVM_TARGETS_TO_BUILD="" \
        -DLLVM_ENABLE_PROJECTS="" \
        -DLLVM_INCLUDE_TESTS=OFF \
        -DLLVM_INCLUDE_EXAMPLES=OFF \
        -DLLVM_INCLUDE_BENCHMARKS=OFF \
        -DLLVM_ENABLE_BINDINGS=OFF \
        -DLLVM_ENABLE_THREADS=OFF \
        -DLLVM_ENABLE_ZLIB=OFF \
        -DLLVM_ENABLE_ZSTD=OFF \
        -DLLVM_ENABLE_LIBXML2=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        -DLLVM_ENABLE_LIBEDIT=OFF \
        -DLLVM_ENABLE_FFI=OFF
    cmake --build "$LLVM_BUILD" --target LLVMSupport -j "$LLVM_JOBS"
fi

echo "Compiling the C* frontend to WebAssembly..."
mkdir -p "$WORK/frontend-obj" "$OUT"

# Translation units of the embeddable frontend. This is everything needed to
# parse, type-check, and generate C code, excluding the LLVM backend, the
# command-line driver, and the Clang-based C importer (C header imports are
# rejected with an error in WebAssembly builds).
SRCS="
    src/ast/ast-print.cpp
    src/ast/decl.cpp
    src/ast/expr.cpp
    src/ast/mangle.cpp
    src/ast/module.cpp
    src/ast/stmt.cpp
    src/ast/token.cpp
    src/ast/type.cpp
    src/backend/c-backend.cpp
    src/backend/ir.cpp
    src/backend/irgen.cpp
    src/backend/irgen-decl.cpp
    src/backend/irgen-expr.cpp
    src/backend/irgen-stmt.cpp
    src/driver/compile.cpp
    src/build/config.cpp
    src/parser/lex.cpp
    src/parser/parse.cpp
    src/sema/c-import.cpp
    src/sema/null-analyzer.cpp
    src/sema/typecheck.cpp
    src/sema/typecheck-decl.cpp
    src/sema/typecheck-expr.cpp
    src/sema/typecheck-stmt.cpp
    src/support/utility.cpp
    src/wasm/api.cpp
"

# Note: embind requires RTTI for type names unless told otherwise; the
# codebase is -fno-rtti throughout and the bindings only use plain strings,
# hence EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 below.
# Note: link with em++ (not emcc) so that the C++ standard library is linked.
# shellcheck disable=SC2086
em++ -std=c++20 -O2 -fno-rtti -fvisibility-inlines-hidden -fexceptions \
    -DCX_NO_C_IMPORT \
    -DEMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0 \
    -I"$ROOT/src" \
    -I"$LLVM_SRC/llvm/include" \
    -I"$LLVM_BUILD/include" \
    $SRCS \
    "$LLVM_BUILD/lib/libLLVMSupport.a" \
    -lembind \
    --no-entry \
    -sMODULARIZE=1 \
    -sEXPORT_NAME=CxWasm \
    -sEXPORTED_RUNTIME_METHODS=FS \
    -sINITIAL_MEMORY=64MB \
    -sALLOW_MEMORY_GROWTH=1 \
    -sSTACK_SIZE=8MB \
    -sENVIRONMENT=web,worker,node \
    -sINVOKE_RUN=0 \
    --preload-file "$ROOT/std@/cx/std" \
    -o "$OUT/cx-wasm.js"

echo "Wrote $OUT/cx-wasm.js (+ .wasm, .data)"
ls -la "$OUT" | grep cx-wasm

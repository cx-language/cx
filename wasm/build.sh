#!/bin/sh
# Builds everything the online playground needs: the C* frontend compiled to
# WebAssembly plus the in-browser C toolchain. See wasm/README.md.
#
# Usage: ./wasm/build.sh [output-dir]
# Output: <output-dir>/cx-wasm.js, cx-wasm.wasm, cx-wasm.data, cc.wasm, wcc-files.zip
#
# Environment: LLVM_VERSION, LLVM_JOBS (see build-frontend.sh),
#   XCC_REF (see build-xcc-dist.sh).

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:-"$ROOT/wasm/dist"}

"$ROOT/wasm/build-frontend.sh" "$OUT"
"$ROOT/wasm/build-xcc-dist.sh" "$OUT"

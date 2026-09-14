#!/bin/sh
# Builds the in-browser C toolchain distribution for the online playground:
# cc.wasm (xcc's single-file C to WebAssembly compiler, self-hosted) and
# wcc-files.zip (its headers and libraries). See wasm/README.md.
#
# Prerequisites: cc, make, llvm-ar, python3.
# Usage: ./wasm/build-xcc-dist.sh [output-dir]
# Output: <output-dir>/cc.wasm, <output-dir>/wcc-files.zip
#
# Environment:
#   XCC_REF  xcc git revision to build (default: the verified commit below).

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:-"$ROOT/wasm/dist"}
XCC_REF=${XCC_REF:-a596d27e7ebc3c898181e240f5de9e08202e1b90}
WORK="$ROOT/wasm/build"

if ! command -v llvm-ar >/dev/null; then
    echo "error: llvm-ar not found (needed to archive xcc's libraries)" >&2
    exit 1
fi

# Resolve the output directory now: the build below changes directories,
# so a relative path would otherwise end up in the wrong place.
mkdir -p "$OUT"
OUT=$(cd "$OUT" && pwd)

if [ ! -f "$WORK/xcc/Makefile" ]; then
    echo "Fetching xcc ($XCC_REF)..."
    rm -rf "$WORK/xcc"
    git clone https://github.com/tyfkda/xcc.git "$WORK/xcc"
fi

cd "$WORK/xcc"
git fetch --depth 1 origin "$XCC_REF" 2>/dev/null || true
git checkout "$XCC_REF"

echo "Building wcc and its libraries..."
make wcc wcc-libs

echo "Self-hosting wcc to cc.wasm..."
make wcc-gen2

mkdir -p "$OUT"
cp cc.wasm "$OUT/cc.wasm"

echo "Packing wcc-files.zip..."
XCC_DIR="$WORK/xcc" OUT_ZIP="$OUT/wcc-files.zip" python3 <<'EOF'
import os
import zipfile

xcc = os.environ["XCC_DIR"]
out_path = os.environ["OUT_ZIP"]

entries = {}

include_dir = os.path.join(xcc, "include")
for dirpath, _, filenames in os.walk(include_dir):
    for filename in filenames:
        host = os.path.join(dirpath, filename)
        guest = os.path.join("usr/include", os.path.relpath(host, include_dir))
        entries[guest] = host

entries["usr/include/wasi.h"] = os.path.join(xcc, "libsrc/_wasm/wasi.h")
entries["usr/lib/wcrt0.a"] = os.path.join(xcc, "lib/wcrt0.a")
entries["usr/lib/wlibc.a"] = os.path.join(xcc, "lib/wlibc.a")

with zipfile.ZipFile(out_path, "w", zipfile.ZIP_DEFLATED) as archive:
    for guest, host in sorted(entries.items()):
        archive.write(host, guest)

print("Wrote", out_path)
EOF

ls -la "$OUT" | grep -E "cc.wasm|wcc-files"

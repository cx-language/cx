#!/usr/bin/env python3
"""Verify the Windows JIT libc pin table covers every reachable extern.

src/backend/jit-pins.h pins libc symbols to host addresses so JITed code
binds the UCRT instead of legacy msvcrt.dll. A missing pin silently reintroduces the
heap/buffer mismatch, so this check fails when an `extern` visible under
`#if Windows` in std/ lacks a PIN entry (or vice versa, modulo extras like
memmove/memcmp and heap functions user code may reference). User and C-import
externs outside std/ are covered at run time: isMsvcrtAmbiguous keeps the
link-and-exec path for unpinned names that msvcrt.dll also exports.
"""

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Pinned but declared nowhere in std/: LLVM-emitted (memmove, memcmp) and
# heap-family functions user code or C imports may reference.
EXTRA_PINS = {"memmove", "memcmp", "calloc", "realloc", "_msize", "_strdup", "_wcsdup", "_aligned_malloc",
              "_aligned_free", "_aligned_realloc"}


def windows_externs():
    names = set()
    for path in sorted(ROOT.glob("std/**/*.cx")):
        active = [True]
        cond = []
        for lineno, line in enumerate(path.read_text().splitlines(), 1):
            stripped = line.strip()
            m = re.match(r"#if\s+(!)?(\w+)", stripped)
            if m:
                if m.group(2) == "Windows":
                    is_win = not bool(m.group(1))
                else:
                    # Unknown conditions (hasInclude, build flags) may be true or
                    # false on Windows; both branches count as active so their
                    # externs need explicit pins.
                    is_win = None
                cond.append(is_win)
                active.append(active[-1] and is_win is not False)
                continue
            if stripped.startswith("#else"):
                active.pop()
                prev = cond[-1]
                active.append(active[-1] and (prev is None or not prev))
                continue
            if stripped.startswith("#endif"):
                active.pop()
                cond.pop()
                continue
            if not active[-1]:
                continue
            code = line.split("//", 1)[0]
            m = re.search(r"\bextern\b.*?([A-Za-z_][A-Za-z0-9_]*)\s*\(", code)
            if m:
                names.add(m.group(1))
            elif re.search(r"\bextern\b", code):
                print(f"error: {path}:{lineno}: extern declaration not recognized (multi-line?)")
                sys.exit(1)
    return names


def pinned_names():
    src = (ROOT / "src/backend/jit-pins.h").read_text()
    return set(re.findall(r"^\s*PIN\(([A-Za-z_][A-Za-z0-9_]*),", src, re.MULTILINE))


def main():
    externs = windows_externs()
    pinned = pinned_names()
    missing = sorted(externs - pinned)
    extra = sorted(pinned - externs - EXTRA_PINS)
    ok = True
    for name in missing:
        print(f"error: std extern '{name}' has no PIN entry in src/backend/jit-pins.h")
        ok = False
    for name in extra:
        print(f"error: PIN entry '{name}' matches no std extern (nor EXTRA_PINS {sorted(EXTRA_PINS)})")
        ok = False
    if ok:
        print(f"ok: {len(pinned)} pins cover {len(externs)} Windows externs")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())

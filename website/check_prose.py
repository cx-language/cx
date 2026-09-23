#!/usr/bin/env python3
"""Fail if prose sources contain smart typography.

Banned: curly quotes, the ellipsis character, em dashes. Use straight
quotes (" and '), three dots (...), and hyphen (-) instead, so snippets
copy-paste cleanly and sources stay grep-friendly ASCII.

Usage:
    website/check_prose.py
"""

import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent

BANNED = {
    "\u201c": "left double quote",
    "\u201d": "right double quote",
    "\u2018": "left single quote",
    "\u2019": "right single quote",
    "\u2014": "em dash",
    "\u2026": "ellipsis",
}

# Prose inputs to the website plus src comments. Test/example fixtures may
# legitimately contain arbitrary Unicode, and website/lib is vendored.
SCAN_DIRS = ["docs", "std", "src", "website"]
SKIP_DIRS = {"lib", "build", ".generated"}
SCAN_FILES = ["README.md", "AGENTS.md"]


def tracked_files():
    """Return tracked paths relative to ROOT, or None if git is unavailable."""
    try:
        result = subprocess.run(
            ["git", "ls-files", "-z"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=False,
        )
    except (OSError, subprocess.SubprocessError):
        return None
    if result.returncode != 0:
        return None
    return {
        p for p in result.stdout.decode("utf-8", errors="surrogateescape").split("\0") if p
    }


def iter_files():
    tracked = tracked_files()
    for entry in SCAN_DIRS:
        base = ROOT / entry
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if not path.is_file():
                continue
            rel = path.relative_to(ROOT)
            if any(part in SKIP_DIRS for part in rel.parts):
                continue
            if tracked is not None and rel.as_posix() not in tracked:
                continue
            yield path
    for entry in SCAN_FILES:
        path = ROOT / entry
        if path.is_file():
            if tracked is not None and pathlib.Path(entry).as_posix() not in tracked:
                continue
            yield path


def main():
    failures = 0
    for path in iter_files():
        try:
            text = path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        for lineno, line in enumerate(text.splitlines(), start=1):
            for char in line:
                if char in BANNED:
                    print(f"{path.relative_to(ROOT)}:{lineno}: banned {BANNED[char]}")
                    failures += 1
                    break
    if failures:
        return 1
    print("Prose check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

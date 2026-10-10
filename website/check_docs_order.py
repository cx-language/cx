#!/usr/bin/env python3
"""Fail if the docs reading order is inconsistent.

The sidebar (website/toc.html) defines the guide reading order and the
Next buttons; the numeric prefixes on docs/*.md must list the same pages
in the same order, and "Where to go next" sections must only link forward
to later guide pages.

Usage:
    website/check_docs_order.py
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS_DIR = ROOT / "docs"
TOC = ROOT / "website" / "toc.html"

# Docs that are not guide chapters and need no sidebar entry.
# Compiler internals live in the repo only.
NON_GUIDE_FILES = {"CODE_OF_CONDUCT.md", "internals.md"}

SIDEBAR_LINK_RE = re.compile(r'href="\./([^"#]+)"')
# Both linked forms: ](./slug) and bare-relative ](slug), each with an
# optional #anchor. The lookahead keeps external URLs (https:) unmatched.
NEXT_LINK_RE = re.compile(r"\]\((?:\./)?([^)#:/]+)(?=[#)])")


def page_id(path):
    """Site-relative page id: 010-foo.md -> foo."""
    return re.sub(r"^[0-9]+-", "", path.stem)


def next_section_links(markdown):
    """./slug targets of the "Where to go next" section, if any."""
    match = re.search(r"^## Where to go next\n(.*?)(?=^## |\Z)", markdown, re.M | re.S)
    if not match:
        return []
    return NEXT_LINK_RE.findall(match.group(1))


def main():
    failures = 0
    sidebar = SIDEBAR_LINK_RE.findall(TOC.read_text(encoding="utf-8"))
    numbered = sorted(
        path for path in DOCS_DIR.glob("*.md")
        if path.name not in NON_GUIDE_FILES
    )
    expected = [page_id(path) for path in numbered]

    if len(set(sidebar)) != len(sidebar):
        print("sidebar lists a page twice")
        failures += 1
    for page in sidebar:
        if page not in expected:
            print(f"sidebar page without a docs source: {page}")
            failures += 1
    for path, page in zip(numbered, expected):
        if page not in sidebar:
            print(f"docs page missing from sidebar: {path.name}")
            failures += 1
    if not failures and sidebar != expected:
        print("sidebar order differs from numeric filename order:")
        print(f"  sidebar: {', '.join(sidebar)}")
        print(f"  numeric: {', '.join(expected)}")
        failures += 1

    position = {page: index for index, page in enumerate(sidebar)}
    for path in numbered:
        current = page_id(path)
        for target in next_section_links(path.read_text(encoding="utf-8")):
            if target not in position or position[target] <= position[current]:
                print(f"{path.name}: 'Where to go next' links back to {target}")
                failures += 1

    if failures:
        return 1
    print("Docs order check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

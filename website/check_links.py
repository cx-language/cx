#!/usr/bin/env python3
"""Fail if Markdown links point at missing files or anchors.

Covers the numbered docs filenames (the README links broke silently when
docs were renumbered) as well as the extensionless website slugs used
inside docs/ (./getting-started), relative files and images, and
same-file #anchors. External URLs are skipped: they flake offline.

Usage:
    website/check_links.py
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS_DIR = ROOT / "docs"

# Vendored and generated trees are not prose sources.
SKIP_DIRS = {"lib", "build", ".generated", "node_modules"}

INLINE_LINK_RE = re.compile(r"!?\[[^\]]*\]\(<([^<>]+)>|!?\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
REF_DEF_RE = re.compile(r"^\s{0,3}\[([^\]^][^\]]*)\]:\s*(\S+)", re.M)
REF_USE_RE = re.compile(r"!?\[[^\]]*\]\[([^\]^][^\]]*)\]")
HEADING_RE = re.compile(r"^#{1,6}\s+(.+?)\s*#*\s*$", re.M)
HTML_ANCHOR_RE = re.compile(r'<a\s+[^>]*(?:id|name)="([^"]+)"')
BRACE_ANCHOR_RE = re.compile(r"\{#([\w-]+)\}")


def strip_code(text):
    text = re.sub(r"```.*?```", "", text, flags=re.S)
    return re.sub(r"`[^`\n]+`", "", text)


def heading_slugs(text):
    # GitHub-style slugs; repeats get -1, -2, ... suffixes.
    slugs = set()
    counts = {}
    for match in HEADING_RE.finditer(text):
        heading = re.sub(r"\[([^\]]+)\]\([^)]+\)", r"\1", match.group(1))
        heading = re.sub(r"[^\w\s-]", "", heading.lower())
        slug = re.sub(r"\s+", "-", heading.strip())
        counts[slug] = counts.get(slug, 0) + 1
        if counts[slug] > 1:
            slug = f"{slug}-{counts[slug] - 1}"
        slugs.add(slug)
    slugs.update(HTML_ANCHOR_RE.findall(text))
    slugs.update(BRACE_ANCHOR_RE.findall(text))
    return slugs


def slugs_of(path):
    try:
        return heading_slugs(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError):
        return None


def docs_source(slug):
    direct = DOCS_DIR / (slug + ".md")
    if direct.is_file():
        return direct
    matches = sorted(DOCS_DIR.glob("[0-9]*-" + slug + ".md"))
    return matches[0] if matches else None


def check_anchor(source, target, path, failures):
    slugs = slugs_of(path)
    if slugs is None:
        failures.append(f"{source.relative_to(ROOT)}: cannot read {target}")
    else:
        anchor = target.split("#", 1)[1]
        if anchor not in slugs:
            failures.append(f"{source.relative_to(ROOT)}: missing anchor #{anchor} in {target}")


def check_target(source, raw_target, failures):
    if raw_target.startswith("<") and raw_target.endswith(">"):
        raw_target = raw_target[1:-1]
    target = raw_target.split()[0] if raw_target.split() else ""
    if re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", target):
        return
    if "#" in target:
        pathpart = target.split("#", 1)[0]
    else:
        pathpart = target
    if not pathpart:
        check_anchor(source, target, source, failures)
        return
    resolved = (source.parent / pathpart).resolve()
    try:
        resolved.relative_to(ROOT.resolve())
    except ValueError:
        failures.append(f"{source.relative_to(ROOT)}: link escapes repo: {target}")
        return
    if resolved.is_file() or resolved.is_dir():
        if "#" in target and resolved.suffix == ".md":
            check_anchor(source, target, resolved, failures)
        return
    slug = pathpart[2:] if pathpart.startswith("./") else pathpart
    if "/" not in slug and "." not in slug:
        page = docs_source(slug)
        if page is not None:
            if "#" in target:
                check_anchor(source, target, page, failures)
            return
    failures.append(f"{source.relative_to(ROOT)}: missing target {target}")


def main():
    files = [
        p
        for p in sorted(ROOT.rglob("*.md"))
        if not any(part in SKIP_DIRS for part in p.relative_to(ROOT).parts)
    ]
    failures = []
    for source in files:
        try:
            text = source.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue
        prose = strip_code(text)
        ref_defs = dict(REF_DEF_RE.findall(prose))
        for match in INLINE_LINK_RE.finditer(prose):
            check_target(source, match.group(1) or match.group(2), failures)
        for match in REF_USE_RE.finditer(prose):
            ref = match.group(1)
            if ref in ref_defs:
                check_target(source, ref_defs[ref], failures)
    for failure in failures:
        print(failure)
    if failures:
        return 1
    print("Link check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

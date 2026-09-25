#!/usr/bin/env python3
"""Build the site search index from the documentation sources.

Reads docs/*.md and the generated stdlib reference (.generated/*.md) and
writes them as a JSON array ('var CxSearchIndex') for website/search.js.
Markdown syntax is stripped, but code content stays searchable.

Usage:
    website/generate_search_index.py                  write website/build/search-index.js
    website/generate_search_index.py --output <path>  write elsewhere
"""

import argparse
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
DOCS_DIR = ROOT / "docs"
GENERATED_DIR = ROOT / "website" / ".generated"
OUTPUT = ROOT / "website" / "build" / "search-index.js"

LINK_RE = re.compile(r"\[((?:\\.|[^\]])*)\]\([^)]*\)")
IMAGE_RE = re.compile(r"!\[([^\]]*)\]\([^)]*\)")
ATTR_RE = re.compile(r"\{#[^}]*\}|\{[a-z]+=\"[^\"]*\"\}")
TAG_RE = re.compile(r"<[^>]+>")
# Capturing so split() retains the spans at odd indices.
CODE_SPAN_RE = re.compile(r"(`[^`]*`)")
HEADING_RE = re.compile(r"^#{1,6}\s*", re.M)
WHITESPACE_RE = re.compile(r"\s+")
# Backslash-escaped punctuation (as produced by heading_text in
# generate_std_docs.py): the backslash is markup, not content.
ESCAPE_RE = re.compile(r"\\([\\<>\[\]*_])")


def unwrap_link(match):
    return ESCAPE_RE.sub(r"\1", match.group(1))


def strip_tags_outside_code(text):
    """Strip HTML tags, leaving generics and operators in code spans alone."""
    parts = CODE_SPAN_RE.split(text)
    for i in range(0, len(parts), 2):
        parts[i] = TAG_RE.sub("", parts[i])
    return "".join(parts)


def strip_prose(text):
    # Tolerates unbalanced fences (their lines are markup either way).
    text = "\n".join(
        line for line in text.splitlines() if not line.startswith(("```", ":::"))
    )
    text = IMAGE_RE.sub(r"\1", text)
    text = LINK_RE.sub(unwrap_link, text)
    text = ATTR_RE.sub("", text)
    text = strip_tags_outside_code(text)
    text = text.replace("`", "")
    text = ESCAPE_RE.sub(r"\1", text)
    text = HEADING_RE.sub("", text)
    return text


# Capturing so split() retains the fence contents at odd indices.
FENCE_RE = re.compile(r"```[^\n]*\n(.*?)```", re.S)


def plain_text(markdown):
    """Strip Markdown syntax, keeping prose and code searchable.

    Fenced code is kept verbatim (only the fence lines go); everything
    else is stripped of links, tags, and other markup.
    """
    parts = FENCE_RE.split(markdown)
    for i in range(0, len(parts), 2):
        parts[i] = strip_prose(parts[i])
    return WHITESPACE_RE.sub(" ", "\n".join(parts)).strip()


def page_title(markdown):
    """First '# ' heading as plain text (generated pages wrap it in a link)."""
    for line in markdown.splitlines():
        if line.startswith("# "):
            return plain_text(line)
    return ""


def page_id(path, base_dir):
    """Site-relative page id: docs/x.md -> x, .generated/std.md -> std."""
    relpath = path.relative_to(base_dir).as_posix()
    if relpath == "std.md":
        return "std"
    return relpath.removesuffix(".md")


def collect_pages(docs_dir, generated_dir):
    """(path, base_dir) pairs in index order: guide first, then reference."""
    by_name = lambda path: path.as_posix()
    pages = [(path, docs_dir) for path in sorted(docs_dir.glob("*.md"), key=by_name)]
    pages += [(path, generated_dir) for path in sorted(generated_dir.rglob("*.md"), key=by_name)]
    return pages


def build_index(docs_dir, generated_dir):
    entries = []
    for path, base_dir in collect_pages(docs_dir, generated_dir):
        markdown = path.read_text()
        entries.append(
            {
                "id": page_id(path, base_dir),
                "title": page_title(markdown),
                "text": plain_text(markdown),
            }
        )
    return entries


def main(argv=None):
    parser = argparse.ArgumentParser(description="Build the site search index.")
    parser.add_argument("--docs-dir", default=DOCS_DIR, help="hand-written docs directory")
    parser.add_argument("--generated-dir", default=GENERATED_DIR, help="generated reference directory")
    parser.add_argument("--output", default=OUTPUT, help="output .js file")
    args = parser.parse_args(argv)

    entries = build_index(pathlib.Path(args.docs_dir), pathlib.Path(args.generated_dir))
    output = pathlib.Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text("var CxSearchIndex = " + json.dumps(entries) + ";\n")
    print(f"Wrote {len(entries)} search entries to {output}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Generate the website's standard library reference from the stdlib sources.

Reads the public declarations and /// doc comments in std/*.cx and writes
them as Markdown to a staging directory for the website build.
Private declarations are omitted.
Only single-line declarations are recognized; anything else is ignored.

Usage:
    website/generate_std_docs.py                    regenerate into website/.generated
    website/generate_std_docs.py --output-dir <dir> regenerate elsewhere
"""

import argparse
import html
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
STD_DIR = ROOT / "std"
STAGING_DIR = ROOT / "website" / ".generated"
SOURCE_URL = "https://github.com/cx-language/cx/blob/main/std"

OPERATOR_SLUGS = {
    "==": "eq",
    "!=": "ne",
    "<=": "le",
    ">=": "ge",
    "<": "lt",
    ">": "gt",
    "+": "plus",
    "[]=": "index-assign",
    "[]": "index",
    "[-]=": "index-from-end-assign",
    "[-]": "index-from-end",
}

TYPE_RE = re.compile(r"(struct|interface|enum)\s+(.+?)\s*\{")
FUNC_RE = re.compile(r"(operator(?:\[-?\]=?|==|!=|<=|>=|<|>|\+)|~?\w+)\s*(?:<[^;({>]*>)?\s*\(")
FIELD_RE = re.compile(r"(.+?)\s+(\w+)\s*;$")
CONST_RE = re.compile(r"const\s+(?:.*\s)?(\w+)\s*=")
VARIANT_RE = re.compile(r"(\w+),?$")


class Declaration:
    def __init__(self, signature, doc, source, line):
        self.signature = signature
        self.doc = doc
        self.source = source
        self.line = line


class Group:
    """Overloads sharing one name, e.g. the println free functions."""

    def __init__(self, name):
        self.name = name
        self.declarations = []

    def add(self, signature, doc, source, line):
        if signature not in [d.signature for d in self.declarations]:
            self.declarations.append(Declaration(signature, doc, source, line))

    @property
    def sources(self):
        seen = []
        for declaration in self.declarations:
            if declaration.source not in seen:
                seen.append(declaration.source)
        return seen


class Type:
    def __init__(self, kind, name, header, doc, source, line):
        self.kind = kind
        self.name = name
        self.header = header
        self.doc = doc
        self.source = source
        self.line = line
        self.members = {}

    def add_member(self, name, signature, doc, source, line):
        self.members.setdefault(name, Group(name)).add(signature, doc, source, line)


def member_name(signature):
    match = FUNC_RE.search(signature)
    if match:
        return match.group(1)
    match = FIELD_RE.match(signature)
    if match:
        return match.group(2)
    match = VARIANT_RE.match(signature)
    if match:
        return match.group(1)
    return None


def slug(name):
    if name.startswith("~"):
        return "dtor"
    if name.startswith("operator"):
        return "operator-" + OPERATOR_SLUGS[name.removeprefix("operator")]
    return name


def strip_line_comment(line):
    return line.split("//", 1)[0]


# Matches single-line 'c' and "s" literals, honoring backslash escapes, so
# that braces inside them don't affect nesting tracking.
_LITERAL_RE = re.compile(r"'(?:\\.|[^'\\])*'|\"(?:\\.|[^\"\\])*\"")


def blank_literals(code):
    return _LITERAL_RE.sub("''", code)


def parse_file(path):
    """Returns (types, functions, constants) declared in one .cx file."""
    types = []
    functions = {}
    constants = []
    current = None
    depth = 0
    doc = []

    def add_free(signature, doc_lines, lineno):
        if signature.startswith("const "):
            match = CONST_RE.match(signature)
            if match:
                constants.append((match.group(1), Declaration(signature, doc_lines, path.name, lineno)))
            return
        name = member_name(signature)
        if name is None:
            return
        functions.setdefault(name, Group(name)).add(signature, doc_lines, path.name, lineno)

    for lineno, line in enumerate(path.read_text().splitlines(), start=1):
        stripped = line.strip()
        if stripped.startswith("///"):
            doc.append(stripped[3:].removeprefix(" ").rstrip())
            continue
        code = strip_line_comment(line).strip()
        if not code:
            doc = []
            continue
        if code.startswith("#"):
            continue
        if code == "private" or code.startswith("private "):
            doc = []
        elif depth == 0 and (match := TYPE_RE.match(code)):
            kind = match.group(1)
            header = code.split("{", 1)[0].rstrip()
            name = header.split(None, 1)[1].split("<", 1)[0].split(":", 1)[0].strip()
            current = Type(kind, name, header, doc, path.name, lineno)
            types.append(current)
            doc = []
        elif depth == 0 and current is None:
            if code.startswith(("extern", "const", "using")) or "(" in code:
                add_free(code.split("{", 1)[0].rstrip(), doc, lineno)
            doc = []
        elif depth == 1 and current is not None and "(" in code:
            signature = code.split("{", 1)[0].rstrip()
            if (name := member_name(signature)) is not None:
                current.add_member(name, signature, doc, path.name, lineno)
            doc = []
        elif depth == 1 and current is not None and code.endswith(";"):
            if (name := member_name(code)) is not None:
                current.add_member(name, code, doc, path.name, lineno)
            doc = []
        elif depth == 1 and current is not None and current.kind == "enum":
            if (match := VARIANT_RE.match(code)) is not None:
                current.add_member(match.group(1), match.group(1), doc, path.name, lineno)
            doc = []
        depth_code = blank_literals(code)
        depth += depth_code.count("{") - depth_code.count("}")
        if depth == 0:
            current = None

    return types, functions, constants


def parse_std(std_dir):
    """Returns one (path, types, functions, constants, conditional) tuple per .cx file.

    Path is relative to std_dir with forward slashes; conditional marks files
    containing #if directives, whose declarations may be platform-specific.
    """
    pages = []
    for path in sorted(std_dir.rglob("*.cx")):
        types, functions, constants = parse_file(path)
        conditional = any(
            strip_line_comment(line).strip().startswith("#if") for line in path.read_text().splitlines()
        )
        pages.append((path.relative_to(std_dir).as_posix(), types, functions, constants, conditional))
    return pages


def page_name(relpath):
    return "std/" + relpath.removesuffix(".cx")


def display_name(relpath):
    return relpath.removesuffix(".cx")


def render_doc(doc, out):
    for line in doc:
        out.append(line)
    if doc:
        out.append("")


def render_signature(signature, out):
    out.append("```cs {.noRun}")
    out.append(signature)
    out.append("```")
    out.append("")


def render_group(group, out):
    for declaration in group.declarations:
        render_signature(declaration.signature, out)
        render_doc(declaration.doc, out)


def finish(out):
    return "\n".join(out).rstrip() + "\n"


def heading_text(text):
    return re.sub(r"([\\<>\[\]*_])", r"\\\1", text)


def source_url(relpath, line=None):
    url = f"{SOURCE_URL}/{relpath}"
    return f"{url}#L{line}" if line is not None else url


def heading_link(text, url):
    return f"[{text}]({url}){{target=\"_blank\"}}"


def render_file_page(relpath, types, functions, constants, conditional):
    # Types, functions, constants, and members render in file order.
    out = [f"# {heading_link(display_name(relpath), source_url(relpath))}", ""]
    if conditional:
        out += ["*Note: parts of this file are platform-conditional (`#if`).*", ""]

    for entry in types:
        out.append(
            f"## {heading_link(heading_text(entry.header), source_url(relpath, entry.line))}"
            f" {{#type-{entry.name}}}"
        )
        out.append("")
        render_doc(entry.doc, out)
        for name in entry.members:
            line = entry.members[name].declarations[0].line
            out.append(
                f"### {heading_link(heading_text(name), source_url(relpath, line))}"
                f" {{#{entry.name}-{slug(name)}}}"
            )
            out.append("")
            render_group(entry.members[name], out)

    for name in functions:
        line = functions[name].declarations[0].line
        out.append(
            f"## {heading_link(heading_text(name), source_url(relpath, line))} {{#fn-{slug(name)}}}"
        )
        out.append("")
        render_group(functions[name], out)

    for name, declaration in constants:
        out.append(
            f"## {heading_link(heading_text(name), source_url(relpath, declaration.line))}"
            f" {{#const-{name}}}"
        )
        out.append("")
        render_signature(declaration.signature, out)
        render_doc(declaration.doc, out)

    return finish(out)


def render_index(title, pages):
    out = [
        f"# {title}",
        "",
        "Auto-generated from the [standard library sources](https://github.com/cx-language/cx/tree/main/std)",
        "by [generate_std_docs.py](https://github.com/cx-language/cx/blob/main/website/generate_std_docs.py).",
        "",
    ]
    for relpath, types, functions, constants, _ in pages:
        names = (
            [f"`{t.name}`" for t in types]
            + [f"`{n}`" for n in functions]
            + [f"`{n}`" for n, _ in constants]
        )
        out.append(f"- [{relpath}](./{page_name(relpath)}): " + ", ".join(names))
    out.append("")
    return finish(out)


TOC_PLACEHOLDER = "<!--STD-PAGES-->"

# Sidebar grouping of the stdlib pages. Files listed here render nested
# under their category; anything else renders directly under the
# Standard library entry in filename order.
STD_CATEGORIES = [
    ("Primitive types", ["bool.cx", "char.cx", "integers.cx", "floats.cx", "never.cx"]),
    ("Strings", ["string.cx", "StringBuf.cx"]),
    (
        "Containers",
        [
            "Array.cx",
            "Slice.cx",
            "Box.cx",
            "List.cx",
            "Map.cx",
            "Optional.cx",
            "OrderedMap.cx",
            "OrderedSet.cx",
            "Queue.cx",
            "Set.cx",
        ],
    ),
    (
        "Ranges & iterators",
        [
            "Range.cx",
            "ClosedRange.cx",
            "Iterator.cx",
            "ArrayIterator.cx",
            "ByteIterator.cx",
            "ClosedRangeIterator.cx",
            "EnumeratedIterator.cx",
            "LineIterator.cx",
            "RangeIterator.cx",
            "StringIterator.cx",
        ],
    ),
    ("Interfaces", ["Comparable.cx", "Copyable.cx", "Hashable.cx", "Printable.cx"]),
    ("Input/output", ["stdio.cx", "FileStream.cx"]),
]


def category_slug(label):
    return "-".join(label.lower().replace("&", "").replace("/", " ").split())


def category_page(label):
    return "std/" + category_slug(label)


def render_toc_items(pages):
    entries = {
        relpath: f'<li><a href="./{page_name(relpath)}">{display_name(relpath)}</a></li>'
        for relpath, *_ in pages
    }
    categorized = {path for _, paths in STD_CATEGORIES for path in paths}
    items = []
    for label, paths in STD_CATEGORIES:
        members = [entries[path] for path in paths if path in entries]
        if not members:
            continue
        nested = "\n".join(f"                        {member}" for member in members)
        items.append(
            f'<li><a href="./{category_page(label)}">{html.escape(label)}</a>\n'
            f"                    <ul>\n{nested}\n                    </ul>\n                </li>"
        )
    items += [entries[relpath] for relpath, *_ in pages if relpath not in categorized]
    return [f"                {item}" for item in items]


def write_toc(output_dir, pages):
    toc = (ROOT / "website" / "toc.html").read_text()
    if TOC_PLACEHOLDER not in toc:
        raise SystemExit(f"website/toc.html lacks the {TOC_PLACEHOLDER} marker")
    items = "\n".join(render_toc_items(pages))
    (output_dir / "toc.html").write_text(re.sub(r"[ \t]*" + re.escape(TOC_PLACEHOLDER), items, toc))


def main(argv=None):
    parser = argparse.ArgumentParser(description="Generate the standard library reference.")
    parser.add_argument("--std-dir", default=STD_DIR, help="stdlib source directory")
    parser.add_argument("--output-dir", default=STAGING_DIR, help="output directory for generated Markdown")
    args = parser.parse_args(argv)

    pages = parse_std(pathlib.Path(args.std_dir))
    output_dir = pathlib.Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "std.md").write_text(render_index("Standard library reference", pages))
    for label, paths in STD_CATEGORIES:
        members = [page for page in pages if page[0] in paths]
        if members:
            category_path = output_dir / f"{category_page(label)}.md"
            category_path.parent.mkdir(parents=True, exist_ok=True)
            category_path.write_text(render_index(label, members))
    for relpath, types, functions, constants, conditional in pages:
        page_path = output_dir / f"{page_name(relpath)}.md"
        page_path.parent.mkdir(parents=True, exist_ok=True)
        page_path.write_text(render_file_page(relpath, types, functions, constants, conditional))
    write_toc(output_dir, pages)
    print(f"Wrote {len(pages)} stdlib pages to {output_dir}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

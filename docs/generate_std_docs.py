#!/usr/bin/env python3
"""Generate the website's standard library reference from the stdlib sources.

Reads the public declarations and /// doc comments in std/*.cx and writes
them as Markdown to docs/book/std.md. Private declarations are omitted.
Only single-line declarations are recognized; anything else is ignored.

Usage:
    docs/generate_std_docs.py           regenerate docs/book/std.md
    docs/generate_std_docs.py --check   exit 1 if docs/book/std.md is stale
"""

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
STD_DIR = ROOT / "std"
OUTPUT = ROOT / "docs" / "book" / "std.md"
SOURCE_URL = "https://github.com/emillaine/cx/blob/main/std"

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
}

TYPE_RE = re.compile(r"(struct|interface|enum)\s+(.+?)\s*\{")
FUNC_RE = re.compile(r"(operator(?:\[\]=?|==|!=|<=|>=|<|>|\+)|~?\w+)\s*(?:<[^;({]*>)?\s*\(")
FIELD_RE = re.compile(r"(.+?)\s+(\w+)\s*;$")
CONST_RE = re.compile(r"const\s+(?:.*\s)?(\w+)\s*=")
VARIANT_RE = re.compile(r"(\w+),?$")


class Declaration:
    def __init__(self, signature, doc, source):
        self.signature = signature
        self.doc = doc
        self.source = source


class Group:
    """Overloads sharing one name, e.g. the println free functions."""

    def __init__(self, name):
        self.name = name
        self.declarations = []

    def add(self, signature, doc, source):
        if signature not in [d.signature for d in self.declarations]:
            self.declarations.append(Declaration(signature, doc, source))

    @property
    def sources(self):
        seen = []
        for declaration in self.declarations:
            if declaration.source not in seen:
                seen.append(declaration.source)
        return seen


class Type:
    def __init__(self, kind, name, header, doc, source):
        self.kind = kind
        self.name = name
        self.header = header
        self.doc = doc
        self.source = source
        self.members = {}

    def add_member(self, name, signature, doc, source):
        self.members.setdefault(name, Group(name)).add(signature, doc, source)


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


def parse_file(path):
    """Returns (types, functions, constants) declared in one .cx file."""
    types = []
    functions = {}
    constants = []
    current = None
    depth = 0
    doc = []

    def add_free(signature, doc_lines):
        if signature.startswith("const "):
            match = CONST_RE.match(signature)
            if match:
                constants.append((match.group(1), Declaration(signature, doc_lines, path.name)))
            return
        name = member_name(signature)
        if name is None:
            return
        functions.setdefault(name, Group(name)).add(signature, doc_lines, path.name)

    for line in path.read_text().splitlines():
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
            current = Type(kind, name, header, doc, path.name)
            types.append(current)
            doc = []
        elif depth == 0 and current is None:
            if code.startswith(("extern", "const", "typealias")) or "(" in code:
                add_free(code.split("{", 1)[0].rstrip(), doc)
            doc = []
        elif depth == 1 and current is not None and "(" in code:
            signature = code.split("{", 1)[0].rstrip()
            if (name := member_name(signature)) is not None:
                current.add_member(name, signature, doc, path.name)
            doc = []
        elif depth == 1 and current is not None and code.endswith(";"):
            if (name := member_name(code)) is not None:
                current.add_member(name, code, doc, path.name)
            doc = []
        elif depth == 1 and current is not None and current.kind == "enum":
            if (match := VARIANT_RE.match(code)) is not None:
                current.add_member(match.group(1), match.group(1), doc, path.name)
            doc = []
        depth += code.count("{") - code.count("}")
        if depth == 0:
            current = None

    return types, functions, constants


def parse_all(std_dir):
    types = []
    functions = {}
    constants = []
    for path in sorted(std_dir.glob("*.cx")):
        file_types, file_functions, file_constants = parse_file(path)
        types.extend(file_types)
        for name, group in file_functions.items():
            merged = functions.setdefault(name, Group(name))
            for declaration in group.declarations:
                merged.add(declaration.signature, declaration.doc, declaration.source)
        constants.extend(file_constants)
    return types, functions, constants


def render_doc(doc, out):
    for line in doc:
        out.append(line)
    if doc:
        out.append("")


def render_group(group, out):
    for declaration in group.declarations:
        out.append(f"`{declaration.signature}`")
        out.append("")
        render_doc(declaration.doc, out)


def index_link(name, anchor):
    return f"[`{name}`](#{anchor})"


def render(types, functions, constants):
    by_name = lambda n: (n.lower(), n)
    types = sorted(types, key=lambda t: (t.name.lower(), t.name))
    function_names = sorted(functions, key=by_name)
    constants = sorted(constants, key=lambda c: c[0])

    out = [
        "# Standard library reference",
        "",
        "Auto-generated from the [standard library sources](https://github.com/emillaine/cx/tree/main/std)",
        "by [generate_std_docs.py](https://github.com/emillaine/cx/blob/main/docs/generate_std_docs.py).",
        "Do not edit by hand.",
        "",
        "**Types:** " + ", ".join(index_link(t.name, f"type-{t.name}") for t in types),
        "",
        "**Functions:** " + ", ".join(index_link(n, f"fn-{slug(n)}") for n in function_names),
        "",
        "**Constants:** " + ", ".join(index_link(n, f"const-{n}") for n, _ in constants),
        "",
    ]

    for entry in types:
        out.append(f"## `{entry.header}` {{#type-{entry.name}}}")
        out.append("")
        render_doc(entry.doc, out)
        out.append(f"Source: [{entry.source}]({SOURCE_URL}/{entry.source})")
        out.append("")
        for name in sorted(entry.members, key=by_name):
            out.append(f"### `{name}` {{#{entry.name}-{slug(name)}}}")
            out.append("")
            render_group(entry.members[name], out)

    for name in function_names:
        group = functions[name]
        out.append(f"## `{name}` {{#fn-{slug(name)}}}")
        out.append("")
        sources = ", ".join(f"[{s}]({SOURCE_URL}/{s})" for s in group.sources)
        out.append(f"Source: {sources}")
        out.append("")
        render_group(group, out)

    for name, declaration in constants:
        out.append(f"## `{name}` {{#const-{name}}}")
        out.append("")
        out.append(f"`{declaration.signature}`")
        out.append("")
        render_doc(declaration.doc, out)
        out.append(f"Source: [{declaration.source}]({SOURCE_URL}/{declaration.source})")
        out.append("")

    return "\n".join(out).rstrip() + "\n"


def main():
    parser = argparse.ArgumentParser(description="Generate the standard library reference.")
    parser.add_argument("--check", action="store_true", help="fail if the output is out of date")
    parser.add_argument("--std-dir", default=STD_DIR, help="stdlib source directory")
    parser.add_argument("--output", default=OUTPUT, help="output Markdown file")
    args = parser.parse_args()

    types, functions, constants = parse_all(pathlib.Path(args.std_dir))
    markdown = render(types, functions, constants)
    output = pathlib.Path(args.output)
    if args.check:
        if not output.exists() or output.read_text() != markdown:
            print(f"{output} is out of date, run docs/generate_std_docs.py to update it", file=sys.stderr)
            return 1
        print(f"{output} is up to date.")
        return 0
    output.write_text(markdown)
    print(f"Wrote {output} from {len(types)} types, {len(functions)} functions, {len(constants)} constants.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Generate the website's standard library reference from the stdlib sources.

Reads the public declarations and /// doc comments in std/*.cx and writes
them as Markdown to a staging directory for the website build.
Private declarations are omitted.
Only single-line declarations are recognized; anything else is ignored.

Usage:
    docs/generate_std_docs.py                       regenerate into docs/.generated
    docs/generate_std_docs.py --output-dir <dir>    regenerate elsewhere
"""

import argparse
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
STD_DIR = ROOT / "std"
STAGING_DIR = ROOT / "docs" / ".generated"
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
    return "std-" + relpath.removesuffix(".cx").replace("/", "-")


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


def by_name(name):
    return (name.lower(), name)


def render_file_page(relpath, types, functions, constants, conditional):
    types = sorted(types, key=lambda t: (t.name.lower(), t.name))
    function_names = sorted(functions, key=by_name)
    constants = sorted(constants, key=lambda c: c[0])

    out = [
        f"# {display_name(relpath)}",
        "",
        f"Auto-generated from [{relpath}]({SOURCE_URL}/{relpath}).",
        "",
    ]
    if conditional:
        out += ["*Note: parts of this file are platform-conditional (`#if`).*", ""]

    for entry in types:
        out.append(f"## `{entry.header}` {{#type-{entry.name}}}")
        out.append("")
        render_doc(entry.doc, out)
        for name in sorted(entry.members, key=by_name):
            out.append(f"### `{name}` {{#{entry.name}-{slug(name)}}}")
            out.append("")
            render_group(entry.members[name], out)

    for name in function_names:
        out.append(f"## `{name}` {{#fn-{slug(name)}}}")
        out.append("")
        render_group(functions[name], out)

    for name, declaration in constants:
        out.append(f"## `{name}` {{#const-{name}}}")
        out.append("")
        render_signature(declaration.signature, out)
        render_doc(declaration.doc, out)

    return finish(out)


def render_index(pages):
    out = [
        "# Standard library reference",
        "",
        "Auto-generated from the [standard library sources](https://github.com/emillaine/cx/tree/main/std)",
        "by [generate_std_docs.py](https://github.com/emillaine/cx/blob/main/docs/generate_std_docs.py).",
        "",
    ]
    for relpath, types, functions, constants, _ in pages:
        names = (
            [f"`{t.name}`" for t in sorted(types, key=lambda t: (t.name.lower(), t.name))]
            + [f"`{n}`" for n in sorted(functions, key=by_name)]
            + [f"`{n}`" for n, _ in sorted(constants, key=lambda c: c[0])]
        )
        out.append(f"- [{relpath}](./{page_name(relpath)}): " + ", ".join(names))
    out.append("")
    return finish(out)


TOC_PLACEHOLDER = "<!--STD-PAGES-->"


def render_toc_items(pages):
    return [
        f'                <li><a href="./{page_name(relpath)}">{display_name(relpath)}</a></li>'
        for relpath, *_ in pages
    ]


def write_toc(output_dir, pages):
    toc = (ROOT / "docs" / "toc.html").read_text()
    if TOC_PLACEHOLDER not in toc:
        raise SystemExit(f"docs/toc.html lacks the {TOC_PLACEHOLDER} marker")
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
    (output_dir / "std.md").write_text(render_index(pages))
    for relpath, types, functions, constants, conditional in pages:
        (output_dir / f"{page_name(relpath)}.md").write_text(
            render_file_page(relpath, types, functions, constants, conditional)
        )
    write_toc(output_dir, pages)
    print(f"Wrote {len(pages)} stdlib pages to {output_dir}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

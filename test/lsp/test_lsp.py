#!/usr/bin/env python3
"""End-to-end tests for cx-lsp.

Exercises the one-shot query subprocess (`cx-lsp --query`) and the stdio LSP
server (`cx-lsp` / `cx lsp`). Every test compiles in a fresh process, matching
the compiler's start-fast/compile-fast/exit design.

Usage:
    test_lsp.py --cx-lsp=<path-to-cx-lsp> --cx=<path-to-cx>
"""

import argparse
import concurrent.futures
import json
import os
import subprocess
import sys
import tempfile
import threading

FAILURES = []
FAILURES_LOCK = threading.Lock()


def check(name, condition, detail=""):
    with FAILURES_LOCK:
        if condition:
            print(f"PASS {name}")
        else:
            print(f"FAIL {name} {detail}")
            FAILURES.append(name)


def run_query(cx_lsp, query):
    proc = subprocess.run(
        [cx_lsp, "--query"],
        input=json.dumps(query).encode(),
        capture_output=True,
        timeout=60,
    )
    if proc.returncode != 0:
        raise AssertionError(f"--query exited with {proc.returncode}: {proc.stderr.decode()[:2000]}")
    envelope = json.loads(proc.stdout.decode())
    if not envelope.get("ok"):
        raise AssertionError(f"--query failed: {envelope.get('error')}")
    return envelope["result"]


GOOD_SOURCE = """\
int add(int x, int y) {
    return x + y;
}

void main() {
    int result = add(1, 2);
    println("done");
}

struct Point: Copyable {
    int x;
    int y;
}

enum Color {
    Red,
    Green,
}

void useTypes() {
    Point origin = Point(0, 0);
    println(origin.x);
}

struct Counter: Copyable {
    int count;
    void bump(int step) {
        int doubled = step * 2;
        println(doubled);
    }
}
"""

ALIAS_SOURCE = """\
struct Point {
    int x;
}
using PointAlias = Point;
void main() {
    PointAlias.
}
"""

BAD_SOURCE = """\
void main() {
    nosuchidentifier;
}
"""

REDEF_SOURCE = """\
int dup = 1;
int dup = 2;
void main() {
}
"""

# Regression tests for frontend crashes once triggered through the LSP:
# a statement-level error in the trailing call of a non-void function, and
# calling an interface as a constructor. Both must come back as diagnostics,
# not take down the query process.
TRAILING_CALL_ERROR_SOURCE = """\
int foo() {
    nosuchfunc();
}
"""

INTERFACE_CONSTRUCT_SOURCE = """\
interface Printable {
    void print(StringBuf* stream);
}

void main() {
    Printable();
}
"""

TOKENS_COMMENT_SOURCE = """\
// line comment
/* block
   comment */
#if Feature
void enabled() {}
#else
void disabled() {}
#endif
"""

UNTERMINATED_SOURCE = """\
void main() {
    return 1;
"""

TOKENS_GENERICS_SOURCE = """\
struct Box<T> {
    T value;
}
T first<T>(T x) {
    return x;
}
void main() {
    var xs = List<int>();
    int total = first(42);
}
"""

# The unterminated block comment keeps this from parsing, so every token
# below comes from the syntax scanner alone.
TOKENS_SCANNER_SOURCE = """\
var a = 0xFF + 0b101 + 0o17 + 123abc;
var b = 1.5 + 0..10 + 0.foo;
var c = "a\\"b";
var d = "unterminated;
var e = /* outer /* inner */ still */;
var f = /* unterminated;
"""

TOKENS_CRLF_SOURCE = "int add(int x, int y) {\r\n    return x;\r\n}\r\n"

GENERIC_DEF_SOURCE = """\
struct Box<T> {
    T value;
    T get() {
        return value;
    }
}
void main() {
    var b = Box<int>(value = 1);
    println(b.get());
    println(first(42));
}

T first<T>(T x) {
    return x;
}
"""

TOKENS_READONLY_SOURCE = """\
void show(const int p) {
    println(p);
}
void main() {
    show(1);
    var mutable = 1;
    const constant = 2;
    println(mutable + constant);
}
"""


def base_query(method, path, content, position=None):
    query = {
        "method": method,
        "file": path,
        "content": content,
        "openDocs": {},
        "workspaceFolders": [],
        "importSearchPaths": [],
        "defines": [],
    }
    if position is not None:
        query["position"] = {"line": position[0], "character": position[1]}
    return query


def test_query_modes(cx_lsp, path):
    result = run_query(cx_lsp, base_query("check", path, GOOD_SOURCE))
    check("query-check-clean", result["diagnostics"] == [], json.dumps(result["diagnostics"])[:500])

    result = run_query(cx_lsp, base_query("check", path, BAD_SOURCE))
    messages = [d["message"] for d in result["diagnostics"]]
    check("query-check-error", any("nosuchidentifier" in m for m in messages), json.dumps(messages)[:500])
    if result["diagnostics"]:
        diag = result["diagnostics"][0]
        check(
            "query-check-range",
            diag["range"]["start"] == {"line": 1, "character": 4},
            json.dumps(diag["range"]),
        )

    # `add` in `add(1, 2)` sits at 0-based line 5, characters 17-19.
    result = run_query(cx_lsp, base_query("hover", path, GOOD_SOURCE, (5, 18)))
    check("query-hover", "int add(int x, int y)" in result.get("hover", ""), result.get("hover", "")[:200])

    # Hover quotes user alias spellings as written.
    alias_source = "using Count = int;\nCount bump(Count x) {\n    return x + 1;\n}\n"
    result = run_query(cx_lsp, base_query("hover", path, alias_source, (1, 7)))
    check("query-hover-alias", "Count bump(Count x)" in result.get("hover", ""), result.get("hover", "")[:200])

    result = run_query(cx_lsp, base_query("definition", path, GOOD_SOURCE, (5, 18)))
    check("query-definition-found", result.get("found") is True, json.dumps(result)[:300])
    check(
        "query-definition-range",
        result.get("range", {}).get("start") == {"line": 0, "character": 4},
        json.dumps(result.get("range"))[:200],
    )

    result = run_query(cx_lsp, base_query("completion", path, GOOD_SOURCE, (6, 4)))
    labels = [item["label"] for item in result.get("items", [])]
    check("query-completion-add", "add" in labels)
    check("query-completion-keyword", "while" in labels)
    check("query-completion-stdlib", "println" in labels)

    # Completion inside a struct method sees the method's params and locals.
    result = run_query(cx_lsp, base_query("completion", path, GOOD_SOURCE, (27, 20)))
    labels = [item["label"] for item in result.get("items", [])]
    check("query-completion-method-param", "step" in labels)
    check("query-completion-method-local", "doubled" in labels)

    result = run_query(cx_lsp, base_query("completion", path, ALIAS_SOURCE, (5, 15)))
    labels = [item["label"] for item in result.get("items", [])]
    check("query-completion-alias-member", "x" in labels, json.dumps(labels)[:300])

    result = run_query(cx_lsp, base_query("documentSymbol", path, GOOD_SOURCE))
    names = [s["name"] for s in result.get("symbols", [])]
    check("query-symbols", "add" in names and "main" in names, json.dumps(names)[:300])
    kinds = {s["name"]: s["kind"] for s in result.get("symbols", [])}
    check("query-symbol-kinds", kinds.get("Point") == "struct" and kinds.get("Color") == "enum", json.dumps(kinds)[:300])
    check("query-symbol-enum-cases", "Red" in names and "Green" in names, json.dumps(names)[:300])

    result = run_query(cx_lsp, base_query("references", path, GOOD_SOURCE, (5, 18)))
    locations = [(r["range"]["start"]["line"], r["range"]["start"]["character"]) for r in result.get("references", [])]
    check("query-references", (0, 4) in locations and (5, 17) in locations, json.dumps(locations)[:300])

    # Hover/definition/references on a type annotation (`Point` at line 20).
    result = run_query(cx_lsp, base_query("hover", path, GOOD_SOURCE, (20, 5)))
    check("query-hover-type", "struct Point" in result.get("hover", ""), result.get("hover", "")[:200])

    result = run_query(cx_lsp, base_query("definition", path, GOOD_SOURCE, (20, 5)))
    check(
        "query-definition-type",
        result.get("found") is True and result.get("range", {}).get("start") == {"line": 9, "character": 7},
        json.dumps(result)[:300],
    )

    result = run_query(cx_lsp, base_query("references", path, GOOD_SOURCE, (20, 5)))
    locations = [(r["range"]["start"]["line"], r["range"]["start"]["character"]) for r in result.get("references", [])]
    check("query-references-type", (9, 7) in locations and (20, 4) in locations, json.dumps(locations)[:300])

    result = run_query(cx_lsp, base_query("semanticTokens", path, GOOD_SOURCE))
    ordered = result.get("tokens", [])
    tokens = {(t["line"], t["start"], t["length"], t["type"]): t["modifiers"] for t in ordered}
    check("query-tokens-function-def", tokens.get((0, 4, 3, "function")) == ["definition"])
    check("query-tokens-function-ref", tokens.get((5, 17, 3, "function")) == [])
    check("query-tokens-keyword", tokens.get((1, 4, 6, "keyword")) == [])
    check("query-tokens-string", tokens.get((6, 12, 6, "string")) == [])
    check("query-tokens-number", tokens.get((5, 21, 1, "number")) == [])
    check("query-tokens-struct-def", tokens.get((9, 7, 5, "struct")) == ["definition"])
    check("query-tokens-enum-member", tokens.get((15, 4, 3, "enumMember")) == ["definition"])
    check("query-tokens-method-def", tokens.get((26, 9, 4, "method")) == ["definition"])
    check("query-tokens-type-ref", tokens.get((20, 4, 5, "struct")) == [])
    check("query-tokens-property", tokens.get((21, 19, 1, "property")) == [])
    check("query-tokens-param-def", tokens.get((0, 12, 1, "parameter")) == ["definition"])
    check("query-tokens-interface", tokens.get((9, 14, 8, "interface")) == [])
    check("query-tokens-enum-def", tokens.get((14, 5, 5, "enum")) == ["definition"])
    check("query-tokens-param-ref", tokens.get((27, 22, 4, "parameter")) == [])
    check("query-tokens-variable-ref", tokens.get((28, 16, 7, "variable")) == [])
    check(
        "query-tokens-sorted",
        all((b["line"], b["start"]) >= (a["line"], a["start"]) for a, b in zip(ordered, ordered[1:])),
    )
    check(
        "query-tokens-no-overlap",
        all(b["line"] != a["line"] or b["start"] >= a["start"] + a["length"] for a, b in zip(ordered, ordered[1:])),
    )

    result = run_query(cx_lsp, base_query("semanticTokens", path, TOKENS_COMMENT_SOURCE))
    tokens = {(t["line"], t["start"], t["length"], t["type"]) for t in result.get("tokens", [])}
    check("query-tokens-line-comment", (0, 0, 15, "comment") in tokens)
    check("query-tokens-block-comment", (1, 0, 8, "comment") in tokens and (2, 0, 13, "comment") in tokens)
    check(
        "query-tokens-macro",
        (3, 0, 3, "macro") in tokens and (5, 0, 5, "macro") in tokens and (7, 0, 6, "macro") in tokens,
    )
    check("query-tokens-active-branch", (6, 5, 8, "function") in tokens and (4, 5, 7, "function") not in tokens)

    # A file that doesn't parse still highlights keywords/numbers/strings/comments.
    result = run_query(cx_lsp, base_query("semanticTokens", path, UNTERMINATED_SOURCE))
    tokens = {(t["line"], t["start"], t["length"], t["type"]) for t in result.get("tokens", [])}
    check("query-tokens-fallback", (1, 4, 6, "keyword") in tokens and (1, 11, 1, "number") in tokens)
    check(
        "query-tokens-fallback-syntax-only",
        {t["type"] for t in result.get("tokens", [])} <= {"keyword", "comment", "string", "number", "macro"},
        json.dumps(result.get("tokens"))[:300],
    )

    result = run_query(cx_lsp, base_query("semanticTokens", path, TOKENS_GENERICS_SOURCE))
    ordered = result.get("tokens", [])
    tokens = {(t["line"], t["start"], t["length"], t["type"]): t["modifiers"] for t in ordered}
    check("query-tokens-type-param-def", tokens.get((0, 11, 1, "typeParameter")) == ["definition"])
    check("query-tokens-type-param-ref", tokens.get((1, 4, 1, "typeParameter")) == [])
    check("query-tokens-func-type-param", tokens.get((3, 8, 1, "typeParameter")) == ["definition"])
    check("query-tokens-call-site-generic", tokens.get((7, 18, 3, "struct")) == [])
    check("query-tokens-generic-ctor", tokens.get((7, 13, 4, "method")) == [])
    check(
        "query-tokens-generics-sorted",
        all((b["line"], b["start"]) >= (a["line"], a["start"]) for a, b in zip(ordered, ordered[1:])),
    )
    check(
        "query-tokens-generics-no-overlap",
        all(b["line"] != a["line"] or b["start"] >= a["start"] + a["length"] for a, b in zip(ordered, ordered[1:])),
    )

    result = run_query(cx_lsp, base_query("semanticTokens", path, TOKENS_SCANNER_SOURCE))
    check("query-tokens-scanner-fallback", result["diagnostics"] != [])
    check(
        "query-tokens-scanner-exact",
        [(t["line"], t["start"], t["length"], t["type"]) for t in result.get("tokens", [])]
        == [
            (0, 0, 3, "keyword"),
            (0, 8, 4, "number"),
            (0, 15, 5, "number"),
            (0, 23, 4, "number"),
            (0, 30, 3, "number"),
            (1, 0, 3, "keyword"),
            (1, 8, 3, "number"),
            (1, 14, 1, "number"),
            (1, 17, 2, "number"),
            (1, 22, 1, "number"),
            (2, 0, 3, "keyword"),
            (2, 8, 6, "string"),
            (3, 0, 3, "keyword"),
            (3, 8, 14, "string"),
            (4, 0, 3, "keyword"),
            (4, 8, 29, "comment"),
            (5, 0, 3, "keyword"),
            (5, 8, 16, "comment"),
        ],
        json.dumps(result.get("tokens"))[:500],
    )

    result = run_query(cx_lsp, base_query("semanticTokens", path, TOKENS_CRLF_SOURCE))
    tokens = {(t["line"], t["start"], t["length"], t["type"]) for t in result.get("tokens", [])}
    check(
        "query-tokens-crlf",
        (0, 4, 3, "function") in tokens and (1, 4, 6, "keyword") in tokens and (1, 11, 1, "parameter") in tokens,
        json.dumps(result.get("tokens"))[:300],
    )

    # The scanner's keyword table must cover every keyword completion offers.
    result = run_query(cx_lsp, base_query("completion", path, "void main() {\n}\n", (0, 0)))
    keywords = sorted(item["label"] for item in result.get("items", []) if item["kind"] == "keyword")
    probe = " ".join(keywords)
    result = run_query(cx_lsp, base_query("semanticTokens", path, probe))
    highlighted = {(t["line"], t["start"], t["length"], t["type"]) for t in result.get("tokens", [])}
    expected = set()
    offset = 0
    for word in keywords:
        expected.add((0, offset, len(word), "keyword"))
        offset += len(word) + 1
    check("query-keyword-parity", highlighted == expected, json.dumps(sorted(highlighted))[:500])

    # Errors with notes surface as relatedInformation.
    result = run_query(cx_lsp, base_query("check", path, REDEF_SOURCE))
    noted = [d for d in result["diagnostics"] if d.get("relatedInformation")]
    check("query-related-information", len(noted) > 0, json.dumps(result["diagnostics"])[:500])

    # Erroneous code that used to crash the frontend must come back as
    # diagnostics instead (the query process must stay alive to answer).
    result = run_query(cx_lsp, base_query("check", path, TRAILING_CALL_ERROR_SOURCE))
    messages = [d["message"] for d in result["diagnostics"]]
    check("query-trailing-call-error", any("nosuchfunc" in m for m in messages), json.dumps(messages)[:500])

    result = run_query(cx_lsp, base_query("check", path, INTERFACE_CONSTRUCT_SOURCE))
    messages = [d["message"] for d in result["diagnostics"]]
    check(
        "query-interface-construct-error",
        any("cannot construct interface 'Printable'" in m for m in messages),
        json.dumps(messages)[:500],
    )

    # Malformed query must come back as a JSON error envelope, not a crash.
    proc = subprocess.run([cx_lsp, "--query"], input=b"not json", capture_output=True, timeout=60)
    envelope = json.loads(proc.stdout.decode())
    check("query-malformed", proc.returncode == 0 and envelope.get("ok") is False)


def test_generic_symbols(cx_lsp, path):
    # `value` in the generic method body (line 3) resolves to the field (line 1).
    result = run_query(cx_lsp, base_query("definition", path, GENERIC_DEF_SOURCE, (3, 16)))
    check(
        "query-definition-generic-method",
        result.get("found") is True and result.get("range", {}).get("start") == {"line": 1, "character": 6},
        json.dumps(result)[:300],
    )

    result = run_query(cx_lsp, base_query("hover", path, GENERIC_DEF_SOURCE, (3, 16)))
    check("query-hover-generic-method", "int Box<int>.value" in result.get("hover", ""), result.get("hover", "")[:200])

    # `x` in the generic function body (line 13) resolves to the parameter (line 12).
    result = run_query(cx_lsp, base_query("definition", path, GENERIC_DEF_SOURCE, (13, 11)))
    check(
        "query-definition-generic-function",
        result.get("found") is True and result.get("range", {}).get("start") == {"line": 12, "character": 13},
        json.dumps(result)[:300],
    )

    result = run_query(cx_lsp, base_query("hover", path, GENERIC_DEF_SOURCE, (13, 11)))
    check("query-hover-generic-function", "int x" in result.get("hover", ""), result.get("hover", "")[:200])


def test_readonly_tokens(cx_lsp, path):
    result = run_query(cx_lsp, base_query("semanticTokens", path, TOKENS_READONLY_SOURCE))
    tokens = {(t["line"], t["start"], t["length"], t["type"]): t["modifiers"] for t in result.get("tokens", [])}
    check("query-tokens-const-param-def", tokens.get((0, 20, 1, "parameter")) == ["definition", "readonly"])
    check("query-tokens-const-param-ref", tokens.get((1, 12, 1, "parameter")) == ["readonly"])
    check("query-tokens-var-def", tokens.get((5, 8, 7, "variable")) == ["definition"])
    check("query-tokens-const-def", tokens.get((6, 10, 8, "variable")) == ["definition", "readonly"])
    check("query-tokens-var-ref", tokens.get((7, 12, 7, "variable")) == [])
    check("query-tokens-const-ref", tokens.get((7, 22, 8, "variable")) == ["readonly"])


def test_completion_members(cx_lsp, path):
    def labels_for(content, pos):
        result = run_query(cx_lsp, base_query("completion", path, content, pos))
        return {item["label"]: item for item in result.get("items", [])}

    POINT = "struct Point: Copyable {\n    int x;\n    int y;\n    void move(int dx) {}\n}\n"

    # Struct instance: only its fields and methods, no keywords or globals.
    content = POINT + "void main() {\n    Point p = Point(0, 0);\n    p.\n}\n"
    items = labels_for(content, (7, 6))
    check("query-completion-member-struct", set(items) == {"x", "y", "move"}, json.dumps(sorted(items))[:300])
    check("query-completion-member-struct-kinds", items.get("x", {}).get("kind") == "field" and items.get("move", {}).get("kind") == "method")

    # Partial member name still restricts to members.
    content = POINT + "void main() {\n    Point p = Point(0, 0);\n    p.m\n}\n"
    items = labels_for(content, (7, 7))
    check("query-completion-member-partial", set(items) == {"x", "y", "move"}, json.dumps(sorted(items))[:300])

    # Enum type: only its cases.
    content = "enum Color {\n    Red,\n    Green,\n}\nvoid main() {\n    Color.\n}\n"
    items = labels_for(content, (5, 10))
    check("query-completion-member-enum", set(items) == {"Red", "Green"}, json.dumps(sorted(items))[:300])

    # Anonymous struct value: only its field names.
    content = "void main() {\n    var t = (a = 1, b = 2);\n    t.\n}\n"
    items = labels_for(content, (2, 6))
    check("query-completion-member-anonymous-struct", set(items) == {"a", "b"}, json.dumps(sorted(items))[:300])

    # Array value: only its member functions.
    content = "void main() {\n    var arr = [1, 2, 3];\n    arr.\n}\n"
    items = labels_for(content, (2, 8))
    check("query-completion-member-array", set(items) == {"data", "size", "iterator"}, json.dumps(sorted(items))[:300])

    # Array pointer: only the compiler-known data() method.
    content = "void main() {\n    int[*] p = [1, 2, 3];\n    p.\n}\n"
    items = labels_for(content, (2, 6))
    check("query-completion-member-array-pointer", set(items) == {"data"}, json.dumps(sorted(items))[:300])

    # Pointer and optional receivers unwrap to the pointee/wrapped members.
    content = POINT + "void main() {\n    Point p = Point(0, 0);\n    Point* ptr = &p;\n    ptr.\n}\n"
    items = labels_for(content, (8, 8))
    check("query-completion-member-pointer", set(items) == {"x", "y", "move"}, json.dumps(sorted(items))[:300])

    content = POINT + "void main() {\n    Point? opt = Point(0, 0);\n    opt.\n}\n"
    items = labels_for(content, (7, 8))
    check("query-completion-member-optional", set(items) == {"x", "y", "move"}, json.dumps(sorted(items))[:300])

    # `this.` inside a method completes the enclosing type.
    content = "struct Point: Copyable {\n    int x;\n    int y;\n    void move(int dx) {\n        this.\n    }\n}\n"
    items = labels_for(content, (4, 13))
    check("query-completion-member-this", set(items) == {"x", "y", "move"}, json.dumps(sorted(items))[:300])

    # Chained member and call-result receivers.
    content = "struct Point: Copyable {\n    int x;\n    int y;\n}\nstruct Wrapper: Copyable {\n    Point inner;\n}\nvoid main() {\n    Wrapper w = Wrapper(Point(1, 2));\n    w.inner.\n}\n"
    items = labels_for(content, (9, 12))
    check("query-completion-member-chained", set(items) == {"x", "y"}, json.dumps(sorted(items))[:300])

    content = POINT + "Point getPoint() {\n    return Point(0, 0);\n}\nvoid main() {\n    getPoint().\n}\n"
    items = labels_for(content, (9, 15))
    check("query-completion-member-call", set(items) == {"x", "y", "move"}, json.dumps(sorted(items))[:300])

    # Overloaded methods list every overload, not just the first.
    content = 'void main() {\n    var buf = StringBuf("hi");\n    buf.\n}\n'
    result = run_query(cx_lsp, base_query("completion", path, content, (2, 8)))
    details = sorted(item["detail"] for item in result.get("items", []) if item["label"] == "append")
    check(
        "query-completion-member-overloads",
        details
        == [
            "void StringBuf.append(RepeatIterator<string> repetitions)",
            "void StringBuf.append(T& value)",
            "void StringBuf.append(char c)",
            "void StringBuf.append(string s)",
        ],
        json.dumps(details)[:300],
    )
    flags = {}
    for item in result.get("items", []):
        flags.setdefault(item["label"], []).append(item.get("hasParams"))
    check(
        "query-completion-member-has-params",
        flags.get("append") == [True, True, True, True] and flags.get("empty") == [False],
        json.dumps({k: flags.get(k) for k in ("append", "empty")})[:300],
    )

    # Overloaded top-level functions list every overload too.
    content = "int add(int x, int y) {\n    return x + y;\n}\nint add(int x) {\n    return x;\n}\nvoid main() {\n\n}\n"
    result = run_query(cx_lsp, base_query("completion", path, content, (7, 0)))
    details = sorted(item["detail"] for item in result.get("items", []) if item["label"] == "add")
    check(
        "query-completion-overloads",
        details == ["int add(int x)", "int add(int x, int y)"],
        json.dumps(details)[:300],
    )
    adds = [item.get("hasParams") for item in result.get("items", []) if item["label"] == "add"]
    mains = [item.get("hasParams") for item in result.get("items", []) if item["label"] == "main"]
    check(
        "query-completion-has-params",
        adds == [True, True] and mains == [False],
        json.dumps({"add": adds, "main": mains})[:300],
    )


def test_package_dedup(cx_lsp):
    # A package directory that the "std" import resolves to (like std/
    # itself) must be analyzed once, not once as the open package and once
    # as the import. Uses an isolated directory so its files can't leak into
    # other tests via sibling loading.
    with tempfile.TemporaryDirectory() as directory:
        package = os.path.join(directory, "std")
        os.mkdir(package)
        # Uses `void` so the shadowing package needs no std declarations.
        with open(os.path.join(package, "defs.cx"), "w") as file:
            file.write("void answer() {\n}\n")
        use_path = os.path.join(package, "use.cx")

        def package_query(method, content, position=None):
            query = base_query(method, use_path, content, position)
            query["importSearchPaths"] = [directory]
            return query

        content = "void doubled() {\n    answer();\n}\n"
        with open(use_path, "w") as file:
            file.write(content)

        result = run_query(cx_lsp, package_query("check", content))
        check("query-package-clean", result["diagnostics"] == [], json.dumps(result["diagnostics"])[:500])

        result = run_query(cx_lsp, package_query("hover", content, (1, 6)))
        check("query-package-hover", "void answer()" in result.get("hover", ""), result.get("hover", "")[:200])

        result = run_query(cx_lsp, package_query("references", content, (1, 6)))
        locations = sorted(
            (r["range"]["start"]["line"], r["range"]["start"]["character"]) for r in result.get("references", [])
        )
        # Definition in defs.cx plus the single use; the definition must not
        # be listed twice via the package's dual role as main and import.
        defs_path = os.path.join(package, "defs.cx")
        files = sorted({r["file"] for r in result.get("references", [])})
        check(
            "query-package-references",
            locations == [(0, 5), (1, 4)] and files == [defs_path, use_path],
            json.dumps(result.get("references"))[:500],
        )

        # An explicit self-import must resolve to the package itself and
        # terminate instead of duplicating or recursing.
        self_content = 'import "std";\n' + content
        with open(use_path, "w") as file:
            file.write(self_content)
        result = run_query(cx_lsp, package_query("check", self_content))
        check(
            "query-package-self-import",
            result["diagnostics"] == [],
            json.dumps(result["diagnostics"])[:500],
        )


def test_build_file_modes(cx_lsp):
    # Files under a build.cx file are analyzed as its target root
    # (even from a nested subdirectory); files without a build file are
    # standalone, like `cx file.cx`.
    with tempfile.TemporaryDirectory() as directory:
        root = os.path.join(directory, "proj")
        subdir = os.path.join(root, "sub")
        os.makedirs(subdir)
        with open(os.path.join(root, "build.cx"), "w") as file:
            file.write('var name = "testproj"\n')
        with open(os.path.join(root, "a.cx"), "w") as file:
            file.write("int answer() {\n    return 42;\n}\n")
        nested_path = os.path.join(subdir, "b.cx")
        nested_content = "int doubled() {\n    return answer() * 2;\n}\n"
        with open(nested_path, "w") as file:
            file.write(nested_content)

        result = run_query(cx_lsp, base_query("check", nested_path, nested_content))
        check(
            "query-build-file",
            result["diagnostics"] == [],
            json.dumps(result["diagnostics"])[:500],
        )

        result = run_query(cx_lsp, base_query("hover", nested_path, nested_content, (1, 12)))
        check("query-build-file-hover", "int answer()" in result.get("hover", ""), result.get("hover", "")[:200])

    with tempfile.TemporaryDirectory() as directory:
        with open(os.path.join(directory, "c.cx"), "w") as file:
            file.write("int answer() {\n    return 42;\n}\n")
        lone_path = os.path.join(directory, "d.cx")
        lone_content = "int doubled() {\n    return answer() * 2;\n}\n"
        with open(lone_path, "w") as file:
            file.write(lone_content)

        result = run_query(cx_lsp, base_query("check", lone_path, lone_content))
        messages = [d["message"] for d in result["diagnostics"]]
        check(
            "query-standalone",
            any("unknown identifier 'answer'" in m for m in messages),
            json.dumps(messages)[:500],
        )

    # Vendored packages are imported by name, not analyzed as part of the
    # importing module: no import errors and no redefinition errors. A build.cx
    # below the package root is an ordinary source file.
    with tempfile.TemporaryDirectory() as directory:
        root = os.path.join(directory, "vproj")
        vendordir = os.path.join(root, "vendor", "greet")
        subdir = os.path.join(root, "sub")
        os.makedirs(vendordir)
        os.makedirs(subdir)
        with open(os.path.join(root, "build.cx"), "w") as file:
            file.write('var name = "vproj"\n')
        with open(os.path.join(vendordir, "greet.cx"), "w") as file:
            file.write('void greet() {\n    println("hi");\n}\n\nvoid unusedHelper() {\n}\n')
        main_path = os.path.join(root, "main.cx")
        main_content = "import greet;\n\nvoid main() {\n    greet();\n    other();\n}\n"
        with open(main_path, "w") as file:
            file.write(main_content)
        nested_path = os.path.join(subdir, "build.cx")
        nested_content = "import greet;\n\nvoid other() {\n    greet();\n}\n"
        with open(nested_path, "w") as file:
            file.write(nested_content)

        result = run_query(cx_lsp, base_query("check", main_path, main_content))
        check(
            "query-vendored-import",
            result["diagnostics"] == [],
            json.dumps(result["diagnostics"])[:500],
        )

        result = run_query(cx_lsp, base_query("check", nested_path, nested_content))
        check(
            "query-vendored-import-nested",
            result["diagnostics"] == [],
            json.dumps(result["diagnostics"])[:500],
        )


def test_fetched_dependency(cx_lsp):
    # Dependencies resolve from ~/.cx with their build file applied: the import
    # resolves and the dependency's defines select its #if branches (the #else
    # branch is a type error, so any failure surfaces as diagnostics).
    with tempfile.TemporaryDirectory() as home:
        depdir = os.path.join(home, ".cx", "dependencies", "shapes@v1")
        os.makedirs(depdir)
        with open(os.path.join(depdir, "build.cx"), "w") as file:
            file.write('var defines = ["SHAPES_ROUND"]\n')
        with open(os.path.join(depdir, "shape.cx"), "w") as file:
            file.write(
                "#if SHAPES_ROUND\n"
                "void describe() {\n"
                '    println("round");\n'
                "}\n"
                "#else\n"
                "void describe() {\n"
                "    nosuchidentifier;\n"
                "}\n"
                "#endif\n"
            )
        with tempfile.TemporaryDirectory() as directory:
            root = os.path.join(directory, "proj")
            os.makedirs(root)
            with open(os.path.join(root, "build.cx"), "w") as file:
                file.write('var dependencies = [(package = "shapes", url = "https://example.com/shapes.git", version = "v1")]\n')
            main_path = os.path.join(root, "main.cx")
            main_content = "import shapes;\n\nvoid main() {\n    describe();\n}\n"
            with open(main_path, "w") as file:
                file.write(main_content)

            old_home = os.environ.get("HOME")
            os.environ["HOME"] = home
            try:
                result = run_query(cx_lsp, base_query("check", main_path, main_content))
            finally:
                if old_home is None:
                    del os.environ["HOME"]
                else:
                    os.environ["HOME"] = old_home
            check(
                "query-fetched-dependency",
                result["diagnostics"] == [],
                json.dumps(result["diagnostics"])[:500],
            )


def test_pkg_config_headers(cx_lsp):
    # Header search paths from build.cx pkg-config dependencies apply to
    # C-header imports, like in the driver. Runs serially: it swaps the
    # process-global PKG_CONFIG_PATH.
    import shutil

    with tempfile.TemporaryDirectory() as directory:
        root = os.path.join(directory, "proj")
        includedir = os.path.join(root, "include")
        os.makedirs(os.path.join(includedir, "mypkg"))
        with open(os.path.join(includedir, "mypkg", "widget.h"), "w") as file:
            file.write("int widgetValue();\n")
        pcdir = os.path.join(root, "pc")
        os.makedirs(pcdir)
        with open(os.path.join(pcdir, "mypkg.pc"), "w") as file:
            file.write(
                "prefix="
                + root.replace(os.sep, "/")
                + "\n"
                + "includedir=${prefix}/include\n"
                + "Name: mypkg\n"
                + "Description: fake pkg for LSP test\n"
                + "Version: 1.0\n"
                + "Cflags: -I${includedir}\n"
            )
        build_path = os.path.join(root, "build.cx")
        main_path = os.path.join(root, "main.cx")
        main_content = 'import "mypkg/widget.h";\nvoid main() {\n    println(widgetValue());\n}\n'
        with open(main_path, "w") as file:
            file.write(main_content)

        # A failing pkg-config query degrades to unresolved headers, not a crash.
        with open(build_path, "w") as file:
            file.write('var name = "pkgtest"\nvar pkgConfigDependencies = ["definitely-not-a-real-package"]\n')
        result = run_query(cx_lsp, base_query("check", main_path, main_content))
        messages = [d["message"] for d in result["diagnostics"]]
        check(
            "query-pkg-config-failure-degrades",
            any("couldn't find C header file" in m for m in messages),
            json.dumps(messages)[:500],
        )

        if sys.platform == "win32" or shutil.which("pkg-config") is None:
            print("SKIP query-pkg-config-headers (no pkg-config on Windows CI)")
            return

        with open(build_path, "w") as file:
            file.write('var name = "pkgtest"\nvar pkgConfigDependencies = ["mypkg"]\n')
        old_path = os.environ.get("PKG_CONFIG_PATH")
        os.environ["PKG_CONFIG_PATH"] = pcdir + (os.pathsep + old_path if old_path else "")
        try:
            result = run_query(cx_lsp, base_query("check", main_path, main_content))
        finally:
            if old_path is None:
                del os.environ["PKG_CONFIG_PATH"]
            else:
                os.environ["PKG_CONFIG_PATH"] = old_path
        check(
            "query-pkg-config-headers",
            result["diagnostics"] == [],
            json.dumps(result["diagnostics"])[:500],
        )


class LspSession:
    def __init__(self, command):
        self.proc = subprocess.Popen(
            command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )

    def send(self, message):
        body = json.dumps(message).encode()
        self.proc.stdin.write(b"Content-Length: %d\r\n\r\n" % len(body) + body)
        self.proc.stdin.flush()

    def send_raw(self, data):
        self.proc.stdin.write(data)
        self.proc.stdin.flush()

    def read(self):
        headers = {}
        while True:
            line = self.proc.stdout.readline().decode()
            if not line:
                raise AssertionError("server closed stdout")
            if line in ("\r\n", "\n"):
                break
            if ":" in line:
                key, value = line.split(":", 1)
                headers[key.strip().lower()] = value.strip()
        return json.loads(self.proc.stdout.read(int(headers["content-length"])))

    def close(self):
        try:
            _, stderr = self.proc.communicate(timeout=10)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            raise
        return self.proc.returncode, stderr.decode()[:2000]


def run_group(name, group):
    try:
        group()
    except Exception as error:
        check(f"group-{name}-crashed", False, f"{type(error).__name__}: {error}")


def decode_semantic_data(data):
    tokens = []
    line, start = 0, 0
    first = True
    for i in range(0, len(data), 5):
        delta_line, delta_start, length, token_type, modifiers = data[i : i + 5]
        if delta_line != 0 or first:
            start = delta_start
        else:
            start += delta_start
        line += delta_line
        first = False
        tokens.append((line, start, length, token_type, modifiers))
    return tokens


def test_server(command, path, label):
    session = LspSession(command)
    # Malformed input must be skipped without killing the session.
    session.send_raw(b"this is not a framed message\r\n\r\n")
    caps = {"textDocument": {"completion": {"completionItem": {"snippetSupport": True}}}}
    session.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {"capabilities": caps}})
    response = session.read()
    capabilities = response["result"]["capabilities"]
    check(f"{label}-initialize", capabilities.get("hoverProvider") is True, json.dumps(capabilities)[:300])
    legend = capabilities.get("semanticTokensProvider", {}).get("legend", {})
    check(
        f"{label}-semantic-legend",
        "keyword" in legend.get("tokenTypes", [])
        and "function" in legend.get("tokenTypes", [])
        and "readonly" in legend.get("tokenModifiers", []),
        json.dumps(legend)[:300],
    )

    session.send({"jsonrpc": "2.0", "method": "initialized", "params": {}})
    uri = "file://" + path
    session.send(
        {
            "jsonrpc": "2.0",
            "method": "textDocument/didOpen",
            "params": {"textDocument": {"uri": uri, "languageId": "cx", "version": 1, "text": GOOD_SOURCE}},
        }
    )
    notification = session.read()
    check(
        f"{label}-didopen-diagnostics",
        notification["method"] == "textDocument/publishDiagnostics" and notification["params"]["diagnostics"] == [],
        json.dumps(notification)[:300],
    )

    session.send(
        {
            "jsonrpc": "2.0",
            "id": 2,
            "method": "textDocument/hover",
            "params": {"textDocument": {"uri": uri}, "position": {"line": 5, "character": 18}},
        }
    )
    response = session.read()
    check(
        f"{label}-hover",
        "int add(int x, int y)" in response["result"]["contents"]["value"],
        json.dumps(response)[:300],
    )

    session.send(
        {
            "jsonrpc": "2.0",
            "id": 20,
            "method": "textDocument/completion",
            "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 4}},
        }
    )
    response = session.read()
    items = {item["label"]: item for item in response["result"]}
    check(
        f"{label}-completion-call-parens",
        items.get("add", {}).get("insertText") == "add(" and "insertTextFormat" not in items.get("add", {}),
        json.dumps(items.get("add"))[:300],
    )
    operators = [item for name, item in items.items() if name in ("+", "==", "<")]
    check(
        f"{label}-completion-operator",
        operators != [] and all("insertText" not in item for item in operators),
        json.dumps(operators)[:300],
    )

    member_uri = uri + ".member.cx"
    member_content = 'void main() {\n    var buf = StringBuf("hi");\n    buf.\n}\n'
    session.send(
        {
            "jsonrpc": "2.0",
            "method": "textDocument/didOpen",
            "params": {"textDocument": {"uri": member_uri, "languageId": "cx", "version": 1, "text": member_content}},
        }
    )
    notification = session.read()
    check(
        f"{label}-completion-member-open",
        notification["method"] == "textDocument/publishDiagnostics",
        json.dumps(notification)[:200],
    )
    session.send(
        {
            "jsonrpc": "2.0",
            "id": 21,
            "method": "textDocument/completion",
            "params": {"textDocument": {"uri": member_uri}, "position": {"line": 2, "character": 8}},
        }
    )
    response = session.read()
    appends = [item for item in response["result"] if item["label"] == "append"]
    check(
        f"{label}-completion-member-call-parens",
        sorted(item.get("insertText", "") for item in appends) == ["append(", "append(", "append(", "append("]
        and all("insertTextFormat" not in item for item in appends),
        json.dumps(appends)[:300],
    )
    empties = [item for item in response["result"] if item["label"] == "empty"]
    check(
        f"{label}-completion-member-noargs",
        [item.get("insertText", "") for item in empties] == ["empty()"],
        json.dumps(empties)[:300],
    )

    session.send(
        {
            "jsonrpc": "2.0",
            "method": "textDocument/didChange",
            "params": {
                "textDocument": {"uri": member_uri, "version": 2},
                "contentChanges": [{"text": 'void main() {\n    var buf = StringBuf("hi");\n    buf.append()\n}\n'}],
            },
        }
    )
    notification = session.read()
    session.send(
        {
            "jsonrpc": "2.0",
            "id": 22,
            "method": "textDocument/completion",
            "params": {"textDocument": {"uri": member_uri}, "position": {"line": 2, "character": 14}},
        }
    )
    response = session.read()
    appends = [item for item in response["result"] if item["label"] == "append"]
    check(
        f"{label}-completion-no-dup-parens",
        appends != [] and all("insertText" not in item for item in appends),
        json.dumps(appends)[:300],
    )
    session.send({"jsonrpc": "2.0", "method": "textDocument/didClose", "params": {"textDocument": {"uri": member_uri}}})

    function_type = legend.get("tokenTypes", []).index("function")
    definition_bit = 1 << legend.get("tokenModifiers", []).index("definition")
    session.send(
        {
            "jsonrpc": "2.0",
            "id": 10,
            "method": "textDocument/semanticTokens/full",
            "params": {"textDocument": {"uri": uri}},
        }
    )
    response = session.read()
    decoded = decode_semantic_data(response["result"]["data"])
    check(
        f"{label}-semantic-full",
        (0, 4, 3, function_type, definition_bit) in decoded and (5, 17, 3, function_type, 0) in decoded,
        json.dumps(decoded[:8]),
    )

    session.send(
        {
            "jsonrpc": "2.0",
            "id": 11,
            "method": "textDocument/semanticTokens/range",
            "params": {
                "textDocument": {"uri": uri},
                "range": {"start": {"line": 0, "character": 0}, "end": {"line": 1, "character": 100}},
            },
        }
    )
    response = session.read()
    decoded = decode_semantic_data(response["result"]["data"])
    check(
        f"{label}-semantic-range",
        decoded != [] and all(line <= 1 for line, _, _, _, _ in decoded) and (0, 4, 3, function_type, definition_bit) in decoded,
        json.dumps(decoded[:8]),
    )
    check(f"{label}-semantic-data-shape", len(response["result"]["data"]) % 5 == 0)

    # Range edges: the token starting at the range start is included, the
    # tokens ending/starting exactly at the edges are excluded.
    session.send(
        {
            "jsonrpc": "2.0",
            "id": 12,
            "method": "textDocument/semanticTokens/range",
            "params": {
                "textDocument": {"uri": uri},
                "range": {"start": {"line": 0, "character": 4}, "end": {"line": 0, "character": 7}},
            },
        }
    )
    response = session.read()
    check(
        f"{label}-semantic-range-edges",
        decode_semantic_data(response["result"]["data"]) == [(0, 4, 3, function_type, definition_bit)],
        json.dumps(response["result"]["data"])[:200],
    )

    # Unknown documents answer null; empty files answer empty data.
    session.send(
        {
            "jsonrpc": "2.0",
            "id": 13,
            "method": "textDocument/semanticTokens/full",
            "params": {"textDocument": {"uri": "file:///nonexistent.cx"}},
        }
    )
    response = session.read()
    check(f"{label}-semantic-unopened", response["result"] is None, json.dumps(response)[:200])

    empty_uri = uri + ".empty.cx"
    session.send(
        {
            "jsonrpc": "2.0",
            "method": "textDocument/didOpen",
            "params": {"textDocument": {"uri": empty_uri, "languageId": "cx", "version": 1, "text": ""}},
        }
    )
    session.send(
        {
            "jsonrpc": "2.0",
            "id": 14,
            "method": "textDocument/semanticTokens/full",
            "params": {"textDocument": {"uri": empty_uri}},
        }
    )
    notification = session.read()
    response = session.read()
    check(
        f"{label}-semantic-empty",
        notification["method"] == "textDocument/publishDiagnostics" and response["result"] == {"data": []},
        json.dumps(response)[:200],
    )
    session.send({"jsonrpc": "2.0", "method": "textDocument/didClose", "params": {"textDocument": {"uri": empty_uri}}})

    session.send(
        {
            "jsonrpc": "2.0",
            "method": "textDocument/didChange",
            "params": {
                "textDocument": {"uri": uri, "version": 2},
                "contentChanges": [{"text": BAD_SOURCE}],
            },
        }
    )
    notification = session.read()
    diagnostics = notification["params"]["diagnostics"]
    check(
        f"{label}-didchange-diagnostics",
        any("nosuchidentifier" in d["message"] for d in diagnostics),
        json.dumps(diagnostics)[:300],
    )

    # Incremental edits (from clients that send ranges anyway) are applied.
    session.send(
        {
            "jsonrpc": "2.0",
            "method": "textDocument/didChange",
            "params": {
                "textDocument": {"uri": uri, "version": 3},
                "contentChanges": [
                    {
                        "range": {"start": {"line": 1, "character": 4}, "end": {"line": 1, "character": 20}},
                        "text": "1",
                    }
                ],
            },
        }
    )
    notification = session.read()
    check(
        f"{label}-incremental-change",
        notification["params"]["diagnostics"] == [],
        json.dumps(notification["params"]["diagnostics"])[:300],
    )

    session.send({"jsonrpc": "2.0", "id": 3, "method": "shutdown", "params": {}})
    session.read()
    session.send({"jsonrpc": "2.0", "method": "exit", "params": {}})
    code, _ = session.close()
    check(f"{label}-exit-code", code == 0, f"exit code {code}")


def test_server_no_snippets(command, label):
    with tempfile.TemporaryDirectory() as directory:
        path = os.path.join(directory, "main.cx")
        with open(path, "w") as file:
            file.write(GOOD_SOURCE)
        session = LspSession(command)
        session.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {"capabilities": {}}})
        session.read()
        session.send({"jsonrpc": "2.0", "method": "initialized", "params": {}})
        uri = "file://" + path
        session.send(
            {
                "jsonrpc": "2.0",
                "method": "textDocument/didOpen",
                "params": {"textDocument": {"uri": uri, "languageId": "cx", "version": 1, "text": GOOD_SOURCE}},
            }
        )
        session.read()
        session.send(
            {
                "jsonrpc": "2.0",
                "id": 2,
                "method": "textDocument/completion",
                "params": {"textDocument": {"uri": uri}, "position": {"line": 6, "character": 4}},
            }
        )
        response = session.read()
        items = {item["label"]: item for item in response["result"]}
        check(
            f"{label}-completion-plain-parens",
            items.get("add", {}).get("insertText") == "add(" and "insertTextFormat" not in items.get("add", {}),
            json.dumps(items.get("add"))[:300],
        )
        session.send({"jsonrpc": "2.0", "id": 3, "method": "shutdown", "params": {}})
        session.read()
        session.send({"jsonrpc": "2.0", "method": "exit", "params": {}})
        code, _ = session.close()
        check(f"{label}-exit-code", code == 0, f"exit code {code}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cx-lsp", required=True)
    parser.add_argument("--cx", required=True)
    parser.add_argument("--jobs", type=int, default=os.cpu_count() or 4,
                        help="number of test groups to run in parallel (each group spends "
                             "most of its time waiting on query subprocesses, so matching "
                             "the core count does not oversubscribe; override as needed)")
    args = parser.parse_args()

    with tempfile.TemporaryDirectory() as directory:
        path = os.path.join(directory, "main.cx")
        with open(path, "w") as file:
            file.write(GOOD_SOURCE)
        # Every group drives its own compiler/server subprocesses. The
        # --query groups share only the read-only main.cx above (each query
        # carries its own content; the main file is analyzed from memory and
        # excluded from sibling loading), and each server session is a
        # separate process, so the groups are independent and can run in
        # parallel worker threads. check() serializes result reporting.
        # test_fetched_dependency stays serial: it swaps the process-global
        # HOME, which concurrent compiler subprocesses would inherit.
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.jobs) as executor:
            groups = [
                ("query-modes", lambda: test_query_modes(args.cx_lsp, path)),
                ("generic-symbols", lambda: test_generic_symbols(args.cx_lsp, path)),
                ("readonly-tokens", lambda: test_readonly_tokens(args.cx_lsp, path)),
                ("completion-members", lambda: test_completion_members(args.cx_lsp, path)),
                ("package-dedup", lambda: test_package_dedup(args.cx_lsp)),
                ("build-file-modes", lambda: test_build_file_modes(args.cx_lsp)),
                ("server", lambda: test_server([args.cx_lsp], path, "server")),
                ("cx-lsp-subcommand", lambda: test_server([args.cx, "lsp"], path, "cx-lsp-subcommand")),
                ("server-nosnippet", lambda: test_server_no_snippets([args.cx_lsp], "server-nosnippet")),
                ("cx-lsp-subcommand-nosnippet", lambda: test_server_no_snippets([args.cx, "lsp"], "cx-lsp-subcommand-nosnippet")),
            ]
            # A crashing group must not abort the others or swallow the
            # failure summary: record it and let the rest finish.
            list(executor.map(lambda group: run_group(*group), groups))
        test_fetched_dependency(args.cx_lsp)
        test_pkg_config_headers(args.cx_lsp)

    if FAILURES:
        print(f"\n{len(FAILURES)} failure(s): {', '.join(FAILURES)}")
        return 1
    print("\nAll LSP tests passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())

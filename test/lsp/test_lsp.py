#!/usr/bin/env python3
"""End-to-end tests for cx-lsp.

Exercises the one-shot query subprocess (`cx-lsp --query`) and the stdio LSP
server (`cx-lsp` / `cx lsp`). Every test compiles in a fresh process, matching
the compiler's start-fast/compile-fast/exit design.

Usage:
    test_lsp.py --cx-lsp=<path-to-cx-lsp> --cx=<path-to-cx>
"""

import argparse
import json
import os
import subprocess
import sys
import tempfile

FAILURES = []


def check(name, condition, detail=""):
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
    void print(StringBuffer* stream);
}

void main() {
    Printable();
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


def test_package_dedup(cx_lsp):
    # A package directory that the "std" import resolves to (like std/
    # itself) must be analyzed once, not once as the open package and once
    # as the import. Uses an isolated directory so its files can't leak into
    # other tests via sibling loading.
    with tempfile.TemporaryDirectory() as directory:
        package = os.path.join(directory, "std")
        os.mkdir(package)
        with open(os.path.join(package, "defs.cx"), "w") as file:
            file.write("int answer() {\n    return 42;\n}\n")
        use_path = os.path.join(package, "use.cx")

        def package_query(method, content, position=None):
            query = base_query(method, use_path, content, position)
            query["importSearchPaths"] = [directory]
            return query

        content = "int doubled() {\n    return answer() * 2;\n}\n"
        with open(use_path, "w") as file:
            file.write(content)

        result = run_query(cx_lsp, package_query("check", content))
        check("query-package-clean", result["diagnostics"] == [], json.dumps(result["diagnostics"])[:500])

        result = run_query(cx_lsp, package_query("hover", content, (1, 12)))
        check("query-package-hover", "int answer()" in result.get("hover", ""), result.get("hover", "")[:200])

        result = run_query(cx_lsp, package_query("references", content, (1, 12)))
        locations = sorted(
            (r["range"]["start"]["line"], r["range"]["start"]["character"]) for r in result.get("references", [])
        )
        # Definition in defs.cx plus the single use; the definition must not
        # be listed twice via the package's dual role as main and import.
        defs_path = os.path.join(package, "defs.cx")
        files = sorted({r["file"] for r in result.get("references", [])})
        check(
            "query-package-references",
            locations == [(0, 4), (1, 11)] and files == [defs_path, use_path],
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


def test_manifest_modes(cx_lsp):
    # Files under a package.cx manifest are analyzed as its target root
    # (even from a nested subdirectory); files without a manifest are
    # standalone, like `cx file.cx`.
    with tempfile.TemporaryDirectory() as directory:
        package = os.path.join(directory, "proj")
        subdir = os.path.join(package, "sub")
        os.makedirs(subdir)
        with open(os.path.join(package, "package.cx"), "w") as file:
            file.write('var name = "testproj"\n')
        with open(os.path.join(package, "a.cx"), "w") as file:
            file.write("int answer() {\n    return 42;\n}\n")
        nested_path = os.path.join(subdir, "b.cx")
        nested_content = "int doubled() {\n    return answer() * 2;\n}\n"
        with open(nested_path, "w") as file:
            file.write(nested_content)

        result = run_query(cx_lsp, base_query("check", nested_path, nested_content))
        check(
            "query-manifest-package",
            result["diagnostics"] == [],
            json.dumps(result["diagnostics"])[:500],
        )

        result = run_query(cx_lsp, base_query("hover", nested_path, nested_content, (1, 12)))
        check("query-manifest-hover", "int answer()" in result.get("hover", ""), result.get("hover", "")[:200])

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


def test_server(command, path, label):
    session = LspSession(command)
    # Malformed input must be skipped without killing the session.
    session.send_raw(b"this is not a framed message\r\n\r\n")
    session.send({"jsonrpc": "2.0", "id": 1, "method": "initialize", "params": {"capabilities": {}}})
    response = session.read()
    capabilities = response["result"]["capabilities"]
    check(f"{label}-initialize", capabilities.get("hoverProvider") is True, json.dumps(capabilities)[:300])

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


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cx-lsp", required=True)
    parser.add_argument("--cx", required=True)
    args = parser.parse_args()

    with tempfile.TemporaryDirectory() as directory:
        path = os.path.join(directory, "main.cx")
        with open(path, "w") as file:
            file.write(GOOD_SOURCE)
        test_query_modes(args.cx_lsp, path)
        test_package_dedup(args.cx_lsp)
        test_manifest_modes(args.cx_lsp)
        test_server([args.cx_lsp], path, "server")
        test_server([args.cx, "lsp"], path, "cx-lsp-subcommand")

    if FAILURES:
        print(f"\n{len(FAILURES)} failure(s): {', '.join(FAILURES)}")
        return 1
    print("\nAll LSP tests passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())

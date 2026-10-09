#!/usr/bin/env python3
"""Text IO in repo scripts must not depend on the system locale.

open(), Path.read_text()/write_text(), and subprocess text mode decode with
the locale encoding by default, which crashes on UTF-8 content under
non-UTF-8 Windows locales (e.g. zh-CN GBK: "gbk codec can't decode").
Every text-mode call must therefore pass encoding="utf-8" explicitly.
Binary modes are exempt, as is bytes.decode(), whose default is UTF-8.
webbrowser.open() opens a URL, not a file, and is exempt too.

Python embedded in shell heredocs (website/build-website.sh) follows the
same rule by convention but is outside what this test parses.

Run with: python3 test/test_py_utf8.py
"""

import io
import re
import tokenize
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SKIP_DIRS = {"out", ".vs", "__pycache__", ".git"}

# F-string literal parts (PEP 701, Python 3.12+); absent on older versions.
_FSTRING_TOKENS = {
    t
    for t in (
        getattr(tokenize, "FSTRING_START", None),
        getattr(tokenize, "FSTRING_MIDDLE", None),
        getattr(tokenize, "FSTRING_END", None),
    )
    if t is not None
}
_BLANKED_TOKENS = {tokenize.STRING, tokenize.COMMENT} | _FSTRING_TOKENS


def is_python_script(path):
    if path.suffix == ".py":
        return True
    if path.suffix or not path.is_file():
        return False
    try:
        with open(path, "rb") as file:
            first_line = file.readline()
    except OSError:
        return False
    return first_line.startswith(b"#!") and b"python" in first_line


def repo_python_files():
    return sorted(
        path
        for path in ROOT.rglob("*")
        if not any(part in SKIP_DIRS or part.startswith("cmake-build-") for part in path.relative_to(ROOT).parts)
        and is_python_script(path)
    )


def blank_strings_and_comments(source):
    """Replaces string/comment contents with spaces, preserving offsets."""
    chars = list(source)
    offsets = [0]
    for line in source.splitlines(keepends=True):
        offsets.append(offsets[-1] + len(line))
    for token in tokenize.generate_tokens(io.StringIO(source).readline):
        if token.type in _BLANKED_TOKENS:
            (start_row, start_col), (end_row, end_col) = token.start, token.end
            start = offsets[start_row - 1] + start_col
            end = offsets[end_row - 1] + end_col
            for i in range(start, end):
                if chars[i] not in ("\n", "\r"):
                    chars[i] = " "
    return "".join(chars)


def full_call(source, open_paren_index):
    """Extracts the parenthesized call starting at the given '(' offset."""
    depth = 0
    for i in range(open_paren_index, len(source)):
        if source[i] == "(":
            depth += 1
        elif source[i] == ")":
            depth -= 1
            if depth == 0:
                return source[open_paren_index : i + 1]
    return source[open_paren_index:]


def top_level_args(call):
    """Splits a parenthesized call into top-level comma-separated arguments."""
    args, depth, current = [], 0, ""
    for char in call[1:-1]:
        if char in "([":
            depth += 1
        elif char in ")]":
            depth -= 1
        if char == "," and depth == 0:
            args.append(current)
            current = ""
        else:
            current += char
    args.append(current)
    return args


def open_mode_kind(call):
    """Classifies an open() call by its mode argument: absent, binary, text, or unknown.

    Only the mode argument is inspected, never the filename. A non-literal
    mode (variable, expression) is unknown and must not be flagged: adding
    encoding= to a binary open would raise ValueError, so the lint fails
    open when it cannot decide.
    """
    args = top_level_args(call)
    candidates = [arg for arg in args[1:2] if "=" not in arg]
    candidates += [arg.split("=", 1)[1] for arg in args if re.match(r"\s*mode\s*=", arg)]
    if not candidates:
        return "absent"  # Default mode is text.
    for candidate in candidates:
        match = re.match(r"""\s*['"]([^'"]*)['"]\s*$""", candidate)
        if not match:
            return "unknown"
        if set(match.group(1)) <= set("rwxatb+") and "b" in match.group(1):
            return "binary"
    return "text"


def check_source(source, display_name):
    clean = blank_strings_and_comments(source)
    violations = []
    for match in re.finditer(r"(?<![\w.])open\(|\.open\(|\.read_text\(|\.write_text\(", clean):
        if match.group() == ".open(":
            end = match.start()
            start = end
            while start > 0 and (clean[start - 1].isalnum() or clean[start - 1] in "._"):
                start -= 1
            if clean[start:end].split(".")[-1] == "webbrowser":
                continue
        elif match.group() == "open(" and re.search(r"\bdef\s*$", clean[: match.start()].rsplit("\n", 1)[-1]):
            continue  # A method definition named open, not a call.
        if "encoding" in full_call(clean, match.end() - 1):
            continue
        if match.group().find("open(") != -1 and open_mode_kind(full_call(source, match.end() - 1)) in ("binary", "unknown"):
            continue
        line = clean.count("\n", 0, match.start()) + 1
        violations.append(f"{display_name}:{line}: {match.group()[:-1]} without encoding=")
    for match in re.finditer(r"\btext\s*=\s*True\b", clean):
        # The enclosing subprocess call starts at the nearest unmatched '(' before.
        depth = 0
        start = None
        for i in range(match.start() - 1, -1, -1):
            if clean[i] == ")":
                depth += 1
            elif clean[i] == "(":
                if depth == 0:
                    start = i
                    break
                depth -= 1
        call = full_call(clean, start) if start is not None else ""
        if "encoding" not in call:
            line = clean.count("\n", 0, match.start()) + 1
            violations.append(f"{display_name}:{line}: text=True without encoding=")
    return violations


def check_file(path):
    try:
        source = path.read_text(encoding="utf-8")
        return check_source(source, str(path.relative_to(ROOT)))
    except (UnicodeDecodeError, SyntaxError, tokenize.TokenError, IndentationError):
        return []  # Not parseable Python; nothing to check.


class PyUtf8Test(unittest.TestCase):
    def test_text_io_passes_encoding(self):
        violations = []
        for path in repo_python_files():
            violations.extend(check_file(path))
        self.assertEqual(violations, [])

    def test_checker_logic(self):
        # The checker flags locale-dependent calls and exempts the rest.
        cases = [
            ("open(p) as f", True),
            ("open(p, 'r') as f", True),
            ("open(p, 'w', encoding='utf-8') as f", False),
            ("open('b') as f", True),  # A file named 'b' is still text mode.
            ("open(p, 'rb') as f", False),
            ("open(p, mode='wb') as f", False),
            ("open(p, mode) as f", False),  # Unknown mode: cannot flag safely.
            ("open(p, buffering=1) as f", True),  # No mode argument: text default.
            ("open(file=p) as f", True),
            ("io.open(p, 'w') as f", True),
            ("Path(p).open('w') as f", True),
            ("webbrowser.open(url)", False),
            ("def open(self, p):", False),
            ("p.read_text()", True),
            ("p.read_text(encoding='utf-8')", False),
            ("p.write_text(x)", True),
            ("run(cmd, text=True)", True),
            ("run(cmd, text=True, encoding='utf-8')", False),
            ("x = 'open('  # open(", False),  # Strings and comments ignored.
            ("msg = f\"result of open( call: {x}\"", False),  # F-string text ignored.
        ]
        for snippet, expect_violation in cases:
            with self.subTest(snippet=snippet):
                source = f"import io\nfrom pathlib import Path\n{snippet}\n"
                try:
                    found = check_source(source, "probe")
                except (SyntaxError, tokenize.TokenError):
                    self.fail(f"probe does not parse: {snippet}")
                self.assertEqual(len(found) > 0, expect_violation)


if __name__ == "__main__":
    unittest.main()

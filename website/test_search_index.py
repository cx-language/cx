#!/usr/bin/env python3
"""Tests for generate_search_index.py. Run with: python3 website/test_search_index.py"""

import json
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from generate_search_index import build_index, main, page_id, page_title, plain_text

WEBSITE_DIR = Path(__file__).resolve().parent


class PlainTextTest(unittest.TestCase):
    def test_strips_syntax_but_keeps_code(self):
        markdown = "# Title\n\nProse with [link](https://example.com) and `code`.\n\n```cs {.noRun}\nvoid main() {}\n```\n"
        self.assertEqual(plain_text(markdown), "Title Prose with link and code. void main() {}")

    def test_strips_generated_page_markup(self):
        markdown = '# [bool](https://example.com/std/bool.cx){target="_blank"}\n\n## [size](https://example.com#L4) {#bool-size}\n'
        self.assertEqual(plain_text(markdown), "bool size")

    def test_keeps_snake_case_identifiers(self):
        self.assertEqual(plain_text("Call `is_even(x)` here."), "Call is_even(x) here.")

    def test_unwraps_escaped_brackets_in_links(self):
        self.assertEqual(plain_text("[operator\\[\\]](https://example.com)"), "operator[]")

    def test_unescapes_punctuation(self):
        self.assertEqual(plain_text("See operator\\[\\] below."), "See operator[] below.")

    def test_keeps_generics_in_code_spans(self):
        self.assertEqual(plain_text("Use `List<bool>` here."), "Use List<bool> here.")

    def test_keeps_fenced_code_verbatim(self):
        self.assertEqual(plain_text("```cs\nvar x = List<int>();\n```\n"), "var x = List<int>();")

    def test_strips_tags_around_fences(self):
        self.assertEqual(plain_text("a<br>b\n```cs\nx<y>\n```\n"), "ab x<y>")

    def test_tolerates_unbalanced_fence(self):
        # Unbalanced input degrades to prose stripping (no fence markup leaks).
        self.assertEqual(plain_text("text\n```cs\nvar x = 1;\n"), "text var x = 1;")

    def test_strips_tags_outside_code_spans(self):
        self.assertEqual(plain_text("a<br>b `x < y`"), "ab x < y")

    def test_images_keep_alt_text(self):
        self.assertEqual(plain_text("See ![diagram](pic.png) here."), "See diagram here.")

    def test_collapses_whitespace(self):
        self.assertEqual(plain_text("a\n\n  b\tc"), "a b c")


class PageTitleTest(unittest.TestCase):
    def test_first_heading(self):
        self.assertEqual(page_title("# Language overview\n\ntext\n"), "Language overview")

    def test_unwraps_heading_link(self):
        self.assertEqual(page_title('# [List](./list)\n'), "List")

    def test_generated_heading_keeps_name_only(self):
        self.assertEqual(page_title('# [Array](https://example.com/std/Array.cx){target="_blank"}\n'), "Array")

    def test_missing_heading(self):
        self.assertEqual(page_title("no heading\n"), "")


class PageIdTest(unittest.TestCase):
    def test_docs_page(self):
        self.assertEqual(page_id(Path("/docs/list.md"), Path("/docs")), "list")

    def test_std_index(self):
        self.assertEqual(page_id(Path("/gen/std.md"), Path("/gen")), "std")

    def test_std_file_page(self):
        self.assertEqual(page_id(Path("/gen/std/Array.md"), Path("/gen")), "std/Array")

    def test_nested_std_page(self):
        self.assertEqual(page_id(Path("/gen/std/os/posix.md"), Path("/gen")), "std/os/posix")


class BuildIndexTest(unittest.TestCase):
    def test_guides_come_before_reference(self):
        with tempfile.TemporaryDirectory() as directory:
            docs = Path(directory, "docs")
            generated = Path(directory, "generated")
            docs.mkdir()
            (generated / "std").mkdir(parents=True)
            (docs / "b.md").write_text("# Bee\n\ntext\n")
            (docs / "a.md").write_text("# Ay\n\ntext\n")
            (generated / "std.md").write_text("# Ref\n\ntext\n")
            (generated / "std" / "Array.md").write_text("# Array\n\ntext\n")
            entries = build_index(docs, generated)
            self.assertEqual(
                [(entry["id"], entry["title"]) for entry in entries],
                [("a", "Ay"), ("b", "Bee"), ("std", "Ref"), ("std/Array", "Array")],
            )

    def test_main_writes_loader(self):
        with tempfile.TemporaryDirectory() as directory:
            docs = Path(directory, "docs")
            docs.mkdir()
            (docs / "a.md").write_text("# Ay\n\ntext\n")
            output = Path(directory, "out", "search-index.js")
            self.assertEqual(main(["--docs-dir", str(docs), "--generated-dir", str(docs), "--output", str(output)]), 0)
            content = output.read_text()
            self.assertTrue(content.startswith("var CxSearchIndex = "))
            # Same file counted twice (docs + generated point at it); entries parse.
            entries = json.loads(content.removeprefix("var CxSearchIndex = ").removesuffix(";\n"))
            self.assertEqual([entry["id"] for entry in entries], ["a", "a"])


if __name__ == "__main__":
    unittest.main()

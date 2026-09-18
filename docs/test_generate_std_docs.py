#!/usr/bin/env python3
"""Tests for generate_std_docs.py. Run with: python3 docs/test_generate_std_docs.py"""

import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from generate_std_docs import parse_all, render

DOCS_DIR = Path(__file__).resolve().parent
STD_DIR = DOCS_DIR.parent / "std"

FIXTURE = """\
/// A widget.
struct Widget: Copyable {
    int size;

//    Widget(int size) {
//        this.size = size;
//    }

    /// Makes a widget.
    Widget(int size) {
        this.size = size;
    }

    /// Returns the size.
    int size() {
        return size;
    }

    private void helper() {
    }
}

/// Section marker, not a doc.

// (blank line above drops the marker)

/// Compares widgets.
bool operator==<T>(Widget* a, Widget* b) {
    return a.size() == b.size();
}

/// An action.
interface Action {
    /// Runs it.
    void run();
}

enum Color {
    Red,
    Green
}

extern int puts(const char* str);

const int answer = 42;
"""


class FixtureTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with tempfile.TemporaryDirectory() as directory:
            Path(directory, "fixture.cx").write_text(FIXTURE)
            types, functions, constants = parse_all(Path(directory))
        cls.markdown = render(types, functions, constants)

    def test_type_header_and_doc(self):
        self.assertIn("## `struct Widget: Copyable` {#type-Widget}", self.markdown)
        self.assertIn("A widget.", self.markdown)

    def test_members(self):
        self.assertIn("### `size` {#Widget-size}", self.markdown)
        self.assertIn("`int size()`", self.markdown)
        self.assertIn("Returns the size.", self.markdown)
        self.assertIn("`int size;`", self.markdown)
        self.assertIn("`Widget(int size)`", self.markdown)

    def test_private_member_omitted(self):
        self.assertNotIn("helper", self.markdown)

    def test_commented_out_code_ignored(self):
        self.assertEqual(self.markdown.count("`Widget(int size)`"), 1)

    def test_section_marker_dropped(self):
        self.assertNotIn("Section marker", self.markdown)

    def test_operator_slug(self):
        self.assertIn("## `operator==` {#fn-operator-eq}", self.markdown)

    def test_interface_keeps_semicolon(self):
        self.assertIn("`void run();`", self.markdown)

    def test_enum_variants(self):
        self.assertIn("### `Red` {#Color-Red}", self.markdown)
        self.assertIn("### `Green` {#Color-Green}", self.markdown)

    def test_extern_and_const(self):
        self.assertIn("`extern int puts(const char* str);`", self.markdown)
        self.assertIn("`const int answer = 42;`", self.markdown)


class StdlibTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        types, functions, constants = parse_all(STD_DIR)
        cls.markdown = render(types, functions, constants)
        cls.functions = functions

    def test_known_entries(self):
        for snippet in [
            "## `struct List<Element>` {#type-List}",
            "### `push` {#List-push}",
            "`void push(Element element)`",
            "Adds the given element to the end of the list.",
            "## `println` {#fn-println}",
            "## `int_max` {#const-int_max}",
            "`const int int_max = 2147483647;`",
        ]:
            self.assertIn(snippet, self.markdown)

    def test_overloads_grouped(self):
        self.assertGreater(len(self.functions["println"].declarations), 10)
        self.assertEqual(self.markdown.count("## `println` {#fn-println}"), 1)

    def test_private_declarations_omitted(self):
        for name in ["unsafeRemoveAt", "quickSort", "insertionSort", "partition",
                     "printSigned", "printUnsigned", "skipEmptySlots", "rebalance",
                     "rotateLeft", "rotateRight", "grow", "indexOutOfBounds",
                     "setBalance", "height", "minInSubtree", "maxInSubtree"]:
            self.assertNotIn(f"`{name}`", self.markdown, name)
            self.assertNotIn(f" {name}(", self.markdown, name)

    def test_no_fenced_code_blocks(self):
        self.assertNotIn("```", self.markdown)

    def test_index_links_resolve(self):
        links = set(re.findall(r"\(#(.*?)\)", self.markdown))
        ids = set(re.findall(r"\{#(.*?)\}", self.markdown))
        self.assertEqual(links - ids, set())

    def test_no_duplicate_ids(self):
        ids = re.findall(r"\{#(.*?)\}", self.markdown)
        self.assertEqual(len(ids), len(set(ids)))


if __name__ == "__main__":
    unittest.main()

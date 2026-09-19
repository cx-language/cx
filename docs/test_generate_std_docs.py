#!/usr/bin/env python3
"""Tests for generate_std_docs.py. Run with: python3 docs/test_generate_std_docs.py"""

import re
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from generate_std_docs import (
    main,
    member_name,
    page_name,
    parse_file,
    parse_std,
    render_file_page,
    render_index,
    render_toc_items,
)

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


def parse_fixture(source=FIXTURE):
    directory = tempfile.TemporaryDirectory()
    path = Path(directory.name, "fixture.cx")
    path.write_text(source)
    return directory, parse_file(path)


def fenced(signature):
    return f"```cs {{.noRun}}\n{signature}\n```"


class MemberNameTest(unittest.TestCase):
    def test_generic_method_with_generic_return(self):
        self.assertEqual(member_name("List<Output> map<Output>(Output(Element*) transform)"), "map")

    def test_plain_method(self):
        self.assertEqual(member_name("void push(Element element)"), "push")

    def test_operator(self):
        self.assertEqual(member_name("bool operator== <T: Comparable>(T* a, T* b)"), "operator==")


class PageNameTest(unittest.TestCase):
    def test_top_level(self):
        self.assertEqual(page_name("List.cx"), "std/List")

    def test_nested(self):
        self.assertEqual(page_name("os/gnu.cx"), "std/os/gnu")


class FixtureTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory, (types, functions, constants) = parse_fixture()
        cls.markdown = render_file_page("fixture.cx", types, functions, constants, False)

    @classmethod
    def tearDownClass(cls):
        cls.directory.cleanup()

    def test_page_title_and_source(self):
        self.assertIn(
            "# [fixture](https://github.com/emillaine/cx/blob/main/std/fixture.cx)"
            '{target="_blank"}\n',
            self.markdown,
        )
        self.assertNotIn("Auto-generated from", self.markdown)

    def test_type_header_and_doc(self):
        self.assertIn(
            "## [struct Widget: Copyable](https://github.com/emillaine/cx/blob/main/std/fixture.cx#L2)"
            '{target="_blank"} {#type-Widget}',
            self.markdown,
        )
        self.assertIn("A widget.", self.markdown)

    def test_members(self):
        self.assertIn(
            "### [size](https://github.com/emillaine/cx/blob/main/std/fixture.cx#L3)"
            '{target="_blank"} {#Widget-size}',
            self.markdown,
        )
        self.assertIn(fenced("int size()"), self.markdown)
        self.assertIn("Returns the size.", self.markdown)
        self.assertIn(fenced("int size;"), self.markdown)
        self.assertIn(fenced("Widget(int size)"), self.markdown)

    def test_private_member_omitted(self):
        self.assertNotIn("helper", self.markdown)

    def test_commented_out_code_ignored(self):
        self.assertEqual(self.markdown.count(fenced("Widget(int size)")), 1)

    def test_section_marker_dropped(self):
        self.assertNotIn("Section marker", self.markdown)

    def test_operator_slug(self):
        self.assertIn(
            "## [operator==](https://github.com/emillaine/cx/blob/main/std/fixture.cx#L28)"
            '{target="_blank"} {#fn-operator-eq}',
            self.markdown,
        )

    def test_interface_keeps_semicolon(self):
        self.assertIn(fenced("void run();"), self.markdown)

    def test_enum_variants(self):
        self.assertIn(
            "### [Red](https://github.com/emillaine/cx/blob/main/std/fixture.cx#L39)"
            '{target="_blank"} {#Color-Red}',
            self.markdown,
        )
        self.assertIn(
            "### [Green](https://github.com/emillaine/cx/blob/main/std/fixture.cx#L40)"
            '{target="_blank"} {#Color-Green}',
            self.markdown,
        )

    def test_extern_and_const(self):
        self.assertIn(
            "## [answer](https://github.com/emillaine/cx/blob/main/std/fixture.cx#L45)"
            '{target="_blank"} {#const-answer}',
            self.markdown,
        )
        self.assertIn(fenced("extern int puts(const char* str);"), self.markdown)
        self.assertIn(fenced("const int answer = 42;"), self.markdown)

    def test_no_conditional_note(self):
        self.assertNotIn("platform-conditional", self.markdown)

    def test_file_order(self):
        widget = self.markdown.index("{#type-Widget}")
        action = self.markdown.index("{#type-Action}")
        color = self.markdown.index("{#type-Color}")
        self.assertLess(widget, action)
        self.assertLess(action, color)
        size = self.markdown.index("{#Widget-size}")
        ctor = self.markdown.index("{#Widget-Widget}")
        self.assertLess(size, ctor)


class ConditionalTest(unittest.TestCase):
    def test_if_marked_conditional(self):
        with tempfile.TemporaryDirectory() as std_dir:
            Path(std_dir, "cond.cx").write_text("#if Windows\nvoid f() {}\n#endif\n")
            Path(std_dir, "plain.cx").write_text("void g() {}\n")
            pages = {relpath: conditional for relpath, *_ , conditional in parse_std(Path(std_dir))}
        self.assertTrue(pages["cond.cx"])
        self.assertFalse(pages["plain.cx"])

    def test_conditional_note_rendered(self):
        self.assertIn("platform-conditional", render_file_page("c.cx", [], {}, [], True))
        self.assertNotIn("platform-conditional", render_file_page("c.cx", [], {}, [], False))


class IndexTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.keepalive, parts = parse_fixture()
        types, functions, constants = parts
        cls.markdown = render_index([("fixture.cx", types, functions, constants, False)])

    @classmethod
    def tearDownClass(cls):
        cls.keepalive.cleanup()

    def test_bullet_links_to_page(self):
        self.assertIn("- [fixture.cx](./std/fixture): ", self.markdown)

    def test_bullet_lists_declarations(self):
        for name in ["`Widget`", "`Action`", "`Color`", "`operator==`", "`puts`", "`answer`"]:
            self.assertIn(name, self.markdown)

    def test_bullet_lists_declarations_in_file_order(self):
        self.assertLess(self.markdown.index("`Widget`"), self.markdown.index("`Action`"))


class FileOrderTest(unittest.TestCase):
    def test_functions_and_constants_in_file_order(self):
        with tempfile.TemporaryDirectory() as std_dir:
            Path(std_dir, "order.cx").write_text(
                "void zebra() {}\nvoid apple() {}\nconst int zed = 1;\nconst int aardvark = 2;\n"
            )
            relpath, types, functions, constants, _ = parse_std(Path(std_dir))[0]
            markdown = render_file_page(relpath, types, functions, constants, False)
        self.assertLess(markdown.index("{#fn-zebra}"), markdown.index("{#fn-apple}"))
        self.assertLess(markdown.index("{#const-zed}"), markdown.index("{#const-aardvark}"))


class TocTest(unittest.TestCase):
    def test_items_link_to_pages(self):
        items = render_toc_items([("List.cx", [], {}, [], False), ("os/gnu.cx", [], {}, [], False)])
        self.assertEqual(
            items,
            [
                '                <li><a href="./std/List">List</a></li>',
                '                <li><a href="./std/os/gnu">os/gnu</a></li>',
            ],
        )


class StdlibTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.pages = parse_std(STD_DIR)
        cls.by_path = {page[0]: page[1:] for page in cls.pages}
        cls.rendered = {
            relpath: render_file_page(relpath, *rest) for relpath, rest in cls.by_path.items()
        }

    def test_one_page_per_file(self):
        self.assertEqual(
            sorted(self.by_path), sorted(p.relative_to(STD_DIR).as_posix() for p in STD_DIR.rglob("*.cx"))
        )

    def test_os_files_included(self):
        self.assertIn("os/gnu.cx", self.by_path)
        self.assertIn("os/windows.cx", self.by_path)

    def test_known_entries(self):
        page = self.rendered["List.cx"]
        base = "https://github.com/emillaine/cx/blob/main/std/List.cx"
        for snippet in [
            f'# [List]({base}){{target="_blank"}}',
            f'## [struct List\\<Element\\>]({base}#L2){{target="_blank"}} {{#type-List}}',
            f'### [push]({base}#L108){{target="_blank"}} {{#List-push}}',
            f'### [operator\\[\\]]({base}#L77){{target="_blank"}} {{#List-operator-index}}',
            fenced("void push(Element element)"),
            "Adds the given element to the end of the list.",
        ]:
            self.assertIn(snippet, page)

    def test_overloads_grouped(self):
        stdio = self.rendered["stdio.cx"]
        self.assertEqual(
            stdio.count(
                "## [println](https://github.com/emillaine/cx/blob/main/std/stdio.cx#L2)"
                '{target="_blank"} {#fn-println}'
            ),
            1,
        )
        self.assertGreater(len(self.by_path["stdio.cx"][1]["println"].declarations), 10)

    def test_private_declarations_omitted(self):
        combined = "\n".join(self.rendered.values())
        for name in ["unsafeRemoveAt", "quickSort", "insertionSort", "partition",
                     "printSigned", "printUnsigned", "skipEmptySlots", "rebalance",
                     "rotateLeft", "rotateRight", "grow", "indexOutOfBounds",
                     "setBalance", "height", "minInSubtree", "maxInSubtree"]:
            self.assertNotIn(f"`{name}`", combined, name)
            self.assertNotIn(f" {name}(", combined, name)

    def test_fences_are_all_highlighted_and_not_runnable(self):
        for relpath, markdown in self.rendered.items():
            for line in markdown.splitlines():
                if line.startswith("```") and line != "```":
                    self.assertEqual(line, "```cs {.noRun}", relpath)

    def test_no_duplicate_ids_per_page(self):
        for relpath, markdown in self.rendered.items():
            ids = re.findall(r"\{#(.*?)\}", markdown)
            self.assertEqual(len(ids), len(set(ids)), relpath)


class StagingTest(unittest.TestCase):
    def test_main_writes_index_pages_and_toc(self):
        with tempfile.TemporaryDirectory() as std_dir, tempfile.TemporaryDirectory() as output_dir:
            Path(std_dir, "fixture.cx").write_text(FIXTURE)
            self.assertEqual(main(["--std-dir", std_dir, "--output-dir", output_dir]), 0)
            out = Path(output_dir)
            index = (out / "std.md").read_text()
            page = (out / "std/fixture.md").read_text()
            toc = (out / "toc.html").read_text()
        self.assertIn("# Standard library reference", index)
        self.assertIn("- [fixture.cx](./std/fixture): ", index)
        self.assertIn("## [struct Widget: Copyable]", page)
        self.assertIn('<li><a href="./std/fixture">fixture</a></li>', toc)
        self.assertNotIn("<!--STD-PAGES-->", toc)


if __name__ == "__main__":
    unittest.main()

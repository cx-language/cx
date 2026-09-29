#!/usr/bin/env python3
"""Tests for serve.py. Run with: python3 website/test_serve.py"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from serve import SNIPPET, collect_files, inject_livereload, snapshot


class InjectTest(unittest.TestCase):
    def test_inserts_before_body_close(self):
        html = "<html><body><p>hi</p></body></html>"
        out = inject_livereload(html)
        self.assertIn(SNIPPET + "</body>", out)
        self.assertEqual(out.count(SNIPPET), 1)

    def test_appends_without_body_close(self):
        html = "<html><p>hi</p></html>"
        self.assertTrue(inject_livereload(html).endswith(SNIPPET))


class SnapshotTest(unittest.TestCase):
    def test_detects_content_change(self):
        import os
        import tempfile

        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "index.html"
            target.write_text("a")
            before = snapshot([target])
            target.write_text("bb")
            later = before[str(target)] + 1_000_000
            os.utime(target, ns=(later, later))
            self.assertNotEqual(snapshot([target]), before)

    def test_stable_when_untouched(self):
        import tempfile

        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "index.html"
            target.write_text("a")
            self.assertEqual(snapshot([target]), snapshot([target]))


class CollectFilesTest(unittest.TestCase):
    def test_skips_build_outputs(self):
        import tempfile

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "docs").mkdir()
            (root / "examples").mkdir()
            (root / "std").mkdir()
            site = root / "website"
            (site / "build").mkdir(parents=True)
            (site / ".generated").mkdir()
            (site / "__pycache__").mkdir()
            page = site / "index.html"
            page.write_text("x")
            (site / "build" / "index.html").write_text("x")
            (site / ".generated" / "toc.html").write_text("x")
            (site / "__pycache__" / "serve.pyc").write_text("x")
            found = collect_files(site)
            self.assertIn(page, found)
            self.assertEqual(len(found), 1)


if __name__ == "__main__":
    unittest.main()

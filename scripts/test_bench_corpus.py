#!/usr/bin/env python3
"""Tests for bench-corpus.py SLOC metrics. Run with: python3 scripts/test_bench_corpus.py"""

import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

# bench-corpus.py has a dash; import it by path.
import importlib.util


def load_bench_corpus():
    spec = importlib.util.spec_from_file_location("bench_corpus", Path(__file__).resolve().parent / "bench-corpus.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


bench_corpus = load_bench_corpus()
ROOT = bench_corpus.ROOT


class SlocTest(unittest.TestCase):
    def make_repo(self, files):
        if shutil.which("git") is None:
            self.skipTest("git not installed")
        tmp = tempfile.TemporaryDirectory()
        root = Path(tmp.name)
        for path, content in files.items():
            target = root / path
            target.parent.mkdir(parents=True, exist_ok=True)
            if isinstance(content, str):
                content = content.encode()
            target.write_bytes(content)
        subprocess.run(["git", "init", "-q"], cwd=root, check=True)
        subprocess.run(["git", "add", "."], cwd=root, check=True)
        return tmp, root

    def test_counts_nonblank_lines_per_group(self):
        tmp, root = self.make_repo(
            {
                "src/a.cpp": "int x;\n\nint y;\n",
                "std/b.cx": "void f() {}\n",
                "vendor/c/c.cx": "// c\n\n",
                "docs/d.md": "# t\n\ntext\n",
                "examples/e.cx": "void main() {}\n",
                "test/f.cx": "// t\n",
                "website/test_g.py": "x = 1\n\n",
                "website/test-h.mjs": "v\n",
            }
        )
        with tmp:
            counts = bench_corpus.sloc_metrics(str(root))
        self.assertEqual(
            counts,
            {"compiler": 2, "stdlib": 1, "vendor": 1, "docs": 2, "examples": 1, "tests": 3, "total": 10},
        )

    def test_skips_binaries_excludes_and_untracked(self):
        tmp, root = self.make_repo(
            {
                "examples/shot.png": b"\x89PNG\x00\x01\x02\nline\n",
                "examples/inputs/data.txt": "data\n",
                "website/serve.py": "code\n",
                "scripts/tool.py": "code\n",
            }
        )
        with tmp:
            (root / "src" / "untracked.cpp").parent.mkdir(exist_ok=True)
            (root / "src" / "untracked.cpp").write_text("untracked\n")
            counts = bench_corpus.sloc_metrics(str(root))
        self.assertEqual(counts["examples"], 0)
        self.assertEqual(counts["tests"], 0)
        self.assertEqual(counts["compiler"], 0)
        self.assertEqual(counts["total"], 0)

    def test_real_repo(self):
        counts = bench_corpus.sloc_metrics(ROOT)
        for group in ["compiler", "stdlib", "vendor", "docs", "examples", "tests"]:
            self.assertGreater(counts[group], 0, group)
        self.assertEqual(counts["total"], sum(counts[group] for group in ["compiler", "stdlib", "vendor", "docs", "examples", "tests"]))

    def test_no_git_returns_none(self):
        with tempfile.TemporaryDirectory() as tmp:
            self.assertIsNone(bench_corpus.sloc_metrics(tmp))


if __name__ == "__main__":
    unittest.main()

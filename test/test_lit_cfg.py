#!/usr/bin/env python3
"""Tests for test/lit.cfg system-library detection. Run with: python3 test/test_lit_cfg.py"""

import os
import sys
import tempfile
import unittest
from pathlib import Path
from types import ModuleType, SimpleNamespace
from unittest.mock import patch

LIT_CFG = Path(__file__).resolve().parent / "lit.cfg"


def _fatal(message):
    raise Exception(message)


def load_lit_cfg(path_env):
    """Executes lit.cfg with stubbed lit globals and PATH=path_env.

    Returns the stub config object, which records available_features.
    """
    config = SimpleNamespace(substitutions=[], available_features=set())
    lit_config = SimpleNamespace(params={}, fatal=_fatal)
    fake_lit = ModuleType("lit")
    fake_lit.formats = ModuleType("lit.formats")
    fake_lit.formats.ShTest = object
    saved = (sys.modules.get("lit"), sys.modules.get("lit.formats"))
    sys.modules["lit"] = fake_lit
    sys.modules["lit.formats"] = fake_lit.formats
    try:
        with patch.dict(os.environ, {"PATH": path_env}):
            exec(compile(LIT_CFG.read_text(encoding="utf-8"), str(LIT_CFG), "exec"), {
                "__file__": str(LIT_CFG),
                "config": config,
                "lit_config": lit_config,
            })
    finally:
        for name, module in zip(("lit", "lit.formats"), saved):
            if module is None:
                sys.modules.pop(name, None)
            else:
                sys.modules[name] = module
    return config


class LitCfgTest(unittest.TestCase):
    def test_unlaunchable_pkg_config_disables_features(self):
        # which() resolves by name, but the result may still fail to launch:
        # a bogus interpreter on POSIX, a .bat on Windows (CreateProcess
        # cannot run those, which crashed config load in CI). lit.cfg must
        # treat the libraries as missing instead of crashing.
        with tempfile.TemporaryDirectory() as tmp:
            if os.name == "nt":
                Path(tmp, "pkg-config.bat").write_text("@exit 1\n", encoding="utf-8")
            else:
                stub = Path(tmp, "pkg-config")
                stub.write_text("#!/nonexistent-interpreter\n", encoding="utf-8")
                stub.chmod(0o755)
            config = load_lit_cfg(tmp)
        self.assertNotIn("sdl3", config.available_features)
        self.assertNotIn("raylib", config.available_features)

    @unittest.skipIf(os.name == "nt", "no launchable pkg-config stub without an exe")
    def test_working_pkg_config_enables_present_libraries(self):
        with tempfile.TemporaryDirectory() as tmp:
            stub = Path(tmp, "pkg-config")
            stub.write_text('#!/bin/sh\n[ "$2" = sdl3 ]\n', encoding="utf-8")
            stub.chmod(0o755)
            config = load_lit_cfg(tmp)
        self.assertIn("sdl3", config.available_features)
        self.assertNotIn("raylib", config.available_features)


if __name__ == "__main__":
    unittest.main()

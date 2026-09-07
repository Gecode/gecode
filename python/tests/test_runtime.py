"""Packaged-library selection is independent of the process working directory."""
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

from gecode_optimize import _runtime


class RuntimeTest(unittest.TestCase):
    def test_absent_bundle_preserves_external_library_workflow(self):
        with tempfile.TemporaryDirectory() as temporary:
            package = Path(temporary) / "package"
            package.mkdir()
            (Path(temporary) / "libgecodeoptimize_c.dylib").touch()
            with patch.object(_runtime, "__file__", str(package / "_runtime.py")):
                self.assertIsNone(_runtime.bundled_library_path())

    def test_exact_platform_entry_and_incomplete_bundle(self):
        for platform, name in (("darwin", "libgecodeoptimize_c.dylib"),
                               ("linux", "libgecodeoptimize_c.so"),
                               ("win32", "gecodeoptimize_c.dll")):
            with self.subTest(platform=platform), tempfile.TemporaryDirectory() as temporary:
                package = Path(temporary)
                native = package / "_native"
                native.mkdir()
                with patch.object(_runtime, "__file__", str(package / "_runtime.py")), \
                        patch.object(_runtime.sys, "platform", platform):
                    with self.assertRaisesRegex(RuntimeError, "incomplete"):
                        _runtime.bundled_library_path()
                    entry = native / name
                    entry.touch()
                    self.assertEqual(_runtime.bundled_library_path(), str(entry.resolve()))

    def test_bundle_cannot_redirect_to_an_external_file(self):
        with tempfile.TemporaryDirectory() as temporary:
            package = Path(temporary)
            native = package / "_native"
            native.mkdir()
            outside = package / "external.dylib"
            outside.touch()
            try:
                (native / "libgecodeoptimize_c.dylib").symlink_to(outside)
            except OSError:
                self.skipTest("creating symlinks is unavailable on this host")
            with patch.object(_runtime, "__file__", str(package / "_runtime.py")), \
                    patch.object(_runtime.sys, "platform", "darwin"):
                with self.assertRaisesRegex(RuntimeError, "outside"):
                    _runtime.bundled_library_path()


if __name__ == "__main__":
    unittest.main()

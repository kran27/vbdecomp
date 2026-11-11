"""Source checkout and Small preparation must work without patch artifacts."""
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("prepare_small_runtime", ROOT / "scripts/prepare_small_runtime.py")
small = importlib.util.module_from_spec(spec)
spec.loader.exec_module(small)


class SourceSetupTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.supplied = self.root / "supplied"
        (self.supplied / "src").mkdir(parents=True)
        (self.root / "config").mkdir()
        (self.root / "adapted").mkdir()
        sources = {}
        for name in ["amx.c", "amx.h", "float.c", "osdefs.h"]:
            data = ("original " + name).encode()
            (self.supplied / "src" / name).write_bytes(data)
            sources[name] = hashlib.sha256(data).hexdigest()
        adapted = {}
        for name in ["amx.c", "amx.h"]:
            data = ("exact adapted " + name).encode()
            (self.root / "adapted" / name).write_bytes(data)
            adapted[name] = dict(path="adapted/" + name, sha256=hashlib.sha256(data).hexdigest())
        (self.root / "config/small-runtime.lock.json").write_text(json.dumps(dict(
            sources=sources, adapted_sources=adapted)))

    def prepare(self):
        with patch.object(small, "ROOT", self.root):
            return small.prepare(self.supplied, self.root / "output")

    def test_preparation_needs_no_patch_and_preserves_supplied_source(self):
        before = {p.name: p.read_bytes() for p in (self.supplied / "src").iterdir()}
        result = self.prepare()
        self.assertFalse((self.root / "patches").exists())
        for name in ["amx.c", "amx.h"]:
            self.assertEqual((self.root / "output" / name).read_bytes(), (self.root / "adapted" / name).read_bytes())
        for name in ["float.c", "osdefs.h"]:
            self.assertEqual((self.root / "output" / name).read_bytes(), before[name])
        self.assertEqual(before, {p.name: p.read_bytes() for p in (self.supplied / "src").iterdir()})
        self.assertFalse(result["user_checkout_modified"])
        self.assertNotIn("patch_sha256", result)
        output = self.root / "output/amx.c"
        timestamp = output.stat().st_mtime_ns
        self.prepare()
        self.assertEqual(output.stat().st_mtime_ns, timestamp)

    def test_changed_supplied_source_is_rejected(self):
        (self.supplied / "src/amx.c").write_bytes(b"changed")
        with self.assertRaisesRegex(ValueError, "Supplied Small source changed"):
            self.prepare()

    def test_changed_adaptation_is_rejected(self):
        (self.root / "adapted/amx.h").write_bytes(b"changed")
        with self.assertRaisesRegex(ValueError, "adaptation identity differs"):
            self.prepare()

    def test_cannot_write_into_user_checkout(self):
        with patch.object(small, "ROOT", self.root):
            with self.assertRaisesRegex(ValueError, "cannot be inside"):
                small.prepare(self.supplied, self.supplied / "generated")

    def test_source_only_setup_in_clean_tree_needs_no_git_or_patch_directory(self):
        scripts = self.root / "scripts"
        scripts.mkdir()
        script = scripts / "Sync-Repos.ps1"
        script.write_bytes((ROOT / "scripts/Sync-Repos.ps1").read_bytes())
        source = self.root / "repos/openvb/src"
        source.mkdir(parents=True)
        (source / "F3.cpp").write_text("source")
        (self.root / "repos.lock.json").write_text(json.dumps(dict(
            workspaceSources=[dict(path="repos/openvb", requiredFile="src/F3.cpp")], repositories=[])))
        command = ["powershell", "-NoProfile", "-NonInteractive", "-File", str(script), "-SourceOnly"]
        good = subprocess.run(command, capture_output=True, text=True)
        self.assertEqual(good.returncode, 0, good.stderr)
        self.assertIn("no patches", good.stdout)
        (source / "F3.cpp").unlink()
        missing = subprocess.run(command, capture_output=True, text=True)
        self.assertNotEqual(missing.returncode, 0)
        self.assertIn("Checked-in source is missing", missing.stderr)


if __name__ == "__main__":
    unittest.main()

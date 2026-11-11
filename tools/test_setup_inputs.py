"""Check archive boundaries and preservation before installing build inputs."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
import zipfile

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("dependency_bundle", ROOT / "scripts/dependency_bundle.py")
bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(bundle)


class DependencyBundleTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        (self.root / "config").mkdir()

    def archive(self, files):
        manifest = {"schemaVersion": 1, "files": {
            name: {"size": len(data), "sha256": hashlib.sha256(data).hexdigest()}
            for name, data in files.items()}}
        payload = bundle.json_bytes(manifest)
        archive = self.root / "input.zip"
        with zipfile.ZipFile(archive, "w") as output:
            for name, content in files.items():
                output.writestr(name, content)
            output.writestr(bundle.MANIFEST, payload)
        lock = {"manifestSha256": hashlib.sha256(payload).hexdigest(), "archive": {
            "size": archive.stat().st_size, "sha256": bundle.digest(archive)}}
        (self.root / bundle.LOCK).write_bytes(bundle.json_bytes(lock))
        return archive

    def test_install_and_repeat_preserve_identical_inputs(self):
        archive = self.archive({"deps/tool/include/header.h": b"original header"})
        bundle.install(archive, self.root)
        target = self.root / "deps/tool/include/header.h"
        timestamp = target.stat().st_mtime_ns
        bundle.install(archive, self.root)
        self.assertEqual(target.read_bytes(), b"original header")
        self.assertEqual(target.stat().st_mtime_ns, timestamp)

    def test_existing_edit_rejects_before_any_other_file_is_written(self):
        archive = self.archive({"deps/new.h": b"new", "deps/edited.h": b"original"})
        existing = self.root / "deps/edited.h"
        existing.parent.mkdir()
        existing.write_bytes(b"user edit")
        with self.assertRaisesRegex(ValueError, "Preserving changed"):
            bundle.install(archive, self.root)
        self.assertEqual(existing.read_bytes(), b"user edit")
        self.assertFalse((self.root / "deps/new.h").exists())

    def test_declared_path_traversal_is_rejected(self):
        archive = self.archive({"deps/../escape.h": b"escape"})
        with self.assertRaisesRegex(ValueError, "Unsafe dependency member"):
            bundle.install(archive, self.root)
        self.assertFalse((self.root / "escape.h").exists())

    def test_windows_case_collisions_are_rejected(self):
        archive = self.archive({"deps/Header.h": b"one", "deps/header.h": b"two"})
        with self.assertRaisesRegex(ValueError, "Duplicate"):
            bundle.install(archive, self.root)

    def test_archive_hash_is_checked_before_extraction(self):
        archive = self.archive({"deps/header.h": b"original"})
        with archive.open("ab") as output:
            output.write(b"tamper")
        with self.assertRaisesRegex(ValueError, "pinned identity"):
            bundle.install(archive, self.root)
        self.assertFalse((self.root / "deps").exists())

    def test_vendor_matches_recorded_public_git_tree(self):
        vendor = ROOT / "src/dependencies/small-script"
        record = json.loads((vendor / "upstream.json").read_bytes())
        self.assertEqual(record["repository"], "https://github.com/kran27/small-script")
        for name, expected in record["files"].items():
            self.assertEqual(bundle.digest(vendor / name), expected, name)
        self.assertFalse((vendor / "dotnet").exists())
        self.assertFalse((vendor / "tools/amxdecomp.py").exists())


if __name__ == "__main__":
    unittest.main()

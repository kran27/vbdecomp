"""Source verification works in a public export without Git or patch artifacts."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]


class SourceSetupTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

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

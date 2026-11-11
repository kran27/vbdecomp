"""Download and prepare pinned build inputs; never execute original game code."""
import argparse
import json
from pathlib import Path, PurePosixPath
import shutil
import subprocess
import tempfile

from dependency_bundle import ROOT, LOCK, digest, install, verify
from workspace_config import configuration


def download(spec, destination):
    destination = Path(destination)
    if destination.exists():
        if destination.stat().st_size != spec["size"] or digest(destination) != spec["sha256"]:
            raise ValueError("Preserving changed cached download: " + str(destination))
        return destination
    if not spec.get("url"):
        raise ValueError("No dependency CDN URL is configured yet. Pass -DependencyBundle <archive.zip> to Setup.ps1.")
    destination.parent.mkdir(parents=True, exist_ok=True)
    partial = destination.with_name(destination.name + ".download")
    curl = shutil.which("curl.exe") or shutil.which("curl")
    if not curl:
        raise FileNotFoundError("curl is required for downloads; alternatively supply the archives explicitly")
    subprocess.run([curl, "--fail", "--location", "--retry", "2", "--continue-at", "-",
                    "--user-agent", "Mozilla/5.0", "--output", str(partial), spec["url"]], check=True)
    if partial.stat().st_size != spec["size"] or digest(partial) != spec["sha256"]:
        raise ValueError("Downloaded archive differs from its pinned identity: " + str(partial))
    if destination.exists():
        raise FileExistsError("Preserving concurrently published download: " + str(destination))
    partial.rename(destination)
    return destination


def verify_game(directory, spec):
    for name, expected in spec["requiredFiles"].items():
        path = directory / name
        if not path.is_file() or digest(path) != expected:
            raise ValueError("Missing or changed original game input: " + str(path))
    print("Verified original F3 executable and runtime DLLs; no game code executed")


def game(archive=None):
    spec = json.loads((ROOT / "config/game-download.json").read_bytes())
    destination = configuration()["gameRoot"]
    if destination.exists():
        verify_game(destination, spec)
        return
    # Download/extract only to our managed location. An explicit external path
    # denotes an existing installation and is never populated or modified.
    if destination != (ROOT / "deps/game").resolve():
        raise FileNotFoundError("Configured gameRoot does not exist: " + str(destination))
    archive = Path(archive).resolve() if archive else download(spec["archive"], ROOT / "deps/downloads/F3_Demo.rar")
    if archive.stat().st_size != spec["archive"]["size"] or digest(archive) != spec["archive"]["sha256"]:
        raise ValueError("Original game archive identity differs")
    tar = shutil.which("tar.exe") or shutil.which("tar")
    if not tar:
        raise FileNotFoundError("A RAR-capable tar is required (included with current Windows)")
    members = subprocess.check_output([tar, "-tf", str(archive)], text=True).splitlines()
    for name in members:
        relative = PurePosixPath(name.replace("\\", "/"))
        if (relative.is_absolute() or ":" in name or ".." in relative.parts
                or any(part.lower() == ".git" for part in relative.parts)):
            raise ValueError("Unsafe game archive member: " + name)
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="game-extract-", dir=destination.parent) as temporary:
        staging = Path(temporary) / "game"
        staging.mkdir()
        subprocess.run([tar, "-xf", str(archive), "-C", str(staging)], check=True)
        verify_game(staging, spec)
        if destination.exists():
            raise FileExistsError("Preserving concurrently created game folder: " + str(destination))
        staging.rename(destination)


def dependencies(archive=None):
    if archive:
        install(Path(archive).resolve())
        return
    if (ROOT / "deps/dependencies-manifest.json").exists():
        verify()
        return
    spec = json.loads((ROOT / LOCK).read_bytes())["archive"]
    install(download(spec, ROOT / "deps/downloads" / spec["filename"]))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dependency-bundle", type=Path)
    parser.add_argument("--game-archive", type=Path)
    parser.add_argument("--game-only", action="store_true")
    args = parser.parse_args()
    if not args.game_only:
        dependencies(args.dependency_bundle)
    game(args.game_archive)


if __name__ == "__main__":
    main()

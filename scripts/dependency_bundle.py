"""Package, install and verify the original build dependencies without running them."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import zipfile

ROOT = Path(__file__).resolve().parents[1]
LOCK = "config/dependency-bundle.json"
MANIFEST = "deps/dependencies-manifest.json"
COMPONENTS = {
    "msvc70": "Microsoft VC7 13.00.9466, LINK/LIB 7.00.9466 and original CRT/STL",
    "psdk2001": "Microsoft Platform SDK, November 2001, 5.1.3590.2",
    "dx81": "Microsoft DirectX 8.1 SDK headers and D3DX library",
    "dx81-audio": "DirectX 8.1 audio declarations and import libraries",
    "dx81-errors": "DirectX 8.1 error helper",
    "dx81-font": "DirectX 8.1 font sample",
    "dx81-samples": "DirectX 8.1 common sample sources",
    "adecode94": "Retained Interplay ACM decoder archive",
}


def digest(path):
    with Path(path).open("rb") as stream:
        return hashlib.file_digest(stream, "sha256").hexdigest()


def json_bytes(value):
    return (json.dumps(value, indent=2) + "\n").encode("utf-8")


def safe_path(root, name):
    relative = PurePosixPath(name)
    if ("\\" in name or ":" in name or relative.is_absolute()
            or any(part in ("..", ".git") for part in relative.parts)
            or not relative.parts or relative.parts[0] != "deps"):
        raise ValueError("Unsafe dependency member: " + name)
    if not (root / "deps").resolve().is_relative_to(root.resolve()):
        raise ValueError("Dependency root escapes the checkout")
    path = (root / Path(*relative.parts)).resolve()
    if not path.is_relative_to((root / "deps").resolve()):
        raise ValueError("Dependency path escapes deps: " + name)
    return path


def read_manifest(root=ROOT):
    lock = json.loads((root / LOCK).read_bytes())
    payload = (root / MANIFEST).read_bytes()
    if hashlib.sha256(payload).hexdigest() != lock["manifestSha256"]:
        raise ValueError("Dependency manifest differs from the pinned bundle")
    manifest = json.loads(payload)
    if manifest.get("schemaVersion") != 1 or not manifest.get("files"):
        raise ValueError("Invalid dependency manifest")
    return manifest


def verify(root=ROOT):
    manifest = read_manifest(root)
    for name, expected in manifest["files"].items():
        path = safe_path(root, name)
        if not path.is_file() or path.stat().st_size != expected["size"] or digest(path) != expected["sha256"]:
            raise ValueError("Missing or changed original dependency: " + name)
    print("Verified %d pinned dependency files" % len(manifest["files"]))
    return manifest


def install(archive, root=ROOT):
    lock = json.loads((root / LOCK).read_bytes())
    if archive.stat().st_size != lock["archive"]["size"] or digest(archive) != lock["archive"]["sha256"]:
        raise ValueError("Dependency archive differs from its pinned identity")
    with zipfile.ZipFile(archive) as bundle:
        names = bundle.namelist()
        if len(names) != len(set(names)) or len(names) != len({name.casefold() for name in names}):
            raise ValueError("Duplicate dependency archive members")
        payload = bundle.read(MANIFEST)
        if hashlib.sha256(payload).hexdigest() != lock["manifestSha256"]:
            raise ValueError("Bundled manifest identity differs")
        manifest = json.loads(payload)
        if set(names) != set(manifest["files"]) | {MANIFEST}:
            raise ValueError("Dependency archive contains undeclared files")
        # Check every member and existing destination before writing anything.
        for name in names:
            target = safe_path(root, name)
            info = bundle.getinfo(name)
            if info.external_attr >> 16 & 0o170000 == 0o120000:
                raise ValueError("Dependency symlinks are not supported")
            content = bundle.read(name)
            expected = manifest["files"].get(name)
            if expected and (len(content) != expected["size"] or hashlib.sha256(content).hexdigest() != expected["sha256"]):
                raise ValueError("Bundled dependency identity differs: " + name)
            if target.exists() and (not target.is_file() or target.read_bytes() != content):
                raise ValueError("Preserving changed existing dependency: " + str(target))
        for name in names:
            target = safe_path(root, name)
            if not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                with target.open("xb") as output:
                    output.write(bundle.read(name))
    return verify(root)


def pack(output, root=ROOT):
    if output.exists():
        raise FileExistsError("Preserving existing dependency archive: " + str(output))
    if (root / MANIFEST).exists():
        verify(root)
    files = {}
    for component in COMPONENTS:
        directory = root / "deps" / component
        if not directory.is_dir():
            raise FileNotFoundError(directory)
        for path in sorted(directory.rglob("*")):
            if not path.is_file() or ".git" in path.parts or path.suffix.lower() == ".json":
                continue  # Old extraction receipts contain machine-specific paths.
            if path.is_symlink():
                raise ValueError("Dependency source symlink: " + str(path))
            name = path.relative_to(root).as_posix()
            files[name] = {"size": path.stat().st_size, "sha256": digest(path)}
    readme = root / 'deps/README.bundle.md'
    readme.write_text("""# Van Buren original-toolchain dependencies

Extract this ZIP at the root of a vbdecomp checkout, or pass its path to
scripts/Setup.ps1 -DependencyBundle <zip>. Setup checks the pinned archive and
per-file hashes and will not replace different existing inputs. It never runs
an installer or game. See config/dependency-bundle.json for the bundle identity.

Included: VC7 13.00.9466 and LINK/LIB 7.00.9466 with their CRT/STL; November 2001
Platform SDK 5.1.3590.2; DirectX 8.1 declarations, libraries and common samples;
and the unchanged identified ACM decoder archive. Component notices and SDK
EULA/REDIST files remain in their original directories.

November 2001 is the supported SDK baseline, with 177 named import fingerprints
and 17 Winsock ordinals independently matched to F3. Retained D3DX checks cover
141 procedures/42,953 bytes; the font sample constructor and ACM provider have
separate original-code evidence. These bounded results are not a whole-library
or whole-executable equality claim. Read docs/dependency-fidelity.md in the source
checkout for the evidence boundaries.

Game files are downloaded separately from https://vb.kran.gg/F3_Demo.rar.
Small is vendored from kran27/small-script in the source checkout; zlib is also
vendored there. This ZIP contains neither of those source dependencies nor the
provisional CString-derived formatting scanner, replacement runtimes, personal
configuration, databases or historical build outputs.
""", encoding='utf-8', newline='\n')
    files['deps/README.bundle.md'] = {'size': readme.stat().st_size, 'sha256': digest(readme)}
    manifest = {"schemaVersion": 1, "components": COMPONENTS, "files": files}
    payload = json_bytes(manifest)
    output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(output, "x", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as bundle:
        for name in sorted(files):
            info = zipfile.ZipInfo(name, date_time=(1980, 1, 1, 0, 0, 0))
            info.compress_type = zipfile.ZIP_DEFLATED
            bundle.writestr(info, (root / name).read_bytes(), compresslevel=9)
        info = zipfile.ZipInfo(MANIFEST, date_time=(1980, 1, 1, 0, 0, 0))
        info.compress_type = zipfile.ZIP_DEFLATED
        bundle.writestr(info, payload, compresslevel=9)
    lock_path = root / LOCK
    previous = json.loads(lock_path.read_bytes()) if lock_path.exists() else {}
    lock = {"schemaVersion": 1, "manifestSha256": hashlib.sha256(payload).hexdigest(),
            "archive": {"filename": output.name, "url": previous.get("archive", {}).get("url"),
                        "size": output.stat().st_size, "sha256": digest(output)},
            "components": COMPONENTS,
            "scope": "Extracted original tools, SDKs and historical libraries; no game files or replacement runtimes. Original notices are retained inside each component."}
    lock_path.write_bytes(json_bytes(lock))
    destination = root / MANIFEST
    destination.write_bytes(payload)
    output.with_suffix(output.suffix + ".sha256").write_text(digest(output) + "  " + output.name + "\n", encoding="ascii")
    print("Created %s (%d files, %.1f MiB)" % (output, len(files), output.stat().st_size / 1048576))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("verify")
    commands.add_parser("install").add_argument("archive", type=Path)
    commands.add_parser("pack").add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.command == "verify":
        verify()
    elif args.command == "install":
        install(args.archive.resolve())
    else:
        pack(args.output.resolve())


if __name__ == "__main__":
    main()

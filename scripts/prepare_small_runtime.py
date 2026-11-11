"""Prepare hash-pinned Small sources using the checked-in F3 adaptation.

The supplied checkout is read-only. No diff is applied and no VM is executed.
"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def sha(data):
    return hashlib.sha256(data).hexdigest()


def prepare(source_root, destination):
    lock = json.loads((ROOT / "config/small-runtime.lock.json").read_text())
    destination = Path(destination).resolve()
    source_root = Path(source_root).resolve()
    if destination == source_root or source_root in destination.parents:
        raise ValueError("Small outputs cannot be inside the supplied source repository")
    inputs, outputs, adapted = {}, {}, {}
    for name, expected in lock["sources"].items():
        data = (source_root / "src" / name).read_bytes()
        if sha(data) != expected:
            raise ValueError("Supplied Small source changed; review before updating the lock: " + name)
        inputs[name] = expected
        replacement = lock["adapted_sources"].get(name)
        if replacement is not None:
            data = (ROOT / replacement["path"]).read_bytes()
            if sha(data) != replacement["sha256"]:
                raise ValueError("Checked-in Small adaptation identity differs: " + name)
            adapted[name] = replacement["sha256"]
        outputs[name] = data
    destination.mkdir(parents=True, exist_ok=True)
    for name, data in outputs.items():
        output = destination / name
        if not output.exists() or output.read_bytes() != data:
            output.write_bytes(data)
    report = dict(source_inputs=inputs, adapted_sources=adapted,
                  outputs={name: sha(data) for name, data in outputs.items()},
                  user_checkout_modified=False, executed_vm_or_script=False)
    (destination / "source-receipt.json").write_text(json.dumps(report, indent=2) + "\n")
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()
    prepare(args.source_root, args.output)


if __name__ == "__main__":
    main()

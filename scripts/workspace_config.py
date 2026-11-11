"""Resolve optional user paths relative to the checkout, never to the shell cwd."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def configuration(root=ROOT):
    values = {"gameRoot": "deps/game", "personalDatabase": ""}
    local = root / "config/local.json"
    if local.exists():
        supplied = json.loads(local.read_text(encoding="utf-8-sig"))
        for key in values:
            if supplied.get(key):
                values[key] = supplied[key]
    for key, value in values.items():
        if value:
            path = Path(value)
            values[key] = (path if path.is_absolute() else root / path).resolve()
    return values

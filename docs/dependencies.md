# Build inputs

See [dependencies](dependency-fidelity.md) for versions, sources and match status.

The dependency ZIP contains VC7, the Platform SDK, DirectX SDK files and
Adecode.lib, with their original notices. Small and zlib are vendored in source.

Pass the ZIP to `scripts/Setup.ps1 -DependencyBundle <path>`. Setup verifies it
against `config/dependency-bundle.json` and extracts it under untracked `deps/`.

To check installed dependencies offline:

```powershell
./.venv/Scripts/python.exe scripts/dependency_bundle.py verify
```

Setup downloads [the original demo](https://vb.kran.gg/F3_Demo.rar) separately
into `deps/game`, including Granny/Bink DLLs. Use `-GameArchive <path>` for a
local RAR or `-GameRoot <path>` for an existing installation.

Host requirements: Windows, PowerShell, Python 3.11+, curl and a RAR-capable tar.
Setup installs the Python dependency `pefile`.

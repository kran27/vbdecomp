# Build inputs

The build uses the original VC7 compiler/CRT/STL, November 2001 Platform SDK
5.1.3590.2, DirectX 8.1 and the identified historical helpers. See the
[evidence table](dependency-fidelity.md) for what has been established.

The shareable dependency ZIP contains the extracted VC7 files, Platform SDK,
DirectX SDK declarations/libraries/common samples and the unchanged ACM archive.
Original notices are retained. It contains neither game files, vendored Small,
replacement runtimes, nor the provisional WTL-derived formatting source.

`config/dependency-bundle.json` pins its complete archive and manifest hashes.
`scripts/dependency_bundle.py install` verifies the archive and every selected
file before extraction and refuses to replace different existing inputs.
`scripts/dependency_bundle.py verify` checks an installed bundle without network
access. Personal extraction receipts and machine paths are excluded.

Small comes directly from the user's public fork and is vendored in source.
zlib 1.1.4 is also vendored. Their original notices and provenance are retained.

The original game is a separate hash-pinned RAR at
[the demo CDN](https://vb.kran.gg/F3_Demo.rar), selected by
`config/game-download.json`. Setup extracts it under ignored `deps/game`, or
checks an explicitly configured existing installation without modifying it.
The original DLLs supply Granny/Bink; modern SDK substitutes are not selected.

The host requires PowerShell, Python 3.11+, curl and a RAR-capable tar. Only
`pefile` is required for setup. The PE/COFF inspection helper also uses pefile; the broader historical analysis
tooling remains local.
No personal IDA database, external toolkit checkout, desktop SDK, CMake graph or
modern Visual Studio installation is required to build.

# Van Buren Decompilation

A work-in-progress decompilation of the original Fallout: Van Buren demo.
The goal is to produce an accurate representation of the original tech demo, using
the original compiler, runtime libraries and period SDKs. This is not yet an exact
or complete reproduction, though the main content is fully functional.

All original bugs and quirks are currently maintained, once a greater level of
accuracy is achieved, a secondary project will be created to house improvements.
This repo will remain a reference project for the original engine functionality.

### State as of Publication

Out of 10,110 original functions, 9,949 (98.4%) have been attributed to their original source.
5,242 of these are specific game/engine functions, 4,662 have bodies, but are not guaranteed accurate.

At least 3,447 game/engine functions (65.8%) have historical exact-match receipts, covering about 34.5% of classified game/engine code bytes. These matches have not been revalidated after the latest cleanup.

The remaining classifications are: 4,115 library functions, 592 compiler-generated "glue" functions, and 161 functions of unresolved origin.


## Build

The original Visual C++ 7.0 compiler and runtime, November 2001 Platform SDK,
and DirectX 8.1 SDK are currently used, and no support is given for modern toolchains.
Game and engine sources use C++98; Small and zlib compile as C.

This project expects Windows, PowerShell and Python 3.11 or newer. Setup uses `curl`
and a RAR-capable `tar`, both supplied with current Windows. These are used to gather
and run the original dependencies.

### Instructions

First, run the following:

```powershell
./scripts/Setup.ps1
```

This downloads and verifies the SDK/tool bundle and the original demo under `deps/`,
generates import libraries from its original DLLs, and builds Release. Pass `-DependencyBundle` for an already downloaded dependency ZIP,
`-GameArchive` for an already downloaded demo archive, or
`-GameRoot` to use an existing original installation without modifying it.

After setup, building is done by running

```powershell
./scripts/Build.ps1 -Configuration Release
```

The build script reads [explicit target lists](config/build-targets.json) directly.
See [build details](docs/build.md) for more information.

For drop-in use, copy `build/legacy-vc7/release/F3-openvb.exe` beside the original
demo's `F3.exe`. Start it with that demo directory as the working directory;
it uses the original assets and DLLs directly. Settings and saves
use `Documents/F3-openvb/`.

## Source and evidence

- Game: `repos/openvb/src`; engine: `repos/openvb/openje/src`.
- Vendored Small: `src/dependencies/small-script`, from
  [kran27/small-script](https://github.com/kran27/small-script), including the F3
  runtime and console adaptations in the fork itself.
- Resource zlib: unmodified 1.1.4 source under `src/dependencies/zlib-1.1.4`.
- [Dependency versions and match status](docs/dependency-fidelity.md).
- [Source control and upstream attribution](docs/source-control.md).
- [Matching policy](docs/matching-policy.md) and [progress reporting](docs/reporting.md).

## Attribution

This work builds on [OpenJE/openvb](https://github.com/OpenJE/openvb) and
[OpenJE/openje](https://github.com/OpenJE/openje), with their original source
notices retained. It is unaffiliated with the original developers or rights
holders. Dependency copyright and license notices are retained with their source
or in the dependency bundle; see [third-party notices](THIRD_PARTY.md).

## AI Disclosure

As is the case with many decompilation projects, AI (specifically ChatGPT Codex) has
been used extensively for the grunt work of decompilation. The AI work builds on years
of community reverse engineering work, and remains bounded to reproducing the original code.

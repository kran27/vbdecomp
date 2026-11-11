# Van Buren reconstruction

A work-in-progress reconstruction of the original Fallout: Van Buren demo and
its Jefferson engine. The baseline is the original Windows x86 program, its
compiler, runtime libraries and period SDKs. This is not yet an exact or complete
reproduction, and the reconstructed startup is unfinished.

## Build

The game uses **Visual C++ 7.0, compiler 13.00.9466 / LINK 7.00.9466**, the
**November 2001 Platform SDK 5.1.3590.2**, **DirectX 8.1**, and the real VC7 CRT/STL.
Game and engine sources use C++98; Small and zlib compile as C.

The host needs Windows, PowerShell and Python 3.11 or newer. Setup uses `curl`
and a RAR-capable `tar`, both supplied with current Windows. These are build
utilities; game compilation uses the pinned original toolchain exclusively.

Obtain the dependency ZIP described in [dependencies](docs/dependencies.md), then:

```powershell
./scripts/Setup.ps1 -DependencyBundle C:/Downloads/vbdecomp-dependencies-v3.zip
```

Setup verifies/extracts the SDK/tool bundle under untracked `deps/`, downloads
[the original demo](https://vb.kran.gg/F3_Demo.rar) into `deps/game`, generates
import libraries from its original DLLs, and builds Release. It does not launch
the game. Once a dependency CDN URL is configured, `Setup.ps1` can download the
bundle too. Pass `-GameArchive` for an already downloaded demo archive, or
`-GameRoot` to use an existing original installation without modifying it.

```powershell
./scripts/Build.ps1 -Configuration Release
./scripts/Build.ps1 -Target startup-core -CompileOnly
./scripts/Build.ps1 -PlanOnly
```

The build reads [explicit target lists](config/build-targets.json) directly.
CMake, Ninja and modern Visual Studio are not required. The executable uses the
reconstructed original startup; no interactive test harness is built into it.
Successful compilation/linking does not establish runtime correctness or exact
binary equality. See [build details](docs/build.md).

## Source and evidence

- Game: `repos/openvb/src`; engine: `repos/openvb/openje/src`.
- Vendored Small: `src/dependencies/small-script`, from
  [kran27/small-script](https://github.com/kran27/small-script), with separately
  recorded F3 runtime adaptations under `src/dependencies/small-runtime`.
- Resource zlib: unmodified 1.1.4 source under `src/dependencies/zlib-1.1.4`.
- [Dependency evidence and remaining questions](docs/dependency-fidelity.md).
- [Source control and upstream attribution](docs/source-control.md).
- [Matching policy](docs/matching-policy.md) and [local reporting](docs/reporting.md).

Accuracy work is paused during repository cleanup. Historical analysis, receipts,
generated reports and retired workflows remain local and are not prerequisites
to building a public checkout. Modern ports and replacement libraries are later
work and are outside this baseline.

## Attribution

This work builds on [OpenJE/openvb](https://github.com/OpenJE/openvb) and
[OpenJE/openje](https://github.com/OpenJE/openje), with their original source
notices retained. It is unaffiliated with the original developers or rights
holders. Dependency copyright and license notices are retained with their source
or in the dependency bundle; see [third-party notices](THIRD_PARTY.md).

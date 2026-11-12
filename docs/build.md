# Build

The host needs Windows, PowerShell, Python 3.11+, curl and a RAR-capable tar.
Compilation uses the pinned VC7 toolchain. See [build inputs](dependencies.md)
for the SDK/tool bundle.

```powershell
./scripts/Setup.ps1
```

Setup downloads, verifies and extracts dependencies, downloads the original demo
and builds Release. Use `-SkipBuild` to install inputs only,
`-DependencyBundle <path>` to reuse a downloaded ZIP, `-GameArchive <path>` to reuse
a downloaded RAR, or `-GameRoot <path>` for an existing game installation.

```powershell
./scripts/Build.ps1 -Configuration Release
./scripts/Build.ps1 -Target startup-core -CompileOnly
```

Outputs go to `build/legacy-vc7/release`. `-OutputDirectory` selects another
location. `-PlanOnly` lists selected units; `-Rebuild` forces compilation;
`-KeepGoing` collects failures. Debug is available through `-Configuration Debug`.

Source groups and link inputs are in `config/build-targets.json`; compiler
options are in `config/legacy-build.json`. Optional local paths go in ignored
`config/local.json`, using `config/local.example.json` as a template.

Copy `build/legacy-vc7/release/F3-openvb.exe` into the original demo directory,
beside `resource.rht`, `data/`, `binkw32.dll` and `granny2.dll`. This single
executable enters the reconstructed game directly. Start it with the demo
directory as its working directory. It needs no launcher, arguments file or
original `F3.exe` at runtime.
Keep the original game files and runtime DLLs in place.

The executable name determines the settings, log and save folder:
`Documents/F3-openvb/` for `F3-openvb.exe`, separate from the original
`Documents/F3/`. Build scripts still inspect the original `F3.exe` when generating
the Granny import library.

The reconstructed startup is incomplete. Successful builds do not yet produce
a verified playable or exact executable.

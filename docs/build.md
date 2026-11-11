# Build

The host needs Windows, PowerShell, Python 3.11+, curl and a RAR-capable tar.
Compilation uses the pinned VC7 toolchain. See [build inputs](dependencies.md)
for the SDK/tool bundle.

```powershell
./scripts/Setup.ps1 -DependencyBundle C:/Downloads/vbdecomp-dependencies-v3.zip
```

Setup verifies and extracts dependencies, downloads the original demo and builds
Release. Use `-SkipBuild` to install inputs only, `-GameArchive <path>` to reuse
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

The reconstructed startup is incomplete. Successful builds do not yet produce
a verified playable or exact executable.

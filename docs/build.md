# Build the original-toolchain baseline

Run `scripts/Setup.ps1` with the pinned SDK/tool ZIP. The dependency bundle and
original game archive are verified before extraction. Existing different files
are preserved and reported as errors. Setup never runs an installer or game.

`config/local.json` is optional and ignored. Relative paths resolve from the
checkout. `gameRoot` defaults to `deps/game`; `personalDatabase` is optional
analysis configuration and is not used by setup or compilation. See
`config/local.example.json`.

```powershell
./scripts/Setup.ps1 -DependencyBundle C:/Downloads/vbdecomp-dependencies-v3.zip -SkipBuild
./scripts/Build.ps1 -Configuration Release
```

Use `-GameArchive` to reuse the downloaded RAR. A configured external `-GameRoot`
is read-only and must already contain the original executable and DLLs. The
managed `deps/game` directory is populated automatically from the pinned CDN
archive. Game files are never part of the SDK/tool ZIP or tracked source.

The PowerShell driver reads `config/build-targets.json` for source groups,
headers, definitions and links. Compiler switches remain explicit in
`config/legacy-build.json`. Every object/archive/executable is produced by the
hash-checked VC7 CL/LIB/LINK tools. Small runtime, header and console sources are compiled directly from the pinned
fork under `src/dependencies/small-script/src`; no source adaptation step is run.
Default outputs are in `build/legacy-vc7/release`.

For an independent build, choose a new output directory:

```powershell
./scripts/Build.ps1 -Configuration Release -OutputDirectory build/check
```

`-Target` selects an explicit target or source group. `-CompileOnly` skips
archives and executable links. `-PlanOnly` lists the selected units without
compiling. `-Rebuild` forces compilation; otherwise source/header/command hashes
govern reuse. `-KeepGoing` collects compilation failures. Debug can be selected
explicitly but is not a separate release gate for routine work.

There are no CMake configure/build steps, modern compiler alternatives, automatic
runtime tests, or interactive fixture launchers in this build. The output uses
the incomplete original startup and must not be described as a playable release.
Compilation, linking, behavior and exact matching require different evidence.

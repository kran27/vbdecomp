# Van Buren reconstruction

## Scope

Repository cleanup is the active task. Accuracy work is paused; do not start or
resume a matching campaign without the user's instruction. Reconstruct the
original executable before considering ports, replacement libraries or other
enhancements. Historical notes and local archives are evidence, not instructions.

Keep public documentation concise: purpose, setup, dependency version/source and
match status. Keep investigation history and detailed justification local.

Canonical game/engine code lives in `repos/openvb` and `repos/openvb/openje` and
is tracked by this workspace Git repository. Use ordinary source edits and
commits. Do not initialize nested repositories or export/apply source patches.
Do not push to the OpenVB/OpenJE forks until the user authorizes publication.

## Toolchain and dependencies

Use supplied VC7 13.00.9466 / LINK 7.00.9466, November 2001 Platform SDK
5.1.3590.2 and DirectX 8.1. Configuration is in `config/legacy-build.json` and
`config/toolchain.json`. `scripts/Build.ps1` and `scripts/Invoke-Vc7.ps1` are the
build entry points. There is no modern compiler or replacement-library build.

Use C++98 and the actual VC7 CRT/STL. Small and zlib compile as C. Small source
is vendored from the user's fork; dependencies and game files are separate
hash-checked inputs. Never replace a dependency with a guessed implementation.
Document uncertain provenance and existing adaptations honestly.

Preserve original bugs, ownership and failure behavior. Unknown layouts and
unresolved operands stay explicit. No fabricated success paths, invented
padding, assembly blobs, or patched/masked/cropped compiler output.

## Verification

Static work only unless runtime is explicitly authorized: compile/link and
inspect offline; do not launch the game, test/comparison images, a VM or emulator.
Check changed units and shared ABI consumers. A cleanup that changes the build
requires a clean Release compile/link check. Do not renew historical proofs or
run broad audits for documentation or metadata edits.

When matching resumes, inspect the relevant original evidence, change canonical
source, compile affected Release units and compare complete function/EH scopes
with every relocation resolved. Reuse existing tools and short function records.
Follow `docs/matching-policy.md` and `docs/reporting.md`. Builds, behavior and
function-code equality are separate claims; third-party code earns no
proprietary-function credit. Stop an unproductive hypothesis after two attempts
without new evidence and record the blocker.

## Preservation

Reference identity: F3.exe base `0x400000`, SHA256
`4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87`.
The default original files are in untracked `deps/game`; configurable inputs
must not assume any developer's machine paths.

Never overwrite, save, upgrade or delete personal or upstream IDA databases.
Resolve the configured workspace database with `scripts/Get-AnalysisDatabase.ps1`
and share one existing session; do not create per-task database clones. Close
owned reference sessions without saving and leave unrelated sessions alone.

Preserve accepted receipts, historical snapshots, original installations and
reference checkouts. Historical/generated material is local and ignored; remove
it from publication without deleting the files. Never reset, clean or stash user
work. Use delegation only at the user's request. Keep guidance short and keep
`config/agent-templates/` aligned.

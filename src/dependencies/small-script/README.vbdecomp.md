# Workspace integration

The upstream Git tree is preserved verbatim at the revision and file hashes in
upstream.json. The fork owns the F3 AMX runtime, its eight user-data slots and the
selected console formatter. The workspace compiles those sources directly as C
with VC7; it does not copy replacement sources or generate an adapted runtime.

The upstream README/CMake files describe that project's own build. Use the
workspace Build.ps1 for the original-toolchain game build. No decompiler project
or private history is vendored.

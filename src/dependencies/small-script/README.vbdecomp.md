# Workspace integration

The upstream Git tree is preserved verbatim at the revision and file hashes in
upstream.json. Its README and CMake files describe the upstream project; the
workspace builds only the selected C sources with its own pinned VC7 driver.
No decompiler code or private development history is vendored.

F3 runtime adaptations live separately in ../small-runtime. The preparation
scripts copy those explicit adaptations and reuse the other vendored files.
Do not modify upstream files or introduce replacement implementations silently.

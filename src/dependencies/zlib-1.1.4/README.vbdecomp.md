# Resource gzip dependency

These 27 files are verbatim members of the official zlib 1.1.4 fossil archive.
`upstream.json` records the archive identity and each retained file hash. The
upstream license is in `zlib.h` and `README`; no implementation/header is patched.
The workspace CMake file supplies the build recipe because this release predates
CMake. Compile the implementation as C with the supplied VC7 CRT.

This replaces the resource wrapper's modern 1.3.2 substitution. Original version
arguments, 56-byte stream initialization and a complete version-distinguishing
`huft_build` match support 1.1.4. D3DX's separate 1.1.3 lineage stays in the
supplied SDK archive. Vendor namespace/I/O changes and complete library equality
remain unresolved; see `docs/dependency-fidelity.md`.

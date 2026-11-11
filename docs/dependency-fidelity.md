# Dependency evidence

The selected baseline uses original-period tools and libraries. Hash checks
identify the supplied files; original-code comparisons support the dependency
choices described below. Neither a matching hash nor a successful build alone
proves that an entire reconstructed executable matches F3.

| Component | Evidence | Scope and remaining work |
| --- | --- | --- |
| VC7 13.00.9466 / LINK 7.00.9466 and CRT/STL | Original supplied tool binaries and headers/libraries; pinned hashes and retained code/layout comparisons | Compiler switches and individual caller layouts remain function-specific. |
| November 2001 Platform SDK 5.1.3590.2 | Authenticated media extraction; all 177 named Windows import fingerprints and 17 Winsock ordinals agree, independently checked | This is the supported original SDK baseline. The library fingerprints do not separately identify every preprocessed header or hypothetical vendor edit. |
| DirectX 8.1 / D3DX | Authentic SDK package; retained static linked comparisons cover 141 complete codec/helper procedures and 42,953 original code bytes | The checked scope is not a claim of whole-library equality. |
| CD3DFont | Authentic SDK sample; original fields/API identified and complete 66-byte constructor matches | Other methods and rendered glyph pixels have separate verification limits. This statistics font is distinct from the game's GUI font. |
| Interplay ACM decoder | Unmodified `lib/win/Adecode.lib` from the [Descent 3 source release](https://github.com/DescentDevelopers/Descent3/blob/df209742fcda59d466b849eff9ee0e7b908d7d40/lib/win/Adecode.lib); retained comparisons show exact equality for 28 complete F3 routines (5,772 code bytes) and 688 initialized data bytes after resolving every relocation | One additional allocator-setting function in the library has no identified F3 match. We do not have its original `adecode.c` or a confirmed decoder version number. |
| Small 2.3.1 | Vendored directly from the user's fork, which owns the F3 runtime/console adaptations; retained complete 8,480-byte AMX runtime match | The result does not certify every compiler/debug-format variation or every original vendor change. |
| Resource zlib 1.1.4 | Original version banners, initialization calls, 56-byte stream layout and complete function comparisons; source is verbatim upstream | No whole-library/vendor-patch claim. D3DX's internal zlib is a separate implementation. |
| Game DLLs | Exact CDN archive, F3.exe hash and runtime-DLL hashes; imports checked without loading DLLs | Original game files remain a separate download. Granny 2.8 and replacement Bink/Granny runtimes are not used. |

The Descent 3 initial import retained `Adecode.lib` as a legacy binary; its active
build uses `libacm`. The adjacent `lib/Adecode.h` describes the newer C++ interface.
Our build uses the binary's C exports declared in
[SoundAcm.cpp](../repos/openvb/openje/src/audio/SoundAcm.cpp). The acquisition
repository, commit, upstream path, download URL, size and SHA-256 are recorded in
[`config/dependency-origins.json`](../config/dependency-origins.json).

Resource zlib supports optional gzip-compressed files with an added `.z` suffix.
If the ordinary file is missing, the file manager expands `<name>.z` into
`<root>\cache\<name>` and opens that cached copy. This can wrap an entire `.grp`
file; the contained resource formats keep their normal layouts. The downloaded
demo contains ordinary `.grp` files and no `.z` files, so this fallback appears
unused by its supplied assets. D3DX's PNG decoding is a separate zlib consumer.

The formatting helper at `0x4A1610` has apparent Microsoft CString scanner
lineage: modifier masks, fixed estimates, and its unusual floating-point buffer
calculation agree. This does not establish that F3 linked WTL. Its original
source/version remains unresolved. The existing source-derived scanner is retained directly beside StringFormat.cpp
with its license and provenance. It remains a provisional source adaptation, not
a verified original WTL dependency or an exact match. No WTL package is installed
or required by the build; the SDK/tool bundle excludes it.

Detailed historical comparisons remain local, including `analysis/sdk-nov2001-v1`,
`analysis/d3dx-linked-codecs-20261023-v1`, `docs/amx-static-fidelity.md`,
`docs/font-dependency.md`, and `docs/legacy-format.md`. They are evidence, not an
active work queue. Accuracy work stays paused during repository cleanup.

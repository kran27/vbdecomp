# Upstream and third-party notices

Game and engine reconstruction build on OpenJE/openvb and OpenJE/openje. Original
copyright comments remain in their source. `repos.lock.json` records origins,
baseline revisions and publication destinations.

Vendored Small is from kran27/small-script at the revision in
`src/dependencies/small-script/upstream.json`; its LICENSE is retained there.
The F3 runtime adaptations retain their notices under
`src/dependencies/small-runtime`. zlib 1.1.4 retains the upstream license text
in its headers and README, with file/archive provenance in `upstream.json`.

The SDK/tool bundle retains Microsoft's VC7 EULA/REDIST files, the Platform SDK
LICENSE/REDIST files and the DirectX SDK EULA. The original font/common sample
source retains its notices. Adecode.lib retains its embedded Interplay notice;
its exact source/release packaging is not claimed.

The provisional formatting scanner beside engine graphics/StringFormat.cpp is
derived from the Microsoft CString source retained in WTL 7.5. Its source donor,
adaptations and limits are recorded in FormatScanner.provenance.json; the
Microsoft notice and CPL license are retained beside it. This is source lineage,
not an established original WTL dependency. No WTL package is installed or linked.

Game data and runtime DLLs are supplied separately and are not part of tracked
source or the SDK/tool archive. No consolidated license grant is inferred for
upstream components; their own notices remain authoritative.

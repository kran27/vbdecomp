# Source control and upstreams

Game source lives in `repos/openvb`; engine source lives in
`repos/openvb/openje`. Both are tracked directly by this repository.

The reconstruction builds on [OpenJE/openvb](https://github.com/OpenJE/openvb)
and [OpenJE/openje](https://github.com/OpenJE/openje). `repos.lock.json` records
upstream revisions. The inherited `.gitmodules` file is provenance only.

Small, including the F3 runtime adaptations, is vendored from
[kran27/small-script](https://github.com/kran27/small-script). zlib 1.1.4 is also
vendored. Each dependency's `upstream.json` records its revision and hashes.

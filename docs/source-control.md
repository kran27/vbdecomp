# Source control and upstreams

The workspace repository owns game and engine source directly under
`repos/openvb` and `repos/openvb/openje`. Do not initialize nested repositories or
apply reconstruction patches. The inherited `.gitmodules` file records upstream
provenance; it is not an active setup step.

`repos.lock.json` records the OpenJE origins and source baselines, plus the user's
publication destinations `kran27/openvb` and `kran27/openje`. Both publication
pushes are on hold until explicitly authorized. The engine fork's merge commit
and our recorded upstream baseline have identical file trees.

Small is vendored as the verbatim Git tree of the public `kran27/small-script`
revision recorded in its `upstream.json`. Its decompiler project and private
history are excluded. The fork owns the F3 runtime and console adaptations; the
workspace compiles that vendored source directly, without a separate overlay,
preparation generator or replacement-source lock. zlib 1.1.4 is similarly pinned
and vendored.

OpenJE's reversal, tooling, IDA, BinSync and format-pattern repositories remain
optional read-only references at their pinned revisions. They do not need forks
and are not build prerequisites. Setup does not fetch them. The existing local
checkouts and their pinned provenance are retained; `scripts/Sync-Repos.ps1` only
checks the canonical source layout.

Historical analysis, receipts, build reports, old source metadata and retired
workflows are preserved locally and excluded from the public file set. Personal
configuration, databases, game files, SDKs, compiler outputs and archives are
ignored. Never remove those local files as part of publication cleanup.

The workspace Git history retains the historical material even after it leaves
the current index. Publication should use the prepared clean public snapshot
branch or source archive, rather than the private workspace history.

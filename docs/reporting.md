# Progress reporting

Report exact matches with their original address, source function, toolchain,
comparison scope and verification date. Count overlapping functions or byte
ranges once, and report third-party library matches separately.

Keep these statuses distinct:

- Exact matches with current inputs.
- Historical matches needing recheck.
- Confirmed mismatches.
- Untested code.

Build and runtime results should state what was built or exercised. Source
presence and successful compilation do not count as exact matches.

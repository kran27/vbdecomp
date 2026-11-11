# F3 Small runtime adaptation

These are the exact adapted `amx.c` and `amx.h` previously produced by the
Small preparation step, checked in as ordinary source. Original copyright
headers and the supplied Small license are retained. The runtime adaptations
and eight user-data slots are documented in `docs/amx-static-fidelity.md`.

`config/small-runtime.lock.json` pins these files. Other Small source still comes
from the supplied read-only checkout. No patch application or runtime/compiler
behavior change is introduced by checking these two files in.

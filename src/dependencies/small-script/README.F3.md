# F3 runtime integration

Small 2.3.1 with F3's eight user-data slots, debugger flags, compact-image
expansion and VM execution behavior, plus VC7-compatible integer declarations.

Compile `src/amx.c`, `src/float.c` and `src/small_console.c` as C with VC7
13.00.9466, using `src/amx.h`. The AMX runtime matches F3's 8,480-byte code span.

The console formatter supports %, c, d, f and s through the consumer-provided
`F3_DebugPutChar` and `F3_DebugPrintf` callbacks.

Original ITB CompuPhase copyright and license notices are retained.

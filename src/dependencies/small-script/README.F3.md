# Van Buren runtime integration

The runtime sources in this fork include the existing F3 adaptations:

- Eight AMX user-data slots.
- The original browse/debug flag transitions and debug-symbol handling.
- The compact-image expansion loop and VM execution/error behavior recovered
  for the F3 runtime, including its original unchecked operations and quirks.
- A selected console formatter in `src/small_console.c` / `.h`, based on the
  original Small console source. It retains %, c, d, f and s and calls the
  consumer-provided F3_DebugPutChar / F3_DebugPrintf callbacks.
- Integer declarations compatible with VC7's C/C++ headers.

The Van Buren workspace consumes `src/amx.c`, `src/float.c` and
`src/small_console.c` directly as C, using this fork's `src/amx.h`. It does not
patch, substitute or generate a different runtime. Its original-toolchain build
uses VC7 13.00.9466; a retained static comparison covers the 8,480-byte F3 AMX
runtime span. That historical result is not a certification of every compiler,
debug-format variation or console integration.

Original ITB CompuPhase copyright and license notices are retained. These files
preserve the existing adaptation rather than introducing new runtime behavior.
The script decompiler is a separate local project and is not included here.

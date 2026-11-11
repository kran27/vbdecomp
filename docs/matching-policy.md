# Exact matching

An exact function match compares VC7 compiler output with every original function
chunk, shared tail and owned exception-handling contribution. Every relocation
must resolve to its original target; unresolved operands leave the match incomplete.

Comparisons use unmodified compiler output and preserve original calling
conventions, layouts, ownership and failure behavior. Third-party library matches
are recorded separately from proprietary game and engine code.

Function-code equality, class ABI, runtime behavior and whole-executable equality
are separate results. A successful build alone establishes compilation and linking.

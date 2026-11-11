# Canonical source style

Use VC7-supported C++98 and the supplied CRT/STL. Preserve original types,
calling conventions, field order, ownership, exceptions and failure behavior.
Do not simplify an original quirk to make the code look more modern.

Follow the surrounding file: tabs, Allman braces and paired .hpp/.cpp files.
Keep established F3/JE/GUI namespaces and class names. Do not introduce a new
namespace scheme, require precompiled headers or reformat unrelated code.

Record known original addresses and unresolved boundaries briefly. Keep detailed
evidence in local function records. Layout assertions express only the offsets
or sizes actually established. No fabricated padding, runtime substitutes or
successful stubs for unrecovered paths.

Tests and analysis fixtures are separate from canonical game/engine source.
Modern compiler and library support is outside the current baseline.

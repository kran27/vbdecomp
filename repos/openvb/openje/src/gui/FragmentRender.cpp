// Keep the real non-inline getter visible to Draw in one compilation unit.
// The evidenced VC7 /Ob1 policy permits register analysis without inlining it.
#include "FragmentCore.cpp"
#include "FragmentDraw.cpp"

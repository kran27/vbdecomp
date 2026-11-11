// One original-style owner for the private PipBoy selectors and page/input code.
// Keeping these definitions together preserves their private compiler ABI and
// lets VC7 see their real exception/lifetime behaviour. Each fragment is included
// once here and must not also be compiled as an independent translation unit.
#include <winsock2.h>
#include "PipBoyNames.hpp"
#include "PipBoyTaskLabels.cpp"
#include "PipBoyNoteEditing.cpp"
#include "PipBoyInterface.cpp"
#include "PipBoyInput.cpp"

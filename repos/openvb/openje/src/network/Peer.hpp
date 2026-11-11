#pragma once

// Compatibility spelling for the original net::base interface. There is no
// additional Peer subobject or RTTI ancestor in the original hierarchy.
#include "Base.hpp"
namespace JE { namespace Net { typedef Base Peer; } }

#pragma once

#include <string>

//Original Gfx_Engine5C0C00, interface+1B0. The actual field is engine+0xF8
//(248 decimal), used by the statistics overlay's "Map: %s" line. This helper
//takes a normal owning string; it does not allocate or model a partial Engine.
void Gfx_SetEngineString248(std::string& target,const char* value);

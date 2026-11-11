#pragma once

#include "GameScriptNatives.hpp"

namespace F3
{
	BYTE CopyScriptFloat(AMX* vm, float& output, float value); //59AA60, actual scalar copy.
}

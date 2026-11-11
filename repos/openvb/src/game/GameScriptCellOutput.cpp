#include "GameScriptMapContext.hpp"
#include "GameScriptScalarContext.hpp"
#include <cstring>

namespace F3
{
	BYTE ReadScriptCells(AMX* vm, cell address, void* values, DWORD count)
	{
		cell* source;
		if (amx_GetAddr(vm, address, &source)) return 0;
		std::memcpy(values, source, count * sizeof(cell));
		return 1;
	}
	BYTE WriteScriptCells(AMX* vm, cell address, const void* values, DWORD count)
	{
		cell* destination;
		if (amx_GetAddr(vm, address, &destination)) return 0;
		std::memcpy(destination, values, count * sizeof(cell));
		return 1;
	}
	BYTE CopyScriptFloat(AMX*, float& output, float value)
	{
		output = value;
		return 1;
	}

}

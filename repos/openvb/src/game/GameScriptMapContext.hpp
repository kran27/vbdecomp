#pragma once
#include "GameScriptNatives.hpp"
#include <d3dx8math.h>

namespace F3
{
	BYTE ReadScriptCells(AMX* vm, cell address, void* values, DWORD count); //59ABC0.
	BYTE WriteScriptCells(AMX* vm, cell address, const void* values, DWORD count); //59AC00.
	BYTE WriteScriptVector(AMX* vm, cell address, const D3DXVECTOR3& value); //59AC90.
}

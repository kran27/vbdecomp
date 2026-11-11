#pragma once

#include "GameScriptNatives.hpp"

namespace F3
{
	class GameEntity;
	BYTE ResolveGameEntity(AMX* vm, DWORD id, GameEntity** output); //45B9F0.
}

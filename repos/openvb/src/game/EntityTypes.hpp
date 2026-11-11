#pragma once

#include <windows.h>

namespace JE { class Entity; }

namespace F3
{
	JE::Entity* CreateGameEntity(DWORD type); // 0x46F530, factored constructor cases; GameWorld owns the base fallback.
	DWORD GetEntityTypeFromFilename(const char* name, BYTE alternateCreature); // 0x46FB20.
}

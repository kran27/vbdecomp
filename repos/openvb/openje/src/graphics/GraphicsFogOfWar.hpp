#pragma once

#include <windows.h>

namespace JE
{
	class GraphicsLevelResources;
	BYTE SetGraphicsFogGray(DWORD& color, float intensity); //5BEB40/5BEBB0 identical conversion policy.
	void ProcessGraphicsFogOfWar(GraphicsLevelResources& level, const DWORD& exploredColor,
		const DWORD& visibleColor, const BYTE& revealExplored); //5BE380.
}

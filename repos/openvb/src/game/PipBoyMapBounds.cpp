#include "game/PipBoyInterface.hpp"
#include "game/AreaMapWindow.hpp"

namespace F3
{
	const float g_PipBoyMapZooms[5] = {.25f,.5f,1.0f,2.0f,4.0f}; //65BB5C.

	JE::AreaMap* PipBoyInterface::SelectedAreaMap() //44AB40.
	{
		if (valueA8 < 0 || valueA8 >= LONG(areaMaps.size())) return NULL;
		return areaMaps[valueA8];
	}

	BYTE PipBoyInterface::SelectedExploredBounds(RECT& output) //44A470.
	{
		if (!secondaryMap) return 0;
		const float zoom = g_PipBoyMapZooms[mapZoomIndex];
		return secondaryMap->GetExploredBounds(output, zoom);
	}
}

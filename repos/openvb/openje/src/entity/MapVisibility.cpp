#include <winsock2.h>
#include "Map.hpp"
#include "MapFog.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Graphics.hpp"

namespace JE
{
	void Map::QueueVisibleCell(DWORD entityID, LONG first, LONG row, BYTE value)
	{
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74->QueueVisibleChange(
			entityID, idB4, first, row, value);
	}
	BYTE Map::SetExploredCell(LONG first, LONG row, BYTE value)
	{
		bool changed = true;
		if (areaMap)
			changed = (value ? 1 : 0) != (areaMap->IsExplored(first,row) ? 1 : 0);
		if (changed)
		{
			static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74->QueueExploredChange(
				idB4, first, row, value);
			if (areaMap) return areaMap->SetExplored(first,row,value);
			return 1;
		}
		return 0;
	}
}

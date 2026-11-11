#include "GameScriptMapContext.hpp"
#include "entity/Entity.hpp"
#include "entity/Trigger.hpp"
#include <cstring>

namespace F3
{
	DWORD ResolveScriptMapID(DWORD ownerID)
	{
		DWORD result = 0;
		switch (ownerID >> 29)
		{
		case 0:
			{
				JE::Entity* entity = JE::GetEntityPtr(ownerID);
				if (entity) result = entity->mapHandle;
				break;
			}
		case 1: result = ownerID; break;
		case 2:
			{
				JE::Trigger* trigger = JE::FindTrigger(ownerID);
				if (trigger) result = trigger->mapID;
				break;
			}
		}
		return result;
	}
	BYTE WriteScriptVector(AMX* vm, cell address, const D3DXVECTOR3& value)
	{
		float values[3];
		values[0] = value.x;
		values[1] = value.y;
		values[2] = value.z;
		return WriteScriptCells(vm, address, values, 3);
	}
}

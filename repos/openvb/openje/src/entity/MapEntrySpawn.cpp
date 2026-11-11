#include "Map.hpp"
#include "PathManager.hpp"
#include "World.hpp"
#include <stdexcept>

namespace JE
{
	void Map::GetEntryPosition(BYTE entryID, LONG slot, D3DXVECTOR3& position,
		D3DXVECTOR3& rotation)
	{
		//56F120 clamps the party slot independently of the entry ID lookup.
		if (slot<0) slot=0;
		else if (slot>5) slot=5;
		std::map<BYTE,MapEntryPoint*>::iterator entry=entryPoints.find(entryID);
		if (entry==entryPoints.end()) entry=entryPoints.begin();
		if (entry==entryPoints.end())
		{
			PathPoint center={0,0};
			center.x=terrainGrid->width/2;
			center.y=terrainGrid->height/2;
			//56F1AB retrieves the real World path owner through IWorld+64;
			//the borrowed result is unused by the following560710 conversion.
			IWorld* world=g_World;
			(void)world->GetPathManager();
			D3DXVECTOR3 converted;
			position=*PathCellToWorld(&converted,center,this);
			rotation.x=rotation.y=rotation.z=0.0f;
			return;
		}
		const VFX_Rotation& entryPosition=entry->second->positions[slot];
		position.x=entryPosition.values[0];
		position.y=entryPosition.values[1];
		position.z=entryPosition.values[2];
		//Original sets only the first rotation component. The caller556270
		//initializes all three components before this method; do not overwrite
		//the other two when a caller supplies different existing values.
		rotation.x=entry->second->values[slot]*57.2957763671875f; //647920 bits42652EE0.
	}
}

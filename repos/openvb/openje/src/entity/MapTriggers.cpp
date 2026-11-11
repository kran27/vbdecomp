#include "Map.hpp"
#include "Trigger.hpp"
#include "EffectTreeTriggers.hpp"
#include "World.hpp"
#include "MapFog.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>
#include <stdexcept>

namespace JE
{
	void Map::NotifyEntityMoved(Entity* entity) //56C500.
	{
		Invalidate(entity);
	}

	DWORD World::AllocateTriggerID()
	{ return (++value88 & 0x1FFFFFFF) | 0x40000000; }
	Trigger* Map::CreateTrigger(DWORD type)
	{
		Trigger* trigger=NULL;
		switch (type)
		{
		case 0: trigger=new TransitionTrigger; break;
		case 1: trigger=new ScriptTrigger; break;
		case 2: trigger=new EffectTreeTrigger; break;
		case 3: trigger=new EffectTreeCircleTrigger; break;
		case 4: trigger=new EffectTreeSemiCircleTrigger; break;
		case 5: trigger=new EffectTreeRingTrigger; break;
		case 6: trigger=new BuildingTrigger; break;
		default: LogDebugString("Error creating trigger. Unknown type %d.\n",type); return NULL;
		}
		IWorldClient* worldClient = g_World;
		IWorld* world = worldClient;
		trigger->id=world->AllocateTriggerID();
		trigger->mapID=idB4;
		return trigger;
	}
	BYTE Map::ReadTrigger(Common::Chunk&,Resources::ResourceMemoryStream* stream)
	{
		DWORD type;
		if (!Resources::g_pResourceSystem->ReadMemory(&type,4,stream)) return 0;
		Trigger* trigger=CreateTrigger(type);
		DWORD count;
		if (!Resources::g_pResourceSystem->ReadMemory(&count,4,stream)) { delete trigger; return 0; }
		D3DXVECTOR3 point(0,0,0);
		for (DWORD index=0; index<count; ++index)
		{
			if (!Resources::g_pResourceSystem->ReadMemory(&point.x,4,stream) || !Resources::g_pResourceSystem->ReadMemory(&point.y,4,stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&point.z,4,stream)) { delete trigger; return 0; }
			trigger->polygon.points.push_back(point);
		}
		// Read consumes the following derived chunk. The outer dispatcher still
		// seeks to the EMTR end afterwards, as the original reader does.
		if (!trigger->Read(stream)) { delete trigger; return 0; }
		RegisterTrigger(trigger);
		return 1;
	}
	void Map::RegisterTrigger(Trigger* trigger) { triggers.push_back(trigger); }
	void Map::UnregisterTrigger(Trigger* trigger) { triggers.remove(trigger); }
	void Map::ClearTriggers()
	{
		for (std::list<Trigger*>::iterator i=triggers.begin(); i!=triggers.end();)
		{
			Trigger* trigger=*i;
			i=triggers.erase(i);
			delete trigger;
		}
	}
	void Map::UpdateTriggers(DWORD time)
	{
		for (std::list<Trigger*>::iterator i=triggers.begin(); i!=triggers.end();)
		{
			Trigger* trigger=*i;
			trigger->Update(time);
			if (trigger->IsDestroyed()) { i=triggers.erase(i); delete trigger; }
			else ++i;
		}
	}
	void Map::HideMaterial(Entity* owner,DWORD material)
	{
		std::set<DWORD>& owners = hiddenMaterialOwners[static_cast<int>(material)];
		DWORD ownerID = owner ? owner->entityID : 0;
		owners.insert(ownerID);
		if (areaMap) { areaMap->changes|=4; areaMap->SetLayerVisible(static_cast<LONG>(material),0); }
	}
	void Map::ShowMaterial(Entity* owner,DWORD material)
	{
		std::map<int,std::set<DWORD> >::iterator i=hiddenMaterialOwners.find(static_cast<int>(material));
		if (i!=hiddenMaterialOwners.end())
		{
			std::set<DWORD>& owners = i->second;
			DWORD ownerID = owner ? owner->entityID : 0;
			std::set<DWORD>::iterator entry=owners.find(ownerID);
			if (entry!=i->second.end())
			{
				i->second.erase(entry);
				if (i->second.empty())
				{
					hiddenMaterialOwners.erase(i);
					if (areaMap) areaMap->SetLayerVisible(static_cast<LONG>(material),1);
				}
			}
		}
		if (areaMap) areaMap->changes|=4;
	}
	BYTE AreaMap::SetLayerVisible(LONG layer,BYTE visible)
	{
		std::map<LONG,AreaMapLayer>::iterator i=layers.find(layer);
		if (i==layers.end()) return 0;
		i->second.disabled=visible==0;
		heightsDirty=1;
		return 1;
	}
}

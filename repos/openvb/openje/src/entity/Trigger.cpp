#include <winsock2.h>
#include "startup/NetworkComponent.hpp"
#include "Trigger.hpp"
#include "resources/ResourceSystem.hpp"
#include "EffectTreeTriggers.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "Map.hpp"
#include "World.hpp"
#include "WorldCollections.hpp"
#include "network/EngineServer.hpp"
#include "model/ModelInstance.hpp"
#include "scripting/ScriptSmall.hpp"
#include <list>
#include <cstddef>
#include <stdexcept>

namespace JE
{
	namespace
	{
		std::list<Trigger*> allTriggers; //Original70C7D8, borrowed entries.
		DWORD FourCC(const char* s)
		{ return BYTE(s[0]) | (DWORD(BYTE(s[1]))<<8) | (DWORD(BYTE(s[2]))<<16) | (DWORD(BYTE(s[3]))<<24); }
	}
	BYTE Common::Polygon::Contains(const D3DXVECTOR3& position) const
	{
		LONG crossings = 0;
		D3DXVECTOR3 current(0,0,0);
		const LONG count = LONG(points.size());
		const D3DXVECTOR3* vertices = &points[0];
		D3DXVECTOR3 previous = *vertices; //The original reads even when count is zero.
		for (LONG index = 1; index <= count; ++index)
		{
			current = vertices[index % count];
			const float& low = current.z < previous.z ? current.z : previous.z;
			if (position.z > low)
			{
				const float& high = previous.z < current.z ? current.z : previous.z;
				if (position.z <= high)
				{
					const float& right = previous.x < current.x ? current.x : previous.x;
					if (position.x <= right && previous.z != current.z
						&& (previous.x == current.x || (double(position.z)-previous.z)*(double(current.x)-previous.x)
							/(double(current.z)-previous.z)+previous.x >= position.x)) ++crossings;
				}
			}
			previous = current;
		}
		return crossings % 2 != 0;
	}
	Trigger::Trigger() : destroyed(0) { allTriggers.push_back(this); }
	Trigger::~Trigger()
	{ allTriggers.remove(this); }
	Trigger* FindTrigger(DWORD id)
	{
		for (std::list<Trigger*>::iterator i=allTriggers.begin(); i!=allTriggers.end(); ++i)
		{
			Trigger* trigger = *i;
			if (trigger->id==id) return trigger;
		}
		return NULL;
	}
	BYTE Trigger::IsDestroyed() const { return destroyed; }
	BYTE Trigger::Contains(const D3DXVECTOR3& position) const { return polygon.Contains(position); }
	void Trigger::TrackEntity(Entity* entity)
	{
		DWORD id = entity->entityID;
		entities.insert(id);
	}
	void Trigger::UntrackEntity(Entity* entity)
	{
		DWORD id = entity->entityID;
		entities.erase(id);
	}
	void Trigger::AddEntity(Entity* entity)
	{
		if (Contains(entity->position))
		{
			DWORD id=entity->entityID;
			std::set<DWORD>::iterator found=entities.find(id);
			if (found==entities.end()) { TrackEntity(entity); OnEnter(entity); }
		}
	}
	void Trigger::RemoveEntity(Entity* entity)
	{
		DWORD id=entity->entityID;
		std::set<DWORD>::iterator found=entities.find(id);
		if (found!=entities.end()) { OnLeave(entity); UntrackEntity(entity); }
	}
	void Trigger::MoveEntity(Entity* entity)
	{
		DWORD id=entity->entityID;
		std::set<DWORD>::iterator found=entities.find(id);
		if (found!=entities.end())
		{
			if (!Contains(entity->position)) { OnLeave(entity); UntrackEntity(entity); }
		}
		else if (Contains(entity->position)) { TrackEntity(entity); OnEnter(entity); }
	}
	void Trigger::Update(DWORD time) { OnUpdate(time); }

	BuildingTrigger::~BuildingTrigger() {}
	BuildingTrigger::BuildingTrigger() : building(0), red(0xC0), green(0xC0), blue(0xC0) {}
	BYTE BuildingTrigger::Read(Resources::ResourceMemoryStream* stream)
	{
		Common::Chunk chunk; chunk.Read(stream);
		if (chunk.type!=FourCC("EBTR")) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&building,4,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&red,1,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&green,1,stream)) return 0;
		return Resources::g_pResourceSystem->ReadMemory(&blue,1,stream)!=0;
	}
	void BuildingTrigger::OnUpdate(DWORD) { F3::g_pNetworkInstance->GetMode(); }
	void BuildingTrigger::OnEnter(Entity* entity)
	{
		DWORD buildingID = building;
		entity->spatialLinks.insert(buildingID);
		ModelInstance* model = entity->model;
		if (model)
		{
			model->SetAmbientColor((DWORD(red)<<16)|(DWORD(green)<<8)|blue);
			model->SetAmbientOverride(1);
		}
		IWorld* world = g_World;
		Party* party = world->GetParty();
		if (party->Contains(entity) || entity->visibilityOwnerFlag)
		{
			world = g_World;
			Map* map = world->GetMap(entity->mapHandle);
			if (map) map->HideMaterial(entity,building);
		}
	}
	void BuildingTrigger::OnLeave(Entity* entity)
	{
		DWORD buildingID = building;
		entity->spatialLinks.erase(buildingID);
		ModelInstance* model = entity->model;
		if (model) model->SetAmbientOverride(0);
		IWorld* world = g_World;
		Party* party = world->GetParty();
		if (party->Contains(entity) || entity->visibilityOwnerFlag)
		{
			world = g_World;
			Map* map = world->GetMap(entity->mapHandle);
			if (map) map->ShowMaterial(entity,building);
		}
	}

	ScriptTrigger::ScriptTrigger() : script(NULL) {}
	ScriptTrigger::~ScriptTrigger() { delete script; }
	BYTE ScriptTrigger::Read(Resources::ResourceMemoryStream* stream)
	{
		Common::Chunk chunk; chunk.Read(stream);
		DWORD count;
		if (chunk.type!=FourCC("ESTR") || !Common::ReadString(scriptName,stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&count,4,stream)) return 0;
		std::map<std::string,cell> variables;
		for (DWORD index=0; index<count; ++index)
		{
			std::string name; cell value;
			if (!Common::ReadString(name,stream)
				|| !Resources::g_pResourceSystem->ReadMemory(&value,4,stream)) return 0;
			variables[name]=value;
		}
		if (!scriptName.empty())
		{
			Script_Small* created = GetWorldServerInstance()->NewScript(0);
			script = created;
			if (!created->Load(scriptName.c_str(),1)) { delete script; script=NULL; return 0; }
			if (!variables.empty()) script->PreLoad(variables);
		}
		return 1;
	}
	void ScriptTrigger::OnEnter(Entity* entity)
	{
		if (F3::g_pNetworkInstance->GetMode()&1)
		{
			cell result;
			cell args[2];
			args[0]=cell(entity->entityID);
			IWorld* world=g_World;
			args[1]=cell(world->GetServerTime());
			RunScript("OnEnter",&result,2,args);
		}
	}
	void ScriptTrigger::OnLeave(Entity* entity)
	{
		if (F3::g_pNetworkInstance->GetMode()&1)
		{
			cell result;
			cell args[2];
			args[0]=cell(entity->entityID);
			IWorld* world=g_World;
			args[1]=cell(world->GetServerTime());
			RunScript("OnLeave",&result,2,args);
		}
	}
	void ScriptTrigger::OnUpdate(DWORD time)
	{
		if (!(F3::g_pNetworkInstance->GetMode()&1))
		{
			IWorld* world=g_World;
			DWORD cutscene=world->GetCutsceneState();
			cell result=0;
			if (static_cast<Server*>(F3::g_pNetworkInstance->server)->field0x70E4)
			{
				if (cutscene)
				{
					cell args[2]; args[0]=cell(time);
					world=g_World;
					args[1]=cell(world->GetCutsceneID());
					switch(cutscene)
					{
					case 1: RunScript("OnCutsceneStart",&result,2,args); break;
					case 2: RunScript("OnCutscene",&result,2,args); break;
					case 3: RunScript("OnCutsceneEnd",&result,2,args); break;
					}
				}
				else
				{
					cell argument=cell(time);
					RunScript("OnUpdate",&result,1,&argument);
				}
			}
		}
	}

	TransitionTrigger::~TransitionTrigger() {}
	TransitionTrigger::TransitionTrigger() : entryPoint(0), flag4D(0) {}
	BYTE TransitionTrigger::Read(Resources::ResourceMemoryStream* stream)
	{
		Common::Chunk chunk; chunk.Read(stream);
		if (chunk.type!=FourCC("ETTR")) return 0;
		DWORD version=chunk.version;
		if (!Common::ReadString(mapName,stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&entryPoint,1,stream)) return 0;
		if (version>=1)
		{ BYTE flag; if (!Resources::g_pResourceSystem->ReadMemory(&flag,1,stream)) return 0; flag4D=flag!=0; }
		return 1;
	}
	void TransitionTrigger::OnEnter(Entity*) {}
	void TransitionTrigger::OnLeave(Entity*) {}
	void TransitionTrigger::OnUpdate(DWORD)
	{
		if (entities.empty()) return;
		std::list<Entity*> party;
		for (std::set<DWORD>::iterator i=entities.begin(); i!=entities.end(); ++i)
		{
			Entity* entity=GetEntityPtr(*i);
			IWorld* world=g_World;
			Party* worldParty=world->GetParty();
			//5B1AF5 inserts before the temporary list's first node, reversing ID order.
			if (worldParty->Contains(entity)) party.push_front(entity);
		}
		if (party.empty()) return;
		LONG formationIndex=0;
		for (std::list<Entity*>::iterator i=party.begin(); i!=party.end(); ++i)
		{
			Entity* entity=*i;
			RemoveEntity(entity);
			entity->ClearActions();
			IWorld* world=g_World;
			entity->QueueAction(world->NewAction(1));
			static_cast<Server*>(F3::g_pNetworkInstance->server)->MoveEntityToNamedMapEntry(entity,mapName.c_str(),entryPoint,5-formationIndex,0);
			formationIndex=(formationIndex+1)%6;
		}
	}
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	namespace
	{
		//Original embedded polygon/Trigger offsets and seven factory allocation extents.
		typedef char PolygonSize[(sizeof(Common::Polygon) == 0x14) ? 1 : -1];
		typedef char PolygonPoints[(offsetof(Common::Polygon, points) == 4) ? 1 : -1];
		typedef char TriggerSize[(sizeof(Trigger) == 0x30) ? 1 : -1];
		typedef char TriggerPolygon[(offsetof(Trigger, polygon) == 4) ? 1 : -1];
		typedef char TriggerEntities[(offsetof(Trigger, entities) == 0x18) ? 1 : -1];
		typedef char TriggerDestroyed[(offsetof(Trigger, destroyed) == 0x24) ? 1 : -1];
		typedef char TriggerID[(offsetof(Trigger, id) == 0x28) ? 1 : -1];
		typedef char TriggerMap[(offsetof(Trigger, mapID) == 0x2C) ? 1 : -1];
		typedef char TransitionSize[(sizeof(TransitionTrigger) == 0x50) ? 1 : -1];
		typedef char TransitionName[(offsetof(TransitionTrigger, mapName) == 0x30) ? 1 : -1];
		typedef char TransitionEntry[(offsetof(TransitionTrigger, entryPoint) == 0x4C) ? 1 : -1];
		typedef char TransitionFlag[(offsetof(TransitionTrigger, flag4D) == 0x4D) ? 1 : -1];
		typedef char ScriptSize[(sizeof(ScriptTrigger) == 0x50) ? 1 : -1];
		typedef char ScriptName[(offsetof(ScriptTrigger, scriptName) == 0x30) ? 1 : -1];
		typedef char ScriptOwner[(offsetof(ScriptTrigger, script) == 0x4C) ? 1 : -1];
		typedef char BuildingSize[(sizeof(BuildingTrigger) == 0x38) ? 1 : -1];
		typedef char BuildingID[(offsetof(BuildingTrigger, building) == 0x30) ? 1 : -1];
		typedef char BuildingColor[(offsetof(BuildingTrigger, red) == 0x34) ? 1 : -1];
		typedef char EffectTreeSize[(sizeof(EffectTreeTrigger) == 0x88) ? 1 : -1];
		typedef char EffectTreePosition[(offsetof(EffectTreeTrigger, position) == 0x30) ? 1 : -1];
		typedef char EffectTreePolygon[(offsetof(EffectTreeTrigger, localPolygon) == 0x3C) ? 1 : -1];
		typedef char EffectTreeCreator[(offsetof(EffectTreeTrigger, creatorID) == 0x50) ? 1 : -1];
		typedef char EffectTreeAttachment[(offsetof(EffectTreeTrigger, attachmentOwnerID) == 0x54) ? 1 : -1];
		typedef char EffectTreeEntity[(offsetof(EffectTreeTrigger, attachedEntity) == 0x58) ? 1 : -1];
		typedef char EffectTreeTemplates[(offsetof(EffectTreeTrigger, templates) == 0x5C) ? 1 : -1];
		typedef char EffectTreeApplied[(offsetof(EffectTreeTrigger, appliedEffects) == 0x68) ? 1 : -1];
		typedef char EffectTreeReevaluate[(offsetof(EffectTreeTrigger, reevaluate) == 0x74) ? 1 : -1];
		typedef char EffectTreeShape[(offsetof(EffectTreeTrigger, shape) == 0x78) ? 1 : -1];
		typedef char EffectTreeForce[(offsetof(EffectTreeTrigger, forceShapeMembership) == 0x7C) ? 1 : -1];
		typedef char EffectTreeExpiration[(offsetof(EffectTreeTrigger, expirationTime) == 0x80) ? 1 : -1];
		typedef char EffectTreeLimit[(offsetof(EffectTreeTrigger, targetLimit) == 0x84) ? 1 : -1];
		typedef char CircleSize[(sizeof(EffectTreeCircleTrigger) == 0x8C) ? 1 : -1];
		typedef char CircleRadius[(offsetof(EffectTreeCircleTrigger, radius) == 0x88) ? 1 : -1];
		typedef char SemiCircleSize[(sizeof(EffectTreeSemiCircleTrigger) == 0x90) ? 1 : -1];
		typedef char SemiCircleRadius[(offsetof(EffectTreeSemiCircleTrigger, radius) == 0x88) ? 1 : -1];
		typedef char SemiCircleAngle[(offsetof(EffectTreeSemiCircleTrigger, angle) == 0x8C) ? 1 : -1];
		typedef char RingSize[(sizeof(EffectTreeRingTrigger) == 0x90) ? 1 : -1];
		typedef char RingInner[(offsetof(EffectTreeRingTrigger, innerRadius) == 0x88) ? 1 : -1];
		typedef char RingOuter[(offsetof(EffectTreeRingTrigger, outerRadius) == 0x8C) ? 1 : -1];
	}
#endif
}

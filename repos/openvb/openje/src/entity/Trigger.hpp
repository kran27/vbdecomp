#pragma once

#include "resources/ResourceMemoryStream.hpp"
#include "WorldCollections.hpp"
#include "amx.h"
#include <d3dx8.h>
#include <vector>
#include <set>
#include <string>

namespace JE
{
	class Entity;
	struct Script_Small;
	class Trigger
	{
	public:
		Trigger(); //5B18A0, registers in the global borrowed list.
		virtual ~Trigger(); //5B17E0.
		virtual BYTE IsDestroyed() const; //56E3F0, raw BYTE.
		virtual BYTE Read(Resources::ResourceMemoryStream* stream) = 0;
		virtual void OnEnter(Entity* entity) = 0;
		virtual void OnLeave(Entity* entity) = 0;
		virtual void OnUpdate(DWORD time) = 0;
		virtual BYTE Contains(const D3DXVECTOR3& position) const; //5B1520.
		virtual void TrackEntity(Entity* entity); //5B1750.
		virtual void UntrackEntity(Entity* entity); //5B1880.
		void AddEntity(Entity* entity); //5B1620.
		void RemoveEntity(Entity* entity); //5B1680.
		void MoveEntity(Entity* entity); //5B16D0.
		void Update(DWORD time); //5B1510.
		Common::Polygon polygon; //+4, actual polymorphic owner.
		std::set<DWORD> entities;
		BYTE destroyed;
		DWORD id, mapID; //Unwritten by construction; assigned by Map::CreateTrigger.
	};
	Trigger* FindTrigger(DWORD id); //5B1570.

	class BuildingTrigger : public Trigger
	{
	public:
		BuildingTrigger(); //5B1C30.
		virtual ~BuildingTrigger(); //5B1C60; actual derived destruction phase.
		virtual BYTE Read(Resources::ResourceMemoryStream* stream); //5B1C70.
		virtual void OnEnter(Entity* entity); //5B1DE0.
		virtual void OnLeave(Entity* entity); //5B1F00.
		virtual void OnUpdate(DWORD time); //5B1D40.
		DWORD building;
		BYTE red, green, blue;
	};
	class ScriptTrigger : public Trigger
	{
	public:
		ScriptTrigger(); //5B2600.
		virtual ~ScriptTrigger(); //5B2630.
		virtual BYTE Read(Resources::ResourceMemoryStream* stream); //5B26D0.
		virtual void OnEnter(Entity* entity); //5B23F0.
		virtual void OnLeave(Entity* entity); //5B2460.
		virtual void OnUpdate(DWORD time); //5B24D0.
		BYTE RunScript(const char* name, cell* result, int count, cell* arguments); //5B23D0.
		std::string scriptName;
		Script_Small* script;
	};
	class TransitionTrigger : public Trigger
	{
	public:
		TransitionTrigger(); //56E3C0.
		virtual ~TransitionTrigger(); //56E420; map-name ownership after derived table publication.
		virtual BYTE Read(Resources::ResourceMemoryStream* stream); //5B1980.
		virtual void OnEnter(Entity* entity); //Shared5A9320 RET4.
		virtual void OnLeave(Entity* entity); //Shared5A9320 RET4.
		virtual void OnUpdate(DWORD time); //5B1A60, tracked-party snapshot and actual server map transfer.
		std::string mapName;
		BYTE entryPoint, flag4D;
	};
}

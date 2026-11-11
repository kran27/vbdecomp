#pragma once

#include <windows.h>
#include "configuration/ConfigurationTypes.hpp"
#include "ItemRegistry.hpp"
#include <map>
#include <string>

class OCTREE;
struct Journal;
struct Encyclopedia;

namespace JE
{
	class Entity; class Action; class Effect; struct Script_Small; class Map;
	class Gregorian; class Combat; class PathManager; class Party; typedef ::Journal Journal;
	typedef ::Encyclopedia Encyclopedia; class StoreManager; class WorldMap; class Cutscene;
	struct WorldParameters; struct TeamSystem; struct SquadSystem;
	namespace Net { class Packet; }
	// Original has no virtual destructor in this interface.
	struct __declspec(novtable) IWorld
	{
		virtual DWORD GetTime() const = 0; //0x0, 0x548580.
		virtual void SetTime(DWORD value) = 0; //0x4, 0x548590.
		virtual DWORD GetServerTime() const = 0; //0x8, 0x5485a0.
		virtual void SetServerTime(DWORD value) = 0; //0xc, 0x5485b0.
		virtual Gregorian* NewCalendar() = 0; //0x10, 0x5485c0.
		virtual Action* NewAction(DWORD type) = 0; //0x14, 0x5485d0.
		virtual Entity* NewEntity(DWORD type) = 0; //0x18, 0x5485e0.
		virtual Map* NewMap() = 0; //0x1c, 0x5485f0.
		virtual DWORD AllocateEffectID() = 0; //0x20, 0x548600.
		virtual DWORD AllocateEntityID() = 0; //0x24, 0x548610.
		virtual void SetEntityIDCounter(DWORD value) = 0; //0x28, 0x548620.
		virtual void ResetEntityIDCounter() = 0; //0x2c, 0x548630.
		virtual DWORD ClassifyEntity(const std::string& name, BYTE alternate) = 0; //0x34, 0x5487e0.
		virtual DWORD ClassifyEntity(const char* name, BYTE alternate) = 0; //0x30, 0x548640.
		virtual DWORD AllocateMapID() = 0; //0x38, 0x548650.
		virtual void SetMapIDCounter(DWORD value) = 0; //0x3c, 0x548660.
		virtual void ResetMapIDCounter() = 0; //0x40, 0x548670.
		virtual DWORD AllocateTriggerID() = 0; //0x44, 0x548680.
		virtual void SetTriggerIDCounter(DWORD value) = 0; //0x48, 0x548690.
		virtual void ResetTriggerIDCounter() = 0; //0x4c, 0x5486a0.
		virtual void Reset() = 0; //0x50, 0x548810.
		virtual Map* GetMap(DWORD id) = 0; //0x54, 0x5487f0.
		virtual Map* LoadMap(const char* name) = 0; //0x58, 0x548830.
		virtual DWORD GetCurrentMapID() = 0; //0x5c, 0x5486b0.
		virtual Map* GetCurrentMap() = 0; //0x60, 0x548800.
		virtual PathManager* GetPathManager() = 0; //0x64, 0x5486c0.
		virtual Party* GetParty() = 0; //0x68, 0x5486d0.
		virtual WorldParameters* GetGlobalParameters() = 0; //0x6c, 0x5486e0.
		virtual WorldParameters* GetLocalParameters() = 0; //0x70, 0x5486f0.
		virtual Gregorian* GetCalendar() = 0; //0x74, 0x548700.
		virtual Journal* GetJournal() = 0; //0x78, 0x548710.
		virtual Encyclopedia* GetEncyclopedia() = 0; //0x7c, 0x548720.
		virtual StoreManager* GetStores() = 0; //0x80, 0x548730.
		virtual TeamSystem* GetTeams() = 0; //0x84, 0x548740.
		virtual SquadSystem* GetSquads() = 0; //0x88, 0x548750.
		virtual WorldMap* GetWorldMap(const char* name) = 0; //0x8c, 0x548820.
		virtual WorldMap* GetCurrentWorldMap() = 0; //0x90, 0x548760.
		virtual void SetCurrentWorldMap(const char* name) = 0; //0x94, 0x548770.
		virtual DWORD GetCutsceneState() const = 0; //0x98, 0x548780.
		virtual DWORD GetCutsceneID() const = 0; //0x9c, 0x548790.
		virtual DWORD StartCutscene(DWORD id, DWORD mapID) = 0; //0xa0, 0x5487a0.
		virtual DWORD StopCutscene() = 0; //0xa4, 0x5487b0.
		virtual Cutscene* GetCutscene() = 0; //0xa8, 0x5487c0.
		virtual ItemRegistry* GetItemRegistry() = 0; //0xac, 0x5487d0.
	};
	// Original has no virtual destructor in this interface.
	struct __declspec(novtable) IWorldServer : public virtual IWorld
	{
		virtual DWORD GetSecondsPerRound() = 0; //0x0, 0x436240.
		virtual float GetTimeScale() = 0; //0x4, 0x46f250.
		virtual void SetTimeScale(float value) = 0; //0x8, 0x46f260.
		virtual DWORD GetUpdateInterval() = 0; //0xc, 0x46f270.
		virtual void SetUpdateInterval(DWORD value) = 0; //0x10, 0x544a60.
		virtual BYTE IsServerFrameDue(DWORD time) const = 0; //0x14, 0x544a70, AL result.
		virtual void UpdateServer(DWORD frameClock) = 0; //0x18, 0x544a90.
		virtual Combat* NewCombat() = 0; //0x1c, 0x545100.
		virtual Combat* GetCombat() = 0; //0x20, 0x46f280.
		virtual Script_Small* NewScript(DWORD ownerID) = 0; //0x24, 0x544b80.
		virtual Effect* NewEffect(DWORD type) = 0; //0x28, 0x544bf0.
		virtual void PackState(Net::Packet& packet) = 0; //0x2c, 0x545930.
	};
	// Original has no virtual destructor in this interface.
	struct __declspec(novtable) IWorldClient : public virtual IWorld
	{
		virtual void UpdateClientFrame(DWORD frameClock) = 0; //0x0, 0x544b30.
		virtual void SetActiveMap(DWORD id) = 0; //0x8, 0x545820.
		virtual void SetActiveMap(const std::string& name) = 0; //0x4, 0x546140.
		virtual OCTREE* GetCurrentMapOctree() = 0; //0xc, 0x544cf0.
		virtual void UnpackState(Net::Packet& packet) = 0; //0x10, 0x547e60.
		virtual void CollectVisibleEffects() = 0; //0x14, 0x548840.
		virtual void DrawVisible() = 0; //0x18, 0x545aa0.
		virtual void PrepareLightIndices() = 0; //0x1c, 0x544d10.
	};
}

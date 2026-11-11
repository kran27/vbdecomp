#pragma once

#include "Entity.hpp"
#include "network/Types.hpp"
#include "IComponent.hpp"
#include "IWorld.hpp"
#include "WorldGroups.hpp"
#include "Gregorian.hpp"
#include "configuration/ConfigurationTypes.hpp"
#include <vector>

class OCTREE;
class Octree2Gfx;
class Gfx_Light;

struct Journal;
struct Encyclopedia;

namespace JE
{
	struct WorldParameters
	{
		WorldParameters(); //5AA3D0, actual out-of-line string/settings lifetime.
		~WorldParameters(); //5AA340, settings then string; supplied library owns both.
		void Load(std::string directory, std::string filename); // 0x5A9AB0.
		BYTE HasValue(std::string variable, std::string section); //5A9D50, by-value strings.
		LONG GetInteger(std::string variable, std::string section = std::string()) const; //5A9E60, missing0.
		BYTE GetString(const std::string& variable,std::string section,std::string& output) const; //5A9F80.
		void SetInteger(std::string variable, LONG value, std::string section = std::string()); //5AA070.
		std::string name;
		Configuration::Settings values;
	};
	class Cutscene
	{
	public:
		Cutscene(); // 0x5A42B0, five DWORDs; no vtable.
		~Cutscene(); //404B70, ordinary nonvirtual lifetime.
		DWORD Start(DWORD cutsceneID, DWORD mapID); //5A42D0, immediate restart or pending start.
		DWORD Stop(); //5A4310, queue ending state; return current ID.
		void Update(); // 0x5A4340, pending/current state transition.
		DWORD state, id, pendingState, pendingID, value10;
	};
	class Gregorian;
	class Combat;
	class Effect;
	class PathManager;
	class Party;
	typedef ::Journal Journal;
	typedef ::Encyclopedia Encyclopedia;
	class StoreManager;
	class WorldMap;
	class Map;
	struct GraphicsSceneState;
	class VFX_Light;
	class VFX_EffectInstance;
	typedef std::map<std::string, Map*, Configuration::CaseInsensitiveLess> WorldMaps;
	class World : public IComponent, public IWorldServer, public IWorldClient
	{
	public:
		World(); //548A80; actual three direct bases and one shared virtual IWorld.
		virtual ~World(); //5483A0, actual member lifetime; not Cleanup.
		// IComponent-primary virtual surface, original slot order.
		virtual void Initialize(); //0x546690, 0x4.
		virtual void Cleanup(); //0x5455b0, 0x8.
		virtual void Update(); //0x404b70, 0xc.
		virtual void ClearAuxiliaryObjects(); //0x5460d0, 0x10.
		virtual void ReadSettings(); //0x546b50, 0x14.
		virtual void BeforeCleanup(); //0x404b70, 0x18.
		virtual PathManager* NewPathManager(); //0x545250, 0x1c.
		virtual Party* NewParty(); //0x5452b0, 0x20.
		virtual Journal* NewJournal(); //0x545310, 0x24.
		virtual Encyclopedia* NewEncyclopedia(); //0x545370, 0x28.
		virtual StoreManager* NewStoreManager(); //0x5453d0, 0x2c.
		virtual WorldMap* NewWorldMap(); //0x545490, 0x30.
		virtual Cutscene* NewCutscene(); //0x545430, 0x34.
		virtual void LoadMainmenu(DWORD unusedTime); //0x546fd0, 0x38.
		virtual void StartGame(DWORD worldTime); //0x5455a0, 0x3c.
		virtual void UpdateServerWorld(DWORD worldTime); //0x545d60, 0x40.
		virtual void UpdateClient(DWORD elapsed); //0x545ff0, 0x44.
		// IWorldServer virtual surface, original slot order.
		virtual DWORD GetSecondsPerRound(); //0x436240, 0x0.
		virtual float GetTimeScale(); //0x46f250, 0x4.
		virtual void SetTimeScale(float value); //0x46f260, 0x8.
		virtual DWORD GetUpdateInterval(); //0x46f270, 0xc.
		virtual void SetUpdateInterval(DWORD value); //0x544a60, 0x10.
		virtual BYTE IsServerFrameDue(DWORD time) const; //0x544a70, 0x14, AL result.
		virtual void UpdateServer(DWORD frameClock); //0x544a90, 0x18.
		virtual Combat* NewCombat(); //0x545100, 0x1c.
		virtual Combat* GetCombat(); //0x46f280, 0x20.
		virtual Script_Small* NewScript(DWORD ownerID); //0x544b80, 0x24.
		virtual Effect* NewEffect(DWORD type); //0x544bf0, 0x28.
		virtual void PackState(Net::Packet& packet); //0x545930, 0x2c.
		// IWorldClient virtual surface, original slot order.
		virtual void UpdateClientFrame(DWORD frameClock); //0x544b30, 0x0.
		virtual void SetActiveMap(DWORD id); //0x545820, 0x8.
		virtual void SetActiveMap(const std::string& name); //0x546140, 0x4.
		virtual OCTREE* GetCurrentMapOctree(); //0x544cf0, 0xc.
		virtual void UnpackState(Net::Packet& packet); //0x547e60, 0x10.
		virtual void CollectVisibleEffects(); //0x548840, 0x14.
		virtual void DrawVisible(); //0x545aa0, 0x18.
		virtual void PrepareLightIndices(); //0x544d10, 0x1c.
		// virtual-IWorld virtual surface, original slot order.
		virtual DWORD GetTime() const; //0x548580, 0x0.
		virtual void SetTime(DWORD value); //0x548590, 0x4.
		virtual DWORD GetServerTime() const; //0x5485a0, 0x8.
		virtual void SetServerTime(DWORD value); //0x5485b0, 0xc.
		virtual Gregorian* NewCalendar(); //0x5485c0, 0x10.
		virtual Action* NewAction(DWORD type); //0x5485d0, 0x14.
		virtual Entity* NewEntity(DWORD type); //0x5485e0, 0x18.
		virtual Map* NewMap(); //0x5485f0, 0x1c.
		virtual DWORD AllocateEffectID(); //0x548600, 0x20.
		virtual DWORD AllocateEntityID(); //0x548610, 0x24.
		virtual void SetEntityIDCounter(DWORD value); //0x548620, 0x28.
		virtual void ResetEntityIDCounter(); //0x548630, 0x2c.
		virtual DWORD ClassifyEntity(const std::string& name, BYTE alternate); //0x5487e0, 0x34.
		virtual DWORD ClassifyEntity(const char* name, BYTE alternate); //0x548640, 0x30.
		virtual DWORD AllocateMapID(); //0x548650, 0x38.
		virtual void SetMapIDCounter(DWORD value); //0x548660, 0x3c.
		virtual void ResetMapIDCounter(); //0x548670, 0x40.
		virtual DWORD AllocateTriggerID(); //0x548680, 0x44.
		virtual void SetTriggerIDCounter(DWORD value); //0x548690, 0x48.
		virtual void ResetTriggerIDCounter(); //0x5486a0, 0x4c.
		virtual void Reset(); //0x548810, 0x50.
		virtual Map* GetMap(DWORD id); //0x5487f0, 0x54.
		virtual Map* LoadMap(const char* name); //0x548830, 0x58.
		virtual DWORD GetCurrentMapID(); //0x5486b0, 0x5c.
		virtual Map* GetCurrentMap(); //0x548800, 0x60.
		virtual PathManager* GetPathManager(); //0x5486c0, 0x64.
		virtual Party* GetParty(); //0x5486d0, 0x68.
		virtual WorldParameters* GetGlobalParameters(); //0x5486e0, 0x6c.
		virtual WorldParameters* GetLocalParameters(); //0x5486f0, 0x70.
		virtual Gregorian* GetCalendar(); //0x548700, 0x74.
		virtual Journal* GetJournal(); //0x548710, 0x78.
		virtual Encyclopedia* GetEncyclopedia(); //0x548720, 0x7c.
		virtual StoreManager* GetStores(); //0x548730, 0x80.
		virtual TeamSystem* GetTeams(); //0x548740, 0x84.
		virtual SquadSystem* GetSquads(); //0x548750, 0x88.
		virtual WorldMap* GetWorldMap(const char* name); //0x548820, 0x8c.
		virtual WorldMap* GetCurrentWorldMap(); //0x548760, 0x90.
		virtual void SetCurrentWorldMap(const char* name); //0x548770, 0x94.
		virtual DWORD GetCutsceneState() const; //0x548780, 0x98.
		virtual DWORD GetCutsceneID() const; //0x548790, 0x9c.
		virtual DWORD StartCutscene(DWORD id, DWORD mapID); //0x5487a0, 0xa0.
		virtual DWORD StopCutscene(); //0x5487b0, 0xa4.
		virtual Cutscene* GetCutscene(); //0x5487c0, 0xa8.
		virtual ItemRegistry* GetItemRegistry(); //0x5487d0, 0xac.
		// Explicit source factors are ordinary methods, outside the original tables.
		void ReleaseInitializedOwners();
		void UpdateServerEntities(DWORD worldTime);
		void UpdateClientMaps(DWORD elapsed);
		void CollectVisibleEffects(OCTREE* tree, Map* activeMap);
		void DrawVisible(Octree2Gfx* renderer, OCTREE* tree, Gfx_Light* staticLights, DWORD shadowMode);
		void DrawVisible(GraphicsSceneState& graphics);
		void PrepareLightIndices(Octree2Gfx* renderer);
		void InitializeClocks(DWORD frameClock);
		void InitializeClocks(DWORD serverFrameClock, DWORD clientFrameClock);
		void ChangeState(DWORD state); //5454F0, actual RET4 state transition.
		void ChangeState(DWORD state, DWORD frameClock);
		bool BeginServerFrame(DWORD frameClock);
		void FinishServerFrame();
		bool BeginClientFrame(DWORD frameClock, DWORD& elapsed);
		void AdvanceNormalServerTime(DWORD worldTime);
		void AdvanceRoundServerTime();
		CalendarDate initialDate; //Original+14, live six-DWORD Gregorian input.
		std::list<WorldMap*> worldMapResources; //Original+2C, owning named WorldMap resources.
		DWORD value38;
		Gregorian* calendar; // Original+3C.
		DWORD value40, value44;
		Net::UInt32 time48, value4C; DWORD value50, value54; // Unwritten until actual startup/time producer.
		DWORD value58, value5C, value60, value64, value68, value6C; Net::UInt32 value70;
		float value74;
		Net::UInt32 value78; DWORD value7C; Net::UInt32 value80, value84, value88; //+7C effect counter is constructor-unwritten.
		Combat* combat; // Original+8C.
		WorldParameters parameters90, parametersB8;
		Net::UInt32 valueE0; // Active map ID.
		WorldMaps maps; // Original+E4, owning pointers; normal destructor frees nodes only.
		PathManager* paths;
		Party* party;
		Journal* journal;
		Encyclopedia* encyclopedia;
		StoreManager* stores;
		WorldMap* worldMap; // Original+104, NULL until a later producer.
		TeamSystem buckets108;
		SquadSystem buckets118;
		WORD* lightIndices; // Original+128, uninitialized scratch sized from the renderer's static lights.
		std::vector<DynamicObject*> visibleObjects; // Original+12C.
		std::list<VFX_Light*> visibleLights; // Original+13C, borrowed, sorted by priority.
		std::list<VFX_EffectInstance*> lateEffects; // Original+148, borrowed.
		Cutscene* cutscene154; // Separate owning pointer, NULL until actual Initialize.
		ItemRegistry values158; //World item registry, IWorld+AC getter5487D0/4701F0.
	private:
		void CollectVisibleObjects(OCTREE* tree); //54886A..548A21 shared collection prefix.
		World(const World&);
		World& operator=(const World&);
	};
	extern World* g_World; // Original70BD98.
	IWorldClient* GetWorldClientInstance(); //41D050, nullable client projection.
	DWORD GetCurrentClientMapID(); //44A4B0; client projection then unchecked shared-IWorld virtual query.
	inline IWorld* GetWorldInstance() { return g_World; } //401130, nullable shared projection.
	IWorldServer* GetWorldServerInstance(); //401150, real mode guard; NULL World returns NULL.
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char WorldDateSize[(sizeof(CalendarDate) == 24) ? 1 : -1];
	typedef char WorldDateYear[(offsetof(CalendarDate, year) == 0) ? 1 : -1];
	typedef char WorldDateMonth[(offsetof(CalendarDate, month) == 4) ? 1 : -1];
	typedef char WorldDateDay[(offsetof(CalendarDate, day) == 8) ? 1 : -1];
	typedef char WorldDateHour[(offsetof(CalendarDate, hour) == 12) ? 1 : -1];
	typedef char WorldDateMinute[(offsetof(CalendarDate, minute) == 16) ? 1 : -1];
	typedef char WorldDateSecond[(offsetof(CalendarDate, second) == 20) ? 1 : -1];
	typedef char WorldAfterDate[(offsetof(World, worldMapResources) == 0x2C) ? 1 : -1];
	typedef char WorldOriginalExtent[(sizeof(World) == 0x16C) ? 1 : -1];
	typedef char WorldOriginalPrefix[(offsetof(World, initialDate) == 0x14) ? 1 : -1];
	typedef char WorldOriginalTime[(offsetof(World, time48) == 0x48) ? 1 : -1];
	typedef char WorldOriginalRegistry[(offsetof(World, values158) == 0x158) ? 1 : -1];
#endif
}

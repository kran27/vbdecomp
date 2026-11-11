#include "startup/NetworkComponent.hpp"
#include "startup/MessageLoop.hpp"
#include "World.hpp"
#include "Map.hpp"
#include "ActionWander.hpp"
#include "Action.hpp"
#include "ActionDie.hpp"
#include "ActionEquipment.hpp"
#include "ActionMoveTo.hpp"
#include "ActionPickUp.hpp"
#include "ActionWait.hpp"
#include "ActionMessage.hpp"
#include "scripting/ScriptSmall.hpp"
#include "model/CommonException.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>
#include <stdexcept>

namespace JE
{
	BYTE ItemRegistry::Contains(const std::string& key) //5A0000.
	{
		const Base& values=*this;
		return values.find(key)!=values.end();
	}
	void ItemRegistry::RegisterName(const std::string& key) //5A04B0.
	{
		(*this)[key] = 0;
	}

	IWorldClient* GetWorldClientInstance() //41D050.
	{
		return g_World;
	}
	DWORD GetCurrentClientMapID() //44A4B0.
	{
		IWorldClient* client = g_World;
		return client->GetCurrentMapID();
	}
	IWorldServer* GetWorldServerInstance() //401150.
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		return g_World;
	}
	Cutscene::Cutscene() : state(0), id(0), pendingState(0), pendingID(0), value10(0) {}
	Cutscene::~Cutscene() {} //404B70.
	DWORD Cutscene::Start(DWORD cutsceneID, DWORD mapID)
	{
		if (!state || id == cutsceneID)
		{
			state = 1;
			id = cutsceneID;
			value10 = mapID;
			return cutsceneID;
		}
		pendingID = cutsceneID;
		pendingState = 1;
		value10 = mapID;
		return mapID;
	}
	DWORD Cutscene::Stop()
	{
		if (state) pendingState = 3;
		else LogDebugString("A call was made to Cutscene::StopCutscene but cutscene state was already invalid.");
		return id;
	}
	void Cutscene::Update()
	{
		switch (state)
		{
		case 1: state = 2; break;
		case 3: state = 0; id = 0; break;
		}
		switch (pendingState)
		{
		case 1:
			state = 1;
			id = pendingID;
			pendingID = 0;
			pendingState = 0;
			break;
		case 3:
			state = 3;
			pendingState = 0;
			break;
		}
	}
	World::World() : initialDate(),
		value38(0), calendar(NULL), value40(6), value44(0), value58(0), value5C(0), value60(0),
		value64(30), value68(0), value6C(0), value70(0), value74(1), value78(10), value80(0),
		value84(0), value88(0), combat(NULL), valueE0(0), paths(NULL), party(NULL), journal(NULL),
		encyclopedia(NULL), stores(NULL), worldMap(NULL), lightIndices(NULL), cutscene154(NULL)
	{
		// Actual56AE70/569F10 prepopulate list/map buckets0..1999. Their
		// container implementation is supplied by the real C++ library.
		worldMapResources.clear();
		if (g_World) throw ::Common::Exception("World object already exists.");
		g_World = this;
	}
	World::~World()
	{
		g_World = NULL; // Original clears before automatic reverse member destruction.
	}
	Entity* World::NewEntity(DWORD type) //545160; original full body, virtual-base wrapper5485E0.
	{
		if (type)
		{
			FatalError("World::CreateEntity() - Unknown entity type %d.\n", type);
			return NULL;
		}
		return new Entity;
	}
	DWORD World::ClassifyEntity(const char* name, BYTE)
	{
		if (name && *name) FatalError("World::GetTypeFromFilename() - Unknown entity type.");
		return 0;
	}
	DWORD World::ClassifyEntity(const std::string& name, BYTE alternate) //5456D0.
	{
		return ClassifyEntity(name.c_str(), alternate);
	}
	void World::Update() {} //404B70, actual IComponent no-op.
	DWORD World::GetSecondsPerRound() { return value40; } //436240.
	float World::GetTimeScale() { return value74; } //46F250.
	void World::SetTimeScale(float value) { value74 = value; } //46F260.
	DWORD World::GetUpdateInterval() { return value64; } //46F270.
	void World::SetUpdateInterval(DWORD value) { value64 = value; } //544A60.
	Combat* World::GetCombat() { return combat; } //46F280.
	void World::ResetEntityIDCounter() { value80 = 0; } //548630.
	void World::ResetMapIDCounter() { value84 = 0; } //548670.
	void World::SetTriggerIDCounter(DWORD value) { value88 = value & 0x1FFFFFFF; } //548690.
	void World::ResetTriggerIDCounter() { value88 = 0; } //5486A0.
	DWORD World::GetCurrentMapID() { return valueE0; } //5486B0.
	Map* World::GetCurrentMap() //548800; direct local lookup5458E0.
	{
		Map* result=NULL;
		WorldMaps::iterator entry=maps.begin();
		const WorldMaps::iterator end=maps.end();
		if (entry!=end)
		{
			const DWORD selectedID=valueE0;
			do
			{
				Map* candidate=entry->second;
				if (candidate->idB4==selectedID) { result=candidate; break; }
				++entry;
			} while (entry!=end);
		}
		return result;
	}
	PathManager* World::GetPathManager() { return paths; } //5486C0.
	Party* World::GetParty() { return party; } //5486D0.
	WorldParameters* World::GetGlobalParameters() { return &parameters90; } //5486E0.
	WorldParameters* World::GetLocalParameters() { return &parametersB8; } //5486F0.
	Gregorian* World::GetCalendar() { return calendar; } //548700.
	Journal* World::GetJournal() { return journal; } //548710.
	Encyclopedia* World::GetEncyclopedia() { return encyclopedia; } //548720.
	StoreManager* World::GetStores() { return stores; } //548730.
	TeamSystem* World::GetTeams() { return &buckets108; } //548740.
	SquadSystem* World::GetSquads() { return &buckets118; } //548750.
	WorldMap* World::GetCurrentWorldMap() { return worldMap; } //548760.
	void World::SetCurrentWorldMap(const char* name) //548770/shared544CB0.
	{
		worldMap = GetWorldMap(name);
	}
	Cutscene* World::GetCutscene() { return cutscene154; } //5487C0.
	ItemRegistry* World::GetItemRegistry() { return &values158; } //5487D0.
	Action* World::NewAction(DWORD type) //544E00; original full body, virtual-base wrapper5485D0.
	{
		switch (type)
		{
		case 0: return new ActionDie;
		case 1: return new ActionIdle;
		case 2: return new ActionMoveTo;
		case 3: return new ActionPickUp;
		case 4: return new ActionWander;
		case 5: return new ActionWait;
		case 6: return new ActionAnim;
		case 7: return new ActionMessage;
		case 9: return new ActionEquip;
		case 10: return new ActionUnequip;
		case 11: return new ActionSetActiveWeapon;
		default: FatalError("Unknown action sent to World::CreateAction() factory.");
		}
		return NULL;
	}
	Script_Small* World::NewScript(DWORD ownerID) { return new Script_Small(ownerID); }
	DWORD World::AllocateEntityID() // 0x548610: 8B adjustor to shared19B suffix46FE20, retained under46FE10.
	{
		return ++value80 & 0x1FFFFFFF;
	}
	DWORD World::GetTime() const { return time48; }
	void World::SetTime(DWORD time) { time48 = time; }
	DWORD World::GetServerTime() const { return value70; }
	void World::SetServerTime(DWORD time) { value70 = time; }
	DWORD World::GetCutsceneState() const
	{
		return cutscene154->state;
	}
	DWORD World::GetCutsceneID() const
	{
		return cutscene154->id;
	}
	void World::InitializeClocks(DWORD frameClock)
	{
		InitializeClocks(frameClock, frameClock);
	}
	void World::InitializeClocks(DWORD serverFrameClock, DWORD clientFrameClock)
	{
		// Two original56AF50 reads are explicit inputs. This is only the clock
		// prefix of Initialize546690, before controller/parameters/child owners.
		time48 = 0;
		value4C = serverFrameClock;
		value50 = 0;
		value54 = clientFrameClock;
	}
	void World::ChangeState(DWORD state)
	{
		if (value38 == 0 && state != 1 && state != 2)
			FatalError("Invalid state change in World::ChangeState().");
		if ((value38 == 1 || value38 == 2) && state != 3)
			FatalError("Invalid state change in World::ChangeState().");
		if (value38 == 3 && state != 4)
			FatalError("Invalid state change in World::ChangeState().");
		if (value38 == 4 && state != 3)
			FatalError("Invalid state change in Entity::ChangeState().");
		if (state == 4)
		{
			DWORD frameClock = F3::GetElapsedTimeMS();
			value38 = state;
			value58 = frameClock;
		}
		else
		{
			if (value38 == 4)
			{
				DWORD elapsed = F3::GetElapsedTimeMS() - value58;
				value60 = elapsed;
				value5C = elapsed;
			}
			value38 = state;
		}
	}

	void World::ChangeState(DWORD state, DWORD frameClock)
	{
		if ((value38 == 0 && state != 1 && state != 2)
			|| ((value38 == 1 || value38 == 2) && state != 3)
			|| (value38 == 3 && state != 4))
			FatalError("Invalid state change in World::ChangeState().");
		if (value38 == 4 && state != 3)
			FatalError("Invalid state change in Entity::ChangeState().");
		if (state == 4)
		{
			value38 = state;
			value58 = frameClock;
		}
		else
		{
			if (value38 == 4) value60 = value5C = frameClock - value58;
			value38 = state;
		}
	}
	BYTE World::IsServerFrameDue(DWORD time) const
	{
		return value6C <= time || value38 == 1 || value38 == 2;
	}
	bool World::BeginServerFrame(DWORD frameClock)
	{
		if (value38 == 4) return false;
		time48 += frameClock - value4C;
		if (value5C)
		{
			time48 -= value5C;
			value5C = 0;
		}
		value4C = frameClock;
		if (!IsServerFrameDue(time48)) return false;
		if (!value38) FatalError("Trying to update world when state is invalid");
		//UpdateServer dispatches state1/2/other through the actual primary children
		//before FinishServerFrame reads their live clock/interval fields.
		return true;
	}
	void World::FinishServerFrame()
	{
		value68 = time48;
		value6C += value64;
	}
	bool World::BeginClientFrame(DWORD frameClock, DWORD& elapsed)
	{
		if (value38 == 4) return false;
		DWORD previous = value50;
		value50 += frameClock - value54;
		if (value60)
		{
			value50 -= value60;
			value60 = 0;
		}
		value54 = frameClock;
		elapsed = value50 - previous;
		// Original tail-calls545FF0(elapsed). The explicit entry calls UpdateClient
		// after accepting this clock factor, preserving that same elapsed value.
		return true;
	}
	void World::AdvanceNormalServerTime(DWORD worldTime)
	{
		// Original unsigned delta is stored as float BEFORE time-scale multiply;
		// the product is stored as float before official __ftol2 truncation.
		float elapsed = static_cast<float>(worldTime - value68);
		float scaled = value74 * elapsed;
		DWORD previous = GetServerTime();
		SetServerTime(previous + static_cast<DWORD>(static_cast<__int64>(scaled)));
		// Original next updates calendar accumulator+44, then if >=1000 invokes
		// +3C owner virtual+58(accumulator/1000), then reloads+44 and takes%1000.
		// That owner and its callback are not part of this clock-only factor.
	}
	void World::AdvanceRoundServerTime()
	{
		SetServerTime(GetServerTime() + 1000 * value40);
		// Original next invokes calendar+3C virtual+58(value40).
	}
}

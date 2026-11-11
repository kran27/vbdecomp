#pragma once

#include "Server.hpp"
#include "Connection.hpp"
#include "DialogueManager.hpp"
#include <string>

struct D3DXVECTOR3;

namespace JE
{
	class Entity;
	class Map;
	class Server : public Net::Server
	{
	public:
		explicit Server(JE::Net::UInt16 port); // 0x5564D0.
		~Server(); // 0x5553C0.
		void Initialize(); // 0x5529A0.
		DWORD AllocateVisualEffectID(); //552A20; actual receiver, shared counter, skips zero.
		void SendDialogueEnd(Net::Packet& packet); //5529C0, reliable packet55.
		void UpdatePolling(); //552B60, Net prefix -> actual World server frame -> dialogue.
		void SetScriptStep(BYTE value); //5567F0; actual World server-frame tail.
		void SetBreakpoints(BYTE enabled); //5568C0, normalized flag and optional debug-window message.
		void SetLocalVariables(BYTE enabled); //5569E0, normalized flag/message then selected script refresh.
		JE::Net::UInt8 ScriptSuppressed(JE::Net::UInt32 ownerID) const; //5529E0, actual debug pause/pass/owner filter.
		void Cleanup(); // 0x552C70.
		void CreateConnections(); //Original vslot+30 ->0x552AC0.
		void PrepareStart(JE::Net::UInt32 index,Net::Packet& packet); //552CA0, identity then full World snapshot.
		void PrepareUpdate(JE::Net::UInt32 index,Net::Packet& packet); //5552C0.
		JE::Net::UInt8 OnDisconnectRequest(JE::Net::UInt32 index,Net::Packet& packet); //555380.
		void OnUserPacket(JE::Net::UInt32 index,Net::Packet& packet); //556EA0, recovered branches.
		virtual ServerDialogue* CreateDialogue(); //Original vslot+74 ->0x552A60.
		virtual JE::Net::UInt32 AllocateClientID(); //46CEA0, original vslot+78; GameServer inherits.
		void WriteClientIdentity(JE::Net::UInt32 clientIndex, Net::Packet& reply); //Explicit552CA0 identity prefix, before World serialization.
		void StartNewGame(); //556B60: broadcast58, actual World reset/load, live client spawn pass.
		void RegisterCharacter(const char* name); //5565B0: template name/suffix, register actual save-root Characters resource.
		const char* GetCharacterTemplate() const; //54A5F0, actual template string c_str boundary.
		Entity* SpawnNewGamePlayer(JE::Net::UInt32 clientIndex, Map* map, JE::Net::Int32 entrySlot); // Explicit556CF0..556E25 factor.
		Entity* CreateEntity(const char* resource, JE::Net::UInt8 loadModel, JE::Net::UInt8 alternateCreature); //5554B0.
		void DeleteEntity(Entity* entity); //5555A0; capture ID, virtual delete, reliable packet10.
		void StartCutscene(DWORD id, DWORD mapID); //556080, World then reliable77(id,mapID).
		void StopCutscene(); //5560C0, World then reliable78 with no payload.
		void SetCameraFog(DWORD map,float nearLimit,float farLimit,DWORD color,float density,BYTE enabled); //555F20.
		void SetHeightFog(DWORD map,float upperLimit,float lowerLimit,DWORD color,float density,BYTE enabled); //555F90.
		void ResetCameraFog(DWORD map); //556040, actual Map reset then reliable75.
		void ResetHeightFog(DWORD map); //556000, actual Map reset then reliable76.
		void SetWorldMapAreaVisible(const char* filename,DWORD id,BYTE enabled); //555D80, bit1 then packet46.
		void SetWorldMapAreaExplorable(const char* filename,DWORD id,BYTE enabled); //555E00, bit2.
		void SetWorldMapAreaExplored(const char* filename,DWORD id,BYTE enabled); //555E80, bit4.
		void SetEncyclopediaData(DWORD clientIndex,DWORD dataID,const char* filename); //555D70/554B40, targeted42.
		void AddItemToInventory(Entity* owner,Entity* item); //555640, actual transfer callback then packet49.
		void ReadInventoryTransferRequest(Net::Packet& packet); //556EA0 case49.
		void ReadInventoryDropRequest(Net::Packet& packet); //556EA0 case51.
		void PublishInventoryRemoval(DWORD owner, DWORD item, BYTE failure); //553B00,50.
		void SendEntityCount(DWORD entityID, DWORD count); //552D40, reliable52, RET8.
		void PublishInventoryTransfer(DWORD destinationID, DWORD itemID, DWORD sourceID, BYTE failure); //553850.
		void RemoveItemFromInventory(Entity* owner,Entity* item); //555680, actual remove callback then packet50.
		void RemoveEntityFromMap(Entity* entity); //5556C0, map detach then reliable packet13.
		void MoveEntityToMap(Entity* entity,DWORD mapID,const D3DXVECTOR3& position,
			const D3DXVECTOR3& rotation,BYTE flag); //5562F0, server guard/map lookup then5555D0.
		void AddEntityToMap(Entity* entity,Map* map,const D3DXVECTOR3& position,
			const D3DXVECTOR3& rotation,BYTE flag); //5555D0, actual spatial/model/floor/publication sequence.
		void SendCreatureFile(JE::Net::UInt32 clientIndex, const char* resource); //555170.
		void AssignEntityClient(JE::Net::UInt32 entityID, JE::Net::UInt32 clientIndex); //555750.
		void MoveEntityToMapEntry(Entity* entity, JE::Net::UInt32 mapID, JE::Net::UInt8 entryID, JE::Net::Int32 entrySlot, JE::Net::UInt8 flag); //556450.
		void MoveEntityToNamedMapEntry(Entity* entity, const char* mapName,
			JE::Net::UInt8 entryID, JE::Net::Int32 entrySlot, JE::Net::UInt8 flag); //5563D0 ->547C30/556270.
		void SendPlayerID(JE::Net::UInt32 clientIndex, JE::Net::UInt32 entityID, JE::Net::UInt8 flag); //555050.
		void SendConsoleMessage(JE::Net::UInt32 clientIndex, const char* text); //554F70, targeted68.
		void BroadcastLoadedMap(const std::string& name, JE::Net::UInt32 mapID); //552E20, reliable61.
		void SendActiveMap(JE::Net::UInt32 mapID, JE::Net::UInt8 broadcast,
			JE::Net::UInt32 clientIndex); //552F20, reliable63 with DWORD map ID.
		void WarpClient(JE::Net::UInt32 clientIndex, const char* name); //556EA0 case70 factor.
		JE::Net::UInt8 JoinParty(Entity* entity); //556370, Add -> packet16 -> team1.
		JE::Net::UInt8 AssignEntityTeam(JE::Net::UInt32 entityID, JE::Net::UInt32 teamID); //555890, mutates then packet22.
		JE::Net::UInt8 RemoveEntityTeam(JE::Net::UInt32 entityID); //555920, packet23.
		void SetEntityTeamRank(JE::Net::UInt32 entityID, JE::Net::UInt32 rank); //5559B0, packet24.
		JE::Net::UInt8 SetTeamAttitude(JE::Net::UInt32 source, JE::Net::UInt32 target, JE::Net::UInt32 attitude); //5559F0, packet27.
		JE::Net::UInt32 CreateTeam(); //555A90, packet28.
		JE::Net::UInt32 CreateSquad(); //555B00, packet29.
		JE::Net::UInt8 RemoveEntitySquad(JE::Net::UInt32 entityID); //555B70, packet30.
		void SetEntitySquadRank(JE::Net::UInt32 entityID, JE::Net::UInt32 rank); //555C00, packet31.
		JE::Net::UInt8 AssignEntitySquad(JE::Net::UInt32 entityID, JE::Net::UInt32 squadID); //555C40, packet32.
		JE::Net::UInt8 SetSquadAttitude(JE::Net::UInt32 source, JE::Net::UInt32 target, JE::Net::UInt32 attitude); //555CD0, packet33.
		void SetGlobal(std::string name, JE::Net::Int32 value); //5566B0, settings then packet65.
		void FloatingSpeechText(JE::Net::UInt32 entityID,JE::Net::UInt32 stringReference,JE::Net::UInt32 duration,
			JE::Net::UInt32 red,JE::Net::UInt32 green,JE::Net::UInt32 blue); //555F00->554D30, packet47.
		void FloatingDamageText(JE::Net::UInt32 entityID, const char* text,
			JE::Net::UInt32 red, JE::Net::UInt32 green, JE::Net::UInt32 blue); //555F10->554E50, packet48.
		void RemoveSound(Entity* entity,JE::Net::UInt32 soundID,JE::Net::UInt8 force); //555850->54F590/5541D0, packet21.
		JE::Net::UInt32 AttachSound(Entity* entity,const char* filename,JE::Net::UInt8 looping); //555810, ID ->required local child ->packet20.
		JE::Net::UInt32 AttachVisualEffect(Entity* entity,const char* name); //555790, allocate/attach then reliable18.
		void RemoveVisualEffect(Entity* entity,JE::Net::UInt32 id,JE::Net::UInt8 force); //5557D0, remove then reliable19.

		ServerDialogue* dialogue; // Original +0x5064; Cleanup does not clear it.
		JE::Net::UInt32 field0x5068;
		Net::Packet packet;
		JE::Net::UInt32 field0x70A4;
		std::string startMap;
		JE::Net::UInt8 entryPoint; // Original +0x70C4; base ctor leaves it unwritten.
		std::string characterTemplate;
		JE::Net::UInt8 field0x70E4, field0x70E5, field0x70E6;
		JE::Net::UInt32 field0x70E8;
		JE::Net::UInt8 field0x70EC, field0x70ED;

	private:
		void SendEntityCreation(const char* resource, DWORD entityID, BYTE loadModel, BYTE alternateCreature); //553600.
		void SendWorldMapArea(const char* filename, DWORD id, DWORD flags); //554C30, actual packet46 sender.
		void SendGlobal(std::string name, Net::Int32 value); //556100, actual by-value packet65 sender.
		void SendEntityDeletion(DWORD entityID); //553790, actual reliable packet10 sender.
		void SendCutsceneStart(DWORD id, DWORD mapID); //553470, actual reliable packet77 sender.
		void SendCutsceneStop(); //553550, actual reliable packet78 sender.
		void SendPartyJoin(DWORD entityID); //553DD0, actual reliable packet16 sender.
		void SendVisualEffectAttach(DWORD entityID, const char* name, DWORD effectID); //553E90, actual reliable packet18 sender.
		void SendVisualEffectRemove(DWORD entityID, DWORD effectID, BYTE force); //553F90, actual reliable packet19 sender.
		void SendSoundAttach(DWORD entityID, const char* filename, BYTE looping, DWORD soundID); //5540C0, actual reliable packet20 sender.
		void SendSoundRemove(DWORD entityID, DWORD soundID, BYTE force); //5541D0, actual reliable packet21 sender.
		void SendTeamAttitude(DWORD source, DWORD target, DWORD attitude); //554580.
		void SendEntitySquad(DWORD id, DWORD squad); //554970.
		void SendSquadAttitude(DWORD source, DWORD target, DWORD attitude); //554A50.
		void SendEntityTeam(DWORD id, DWORD team); //554300.
		void SendTeamRank(DWORD entityID, DWORD rank); //5544A0, actual reliable packet24 sender.
		void SendSquadRank(DWORD entityID, DWORD rank); //554890, actual reliable packet31 sender.
		void SendTeamRemoval(DWORD entityID); //5543E0, actual reliable packet23 sender.
		void SendSquadRemoval(DWORD entityID); //5547D0, actual reliable packet30 sender.
		void PlaceEntityAtMapEntry(Entity* entity, Map* map, BYTE entryID, Net::Int32 entrySlot, BYTE flag); //556270, actual shared map-entry placement.
		void SendEntityMapAddition(DWORD entityID, DWORD mapID, BYTE flag, const D3DXVECTOR3& position); //553990, actual reliable packet12 sender.
		void SendEntityMapRemoval(DWORD entityID); //553C30, actual reliable packet13 sender.
		void SendEntityClientAssignment(DWORD entityID, DWORD clientID); //553CF0, actual reliable packet14 sender.
		void SendTeamCreation(); //554670, actual reliable packet28 sender.
		void SendSquadCreation(); //554720, actual reliable packet29 sender.
		void SendCameraFogReset(DWORD mapID); //5532F0, actual reliable packet75 sender.
		void SendHeightFogReset(DWORD mapID); //5533B0, actual reliable packet76 sender.
		void SendCameraFog(DWORD mapID, float nearLimit, float farLimit, DWORD color, float density, BYTE enabled); //553010, actual reliable packet73 sender.
		void SendHeightFog(DWORD mapID, float upperLimit, float lowerLimit, DWORD color, float density, BYTE enabled); //553180, actual reliable packet74 sender.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char EngineServerDialogueOffset[(offsetof(Server, dialogue) == 0x5064) ? 1 : -1];
	typedef char EngineServerStateOffset[(offsetof(Server, field0x5068) == 0x5068) ? 1 : -1];
	typedef char EngineServerPacketOffset[(offsetof(Server, packet) == 0x506C) ? 1 : -1];
	typedef char EngineServerClientCounterOffset[(offsetof(Server, field0x70A4) == 0x70A4) ? 1 : -1];
	typedef char ServerOriginalStringSize[(sizeof(std::string) == 0x1C) ? 1 : -1];
	typedef char ServerOriginalStartMap[(offsetof(Server, startMap) == 0x70A8) ? 1 : -1];
	typedef char ServerOriginalEntryPoint[(offsetof(Server, entryPoint) == 0x70C4) ? 1 : -1];
	typedef char ServerOriginalCharacterTemplate[(offsetof(Server, characterTemplate) == 0x70C8) ? 1 : -1];
	typedef char ServerOriginalScriptStep[(offsetof(Server, field0x70E4) == 0x70E4) ? 1 : -1];
	typedef char ServerOriginalScriptPaused[(offsetof(Server, field0x70E5) == 0x70E5) ? 1 : -1];
	typedef char ServerOriginalScriptPass[(offsetof(Server, field0x70E6) == 0x70E6) ? 1 : -1];
	typedef char ServerOriginalScriptOwner[(offsetof(Server, field0x70E8) == 0x70E8) ? 1 : -1];
	typedef char ServerOriginalBreakpoints[(offsetof(Server, field0x70EC) == 0x70EC) ? 1 : -1];
	typedef char ServerOriginalLocalVariables[(offsetof(Server, field0x70ED) == 0x70ED) ? 1 : -1];
	typedef char ServerOriginalEngineSize[(sizeof(Server) == 0x70F0) ? 1 : -1];
#endif

	namespace EngineServerServices
	{
		void AssignString(std::string& target, const char* value);
		void DestroyDialogue(ServerDialogue* dialogue); // Qualified actual 0x55FA10.
		void FreeDialogue(ServerDialogue* dialogue);
		void CleanupNetServer(Net::Server& server);
	}

	namespace EngineServerConnectionServices
	{
		void* Allocate(std::size_t size);
		void Construct(Net::EngineServerClientConnection* connection);
		void Free(void* block) throw();
	}
}

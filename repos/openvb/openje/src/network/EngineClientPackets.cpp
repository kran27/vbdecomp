#include "EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "gui/DialogueInterface.hpp"
#include "gui/ConsoleInterface.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"
#include "entity/Inventory.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "entity/Store.hpp"
#include "resources/ResourceSystem.hpp"
#include "startup/SaveDirectory.hpp"
#include "startup/Path.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdio>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>

namespace JE
{
	namespace
	{
		float UpdateFloat(JE::Net::UInt32 bits)
		{
			float value;
			std::memcpy(&value,&bits,sizeof value);
			return value;
		}

		World& PacketWorld()
		{
			//41D050/401130 return the actual global World's two interfaces. They
			//do not add a Network-mode guard, and each original call reloads it.
			return *g_World;
		}
	}

	void Client::OnGamePacket(Net::Packet& packet) //58C660.
	{
		if (FilterGamePacket(packet)) return; //Original virtual+CC, before every payload read.
		switch (packet.field0x30)
		{
		case 9:
		{
			char name[260]; JE::Net::UInt32 id; JE::Net::UInt8 loadModel,alternate;
			packet.ReadCString(name); packet.ReadDword(id); packet.ReadBool(loadModel); packet.ReadBool(alternate);
			GetWorldClientInstance()->SetEntityIDCounter(id-1);
			DWORD type=GetWorldClientInstance()->ClassifyEntity(name,alternate);
			Entity* entity=GetWorldClientInstance()->NewEntity(type);
			entity->Create(name,loadModel,0);
			return;
		}
		case 10:
		{
			JE::Net::UInt32 id; packet.ReadDword(id);
			Entity* entity=GetEntityPtr(id);
			if (entity) delete entity; //Immediate virtual destruction, not Destroy's state transition.
			return;
		}
		case 12:
		{
			JE::Net::UInt32 id,mapID; JE::Net::UInt8 focus; float x,y,z;
			packet.ReadDword(id); packet.ReadDword(mapID); packet.ReadBool(focus);
			packet.ReadFloat(x); packet.ReadFloat(y); packet.ReadFloat(z);
			Map* map=GetWorldClientInstance()->GetMap(mapID);
			Entity* entity=GetEntityPtr(id);
			map->AttachEntity(entity);
			entity->SetPosition(D3DXVECTOR3(x,y,z));
			if (focus || (entity->visibilityOwnerFlag==clientID && id==field0xE0)) SelectEntity(id,1,1);
			return;
		}
		case 13:
		{
			JE::Net::UInt32 id; packet.ReadDword(id);
			RemoveClientEntity(id);
			Entity* entity=GetEntityPtr(id);
			if (!entity) FatalError("Error handling Network::ENGINE_ID_ENTITY_REMOVEFROMMAP (Client::recv_packet)\n\tInvalid entity\n");
			DWORD mapID=entity->mapHandle;
			Map* map=GetWorldClientInstance()->GetMap(mapID);
			if (!map) FatalError("Error handling Network::ENGINE_ID_ENTITY_REMOVEFROMMAP (Client::recv_packet)\n\tInvalid map\n");
			map->DetachEntity(entity);
			return;
		}
		case 14:
		{
			JE::Net::UInt32 id,owner; packet.ReadDword(id); packet.ReadDword(owner);
			Entity* entity=GetEntityPtr(id); if (entity) entity->SetClientOwner(owner);
			return;
		}
		case 16:
		case 17:
		{
			JE::Net::UInt32 id; packet.ReadDword(id);
			Entity* entity=GetEntityPtr(id);
			if (!entity)
				FatalError(packet.field0x30==16
					? "Error handling Network::ENGINE_ID_ENTITY_ADDTOPARTY (Client::recv_packet)\n\tInvalid Entity\n"
					: "Error handling Network::ENGINE_ID_ENTITY_REMOVEFROMPARTY (Client::recv_packet)\n\tInvalid Entity\n");
			if (packet.field0x30==16) PacketWorld().party->Add(entity);
			else PacketWorld().party->Remove(entity);
			return;
		}
		case 18:
		{
			JE::Net::UInt32 id,effect; char name[260];
			packet.ReadDword(id); packet.ReadCString(name); packet.ReadDword(effect);
			Entity* entity=GetEntityPtr(id); if (entity) entity->AttachVisualEffect(name,effect,1);
			return;
		}
		case 19:
		{
			JE::Net::UInt32 id,effect; JE::Net::UInt8 force;
			packet.ReadDword(id); packet.ReadDword(effect); packet.ReadBool(force);
			Entity* entity=GetEntityPtr(id); if (entity) entity->RemoveVisualEffectByID(effect,force);
			return;
		}
		case 20:
		{
			JE::Net::UInt32 id,sound; JE::Net::UInt8 loop; char name[260];
			packet.ReadDword(id); packet.ReadCString(name); packet.ReadBool(loop); packet.ReadDword(sound);
			Entity* entity=GetEntityPtr(id);
			if (entity)
			{
				entity->AttachSound(name,loop,sound);
			}
			return;
		}
		case 21:
		{
			JE::Net::UInt32 id,sound; JE::Net::UInt8 force;
			packet.ReadDword(id); packet.ReadDword(sound); packet.ReadBool(force);
			Entity* entity=GetEntityPtr(id); if (entity) entity->RemoveSound(sound,force);
			return;
		}
		case 22:
		{
			JE::Net::UInt32 id,group; packet.ReadDword(id); packet.ReadDword(group);
			IWorldClient* world = GetWorldClientInstance();
			world->GetTeams()->AddEntityTeam(id,group);
			return;
		}
		case 32:
		{
			JE::Net::UInt32 id,group; packet.ReadDword(id); packet.ReadDword(group);
			IWorldClient* world = GetWorldClientInstance();
			world->GetSquads()->AddEntitySquad(id,group);
			return;
		}
		case 23:
		{
			JE::Net::UInt32 id; packet.ReadDword(id);
			IWorldClient* world = GetWorldClientInstance();
			world->GetTeams()->RemoveEntityTeam(id);
			return;
		}
		case 30:
		{
			JE::Net::UInt32 id; packet.ReadDword(id);
			IWorldClient* world = GetWorldClientInstance();
			world->GetSquads()->RemoveEntitySquad(id);
			return;
		}
		case 24:
		case 31:
		{
			JE::Net::UInt32 id,rank; packet.ReadDword(id); packet.ReadDword(rank);
			Entity* entity=GetEntityPtr(id);
			if (entity) { if (packet.field0x30==24) entity->value36C=rank; else entity->squadRank=rank; }
			return;
		}
		case 25:
		{
			JE::Net::UInt32 sourceID,newID; packet.ReadDword(sourceID); packet.ReadDword(newID);
			Entity* source=GetEntityPtr(sourceID);
			PacketWorld().SetEntityIDCounter(newID-1);
			Entity* copy=PacketWorld().NewEntity(source->entityClass);
			copy->Create("",0,0);
			copy->CopyFrom(source);
			return;
		}
		case 27:
		{
			JE::Net::UInt32 source,target,attitude;
			packet.ReadDword(source); packet.ReadDword(target); packet.ReadDword(attitude);
			IWorldClient* world = GetWorldClientInstance();
			world->GetTeams()->SetTeamAttitude(source,target,attitude);
			return;
		}
		case 33:
		{
			JE::Net::UInt32 source,target,attitude;
			packet.ReadDword(source); packet.ReadDword(target); packet.ReadDword(attitude);
			IWorldClient* world = GetWorldClientInstance();
			world->GetSquads()->SetSquadAttitude(source,target,attitude);
			return;
		}
		case 28:
		{
			IWorldClient* world = GetWorldClientInstance();
			world->GetTeams()->CreateGroup();
			return;
		}
		case 29:
		{
			IWorldClient* world = GetWorldClientInstance();
			world->GetSquads()->CreateGroup();
			return;
		}
		case 34:
		{
			JE::Net::UInt32 entry,value; packet.ReadDword(entry); packet.ReadDword(value);
			PacketWorld().journal->AddEntry(entry,value); //Direct5A0610, not GameJournal's.
			RefreshInterface();
			return;
		}
		case 35: case 36: case 37: return; //Genuine empty original branches.
		case 38:
		{
			JE::Net::UInt32 id,value40,value4; char icon[128];
			packet.ReadDword(id); packet.ReadDword(value40); packet.ReadCString(icon); packet.ReadDword(value4);
			Encyclopedia* encyclopedia=PacketWorld().GetEncyclopedia();
			encyclopedia->AddEntry(id,value40,icon,value4);
			RefreshInterface();
			return;
		}
		case 39:
		{
			char name[260]; packet.ReadCString(name);
			Encyclopedia* encyclopedia=PacketWorld().GetEncyclopedia();
			encyclopedia->Read(name);
			RefreshInterface();
			return;
		}
		case 42:
		{
			JE::Net::UInt32 id; char name[260]; packet.ReadDword(id); packet.ReadCString(name);
			OnInterfaceMessage(id,name); //58CEED, actual virtual+7C; engine hook is empty.
			return;
		}
		case 43:
		case 44:
		{
			const bool addItem=packet.field0x30==43;
			char storeName[260],itemName[260]; JE::Net::UInt32 count;
			packet.ReadCString(storeName); packet.ReadCString(itemName); packet.ReadDword(count);
			StoreManager* stores=PacketWorld().GetStores();
			if (stores)
			{
				CStore* store=stores->GetStore(storeName);
				if (store)
				{
					if (addItem) store->Add(itemName,count,0,0,0,0);
					else store->Remove(itemName,count);
				}
			}
			RefreshInterface(); return;
		}
		case 46:
		{
			char resource[260]; JE::Net::UInt32 id,value;
			packet.ReadCString(resource); packet.ReadDword(id); packet.ReadDword(value);
			WorldMap* map=PacketWorld().GetWorldMap(resource);
			if (map)
			{
				WorldMapLocation* area=map->FindLocation(id);
				if (area) area->values[0]=value;
			}
			RefreshInterface(); //58CF55, including absent map/location.
			return;
		}
		case 47:
		{
			JE::Net::UInt32 id,text,duration,red,green,blue;
			packet.ReadDword(id); packet.ReadDword(text); packet.ReadDword(duration);
			packet.ReadDword(red); packet.ReadDword(green); packet.ReadDword(blue);
			AddFloatingSpeechText(id, text, duration, red, green, blue);
			return;
		}
		case 48:
		{
			JE::Net::UInt32 id,first,second,third; char text[128];
			packet.ReadDword(id); packet.ReadCString(text);
			packet.ReadDword(first); packet.ReadDword(second); packet.ReadDword(third);
			std::string value(text); //58D109 temporary, before actual owner dispatch.
			AddFloatingDamageText(id, value, first, second, third);
			return;
		}
		case 49:
		{
			JE::Net::UInt8 alreadyApplied; JE::Net::UInt32 ownerID,itemID,oldOwnerID;
			packet.ReadBool(alreadyApplied); packet.ReadDword(ownerID); packet.ReadDword(itemID); packet.ReadDword(oldOwnerID);
			if (!alreadyApplied)
			{
				Entity* owner=GetEntityPtr(ownerID);
				Entity* oldOwner=GetEntityPtr(oldOwnerID);
				Entity* item=oldOwner->inventory->FindByID(itemID);
				owner->inventory->Add(item);
			}
			RefreshInterface(); return;
		}
		case 50:
		{
			JE::Net::UInt8 alreadyApplied; JE::Net::UInt32 ownerID,itemID;
			packet.ReadBool(alreadyApplied); packet.ReadDword(ownerID); packet.ReadDword(itemID);
			if (!alreadyApplied)
			{
				Entity* owner=GetEntityPtr(ownerID);
				Entity* item=owner->inventory->FindByID(itemID);
				owner->inventory->Remove(item);
			}
			RefreshInterface(); return;
		}
		case 52:
		{
			JE::Net::UInt32 id,count; packet.ReadDword(id); packet.ReadDword(count);
			Entity* entity=GetEntityPtr(id);
			if (entity) { entity->SetCount(count); RefreshInterface(); }
			else LogDebugString("ENGINE_ID_ITEM_SETSTACKSIZE - Unable to look up entity to set the stacksize of.\n");
			return;
		}
		case 55: //588230: this packet removes a dialogue; updates arrive through5880C0.
		{
			ReceiveDialogueRemoval(packet);
			return;
		}
		case 58:
		{
			char name[260]; packet.ReadCString(name);
			PrepareMapTransition(0x58D2BA,0x58D2C5,0x58D2D7);
			PacketWorld().Reset();
			Map* loaded=PacketWorld().LoadMap(name);
			{
				std::string activeName(name); //58D312 precedes the next World reload.
				PacketWorld().SetActiveMap(activeName);
			}
			PublishMapID(loaded->idB4); //Captured LoadMap result, after temporary destruction.
			return;
		}
		case 61:
		{
			char name[260]; JE::Net::UInt32 mapID; packet.ReadCString(name); packet.ReadDword(mapID);
			PacketWorld().SetMapIDCounter(mapID);
			PacketWorld().LoadMap(name);
			return;
		}
		case 62:
			PacketWorld().Reset();
			MapTransitionInterface(0x58D3CD); //Reload actual cursorA4 after World.Reset.
			return;
		case 63:
		{
			char name[260]; packet.ReadCString(name);
			PacketWorld().SetActiveMap(name);
			MapTransitionInterface(0x58D42F); //A0 virtual+10(0).
			PublishMapID(PacketWorld().valueE0);
			return;
		}
		case 65:
		{
			char name[260]; JE::Net::UInt32 value; packet.ReadCString(name); packet.ReadDword(value);
			PacketWorld().parameters90.SetInteger(name,LONG(value),""); return;
		}
		case 66:
		{
			char name[260]; JE::Net::UInt32 value,mapID;
			packet.ReadCString(name); packet.ReadDword(value); packet.ReadDword(mapID);
			Map* map=PacketWorld().GetMap(mapID);
			if (!map) { LogDebugString("Invalid map ID caught in ENGINE_ID_SETLOCAL in Client::recv_packet: %d",mapID); return; }
			std::string section=map->resourceName;
			PacketWorld().parametersB8.SetInteger(name,LONG(value),section);
			return;
		}
		case 68:
		{
			char text[1024]; packet.ReadCString(text);
			child0x88->Print("%s",text); //58D5A2; actual required console and format671DC0.
			return;
		}
		case 69:
		{
			JE::Net::UInt32 id; JE::Net::UInt8 audio; packet.ReadDword(id); packet.ReadBool(audio);
			SelectEntity(id,1,audio); return;
		}
		case 71:
		{
			char name[260]; JE::Net::UInt32 size; packet.ReadCString(name); packet.ReadDword(size);
			void* payload=::operator new(size);
			packet.Read(payload,size); //Original ignores the reader result and retains the allocation.
			char path[260];
			std::strcpy(path,F3::GetSaveDirectoryAbsolute());
			F3::PathConcat(path,"characters");
			F3::PathConcat(path,name);
			Resources::ResourceFileHandle* file=Resources::g_pResourceSystem->OpenStream(path,0);
			if (file)
			{
				if (!Resources::g_pResourceSystem->Write(payload,size,file))
					FatalError("Error writing creature file %s"); //58D67C supplies no filename argument.
				Resources::g_pResourceSystem->Release(file,0);
			}
			return; //Original leaks payload on success and failure; no new cleanup or registration.
		}
		case 73: //58D69C..58D715; selected before reads and World callbacks.
		{
			JE::Net::UInt32 mapID,color; float first,second,third; JE::Net::UInt8 enabled;
			packet.ReadDword(mapID); packet.ReadFloat(first); packet.ReadFloat(second);
			packet.ReadDword(color); packet.ReadFloat(third); packet.ReadBool(enabled);
			IWorld* world = GetWorldInstance();
			world->GetMap(mapID)->SetCameraFog(first,second,color,third,enabled);
			return;
		}
		case 74: //58D71A..58D793; native normalized enabled BYTE.
		{
			JE::Net::UInt32 mapID,color; float first,second,third; JE::Net::UInt8 enabled;
			packet.ReadDword(mapID); packet.ReadFloat(first); packet.ReadFloat(second);
			packet.ReadDword(color); packet.ReadFloat(third); packet.ReadBool(enabled);
			IWorld* world = GetWorldInstance();
			world->GetMap(mapID)->SetHeightFog(first,second,color,third,enabled);
			return;
		}
		case 75: //58D798..58D7BC; original unchecked Map receiver.
		{
			JE::Net::UInt32 mapID; packet.ReadDword(mapID);
			GetWorldInstance()->GetMap(mapID)->ResetCameraFog();
			return;
		}
		case 76: //58D7C1..58D7E5; original unchecked Map receiver.
		{
			JE::Net::UInt32 mapID; packet.ReadDword(mapID);
			GetWorldInstance()->GetMap(mapID)->ResetHeightFog();
			return;
		}
		case 77:
		{
			JE::Net::UInt32 id,mapID; packet.ReadDword(id); packet.ReadDword(mapID);
			PacketWorld().StartCutscene(id,mapID); return; //58D812, actual World interface+A0.
		}
		case 78: PacketWorld().StopCutscene(); return; //58D823, actual World interface+A4.
		default:
		{
			char address[64];
			std::sprintf(address,"%u.%u.%u.%u:%u",packet.address.bytes[0],packet.address.bytes[1],
				packet.address.bytes[2],packet.address.bytes[3],packet.address.port);
			LogDebugString("Client received packet with unknown id %d from %s\r\n",packet.field0x30,address);
			return;
		}
		}
	}
	void Entity::ReadUpdate(Net::Packet& packet) //54ABB0.
	{
		Net::UInt32 word;
		D3DXVECTOR3 nextPosition(0, 0, 0);
		packet.ReadDword(word); std::memcpy(&nextPosition.x, &word, sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextPosition.y, &word, sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextPosition.z, &word, sizeof(word));
		D3DXVECTOR3 nextRotation(0, 0, 0);
		packet.ReadDword(word); std::memcpy(&nextRotation.x, &word, sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextRotation.y, &word, sizeof(word));
		packet.ReadDword(word); std::memcpy(&nextRotation.z, &word, sizeof(word));
		Net::UInt32 group, file, elapsed;
		packet.ReadDword(group);
		packet.ReadDword(file);
		packet.ReadDword(elapsed);
		if (F3::g_pNetworkInstance->GetMode() & 1) return;
		if (position.x != nextPosition.x || position.y != nextPosition.y || position.z != nextPosition.z)
			SetPosition(nextPosition);
		if (rotation.x != nextRotation.x || rotation.y != nextRotation.y || rotation.z != nextRotation.z)
			SetRotation(nextRotation);
		if (group != currentAnimationGroup || file != currentAnimationIndex)
		{
			PlayAnimation(group, file, 0, 0, 0);
			// Native54AD56 samples the ninth received DWORD after pushing EBX.
			animationStartTime = GetWorldInstance()->GetTime() - elapsed;
			float duration;
			if (model)
			{
				Gfx_AnimationInstance* animation = model->animation;
				duration = animation->animationObject->GetDuration(currentAnimationGroup, currentAnimationIndex, 0);
			}
			else duration = 2.0f;
			const DWORD negativeDuration = DWORD(static_cast<__int64>(duration * -1000.0f));
			animationEndTime = animationStartTime - negativeDuration;
			OnAnimationUpdateReceived(group, file);
		}
	}
}

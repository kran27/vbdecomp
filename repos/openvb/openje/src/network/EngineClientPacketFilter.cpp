#include "EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClientBinding.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "entity/VisualEffect.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/CameraControl.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/ClientCursor.hpp"
#include "audio/Audio.hpp"
#include "gui/Interfaces.hpp"
#include "CommandProcessor.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace JE
{
	void Client::PublishMapID(DWORD id) //589D90.
	{
		Net::Packet packet;
		packet.field0x30=64;
		packet.WriteDword(id);
		Send(packet,1);
	}
	void Client::MapTransitionInterface(DWORD address)
	{
		if ((address == 0x58C52A || address == 0x58D2C5 || address == 0x58D3CD) && child0xA4)
		{
			child0xA4->OnUnused(); //Actual cursor virtual+20; Game437B40 resets drag/context state.
			return;
		}
		if (child0xA0)
		{
			switch (address)
			{
			case 0x589F78: child0xA0->SetLoadingVisible(1); return;
			case 0x58C51F:
			case 0x58C5BD:
			case 0x58D2BA:
			case 0x58D42F: child0xA0->SetLoadingVisible(0); return;
			case 0x58DADC: child0xA0->RefreshSelection(); return;
			case 0x58C53C:
			case 0x58D2D7: child0xA0->ShowGameplay(); return;
			}
		}
		if (JE::GetEngineClientBinding(*this).unconsumedInterfaceAction)
			JE::GetEngineClientBinding(*this).unconsumedInterfaceAction(*this,address,JE::GetEngineClientBinding(*this).unconsumedInterfaceActionContext);
		else
		{
			char message[128];
			std::sprintf(message,"Client%08lX: actual map-transition interface child unresolved",address);
			throw std::logic_error(message);
		}
	}
	void Client::PrepareMapTransition()
	{
		PrepareMapTransition(0x58C51F, 0x58C52A, 0x58C53C);
	}
	void Client::PrepareMapTransition(DWORD loadingAddress, DWORD cursorAddress,
		DWORD gameplayAddress)
	{
		ClientChild0x84* camera=child0x84; //58C4F5 captures one receiver for both calls.
		if (!camera) throw std::logic_error("Client58C4FF: actual CameraControl required");
		camera->SetConstraints(1);
		camera->Reset();
		if (child0x74)
			child0x74->SetFogOfWarEnabled(1); //5810F0 uses this Graphics owner's backend+8.
		else
		{
			//Explicit workspace entry without child74 retains its borrowed global backend.
			if (!g_GfxSystem) throw std::logic_error("Client58C510: actual graphics interface required");
			g_GfxSystem->SetFogOfWarEnabled(1,NULL);
		}
		MapTransitionInterface(loadingAddress); //A0 virtual+10(0).
		MapTransitionInterface(cursorAddress); //A4 virtual+20().
		ClientChild0xA0* interfaces=child0xA0; //58C52D captures the owner before publishing110.
		field0x110=1;
		if (interfaces)
			interfaces->ShowGameplay(); //A0 virtual+14().
		else
			MapTransitionInterface(gameplayAddress); //Explicit absent-owner callback/diagnostic.
	}

	void Client::RefreshInterface() {} //Original vtable69C814+78 ->404B70 RET.

	void Client::ClearHoverEffect() //5881A0.
	{
		if (field0xDC)
		{
			Entity* first = GetEntityPtr(field0xDC);
			if (first)
			{
				Entity* second = GetEntityPtr(field0xDC);
				if (second && field0xFC)
				{
					second->RemoveVisualEffect(reinterpret_cast<VisualEffect*>(field0xFC), 1);
					field0xFC = 0;
				}
			}
		}
		field0xDC = 0;
	}

	void Client::RemoveClientEntity(DWORD id) //588200.
	{
		if (!id || id != field0xDC) return;
		ClearHoverEffect();
	}

	void Client::SelectEntity(DWORD id, BYTE select, BYTE audio) //58D9A0, not41C9D0.
	{
		Entity* entity;
		if (id && (entity = GetEntityPtr(id)))
		{
			DWORD mapID = entity->mapHandle;
			IWorldClient* world = g_World;
			world->SetActiveMap(mapID);
			PublishMapID(mapID);
			world = g_World;
			Map* current = world->GetCurrentMap();
			std::string name(current->resourceName.c_str());
			name.erase(name.size() - 4, 4);
			Gfx_System_Interface* graphics = g_GfxSystem;
			graphics->SetString248(name.c_str());
			char command[64];
			std::sprintf(command,"CameraSetTarget %u",id);
			child0x7C->Queue(command);
			if (select)
			{
				ClientChild0xA0* interfaces = child0xA0;
				field0xE0 = id;
				interfaces->RefreshSelection();
			}
			else field0xE0 = 0;
			field0xDC = 0;
			if (audio)
			{
				world = g_World;
				Map* musicMap = world->GetMap(mapID);
				child0x70->PlayMusic(musicMap->musicName.c_str());
			}
		}
		else LogDebugString("Invalid entity id passed to Client::SetFocus()\n");
	}

	BYTE Client::FilterGamePacket(Net::Packet& packet) //58C380.
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1)) return 0;
		switch (packet.field0x30)
		{
		case 9: case 10: case 14:
		case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
		case 24: case 25: case 27: case 28: case 29: case 30: case 31: case 32: case 33:
		case 61: case 62: case 65: case 66: case 71:
		case 73: case 74: case 75: case 76: case 77: case 78:
			return 1;
		case 34: case 38: case 39: case 49: case 50: case 52:
			RefreshInterface();
			return 1;
		case 12:
		{
			JE::Net::UInt32 id, mapID;
			BYTE focus;
			float positionX, positionY, positionZ;
			packet.ReadDword(id);
			packet.ReadDword(mapID); //Read but not used by this local-server branch.
			packet.ReadBool(focus);
			packet.ReadFloat(positionX);
			packet.ReadFloat(positionY);
			packet.ReadFloat(positionZ);
			Entity* entity = GetEntityPtr(id);
			if (entity)
			{
				D3DXVECTOR3 position;
				std::memcpy(&position.x, &positionX, sizeof(positionX));
				std::memcpy(&position.y, &positionY, sizeof(positionY));
				std::memcpy(&position.z, &positionZ, sizeof(positionZ));
				entity->SetPosition(position);
				if (focus || (entity->visibilityOwnerFlag == clientID && id == field0xE0))
					SelectEntity(id, 1, 1);
			}
			else
			{
				LogDebugString("Error handling Network::ENGINE_ID_ENTITY_ADDTOMAP (Client::FilterPacket)\n"
					"\tEntity not found.\n");
			}
			return 1;
		}
		case 13:
		{
			JE::Net::UInt32 id;
			packet.ReadDword(id);
			RemoveClientEntity(id);
			return 1;
		}
		case 58:
		{
			char name[260];
			packet.ReadCString(name);
			ClientChild0x84* camera=child0x84;
			camera->SetConstraints(1);
			camera->Reset();
			child0x74->SetFogOfWarEnabled(1);
			child0xA0->SetLoadingVisible(0);
			child0xA4->OnUnused();
			ClientChild0xA0* interfaces=child0xA0;
			field0x110=1;
			interfaces->ShowGameplay();
			GetWorldClientInstance()->SetActiveMap(std::string(name));
			PublishMapID(GetWorldClientInstance()->GetCurrentMapID());
			return 1;
		}
		case 63:
		{
			JE::Net::UInt32 mapID;
			packet.ReadDword(mapID);
			GetWorldClientInstance()->SetActiveMap(mapID);
			child0xA0->SetLoadingVisible(0);
			PublishMapID(mapID);
			return 1;
		}
		default:
			return 0;
		}
	}
}

#include <winsock2.h>
#include "ClientWorldCommands.hpp"
#include "GameClient.hpp"
#include "NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "entity/World.hpp"
#include "entity/Map.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/SystemInterface.hpp"
#include "entity/WorldCollections.hpp"
#include "gui/ConsoleInterface.hpp"
#include "gui/FadeManager.hpp"
#include "gui/Interfaces.hpp"
#include "graphics/WaterSampling.hpp"
#include <cstdlib>
#include <cstring>
#include <list>
#include <string>

namespace JE
{
	void Client::ToggleStartupMovies() //587930.
	{
		child0x74->SetStartupMovies(!child0x74->startupMovies);
	}

	void Client::ToggleDebugLights() //5879B0.
	{
		Graphics* graphics = child0x74;
		graphics->SetDebugLights(!graphics->flag5A);
	}

	void Client::ToggleDebugOctreeLights() //5879D0.
	{
		Graphics* graphics = child0x74;
		graphics->SetDebugOctreeLights(!graphics->flag5B);
	}

	void Client::ToggleBilinearFiltering() //587910.
	{
		Graphics* graphics = child0x74;
		graphics->SetLinearFiltering(!graphics->linearFiltering);
	}

	void Client::ToggleWireframe() //587970.
	{
		Graphics* graphics = child0x74;
		graphics->SetWireframe(!graphics->wireframe);
	}

	void Client::ToggleTransparency() //587990.
	{
		Graphics* graphics = child0x74;
		graphics->SetTransparency(!graphics->transparency);
	}

	void Client::ToggleMipMapping() //587A00.
	{
		Graphics* graphics = child0x74;
		graphics->SetMipFiltering(!graphics->flag57);
	}

	void Client::ToggleOctreeDrawing() //5879F0.
	{
		Graphics* graphics = child0x74;
		graphics->sceneEnabled = !graphics->sceneEnabled;
	}

	void Client::ToggleOctreeNodeDebug() //587A30.
	{
		Graphics* graphics = child0x74;
		graphics->debugBounds = !graphics->debugBounds;
	}

	void Client::ToggleShadow() //5878A0.
	{
		Client* client = static_cast<Client*>(F3::g_pNetworkInstance->client);
		Graphics* graphics = client->child0x74;
		if (graphics)
		{
			graphics->CycleShadowMode();
			ConsoleInterface* console = client->child0x88;
			if (console)
			{
				switch (child0x74->shadowMode)
				{
				case 0: console->Print("ShadowType: None"); break;
				case 1: console->Print("ShadowType: Projected"); break;
				case 2: console->Print("ShadowType: Stencil Volume"); break;
				}
			}
		}
	}

	void Client::CaptureScreen() //587A50.
	{
		child0x74->CaptureScreen();
	}

	void Client::SetCameraMouseRotate(BYTE value) //587870.
	{
		field0xD4 = value;
	}

	void Client::ToggleFogOfWar() //587A40.
	{
		child0x74->ToggleFogOfWar();
	}

	void Client::ToggleCameraFog() //587950.
	{
		child0x74->SetFogEnabled(!child0x74->GetFogEnabled());
	}

	void Client::ToggleLighting() //587880.
	{
		Graphics* graphics = child0x74;
		graphics->SetLighting(!graphics->lighting);
	}

	void Client::ToggleHeightFog() //587A20.
	{
		child0x74->ToggleHeightFog();
	}

	void Client::ToggleBuilding(LONG building) //58A890.
	{
		Graphics* graphics = child0x74;
		graphics->SetBuildingVisible(building, !graphics->GetBuildingVisible(building));
		IWorldClient* world = g_World;
		DWORD mapID = world->GetCurrentMapID();
		world = g_World;
		Map* map = world->GetMap(mapID);
		if (map)
		{
			AreaMap* areaMap = map->areaMap;
			if (areaMap)
			{
				areaMap->SetLayerVisible(building, static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74->GetBuildingVisible(building));
				IWorld* partyWorld = g_World;
				Party* party = partyWorld->GetParty();
				std::vector<Entity*>& members = party->members;
				for (std::vector<Entity*>::iterator i = members.begin(); i != members.end(); ++i)
					(*i)->JE::Entity::UpdateVisibility(1);
			}
		}
	}

	void Client::StartTestDialogue() //589CB0, shared body of58A7D0.
	{
		Net::Packet packet;
		packet.field0x30 = 53;
		const char* name = "White Ford Test Dialogue";
		packet.Write(name, static_cast<Net::UInt32>(std::strlen(name) + 1));
		Send(packet, 0);
	}

	void Client::RequestNewGame() //589F00, shared body of58DBE0.
	{
		if (F3::g_pNetworkInstance->GetMode() & 1)
		{
			Net::Packet request;
			request.field0x30 = 58;
			Send(request, 0);
			child0xA0->SetLoadingVisible(1);
		}
	}

	void Client::EndDialogue() //589E50.
	{
		Net::Packet packet;
		packet.field0x30 = 56;
		Send(packet, 0);
	}

	void Client::Warp(const char* map) //58A1F0.
	{
		Net::Packet packet;
		packet.field0x30 = 70;
		packet.Write(map, static_cast<Net::UInt32>(std::strlen(map) + 1));
		Send(packet, 0);
	}

	void Client::RequestCreateEntity(const std::string& resource, DWORD map,
		const D3DXVECTOR3& position, BYTE flag) //589820.
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1)) return;
		Net::Packet packet;
		packet.field0x30 = 9;
		const char* text = resource.c_str();
		packet.Write(text, static_cast<Net::UInt32>(std::strlen(text) + 1));
		packet.WriteDword(map);
		float value;
		value = position.x; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		value = position.y; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		value = position.z; packet.WriteDword(*reinterpret_cast<Net::UInt32*>(&value));
		packet.WriteByte(flag);
		Send(packet, 1);
	}

	void Client::RequestSetGlobal(std::string name, LONG value) //58A9A0.
	{
		Net::Packet packet;
		packet.field0x30 = 65;
		const char* text = name.c_str();
		packet.Write(text, static_cast<Net::UInt32>(std::strlen(text) + 1));
		packet.WriteSignedDword(value);
		Send(packet, 1);
	}

	void Client::RequestSetScript(DWORD entity, const std::string& name) //589A50.
	{
		if (!(F3::g_pNetworkInstance->GetMode() & 1)) return;
		Net::Packet packet;
		packet.field0x30 = 15;
		packet.WriteDword(entity);
		const char* text = name.c_str();
		packet.Write(text, static_cast<Net::UInt32>(std::strlen(text) + 1));
		Send(packet, 1);
	}

	void Client::FadeIn(DWORD duration, LONG red, DWORD green, DWORD blue) //588CE0, shared5B4B30.
	{
		child0xA8->FadeIn(duration, red, green, blue);
	}

	void Client::FadeOut(DWORD duration, LONG red, DWORD green, DWORD blue) //588CF0, shared5B4BD0.
	{
		child0xA8->FadeOut(duration, red, green, blue);
	}

	void Client::ToggleFogOfWarAutomap() //588F20; actual receiver unused.
	{
		g_GfxSystem->SetFlag2194(!g_GfxSystem->Flag2194());
	}

	void Client::ToggleVisualEffects() //588800.
	{
		++field0x104;
		if (static_cast<LONG>(field0x104) > 2) field0x104 = 0;
	}

	void Client::ToggleWater() //5888B0.
	{
		++field0x10C;
		if (static_cast<LONG>(field0x10C) > 2) field0x10C = 0;
		SetWaterSampling(field0x10C != 2);
	}

	void Client::ToggleFOWLOS() //58BCB0.
	{
		field0x100 = !field0x100;
		//The original constructs and destroys an unused empty list around the
		//Party traversal. Its element type is not established by the empty list.
		std::list<DWORD> unused;
		IWorld* world = g_World;
		Party* party = world->GetParty();
		std::vector<Entity*>& members = party->members;
		for (std::vector<Entity*>::iterator i = members.begin(); i != members.end(); ++i)
			(*i)->JE::Entity::UpdateVisibility(1);
	}
}


namespace F3
{
	void GameClient::ToggleRain() //4601F0, owned by command4606C0.
	{
		JE::Net::Packet packet;
		packet.field0x30 = 110;
		Send(packet, 1);
	}

	void GameClient::ToggleSnow() //4602A0.
	{
		JE::Net::Packet packet;
		packet.field0x30 = 111;
		Send(packet, 1);
	}

	void GameClient::ToggleWind() //460350.
	{
		JE::Net::Packet packet;
		packet.field0x30 = 112;
		Send(packet, 1);
	}
}

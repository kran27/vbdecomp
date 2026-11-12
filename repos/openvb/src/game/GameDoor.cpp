#include <winsock2.h>
#include "GameDoor.hpp"
#include "GameInterfaces.hpp"
#include "FeedbackInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/Inventory.hpp"
#include "entity/Map.hpp"
#include "entity/MapGrid.hpp"
#include "entity/World.hpp"
#include "resources/ResourceSystem.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/AnimationInstance.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "startup/EngineStringSystem.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>

namespace F3
{
	GameDoor::GameDoor() : open(0), locked(0)
	{
		entityClass = 3;
		value17C &= 0xFFFF00FF;
		// valueC0 is established by GDOR version1, not the constructor.
	}
	void GameDoor::CopyFrom(JE::Entity* source) //40F000, virtual adjustor40F2F0.
	{
		GameDoor* door = dynamic_cast<GameDoor*>(source);
		if (door)
		{
			open = door->open;
			locked = door->locked;
			keyResource = door->keyResource;
			valueC0 = door->valueC0;
		}
		//The original leaves doorHeights untouched and always delegates.
		GameUsableObject::CopyFrom(source);
	}

	BYTE GameDoor::IsVisible() { return 1; }
	std::list<JE::AreaMapHeight>* GameDoor::GetHeightInfluence()
	{
		return doorHeights.empty() ? NULL : &doorHeights;
	}
	BYTE GameDoor::ReadDoorData(JE::Resources::ResourceMemoryStream* stream)
	{
		BYTE value;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		open = value != 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		locked = value != 0;
		return JE::Common::ReadString(keyResource, stream) != 0;
	}
	BYTE GameDoor::ReadDoorVersion1(JE::Resources::ResourceMemoryStream* stream)
	{
		LONG randomRange = 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&valueC0, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&randomRange, 4, stream)) return 0;
		if (randomRange) valueC0 += rand() % (2 * randomRange) / -2;
		return 1;
	}
	BYTE GameDoor::ReadDoor(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		const DWORD version = chunk.version;
		if (!ReadDoorData(stream)) return 0;
		if (version >= 1 && !ReadDoorVersion1(stream)) return 0;
		return 1;
	}
	BYTE GameDoor::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'RODG') return GameUsableObject::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadDoor(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading door game data from entity file.");
		JE::Resources::CloseResourceMemoryStream(stream);
		return 0;
	}
	BYTE GameDoor::CanOperate(JE::Entity* actor)
	{
		if (locked) return actor->inventory->ContainsResource(keyResource) == 1;
		return 1;
	}
	void GameDoor::AddFootprint(JE::RLEGrid<JE::Entity*>* grid)
	{
		if (!open && !inFootprintGrid)
		{
			grid->AddFootprint(spatialFootprint);
			inFootprintGrid = 1;
		}
		JE::g_World->GetMap(mapHandle)->AddEntityHeightInfluence(this);
	}
	void GameDoor::RemoveFootprint(JE::RLEGrid<JE::Entity*>* grid)
	{
		if (inFootprintGrid)
		{
			grid->RemoveEntity(spatialFootprint->value);
			inFootprintGrid = 0;
		}
		JE::g_World->GetMap(mapHandle)->RemoveEntityHeightInfluence(this);
	}
	void GameDoor::OnActivate(DWORD time)
	{
		GameEntity::OnActivate(time);
		JE::Map* map = JE::g_World->GetMap(mapHandle);
		suppressIdleAnimation = 1;
		PlayAnimation(514, 0, 1, open ? 0 : 1, 0);
		BuildFogHeights();
		if (model && model->animation)
		{
			float duration = model->animation->animationObject->GetDuration(514, 0, 0);
			animationStartTime = static_cast<DWORD>(static_cast<__int64>(double(animationStartTime) - duration));
		}
		if (JE::GetEntityEnvironment().NetworkMode() & 1)
		{
			if (open) RemoveFootprint(map->entityGrid);
			else AddFootprint(map->entityGrid);
		}
	}
	void GameDoor::Operate(JE::Entity* actor)
	{
		JE::Map* map = JE::g_World->GetMap(mapHandle);
		suppressIdleAnimation = 1;
		cell result;
		cell argument = static_cast<cell>(actor->entityID);
		if (open)
		{
			if (!RunScript("OnClose", &result, 1, &argument) || result)
			{
				PlayAnimation(514, 0, 1, 1, 0);
				open = 0;
				if (JE::GetEntityEnvironment().NetworkMode() & 1) AddFootprint(map->entityGrid);
			}
			return;
		}
		if (RunScript("OnOpen", &result, 1, &argument) && !result) return;
		if (locked == 1)
		{
			if (actor->inventory->ContainsResource(keyResource) != 1)
			{
				if (JE::GetEntityEnvironment().IsPartyMember(actor) == 1)
				{
					JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
					FeedbackInterface* feedback =
						static_cast<GameInterfaces*>(client->child0xA0)->feedback;
					if (feedback)
					{
						std::string text;
						g_EngineStringSystem.GetString(0x513, text);
						feedback->AddMessage(text, 255, 0, 0, std::string());
					}
				}
				return;
			}
			if (JE::GetEntityEnvironment().IsPartyMember(actor) == 1)
			{
				std::string format;
				g_EngineStringSystem.GetString(0x54E, format);
				char buffer[256];
				sprintf(buffer, format.c_str(), keyResource.c_str());
				JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
				FeedbackInterface* feedback =
					static_cast<GameInterfaces*>(client->child0xA0)->feedback;
				if (feedback) feedback->AddMessage(buffer, 0, 255, 0, std::string());
			}
			locked = 0;
		}
		PlayAnimation(514, 0, 1, 0, 0);
		open = 1;
		if (JE::GetEntityEnvironment().NetworkMode() & 1) RemoveFootprint(map->entityGrid);
	}
}

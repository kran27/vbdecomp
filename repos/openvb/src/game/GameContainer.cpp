#include "GameContainer.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "resources/ResourceSystem.hpp"
#include <stdlib.h>

namespace F3
{
	GameContainer::GameContainer() : open(0), locked(0), valueC0(0)
	{
		entityClass = 4;
		value17C &= 0xFFFF00FF; //408CED clears the actual Entity byte17D.
	}

	void GameContainer::CopyFrom(JE::Entity* source) //409050, virtual adjustor409150.
	{
		GameContainer* container = dynamic_cast<GameContainer*>(source);
		if (container)
		{
			open = container->open;
			locked = container->locked;
			keyResource = container->keyResource;
			valueC0 = container->valueC0;
			valueC4 = container->valueC4;
		}
		GameUsableObject::CopyFrom(source);
	}

	BYTE GameContainer::IsVisible()
	{
		return 1;
	}

	BYTE GameContainer::ReadVersion0(JE::Resources::ResourceMemoryStream* stream)
	{
		BYTE value;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		open = value != 0; //ASM408A96 tests AL, not the full stream argument word.
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 1, stream)) return 0;
		locked = value != 0;
		return JE::Common::ReadString(keyResource, stream) != 0;
	}

	BYTE GameContainer::ReadVersion1(JE::Resources::ResourceMemoryStream* stream)
	{
		LONG randomRange = 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&valueC0, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&randomRange, 4, stream)) return 0;
		if (randomRange) valueC0 += rand() % (2 * randomRange) / -2;
		return JE::Common::ReadString(valueC4, stream) != 0;
	}

	BYTE GameContainer::ReadContainer(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		const DWORD version = chunk.version;
		if (!ReadVersion0(stream)) return 0;
		if (version >= 1 && !ReadVersion1(stream)) return 0;
		return 1;
	}

	BYTE GameContainer::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'NOCG') return GameUsableObject::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadContainer(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading container game data from entity file.");
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return 0;
	}

	void GameContainer::Operate(JE::Entity* actor)
	{
		suppressIdleAnimation = 1;
		cell result;
		cell argument = static_cast<cell>(actor->entityID);
		if (open)
		{
			if (!RunScript("OnClose", &result, 1, &argument) || result)
			{
				PlayAnimation(514, 0, 1, 1, 0);
				open = 0;
			}
		}
		else if (!RunScript("OnOpen", &result, 1, &argument) || result)
		{
			PlayAnimation(514, 0, 1, 0, 0);
			open = 1;
		}
		//408990 itself has no locked/key/permission or footprint mutation branch.
		//Its GameActionOperateContainer caller owns interaction/UI policy.
	}
}

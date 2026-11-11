#include "GameArmor.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <string.h>

namespace F3
{
	GameArmor::GameArmor() : value968(0)
	{
		entityClass = 7;
		memset(values944, 0, sizeof(values944));
	}

	void GameArmor::CopyFrom(JE::Entity* source) //4086E0, virtual adjustor408770.
	{
		GameArmor* armor = dynamic_cast<GameArmor*>(source);
		if (armor)
		{
			values944[0] = armor->values944[0];
			values944[1] = armor->values944[1];
			values944[2] = armor->values944[2];
			values944[3] = armor->values944[3];
			values944[4] = armor->values944[4];
			values944[5] = armor->values944[5];
			value968 = armor->value968;
		}
		GameItem::CopyFrom(source);
	}

	BYTE GameArmor::ReadArmor(JE::Resources::ResourceMemoryStream* stream)
	{
		for (DWORD i = 0; i < 6; ++i)
			if (!JE::Resources::g_pResourceSystem->ReadMemory(values944 + i, 4, stream)) return 0;
		return JE::Resources::g_pResourceSystem->ReadMemory(&value968, 4, stream) != 0;
	}

	BYTE GameArmor::ReadArmorData(JE::Common::Chunk&, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		return ReadArmor(stream) != 0;
	}

	BYTE GameArmor::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'RAIG') return GameItem::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadArmorData(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading armor game data from item file.");
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return 0;
	}
}

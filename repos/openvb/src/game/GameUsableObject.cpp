#include "GameUsableObject.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "resources/ResourceSystem.hpp"
#include <string.h>

namespace F3
{
	GameUsableObject::GameUsableObject() : value160(0)
	{
		entityClass = 2;
		memset(values164, 0, sizeof(values164));
		value17C &= 0xFFFF00FF; // Original4124C8 clears Entity byte+17D.
	}

	void GameUsableObject::CopyFrom(JE::Entity* source) //4125A0, virtual adjustor412630.
	{
		GameUsableObject* usable = dynamic_cast<GameUsableObject*>(source);
		if (usable)
		{
			value160 = usable->value160;
			values164[0] = usable->values164[0];
			values164[1] = usable->values164[1];
			values164[2] = usable->values164[2];
			values164[3] = usable->values164[3];
			values164[4] = usable->values164[4];
			values164[5] = usable->values164[5];
		}
		GameEntity::CopyFrom(source); //Original qualified call also runs on cast miss.
	}

	BYTE GameUsableObject::ReadUsable(JE::Common::Chunk&, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		BYTE read = JE::Resources::g_pResourceSystem->ReadMemory(&value160, 4, stream);
		if (!read) return read;
		for (DWORD i = 0; i < 6; ++i)
			if (!JE::Resources::g_pResourceSystem->ReadMemory(values164 + i, 4, stream)) return 0;
		return 1;
	}

	BYTE GameUsableObject::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'JBOG') return GameEntity::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadUsable(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading usable object game data from entity file.");
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return 0;
	}
}

#include "GameAmmo.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace F3
{
	GameAmmo::GameAmmo() : caliber(0), modifier(0), value952(0), value956(0), value960(0),
		value964(0), value972(0), value976(0)
	{
		entityClass = 9;
	}

	BYTE GameAmmo::ReadBaseAmmo(JE::Resources::ResourceMemoryStream* stream) //408160.
	{
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&caliber, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value952, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value956, 4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value960, 4, stream)) return 0;
		return JE::Resources::g_pResourceSystem->ReadMemory(&value964, 4, stream) != 0;
	}

	BYTE GameAmmo::ReadAmmo(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE) //408420, last incoming byte is unused.
	{
		const DWORD version = chunk.version;
		if (!ReadBaseAmmo(stream)) return 0;
		if (version >= 1 && (!JE::Resources::g_pResourceSystem->ReadMemory(&value972, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&value976, 4, stream))) return 0;
		if (version >= 2 && (!JE::Resources::g_pResourceSystem->ReadMemory(&modifier, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&value968, 4, stream))) return 0;
		return 1;
	}

	BYTE GameAmmo::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'MAIG') return GameItem::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadAmmo(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading ammo game data from item file.");
		JE::Resources::CloseResourceMemoryStream(stream);
		return 0;
	}
}

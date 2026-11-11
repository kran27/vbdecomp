#include "GameItem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "resources/ResourceSystem.hpp"

namespace F3
{
	std::string g_EmptySocketModel(""); //6F257C, constructed before WinMain; distinct from6F34FC.

	GameItem::GameItem() : value160(0)
	{
		entityClass = 5;
		// The remaining scalar fields are first written by the resource reader.
	}

	const std::string& GameItem::GetSocketModel(DWORD slot) const
	{
		switch (slot)
		{
		case 1: return socketModel1;
		case 2: return socketModel2;
		case 3: return socketModel3;
		case 4: return socketModel4;
		case 5: return socketModel5;
		case 6: return socketModel6;
		case 7: return socketModel7;
		case 8: return socketModel8;
		case 9: return socketModel9;
		case 10: return socketModel10;
		case 11: return socketModel11;
		case 12: return socketModel12;
		default: return g_EmptySocketModel;
		}
	}

	BYTE GameItem::ReadVersion0(JE::Resources::ResourceMemoryStream* stream)
	{
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&value160, 4, stream)
			|| !JE::Resources::g_pResourceSystem->ReadMemory(&value164, 4, stream)) return 0;
		if (value164 > 0) value160 = value164;
		DWORD count, value;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&count, 4, stream)) return 0;
		for (DWORD i = 0; i < count; ++i)
		{
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&value, 4, stream)) return 0;
			values168.push_back(value); // Original appends; a repeated read does not clear it.
		}
		BYTE flag;
		if (!JE::Common::ReadString(socketModel1, stream)
			|| !JE::Common::ReadString(socketTexture1, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		hideHair = flag != 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		hideBeard = flag != 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		hideMoustache = flag != 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		hideEyes = flag != 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		hidePonytail = flag != 0;
		if (!JE::Common::ReadString(socketModel2, stream)
			|| !JE::Common::ReadString(socketTexture2, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		socketFlag2 = flag != 0;
		if (!JE::Common::ReadString(socketModel5, stream)
			|| !JE::Common::ReadString(socketTexture5, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		socketFlag5 = flag != 0;
		if (!JE::Common::ReadString(socketModel4, stream)
			|| !JE::Common::ReadString(socketTexture4, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		socketFlag4 = flag != 0;
		if (!JE::Common::ReadString(socketModel6, stream)
			|| !JE::Common::ReadString(socketTexture6, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		socketFlag6 = flag != 0;
		if (!JE::Common::ReadString(socketModel3, stream)
			|| !JE::Common::ReadString(socketTexture3, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		socketFlag3 = flag != 0;
		if (!JE::Common::ReadString(socketModel7, stream)
			|| !JE::Common::ReadString(socketTexture7, stream)) return 0;
		if (!JE::Common::ReadString(socketModel10, stream)
			|| !JE::Common::ReadString(socketTexture10, stream)) return 0;
		if (!JE::Common::ReadString(socketModel8, stream)
			|| !JE::Common::ReadString(socketTexture8, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		hideVanity = flag != 0;
		if (!JE::Common::ReadString(socketModel9, stream)
			|| !JE::Common::ReadString(socketTexture9, stream)) return 0;
		if (!JE::Common::ReadString(socketModel11, stream)
			|| !JE::Common::ReadString(socketTexture11, stream)) return 0;
		if (!JE::Common::ReadString(socketModel12, stream)
			|| !JE::Common::ReadString(socketTexture12, stream)) return 0;
		if (!JE::Resources::g_pResourceSystem->ReadMemory(&flag, 1, stream)) return 0;
		socketFlag12 = flag != 0;
		return JE::Common::ReadString(heldModel, stream)
			&& JE::Common::ReadString(heldTexture, stream);
	}

	BYTE GameItem::ReadItem(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		const DWORD version = chunk.version;
		if (!ReadVersion0(stream)) return 0;
		if (version >= 1)
		{
			DWORD count = 0;
			if (!JE::Resources::g_pResourceSystem->ReadMemory(&count, 4, stream)) return 0;
			if (count > 0) SetCount(count);
		}
		return 1;
	}

	BYTE GameItem::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'MTIG') return GameEntity::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadItem(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading item game data from entity file.");
		JE::Resources::g_pResourceSystem->CloseMemoryStream(stream);
		return 0;
	}
}

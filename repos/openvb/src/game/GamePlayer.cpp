#include "GamePlayer.hpp"
#include "entity/MapFog.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>

namespace F3
{
	PlayerAreaMaps::PlayerAreaMaps() {}
	PlayerAreaMaps::~PlayerAreaMaps() { Clear(); }

	JE::AreaMap* PlayerAreaMaps::Find(const std::string& name)
	{
		std::map<std::string, JE::AreaMap*>::iterator i = maps.find(name);
		return i == maps.end() ? NULL : i->second;
	}

	void PlayerAreaMaps::Set(const std::string& name, JE::AreaMap* value)
	{
		std::map<std::string, JE::AreaMap*>::iterator i = maps.find(name);
		if (i != maps.end() && i->second) delete i->second;
		// Original470D90 deletes the old payload even when value is the same
		// pointer. Preserve that ownership policy rather than adding a guard.
		maps[name] = value;
	}

	void PlayerAreaMaps::Clear()
	{
		for (std::map<std::string, JE::AreaMap*>::iterator i = maps.begin(); i != maps.end(); ++i)
			if (i->second) delete i->second;
		maps.clear();
	}

	GamePlayer::GamePlayer()
	{
		// Original constructs characterName before allocating the cache.
		areaMaps = new(std::nothrow) PlayerAreaMaps;
	}

	GamePlayer::~GamePlayer()
	{
		if (areaMaps) delete areaMaps;
	}

	BYTE GamePlayer::ReadCharacter(JE::Resources::ResourceMemoryStream* stream)
	{
		return JE::Common::ReadString(characterName, stream) != 0;
	}

	BYTE GamePlayer::ReadCharacterData(JE::Common::Chunk&, JE::Resources::ResourceMemoryStream* stream,
		BYTE)
	{
		return ReadCharacter(stream) != 0;
	}

	BYTE GamePlayer::ReadFromFile(JE::Common::Chunk& chunk, JE::Resources::ResourceMemoryStream* stream,
		BYTE skipDefaultOverride)
	{
		if (chunk.type != 'RHCG') return GameCreature::ReadFromFile(chunk, stream, skipDefaultOverride);
		if (ReadCharacterData(chunk, stream, skipDefaultOverride)) return 1;
		JE::LogDebugString("Error reading character data from entity fil");
		JE::Resources::CloseResourceMemoryStream(stream);
		return 0;
	}
}

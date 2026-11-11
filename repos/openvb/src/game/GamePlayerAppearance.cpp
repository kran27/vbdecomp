#include "GamePlayer.hpp"
#include "../startup/EngineStringSystem.hpp"

namespace F3
{
	std::string GameEntity::GetDisplayName()
	{
		std::string text;
		GetString(stringRefs[0], text); //Actual56C280, including real token resolution.
		return text;
	}
	std::string GamePlayer::GetDisplayName()
	{
		if (!characterName.empty()) return characterName;
		return GameEntity::GetDisplayName();
	}
}

#pragma once

#include "audio/Audio.hpp"
#include <cstddef>

namespace F3
{
	class GameAudio : public JE::Audio
	{
	public:
		GameAudio() {}
		virtual ~GameAudio() {}
		void ResetDefaults(); // 0x45E390.
	};

	GameAudio* CreateGameAudio(); // 0x45E830.

	namespace GameAudioFactoryServices
	{
		void* Allocate(std::size_t size);
		void Construct(GameAudio* audio);
		void Free(void* block);
	}
	namespace GameAudioDefaultsServices
	{
		void Assign(std::string& target, const char* value);
	}
}

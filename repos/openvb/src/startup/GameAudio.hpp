#pragma once

#include "audio/Audio.hpp"

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
}

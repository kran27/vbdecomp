#include "GameAudio.hpp"

#include <new>

namespace F3
{
	void GameAudio::ResetDefaults()
	{
		JE::Audio::ResetDefaults();
		groupList = "FO3.gls";
		musicJumpList = "F3.sco";
	}

	GameAudio* CreateGameAudio()
	{
		void* block = ::operator new(sizeof(GameAudio));
		if (!block) return NULL;
		GameAudio* audio = static_cast<GameAudio*>(block);
		try
		{
			::new (audio) GameAudio;
			return audio;
		}
		catch (...)
		{
			::operator delete(block);
			throw;
		}
	}
}

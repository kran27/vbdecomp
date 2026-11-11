#include "GameAudio.hpp"

#include <new>

namespace F3
{
	void GameAudio::ResetDefaults()
	{
		JE::Audio::ResetDefaults();
		GameAudioDefaultsServices::Assign(groupList, "FO3.gls");
		GameAudioDefaultsServices::Assign(musicJumpList, "F3.sco");
	}

	GameAudio* CreateGameAudio()
	{
		void* block = GameAudioFactoryServices::Allocate(sizeof(GameAudio));
		if (!block) return NULL;
		GameAudio* audio = static_cast<GameAudio*>(block);
		try
		{
			GameAudioFactoryServices::Construct(audio);
			return audio;
		}
		catch (...)
		{
			GameAudioFactoryServices::Free(block);
			throw;
		}
	}

	namespace GameAudioFactoryServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Construct(GameAudio* audio) { ::new (audio) GameAudio; }
		void Free(void* block) { ::operator delete(block); }
	}
	namespace GameAudioDefaultsServices
	{
		void Assign(std::string& target, const char* value) { target = value; }
	}
}

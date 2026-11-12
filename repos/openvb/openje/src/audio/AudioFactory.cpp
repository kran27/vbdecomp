#include "AudioFactory.hpp"
#include <new>

namespace JE
{
	Audio::~Audio()
	{
		Audio::SaveSettings();
		Cleanup();
		// Ordinary reverse member lifetime releases musicJumpList then groupList.
	}

	Audio* CreateAudio()
	{
		void* block = ::operator new(sizeof(Audio));
		if (!block) return NULL;
		Audio* audio = static_cast<Audio*>(block);
		try
		{
			::new (audio) Audio;
			return audio;
		}
		catch (...)
		{
			::operator delete(block);
			throw;
		}
	}
}

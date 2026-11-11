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
		void* block = AudioFactoryServices::Allocate(sizeof(Audio));
		if (!block) return NULL;
		Audio* audio = static_cast<Audio*>(block);
		try
		{
			AudioFactoryServices::Construct(audio);
			return audio;
		}
		catch (...)
		{
			AudioFactoryServices::Free(block);
			throw;
		}
	}

	namespace AudioFactoryServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Construct(Audio* audio) { ::new (audio) Audio; }
		void Free(void* block) throw() { ::operator delete(block); }
	}
}

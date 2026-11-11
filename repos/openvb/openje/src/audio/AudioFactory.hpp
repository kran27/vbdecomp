#pragma once

#include "Audio.hpp"
#include <cstddef>

namespace JE
{
	Audio* CreateAudio(); // 0x587B10.
	namespace AudioFactoryServices
	{
		void* Allocate(std::size_t size);
		void Construct(Audio* audio);
		void Free(void* block) throw();
	}
}

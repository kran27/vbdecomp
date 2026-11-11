#pragma once

#include <windows.h>

namespace JE
{
	class Entity;
	namespace EngineServerSoundServices
	{
		// Explicit opt-in preview capture of an UNCONSUMED local audio request.
		// No CSound/binding or playback result is produced. NULL uses the actual
		// Entity owner, whose required backend frontier remains addressed.
		class UnconsumedRequests
		{
		public:
			virtual ~UnconsumedRequests() {}
			virtual void CaptureAttachSound(Entity* entity, const char* filename,
				BYTE looping, DWORD soundID)=0;
		};
		extern UnconsumedRequests* g_UnconsumedRequests; //Borrowed, NULL by default.
		void AttachSound(Entity* entity, const char* filename, BYTE looping, DWORD soundID);
	}
}

#include "SoundSystem.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstdlib>
#include <new>

namespace JE
{
	SoundSystem::~SoundSystem()
	{
		ClearSoundNodes();
	}

	namespace AudioBackend
	{
		SoundSystem* g_System = 0; // Original initialized DWORD is zero.
		SoundSystem* GetSystem() { return g_System; } //52A000.

		void InitializeBackend(HWND window,Resources::ResourceSystem* resources,
			const char* groups,const char* jumps,bool directShow,bool directMusic,bool cdAudio)
		{
			if (g_System) return;
			SoundSystem* created=new SoundSystem;
			g_System=created; //52ACF0 publishes only after construction.
			if (!created) return;
			created->InitializeDevice(window,2,2,22050,16);
			g_System->AttachResources(resources);
			if (groups) g_System->InitializeGroups(groups);
			if (jumps) g_System->InitializeJumpList(jumps);
			if (directShow) g_System->InitializeDirectShow(window);
			if (directMusic) g_System->InitializeDirectMusic(window);
			if (cdAudio) g_System->InitializeCD();
		}
		void UpdateBackend() { if (g_System && g_System->initialized) g_System->Update(); }
		void ShutdownBackend()
		{
			if (!g_System) return;
			g_System->CleanupCD();
			g_System->CleanupDirectMusic();
			g_System->CleanupDirectShow();
			g_System->CleanupJumpList();
			g_System->CleanupInstanceGroups();
			g_System->CleanupDevice();
			SoundSystem* victim=g_System; //The final deleting destructor is conditional.
			if (victim) delete victim;
			g_System=NULL; //Also clears a replacement published during destruction.
		}
	}
}

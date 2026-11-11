#include <winsock2.h>
#include "audio/Audio.hpp"
#include "audio/SoundSystem.hpp"
#include "audio/SoundInstanceManager.hpp"
#include "audio/Music.hpp"
#include "network/EngineClient.hpp"
#include "graphics/CameraControl.hpp"
#include "ConfigurationObject.hpp"
#include "EngineSettings.hpp"
#include <windows.h>
#include <new>
#include <process.h>

namespace JE
{
	namespace AudioServices
	{
		unsigned __stdcall Worker(void*) //5909A0.
		{
			while (g_WorkerRunning)
			{
				::Sleep(100);
				AudioBackend::UpdateBackend();
			}
			::_endthreadex(0);
			return 0;
		}

		JE::Net::UInt8 g_WorkerRunning = 1; // Original byte 0x6FF2ED is one.

		Configuration::Settings* GetEngineSettings() { return F3::GetEngineSettings(); }
		Configuration::Settings* GetSettings() { return F3::GetSettingsObject(); }

		void InitializeBackend(const char* groups, const char* jumps,
			bool directShow, bool directMusic, bool cdAudio)
		{
			AudioBackend::InitializeBackend(groups, jumps, directShow, directMusic, cdAudio);
		}
		::uintptr_t StartWorker()
		{
			unsigned threadID;
			return ::_beginthreadex(NULL, 0, Worker, NULL, 0, &threadID);
		}
		void CloseWorker(::uintptr_t handle)
		{
			::CloseHandle(reinterpret_cast<HANDLE>(handle));
		}
		void Sleep(JE::Net::UInt32 milliseconds) { ::Sleep(milliseconds); }
		AudioMusic* CreateMusic()
		{
			return new (std::nothrow) CMusic;
		}
		void DestroyMusic(AudioMusic* music)
		{
			music->~CMusic();
		}
		void FreeMusic(AudioMusic* music) { ::operator delete(music); }
		void ShutdownBackend()
		{
			AudioBackend::ShutdownBackend();
		}
	}
}

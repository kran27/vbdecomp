#include <winsock2.h>
#include "Audio.hpp"
#include "Music.hpp"

#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include "SoundSystem.hpp"
#include "SoundInstanceManager.hpp"
#include "graphics/CameraControl.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "resources/ResourceSystem.hpp"
#include "startup/Window.hpp"
#include "startup/ConfigurationObject.hpp"
#include "startup/EngineSettings.hpp"
#include <process.h>

namespace JE
{
	Audio::Audio() : directShow(0), directMusic(0), cdAudio(0), music(NULL),
		musicEnabled(1), musicVolume(100)
	{
	}

	void Audio::ResetDefaults()
	{
		groupList.clear();
		musicJumpList.clear();
		directShow = 0;
		directMusic = 0;
		cdAudio = 0;
		musicEnabled = 1;
		musicVolume = 100;
	}

	void Audio::Initialize()
	{
		ResetDefaults();
		typedef Configuration::Settings Settings;
		typedef Configuration::Values Values;
		Settings& engine = *F3::GetEngineSettings();
		{
			Settings::iterator section = engine.find("Audio");
			if (section != engine.end())
			{
				Values& values = section->second;
				{
					Values::const_iterator value = values.find("GroupList");
					if (value != values.end()) groupList.assign(value->second.c_str());
				}
				{
					Values::const_iterator value = values.find("MusicJumpList");
					if (value != values.end()) musicJumpList.assign(value->second.c_str());
				}
			}
		}
		Settings& settings = *F3::GetSettingsObject();
		{
			Settings::iterator section = settings.find("Audio");
			if (section != settings.end())
			{
				Values& values = section->second;
				{
					Values::const_iterator value = values.find("DirectShow");
					if (value != values.end()) directShow = std::atol(value->second.c_str()) > 0;
				}
				{
					Values::const_iterator value = values.find("DirectMusic");
					if (value != values.end()) directMusic = std::atol(value->second.c_str()) > 0;
				}
				{
					Values::const_iterator value = values.find("CDAudio");
					if (value != values.end()) cdAudio = std::atol(value->second.c_str()) > 0;
				}
				{
					Values::const_iterator value = values.find("Music");
					if (value != values.end()) musicEnabled = std::atol(value->second.c_str()) > 0;
				}
				{
					Values::const_iterator value = values.find("MusicVolume");
					if (value != values.end()) musicVolume = static_cast<JE::Net::UInt32>(std::atol(value->second.c_str()));
				}
			}
		}
	}

	void Audio::SaveSettings()
	{
		Configuration::Settings& settings = *F3::GetSettingsObject();
		Configuration::Values& section = settings["Audio"];
		char buffer[36];
		section["DirectShow"] = ::_itoa(directShow != 0, buffer, 10);
		section["DirectMusic"] = ::_itoa(directMusic != 0, buffer, 10);
		section["CDAudio"] = ::_itoa(cdAudio != 0, buffer, 10);
		section["Music"] = ::_itoa(musicEnabled != 0, buffer, 10);
		section["MusicVolume"] = ::_itoa(static_cast<JE::Net::Int32>(musicVolume), buffer, 10);
	}

	void Audio::Prepare()
	{
		const char* groups = NULL;
		if (!groupList.empty()) groups = groupList.c_str();
		const char* jumps = NULL;
		if (!musicJumpList.empty()) jumps = musicJumpList.c_str();
		AudioBackend::InitializeBackend(F3::g_window, Resources::g_pResourceSystem,
			groups, jumps, directShow != 0, directMusic != 0, cdAudio != 0);
		unsigned threadID;
		const uintptr_t worker = ::_beginthreadex(NULL, 0, AudioServices::Worker, NULL, 0, &threadID);
		if (worker) ::CloseHandle(reinterpret_cast<HANDLE>(worker));
		music = new CMusic;
	}

	void Audio::Cleanup()
	{
		AudioServices::g_WorkerRunning = 0;
		::Sleep(500);
		CMusic* captured = music;
		if (captured)
		{
			delete captured;
			music = NULL;
		}
		AudioBackend::ShutdownBackend();
	}

	void Audio::Update()
	{
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		CameraControl* camera = static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x84;
		SoundInstanceManager* manager = AudioBackend::GetSystem()->instanceManager;
		if (manager)
		{
			if (camera)
			{
				camera->GetTarget(target);
				manager->SetListenerPosition(target);
			}
			else
				manager->SetListenerPosition(target);
		}
	}

	void Audio::StopMusic(JE::Net::UInt32 fadeOut)
	{
		if (music)
			music->Stop(fadeOut);
	}

	void Audio::PlayMusic(const char* name)
	{
		if (!music || !musicEnabled)
			return;
		char text[256];
		std::sprintf(text, "Music volume being set to %d", musicVolume);
		::OutputDebugStringA(text);
		::OutputDebugStringA("\n");
		music->SetVolume(static_cast<int>(musicVolume), 0);
		music->SetPiece(name);
	}
}

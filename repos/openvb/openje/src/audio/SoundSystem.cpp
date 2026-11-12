#include "SoundSystem.hpp"
#include "Sound.hpp"
#include "SoundInstance.hpp"
#include "SoundInstanceManager.hpp"
#include "SoundPreset.hpp"
#include "SoundGroups.hpp"
#include "Music.hpp"
#include "OptionalAudio.hpp"
#include "resources/ResourceSystem.hpp"
#include <mmsystem.h>
#include <cmath>
#include <cstdlib>
#include <new>

namespace JE
{
	namespace
	{
		struct SoundGuard
		{
			explicit SoundGuard(Resources::ResourceLock& value) : lock(value) { lock.Lock(); }
			~SoundGuard() { lock.Unlock(); }
			Resources::ResourceLock& lock;
		};
	}
	SoundSystem::SoundSystem()
	{
		ResetDefaults();
		BuildVolumeTable();
		now=timeGetTime();
		std::srand(now);
	}
	void SoundSystem::ResetDefaults() //52AB30.
	{
		device=NULL; primary=NULL; listener=NULL; scratch=NULL; scratchProperties=NULL;
		resources=NULL;
		optionalShow=NULL; optionalMusic=NULL; optionalCD=NULL; instanceManager=NULL;
		bitsPerSample=16; sampleRate=22050; channels=2;
		sounds.clear();
		jumps=NULL; groups=NULL; value18=0;
		initialized=0; masterVolume=100;
		enable3D=1; flag3C=1; flag40=1; flag44=1;
	}
	void SoundSystem::BuildVolumeTable() //52A1A0, native constructor call52AC3B.
	{
		volumeTable[0]=-10000; volumeTable[100]=0;
		for (int i=1;i<100;++i) volumeTable[i]=LONG(std::log(double(i)*0.01)*2000.0);
	}
	void SoundSystem::ClearSoundNodes()
	{
		sounds.clear();
	}
	void SoundSystem::RegisterSound(CSound* sound) //52AEC0.
	{
		SoundGuard guard(soundLock);
		sounds.push_back(sound);
	}
	void SoundSystem::UnregisterSound(CSound* sound) //52A930.
	{
		soundLock.Lock();
		sounds.remove(sound);
		soundLock.Unlock();
	}
	void SoundSystem::AttachResources(Resources::ResourceSystem* owner)
	{
		resources=owner;
	}
	void SoundSystem::InitializeGroups(const char* name)
	{
		groups=new MasterSoundGroups;
		if (groups) groups->Read(name);
		instanceManager=new SoundInstanceManager; //Even if group allocation failed.
		if (instanceManager && instanceManager->Initialize()<0)
		{
			SoundInstanceManager* victim=instanceManager;
			if (victim)
			{
				delete victim;
				instanceManager=NULL;
			}
		}
	}
	void SoundSystem::InitializeJumpList(const char* name)
	{
		jumps=new CMasterJumplist;
		if (jumps) jumps->Read(name);
	}
	int SoundSystem::InitializeDirectShow(HWND) //52A5F0.
	{
		if (optionalShow) { delete optionalShow; optionalShow = NULL; }
		optionalShow = new DirectShowAudio;
		if (optionalShow && optionalShow->Initialize() < 0)
		{
			if (optionalShow) { delete optionalShow; optionalShow = NULL; }
		}
		return optionalShow ? 0 : 9;
	}
	int SoundSystem::InitializeDirectMusic(HWND window) //52A6C0.
	{
		if (optionalMusic) { delete optionalMusic; optionalMusic = NULL; }
		optionalMusic = new DirectMusicAudio;
		if (optionalMusic && optionalMusic->Initialize(window) < 0)
		{
			if (optionalMusic) { delete optionalMusic; optionalMusic = NULL; }
		}
		return optionalMusic ? 0 : 9;
	}
	int SoundSystem::InitializeCD() //52A790.
	{
		if (optionalCD) { delete optionalCD; optionalCD = NULL; }
		optionalCD = new CDAudio;
		if (optionalCD && optionalCD->Initialize())
		{
			if (optionalCD) { delete optionalCD; optionalCD = NULL; }
		}
		return optionalCD ? 0 : 10;
	}
	void SoundSystem::CleanupDirectShow() //52A220.
	{
		if (optionalShow)
		{
			optionalShow->Close();
			if (optionalShow) delete optionalShow;
			optionalShow = NULL;
		}
	}
	void SoundSystem::CleanupDirectMusic() //52A260.
	{
		if (optionalMusic)
		{
			optionalMusic->Close();
			if (optionalMusic) delete optionalMusic;
			optionalMusic = NULL;
		}
	}
	void SoundSystem::CleanupCD() //52A2A0.
	{
		if (optionalCD)
		{
			optionalCD->Close();
			if (optionalCD) delete optionalCD;
			optionalCD = NULL;
		}
	}
	void SoundSystem::Update()
	{
		now=timeGetTime();
		if (optionalCD) optionalCD->Update();
		if (optionalShow) optionalShow->Update();
		if (optionalMusic) optionalMusic->Update();
		if (instanceManager) instanceManager->Update();
		SoundGuard guard(soundLock);
		bool retire=false;
		for (std::list<CSound*>::iterator i=sounds.begin();i!=sounds.end();++i)
		{
			CSound* current=*i;
			current->Update();
			if (current->CanDelete()) retire=true;
		}
		if (retire)
		{
			for (;;)
			{
				std::list<CSound*>::iterator i=sounds.begin();
				CSound* ended;
				while (i!=sounds.end())
				{
					ended=*i;
					if (ended->CanDelete()) break;
					++i;
				}
				if (i==sounds.end()) break;
				sounds.erase(i);
				delete ended;
			}
		}
	}
	void SoundSystem::CleanupJumpList()
	{
		if (jumps)
		{
			jumps->Clear();
			CMasterJumplist* victim=jumps; //Reload after the child call.
			if (victim) { delete victim; }
			jumps=NULL;
		}
	}
	void SoundSystem::CleanupInstanceGroups()
	{
		if (instanceManager)
		{
			instanceManager->Cleanup();
			SoundInstanceManager* victim=instanceManager;
			if (victim) { delete victim; }
			instanceManager=NULL;
		}
		if (groups)
		{
			groups->Clear();
			MasterSoundGroups* victim=groups; //Reload after the child call.
			if (victim) { delete victim; }
			groups=NULL;
		}
	}
	int SoundSystem::LookupVolume(int volume) const
	{ return volumeTable[volume<0 ? 0 : volume>100 ? 100 : volume]; }
}

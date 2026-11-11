#include "SoundInstanceServices.hpp"
#include "SoundInstance.hpp"
#include "Sound.hpp"
#include "SoundPreset.hpp"
#include "SoundSystem.hpp"
#include "SoundInstanceManager.hpp"
#include "SoundGroups.hpp"
#include <mmsystem.h>
#include <cstdio>
#include <stdexcept>

namespace JE
{
	SoundPlaybackOwner::SoundPlaybackOwner(SoundInstanceManager* owner,
		SoundInstanceExternalServices* fixture,BYTE present) : owner(owner),fixture(fixture),present(present) {}
	int SoundPlaybackOwner::CountActiveGroup(LONG id)
	{ return owner ? owner->CountActiveGroup(id) : fixture->CountActiveGroup(id); }
	void SoundPlaybackOwner::EvictOldestGroup(LONG id,VfxSoundInstance& incoming)
	{ if (owner) owner->EvictOldestGroup(id,incoming); else fixture->EvictOldestGroup(id,incoming); }
	int SoundPlaybackOwner::MixVolume(VfxSoundInstance& incoming)
	{ return owner ? owner->MixVolume(incoming) : fixture->MixVolume(incoming); }
	SoundGroupOwner::SoundGroupOwner(MasterSoundGroups* owner,
		SoundInstanceExternalServices* fixture,BYTE present) : owner(owner),fixture(fixture),present(present) {}
	SoundGroup* SoundGroupOwner::FindGroup(const char* name,BYTE preset)
	{ return owner ? (preset ? owner->FindGroup(name) : owner->fallback) : fixture->FindGroup(name,preset); }
	SoundPlaybackOwner SoundInstanceExternalServices::CapturePlaybackOwner()
	{ return SoundPlaybackOwner(NULL,this,HasPlaybackOwner()); }
	SoundGroupOwner SoundInstanceExternalServices::CaptureGroupOwner()
	{ return SoundGroupOwner(NULL,this,HasGroupOwner()); }
	BYTE SoundInstanceExternalServices::HasPlaybackOwner() { return 0; }
	int SoundInstanceExternalServices::CountActiveGroup(LONG)
	{ throw std::logic_error("Sound manager52C760 requires actual instance registry"); }
	void SoundInstanceExternalServices::EvictOldestGroup(LONG,VfxSoundInstance&)
	{ throw std::logic_error("Sound manager52C7D0 requires actual instance registry"); }
	int SoundInstanceExternalServices::MixVolume(VfxSoundInstance&)
	{ throw std::logic_error("Sound manager52C890 requires actual sound groups"); }
	namespace
	{
		SoundInstanceExternalServices* currentServices=NULL;
		// Host compatibility only: no owner state or native SoundSystem base/slots.
		class NativeSoundInstanceServices : public SoundInstanceExternalServices
		{
		public:
			virtual Resources::ResourceSystem& ResourceOwner()
			{ return AudioBackend::g_System->ResourceOwner(); }
			virtual void RegisterInstance(VfxSoundInstance& instance)
			{ AudioBackend::g_System->RegisterInstance(instance); }
			virtual void UnregisterInstance(VfxSoundInstance& instance)
			{ AudioBackend::g_System->UnregisterInstance(instance); }
			virtual BYTE HasGroupOwner()
			{ return AudioBackend::g_System->HasGroupOwner(); }
			virtual BYTE HasPlaybackOwner()
			{ return AudioBackend::g_System->HasPlaybackOwner(); }
			virtual SoundPlaybackOwner CapturePlaybackOwner()
			{ return AudioBackend::g_System->CapturePlaybackOwner(); }
			virtual SoundGroupOwner CaptureGroupOwner()
			{ return AudioBackend::g_System->CaptureGroupOwner(); }
			virtual int CountActiveGroup(LONG groupID)
			{ return AudioBackend::g_System->CountActiveGroup(groupID); }
			virtual void EvictOldestGroup(LONG groupID,VfxSoundInstance& incoming)
			{ AudioBackend::g_System->EvictOldestGroup(groupID,incoming); }
			virtual int MixVolume(VfxSoundInstance& instance)
			{ return AudioBackend::g_System->MixVolume(instance); }
			virtual SoundGroup* FindGroup(const char* name,BYTE preset)
			{ return AudioBackend::g_System->FindGroup(name,preset); }
			virtual void ApplyGroup(CSound& sound,SoundGroup& group)
			{ AudioBackend::g_System->ApplyGroup(sound,group); }
			virtual int RequestPriority(VfxSoundInstance& instance)
			{ return AudioBackend::g_System->RequestPriority(instance); }
			virtual DWORD Clock()
			{ return AudioBackend::g_System->Clock(); }
			virtual void PlayInstance(VfxSoundInstance& instance,DWORD flags,DWORD delay)
			{ AudioBackend::g_System->PlayInstance(instance,flags,delay); }
			virtual void StopSound(CSound& sound,DWORD first,DWORD second)
			{ AudioBackend::g_System->StopSound(sound,first,second); }
			virtual void SetSoundVolume(CSound& sound,int volume,DWORD force)
			{ AudioBackend::g_System->SetSoundVolume(sound,volume,force); }
			virtual int SoundVolume(CSound& sound)
			{ return AudioBackend::g_System->SoundVolume(sound); }
			virtual void InitializeSound(CSound& sound)
			{ AudioBackend::g_System->InitializeSound(sound); }
			virtual void DestroySound(CSound& sound)
			{ AudioBackend::g_System->DestroySound(sound); }
			virtual void LoadSound(CSound& sound,const DSBUFFERDESC& description,const BYTE* bytes,DWORD size)
			{ AudioBackend::g_System->LoadSound(sound,description,bytes,size); }
		};
	}
	SoundInstanceExternalServices* SetSoundInstanceServices(SoundInstanceExternalServices* services)
	{
		SoundInstanceExternalServices* previous=currentServices; currentServices=services; return previous;
	}
	SoundInstanceExternalServices& GetSoundInstanceServices()
	{
		if (currentServices) return *currentServices;
		if (AudioBackend::g_System)
		{
			static NativeSoundInstanceServices native;
			return native;
		}
		throw std::logic_error("Actual SoundSystem registry/playback remains unrecovered; no sound service is bound");
	}
	namespace SoundInstanceServices
	{
		SoundSystem* PlaybackDevice()
		{
			SoundSystem* system=AudioBackend::g_System;
			return system && !currentServices ? system : NULL;
		}
		int MasterVolume()
		{ return AudioBackend::g_System ? AudioBackend::g_System->masterVolume : 100; }
		int VolumeToDecibels(int volume)
		{
			if (!AudioBackend::g_System) throw std::logic_error("SoundSystem52A1F0 requires actual volume table");
			return AudioBackend::g_System->LookupVolume(volume);
		}
		Resources::ResourceSystem& ResourceOwner() { return GetSoundInstanceServices().ResourceOwner(); }
		int RequestPriority(VfxSoundInstance& instance) { return GetSoundInstanceServices().RequestPriority(instance); }
		void InitializeSound(CSound& sound) { GetSoundInstanceServices().InitializeSound(sound); }
		void DestroySound(CSound& sound) { GetSoundInstanceServices().DestroySound(sound); }
		void LoadSound(CSound& sound,const DSBUFFERDESC& description,const BYTE* bytes,DWORD size)
		{ GetSoundInstanceServices().LoadSound(sound,description,bytes,size); }
	}
	NonPlayingSoundInstanceServices::NonPlayingSoundInstanceServices(Resources::ResourceSystem& owner)
		: registeredInstances(0),unregisteredInstances(0),initializedSounds(0),loadedSounds(0),
		unconsumedPlayRequests(0),unconsumedStopRequests(0),unconsumedMixingRequests(0),
		resources(owner),previous(SetSoundInstanceServices(this))
	{
		puts("Temporary non-playing audio service: real resources/data, unconsumed registry/mixer/device; no SoundSystem or playback success");
	}
	NonPlayingSoundInstanceServices::~NonPlayingSoundInstanceServices()
	{
		//A real fire-and-forget request drops its VFX pointer. The temporary
		//registry retains that allocation until this explicit host cleanup,
		//rather than claiming the missing audio worker completed its lifetime.
		if (!instances.empty() || !sounds.empty())
			std::printf("Temporary non-playing cleanup: %u outer instances, %u sound files retained after original owners\n",
				unsigned(instances.size()),unsigned(sounds.size()));
		while (!instances.empty()) delete instances.front();
		while (!sounds.empty()) delete sounds.front();
		SetSoundInstanceServices(previous);
	}
	Resources::ResourceSystem& NonPlayingSoundInstanceServices::ResourceOwner() { return resources; }
	void NonPlayingSoundInstanceServices::RegisterInstance(VfxSoundInstance& instance)
	{ instances.push_back(&instance); ++registeredInstances; }
	void NonPlayingSoundInstanceServices::UnregisterInstance(VfxSoundInstance& instance)
	{ instances.remove(&instance); ++unregisteredInstances; }
	BYTE NonPlayingSoundInstanceServices::HasGroupOwner() { ++unconsumedMixingRequests; return 0; }
	SoundGroup* NonPlayingSoundInstanceServices::FindGroup(const char*,BYTE)
	{ ++unconsumedMixingRequests; return NULL; }
	void NonPlayingSoundInstanceServices::ApplyGroup(CSound&,SoundGroup&)
	{ throw std::logic_error("Temporary non-playing service has no actual master sound-group object"); }
	int NonPlayingSoundInstanceServices::RequestPriority(VfxSoundInstance&)
	{ ++unconsumedMixingRequests; return 0; }
	DWORD NonPlayingSoundInstanceServices::Clock() { return timeGetTime(); }
	void NonPlayingSoundInstanceServices::PlayInstance(VfxSoundInstance& instance,DWORD flags,DWORD delay)
	{
		++unconsumedPlayRequests;
		std::printf("Unconsumed audio Play52CCA0: %s flags%lu delay%lu, %u parsed sound entries; no voice\n",
			instance.resourceName.c_str(),flags,delay,unsigned(instance.preset->entries.size()));
	}
	void NonPlayingSoundInstanceServices::StopSound(CSound&,DWORD first,DWORD second)
	{
		++unconsumedStopRequests;
		std::printf("Unconsumed audio Stop4C79E0(%lu,%lu); no voice\n",first,second);
	}
	void NonPlayingSoundInstanceServices::SetSoundVolume(CSound&,int,DWORD) { ++unconsumedMixingRequests; }
	int NonPlayingSoundInstanceServices::SoundVolume(CSound&) { ++unconsumedMixingRequests; return 0; }
	void NonPlayingSoundInstanceServices::InitializeSound(CSound& sound)
	{
		//Captured original52ACF0 startup tuple/master volume, explicitly input
		//to this stub, not proof of a running or completely constructed backend.
		sound.bitsPerSample=16; sound.sampleRate=22050; sound.channels=2; sound.effectiveVolume=100;
		sounds.push_back(&sound); ++initializedSounds;
	}
	void NonPlayingSoundInstanceServices::DestroySound(CSound& sound) { sounds.remove(&sound); }
	void NonPlayingSoundInstanceServices::LoadSound(CSound& sound,const DSBUFFERDESC& description,
		const BYTE* sampleBytes,DWORD sampleSize)
	{
		++loadedSounds;
		const WAVEFORMATEX* format=description.lpwfxFormat;
		std::printf("Unconsumed audio buffer: %s, %lu sample bytes requested through actual resource, buffer%lu flags%08lX, format%u/%lu/%u; no COM buffer\n",
			sound.resourceName.c_str(),sampleSize,description.dwBufferBytes,description.dwFlags,
			format ? unsigned(format->nChannels) : 0,format ? format->nSamplesPerSec : 0,
			format ? unsigned(format->wBitsPerSample) : 0);
		//Only observe the actual borrowed bytes during this callback. Never
		//retain caller storage, construct a fake buffer, or claim it played.
		(void)sampleBytes;
	}
}

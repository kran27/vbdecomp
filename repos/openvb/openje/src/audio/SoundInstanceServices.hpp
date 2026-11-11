#pragma once

#include <windows.h>
#include <list>

typedef struct _DSBUFFERDESC DSBUFFERDESC;

namespace JE
{
	class CSound;
	class VfxSoundInstance;
	class SoundGroup;
	class MasterSoundGroups;
	class SoundInstanceManager;
	class SoundInstanceExternalServices;
	//Source API adapters retain an original captured child or an explicit fixture provider.
	class SoundPlaybackOwner
	{
	public:
		SoundPlaybackOwner(SoundInstanceManager* owner,SoundInstanceExternalServices* fixture,BYTE present);
		BYTE HasOwner() const { return present; }
		int CountActiveGroup(LONG id);
		void EvictOldestGroup(LONG id,VfxSoundInstance& incoming);
		int MixVolume(VfxSoundInstance& incoming);
	private:
		SoundInstanceManager* owner;
		SoundInstanceExternalServices* fixture;
		BYTE present;
	};
	class SoundGroupOwner
	{
	public:
		SoundGroupOwner(MasterSoundGroups* owner,SoundInstanceExternalServices* fixture,BYTE present);
		BYTE HasOwner() const { return present; }
		SoundGroup* FindGroup(const char* name,BYTE preset);
	private:
		MasterSoundGroups* owner;
		SoundInstanceExternalServices* fixture;
		BYTE present;
	};
	class SoundSystem;
	namespace Resources { class ResourceSystem; }

	// Borrowed sound-system APIs; defaults to SoundSystem.

	class SoundInstanceExternalServices
	{
	public:
		virtual ~SoundInstanceExternalServices() {}
		virtual Resources::ResourceSystem& ResourceOwner() = 0;
		virtual void RegisterInstance(VfxSoundInstance& instance) = 0;
		virtual void UnregisterInstance(VfxSoundInstance& instance) = 0;
		virtual BYTE HasGroupOwner() = 0;
		virtual BYTE HasPlaybackOwner();
		virtual SoundPlaybackOwner CapturePlaybackOwner();
		virtual SoundGroupOwner CaptureGroupOwner();
		virtual int CountActiveGroup(LONG groupID);
		virtual void EvictOldestGroup(LONG groupID,VfxSoundInstance& incoming);
		virtual int MixVolume(VfxSoundInstance& instance);
		virtual SoundGroup* FindGroup(const char* name,BYTE preset) = 0;
		virtual void ApplyGroup(CSound& sound,SoundGroup& group) = 0;
		virtual int RequestPriority(VfxSoundInstance& instance) = 0;
		virtual DWORD Clock() = 0;
		virtual void PlayInstance(VfxSoundInstance& instance,DWORD flags,DWORD delay) = 0;
		virtual void StopSound(CSound& sound,DWORD first,DWORD second) = 0;
		virtual void SetSoundVolume(CSound& sound,int volume,DWORD time) = 0;
		virtual int SoundVolume(CSound& sound) = 0;
		virtual void InitializeSound(CSound& sound) = 0;
		virtual void DestroySound(CSound& sound) = 0;
		virtual void LoadSound(CSound& sound,const DSBUFFERDESC& description,
			const BYTE* sampleBytes,DWORD sampleSize) = 0;
	};

	SoundInstanceExternalServices* SetSoundInstanceServices(SoundInstanceExternalServices* services);
	SoundInstanceExternalServices& GetSoundInstanceServices();
	namespace SoundInstanceServices
	{
		Resources::ResourceSystem& ResourceOwner();
		int RequestPriority(VfxSoundInstance& instance); //Actual manager52C480 boundary.
		void InitializeSound(CSound& sound);
		void DestroySound(CSound& sound);
		void LoadSound(CSound& sound,const DSBUFFERDESC& description,
			const BYTE* sampleBytes,DWORD sampleSize);
		SoundSystem* PlaybackDevice();
		int MasterVolume();
		int VolumeToDecibels(int volume);
	}

	// User-authorized temporary playback stub. It binds itself for its lifetime,
	// borrows the real ResourceSystem, and never creates a SoundSystem or COM
	// buffer. Queries/admission report no playback; no duration/success is faked.
	class NonPlayingSoundInstanceServices : public SoundInstanceExternalServices
	{
	public:
		explicit NonPlayingSoundInstanceServices(Resources::ResourceSystem& resources);
		virtual ~NonPlayingSoundInstanceServices();
		virtual Resources::ResourceSystem& ResourceOwner();
		virtual void RegisterInstance(VfxSoundInstance& instance);
		virtual void UnregisterInstance(VfxSoundInstance& instance);
		virtual BYTE HasGroupOwner();
		virtual SoundGroup* FindGroup(const char* name,BYTE preset);
		virtual void ApplyGroup(CSound& sound,SoundGroup& group);
		virtual int RequestPriority(VfxSoundInstance& instance);
		virtual DWORD Clock();
		virtual void PlayInstance(VfxSoundInstance& instance,DWORD flags,DWORD delay);
		virtual void StopSound(CSound& sound,DWORD first,DWORD second);
		virtual void SetSoundVolume(CSound& sound,int volume,DWORD time);
		virtual int SoundVolume(CSound& sound);
		virtual void InitializeSound(CSound& sound);
		virtual void DestroySound(CSound& sound);
		virtual void LoadSound(CSound& sound,const DSBUFFERDESC& description,
			const BYTE* sampleBytes,DWORD sampleSize);
		DWORD registeredInstances,unregisteredInstances,initializedSounds,loadedSounds;
		DWORD unconsumedPlayRequests,unconsumedStopRequests,unconsumedMixingRequests;
	private:
		Resources::ResourceSystem& resources;
		SoundInstanceExternalServices* previous;
		std::list<VfxSoundInstance*> instances;
		std::list<CSound*> sounds;
		NonPlayingSoundInstanceServices(const NonPlayingSoundInstanceServices&);
		NonPlayingSoundInstanceServices& operator=(const NonPlayingSoundInstanceServices&);
	};
}

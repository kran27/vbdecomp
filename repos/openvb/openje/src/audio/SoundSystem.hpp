#pragma once

#include "resources/ResourceLock.hpp"
#include <dsound.h>
#include <list>
#include <d3dx8.h>

namespace JE
{
	class CSound;
	namespace Resources { class ResourceSystem; }
	class MasterSoundGroups;
	class CMasterJumplist;
	class DirectShowAudio;
	class DirectMusicAudio;
	class CDAudio;
	class SoundInstanceManager;
	class SoundSystem
	{
	public:
		SoundSystem(); //52ABF0, sound lock; the instance manager owns its own lock.
		virtual ~SoundSystem(); //52AC70: list nodes/lock only, not global shutdown.
		void AttachResources(Resources::ResourceSystem* owner); //52A010.
		int InitializeDevice(HWND window,DWORD cooperativeLevel,DWORD channels,
			DWORD sampleRate,DWORD bitsPerSample); //52A2E0, ECX is this owner.
		int PrepareScratch3D(); //52A020, same owner/no formal arguments.
		void ResetDefaults(); //52AB30.
		void BuildVolumeTable(); //52A190.
		void InitializeGroups(const char* name); //52A460, groups then actual instance manager.
		void InitializeJumpList(const char* name); //52A130.
		int InitializeDirectShow(HWND window); //52A5F0, window argument unused.
		int InitializeDirectMusic(HWND window); //52A6C0.
		int InitializeCD(); //52A790.
		void CleanupDirectShow(); //52A220.
		void CleanupDirectMusic(); //52A260.
		void CleanupCD(); //52A2A0.
		void CleanupJumpList(); //52A5B0.
		void CleanupInstanceGroups(); //52A540, instance manager before groups.
		void CleanupDevice(); //52A860, captures this throughout.
		void ClearSoundNodes(); //52ABC0, borrowed elements are not deleted.
		void RegisterSound(CSound* sound); //52AEC0, borrowed pointer value under the sound lock.
		void UnregisterSound(CSound* sound); //52A930, pointer argument cell passed to list removal.
		void Update(); //52AB10/52A960.
		int LookupVolume(int volume) const; //52A1F0.

		// Observed native owner prefix. Occupied scalar cells carry their actual
		// stored width; unresolved names do not imply padding or nominal types.
		IDirectSound8* device; //+04.
		IDirectSoundBuffer* primary; //+08.
		IDirectSound3DListener* listener; //+0C.
		IDirectSoundBuffer* scratch; //+10.
		IKsPropertySet* scratchProperties; //+14, actual IID_IKsPropertySet.
		DWORD value18; //+18, observed zero store; meaning/original nominal type unresolved.
		DWORD bitsPerSample, sampleRate, channels; //+1C/+20/+24.
		LONG masterVolume; //+28.
		std::list<CSound*> sounds; //+2C, borrowed sound nodes.
		DWORD enable3D; //+38.
		DWORD flag3C, flag40, flag44; //Observed32-bit flags; semantic names unresolved.
		BYTE initialized; //+48.
		DWORD now; //+4C.
		Resources::ResourceSystem* resources; //+50.
		Resources::ResourceLock soundLock; //+54, actual4-byte lock owner.
		LONG volumeTable[101]; //+58..1E8.
		MasterSoundGroups* groups; //+1EC.
		SoundInstanceManager* instanceManager; //+1F0, separately owned.
		CMasterJumplist* jumps; //+1F4.
		DirectShowAudio* optionalShow; //+1F8.
		DirectMusicAudio* optionalMusic; //+1FC.
		CDAudio* optionalCD; //+200, end of observed prefix at204.
	private:
		SoundSystem(const SoundSystem&);
		SoundSystem& operator=(const SoundSystem&);
	};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char SoundOwnerLockSize[(sizeof(Resources::ResourceLock)==4)?1:-1];
	typedef char SoundOwner_device[(offsetof(SoundSystem,device)==0x4)?1:-1];
	typedef char SoundOwner_primary[(offsetof(SoundSystem,primary)==0x8)?1:-1];
	typedef char SoundOwner_listener[(offsetof(SoundSystem,listener)==0xC)?1:-1];
	typedef char SoundOwner_scratch[(offsetof(SoundSystem,scratch)==0x10)?1:-1];
	typedef char SoundOwner_scratchProperties[(offsetof(SoundSystem,scratchProperties)==0x14)?1:-1];
	typedef char SoundOwner_value18[(offsetof(SoundSystem,value18)==0x18)?1:-1];
	typedef char SoundOwner_bitsPerSample[(offsetof(SoundSystem,bitsPerSample)==0x1C)?1:-1];
	typedef char SoundOwner_sampleRate[(offsetof(SoundSystem,sampleRate)==0x20)?1:-1];
	typedef char SoundOwner_channels[(offsetof(SoundSystem,channels)==0x24)?1:-1];
	typedef char SoundOwner_masterVolume[(offsetof(SoundSystem,masterVolume)==0x28)?1:-1];
	typedef char SoundOwner_sounds[(offsetof(SoundSystem,sounds)==0x2C)?1:-1];
	typedef char SoundOwner_enable3D[(offsetof(SoundSystem,enable3D)==0x38)?1:-1];
	typedef char SoundOwner_flag3C[(offsetof(SoundSystem,flag3C)==0x3C)?1:-1];
	typedef char SoundOwner_flag40[(offsetof(SoundSystem,flag40)==0x40)?1:-1];
	typedef char SoundOwner_flag44[(offsetof(SoundSystem,flag44)==0x44)?1:-1];
	typedef char SoundOwner_initialized[(offsetof(SoundSystem,initialized)==0x48)?1:-1];
	typedef char SoundOwner_now[(offsetof(SoundSystem,now)==0x4C)?1:-1];
	typedef char SoundOwner_resources[(offsetof(SoundSystem,resources)==0x50)?1:-1];
	typedef char SoundOwner_soundLock[(offsetof(SoundSystem,soundLock)==0x54)?1:-1];
	typedef char SoundOwner_volumeTable[(offsetof(SoundSystem,volumeTable)==0x58)?1:-1];
	typedef char SoundOwner_groups[(offsetof(SoundSystem,groups)==0x1EC)?1:-1];
	typedef char SoundOwner_instanceManager[(offsetof(SoundSystem,instanceManager)==0x1F0)?1:-1];
	typedef char SoundOwner_jumps[(offsetof(SoundSystem,jumps)==0x1F4)?1:-1];
	typedef char SoundOwner_optionalShow[(offsetof(SoundSystem,optionalShow)==0x1F8)?1:-1];
	typedef char SoundOwner_optionalMusic[(offsetof(SoundSystem,optionalMusic)==0x1FC)?1:-1];
	typedef char SoundOwner_optionalCD[(offsetof(SoundSystem,optionalCD)==0x200)?1:-1];
	typedef char SoundOwner_nativeExtent[(sizeof(SoundSystem)==0x204)?1:-1];
#endif

	namespace AudioBackend
	{
		extern SoundSystem* g_System; // Original singleton pointer 0x70B250.
		SoundSystem* GetSystem(); //52A000, returns the published owner without a guard.
		void InitializeBackend(HWND window,Resources::ResourceSystem* resources,
			const char* groups,const char* jumps,bool directShow,bool directMusic,bool cdAudio); //52ACF0.
		void UpdateBackend();
		void ShutdownBackend();
	}
}

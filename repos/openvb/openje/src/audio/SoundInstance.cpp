#include "SoundInstance.hpp"
#include "SoundSystem.hpp"
#include "SoundInstanceManager.hpp"
#include "SoundInstanceServices.hpp"
#include "SoundPreset.hpp"
#include "SoundGroups.hpp"
#include "Sound.hpp"
#include <cstdio>

namespace JE
{
	VfxSoundInstance::VfxSoundInstance() : position(0,0,0)
	{
		resourceName = "";
		preset = NULL;
		group = NULL;
		startedAt = 0;
		spatial = 0;
		position.x = 0; position.y = 0; position.z = 0;
		sequenceIndex = -1;
		loadFlags = 0;
		flags = 0;
		AudioBackend::g_System->instanceManager->RegisterInstance(this); //52E260, last ctor child.
	}
	VfxSoundInstance::~VfxSoundInstance()
	{
		AudioBackend::g_System->instanceManager->UnregisterInstance(this); //52CC70 before preset destruction.
		if (preset) { delete preset; preset=NULL; }
	}
	int VfxSoundInstance::Load(const char* name,DWORD requestedFlags,const char* overrideName)
	{
		MasterSoundGroups* groups=AudioBackend::g_System->groups; //52D85E, before preset replacement.
		if (preset) { delete preset; preset=NULL; }
		preset=new SoundPresetInfo;
		loadFlags=requestedFlags;
		resourceName=name;
		if (IsSoundPreset(resourceName.c_str()))
		{
			if (!preset->Read(resourceName.c_str()))
			{
				if (groups) group=groups->FindGroup(resourceName.c_str());
			}
			else
			{
				char message[256];
				std::sprintf(message,"Warning - Could not load preset file, %s.\n",resourceName.c_str());
				OutputDebugStringA(message);
				std::string filename=resourceName.substr(0,resourceName.find_last_of('.'));
				filename.append(".wav");
				SoundFileEntry* entry=new SoundFileEntry;
				entry->name=filename;
				preset->entries.push_back(entry);
				if (groups) group=groups->fallback;
			}
		}
		else
		{
			SoundFileEntry* entry=new SoundFileEntry;
			entry->name=resourceName;
			preset->entries.push_back(entry);
			if (groups) group=groups->fallback;
		}
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
		{
			SoundFileEntry* entry=*file;
			if (overrideName) entry->name=overrideName;
			entry->sound=new CSound;
			entry->sound->Load(entry->name.c_str(),requestedFlags); //Original ignores the child result.
			if (group)
			{
				entry->sound->SetMixFadeInDuration(group->fadeIn);
				entry->sound->SetMixFadeOutDuration(group->fadeOut);
			}
		}
		return 0; //52DBB7, even if an individual resource Load returned an error.
	}
	D3DXVECTOR3* VfxSoundInstance::GetPosition(D3DXVECTOR3* output) const { output->x=position.x; output->y=position.y; output->z=position.z; return output; }
	void VfxSoundInstance::SetPosition(D3DXVECTOR3 value)
	{
		position.x=value.x; position.y=value.y; position.z=value.z;
	}
	int VfxSoundInstance::HasActiveSound()
	{
		if (sequenceIndex>-1) return 1;
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
			if (*file && (*file)->sound && ((*file)->sound->flags&4)) return 1;
		return 0;
	}
	int VfxSoundInstance::HasPendingSound()
	{
		int pending=0;
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
		{
			if (*file && (*file)->sound)
			{
				DWORD status=(*file)->sound->flags;
				if (status&4) return 0;
				if (status&0x1000) pending=1;
			}
		}
		return pending;
	}
	DWORD VfxSoundInstance::GetStartedAt() const
	{
		return startedAt;
	}

	DWORD VfxSoundInstance::IsPlaying()
	{
		if (HasActiveSound()) return 1;
		return HasPendingSound()!=0;
	}
	int VfxSoundInstance::Stop(DWORD value)
	{
		sequenceIndex=-1;
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
			if (*file && (*file)->sound) (*file)->sound->Stop(value);
		return 0;
	}
	int VfxSoundInstance::Retire()
	{
		sequenceIndex=-1;
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
			if (*file && (*file)->sound) (*file)->sound->Stop(0);
		return 0;
	}
	int VfxSoundInstance::CanAutoDelete()
	{
		if ((flags&2) && !(flags&1) && !HasActiveSound() && !HasPendingSound())
			for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
				if (*file && (*file)->sound && ((*file)->sound->flags&0x48)) return 1;
		return 0;
	}
	int VfxSoundInstance::Priority()
	{
		int maximum=0;
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
			if (*file && (*file)->sound)
			{
				int value=static_cast<LONG>((*file)->sound->GetRequestedVolume());
				if (value>maximum) maximum=value;
			}
		return maximum;
	}
	int VfxSoundInstance::SetPriority(int value)
	{
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
			if (*file && (*file)->sound) (*file)->sound->SetVolume(value,0);
		return 0;
	}
	int VfxSoundInstance::PlayEntry(SoundFileEntry* entry,DWORD entryFlags,DWORD fadeIn)
	{
		SoundInstanceManager* owner=AudioBackend::g_System->instanceManager;
		if (!owner) return 1;
		if (!entry || !entry->sound) return 2;
		CSound& sound=*entry->sound;
		sound.SetBasePitch(entry->basePitch);
		int pitchOffset=0;
		if (entry->randomPitch) pitchOffset=Random(-static_cast<int>(entry->randomPitch),entry->randomPitch);
		if (group->pitch!=100) pitchOffset+=group->pitch-100;
		sound.SetPitchOffset(pitchOffset);
		sound.SetBaseVolume(entry->baseVolume);
		int volumeOffset=0;
		if (entry->randomVolume) volumeOffset=Random(-static_cast<int>(entry->randomVolume),entry->randomVolume);
		if (group->volume!=100) volumeOffset+=group->volume-100;
		sound.SetVolumeOffset(volumeOffset);
		sound.SetMixVolume(owner->MixVolume(*this));
		int delay=entry->baseOffset;
		if (static_cast<LONG>(entry->randomOffset)>0) delay+=Random(0,entry->randomOffset);
		return sound.Play(entryFlags,fadeIn,delay);
	}
	int VfxSoundInstance::Play(DWORD requestedFlags,DWORD fadeIn)
	{
		SoundInstanceManager* owner=AudioBackend::g_System->instanceManager;
		if (!owner) return 1;
		if (!preset || preset->entries.empty()) return 2;
		if (!HasActiveSound() && !HasPendingSound())
		{
			flags=requestedFlags;
			DWORD entryFlags=requestedFlags&~2u;
			SoundGroup* admissionGroup=group;
			const int activeCount=owner->CountActiveGroup(admissionGroup->id);
			if (activeCount>=admissionGroup->voiceLimit)
				owner->EvictOldestGroup(admissionGroup->id,*this);
			startedAt=AudioBackend::g_System->now;
			if (preset->sequencing==0)
			{
				for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
					PlayEntry(*file,entryFlags,fadeIn);
			}
			else if (preset->sequencing==1)
			{
				sequenceIndex=0;
				if (requestedFlags&1) entryFlags&=~1u;
				PlayEntry(preset->entries.front(),entryFlags,fadeIn);
			}
			else if (preset->sequencing==2)
			{
				int selected=Random(1,static_cast<int>(preset->entries.size()));
				int index=1;
				for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file,++index)
					if (index==selected)
					{
						if (requestedFlags&1)
						{
							entryFlags&=~1u;
							sequenceIndex=index-1;
						}
						PlayEntry(*file,entryFlags,fadeIn);
						break;
					}
			}
			if (spatial) SetPriority(AudioBackend::g_System->instanceManager->RequestPriority(*this));
		}
		return 0;
	}
	void VfxSoundInstance::Update()
	{
		if (!preset) return;
		if (spatial) SetPriority(AudioBackend::g_System->instanceManager->RequestPriority(*this));
		if (sequenceIndex>=0)
		{
			SoundFileEntry* current=NULL;
			SoundFileEntry* next=NULL;
			int index=0;
			for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file,++index)
			{
				if (index==sequenceIndex) current=*file;
				if (index==sequenceIndex+1) { next=*file; break; }
			}
			if (current && current->sound)
			{
				if (current->sound->flags&(4|0x1000)) goto RefreshVolumes;
				if (preset->sequencing==1)
				{
					if (next)
					{
						PlayEntry(next,0,0);
						++sequenceIndex;
						goto RefreshVolumes;
					}
					if (flags&1)
					{
						PlayEntry(preset->entries.front(),0,0);
						sequenceIndex=0;
						goto RefreshVolumes;
					}
				}
				else
				{
					if (preset->sequencing!=2) goto RefreshVolumes;
					if (flags&1)
					{
						int selected=Random(1,static_cast<int>(preset->entries.size()));
						int selectedIndex=1;
						for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file,++selectedIndex)
							if (selectedIndex==selected)
							{
								PlayEntry(*file,0,0);
								sequenceIndex=selectedIndex-1;
								break;
							}
						goto RefreshVolumes;
					}
				}
			}
			sequenceIndex=-1;
		}
	RefreshVolumes:
		for (std::list<SoundFileEntry*>::iterator file=preset->entries.begin();file!=preset->entries.end();++file)
			if (*file && (*file)->sound)
			{
				CSound& sound=*(*file)->sound;
				if (sound.flags&4) sound.SetMixVolume(AudioBackend::g_System->instanceManager->MixVolume(*this));
				else if (sound.flags&0x1000)
				{
					startedAt=AudioBackend::g_System->now;
					sound.SetMixVolume(AudioBackend::g_System->instanceManager->MixVolume(*this));
				}
			}
	}
}

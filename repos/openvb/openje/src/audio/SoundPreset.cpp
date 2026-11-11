#include "SoundPreset.hpp"
#include "SoundInstance.hpp"
#include "Sound.hpp"
#include <cstdlib>

namespace JE
{
	SoundFileEntry::SoundFileEntry() : name()
	{
		name = "";
		sound = NULL;
		baseVolume = 80;
		randomVolume = 0;
		basePitch = 100;
		randomPitch = 0;
		baseOffset = 0;
		randomOffset = 0;
	}

	SoundPresetInfo::SoundPresetInfo() : sequencing(0), fullSoundRadius(40), noSoundRadius(60)
	{
		entries.clear();
	}

	SoundPresetInfo::~SoundPresetInfo()
	{
		for (std::list<SoundFileEntry*>::iterator it = entries.begin(); it != entries.end(); ++it)
		{
			SoundFileEntry* entry = *it;
			if (entry)
			{
				if (entry->sound)
				{
					delete entry->sound;
					entry->sound = NULL;
				}
				delete entry;
				*it = NULL;
			}
		}
		entries.clear();
	}

	void SoundPresetInfo::Parse(const BYTE* input, DWORD size)
	{
		data = input;
		dataSize = size;
		cursor = 0;
		char token[32];
		char filename[32];
		ReadToken(token, 32);
		ReadToken(token, 32);
		sequencing = std::atol(token);
		ReadToken(token, 32);
		ReadToken(token, 32);
		fullSoundRadius = std::atol(token);
		ReadToken(token, 32);
		ReadToken(token, 32);
		noSoundRadius = std::atol(token);
		ReadToken(filename, 32);
		ReadToken(token, 32);
		while (filename[0])
		{
			SoundFileEntry* entry = new SoundFileEntry;
			entry->name.assign(filename);
			ReadToken(token, 32);
			ReadToken(token, 32);
			entry->baseVolume = std::atol(token);
			ReadToken(token, 32);
			ReadToken(token, 32);
			entry->randomVolume = std::atol(token);
			ReadToken(token, 32);
			ReadToken(token, 32);
			entry->basePitch = std::atol(token);
			ReadToken(token, 32);
			ReadToken(token, 32);
			entry->randomPitch = std::atol(token);
			ReadToken(token, 32);
			ReadToken(token, 32);
			entry->baseOffset = std::atol(token);
			ReadToken(token, 32);
			ReadToken(token, 32);
			entry->randomOffset = std::atol(token);
			ReadToken(token, 32);
			entries.push_back(entry);
			ReadToken(filename, 32);
			ReadToken(token, 32);
		}
	}

	BOOL VfxSoundInstance::IsSoundPreset(const char* name)
	{
		char extension[3];
		int count = 0;
		bool foundDot = false;
		if (!*name)
			return FALSE;
		for (;;)
		{
			if (foundDot)
			{
				extension[count++] = *name;
				if (count == 3)
					break;
			}
			if (*name == '.')
				foundDot = true;
			if (!*++name)
				return FALSE;
		}
		return (extension[0] == 'P' || extension[0] == 'p')
			&& (extension[1] == 'S' || extension[1] == 's')
			&& (extension[2] == 'F' || extension[2] == 'f');
	}
}

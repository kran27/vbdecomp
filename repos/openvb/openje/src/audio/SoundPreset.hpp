#pragma once

#include "SoundFile.hpp"
#include <list>

namespace JE
{
	class CSound;

	struct SoundFileEntry
	{
		SoundFileEntry(); // 52D4C0.
		std::string name;
		DWORD baseVolume;
		DWORD randomVolume;
		DWORD basePitch;
		DWORD randomPitch;
		DWORD baseOffset;
		DWORD randomOffset;
		CSound* sound;
	};

	class SoundPresetInfo : public SoundFile
	{
	public:
		SoundPresetInfo(); // 52CEF0.
		~SoundPresetInfo(); // 52CF80, explicitly owns entry sounds.
		virtual void Parse(const BYTE* input, DWORD size); // 52D550.

		std::list<SoundFileEntry*> entries;
		DWORD sequencing;
		DWORD fullSoundRadius;
		DWORD noSoundRadius;
	};

}

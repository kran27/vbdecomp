#pragma once

#include "SoundFile.hpp"
#include <list>

namespace JE
{
	class SoundGroup : public SoundFile
	{
	public:
		SoundGroup(); //52DBF0.
		~SoundGroup();
		virtual void Parse(const BYTE* input,DWORD size); //52DD00.
		int Contains(const char* name) const; //52D290, case-insensitive preset membership.

		LONG voiceLimit; //+2C.
		DWORD fadeIn,fadeOut; //+30/+34, default voice fade durations.
		LONG volume,pitch; //+38/+3C, percentages.
		LONG fullSoundRadius,noSoundRadius; //+40/+44.
		LONG minimumVolume; //+48, lower bound for group interaction attenuation.
		std::list<std::string> presets; //+4C.
		LONG id,matrixIndex; //+58/+5C.
	private:
		SoundGroup(const SoundGroup&);
		SoundGroup& operator=(const SoundGroup&);
	};

	class MasterSoundGroups : public SoundFile
	{
	public:
		MasterSoundGroups(); //52DFB0.
		~MasterSoundGroups();
		virtual void Parse(const BYTE* input,DWORD size); //52E040.
		SoundGroup* FindGroup(const char* name); //52D360; callers select fallback for raw files.
		LONG MixingAmount(LONG source,LONG target) const; //52C420.
		void Clear(); //52E220, leaves fallback alive.

		SoundGroup* groups[16]; //+2C.
		LONG count; //+6C.
		LONG mixing[16][16]; //+70.
		SoundGroup* fallback; //+470.
	private:
		MasterSoundGroups(const MasterSoundGroups&);
		MasterSoundGroups& operator=(const MasterSoundGroups&);
	};

	int __stdcall SoundRandom(int minimum,int maximum); //52C390, inclusive real CRT rand.
}

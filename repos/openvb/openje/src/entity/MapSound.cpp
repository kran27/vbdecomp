#include "MapSound.hpp"
#include "Map.hpp"
#include "audio/SoundInstance.hpp"
#include "graphics/StringFormat.hpp"
#include <mbctype.h>
#include <mbstring.h>
#include <cstring>

namespace JE
{
	char UpperMapSoundByte(char value) //4A11E0, actual call at4A1526.
	{
		if (static_cast<unsigned char>(value) < 0x80u && _ismbclower(value))
			return char(_mbctoupper(value));
		return value;
	}
	namespace
	{
		int UpperMapSoundBytes(char* value) //4A1500, recomputes C-string length.
		{
			int i;
			for (i = 0; i < int(strlen(value)); ++i)
				value[i] = UpperMapSoundByte(value[i]);
			return i;
		}
	}
	std::string& __cdecl UpperString(std::string& value) //4A15A0.
	{
		if (value.length() > 0)
		{
			char* buffer = new char[value.length() + 1];
			strcpy(buffer, value.c_str());
			UpperMapSoundBytes(buffer);
			value.assign(buffer, strlen(buffer));
			delete[] buffer;
		}
		return value;
	}
	void Map::FindSounds(const std::string& name, std::list<MapSound*>& output) //574690.
	{
		std::string wanted = name;
		UpperString(wanted);
		for (std::map<std::string,MapSound*>::iterator i = mapSounds.begin(); i != mapSounds.end(); ++i)
		{
			std::string key = i->first;
			UpperString(key);
			if (key.find(wanted) != std::string::npos)
			{
				MapSound* sound = i->second;
				output.push_back(sound);
			}
		}
	}

	MapSound::MapSound() : position(0,0,0), looping(1), enabled(1), sound(NULL) {}
	void MapSound::Clear()
	{
		if (sound)
		{
			delete sound;
			sound = NULL;
		}
	}
	void MapSound::Start()
	{
		if (sound)
			sound->Play(looping != 0, 0);
	}
	void MapSound::Stop(LONG argument)
	{
		if (sound)
			sound->Stop(static_cast<DWORD>(argument));
	}

}

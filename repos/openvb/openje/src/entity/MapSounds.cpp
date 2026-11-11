#include "Map.hpp"
#include "MapSound.hpp"
#include "resources/ResourceSystem.hpp"
#include "graphics/StringFormat.hpp"
#include "model/CommonException.hpp"
#include "audio/SoundInstance.hpp"
#include "audio/SoundSystem.hpp"
#include "audio/SoundInstanceManager.hpp"
#include <cctype>
#include <stdexcept>

namespace JE
{
	MapSound::~MapSound()
	{ Clear(); }

	BYTE Map::ReadSound(Common::Chunk&, Resources::ResourceMemoryStream* stream)
	{
		std::string name;
		D3DXVECTOR3 position(0,0,0);
		std::string resource;
		BYTE rawLooping, enabled;
		bool looping;
		if (!Common::ReadString(name, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&position.x, 4, stream)) return 0;
		if (!Resources::g_pResourceSystem->ReadMemory(&position.y, 4, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&position.z, 4, stream)
			|| !Common::ReadString(resource, stream)
			|| !Resources::g_pResourceSystem->ReadMemory(&rawLooping, 1, stream)
			|| (looping = rawLooping != 0,
				!Resources::g_pResourceSystem->ReadMemory(&enabled, 1, stream))) return 0;
		MapSound* sound = new MapSound;
		sound->name = name;
		sound->position.x = position.x;
		sound->position.y = position.y;
		sound->position.z = position.z;
		sound->resourceName = resource;
		sound->looping = looping;
		sound->enabled = enabled != 0;
		UpperString(name);
		if (mapSounds.find(name) != mapSounds.end())
		{
			delete sound;
			std::string message;
			FormatString(message, "Duplicate instance name in Map::ReadMapSound(): %s.", name.c_str());
			throw ::Common::Exception(message.c_str());
		}
		mapSounds[name] = sound;
		return 1;
	}
	void Map::ClearSounds()
	{
		for (std::map<std::string,MapSound*>::iterator i = mapSounds.begin(); i != mapSounds.end(); ++i)
			if (i->second) delete i->second;
		mapSounds.clear();
	}
	void Map::SetActiveMap(DWORD id)
	{
		if (id == idB4)
		{
			for (std::map<std::string,MapSound*>::iterator i = mapSounds.begin(); i != mapSounds.end(); ++i)
				i->second->Initialize();
		}
		else
		{
			for (std::map<std::string,MapSound*>::iterator i = mapSounds.begin(); i != mapSounds.end(); ++i)
				i->second->Stop(0);
		}
	}
	void MapSound::SetEnabled(BYTE value)
	{
		if ((enabled != 0) == (value != 0)) return;
		enabled = value;
		if (value) Start();
		else Stop(0);
	}
	void MapSound::Initialize()
	{
		Clear();
		// Enabled is deliberately not a gate:5B2A00 creates/registers the real
		// sound, loads its resource and positions it whenever this string is set.
		if (!resourceName.empty())
		{
			sound = new VfxSoundInstance;
			sound->Load(resourceName.c_str(), 0, NULL); //Original ignores load result.
			sound->spatial = 1;
			sound->SetPosition(position);
		}
	}
	void MapSound::Update(DWORD)
	{
		if (!enabled || !sound) return;
		if (sound->IsPlaying())
		{
			if (sound->Priority() <= 0) Stop(0);
		}
		else
		{
			//52C480 uses the actual instance manager;52C710 is stored volume.
			SoundSystem* system = AudioBackend::g_System;
			const int priority = system->instanceManager->RequestPriority(*sound);
			if (priority > 0)
			{
				sound->SetPriority(priority);
				Start();
			}
		}
	}
}

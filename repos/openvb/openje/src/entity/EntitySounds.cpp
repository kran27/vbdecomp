#include "Entity.hpp"
#include "audio/SoundInstance.hpp"

namespace JE
{
	namespace { DWORD nextSoundID; } //Original70BE64 loader-zero.
	DWORD AllocateSoundID() //552A40.
	{
		if (!++nextSoundID) nextSoundID=1;
		return nextSoundID;
	}
	VfxSoundInstance* Entity::AttachSound(const char* filename, BYTE looping, DWORD id) //5512F0.
	{
		VfxSoundInstance* sound = new VfxSoundInstance;
		sound->Load(filename, 0, NULL); //The original ignores the loader result.
		sound->spatial = 1;
		sound->SetPosition(D3DXVECTOR3(position.x,position.y,position.z));
		return AttachSound(sound, looping, 1, id);
	}

	VfxSoundInstance* Entity::AttachSound(VfxSoundInstance* sound, BYTE looping,
		BYTE own, DWORD id) //54F3B0.
	{
		if (!sound) return NULL;
		std::map<VfxSoundInstance*, EntitySoundBinding>::iterator binding = activeSounds.find(sound);
		if (binding == activeSounds.end())
		{
			std::map<VfxSoundInstance*, EntitySoundBinding>::iterator inactive;
			inactive = inactiveSounds.find(sound);
			if (inactive != inactiveSounds.end())
			{
				activeSounds[sound] = inactive->second;
				inactiveSounds.erase(inactive);
				binding = activeSounds.find(sound);
			}
			else binding = activeSounds.end();
		}
		VfxSoundInstance* selected;
		if (binding != activeSounds.end())
		{
			selected = binding->second.sound;
			//54F468 sets ownership1 for every existing/reactivated binding.
			//Its elapsed time and stored sound pointer are retained.
			binding->second.deleteObject = 1;
			binding->second.id = id;
		}
		else
		{
			EntitySoundBinding& created = activeSounds[sound];
			created.sound = sound;
			created.deleteObject = own;
			created.elapsedSeconds = 0.0f;
			created.id = id;
			selected = sound;
		}
		if (selected && !selected->IsPlaying()) selected->Play(looping != 0, 0);
		return selected;
	}

	BYTE Entity::RemoveSound(VfxSoundInstance* sound, BYTE force) //54F4D0.
	{
		if (!sound) return 0;
		std::map<VfxSoundInstance*, EntitySoundBinding>::iterator found = activeSounds.find(sound);
		if (found != activeSounds.end())
		{
			EntitySoundBinding& binding = found->second;
			if (!force && binding.sound && binding.sound->IsPlaying())
			{
				binding.sound->flags &= ~DWORD(1);
				inactiveSounds[binding.sound] = binding;
			}
			else if (binding.sound && binding.deleteObject) delete binding.sound;
			activeSounds.erase(found);
			return 1;
		}
		return 0;
	}

	BYTE Entity::RemoveSound(DWORD id, BYTE force) //54F590; binding ID, not pointer key.
	{
		BYTE removed=0;
		std::map<VfxSoundInstance*,EntitySoundBinding>::iterator i=activeSounds.begin();
		while (i!=activeSounds.end())
		{
			if (i->second.id==id)
			{
				EntitySoundBinding& binding=i->second;
				if (!force && binding.sound && binding.sound->IsPlaying())
				{
					binding.sound->flags &= ~DWORD(1);
					inactiveSounds[binding.sound] = binding;
				}
				else if (binding.sound && binding.deleteObject) delete binding.sound;
				i=activeSounds.erase(i); //Use the original returned successor, including ID0.
				removed=1;
			}
			else ++i;
		}
		return removed;
	}

	void Entity::ClearSounds() //54C180.
	{
		for (std::map<VfxSoundInstance*, EntitySoundBinding>::iterator active = activeSounds.begin();
			active != activeSounds.end(); ++active)
			if (active->second.sound && active->second.deleteObject) delete active->second.sound;
		activeSounds.clear();
		for (std::map<VfxSoundInstance*, EntitySoundBinding>::iterator inactive = inactiveSounds.begin();
			inactive != inactiveSounds.end(); ++inactive)
			if (inactive->second.sound && inactive->second.deleteObject) delete inactive->second.sound;
		inactiveSounds.clear();
	}

	void Entity::UpdateSounds(DWORD elapsed) //550AD0 sound-map portion, before VFX.
	{
		std::list<VfxSoundInstance*> finished;
		const float seconds = static_cast<float>(double(elapsed) * 0.001);
		for (std::map<VfxSoundInstance*, EntitySoundBinding>::iterator active = activeSounds.begin();
			active != activeSounds.end(); ++active)
		{
			EntitySoundBinding& binding = active->second;
			binding.elapsedSeconds += seconds;
			binding.sound->SetPosition(D3DXVECTOR3(position.x,position.y,position.z));
			if (!(binding.sound->flags & 1) && !binding.sound->IsPlaying())
				finished.push_back(binding.sound);
		}
		for (std::list<VfxSoundInstance*>::iterator completed = finished.begin();
			completed != finished.end(); ++completed)
			RemoveSound(*completed, 1);
		std::map<VfxSoundInstance*, EntitySoundBinding>::iterator inactive = inactiveSounds.begin();
		while (inactive != inactiveSounds.end())
		{
			EntitySoundBinding& binding = inactive->second;
			binding.elapsedSeconds += seconds;
			binding.sound->SetPosition(D3DXVECTOR3(position.x,position.y,position.z));
			if (binding.sound->IsPlaying()) ++inactive;
			else
			{
				if (binding.deleteObject && binding.sound) delete binding.sound;
				std::map<VfxSoundInstance*, EntitySoundBinding>::iterator old = inactive++;
				inactiveSounds.erase(old);
			}
		}
	}
}

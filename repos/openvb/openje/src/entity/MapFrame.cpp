#include "startup/NetworkComponent.hpp"
#include "Map.hpp"
#include "MapFog.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Graphics.hpp"
#include "MapSound.hpp"
#include "Water.hpp"
#include "World.hpp"
#include "startup/CommonException.hpp"
#include "VisualEffect.hpp"
#include "graphics/VfxFrame.hpp"
#include "graphics/VfxVeg.hpp"
#include <stdexcept>

namespace JE
{
	void Map::Update(DWORD time)
	{
		if (!UpdateDue(time)) return;
		switch (state)
		{
		case 0:
			throw ::Common::Exception("Trying to update map when its state is invalid");
		case 1:
			OnFirstUpdate(time);
			break;
		case 2:
			OnSecondUpdate(time);
			break;
		default:
			OnUpdate(time);
			break;
		}
		lastUpdateTime = time;
		nextUpdateTime += GetUpdateInterval();
	}

	void Map::OnFirstUpdate(DWORD time)
	{
		static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74->ClearMapMasks();
		if (areaMap) areaMap->visible.Clear();
		cell result, argument = cell(time);
		RunScript("OnFirstUpdate", &result, 1, &argument);
		ChangeState(3);
	}

	void Map::UpdateClient(DWORD elapsed) //573730.
	{
		VfxFrame::SetWind(wind.x, wind.y, wind.z);
		for (std::list<Water*>::iterator water = waterObjects.begin(); water != waterObjects.end(); ++water)
			(*water)->ClientFrame(elapsed);
		if (static_cast<IWorld*>(g_World)->GetCurrentMapID() == idB4)
			for (std::map<std::string,MapSound*>::iterator sound = mapSounds.begin(); sound != mapSounds.end(); ++sound)
			{
				MapSound* currentSound = sound->second;
				currentSound->Update(elapsed);
			}
		std::list<VisualEffect*> completed;
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			EntityEffectBinding& binding = i->second;
			const float seconds = float(elapsed) * 0.001f;
			binding.elapsedSeconds += seconds;
			binding.object->ClientFrame(elapsed);
			if (binding.enabled && binding.object->IsFinished()) completed.push_back(binding.object);
		}
		for (std::list<VisualEffect*>::iterator i = completed.begin(); i != completed.end(); ++i)
			RemoveVisualEffect(*i, 1);
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator inactive = inactiveEffects.begin();
		while (inactive != inactiveEffects.end())
		{
			EntityEffectBinding& binding = inactive->second;
			const float seconds = float(elapsed) * 0.001f;
			binding.elapsedSeconds += seconds;
			binding.object->ClientFrame(elapsed);
			if (binding.object->IsFinished())
			{
				if (binding.deleteObject && binding.object) delete binding.object;
				if (binding.instance && binding.releaseInstance) GetVfxVegCache().Release(binding.instance);
				if (binding.id)
				{
					std::map<DWORD,VisualEffect*>::iterator id = effectIDs.find(binding.id);
					if (id != effectIDs.end()) effectIDs.erase(id);
				}
				inactive = inactiveEffects.erase(inactive);
			}
			else ++inactive;
		}
		VfxFrame::AdvanceWind(float(elapsed) * 0.001f);
		VfxFrame::GetWind(wind.x, wind.y, wind.z);
		if (areaMap) areaMap->changes = 0;
	}
}

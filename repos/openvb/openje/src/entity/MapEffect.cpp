#include "startup/NetworkComponent.hpp"
#include "network/EngineServer.hpp"
#include "MapEffect.hpp"
#include "VisualEffect.hpp"
#include "graphics/VfxVeg.hpp"
#include "graphics/StringFormat.hpp"
#include <cstring>
#include <stdexcept>
#include <cctype>

namespace JE
{
	MapEffect::MapEffect(Map* input) : position(0,0,0),angles(0,0,0),enabled(1),owner(input) {}
	MapEffect::~MapEffect()
	{ Clear(); }
	void MapEffect::Clear()
	{
		for(std::map<DWORD,VisualEffect*>::iterator i=instances.begin();i!=instances.end();++i)
			owner->RemoveVisualEffect(i->second,1);
		instances.clear();
	}
	void MapEffect::Initialize()
	{
		Clear();
		if(resources.empty() || !owner) return;
		for(std::vector<std::string>::iterator i=resources.begin();i!=resources.end();++i)
		{
			DWORD id=static_cast<Server*>(F3::g_pNetworkInstance->server)->AllocateVisualEffectID();
			VisualEffect* effect=owner->AttachVisualEffect(i->c_str(),id,0);
			if(!effect) continue;
			instances[id]=effect;
			D3DXMATRIX translation,rotation;
			D3DXMatrixTranslation(&translation,position.x,position.y,position.z);
			D3DXMatrixRotationYawPitchRoll(&rotation,angles.x,angles.y,angles.z);
			D3DXMatrixMultiply(&translation,&rotation,&translation);
			effect->SetTransform(1,&translation);
			effect->SetEnabled(enabled);
			effect->selectable=0; //5B2EB2, raw BYTE clear at+170.
		}
	}
	void MapEffect::SetEnabled(BYTE input)
	{
		if((enabled!=0)==(input!=0)) return;
		enabled=input;
		for(std::map<DWORD,VisualEffect*>::iterator i=instances.begin();i!=instances.end();++i)
			i->second->SetEnabled(input);
	}
	void MapEffect::ResetElapsedTime()
	{
		for(std::map<DWORD,VisualEffect*>::iterator i=instances.begin();i!=instances.end();++i)
			i->second->elapsedSeconds=0;
	}
	void Map::ClearMapEffects() //572560.
	{
		for (std::map<std::string,MapEffect*>::iterator i=mapEffects.begin(); i!=mapEffects.end(); ++i)
			delete i->second;
		mapEffects.clear();
	}

	void Map::FindEffects(const std::string& name,std::list<MapEffect*>& output)
	{
		std::string wanted=name;
		UpperString(wanted);
		for(std::map<std::string,MapEffect*>::iterator i=mapEffects.begin();i!=mapEffects.end();++i)
		{
			std::string key=i->first;
			UpperString(key);
			if(key.find(wanted)!=std::string::npos)
			{
				MapEffect* effect=i->second;
				output.push_back(effect);
			}
		}
	}

	VisualEffect* Map::AttachVisualEffect(VFX_VEGInstance* instance, BYTE release,
		BYTE own, DWORD id, BYTE flag) //573270.
	{
		if (!instance) return NULL;
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = effects.find(instance);
		if (i == effects.end())
		{
			std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator old;
			old = inactiveEffects.find(instance);
			if (old != inactiveEffects.end())
			{
				EntityEffectBinding& previous = old->second;
				effects[instance] = previous;
				inactiveEffects.erase(old);
				i = effects.find(instance);
			}
			else i = effects.end();
		}
		if (i != effects.end())
		{
			i->second.deleteObject = flag;
			i->second.id = id;
			i->second.enabled = flag;
			if (id) effectIDs[id] = i->second.object;
			return i->second.object;
		}
		instance->Reset();
		EntityEffectBinding& binding = effects[instance];
		BYTE instanceOwnership = own;
		binding.object = new VisualEffect(0, instance, release & BYTE((instanceOwnership != 0) - 1), idB4);
		binding.deleteObject = flag;
		binding.elapsedSeconds = 0;
		binding.instance = instance;
		binding.releaseInstance = instanceOwnership;
		binding.id = id;
		binding.enabled = flag;
		if (id) effectIDs[id] = binding.object;
		return binding.object;
	}

	BYTE Map::RemoveVisualEffect(VisualEffect* object, BYTE force) //573430.
	{
		if (!object || !object->instance) return 0;
		VFX_VEGInstance* instance = object->instance;
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = effects.find(instance);
		if (i == effects.end()) return 0;
		EntityEffectBinding& binding = i->second;
		if (!force && binding.instance && binding.instance->GetDuration(0) >= 0)
		{
			binding.instance->Stop();
			inactiveEffects[binding.instance] = binding;
		}
		else
		{
			if (binding.object && binding.deleteObject) delete binding.object;
			if (binding.instance && binding.releaseInstance) GetVfxVegCache().Release(binding.instance);
			if (binding.id)
			{
				std::map<DWORD,VisualEffect*>::iterator id = effectIDs.find(binding.id);
				if (id != effectIDs.end()) effectIDs.erase(id);
			}
		}
		effects.erase(i);
		return 1;
	}

	void Map::AppendVisibleEffects(std::list<VFX_EffectInstance*>& output) //573550.
	{
		std::list<VisualEffect*> completed;
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = effects.begin();
			i != effects.end(); ++i)
		{
			const LONG count = LONG(i->second.instance->effects.size());
			for (LONG index = 0; index < count; ++index)
			{
				VFX_EffectInstance* child = i->second.instance->GetEffect(index);
				if (child->SkipBounds() && !i->second.object->IsFinished()) output.push_back(child);
			}
		}
	}

	void Map::ClearVisualEffects()
	{
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = effects.begin();
			i != effects.end(); ++i)
		{
			if (i->second.object && i->second.deleteObject) delete i->second.object;
			if (i->second.instance && i->second.releaseInstance)
				GetVfxVegCache().Release(i->second.instance);
		}
		effects.clear();
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = inactiveEffects.begin();
			i != inactiveEffects.end(); ++i)
		{
			if (i->second.object && i->second.deleteObject) delete i->second.object;
			if (i->second.instance && i->second.releaseInstance)
				GetVfxVegCache().Release(i->second.instance);
		}
		inactiveEffects.clear();
		effectIDs.clear();
	}
}

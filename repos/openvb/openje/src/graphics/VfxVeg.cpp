#include "VfxVeg.hpp"
#include "VfxFrame.hpp"
#include "VfxFormats.hpp"
#include "StringFormat.hpp"
#include "resources/ResourceSystem.hpp"
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <math.h>
#include <set>

namespace JE
{
	namespace
	{
		VFX_VEGCache g_VfxVegCache;
	}

	VFX_VEGInstance::VFX_VEGInstance()
	{
		data = NULL;
		ownsData = 0;
		value4C = NULL;
		startTime = 0;
		scale = 1.0f;
	}
	VFX_VEGInstance::~VFX_VEGInstance()
	{
		Cleanup();
	}

	void VFX_VEGInstance::ClearEffects()
	{
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i) delete *i;
		effects.clear();
	}

	void VFX_VEGInstance::Cleanup()
	{
		if (data && ownsData) { delete data; data = NULL; }
		if (value4C) { free(value4C); value4C = NULL; }
		ClearEffects();
	}

	void VFX_VEGInstance::Create(VFX_VEG* input, float value, BYTE clone)
	{
		Cleanup();
		if (!input) return;
		if (clone)
		{
			VFX_VEG* created = new VFX_VEG;
			data = created;
			created->VFX_ParameterHolder::operator=(*input);
			created->effects = input->effects; //48F024, actual owning value-vector assignment.
			//48F029..48F038 copies this pointer, not its contents. Prepare below
			//frees it; the source retains the former pointer, including that hazard.
			created->settings = input->settings;
			created->currentVersion = input->currentVersion;
			created->loadedVersion = input->loadedVersion;
			ownsData = 1;
		}
		else
		{
			data = input;
			ownsData = 0;
		}
		if (data) data->Prepare();
		scale = value;
		Refresh(-1);
	}

	LONG VFX_VEGInstance::Refresh(LONG index)
	{
		while (data)
		{
			if (index < 0)
			{
				startTime = float(VfxFrame::RandomRange(data->settings->minStartTime,
					data->settings->maxStartTime));
				ClearEffects();
				const LONG count = LONG(data->effects.size());
				if (count <= 0) return 0;
				effects.resize(count, NULL);
				for (LONG i = 0; i < count; ++i) effects[i] = NULL;
				LONG created = 0;
				for (LONG i = 0; i < LONG(data->effects.size()); ++i)
				{
					data->effects[i].Prepare();
					created += Refresh(i);
				}
				return created;
			}
			if (index >= LONG(effects.size()))
			{
				index = -1;
				continue;
			}
			VFX_EffectInstance* previous = effects[index];
			if (previous)
			{
				delete previous;
				effects[index] = NULL;
			}
			VFX_Effect& effect = data->effects[index];
			VFX_Parameter* type = effect.Find("EffectType");
			if (type) GetVfxFormatManager().Merge(effect, type->GetEnum());
			effect.Prepare();
			VFX_EffectInstance** slots = &effects[0];
			slots[index] = CreateEffect(&effect);
			return effects[index] != NULL;
		}
		return 0;
	}

	void VFX_VEGInstance::Reset()
	{
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance) instance->Reset();
		}
	}
	void VFX_VEGInstance::Stop()
	{
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance) instance->Stop();
		}
	}
	void VFX_VEGInstance::SetEnabled(BYTE enabled) //48DC60; owned tail of59F890.
	{
		for (std::vector<VFX_EffectInstance*>::iterator i=effects.begin();i!=effects.end();++i)
		{
			VFX_EffectInstance* instance=*i;
			if (instance) instance->SetEnabled(enabled);
		}
	}
	VFX_EffectInstance* VFX_VEGInstance::GetEffect(LONG index) const
	{
		if (index >= 0)
		{
			if (index < LONG(effects.size())) return effects[index];
		}
		return NULL;
	}
	float VFX_VEGInstance::GetDuration(BYTE includeInfinite) const
	{
		volatile float duration = 0; //48DA14/48DA23, each accepted maximum is stored/reloaded32.
		std::vector<VFX_EffectInstance*>::const_iterator last = effects.end();
		for (std::vector<VFX_EffectInstance*>::const_iterator i = effects.begin(); i != last; ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (!instance || instance->baseData.triggeredEffect) continue;
			const VFX_BaseData& effect = instance->baseData;
			if (!includeInfinite && !(effect.duration >= 0)) return -1.0f; //48D9EF, unordered also rejects.
			double end = effect.delay;
			if (effect.duration > 0) end += effect.duration;
			if (!(duration > end)) duration = float(end); //48DA12 also stores an unordered candidate.
		}
		return duration;
	}
	BYTE VFX_VEGInstance::IsFinished() const
	{
		if (!value4C) return 0;
		for (std::vector<VFX_EffectInstance*>::const_iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance && !instance->IsFinished(*value4C)) return 0;
		}
		return 1;
	}
	void VFX_VEGInstance::Draw(Gfx_Viewport* viewport)
	{
		for (std::list<VFX_EffectInstance*>::iterator i = values40.begin(); i != values40.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance && instance->active && !instance->SkipBounds()) instance->Draw(viewport);
		}
	}
	BYTE VFX_VEGInstance::AddLight(Gfx_Light* light) //48D8A0.
	{
		BYTE accepted = 0;
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance) accepted |= instance->AddLight(light);
		}
		return accepted;
	}
	void VFX_VEGInstance::RemoveLight(Gfx_Light* light) //48D8E0.
	{
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance) instance->RemoveLight(light);
		}
	}
	void VFX_VEGInstance::ClearLights() //48D910.
	{
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance) instance->ClearLights();
		}
	}
	void VFX_VEGInstance::FinishLights() //48D940.
	{
		for (std::vector<VFX_EffectInstance*>::iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance) instance->FinishLights();
		}
	}
	void VFX_VEGInstance::Update(float time, Gfx_ModelInstance* source, Gfx_ModelInstance* target,
		const std::list<Gfx_ModelInstance*>* extraSources,
		const D3DXMATRIX* sourceTransform, const D3DXMATRIX* targetTransform)
	{
		if (!data) return;
		volatile float shiftedTime = time + startTime; //48F0B7, round before timing calls/comparisons.
		time = shiftedTime;
		if (data->settings->loop)
		{
			float duration = GetDuration(1);
			volatile float roundedDuration = duration; //48F0C6 stores, but compares the returned value.
			if (duration > 0)
			{
				volatile float wrapped = float(fmod(time, roundedDuration)); //48F0E4.
				if (value4C && time > *value4C && wrapped < fmod(*value4C, roundedDuration)) Reset();
				time = wrapped;
			}
		}
		if (!value4C) value4C = static_cast<float*>(::operator new(sizeof(float))); //48F12C.
		*value4C = time;
		std::map<VFX_EffectInstance*,LONG> indices;
		values40.clear();
		std::list<VFX_EffectInstance*> pending;
		for (LONG i = 0; i < LONG(data->effects.size()); ++i)
		{
			VFX_Effect& effect = data->effects[i];
			if (!effect.data) effect.Prepare();
			if (!effects[i]) Refresh(i);
			const VFX_BaseData& definition = *effect.data;
			VFX_EffectInstance* instance = NULL;
			if (definition.effectType == -1 || i >= LONG(effects.size()) || time < definition.delay
				|| (definition.duration > 0 && double(definition.duration) + definition.delay <= time)
				|| definition.duration == 0 || !(time >= definition.delay)) goto inactive;
			instance = effects[i];
			if (!instance) goto inactive;
			if (instance->IsFinished(time))
			{
				if (instance->active) instance->Reset();
				goto inactive;
			}
			if (instance->baseData.effectType == 1 || instance->baseData.effectType == 8) pending.push_front(instance);
			else pending.push_back(instance);
			if (!effect.data->triggeredEffect) //48F2FE, reload after callbacks and list allocation.
			{
				instance->effectIndex = DWORD(i);
				values40.push_back(instance);
				indices[instance] = i;
			}
			continue;
		inactive:
			instance = effects[i]; //48F2A2, current receiver after IsFinished/Reset, including replacements.
			if (instance) { if (instance->active) instance->OnStop(); instance->active = 0; }
		}
		std::set<LONG> completed;
		while (!pending.empty())
		{
			VFX_EffectInstance* effect = pending.front(); pending.pop_front();
			LONG index = -1;
			std::map<VFX_EffectInstance*,LONG>::iterator selected = indices.find(effect);
			if (selected != indices.end()) { index = selected->second; indices.erase(selected); }
			if (LONG(effect->baseData.target.selector) >= 3
				&& effect->baseData.effectType != 1 && effect->baseData.effectType != 8)
			{
				LONG dependencyIndex = LONG(effect->baseData.target.selector) - 3;
				VFX_EffectInstance* dependency = effects[dependencyIndex]; //48F422, original unchecked lookup.
				if (dependency && indices.find(dependency) != indices.end()
					&& completed.find(dependencyIndex) == completed.end())
				{
					pending.push_back(effect);
					indices.insert(std::make_pair(effect, LONG(0))).first->second = index;
					continue;
				}
			}
			VFX_UpdateContext context;
			context.source = source; context.target = target; context.group = this;
			context.sourceTransform = sourceTransform; context.targetTransform = targetTransform;
			BYTE updated = 0;
			if (effect->Prepare(context))
			{
				updated = 1;
				effect->Update(time - effect->baseData.delay, context);
			}
			if (effect->AppliesToExtraSources() && extraSources && !extraSources->empty())
			{
				for (std::list<Gfx_ModelInstance*>::const_iterator i = extraSources->begin(); i != extraSources->end(); ++i)
				{
					context.source = *i; context.sourceTransform = context.targetTransform = NULL;
					effect->Update(time - effect->baseData.delay, context);
				}
			}
			if (updated) { if (!effect->active) effect->OnStart(); }
			else if (effect->active) effect->OnStop();
			effect->active = updated;
			completed.insert(index);
		}
	}
	BYTE VFX_VEGInstance::GetFlag() const
	{
		for (std::vector<VFX_EffectInstance*>::const_iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (instance && instance->GetFlag()) return 1;
		}
		return 0;
	}
	void VFX_VEGInstance::GetBounds(D3DXVECTOR3& low, D3DXVECTOR3& high) const
	{
		low = D3DXVECTOR3(FLT_MAX, FLT_MAX, FLT_MAX);
		high = D3DXVECTOR3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (std::vector<VFX_EffectInstance*>::const_iterator i = effects.begin(); i != effects.end(); ++i)
		{
			VFX_EffectInstance* instance = *i;
			if (!instance || !instance->active || instance->SkipBounds()) continue;
			D3DXVECTOR3 a(FLT_MAX, FLT_MAX, FLT_MAX), b(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			instance->GetBounds(a, b);
			low.x = low.x < a.x ? low.x : a.x;
			low.y = low.y < a.y ? low.y : a.y;
			low.z = low.z < a.z ? low.z : a.z;
			high.x = high.x > b.x ? high.x : b.x;
			high.y = high.y > b.y ? high.y : b.y;
			high.z = high.z > b.z ? high.z : b.z;
		}
	}

	VFX_VEGCache::VFX_VEGCache() {}
	VFX_VEGCache::~VFX_VEGCache()
	{
		for (Entries::iterator i = entries.begin(); i != entries.end(); ++i)
			delete i->second.data;
	}
	VFX_VEGCache& GetVfxVegCache() { return g_VfxVegCache; }

	VFX_VEGInstance* VFX_VEGCache::Acquire(const char* name, float scale)
	{
		if (IsStringBlank(name, " \n\t\r")) return NULL;
		VFX_VEG* loaded = NULL;
		DWORD* references = NULL;
		Entries::iterator i = entries.find(name);
		if (i == entries.end())
		{
			Resources::ResourceMemoryStream* stream = Resources::g_pResourceSystem->OpenMemoryStream(name, 0);
			if (!stream) return NULL;
			VFX_VEG* created = new VFX_VEG;
			DWORD offset = 0;
			if (created->Read(stream->begin, offset))
			{
				loaded = created;
				Entry fresh = { loaded, 0 };
				entries[name] = fresh;
				references = &entries[name].references;
			}
			else delete created;
			Resources::g_pResourceSystem->CloseMemoryStream(stream);
		}
		else
		{
			loaded = i->second.data;
			references = &i->second.references;
		}
		if (!loaded) return NULL;
		VFX_VEGInstance* instance = new VFX_VEGInstance;
		if (!instance) return NULL;
		instance->Create(loaded, scale, 0);
		instance->name = std::string(name);
		if (references) ++*references;
		return instance;
	}
	void VFX_VEGCache::Release(VFX_VEGInstance* instance)
	{
		if (!instance) return;
		std::string name = instance->name;
		delete instance;
		Entries::iterator i = entries.find(name);
		if (i == entries.end()) return;
		--i->second.references;
	}
}

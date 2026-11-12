#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "Map.hpp"
#include "VisualEffect.hpp"
#include "Water.hpp"
#include "World.hpp"
#include "graphics/AnimationInstance.hpp"
#include "graphics/Octree.hpp"
#include "graphics/VfxVeg.hpp"
#include "model/ModelInstance.hpp"
#include <float.h>
#include <stdexcept>

namespace JE
{
	void Entity::ClientFrame(DWORD elapsed) //549240, virtual+08.
	{
		OnClientFrame(elapsed); // Actual live virtual+D0 dispatch.
	}

	void Entity::OnAnimationCue(const Gfx_AnimationCue&) {} // Shared5A9320.

	BYTE Entity::RemoveVisualEffect(VisualEffect* effect, BYTE force) //54F150.
	{
		if (!effect || !effect->instance) return 0;
		VFX_VEGInstance* instance = effect->instance;
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.find(instance);
		if (i == activeEffects.end()) return 0;
		EntityEffectBinding& binding = i->second;
		if (!force && binding.instance)
		{
			binding.instance->Stop();
			inactiveEffects[binding.instance] = binding;
		}
		else
		{
			if (binding.object)
			{
				if (binding.object->instance) binding.object->instance->Reset();
				if (binding.deleteObject) delete binding.object;
			}
			if (binding.instance && binding.releaseInstance) GetVfxVegCache().Release(binding.instance);
			if (binding.id)
			{
				std::map<DWORD,VisualEffect*>::iterator id = effectIDs.find(binding.id);
				if (id != effectIDs.end()) effectIDs.erase(id);
			}
		}
		activeEffects.erase(i);
		return 1;
	}

	void Entity::OnClientFrame(DWORD elapsed) //550AD0, virtual+D0.
	{
		if (!renderEnabled) return;
		if (model)
		{
			model->UpdateAnimation(GetAnimationTime());
			if (model->animation)
			{
				std::list<Gfx_AnimationCue>& cues = model->animation->cues;
				for (std::list<Gfx_AnimationCue>::iterator i = cues.begin(); i != cues.end(); ++i)
					OnAnimationCue(*i);
			}
		}
		if (GetEntityEnvironment().WaterMode() != 2)
		{
			const D3DXVECTOR3 sample = position;
			if (!dynamic_cast<Water*>(this) && (sample.x != lastWaterPosition.x
				|| sample.y != lastWaterPosition.y || sample.z != lastWaterPosition.z))
			{
				Map* owner = g_World->GetMap(mapHandle);
				if (owner && owner->GetOctree())
				{
					OCTREE* scene = owner->GetOctree();
					scene->ResetQueryCursor();
					const D3DXVECTOR3 origin(sample.x, sample.y + maximum.y, sample.z);
					const D3DXVECTOR3 direction(0, minimum.y - maximum.y, 0);
					const OctreeRay ray(&origin.x, &direction.x);
					OctreeObjectHit hits[32];
					if (scene->QueryObjects(ray, 32, hits) == 1)
					{
						Water* selected = NULL;
						float nearest = FLT_MAX;
						for (OctreeObjectHit* hit = hits; hit->hit; ++hit)
							if (Water* water = dynamic_cast<Water*>(static_cast<DynamicObject*>(hit->object)))
								if (hit->distance < nearest)
								{
									const float floorPosition[3] = {sample.x, float(double(sample.y) + 0.001), sample.z};
									float height;
									if (!scene->QueryFloorHeight(floorPosition, &height, 1, 0, NULL)
										|| height <= water->position.y)
									{
										selected = water;
										nearest = hit->distance;
									}
								}
						//550D34 checks the selected tile's configuration+3F only after
						//nearest/floor filtering; it does not choose a farther enabled tile.
						if (selected && selected->configuration.flag3F) selected->AddRipple(sample.x, sample.z, 0.01f);
					}
				}
				lastWaterPosition = sample;
			}
		}

		UpdateSounds(elapsed); //Actual550D74 sound-map pass precedes VFX.
		std::list<VisualEffect*> finished;
		const float seconds = static_cast<float>(double(elapsed) * 0.001);
		for (std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i = activeEffects.begin();
			i != activeEffects.end(); ++i)
		{
			EntityEffectBinding& binding = i->second;
			binding.elapsedSeconds += seconds;
			binding.object->ClientFrame(elapsed);
			if (binding.enabled && binding.object->IsFinished()) finished.push_back(binding.object);
		}
		for (std::list<VisualEffect*>::iterator i = finished.begin(); i != finished.end(); ++i)
			RemoveVisualEffect(*i, 1);
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator inactive = inactiveEffects.begin();
		while (inactive != inactiveEffects.end())
		{
			EntityEffectBinding& binding = inactive->second;
			binding.elapsedSeconds += seconds;
			binding.object->ClientFrame(elapsed);
			if (binding.object->IsFinished())
			{
				if (binding.deleteObject && binding.object) delete binding.object;
				if (binding.instance && binding.releaseInstance) GetVfxVegCache().Release(binding.instance);
				if (binding.id) effectIDs.erase(binding.id);
				std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator old = inactive++;
				inactiveEffects.erase(old);
			}
			else ++inactive;
		}
		if (model)
		{
			model->UpdateAfterimages(elapsed);
			if (model->IsAttached())
			{
				D3DXVECTOR3 location;
				model->GetPosition(location);
				ModelInstance* held = model;
				model = NULL;
				SetPosition(location);
				model = held;
			}
		}
	}
}

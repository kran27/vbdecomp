#include <winsock2.h>
#include "World.hpp"
#include "graphics/Graphics.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "Map.hpp"
#include "VisualEffect.hpp"
#include "graphics/Octree.hpp"
#include "graphics/OctreeRender.hpp"
#include "graphics/VfxLight.hpp"
#include "graphics/VfxVeg.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/GraphicsScene.hpp"
#include <stdexcept>

namespace
{
	struct EffectLightOrder
	{
		bool operator()(const JE::VFX_Light* left, const JE::VFX_Light* right) const
		{
			return left->data.priority > right->data.priority; //547AB0 signed descending.
		}
	};
	JE::World& CurrentWorld()
	{
		return *JE::g_World;
	}
	void AppendMapEffects(std::list<JE::VFX_EffectInstance*>& output, JE::Map* map)
	{
		if (map) map->AppendVisibleEffects(output);
	}
}

namespace JE
{
	void World::CollectVisibleEffects(OCTREE* tree, Map* activeMap) //548840, World+0C interface.
	{
		if (!tree) return; // The original leaves all three previous collections intact.
		CollectVisibleObjects(tree);
		AppendMapEffects(lateEffects, activeMap); //Retained explicit stable-map input factor.
	}

	void World::CollectVisibleEffects()
	{
		OCTREE* tree = GetCurrentMapOctree(); //54884F, actual client interface.
		if (!tree) return;
		visibleObjects.clear(); //Release storage before resetting its three owner pointers.
		visibleLights.clear();
		lateEffects.clear();
		tree->ResetQueryCursor();
		void* objects[20] = {0};
		DWORD more = 0;
		do
		{
			tree->QueryVisibleObjects(20, objects, NULL, &more);
			for (DWORD i = 0; objects[i]; ++i)
			{
				DynamicObject* object = static_cast<DynamicObject*>(objects[i]);
				if (object)
				{
					visibleObjects.push_back(object);
					if (VisualEffect* effect = dynamic_cast<VisualEffect*>(object))
					{
						effect->ResetVisibility();
						effect->CollectLights(visibleLights);
					}
				}
			}
		} while (more);
		visibleLights.sort(EffectLightOrder()); //548130 is library sort, not another effect producer.
		for (std::list<VFX_Light*>::iterator i = visibleLights.begin(); i != visibleLights.end(); ++i)
			(*i)->UpdateInfluence(tree);

		const DWORD activeID = static_cast<IWorldClient*>(g_World)->GetCurrentMapID();
		Map* map = g_World->GetMap(activeID); //Reload after the virtual map-ID callback.
		if (map) map->AppendVisibleEffects(lateEffects);
	}

	void World::CollectVisibleObjects(OCTREE* tree)
	{
		visibleObjects.clear(); //Release storage before resetting its three owner pointers.
		visibleLights.clear();
		lateEffects.clear();
		tree->ResetQueryCursor();
		void* objects[20] = {0};
		DWORD more = 0;
		do
		{
			tree->QueryVisibleObjects(20, objects, NULL, &more);
			for (DWORD i = 0; objects[i]; ++i)
			{
				DynamicObject* object = static_cast<DynamicObject*>(objects[i]);
				visibleObjects.push_back(object);
				if (VisualEffect* effect = dynamic_cast<VisualEffect*>(object))
				{
					effect->ResetVisibility();
					effect->CollectLights(visibleLights);
				}
			}
		} while (more);
		visibleLights.sort(EffectLightOrder()); //548130 is library sort, not another effect producer.
		for (std::list<VFX_Light*>::iterator i = visibleLights.begin(); i != visibleLights.end(); ++i)
			(*i)->UpdateInfluence(tree);

	}

	void World::DrawVisible() //545AA0, actual zero-argument client entry.
	{
		GraphicsSceneState graphics(*static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74);
		DrawVisible(graphics);
	}

	void World::DrawVisible(GraphicsSceneState& graphics) //545AA0, actual owner reads within the draw pass.
	{
		if (graphics.renderer) graphics.renderer->ResetLights();
		for (std::list<VFX_Light*>::iterator i = visibleLights.begin(); i != visibleLights.end(); ++i)
		{
			VFX_Light* effect = *i;
			if (effect->active && effect->light)
			{
				OCTREE* tree = CurrentWorld().GetCurrentMapOctree();
				effect->PositionInfluence(tree);
				Gfx_Light* light = effect->active ? effect->light : NULL;
				OctreeShapeNodes* nodes = effect->GetInfluenceNodes();
				if (graphics.renderer) graphics.renderer->AddLight(light, nodes);
			}
		}
		const DWORD shadowMode = graphics.shadowMode; //545B75 captures this once after terrain light callbacks.
		for (std::vector<DynamicObject*>::iterator i = visibleObjects.begin(); i != visibleObjects.end(); ++i)
		{
			DynamicObject* object = *i;
			const D3DXVECTOR3 position = object->position;
			if (object->flag30)
			{
				Gfx_Light* staticLights = graphics.renderer ? graphics.renderer->lights : NULL;
				object->ClearLights(); //545BC3 captures the static-light array before this callback.
				for (std::list<VFX_Light*>::iterator light = visibleLights.begin(); light != visibleLights.end(); ++light)
					if ((*light)->active && (*light)->light) object->AddLight((*light)->light);
				OCTREE* tree = CurrentWorld().GetCurrentMapOctree();
				if (tree)
				{
					DWORD count = 0;
					tree->GetLightVisibility(&position.x, &count, lightIndices);
					for (DWORD index = 0; index < count; ++index)
						object->AddLight(staticLights + lightIndices[index]);
				}
				object->FinishLights();
			}
			object->Draw();
			if (shadowMode == 1)
				if (Entity* entity = dynamic_cast<Entity*>(object))
					if (entity->renderEnabled && entity->shadow && entity->IsVisible() && entity->model)
					{
						const float point[3] = {position.x, position.z, 1.5f};
						g_GfxSystem->AddProjectedShadowPoint(point);
					}
		}
		for (std::list<VFX_EffectInstance*>::iterator i = lateEffects.begin(); i != lateEffects.end(); ++i)
			(*i)->Draw(NULL);
	}

	//Retained stable-input factor for isolated scenes/comparisons. The normal
	//Client frame uses the owner-reading overload above, preserving callback reloads.
	void World::DrawVisible(Octree2Gfx* renderer, OCTREE* tree, Gfx_Light* staticLights, DWORD shadowMode)
	{
		if (renderer) renderer->ResetLights();
		for (std::list<VFX_Light*>::iterator i = visibleLights.begin(); i != visibleLights.end(); ++i)
		{
			VFX_Light* effect = *i;
			if (effect->active && effect->light)
			{
				effect->PositionInfluence(tree);
				Gfx_Light* light = effect->active ? effect->light : NULL;
				if (renderer) renderer->AddLight(light, effect->GetInfluenceNodes());
			}
		}
		for (std::vector<DynamicObject*>::iterator i = visibleObjects.begin(); i != visibleObjects.end(); ++i)
		{
			DynamicObject* object = *i;
			const D3DXVECTOR3 position = object->position; // Snapshot precedes every callback.
			if (object->flag30)
			{
				object->ClearLights();
				for (std::list<VFX_Light*>::iterator light = visibleLights.begin(); light != visibleLights.end(); ++light)
					if ((*light)->active && (*light)->light) object->AddLight((*light)->light);
				if (tree)
				{
					DWORD count = 0;
					WORD* indices = lightIndices;
					tree->GetLightVisibility(&position.x, &count, indices);
					for (DWORD index = 0; index < count; ++index) object->AddLight(staticLights + indices[index]);
				}
				object->FinishLights();
			}
			object->Draw();
			if (shadowMode == 1)
				if (Entity* entity = dynamic_cast<Entity*>(object))
					if (entity->renderEnabled && entity->shadow && entity->IsVisible() && entity->model)
					{
						const float point[3] = {position.x, position.z, 1.5f};
						g_GfxSystem->AddProjectedShadowPoint(point);
					}
		}
		for (std::list<VFX_EffectInstance*>::iterator i = lateEffects.begin(); i != lateEffects.end(); ++i)
			(*i)->Draw(NULL); // Original calls+08 without repeating active/visibility gates.
	}
}

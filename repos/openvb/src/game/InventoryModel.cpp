#include "game/InventoryInterface.hpp"
#include "game/GameEntity.hpp"
#include "game/GameInventory.hpp"
#include "entity/Entity.hpp"
#include "entity/EntityEnvironment.hpp"
#include "model/ModelInstance.hpp"
#include "graphics/Viewport.hpp"
#include "graphics/RenderManager.hpp"
#include "graphics/CameraController.hpp"
#include "graphics/VfxVeg.hpp"
#include "gui/Manager.hpp"
#include <mmsystem.h>
#include <stdexcept>

namespace F3
{
	void GameEntity::SubmitEffects(Gfx_Viewport* viewport) //4671A0.
	{
		JE::Entity::SubmitEffects(viewport);
		if (JE::GetEntityEnvironment().RenderMode()) return; //Original Client+104 reload after base.
		if (!inventory) return;
		GameInventory* gameInventory = dynamic_cast<GameInventory*>(inventory);
		if (!gameInventory) return;
		for (std::list<JE::Entity*>::iterator i = gameInventory->items.begin(); i != gameInventory->items.end(); ++i)
			if (gameInventory->IsEquipped(*i)) (*i)->SubmitEffects(viewport);
	}
	void InventoryInterface::BeforeUpdate() //43A620, primary GUI slot43B180.
	{
		if (!actorID) return;
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		if (!actor || !actor->model) return;
		D3DXVECTOR3 position;
		actor->model->GetPosition(position);
		const DWORD now = timeGetTime();
		rotation = float(double(DWORD(now - lastModelTick)) * 0.001 + double(rotation));
		if (rotation > 6.2831855f)
		{
			double angle = rotation;
			do { angle -= 6.2831855f; } while (angle > 6.2831855f);
			rotation = float(angle);
		}
		lastModelTick = now;
		orbit->target.x = position.x;
		orbit->target.y = float(double(position.y) + 2.8);
		orbit->target.z = position.z;
		orbit->dirty = 1;
		orbit->SetOrbit(rotation, float(double(elevation) * 0.017453292f), distance);
		orbit->Update();
		::GUI::g_Manager.entries148.push_back(viewport);
		viewport->flag1C = 1;
		viewport->renderer->QueueMeshInstances(&viewport->instance, 1);
	}
	void InventoryInterface::BeforeDraw() //43A750, GUI slot43B190.
	{
		if (!actorID) return;
		JE::Entity* actor = JE::GetEntityPtr(actorID);
		if (!actor || !actor->model) return;
		actor->model->Submit(viewport, 0, 0.05f, 0xFF00FFFF);
		actor->SubmitEffects(viewport);
	}
}

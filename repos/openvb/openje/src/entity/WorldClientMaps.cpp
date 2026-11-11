#include <winsock2.h>
#include "startup/NetworkComponent.hpp"
#include "World.hpp"
#include "network/EngineClient.hpp"
#include "graphics/Graphics.hpp"
#include "Map.hpp"
#include "Entity.hpp"
#include "EntityEnvironment.hpp"
#include "graphics/VfxFrame.hpp"
#include "model/ModelInstance.hpp"

namespace JE
{
	void World::UpdateClientMaps(DWORD elapsed)
	{
		if(value38==4) return;
		VfxFrame::SetCamera(GetEntityEnvironment().CurrentMapCamera());
		for(WorldMaps::iterator i=maps.begin();i!=maps.end();++i)
			i->second->UpdateClient(elapsed);
	}

	void World::UpdateClient(DWORD elapsed)
	{
		if (value38 == 4) return;
		Graphics* graphics = static_cast<Client*>(F3::g_pNetworkInstance->client)->child0x74;
		VfxFrame::SetCamera(graphics->GetPickingCamera());
		for (WorldMaps::iterator i = maps.begin(); i != maps.end(); ++i)
			i->second->UpdateClient(elapsed);
		for (std::list<Entity*>::iterator i = g_createdEntities.begin(); i != g_createdEntities.end(); ++i)
		{
			Entity* entity = *i;
			entity->SetShadow(entity->shadow);
			if (entity->model)
			{
				entity->model->ResetMaterialStates();
				entity->model->SetMaterialFlagBC(1);
			}
		}
		for (std::list<Entity*>::iterator i = g_createdEntities.begin(); i != g_createdEntities.end(); ++i)
			(*i)->ClientFrame(elapsed);
	}

}

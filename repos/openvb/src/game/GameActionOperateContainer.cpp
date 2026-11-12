#include <winsock2.h>
#include "GameActionOperateContainer.hpp"
#include "GameContainer.hpp"
#include "GameCreature.hpp"
#include "entity/ActionMoveTo.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "graphics/Camera.hpp"
#include "graphics/SystemInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <math.h>
#include <new>
#include <stdexcept>

namespace F3
{
	GameActionOperateContainer::GameActionOperateContainer() : target(NULL),targetID(0),completed(0)
	{
		//406A10 writes starting1 but no kind or minimumBudget; preserve those holes.
	}
	BYTE GameActionOperateContainer::IsComplete() const { return completed; }
	void GameActionOperateContainer::Cancel() {}
	int GameActionOperateContainer::Start(DWORD) { return 0; }
	int GameActionOperateContainer::Finish(DWORD) { return 0; }
	void GameActionOperateContainer::SetTarget(JE::Entity* entity)
	{
		target=entity;
		if (entity) targetID=entity->entityID;
	}
	int GameActionOperateContainer::Update(DWORD,DWORD)
	{
		target=JE::GetEntityPtr(targetID);
		if (!target) { completed=1;return 0; }
		GameContainer* container=dynamic_cast<GameContainer*>(target);
		GameCreature* creature; //406B03 leaves the saved discriminator unwritten for containers.
		if (!container)
		{
			creature=dynamic_cast<GameCreature*>(target);
			if (!creature)
			{
				JE::LogDebugString("GameActionOperateContainer::OnServerUpdate - Target %s wasn't a container or a creature.\n",
					target->resourceName.c_str());
				completed=1;
				return 0;
			}
		}
		if (target->mapHandle!=owner->mapHandle) { completed=1;return 0; }
		const double x=double(owner->position.x)-target->position.x;
		const double y=double(owner->position.y)-target->position.y;
		const double z=double(owner->position.z)-target->position.z;
		if (sqrt(x*x+y*y+z*z)>=5.0)
		{
			JE::ActionMoveTo* move=static_cast<JE::ActionMoveTo*>(JE::g_World->NewAction(2));
			move->SetTarget(target);
			move->stopDistance=5;
			owner->QueueAction(move);
			GameActionOperateContainer* retry=new(std::nothrow) GameActionOperateContainer;
			retry->SetTarget(target);
			owner->QueueAction(retry);
			completed=1;
			return 0;
		}
		D3DXVECTOR3 direction=target->position-owner->position;
		const double inverse=1.0/sqrt(double(direction.x)*direction.x+
			double(direction.y)*direction.y+double(direction.z)*direction.z);
		direction.x=float(inverse*direction.x);
		direction.y=float(inverse*direction.y);
		direction.z=float(inverse*direction.z);
		D3DXVECTOR3 rotation=owner->rotation;
		const double cosine=double(direction.y)*0.0f+double(direction.x)*0.0f+direction.z;
		rotation.x=float(acos(cosine)*57.295776f);
		if (direction.x<0) rotation.x=-rotation.x;
		owner->SetRotation(rotation);

		//406C86..406D3E. Camera() has no argument: each destination is
		//pre-pushed for the following real matrix getter. Use the SDK math.
		const D3DXVECTOR3 position=target->position;
		D3DXMATRIX view,projection,combined;
		g_GfxSystem->Camera()->GetViewMatrix(view);
		g_GfxSystem->Camera()->GetProjectionMatrix(projection);
		D3DXMatrixMultiply(&combined,&view,&projection);
		D3DXVECTOR4 projected;
		D3DXVec3Transform(&projected,&position,&combined);
		const double inverseW=1.0/projected.w;
		const float screenX=float((projected.x*inverseW+1.0)*512.0f);
		const float screenY=float((2.0-(projected.y*inverseW+1.0))*384.0f);
		if (container) container->Operate(owner);
		JE::Party* party=JE::g_World->GetParty();
		//406D7E tests the captured pointer word before reading the live open BYTE.
		//Its initial container-path contents remain indeterminate, as in the original.
		if (party->Contains(owner)==1 && (creature!=NULL || container->open==1))
		{
			JE::Client* published = g_pNetworkInstance ?
				dynamic_cast<JE::Client*>(g_pNetworkInstance->client) : NULL;
			GameClient* client = dynamic_cast<GameClient*>(published);
			//Population now precedes original Y/X truncations in the connected
			//suffix. Keep both saved float inputs until the popup has populated.
			client->RequestInventoryInspection(target->entityID, owner->entityID, screenX, screenY);
		}
		completed=1;
		return 12; //406E42, with no budget threshold in this action.
	}
}

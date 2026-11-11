#include "startup/NetworkComponent.hpp"
#include "Entity.hpp"
#include "Effect.hpp"
#include "EntityEnvironment.hpp"
#include "World.hpp"
#include "model/CommonException.hpp"
#include <stdexcept>

namespace JE
{
	void Entity::ClearMapMessages() { mapMessages.clear(); }
	void Entity::ClearMapAssociations() { mapAssociations.clear(); }

	void Entity::UpdateClient(DWORD time)
	{
		actionBlocked = 0;
		frameTime = time;
	}

	void Entity::ServerUpdate(DWORD time)
	{
		if (!UpdateDue(time) || !renderEnabled) return;
		switch (state)
		{
		case 0:
			throw ::Common::Exception("Trying to update entity when its state is invalid");
		case 1:
			OnActivate(time);
			break;
		case 2:
			OnDestroy(time);
			break;
		case 3:
			OnLoad(time);
			break;
		case 6:
			OnDeath(time);
			break;
		case 7:
			if (pendingAnimation) Update(time);
			break;
		case 4:
		case 5:
			Update(time);
			break;
		case 8:
			break;
		default:
			throw ::Common::Exception("Unknown state in Entity::ServerUpdate()\n");
		}
		lastUpdateTime = time;
		DWORD interval = GetUpdateInterval();
		nextUpdateTime += interval;
	}

	void Entity::OnActivate(DWORD time)
	{
		cell result;
		cell argument = static_cast<cell>(time);
		RunScript("OnCreate", &result, 1, &argument);
		ChangeState(4);
	}

	void Entity::OnLoad(DWORD time)
	{
		cell result;
		cell argument = static_cast<cell>(time);
		RunScript("OnLoad", &result, 1, &argument);
		ChangeState(4);
	}

	void Entity::OnDestroy(DWORD time)
	{
		cell result;
		cell argument = static_cast<cell>(time);
		RunScript("OnDestroy", &result, 1, &argument);
		ChangeState(8);
	}

	void Entity::OnDeath(DWORD time)
	{
		cell result;
		cell argument = static_cast<cell>(time);
		RunScript("OnDeath", &result, 1, &argument);
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		IWorldServer* world = g_World;
		world->GetTeams()->RemoveEntityTeam(entityID);
		ChangeState(7); //Original ignores a failed/absent-team removal result.
	}

	void Entity::OnRefreshAssociations() {} // Original shared404B70.

	void Entity::RefreshAssociations()
	{
		OnRefreshAssociations();
		for (std::deque<Effect*>::iterator i = effects.begin(); i != effects.end(); ++i)
			(*i)->RefreshTree();
		associationsDirty = 0;
	}
}

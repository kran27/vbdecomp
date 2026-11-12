#include "game/MiniMapServices.hpp"
#include "game/ActionbarInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "entity/EntityEnvironment.hpp"
#include <stdexcept>

namespace F3 { namespace MiniMapServices
{
	JE::Client& GetClient()
	{
		return *static_cast<JE::Client*>(g_pNetworkInstance->client);
	}
	MapMaskManager& GetMaskManager()
	{
		MapMaskManager* masks=JE::GetEntityEnvironment().GetMapMaskManager();
		return *masks;
	}
	ActionbarInterface& GetActionbar()
	{
		GameClient* client=dynamic_cast<GameClient*>(&GetClient());
		return *client->Actionbar();
	}
} }

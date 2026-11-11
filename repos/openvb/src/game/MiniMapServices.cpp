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
		if (!g_pNetworkInstance || !g_pNetworkInstance->client)
			throw std::logic_error("MiniMap: actual published Network Client required");
		return *static_cast<JE::Client*>(g_pNetworkInstance->client);
	}
	MapMaskManager& GetMaskManager()
	{
		MapMaskManager* masks=JE::GetEntityEnvironment().GetMapMaskManager();
		if (!masks) throw std::logic_error("MiniMap441D20: actual Graphics MapMaskManager required");
		return *masks;
	}
	ActionbarInterface& GetActionbar()
	{
		GameClient* client=dynamic_cast<GameClient*>(&GetClient());
		if (!client || !client->Actionbar())
			throw std::logic_error("MiniMap4420CA: actual Actionbar owner required");
		return *client->Actionbar();
	}
} }

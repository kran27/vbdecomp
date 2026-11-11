#include "EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "startup/MessageLoop.hpp"
#include "entity/World.hpp"
#include "model/CommonException.hpp"
#include <stdexcept>

namespace JE
{
	void Server::UpdatePolling() //552B60; also the actual GameServer table slot.
	{
		Net::Server::UpdatePolling();
		if (!(F3::g_pNetworkInstance->GetMode() & 1))
			throw ::Common::Exception("Invalid attempt to retrieve world server instance.");
		// Capture the actual server-interface owner before sampling the frame clock.
		IWorldServer* world = g_World;
		world->UpdateServer(F3::GetElapsedTimeMS());
		// Native reloads the dialogue only after the World callback returns.
		static_cast<DialogueManager*>(dialogue)->Update();
	}
}

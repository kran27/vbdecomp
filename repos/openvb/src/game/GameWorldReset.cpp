#include <winsock2.h>
#include "game/GameWorld.hpp"
#include "game/PipBoyInterface.hpp"
#include "game/ActionbarInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <stdexcept>

namespace F3
{
	void GameWorld::Reset() //46FFA0 adjustor ->46FCC0.
	{
		JE::World::Reset(); //46FCC7, actual game suffix follows full engine reset.
		GameClient* client = dynamic_cast<GameClient*>(static_cast<JE::Client*>(g_pNetworkInstance->client));
		if (client)
		{
			//Raw46FCF1 reads+A0 (decompiler's+28 is wrong), then+44 PipBoy.
			PipBoyInterface* pipBoy = client->Interfaces()->pipBoy;
			pipBoy->Reset(); //455A70.

			//46FCFF reloads the same captured client's parent/Actionbar after the
			//PipBoy callback; do not capture both screens before that operation.
			ActionbarInterface* actionbar = client->Interfaces()->actionbar;
			actionbar->SetCompanionMode(1); //4172D0 ->RefreshCompanions.
		}
	}
}

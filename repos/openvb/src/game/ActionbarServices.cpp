#include "game/ActionbarServices.hpp"
#include "game/InventoryInputServices.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/FeedbackServices.hpp"
#include "game/MiniMapInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/CommandProcessor.hpp"
#include <stdexcept>

namespace F3 { namespace ActionbarServices
{
	GameClient* GetClient()
	{ return static_cast<GameClient*>(g_pNetworkInstance->client); }
	void SetCompanionHidden(DWORD offset, BYTE hidden)
	{
		if (offset==0x0C)
		{
			FeedbackInterface* feedback=GetClientFeedback(GetClient());
			feedback->SetHidden(hidden);
		}
		else if (offset==0x48)
		{
			GameClient* client=GetClient();
			MiniMapInterface* miniMap=client->MiniMap();
			miniMap->SetHidden(hidden);
		}
		else throw std::logic_error("Actionbar416B80: unresolved companion offset");
	}
	void RefreshMinimap()
	{
		GameClient* client=GetClient();
		MiniMapInterface* miniMap=client->MiniMap();
		miniMap->Refresh();
	}
	void Feedback(const std::string& text, DWORD red, DWORD green, DWORD blue)
	{ DisplayClientFeedback(text,red,green,blue,std::string()); }
	void QueueCommand(const char* command) { GetClient()->child0x7C->Queue(command); }
} }

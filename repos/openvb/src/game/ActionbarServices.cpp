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
	JE::ClientInputState& InputState() { return InventoryInputServices::InputState(); }
	HWND Window() { return InventoryInputServices::Window(); }
	void SetCompanionHidden(DWORD offset, BYTE hidden)
	{
		if (offset==0x0C)
		{
			FeedbackInterface* feedback=GetClientFeedback(GetClient());
			if (!feedback) throw std::logic_error("Actionbar416B80: actual Feedback owner required");
			feedback->SetHidden(hidden);
		}
		else if (offset==0x48)
		{
			GameClient* client=GetClient();
			MiniMapInterface* miniMap=client->MiniMap();
			if (!miniMap)
				throw std::logic_error("Actionbar416B80: actual MiniMap owner required");
			miniMap->SetHidden(hidden);
		}
		else throw std::logic_error("Actionbar416B80: unresolved companion offset");
	}
	void RefreshMinimap()
	{
		GameClient* client=GetClient();
		MiniMapInterface* miniMap=client->MiniMap();
		if (!miniMap)
			throw std::logic_error("Actionbar416C17: actual MiniMap owner required");
		miniMap->Refresh();
	}
	void Feedback(const std::string& text, DWORD red, DWORD green, DWORD blue)
	{ DisplayClientFeedback(text,red,green,blue,std::string()); }
	void QueueCommand(const char* command) { GetClient()->child0x7C->Queue(command); }
} }

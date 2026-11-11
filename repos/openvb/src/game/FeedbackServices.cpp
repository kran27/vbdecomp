#include "game/FeedbackServices.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/ActionbarInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include <stdexcept>

namespace F3
{
	FeedbackInterface* GetClientFeedback(JE::Client* client)
	{
		if (!client) return NULL;
		GameClient* game=dynamic_cast<GameClient*>(client);
		return game ? game->Feedback() : NULL;
	}
	ActionbarInterface* GetFeedbackActionbar()
	{
		GameClient* game=dynamic_cast<GameClient*>(g_pNetworkInstance->client);
		if (!game || !game->Actionbar())
			throw std::logic_error("Feedback4341AA: actual GameInterfaces Actionbar owner required");
		return game->Actionbar();
	}
	void DisplayClientFeedback(std::string text,DWORD red,DWORD green,DWORD blue,std::string speaker)
	{
		FeedbackInterface* feedback=GetClientFeedback(static_cast<JE::Client*>(g_pNetworkInstance->client));
		if (!feedback)
			throw std::logic_error("Feedback434B70: actual client feedback owner required");
		feedback->AddMessage(text,red,green,blue,speaker);
	}
}

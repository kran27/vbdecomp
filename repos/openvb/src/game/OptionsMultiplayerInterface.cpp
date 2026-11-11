#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/OptionsMultiplayerInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "gui/Event.hpp"

namespace F3
{
	OptionsMultiplayerInterface::OptionsMultiplayerInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name) //4491F0.
	{
		LoadResource("Options_Multiplayer.INT");
		rootWindow = FindWindow("picBackground");
	}

	OptionsMultiplayerInterface::~OptionsMultiplayerInterface() //449100.
	{
		//No extra owner:the original goes directly to GameInterface destruction.
	}

	int OptionsMultiplayerInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		//Raw449130..449191:even event6 goes directly to the ordinary Engine tail.
		//No Multiplayer button-specific backend behavior is present.
		switch (event->type)
		{
		case 6:
			break; //Recognized prototype event, without button-specific behavior.
		case 17:
			if (window == rootWindow)
			{
				switch (event->x)
				{
				case 1:
					{
						OptionsInterface* options = static_cast<GameInterfaces*>(static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0)->options;
						options->CloseChildren();
						options->SetHidden(1);
						return 1;
					}
				}
			}
			break;
		}
		return JE::EngineInterface::OnEvent(window, event);
	}
}

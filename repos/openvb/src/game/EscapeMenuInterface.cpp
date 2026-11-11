#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/EscapeMenuInterface.hpp"
#include "game/MainMenuServices.hpp"
#include "network/CommandProcessor.hpp"
#include "audio/Audio.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "gui/Window.hpp"
#include <cstring>

namespace F3
{
	EscapeMenuInterface::EscapeMenuInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name)
	{
		LoadResource("EscapeMenu.int");
		rootWindow = FindWindow("EscapeMenuWin");
	}
	EscapeMenuInterface::~EscapeMenuInterface() {}

	int EscapeMenuInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		if (GetHidden()) return JE::EngineInterface::OnEvent(window, event);
		if (event->type == 6)
		{
			if (window->type == 1)
			{
				if (!_stricmp(window->GetName(), "QuitBtn"))
				{
					//433F63 passes only2000;590990 forwards to the actual music
					//voice's4C79E0 stop/fade before the GUI and map transition.
					JE::Audio* audio = static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x70;
					if (audio)
						audio->StopMusic(2000);
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
					ReturnToMainMenu();
				}
				else if (!_stricmp(window->GetName(), "ContinueBtn"))
				{
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
				}
				else if (!_stricmp(window->GetName(), "OptionsBtn"))
				{
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
					static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C->Queue("GameOptions");
				}
				else if (!_stricmp(window->GetName(), "LoadBtn")
					|| !_stricmp(window->GetName(), "SaveBtn"))
				{
					SetHidden(1);
					::GUI::g_Manager.RemoveModal(this);
					static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C->Queue("LoadGameOptions");
				}
			}
		}
		else if (event->type == 17 && window == rootWindow && event->x == 1)
		{
			SetHidden(1);
			::GUI::g_Manager.RemoveModal(this);
		}
		return JE::EngineInterface::OnEvent(window, event);
	}
}

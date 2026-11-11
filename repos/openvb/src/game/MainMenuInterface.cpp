#include "startup/GameClient.hpp"
#include "game/MainMenuInterface.hpp"
#include "game/MainMenuServices.hpp"
#include "game/CharacterGenerationInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/CommandProcessor.hpp"
#include "startup/Window.hpp"
#include "gui/Event.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/Window.hpp"
#include <cstring>

namespace F3
{
	void MainMenuInterface::PrepareForOpen() //Native461904 calls404B70 with the unadjusted view.
	{
	}

	MainMenuInterface::MainMenuInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name), singleMulti(0) //440990;store at440A0C is a BYTE.
	{
		//Original ignores LoadResource's BYTE result and publishes the actual lookup.
		LoadResource("Mainmenu.INT");
		mainWindow = FindWindow("MainmenuWin");
		//Parent4398E0 hides its Mainmenu after construction;the ctor does not.
	}

	MainMenuInterface::~MainMenuInterface() //440910.
	{
		//No added owner:actual ordinary phase delegates to GameInterface lifetime.
	}

	void MainMenuInterface::RefreshSingleMulti() //440AF0.
	{
		::GUI::Label* singleMultiLabel = static_cast<::GUI::Label*>(FindWindow("lblSingleMulti"));
		if (singleMultiLabel)
			singleMultiLabel->SetStringReference(singleMulti ? 2 : 1);
		::GUI::Label* newGameLabel = static_cast<::GUI::Label*>(FindWindow("lblNewGame"));
		if (newGameLabel)
			newGameLabel->SetStringReference(singleMulti ? 5 : 3);
		::GUI::Label* loadGameLabel = static_cast<::GUI::Label*>(FindWindow("lblLoadGame"));
		if (loadGameLabel)
			loadGameLabel->SetStringReference(singleMulti ? 6 : 4);
		//Keep each live BYTE reload after its lookup and preceding callbacks.
	}

	int MainMenuInterface::OnEvent(::GUI::Window* window, ::GUI::Event* event)
	{
		//Logical440CE0 reached through compiler adjustor440FB0. No hidden-page gate.
		if (event->type == 6)
		{
			if (_stricmp(window->GetName(), "SingleMultiBtn") == 0)
			{
				singleMulti = singleMulti == 0;
				RefreshSingleMulti();
			}
			else if (_stricmp(window->GetName(), "NewGameBtn") == 0)
			{
				if (static_cast<WORD>(::GetAsyncKeyState(VK_SHIFT)) & 0x8000)
				{
					if (!singleMulti)
					{
						static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C->Queue("RequestNewGame");
						SetHidden(1);
						::GUI::g_Manager.RemoveModal(this);
					}
					//Shift+nonzero mode is the original no-action branch.
				}
				else
				{
					static_cast<JE::Client*>(g_pNetworkInstance->client)->FadeOut(1000, 0, 0, 0);
					::GUI::g_Manager.RemoveModal(this);
					SetHidden(1);
					GameInterfaces* interfaces = static_cast<GameInterfaces*>(
						static_cast<JE::Client*>(g_pNetworkInstance->client)->child0xA0);
					interfaces->characterGeneration->Open();
					static_cast<JE::Client*>(g_pNetworkInstance->client)->FadeIn(1000, 0, 0, 0);
				}
			}
			else if (_stricmp(window->GetName(), "LoadGameBtn") == 0)
			{
				if (!singleMulti)
					static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C->Queue("LoadMap");
			}
			else if (_stricmp(window->GetName(), "OptionsBtn") == 0)
			{
				//Original recognized prototype has no Options action.
			}
			else if (_stricmp(window->GetName(), "CinematicsBtn") == 0)
			{
				//Original recognized prototype has no Cinematics action.
			}
			else if (_stricmp(window->GetName(), "QuitBtn") == 0)
			{
				F3::Quit(); //56AFF0:PostQuitMessage(1),then modal removal.
				::GUI::g_Manager.RemoveModal(this);
			}
		}
		else if (event->type == 17 && window == mainWindow && event->x == 41)
		{
			const std::string command("ConsoleWindow");
			const char* text = command.c_str();
			JE::ConsoleCommandProcessor* processor =
				static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x7C;
			processor->Queue(text);
		}

		//Every returning branch reaches the actual Engine owner with the live event.
		return JE::EngineInterface::OnEvent(window, event);
	}
}

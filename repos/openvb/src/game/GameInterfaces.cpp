#include <winsock2.h>
#include "game/GameInterfaces.hpp"
#include "game/MainMenuInterface.hpp"
#include "game/LoadMapInterface.hpp"
#include "game/CharacterGenerationInterface.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/FeedbackInterface.hpp"
#include "game/InventoryInterface.hpp"
#include "game/EscapeMenuInterface.hpp"
#include "game/PipBoyInterface.hpp"
#include "game/LoadingBarInterface.hpp"
#include "game/MiniMapInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "game/OptionsLoadInterface.hpp"
#include "game/GameOptionsInterface.hpp"
#include "game/HotKeysOptionsInterface.hpp"
#include "game/OptionsMultiplayerInterface.hpp"
#include "game/OptionsVideoInterface.hpp"
#include "gui/WorldContainerInterface.hpp"
#include "gui/Manager.hpp"
#include "graphics/ClientCursor.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"

namespace F3
{
	GameInterfaces::GameInterfaces() : mainMenu(NULL), loadMap(NULL), feedback(NULL),
		actionbar(NULL), auxiliary14(NULL), inventory(NULL), escapeMenu(NULL), loading(NULL),
		auxiliary24(NULL), container(NULL), value2C(0), auxiliary30(NULL), value34(0),
		characterGeneration(NULL), auxiliary3C(NULL), auxiliary40(NULL), pipBoy(NULL),
		miniMap(NULL), options(NULL), optionsLoad(NULL), optionsGame(NULL), optionsHotKeys(NULL),
		optionsMultiplayer(NULL), optionsVideo(NULL)
	{
		//Publish and hide each required child before acquiring the next one.
		mainMenu = new MainMenuInterface("Mainmenu"); mainMenu->SetHidden(1);
		loadMap = new LoadMapInterface("LoadMap"); loadMap->SetHidden(1);
		characterGeneration = new CharacterGenerationInterface; characterGeneration->SetHidden(1);
		actionbar = new ActionbarInterface("Actionbar"); actionbar->SetHidden(1);
		feedback = new FeedbackInterface("Feedback"); feedback->SetHidden(1);
		inventory = new InventoryInterface("Inventory"); inventory->SetHidden(1);
		escapeMenu = new EscapeMenuInterface("EscapeMenu"); escapeMenu->SetHidden(1);
		pipBoy = new PipBoyInterface("PipBoy"); pipBoy->SetHidden(1);
		loading = new LoadingBarInterface("LoadingBar"); loading->SetHidden(1);
		miniMap = new MiniMapInterface("MiniMap"); miniMap->SetHidden(1);
		options = new OptionsInterface("Options"); options->SetHidden(1);
		optionsLoad = new OptionsLoadInterface("OptionsLoad"); optionsLoad->SetHidden(1);
		optionsGame = new GameOptionsInterface("OptionsGame"); optionsGame->SetHidden(1);
		optionsHotKeys = new HotKeysOptionsInterface("OptionsHotKeys"); optionsHotKeys->SetHidden(1);
		optionsMultiplayer = new OptionsMultiplayerInterface("OptionsMultiplayer"); optionsMultiplayer->SetHidden(1);
		optionsVideo = new OptionsVideoInterface("OptionsVideo"); optionsVideo->SetHidden(1);
		container = new JE::WorldContainerInterface("Container"); container->SetHidden(1);
	}
	GameInterfaces::~GameInterfaces()
	{
		//4396B0 only runs the base phase. Preserve the original omitted deletes.
	}
	void GameInterfaces::Initialize()
	{
		if (feedback)
		{
			feedback->Resize(280, 132);
			feedback->SetAbsolutePosition(10, 620);
		}
	}
	void GameInterfaces::Activate()
	{
		::GUI::g_Manager.PushModal(mainMenu);
	}
	void GameInterfaces::SetLoadingVisible(DWORD visible)
	{
		if (loading)
		{
			if (BYTE(visible))
			{
				loading->SetHidden(0);
				JE::Interfaces::SetLoadingVisible(visible);
			}
			else
			{
				loading->SetHidden(1);
				JE::Interfaces::SetLoadingVisible(visible);
			}
		}
	}
	void GameInterfaces::ShowGameplay()
	{
		JE::Client* baseClient = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameClient* client = dynamic_cast<GameClient*>(baseClient);
		static_cast<GameCursorManager*>(client->child0xA4)->SetContext(1);
		if (actionbar)
		{
			const DWORD selected = static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0;
			actionbar->SetHidden(0);
			actionbar->RefreshEntity(selected);
		}
		JE::Interfaces::ShowGameplay();
	}
	void GameInterfaces::HideGameplay()
	{
		if (actionbar)
		{
			actionbar->SetHidden(1);
			actionbar->HideCompanions();
		}
		JE::Interfaces::HideGameplay();
	}
	void GameInterfaces::RefreshSelection()
	{
		if (actionbar && !actionbar->GetHidden())
			actionbar->SelectEntity(static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0);
	}
	void GameInterfaces::GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height)
	{
		JE::Interfaces::GetScreenRectangle(x, y, width, height);
		if (actionbar && !actionbar->GetHidden())
		{
			LONG barX, barY, barWidth, barHeight;
			actionbar->GetWindowRectangle(barX, barY, barWidth, barHeight);
			height = LONG(DWORD(height) + DWORD(barY) - 768);
		}
	}
	void GameInterfaces::OnUnusedGame() {}
}

#pragma once

#include "gui/Interfaces.hpp"
#include "game/InterfaceMessageScreen.hpp"
#include "game/StoreInventoryScreen.hpp"
#include "game/StoreCatalogueScreen.hpp"

namespace GUI { class Interface; }
namespace JE { class WorldContainerInterface; }

class COptionsGameInterface;

namespace F3
{
	class MainMenuInterface;
	class LoadMapInterface;
	class FeedbackInterface;
	class ActionbarInterface;
	class InventoryInterface;
	class EscapeMenuInterface;
	class LoadingBarInterface;
	class CharacterGenerationInterface;
	class PipBoyInterface;
	class MiniMapInterface;
	class OptionsInterface;
	class OptionsLoadInterface;
	typedef ::COptionsGameInterface GameOptionsInterface;
	class HotKeysOptionsInterface;
	class OptionsMultiplayerInterface;
	class OptionsVideoInterface;

	class GameInterfaces : public JE::Interfaces
	{
	public:
		GameInterfaces(); //4398E0, all seventeen actual constructor children.
		virtual ~GameInterfaces(); //4396B0, deliberately omits child deletion.
		virtual void Initialize(); //4396C0.
		virtual void Activate(); //4396F0.
		virtual void SetLoadingVisible(DWORD visible); //439720.
		virtual void ShowGameplay(); //439770.
		virtual void HideGameplay(); //4397E0.
		virtual void RefreshSelection(); //439810.
		virtual void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height); //439850.
		virtual void OnUnusedGame(); //404B70, additional original slot28.

		MainMenuInterface* mainMenu; //Original+04.
		LoadMapInterface* loadMap; //+08.
		FeedbackInterface* feedback; //+0C.
		ActionbarInterface* actionbar; //+10.
		::GUI::Interface* auxiliary14; //+14, constructor NULL; producer unresolved.
		InventoryInterface* inventory; //+18.
		EscapeMenuInterface* escapeMenu; //+1C.
		LoadingBarInterface* loading; //+20.
		void* auxiliary24; //+24, texture-owning producer/type unresolved.
		JE::WorldContainerInterface* container; //+28.
		DWORD value2C; //+2C, constructor zero; meaning unresolved.
		InterfaceMessageScreen* auxiliary30; //+30; borrowed partial view, constructor NULL.
		DWORD value34; //+34, constructor zero; meaning unresolved.
		CharacterGenerationInterface* characterGeneration; //+38.
		StoreCatalogueScreen* auxiliary3C; //+3C, borrowed partial view; constructor NULL.
		StoreInventoryScreen* auxiliary40; //+40, complete borrowed pointer; constructor NULL.
		PipBoyInterface* pipBoy; //+44.
		MiniMapInterface* miniMap; //+48.
		OptionsInterface* options; //+4C.
		OptionsLoadInterface* optionsLoad; //+50.
		GameOptionsInterface* optionsGame; //+54.
		HotKeysOptionsInterface* optionsHotKeys; //+58.
		OptionsMultiplayerInterface* optionsMultiplayer; //+5C.
		OptionsVideoInterface* optionsVideo; //+60.
	};
}

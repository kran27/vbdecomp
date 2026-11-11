#include <winsock2.h>
#include "game/GameFloatingTextInterface.hpp"
#include "game/GameInterface.hpp"
#include "game/GameInterfaces.hpp"
#include "game/InventoryInterface.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/OptionsInterface.hpp"
#include "game/PipBoyInterface.hpp"
#include "startup/NetworkComponent.hpp"
#include "network/EngineClient.hpp"
#include "gui/DebugWindow.hpp"
#include "gui/Window.hpp"
#include "entity/Entity.hpp"
#include <cstdio>

namespace F3
{
	GameFloatingTextInterface::GameFloatingTextInterface(const std::string& name)
		: JE::EngineInterface(name), GameInterface(name), JE::FloatingTextInterface(name)
	{
		speech.profile = "quickslot.ini"; speech.font = "fallouty";
		speech.pointSize = 9; speech.fontFlags = 0;
		speech.red = 128; speech.green = 128; speech.blue = 128; speech.duration = 2000;
		speech.speed = 0; speech.fadeOut = 250; speech.fadeIn = 250;
		speech.width = 200; speech.height = 11; speech.wrap = 1; speech.alignment = 0;
		speech.backgroundProfile = "quickslot.ini"; speech.backgroundAlpha = 150;
		speech.paddingLeft = 8; speech.paddingRight = 8; speech.paddingTop = 4; speech.paddingBottom = 4;
		damage.profile = "quickslot.ini"; damage.font = "arial";
		damage.pointSize = 14; damage.fontFlags = 1;
		damage.red = 128; damage.green = 128; damage.blue = 0; damage.duration = 2000;
		damage.speed = 40; damage.fadeOut = 250; damage.fadeIn = 0;
		damage.width = 200; damage.height = 24; damage.wrap = 0; damage.alignment = 2;
	}
	GameFloatingTextInterface::~GameFloatingTextInterface() {}
	void GameFloatingTextInterface::AddSpeech(DWORD id, const std::string& text, DWORD duration,
		DWORD red, DWORD green, DWORD blue)
	{
		JE::FloatingTextInterface::AddSpeech(id, text, duration, red, green, blue);
		JE::Entity* entity = JE::GetEntityPtr(id);
		std::string name("");
		if (entity) name = entity->displayName;
		std::string output;
		char time[32]; std::sprintf(time, " %dms ", static_cast<LONG>(duration));
		output = name + std::string(time) + text;
		static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x8C->Write(output, 128, 128, 128);
	}
	void GameFloatingTextInterface::AfterPosition(GUI::FloatingLabel* label,
		LONG /*x*/, LONG y, LONG /*width*/, LONG height)
	{
		JE::Client* client = static_cast<JE::Client*>(g_pNetworkInstance->client);
		GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
		//438F8F..438FA2 captures all four pointers before the first GUI callback.
		InventoryInterface* inventory = interfaces->inventory;
		ActionbarInterface* actionbar = interfaces->actionbar;
		OptionsInterface* options = interfaces->options;
		PipBoyInterface* pipBoy = interfaces->pipBoy;
		if (inventory && !inventory->GetHidden())
		{ label->SetHidden(1); label->duration = 0; }
		if (pipBoy && !pipBoy->GetHidden())
		{ label->SetHidden(1); label->duration = 0; }
		if (options && !options->GetHidden())
		{ label->SetHidden(1); label->duration = 0; }
		if (actionbar && !actionbar->GetHidden())
		{
			LONG x, top;
			actionbar->rootWindow->GetAbsolutePosition(x, top); //Position only, original virtual+48.
			if (y + height >= top)
			{ label->SetHidden(1); label->duration = 0; }
		}
	}
}

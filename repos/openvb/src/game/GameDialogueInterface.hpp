#pragma once

#include "game/GameInterface.hpp"
#include "gui/DialogueInterface.hpp"

namespace F3
{
	class GameDialogueInterface : public GameInterface, public JE::DialogueInterface
	{
	public:
		explicit GameDialogueInterface(std::string name); //438080.
		virtual JE::DialogueWindow* CreateDialogueWindow(BYTE id, BYTE revision); //437FE0.
	};
}

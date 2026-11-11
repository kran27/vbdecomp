#pragma once

#include "gui/DialogueWindow.hpp"

namespace F3
{
	class GameDialogueWindow : public JE::DialogueWindow
	{
	public:
		GameDialogueWindow(BYTE id, BYTE revision, JE::DialogueInterface* source); //437ED0.
		//Implicit virtual destruction forwards to DialogueWindow (438070).
		void SetParticipants(DWORD listenerID, DWORD speakerID); //437F00.
	};
}

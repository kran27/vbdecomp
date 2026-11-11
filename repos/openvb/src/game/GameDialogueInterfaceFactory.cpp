#include "game/GameDialogueInterface.hpp"
#include "game/GameDialogueWindow.hpp"

namespace F3
{
	JE::DialogueWindow* GameDialogueInterface::CreateDialogueWindow(BYTE id, BYTE revision) //437FE0.
	{
		//Actual4A7680 is inherited GUI::HeapTracker allocation. Ordinary tracked new
		//uses real C++ constructor-failure cleanup; no custom old EH/allocator code.
		return new GameDialogueWindow(id, revision, this);
	}
}

#pragma once

#include <windows.h>

namespace JE
{
	//Explicit outer HUD screen-rectangle dependency of the recovered dialogue.
	//The complete Client+A0 owner remains separate from the runnable scene.
	class DialogueScreenServices
	{
	public:
		virtual ~DialogueScreenServices() {}
		virtual void GetScreenRectangle(LONG& x, LONG& y, LONG& width, LONG& height) = 0;
	};
	DialogueScreenServices* SetDialogueScreenServices(DialogueScreenServices* services);
}

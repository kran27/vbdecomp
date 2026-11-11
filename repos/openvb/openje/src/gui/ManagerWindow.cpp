#include "Manager.hpp"
#include "Window.hpp"

namespace GUI
{
	namespace
	{
		// Original writable pointer table6FAF6C; five read-only literal targets.
		// Original lookup has no range check. Ordinary factory inputs are0..4.
		const char* fragmentTypes[] = { "Window", "Button", "Picture", "Label", "Edit" };
	}

	const char* Manager::GetFragmentType(DWORD index)
	{
		return fragmentTypes[index];
	}

	void Manager::ForgetWindow(Window* window)
	{
		if (capturedWindow && window == capturedWindow)
			SetCapture(NULL);
		if (hoveredWindow == window)
		{
			hoveredWindow = NULL;
			window->SetHidden(1);
			ProcessMouse(&mouse, 0);
		}
	}
}

#include "Manager.hpp"
#include "Window.hpp"

namespace GUI
{
	BYTE Manager::SetCapture(Window* next)
	{
		Window* previous = capturedWindow;
		if (next == previous)
			return 0;
		if (previous)
			previous->OnCaptureChanged(next);
		capturedWindow = next;
		return 1;
	}
}

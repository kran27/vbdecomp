#include "Window.hpp"

namespace GUI
{
	void Window::SetHidden(BYTE value)
	{
		hidden = value;
	}

	BYTE Window::GetHidden()
	{
		return hidden;
	}

	void Window::OnCaptureChanged(Window*)
	{
		// Actual base Window table+118 uses the shared empty RET4 at5A9320.
	}
}

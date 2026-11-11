#include "Window.hpp"

namespace GUI
{
	const char* Window::GetName()
	{
		return name.c_str(); // 4AF890, real string storage policy.
	}

	void Window::SetValue3B(BYTE value)
	{
		value3B = value; // 4AF4D0 preserves the raw byte.
	}

	void Window::SetMaterialModes(DWORD value)
	{
		materialModes = value; // 4AF290; reload the member for every child call.
		for (int i = 0; i < 9; ++i) fragments[i].SetMaterialModes(materialModes);
	}
}

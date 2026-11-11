#include "gui/Window.hpp"
#include "gui/Manager.hpp"

namespace GUI
{
	void Manager::SetStringCallback(StringCallback callback) //4B2AC0.
	{
		callback172 = callback;
	}

	BYTE Manager::ResolveString(DWORD reference, std::string& output) //4B4180.
	{
		if (callback172) return callback172(reference, output);
		output = "Beetle"; //Original default literal, before the callback is installed.
		return 1;
	}

	std::string Window::GetStringCB8() //4B0040.
	{
		if (!stringCB8.empty()) return stringCB8;
		std::string resolved;
		if (valueCD4 != DWORD(-1) && g_Manager.ResolveString(valueCD4, resolved))
			return resolved;
		return name;
	}
}

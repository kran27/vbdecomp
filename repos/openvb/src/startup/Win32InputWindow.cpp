#include "gui/Win32Input.hpp"
#include "startup/Window.hpp"
#include "startup/ConfigurationObject.hpp"

namespace JE { namespace Input { namespace Win32InputServices
{
	HWND Window() { return F3::g_window; }
	const Configuration::Settings& MainSettings() { return *F3::GetSettingsObject(); }
} } }

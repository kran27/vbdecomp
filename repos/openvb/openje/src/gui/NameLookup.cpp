#include "gui/Interface.hpp"
#include "gui/Window.hpp"

namespace GUI
{
	//495740: root-first recursive lookup, preserving the live list order.
	Window* Interface::FindWindow(const std::string& name)
	{
		for (std::list<Window*>::iterator entry = roots.begin(); entry != roots.end(); ++entry)
		{
			Window* window = *entry;
			const std::string current(window->GetName());
			if (current == name) return window;
			Window* result = window->FindWindow(name);
			if (result) return result;
		}
		return NULL;
	}

	//4B02E0: excludes this Window; caller checks it before recursion.
	Window* Window::FindWindow(const std::string& name)
	{
		for (std::list<Window*>::iterator entry = children.begin(); entry != children.end(); ++entry)
		{
			Window* window = *entry;
			const std::string current(window->GetName());
			if (current == name) return window;
			Window* result = window->FindWindow(name);
			if (result) return result;
		}
		return NULL;
	}
}

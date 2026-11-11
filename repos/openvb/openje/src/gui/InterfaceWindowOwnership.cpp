#include "Interface.hpp"
#include "Window.hpp"
#include <algorithm>

namespace GUI
{
	void Interface::AddWindow(Window* window)
	{
		if (window->GetOwner() != this)
			window->SetOwner(this);
		for (std::list<Window*>::iterator entry = roots.begin();
			entry != roots.end(); ++entry)
		{
			if (window == *entry)
			{
				roots.erase(entry);
				break;
			}
		}
		roots.push_front(window);
	}

	void Interface::RemoveWindow(Window* window)
	{
		for (std::list<Window*>::iterator entry = roots.begin();
			entry != roots.end(); ++entry)
		{
			if (window == *entry)
			{
				roots.erase(entry);
				window->SetOwner(NULL);
				return;
			}
		}
	}
}

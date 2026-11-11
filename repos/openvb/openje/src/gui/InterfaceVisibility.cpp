#include "Interface.hpp"
#include "Manager.hpp"
#include "Window.hpp"
#include <algorithm>

namespace GUI
{
	void Interface::SetHidden(BYTE value)
	{
		hidden = value;
		g_Manager.MarkDepthDirty();
		if (hidden)
		{
			if (g_Manager.capturedWindow)
			{
				Window* window = g_Manager.capturedWindow;
				Interface* capturedOwner = NULL;
				while (window)
				{
					capturedOwner = window->GetOwner();
					if (capturedOwner)
						break;
					window = window->GetParent();
				}
				if (capturedOwner == this)
					g_Manager.SetCapture(NULL);
			}
		}
	}

	void Interface::BringToFront()
	{
		g_Manager.BringToFront(this);
	}
	void Interface::BringWindowToFront(Window* window)
	{
		for (std::list<Window*>::iterator entry = roots.begin();
			entry != roots.end(); ++entry)
		{
			if (*entry == window)
			{
				roots.erase(entry);
				roots.push_front(window);
				g_Manager.MarkDepthDirty();
				return;
			}
		}
	}
}

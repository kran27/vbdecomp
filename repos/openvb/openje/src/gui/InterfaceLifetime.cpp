#include "Interface.hpp"
#include "Manager.hpp"
#include "Window.hpp"

namespace GUI
{
	Interface::Interface(const std::string& value)
		: name(value), hidden(0), pendingDestroy(0)
	{
		g_Manager.RegisterInterface(this);
	}

	Interface::~Interface()
	{
		Cleanup();
	}

	void Interface::Cleanup()
	{
		std::list<Window*>::iterator entry = roots.begin();
		while (entry != roots.end())
		{
			Window* window = *entry;
			entry = roots.erase(entry);
			// Original queries the secondary HeapTracker receiver before NULL.
			// Keep that order; a NULL member call is outside the C++ source domain.
			if (window->IsHeapAllocated() && window)
				delete window;
		}
		g_Manager.RemoveModal(this);
		g_Manager.UnregisterInterface(this);
	}

	void Interface::BeforeUpdate()
	{
	}

	void Interface::BeforeDraw()
	{
	}

	BYTE Interface::OnCursor(Window*, LONG, LONG)
	{
		return 0;
	}
}

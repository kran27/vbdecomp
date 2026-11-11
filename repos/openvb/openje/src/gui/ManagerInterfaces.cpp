#include "Manager.hpp"
#include "Window.hpp"

namespace GUI
{
	void Manager::RegisterInterface(Interface* value)
	{
		for (std::list<Interface*>::iterator entry = interfaces.begin();
			entry != interfaces.end(); ++entry)
		{
			if (value == *entry)
			{
				interfaces.erase(entry);
				break;
			}
		}
		interfaces.push_front(value);
		MarkDepthDirty();
	}

	void Manager::UnregisterInterface(Interface* value)
	{
		Window* window = capturedWindow;
		if (window)
		{
			Interface* capturedOwner = NULL;
			while (window)
			{
				capturedOwner = window->GetOwner();
				if (capturedOwner)
					break;
				window = window->GetParent();
			}
			if (capturedOwner && capturedOwner == value)
				SetCapture(NULL);
		}
		interfaces.remove(value);
		MarkDepthDirty();
		if (hoveredOwner == value)
		{
			hoveredOwner = NULL;
			ProcessMouse(&mouse, 0);
		}
	}
}

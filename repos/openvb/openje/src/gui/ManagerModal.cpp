#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "gui/Window.hpp"
#include "gui/Event.hpp"

namespace GUI
{
	void Manager::PushModal(Interface* value) //4B4F00.
	{
		Window* ancestor = capturedWindow;
		while (ancestor)
		{
			Interface* capturedOwner = ancestor->GetOwner();
			if (capturedOwner)
			{
				if (capturedOwner != value) SetCapture(NULL);
				break;
			}
			ancestor = ancestor->GetParent();
		}
		if (hoveredOwner && hoveredWindow)
		{
			Event event;
			event.buttons = mouse.buttons;
			event.x = mouse.x;
			event.y = mouse.y;
			event.wheel = mouse.wheel;
			event.type = 3;
			//Original4B4F52..4B4F74 leaves the final modifiers word unwritten.
			DispatchEvent(&event, hoveredOwner, hoveredWindow);
		}
		modal.push_front(value); //Duplicates and NULL values are not filtered.
	}
}

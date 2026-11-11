#include "GraphicsInput.hpp"

#include <stdexcept>

namespace JE
{
	namespace GUI
	{
		BYTE DispatchGraphicsInput(const Input::InputEvent& event, GraphicsInputServices& services)
		{
			if (event.category == 0)
			{
				if (event.subtype == 0)
					return services.Key(event.x, 1, event.modifiers);
				if (event.subtype == 1)
					return services.Key(event.x, 0, event.modifiers);
			}
			else if (event.category == 1)
				return services.Character(event.character, event.modifiers);
			else if (event.category == 2)
			{
				MenuMouseState mouse = {event.x, event.y, 0, event.wheel};
				if (event.left) mouse.buttons |= 1;
				if (event.middle) mouse.buttons |= 2;
				if (event.right) mouse.buttons |= 4;
				if (event.subtype == 3) mouse.buttons <<= 16;
				return services.Mouse(mouse, event.modifiers);
			}
			return 0;
		}

		BYTE GraphicsInputServices::Key(LONG, BYTE, DWORD)
		{
			throw std::logic_error("Unrecovered GUI keyboard consumer0x4B40A0");
		}

		BYTE GraphicsInputServices::Character(BYTE, DWORD)
		{
			throw std::logic_error("Unrecovered GUI character consumer0x4B40F0");
		}
	}
}

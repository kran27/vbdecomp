// Original Manager ProcessMouse4B3040/1001. Real VC7 conversion/containers.
#include "gui/Manager.hpp"
#include "gui/Interface.hpp"
#include "gui/Window.hpp"
#include "gui/Event.hpp"
#include <cstddef>

namespace GUI
{
#if defined(_MSC_VER) && _MSC_VER == 1300
	typedef char ManagerCursorOriginalSize[(sizeof(Manager) == 0xBC) ? 1 : -1];
	typedef char ManagerCursorOriginalList[(offsetof(Manager, interfaces) == 0x1C) ? 1 : -1];
	typedef char ManagerCursorOriginalScaleX[(offsetof(Manager, scaleX) == 0x68) ? 1 : -1];
	typedef char ManagerCursorOriginalScaleY[(offsetof(Manager, scaleY) == 0x6C) ? 1 : -1];
	typedef char ManagerCursorOriginalCapture[(offsetof(Manager, capturedWindow) == 0xB8) ? 1 : -1];
#endif

	Window* Manager::FindCursorWindow(LONG x, LONG y) //4B2C20.
	{
		const LONG scaledX = static_cast<LONG>(static_cast<double>(x) / scaleX);
		const LONG scaledY = static_cast<LONG>(static_cast<double>(y) / scaleY);
		if (capturedWindow) return capturedWindow;
		for (std::list<Interface*>::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
		{
			Window* window = (*i)->FindAt(scaledX, scaledY, 0);
			if (window) return window;
		}
		return NULL;
	}

	BYTE Manager::ProcessCursor(HWND window) //4B2D40.
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(window, &point);
		Window* target = FindCursorWindow(point.x, point.y);
		Window* ancestor = target;
		if (!target) return 0;
		while (ancestor)
		{
			Interface* owner = ancestor->GetOwner();
			if (owner) return owner->OnCursor(target, point.x, point.y);
			ancestor = ancestor->GetParent();
		}
		return 0;
	}

	BYTE Manager::ProcessMouse(MouseState* state, DWORD modifiers)
	{
		BYTE entered = 0;
		if (inputBlocked)
			return 0;
		const MouseState previous = mouse;
		mouse = *state;
		const LONG x = static_cast<LONG>(static_cast<double>(mouse.x) / scaleX);
		const double yValue = static_cast<double>(mouse.y) / scaleY;
		mouse.x = x;
		const LONG y = static_cast<LONG>(yValue);
		mouse.y = y;
		BYTE doubleClick = 0;
		if (mouse.buttons & 0xFFFF0000)
		{
			doubleClick = 1;
			mouse.buttons = static_cast<LONG>(mouse.buttons) >> 16;
		}

		Event event;
		event.x = x;
		event.y = y;
		event.buttons = mouse.buttons;
		event.wheel = mouse.wheel;
		event.modifiers = modifiers;
		Interface* owner = NULL;
		Window* target = NULL;
		if (capturedWindow)
		{
			Window* ancestor = capturedWindow;
			while (ancestor)
			{
				owner = ancestor->GetOwner();
				if (owner)
				{
					target = capturedWindow;
					break;
				}
				ancestor = ancestor->GetParent();
			}
		}
		else if (!modal.empty())
		{
			owner = modal.front();
			target = owner->FindAt(x, y, 0);
			if (!target)
				target = owner->GetRoots()->front();
		}
		else
		{
			for (std::list<Interface*>::iterator entry = interfaces.begin(); entry != interfaces.end(); ++entry)
			{
				owner = *entry;
				target = owner->FindAt(mouse.x, mouse.y, 0);
				if (target)
					break;
			}
		}

		if (capturedWindow)
		{
			if (target && owner && target == owner->FindAt(mouse.x, mouse.y, 0))
			{
				if (!hoveredWindow)
				{
					event.type = 1;
					DispatchEvent(&event, owner, target);
					entered = 1;
				}
				hoveredWindow = target;
				hoveredOwner = owner;
			}
			else
			{
				if (hoveredWindow)
				{
					event.type = 3;
					DispatchEvent(&event, hoveredOwner, hoveredWindow);
				}
				hoveredWindow = NULL;
				hoveredOwner = NULL;
			}
		}
		else
		{
			if (hoveredWindow && target != hoveredWindow)
			{
				event.type = 3;
				DispatchEvent(&event, hoveredOwner, hoveredWindow);
			}
			if (target && target != hoveredWindow)
			{
				event.type = 1;
				DispatchEvent(&event, owner, target);
				entered = 1;
			}
			hoveredWindow = target;
			hoveredOwner = owner;
		}

		const DWORD buttons = mouse.buttons;
		const DWORD left = buttons & 1;
		if (left != (previous.buttons & 1) && left)
			pressedLeft = target;
		if ((buttons & 2) != (previous.buttons & 2) && (buttons & 2))
			pressedRight = target;
		if ((buttons & 4) != (previous.buttons & 4) && (buttons & 4))
			pressedMiddle = target;
		if (!target)
			return entered;

		if (left != (previous.buttons & 1))
		{
			if (left)
			{
				event.type = 4;
				if (doubleClick)
					event.type = 7;
			}
			else
			{
				event.type = 5;
				if (pressedLeft == target)
				{
					DispatchEvent(&event, owner, target);
					event.type = 6;
				}
			}
			DispatchEvent(&event, owner, target);
		}
		const DWORD middle = mouse.buttons & 2;
		if (middle != (previous.buttons & 2))
		{
			if (middle)
			{
				event.type = 8;
				if (doubleClick)
					event.type = 11;
			}
			else
			{
				event.type = 9;
				if (pressedRight == target)
				{
					DispatchEvent(&event, owner, target);
					event.type = 10;
				}
			}
			DispatchEvent(&event, owner, target);
		}
		const DWORD right = mouse.buttons & 4;
		if (right != (previous.buttons & 4))
		{
			if (right)
			{
				event.type = 12;
				if (doubleClick)
					event.type = 15;
			}
			else
			{
				event.type = 13;
				if (pressedMiddle == target)
				{
					DispatchEvent(&event, owner, target);
					event.type = 14;
				}
			}
			DispatchEvent(&event, owner, target);
		}
		if (mouse.x != previous.x || mouse.y != previous.y)
		{
			event.type = 2;
			DispatchEvent(&event, owner, target);
		}
		if (mouse.wheel)
		{
			event.type = 16;
			DispatchEvent(&event, owner, target);
		}
		return 1;
	}
}

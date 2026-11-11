#include "Window.hpp"

namespace GUI
{
	void Window::SetAbsolutePosition(LONG x, LONG y)
	{
		Base::SetPosition(x, y);
	}

	void Window::GetAbsolutePosition(LONG& x, LONG& y)
	{
		Base::GetPosition(x, y);
	}
}

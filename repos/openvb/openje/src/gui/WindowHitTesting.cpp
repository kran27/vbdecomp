// Original Window Contains4AF170/23, GetValue3B4AF4E0/4 and FindAt4AF8F0/136.
#include "gui/Window.hpp"

namespace GUI
{
	bool Window::Contains(LONG x, LONG y)
	{
		return Base::Contains(x, y) ? true : false;
	}

	BYTE Window::GetValue3B()
	{
		return value3B;
	}

	Window* Window::FindAt(LONG x, LONG y, BYTE overrideSelf)
	{
		if (hidden)
			return NULL;
		for (std::list<Window*>::iterator entry = children.begin(); entry != children.end(); ++entry)
		{
			Window* result = (*entry)->FindAt(x, y, overrideSelf);
			if (result)
				return result;
		}
		if (GetValue3B() && !overrideSelf)
			return NULL;
		return Contains(x, y) ? this : NULL;
	}
}

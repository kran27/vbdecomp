#include "Window.hpp"

namespace GUI
{
	float Window::SetDepth(float depth, float increment)
	{
		for (std::list<Window*>::iterator entry = children.begin(); entry != children.end(); ++entry)
			depth = (*entry)->SetDepth(depth, increment) + increment;
		for (int index = 0; index < 9; ++index)
			fragments[index].SetDepth(depth, increment);
		return depth;
	}
}

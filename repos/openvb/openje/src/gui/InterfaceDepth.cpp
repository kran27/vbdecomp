#include "Interface.hpp"
#include "Window.hpp"

namespace GUI
{
	float Interface::SetDepth(float depth, float increment)
	{
		for (std::list<Window*>::iterator entry = roots.begin(); entry != roots.end(); ++entry)
			depth = (*entry)->SetDepth(depth, increment) + increment;
		if (!roots.empty())
			return depth - increment;
		return depth;
	}
}

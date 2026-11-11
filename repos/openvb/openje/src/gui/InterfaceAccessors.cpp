// Isolated actual Interface accessor candidates; no new virtual slots.
#include "gui/Interface.hpp"

namespace GUI
{
	// 0x495290/4.
	std::list<Window*>* Interface::GetRoots()
	{
		return &roots;
	}

	// Shared0x406A50/4; preserve the raw BYTE, not a normalized Boolean.
	BYTE Interface::GetHidden()
	{
		return hidden;
	}
}

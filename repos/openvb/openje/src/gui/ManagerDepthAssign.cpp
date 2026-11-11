// 0x4B35F0.
#include "gui/Manager.hpp"
#include "gui/Interface.hpp"

// The original subtraction and float store use the caller's rounding mode.



namespace GUI
{
	void Manager::AssignDepth()
	{
		if (!depthDirty) return;
		const float numerator = 0.25f - 0.005f; //Original FSTP32 before CountDepthSlots.
		const DWORD count = CountDepthSlots();
		const float increment = static_cast<float>(static_cast<double>(numerator) / static_cast<double>(count));
		float depth = 0.005f;
		std::list<Interface*>::iterator entry = interfaces.begin();
		while (entry != interfaces.end())
		{
			Interface* current = *entry;
			if (!current->GetHidden()) depth = current->SetDepth(depth, increment) + increment;
			// Reload the held node's next link after the callback, as in4B3675.
			++entry;
		}
		depthDirty = 0;
	}
}

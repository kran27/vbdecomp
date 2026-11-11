#include "PacketManager.hpp"

namespace GUI
{
	void CPktManager::Clear()
	{
		packets0.clear();
		packets1.clear();
		packets2.clear();
		packets3.clear();
		entries38.clear();
	}

	CPktManager::CPktManager()
	{
		optimizationLevel = 1;
		// The constructor itself performs these five operations; it does not
		// call the separate public Clear method in the original executable.
		packets0.clear();
		packets1.clear();
		packets2.clear();
		packets3.clear();
		entries38.clear();
	}

	CPktManager::~CPktManager()
	{
		// Real list members release only their own nodes, in reverse order.
	}
}

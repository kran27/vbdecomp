#include "Inventory.hpp"

namespace JE
{
	BYTE Inventory::ContainsResource(const std::string& name)
	{
		BYTE result = 0;
		for (std::list<Entity*>::iterator entry = items.begin(); entry != items.end(); ++entry)
		{
			//55FF78 tests the instance name at Entity+1C0. FindResource's
			//separate55FDA0 path searches the resource filename at+18C.
			if ((*entry)->displayName == name)
			{
				result = 1;
				break;
			}
		}
		return result;
	}
}

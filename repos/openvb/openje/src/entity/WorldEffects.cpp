#include "World.hpp"
#include "Condition.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>

namespace JE
{
	DWORD World::AllocateEffectID() //548600/46FDE0 ->46FDF0.
	{
		return (++value7C&0x1FFFFFFF)|0x60000000;
	}
	Effect* World::NewEffect(DWORD type) //544BF0.
	{
		if (type)
		{
			FatalError("World::CreateEffect() - Unknown effect type %d.\n",type);
			return NULL;
		}
		return new Condition(0);
	}
}

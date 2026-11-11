#include "Skinning.hpp"
#include <new>

Gfx_Skinning::~Gfx_Skinning()
{
	if (owned14) { ::operator delete[](owned14); owned14 = 0; }
	Free();
}

void Gfx_Skinning::FinishAllocation()
{
}

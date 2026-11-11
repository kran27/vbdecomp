#include "Skinning.hpp"
#include <new>

Gfx_Skinning::Gfx_Skinning()
	: value00(0), value04(0), value08(0), owned0C(0), owned10(0), owned14(0)
{
}

void Gfx_Skinning::Free()
{
	value00 = 0;
	value08 = 0;
	if (owned0C) { ::operator delete[](owned0C); owned0C = 0; }
	if (owned10) { ::operator delete[](owned10); owned10 = 0; }
	if (owned14) { ::operator delete[](owned14); owned14 = 0; }
}

void Gfx_Skinning::Allocate(DWORD faceCount)
{
	if (!faceCount) return;
	if (owned0C || owned10) Free();
	value00 = faceCount;
	value04 = 18 * faceCount;
	owned0C = ::operator new[](36 * faceCount);
	owned10 = ::operator new[](faceCount);
	owned14 = ::operator new[](12 * value04);
	FinishAllocation();
}

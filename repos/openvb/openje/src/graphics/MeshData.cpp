#include "MeshData.hpp"
#include <new>

void Gfx_MeshData::Free()
{
	if (primitives) { delete[] primitives; primitives = 0; }
	primitiveCount = 0;
	Gfx_MaterialSet* set = &materialSet;
	set->count = 0;
	set->ResetGroups();
	if (set->materials) { delete[] set->materials; set->materials = 0; }
	value40 = 0;
	if (owned44) { ::operator delete[](owned44); owned44 = 0; }
	if (owned48) { ::operator delete[](owned48); owned48 = 0; }
	value4C = 0;
	if (owned50) { ::operator delete[](owned50); owned50 = 0; }
}

Gfx_Primitive* Gfx_MeshData::SetPrimitiveCount(DWORD count)
{
	if (primitives) { delete[] primitives; primitives = 0; }
	primitiveCount = count;
	primitives = new Gfx_Primitive[count];
	materialSet.SetCount(count);
	return primitives;
}

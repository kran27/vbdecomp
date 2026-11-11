#include "MeshData.hpp"

Gfx_MaterialSet& Gfx_MaterialSet::operator=(const Gfx_MaterialSet& source)
{
	if (this != &source)
	{
		SetCount(source.count);
		for (DWORD i = 0; i < count; ++i)
		{
			// Real string/member assignment supplies the original material policy.
			materials[i] = source.materials[i];
		}
	}
	return *this;
}

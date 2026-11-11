#include "VfxLightningMesh.hpp"
#include <stdlib.h>

namespace JE
{
	VFX_LightningMesh::VFX_LightningMesh()
		: instances(NULL), packet(NULL)
	{
		D3DXMatrixIdentity(&transform);
		instanceCount = 0;
		vertices = NULL;
		vertexCount = 0;
		indices = NULL;
		indexCount = 0;
		flagC8 = 0;
	}
}

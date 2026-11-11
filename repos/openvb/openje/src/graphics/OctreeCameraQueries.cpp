#include "OctreeCamera.hpp"
#include <string.h>

D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

int OCTREE_Camera::ClassifyBox(const float* bounds)
{
	D3DXVECTOR3 positive, negative;
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	bool inside = true;
	for (DWORD index = 0; index < 6; ++index)
	{
		switch (planeCorners[index])
		{
		case 0:
			positive.x = bounds[3];
			positive.y = bounds[4];
			positive.z = bounds[5];
			negative.x = bounds[0];
			negative.y = bounds[1];
			negative.z = bounds[2];
			break;
		case 1:
			positive.x = bounds[3];
			positive.y = bounds[4];
			positive.z = bounds[2];
			negative.x = bounds[0];
			negative.y = bounds[1];
			negative.z = bounds[5];
			break;
		case 2:
			positive.x = bounds[3];
			positive.y = bounds[1];
			positive.z = bounds[5];
			negative.x = bounds[0];
			negative.y = bounds[4];
			negative.z = bounds[2];
			break;
		case 3:
			positive.x = bounds[3];
			positive.y = bounds[1];
			positive.z = bounds[2];
			negative.x = bounds[0];
			negative.y = bounds[4];
			negative.z = bounds[5];
			break;
		case 4:
			positive.x = bounds[0];
			positive.y = bounds[4];
			positive.z = bounds[5];
			negative.x = bounds[3];
			negative.y = bounds[1];
			negative.z = bounds[2];
			break;
		case 5:
			positive.x = bounds[0];
			positive.y = bounds[4];
			positive.z = bounds[2];
			negative.x = bounds[3];
			negative.y = bounds[1];
			negative.z = bounds[5];
			break;
		case 6:
			positive.x = bounds[0];
			positive.y = bounds[1];
			positive.z = bounds[5];
			negative.x = bounds[3];
			negative.y = bounds[4];
			negative.z = bounds[2];
			break;
		case 7:
			positive.x = bounds[0];
			positive.y = bounds[1];
			positive.z = bounds[2];
			negative.x = bounds[3];
			negative.y = bounds[4];
			negative.z = bounds[5];
			break;
		}
		if (index == 0)
		{
			D3DXVECTOR3 origin;
			origin = GfxAddVectors(&position.x, &transformedRays[4].x);
			const D3DXVECTOR3 value = GfxSubtractVectors(&positive.x, &origin.x);
			memcpy(difference.value, &value.x, sizeof(difference.value));
		}
		else if (index == 5)
		{
			D3DXVECTOR3 origin;
			origin = GfxAddVectors(&position.x, &transformedRays[5].x);
			const D3DXVECTOR3 value = GfxSubtractVectors(&positive.x, &origin.x);
			memcpy(difference.value, &value.x, sizeof(difference.value));
		}
		else
		{
			const D3DXVECTOR3 value = GfxSubtractVectors(&positive.x, &position.x);
			memcpy(difference.value, &value.x, sizeof(difference.value));
		}
		const float* plane = &planes[index].x;
		if (difference.Dot(plane) < 0.0f) return 0;
		if (inside)
		{
			if (index == 0)
			{
				D3DXVECTOR3 origin;
				origin = GfxAddVectors(&position.x, &transformedRays[4].x);
				const D3DXVECTOR3 value = GfxSubtractVectors(&negative.x, &origin.x);
				memcpy(difference.value, &value.x, sizeof(difference.value));
			}
			else if (index == 5)
			{
				D3DXVECTOR3 origin;
				origin = GfxAddVectors(&position.x, &transformedRays[5].x);
				const D3DXVECTOR3 value = GfxSubtractVectors(&negative.x, &origin.x);
				memcpy(difference.value, &value.x, sizeof(difference.value));
			}
			else
			{
				const D3DXVECTOR3 value = GfxSubtractVectors(&negative.x, &position.x);
				memcpy(difference.value, &value.x, sizeof(difference.value));
			}
			if (difference.Dot(plane) < 0.0f) inside = false;
		}
	}
	return inside ? 2 : 1;
}

int OCTREE_Camera::ClassifySpatialBox(const D3DXVECTOR3* center, const D3DXVECTOR3* extent)
{
	D3DXVECTOR3 positive, negative;
	Gfx_Vector3f difference = {{0.0f, 0.0f, 0.0f}};
	bool inside = true;
	D3DXVECTOR3 high, low;
	high = GfxAddVectors(&center->x, &extent->x);
	low = GfxSubtractVectors(&center->x, &extent->x);
	for (DWORD index = 0; index < 6; ++index)
	{
		switch (planeCorners[index])
		{
		case 0:
			positive.x = high.x;
			positive.y = high.y;
			positive.z = high.z;
			negative.x = low.x;
			negative.y = low.y;
			negative.z = low.z;
			break;
		case 1:
			positive.x = high.x;
			positive.y = high.y;
			positive.z = low.z;
			negative.x = low.x;
			negative.y = low.y;
			negative.z = high.z;
			break;
		case 2:
			positive.x = high.x;
			positive.y = low.y;
			positive.z = high.z;
			negative.x = low.x;
			negative.y = high.y;
			negative.z = low.z;
			break;
		case 3:
			positive.x = high.x;
			positive.y = low.y;
			positive.z = low.z;
			negative.x = low.x;
			negative.y = high.y;
			negative.z = high.z;
			break;
		case 4:
			positive.x = low.x;
			positive.y = high.y;
			positive.z = high.z;
			negative.x = high.x;
			negative.y = low.y;
			negative.z = low.z;
			break;
		case 5:
			positive.x = low.x;
			positive.y = high.y;
			positive.z = low.z;
			negative.x = high.x;
			negative.y = low.y;
			negative.z = high.z;
			break;
		case 6:
			positive.x = low.x;
			positive.y = low.y;
			positive.z = high.z;
			negative.x = high.x;
			negative.y = high.y;
			negative.z = low.z;
			break;
		case 7:
			positive.x = low.x;
			positive.y = low.y;
			positive.z = low.z;
			negative.x = high.x;
			negative.y = high.y;
			negative.z = high.z;
			break;
		}
		if (index == 0)
		{
			D3DXVECTOR3 origin;
			origin = GfxAddVectors(&position.x, &transformedRays[4].x);
			const D3DXVECTOR3 value = GfxSubtractVectors(&positive.x, &origin.x);
			memcpy(difference.value, &value.x, sizeof(difference.value));
		}
		else if (index == 5)
		{
			D3DXVECTOR3 origin;
			origin = GfxAddVectors(&position.x, &transformedRays[5].x);
			const D3DXVECTOR3 value = GfxSubtractVectors(&positive.x, &origin.x);
			memcpy(difference.value, &value.x, sizeof(difference.value));
		}
		else
		{
			const D3DXVECTOR3 value = GfxSubtractVectors(&positive.x, &position.x);
			memcpy(difference.value, &value.x, sizeof(difference.value));
		}
		const float* plane = &planes[index].x;
		if (difference.Dot(plane) < 0.0f) return 0;
		if (inside)
		{
			if (index == 0)
			{
				D3DXVECTOR3 origin;
				origin = GfxAddVectors(&position.x, &transformedRays[4].x);
				const D3DXVECTOR3 value = GfxSubtractVectors(&negative.x, &origin.x);
				memcpy(difference.value, &value.x, sizeof(difference.value));
			}
			else if (index == 5)
			{
				D3DXVECTOR3 origin;
				origin = GfxAddVectors(&position.x, &transformedRays[5].x);
				const D3DXVECTOR3 value = GfxSubtractVectors(&negative.x, &origin.x);
				memcpy(difference.value, &value.x, sizeof(difference.value));
			}
			else
			{
				const D3DXVECTOR3 value = GfxSubtractVectors(&negative.x, &position.x);
				memcpy(difference.value, &value.x, sizeof(difference.value));
			}
			if (difference.Dot(plane) < 0.0f) inside = false;
		}
	}
	return inside ? 2 : 1;
}

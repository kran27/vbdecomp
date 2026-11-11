#include "Octree.hpp"
#include <d3dx8.h>
#include <cstddef>
#include <float.h>
#include <stdexcept>

namespace
{
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char RaySize[(sizeof(OctreeRay) == 32) ? 1 : -1];
	typedef char RayAlign[(__alignof(OctreeRay) == 4) ? 1 : -1];
	typedef char RayDirection[(offsetof(OctreeRay, direction) == 12) ? 1 : -1];
	typedef char RayCallback[(offsetof(OctreeRay, testBounds) == 28) ? 1 : -1];
	typedef char RayMemberPointer[(sizeof(OctreeRay::BoundsTest) == 4) ? 1 : -1];
#endif


}

//Three-float value-return addition521EB0; genuine provider remains out of TU.
D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

//Eight full callbacks specialize the cached signs, retaining their own plane
//policy and actual scale/add calls. Inside-box success leaves point unwritten.
int OctreeRay::TestBoundsPPP(const float* minimum, const float* maximum,
	float* distance, float* point) const //4AC810; support0.
{
	if (!(maximum[0] >= origin[0])) return 0;
	if (!(maximum[1] >= origin[1])) return 0;
	if (!(maximum[2] >= origin[2])) return 0;
	if ((origin[0] >= minimum[0])
		&& (origin[1] >= minimum[1])
		&& (origin[2] >= minimum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = minimum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = minimum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = minimum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsPPN(const float* minimum, const float* maximum,
	float* distance, float* point) const //4ACA90; support0.
{
	if (!(maximum[0] >= origin[0])) return 0;
	if (!(maximum[1] >= origin[1])) return 0;
	if (!(minimum[2] <= origin[2])) return 0;
	if ((origin[0] >= minimum[0])
		&& (origin[1] >= minimum[1])
		&& (origin[2] <= maximum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = minimum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = minimum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = maximum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsPNP(const float* minimum, const float* maximum,
	float* distance, float* point) const //4ACD10; support0.
{
	if (!(maximum[0] >= origin[0])) return 0;
	if (!(minimum[1] <= origin[1])) return 0;
	if (!(maximum[2] >= origin[2])) return 0;
	if ((origin[0] >= minimum[0])
		&& (origin[1] <= maximum[1])
		&& (origin[2] >= minimum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = maximum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = minimum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = minimum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsPNN(const float* minimum, const float* maximum,
	float* distance, float* point) const //4ACF90; support0.
{
	if (!(maximum[0] >= origin[0])) return 0;
	if (!(minimum[1] <= origin[1])) return 0;
	if (!(minimum[2] <= origin[2])) return 0;
	if ((origin[0] >= minimum[0])
		&& (origin[1] <= maximum[1])
		&& (origin[2] <= maximum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = maximum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = minimum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = maximum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsNPP(const float* minimum, const float* maximum,
	float* distance, float* point) const //4AD210; support0.
{
	if (!(minimum[0] <= origin[0])) return 0;
	if (!(maximum[1] >= origin[1])) return 0;
	if (!(maximum[2] >= origin[2])) return 0;
	if ((origin[0] <= maximum[0])
		&& (origin[1] >= minimum[1])
		&& (origin[2] >= minimum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = minimum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = maximum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = minimum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsNPN(const float* minimum, const float* maximum,
	float* distance, float* point) const //4AD490; support0.
{
	if (!(minimum[0] <= origin[0])) return 0;
	if (!(maximum[1] >= origin[1])) return 0;
	if (!(minimum[2] <= origin[2])) return 0;
	if ((origin[0] <= maximum[0])
		&& (origin[1] >= minimum[1])
		&& (origin[2] <= maximum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = minimum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = maximum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = maximum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsNNP(const float* minimum, const float* maximum,
	float* distance, float* point) const //4AD720; support0.
{
	if (!(minimum[0] <= origin[0])) return 0;
	if (!(minimum[1] <= origin[1])) return 0;
	if (!(maximum[2] >= origin[2])) return 0;
	if ((origin[0] <= maximum[0])
		&& (origin[1] <= maximum[1])
		&& (origin[2] >= minimum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = maximum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = maximum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = minimum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

int OctreeRay::TestBoundsNNN(const float* minimum, const float* maximum,
	float* distance, float* point) const //4AD9A0; support0.
{
	if (!(minimum[0] <= origin[0])) return 0;
	if (!(minimum[1] <= origin[1])) return 0;
	if (!(minimum[2] <= origin[2])) return 0;
	if ((origin[0] <= maximum[0])
		&& (origin[1] <= maximum[1])
		&& (origin[2] <= maximum[2]))
	{
		*distance = 0.0f;
		return 1;
	}
	if (direction.value[1] >= 0.000099999997f || -0.000099999997f >= direction.value[1])
	{
		const float boundary = maximum[1];
		const float fraction = float((double(boundary) - origin[1]) / direction.value[1]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[0] >= 0.000099999997f || -0.000099999997f >= direction.value[0])
	{
		const float boundary = maximum[0];
		const float fraction = float((double(boundary) - origin[0]) / direction.value[0]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[1] >= minimum[1] && point[1] <= maximum[1]
			&& point[2] >= minimum[2] && point[2] <= maximum[2]) return 1;
	}
	if (direction.value[2] >= 0.000099999997f || -0.000099999997f >= direction.value[2])
	{
		const float boundary = maximum[2];
		const float fraction = float((double(boundary) - origin[2]) / direction.value[2]);
		*distance = fraction;
		//The distance output may alias ray storage: use the live subobject.
		const Gfx_Vector3f scaled = direction.Scale(fraction);
		const D3DXVECTOR3 candidate = GfxAddVectors(origin, scaled.value);
		memcpy(point, &candidate.x, sizeof(float));
		memcpy(point + 1, &candidate.y, sizeof(float));
		memcpy(point + 2, &candidate.z, sizeof(float));
		if (point[0] >= minimum[0] && point[0] <= maximum[0]
			&& point[1] >= minimum[1] && point[1] <= maximum[1]) return 1;
	}
	return 0;
}

//4ADC20 dispatches current signs to the real per-sign bodies.
int OctreeRay::TestBounds(const float* minimum, const float* maximum,
	float* distance, float* point) const
{
	if (direction.value[0] > 0.0f)
	{
		if (direction.value[1] > 0.0f)
		{
			if (direction.value[2] > 0.0f) return TestBoundsPPP(minimum, maximum, distance, point);
			return TestBoundsPPN(minimum, maximum, distance, point);
		}
		else
		{
			if (direction.value[2] > 0.0f) return TestBoundsPNP(minimum, maximum, distance, point);
			return TestBoundsPNN(minimum, maximum, distance, point);
		}
	}
	else
	{
		if (direction.value[1] > 0.0f)
		{
			if (direction.value[2] > 0.0f) return TestBoundsNPP(minimum, maximum, distance, point);
			return TestBoundsNPN(minimum, maximum, distance, point);
		}
		else
		{
			if (direction.value[2] > 0.0f) return TestBoundsNNP(minimum, maximum, distance, point);
			return TestBoundsNNN(minimum, maximum, distance, point);
		}
	}
}

void OctreeRay::SetDirection(const float* inputDirection) //4ADC90.
{
	if (inputDirection)
	{
		memcpy(direction.value, inputDirection, sizeof(float));
		memcpy(direction.value + 1, inputDirection + 1, sizeof(float));
		memcpy(direction.value + 2, inputDirection + 2, sizeof(float));
	}
	if (direction.value[0] > 0.0f)
	{
		if (direction.value[1] > 0.0f)
		{
			testBounds = direction.value[2] > 0.0f ? &OctreeRay::TestBoundsPPP
				: &OctreeRay::TestBoundsPPN;
		}
		else
		{
			testBounds = direction.value[2] > 0.0f ? &OctreeRay::TestBoundsPNP
				: &OctreeRay::TestBoundsPNN;
		}
	}
	else
	{
		if (direction.value[1] > 0.0f)
		{
			testBounds = direction.value[2] > 0.0f ? &OctreeRay::TestBoundsNPP
				: &OctreeRay::TestBoundsNPN;
		}
		else
		{
			testBounds = direction.value[2] > 0.0f ? &OctreeRay::TestBoundsNNP
				: &OctreeRay::TestBoundsNNN;
		}
	}
}



int OCTREE::ResetQueryCursor()
{
	spatialCursor = 0;
	return 0;
}

int OCTREE::IsObjectVisible(DWORD handle)
{
	OctreeSpatialObject* object = handle < handles.capacity
		? static_cast<OctreeSpatialObject*>(handles.objects[handle]) : NULL;
	if (!object) return 2;
	return (static_cast<BYTE>(object->flags) >> 1) & 1;
}

int OCTREE::QueryObjects(const OctreeRay& ray, DWORD capacity, OctreeObjectHit* output)
{
	DWORD count = 0, limit = capacity - 1;
	int level = 0;
	OctreeNode* node = &root;
	const float maximumDistance = ray.maximumDistance;
	while (node && count < limit)
	{
		DWORD next = traversal[level].nextChild;
		if (!next)
		{
			OctreeObjectHit& candidate = output[count];
			if ((ray.*ray.testBounds)(node->bounds, node->bounds+3, &candidate.distance, candidate.point)
				&& candidate.distance < maximumDistance)
			{
				for (OctreeSpatialObject* object = node->objects; object && count < limit; object = object->next)
				{
					D3DXVECTOR3 maximum, minimum;
					maximum = GfxAddVectors(object->center, object->extent);
					minimum = GfxSubtractVectors(object->center, object->extent);
					OctreeObjectHit& hit = output[count];
					if ((ray.*ray.testBounds)(&minimum.x, &maximum.x, &hit.distance, hit.point)
						&& hit.distance < maximumDistance)
					{
						hit.hit = 1;
						hit.object = object->object;
						++count;
					}
				}
			}
			else next = 8;
			if (!(node->flags & 1))
			{
				const DWORD* order = node->GetChildOrder(ray.origin, &traversal[level].childCount);
				traversal[level].childOrder = order;
			}
		}
		if (!(node->flags & 1))
		{
			OctreeTraversal* frame = traversal + level;
			const DWORD childCount = frame->childCount;
			while (next < childCount)
			{
				OctreeNode* child = node->children[frame->childOrder[next++]];
				if (child)
				{
					frame->nextChild = next;
					node = child;
					++level;
					goto nextNode;
				}
			}
		}
		traversal[level].nextChild = 0;
		node = node->parent;
		--level;
	nextNode:;
	}
	output[count].hit = 0;
	while (level >= 0) traversal[level--].nextChild = 0;
	return count != 0;
}

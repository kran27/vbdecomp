#include "OctreeShape.hpp"
#include <d3dx8math.h>

float GfxDistanceSquaredXZ(const float* first, const float* second);
D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

namespace
{
	int ClassifyRing(const OCTREE_Ring2D& ring, const float* lower, const float* upper, bool requireBothSides)
	{
		double nearest = 0.0;
		float farthest = 0.0f;
		bool spansInner = false;
		for (int axis = 0; axis < 3; axis += 2)
		{
			if (ring.position[axis] < lower[axis])
			{
				const double nearDistance = double(lower[axis]) - ring.position[axis];
				const double farDistance = double(ring.position[axis]) - upper[axis];
				nearest += nearDistance * nearDistance;
				farthest = float(farDistance * farDistance + farthest);
			}
			else if (ring.position[axis] > upper[axis])
			{
				const double nearDistance = double(ring.position[axis]) - upper[axis];
				const double farDistance = double(lower[axis]) - ring.position[axis];
				nearest += nearDistance * nearDistance;
				farthest = float(farDistance * farDistance + farthest);
			}
			else
			{
				const bool lowerSpans = double(ring.position[axis]) - lower[axis] >= ring.innerRadius;
				const bool upperSpans = double(upper[axis]) - ring.position[axis] >= ring.innerRadius;
				//5BA150 uses OR;5BA2B0 uses AND for these opposite-side tests.
				if (requireBothSides ? (lowerSpans && upperSpans) : (lowerSpans || upperSpans)) spansInner = true;
			}
		}
		if (!spansInner && (nearest > ring.outerRadiusSquared || farthest < ring.innerRadiusSquared)) return 0;
		//Ordered >=/<= retain the original final unordered-result classification1.
		return nearest >= ring.innerRadiusSquared && farthest <= ring.outerRadiusSquared ? 2 : 1;
	}
}

OCTREE_Ring2D::OCTREE_Ring2D()
{
	//Existing OCTREE_Shape owns the actual map and shared release/query state.
}

void OCTREE_Ring2D::Configure(float outer, float inner)
{
	outerRadius = outer;
	innerRadius = inner;
	outerRadiusSquared = outer * outer;
	innerRadiusSquared = inner * inner;
}

int OCTREE_Ring2D::ContainsPoint(const float* point) const
{
	const float squaredDistance = GfxDistanceSquaredXZ(position, point);
	return squaredDistance <= outerRadiusSquared && squaredDistance >= innerRadiusSquared;
}

int OCTREE_Ring2D::ClassifyBox(const float* bounds) const
{
	float nearest = 0.0f;
	int spansInner = 0;
	float farthest = 0.0f;
	if (position[0] < bounds[0])
	{
		const float nearDistance = bounds[0] - position[0];
		nearest = nearDistance * nearDistance;
		const float farDistance = position[0] - bounds[3];
		farthest = float(farDistance * farDistance);
	}
	else if (position[0] > bounds[3])
	{
		const float nearDistance = position[0] - bounds[3];
		nearest = nearDistance * nearDistance;
		const float farDistance = bounds[0] - position[0];
		farthest = float(farDistance * farDistance);
	}
	else if (position[0] - bounds[0] >= innerRadius
		|| bounds[3] - position[0] >= innerRadius) spansInner = 1;
	if (position[2] < bounds[2])
	{
		const float nearDistance = bounds[2] - position[2];
		nearest += nearDistance * nearDistance;
		const float farDistance = position[2] - bounds[5];
		farthest = float(farDistance * farDistance + farthest);
	}
	else if (position[2] > bounds[5])
	{
		const float nearDistance = position[2] - bounds[5];
		nearest += nearDistance * nearDistance;
		const float farDistance = bounds[2] - position[2];
		farthest = float(farDistance * farDistance + farthest);
	}
	else if (position[2] - bounds[2] >= innerRadius
		|| bounds[5] - position[2] >= innerRadius) spansInner = 1;
	if (!spansInner && (nearest > outerRadiusSquared || farthest < innerRadiusSquared)) return 0;
	return nearest >= innerRadiusSquared && farthest <= outerRadiusSquared ? 2 : 1;
}

int OCTREE_Ring2D::ClassifyObject(const float* center, const float* extent) const
{
	int spansInner = 0;
	float nearest = 0.0f;
	float farthest = 0.0f;
	D3DXVECTOR3 lower, upper;
	upper = GfxAddVectors(center, extent);
	lower = GfxSubtractVectors(center, extent);
	if (position[0] < lower.x)
	{
		const float nearDistance = lower.x - position[0];
		nearest = nearDistance * nearDistance;
		const float farDistance = position[0] - upper.x;
		farthest = float(farDistance * farDistance);
	}
	else if (position[0] > upper.x)
	{
		const float nearDistance = position[0] - upper.x;
		nearest = nearDistance * nearDistance;
		const float farDistance = lower.x - position[0];
		farthest = float(farDistance * farDistance);
	}
	else if (position[0] - lower.x >= innerRadius
		&& upper.x - position[0] >= innerRadius) spansInner = 1;
	if (position[2] < lower.z)
	{
		const float nearDistance = lower.z - position[2];
		nearest += nearDistance * nearDistance;
		const float farDistance = position[2] - upper.z;
		farthest = float(farDistance * farDistance + farthest);
	}
	else if (position[2] > upper.z)
	{
		const float nearDistance = position[2] - upper.z;
		nearest += nearDistance * nearDistance;
		const float farDistance = lower.z - position[2];
		farthest = float(farDistance * farDistance + farthest);
	}
	else if (position[2] - lower.z >= innerRadius
		&& upper.z - position[2] >= innerRadius) spansInner = 1;
	if (!spansInner && (nearest > outerRadiusSquared || farthest < innerRadiusSquared)) return 0;
	return nearest >= innerRadiusSquared && farthest <= outerRadiusSquared ? 2 : 1;
}

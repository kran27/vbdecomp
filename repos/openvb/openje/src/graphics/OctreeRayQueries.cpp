#include "OctreeCamera.hpp"
#include "GfxVector3.hpp"
#include <stdexcept>

D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

namespace
{


	bool MatchesTriangle(DWORD flags, DWORD mask, DWORD mode)
	{
		switch (mode)
		{
		case 0: return !mask || (flags & mask) != 0;
		case 1: return (flags & mask) != 0;
		case 2: return (flags & mask) == mask;
		case 3: return (flags & mask) == 0;
		case 4: return (flags & mask) != mask;
		default: return true;
		}
	}

	//Proprietary4AC2B0 two-sided barycentric policy, using the real SDK vector
	//operations. Original x87 versus SDK rounding is not claimed equivalent.
	bool IntersectTriangle(const OctreeRay& ray, const float* first, const float* second,
		const float* third, float& distance, float* point)
	{
		D3DXVECTOR3 a(first), b(second), c(third), direction(ray.direction.value), origin(ray.origin);
		D3DXVECTOR3 edge1 = b - a, edge2 = c - a, p, q;
		D3DXVec3Cross(&p, &direction, &edge2);
		float determinant = D3DXVec3Dot(&edge1, &p);
		if (!(determinant > 0.000099999997f) && !(-0.000099999997f > determinant)) return false;
		D3DXVECTOR3 offset = origin - a;
		float u = D3DXVec3Dot(&offset, &p);
		D3DXVec3Cross(&q, &offset, &edge1);
		float v = D3DXVec3Dot(&direction, &q);
		if (determinant > 0.000099999997f)
		{
			if (u < 0.0f || u > determinant || v < 0.0f || u + v > determinant) return false;
		}
		else if (u > 0.0f || u < determinant || v > 0.0f || u + v < determinant) return false;
		double value = static_cast<double>(D3DXVec3Dot(&edge2, &q)) * (1.0 / determinant);
		distance = static_cast<float>(value);
		if (!(value >= 0.0)) return false;
		for (int axis = 0; axis < 3; ++axis)
		{
			float scaled = ray.direction.value[axis] * distance;
			point[axis] = ray.origin[axis] + scaled;
		}
		return true;
	}

	bool IntersectNode(const OctreeNode& node, const OctreeRay& ray, const float* positions,
		float& distance, float* point, DWORD mask, DWORD mode, DWORD* flags) //5BD260.
	{
		if (!node.indices) return false;
		distance = ray.maximumDistance;
		DWORD cursor = 0, triangle = 0;
		while (cursor < node.wordCount)
		{
			WORD marker = node.indices[cursor];
			if (marker & 0xC000)
			{
				cursor += (marker & 0x8000) ? 4 : 2;
				continue;
			}
			DWORD count = node.indices[cursor+1];
			cursor += 2;
			DWORD end = cursor + count;
			while (cursor < end)
			{
				const float* a = positions + 3*node.indices[cursor];
				const float* b = positions + 3*node.indices[cursor+1];
				const float* c = positions + 3*node.indices[cursor+2];
				DWORD triangleFlags = node.triangleFlags[triangle++];
				cursor += 3;
				if (!MatchesTriangle(triangleFlags, mask, mode)) continue;
				float candidateDistance, candidatePoint[3];
				if (IntersectTriangle(ray, a, b, c, candidateDistance, candidatePoint)
					&& candidateDistance < distance)
				{
					distance = candidateDistance;
					for (int axis = 0; axis < 3; ++axis) point[axis] = candidatePoint[axis];
					if (flags) *flags = node.triangleFlags[triangle-1];
				}
			}
			cursor += cursor & 1;
		}
		return distance != ray.maximumDistance;
	}
}

int OCTREE_Camera::MakeScreenRay(LONG x, LONG y, OctreeRay* output)
{
	Gfx_Vector3f local;
	local.value[0] = static_cast<float>((static_cast<double>(x) / halfWidth - 1.0)
		* projectionScaleX + projectionOffsetX);
	local.value[1] = static_cast<float>((1.0 - static_cast<double>(y) / halfHeight)
		* projectionScaleY + projectionOffsetY);
	local.value[2] = 1.0f;
	local = local.Transform(transform);
	*output = OctreeRay(&position.x, local.value);
	output->SetDirection(NULL); //5BCDB6 reselects the actual cached sign callback.
	return 0;
}

int OCTREE_Camera::QueryObjects(const OctreeRay& ray, DWORD capacity,
	OctreeObjectHit* output, DWORD* more)
{
	DWORD count = 0;
	if (!*more)
	{
		objectCursor = 0;
	}
	DWORD limit = capacity - 1;
	*more = 0;
	if (limit)
	{
		do
		{
			OctreeSpatialObject* object = objects[objectCursor];
			if (!object) break;
			D3DXVECTOR3 maximum, minimum;
			maximum = GfxAddVectors(object->center, object->extent);
			object = objects[objectCursor];
			minimum = GfxSubtractVectors(object->center, object->extent);
			OctreeObjectHit& hit = output[count];
			if ((ray.*ray.testBounds)(&minimum.x, &maximum.x, &hit.distance, hit.point)
				&& ray.maximumDistance > hit.distance)
			{
				hit.hit = 1;
				//The original reloads the current reference after the ray callback.
				hit.object = objects[objectCursor]->object;
				++count;
			}
			++objectCursor;
		}
		while (count < limit);
	}
	output[count].hit = 0;
	if (objects[objectCursor])
	{
		*more = 1;
	}
	else
	{
		objectCursor = 0;
	}
	return 0;
}

int OCTREE::QueryGroundRay(const OctreeRay& ray, OctreeGroundHit* output, DWORD mask, DWORD mode)
{
	float best = ray.maximumDistance;
	DWORD triangleFlags = 0;
	OctreeNode* node = &root;
	int level = 0;
	do
	{
		OctreeTraversal& frame = traversal[level];
		DWORD next = frame.nextChild;
		if (!next)
		{
			float boundsDistance, boundsPoint[3];
			if ((ray.*ray.testBounds)(node->bounds, node->bounds+3, &boundsDistance, boundsPoint))
			{
				if (boundsDistance < best && (node->flags & 1))
				{
					//Original EDI starts as the ray pointer and becomes the node on
					//descent; a terminal root reads its buffer selector at ray+24h.
					if (node == &root)
						throw std::logic_error("OCTREE5AEB93: root-leaf ray+24h buffer selector unresolved");
					float distance, point[3];
					if (IntersectNode(*node, ray, vertexBuffers[node->vertexBuffer].positions,
						distance, point, mask, mode, &triangleFlags) && distance < best)
					{
						best = distance;
						output->distance = distance;
						for (int axis = 0; axis < 3; ++axis) output->point[axis] = point[axis];
						output->triangleFlags = triangleFlags;
					}
				}
			}
			else next = 8;
			if (!(node->flags & 1)) frame.childOrder = node->GetChildOrder(ray.origin, &frame.childCount);
		}
		if (!(node->flags & 1))
		{
			while (next < frame.childCount)
			{
				OctreeNode* child = node->children[frame.childOrder[next++]];
				if (child)
				{
					frame.nextChild = next;
					node = child;
					++level;
					goto NextNode;
				}
			}
		}
		frame.nextChild = 0;
		node = node->parent;
		--level;
	NextNode:;
	} while (node);
	return best != ray.maximumDistance;
}

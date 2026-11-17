#include "OctreeCamera.hpp"
#include "GfxVector3.hpp"
#include <stdexcept>

D3DXVECTOR3 GfxAddVectors(const float* first, const float* second);
D3DXVECTOR3 GfxSubtractVectors(const float* first, const float* second);

int OctreeRay::IntersectTriangle(const float* first, const float* second, const float* third, float* distance,
                                 float* point) const // 4AC2B0.
{
    D3DXVECTOR3 edgeValue = GfxSubtractVectors(second, first);
    Gfx_Vector3f edge1 = { { edgeValue.x, edgeValue.y, edgeValue.z } };
    edgeValue = GfxSubtractVectors(third, first);
    Gfx_Vector3f edge2 = { { edgeValue.x, edgeValue.y, edgeValue.z } };
    Gfx_Vector3f p = direction.Cross(edge2);
    float determinant = edge1.Dot(p.value);
    Gfx_Vector3f q;
    if (determinant > 0.000099999997f)
    {
        D3DXVECTOR3 offsetValue = GfxSubtractVectors(origin, first);
        Gfx_Vector3f offset = { { offsetValue.x, offsetValue.y, offsetValue.z } };
        float u = offset.Dot(p.value);
        if (u < 0.0 || u > determinant)
        {
            return 0;
        }
        q = offset.Cross(edge1);
        float v = direction.Dot(q.value);
        if (v < 0.0 || u + v > determinant)
        {
            return 0;
        }
    }
    else
    {
        if (!(-0.000099999997f > determinant))
        {
            return 0;
        }
        D3DXVECTOR3 offsetValue = GfxSubtractVectors(origin, first);
        Gfx_Vector3f offset = { { offsetValue.x, offsetValue.y, offsetValue.z } };
        float u = offset.Dot(p.value);
        if (u > 0.0 || u < determinant)
        {
            return 0;
        }
        q = offset.Cross(edge1);
        float v = direction.Dot(q.value);
        if (v > 0.0 || u + v < determinant)
        {
            return 0;
        }
    }
    float value = edge2.Dot(q.value) * (g_GfxScalarOne / determinant);
    *distance = value;
    if (value < g_GfxScalarZero)
    {
        return 0;
    }
    Gfx_Vector3f scaled = direction.Scale(value);
    D3DXVECTOR3 hit = GfxAddVectors(origin, scaled.value);
    memcpy(point, &hit, sizeof(hit));
    return 1;
}

int OCTREE_Camera::MakeScreenRay(LONG x, LONG y, OctreeRay* output)
{
    Gfx_Vector3f local;
    local.value[0] =
        static_cast<float>((static_cast<double>(x) / halfWidth - 1.0) * projectionScaleX + projectionOffsetX);
    local.value[1] =
        static_cast<float>((1.0 - static_cast<double>(y) / halfHeight) * projectionScaleY + projectionOffsetY);
    local.value[2] = 1.0f;
    local = local.Transform(transform);
    *output = OctreeRay(&position.x, local.value);
    output->SetDirection(NULL); // 5BCDB6 reselects the actual cached sign callback.
    return 0;
}

int OCTREE_Camera::QueryObjects(const OctreeRay& ray, DWORD capacity, OctreeObjectHit* output, DWORD* more)
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
            if (!object)
            {
                break;
            }
            D3DXVECTOR3 maximum, minimum;
            maximum = GfxAddVectors(object->center, object->extent);
            object = objects[objectCursor];
            minimum = GfxSubtractVectors(object->center, object->extent);
            OctreeObjectHit& hit = output[count];
            if ((ray.*ray.testBounds)(&minimum.x, &maximum.x, &hit.distance, hit.point) &&
                ray.maximumDistance > hit.distance)
            {
                hit.hit = 1;
                // The original reloads the current reference after the ray callback.
                hit.object = objects[objectCursor]->object;
                ++count;
            }
            ++objectCursor;
        } while (count < limit);
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
            if ((ray.*ray.testBounds)(node->bounds, node->bounds + 3, &boundsDistance, boundsPoint))
            {
                if (boundsDistance < best && (node->flags & 1))
                {
                    // Original EDI starts as the ray pointer and becomes the node on
                    // descent; a terminal root reads its buffer selector at ray+24h.
                    if (node == &root)
                    {
                        throw std::logic_error("OCTREE5AEB93: root-leaf ray+24h buffer selector unresolved");
                    }
                    float distance, point[3];
                    if (node->IntersectRay(ray, vertexBuffers[node->vertexBuffer].positions, &distance, point, mask,
                                           mode, &triangleFlags) &&
                        distance < best)
                    {
                        best = distance;
                        output->distance = distance;
                        for (int axis = 0; axis < 3; ++axis)
                        {
                            output->point[axis] = point[axis];
                        }
                        output->triangleFlags = triangleFlags;
                    }
                }
            }
            else
            {
                next = 8;
            }
            if (!(node->flags & 1))
            {
                frame.childOrder = node->GetChildOrder(ray.origin, &frame.childCount);
            }
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

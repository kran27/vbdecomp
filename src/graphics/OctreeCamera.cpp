#include "OctreeCamera.hpp"
#include "GfxVector3.hpp"
#include <math.h>
#include <new>
#include <stdexcept>
#include <string.h>

// Actual separate float operands; shared readonly storage prevents reassociating
// their ordered products into a rounded compile-time conversion constant.
extern const float g_GfxCameraDegreeFraction;
extern const float g_GfxCameraPi;
extern const float g_GfxCameraHalfPi;

namespace
{
    // The observed non-axis transform disagrees with D3DX's SIMD association.
    // This local camera boundary retains the original float output after wide
    // z/y/x products, without providing another general matrix implementation.
    __declspec(noinline) void TransformCameraVector(D3DXVECTOR3* output, const D3DXVECTOR3* input,
                                                    const D3DXMATRIX* matrix)
    {
        double x = input->x, y = input->y, z = input->z;
        float result[3];
        for (DWORD index = 0; index < 3; ++index)
        {
            result[index] = static_cast<float>(matrix->m[2][index] * z + matrix->m[1][index] * y +
                                               matrix->m[0][index] * x + matrix->m[3][index]);
        }
        output->x = result[0];
        output->y = result[1];
        output->z = result[2];
    }
} // namespace

OCTREE_Camera::OCTREE_Camera() : objectCursor(0), nodes(0), mask(1), buckets(0), objects(0)
{
    D3DXMatrixIdentity(&transform);
    memset(transformedRays, 0, sizeof(transformedRays));
    memset(planes, 0, sizeof(planes));
    memset(localRays, 0, sizeof(localRays));
    memset(localPlanes, 0, sizeof(localPlanes));
}

int OCTREE_Camera::Release()
{
    if (buckets)
    {
        for (DWORD index = 0; buckets[index]; ++index)
        {
            delete[] buckets[index];
        }
        delete[] buckets;
    }
    if (nodes)
    {
        delete[] nodes;
    }
    if (objects)
    {
        for (DWORD index = 0; objects[index]; ++index)
        {
            --objects[index]->references;
            OctreeSpatialObject* object = objects[index];
            if (!object->references)
            {
                ::operator delete(object);
            }
            else
            {
                object->flags &= ~2u;
            }
        }
        delete[] objects;
    }
    ::operator delete(this);
    return 0;
}

int OCTREE::CreateCamera(float degrees, float aspect, float nearPlane, float farPlane, OCTREE_Camera** output)
{
    OctreeNode** nodes;
    OctreeNodeReference** buckets;
    OctreeSpatialObject** objects = NULL;
    OCTREE_Camera* camera = new OCTREE_Camera;
    if (!camera)
    {
        goto noCamera;
    }
    nodes = new OctreeNode*[value10 + 1];
    if (!nodes)
    {
        goto releaseCamera;
    }
    nodes[0] = NULL;
    nodes[value10] = NULL;
    buckets = new OctreeNodeReference*[vertexBufferCount + 1];
    if (!buckets)
    {
        goto failed;
    }
    memset(buckets, 0, sizeof(OctreeNodeReference*) * (vertexBufferCount + 1));
    for (DWORD index = 0; index < vertexBufferCount; ++index)
    {
        buckets[index] = new OctreeNodeReference[indexedNodeCounts[index] + 1];
        if (!buckets[index])
        {
            goto failed;
        }
        buckets[index][indexedNodeCounts[index]].node = NULL;
        buckets[index][0].node = NULL;
    }
    objects = new OctreeSpatialObject*[1025];
    if (!objects)
    {
        goto failed;
    }
    objects[0] = NULL;
    objects[1024] = NULL;
    camera->SetFrustum(degrees, aspect, nearPlane, farPlane);
    camera->objects = objects;
    camera->nodes = nodes;
    camera->buckets = buckets;
    *output = camera;
    return 0;

failed:
    delete[] nodes;
    if (buckets)
    {
        for (DWORD item = 0; item < vertexBufferCount; ++item)
        {
            if (buckets[item])
            {
                delete[] buckets[item];
            }
        }
        delete[] buckets;
    }
    if (objects)
    {
        delete[] objects;
    }
releaseCamera:
    camera->Release();
noCamera:
    return 1;
}

void OCTREE_Camera::SetFrustum(float degrees, float aspect, float nearPlane, float farPlane)
{
    const float vertical =
        static_cast<float>(static_cast<double>(degrees) * g_GfxScalarHalf * g_GfxCameraDegreeFraction * g_GfxCameraPi);
    Gfx_Vector3f forward = { { 0.0f, 0.0f, 1.0f } };
    Gfx_Vector3f up = { { 0.0f, 1.0f, 0.0f } };
    const double horizontal = g_GfxCameraHalfPi - static_cast<double>(degrees) * aspect * g_GfxScalarHalf *
                                                      g_GfxCameraDegreeFraction * g_GfxCameraPi;
    const float sineHorizontal = static_cast<float>(sin(horizontal));
    const float cosineHorizontal = static_cast<float>(cos(horizontal));
    forward.RotateXZ(cosineHorizontal, sineHorizontal);
    const float sineVertical = static_cast<float>(-sin(vertical));
    const float cosineVertical = static_cast<float>(cos(vertical));
    up.RotateYZ(cosineVertical, sineVertical);

    Gfx_Vector3f corner = forward.Cross(up);
    memcpy(localRays, corner.value, sizeof(corner.value));
    corner.Normalize();
    memcpy(localRays, corner.value, sizeof(corner.value));
    const float negativeX = -localRays[0].x;
    localRays[1].x = negativeX;
    localRays[3].x = negativeX;
    const float negativeY = -localRays[0].y;
    localRays[1].y = localRays[0].y;
    localRays[3].y = negativeY;
    localRays[1].z = localRays[0].z;
    localRays[3].z = localRays[0].z;
    localRays[2].x = localRays[0].x;
    localRays[2].y = negativeY;
    localRays[2].z = localRays[0].z;
    localRays[4].x = 0;
    localRays[4].y = 0;
    localRays[4].z = nearPlane;
    localRays[5].x = 0;
    localRays[5].y = 0;
    localRays[5].z = farPlane;

    Gfx_Vector3f right, lowerLeft, lowerRight;
    memcpy(right.value, localRays + 1, sizeof(right.value));
    memcpy(lowerLeft.value, localRays + 2, sizeof(lowerLeft.value));
    memcpy(lowerRight.value, localRays + 3, sizeof(lowerRight.value));
    {
        Gfx_Vector3f normal = right.Cross(corner);
        memcpy(localPlanes, normal.value, sizeof(normal.value));
        normal.Normalize();
        memcpy(localPlanes, normal.value, sizeof(normal.value));
    }
    {
        Gfx_Vector3f normal = lowerLeft.Cross(lowerRight);
        memcpy(localPlanes + 1, normal.value, sizeof(normal.value));
        normal.Normalize();
        memcpy(localPlanes + 1, normal.value, sizeof(normal.value));
    }
    {
        Gfx_Vector3f normal = corner.Cross(lowerLeft);
        memcpy(localPlanes + 2, normal.value, sizeof(normal.value));
        normal.Normalize();
        memcpy(localPlanes + 2, normal.value, sizeof(normal.value));
    }
    {
        Gfx_Vector3f normal = lowerRight.Cross(right);
        memcpy(localPlanes + 3, normal.value, sizeof(normal.value));
        normal.Normalize();
        memcpy(localPlanes + 3, normal.value, sizeof(normal.value));
    }
    localPlanes[4].x = 0;
    localPlanes[4].y = 0;
    localPlanes[5].x = 0;
    localPlanes[5].y = 0;
    projectionOffsetX = 0;
    projectionOffsetY = 0;
    localPlanes[4].z = 1;
    localPlanes[5].z = -1;
    projectionScaleX = 1;
    projectionScaleY = 1;
}

void OCTREE_Camera::SetScreen(float width, float height, const D3DXMATRIX* projection)
{
    halfWidth = static_cast<float>(width * 0.5f);
    halfHeight = static_cast<float>(height * 0.5f);
    if (projection->_11 != 0)
    {
        projectionScaleX = static_cast<float>(1.0f / projection->_11);
        projectionOffsetX = static_cast<float>(-(static_cast<double>(projection->_41) / projection->_11));
    }
    if (projection->_22 != 0)
    {
        projectionScaleY = static_cast<float>(1.0f / projection->_22);
        projectionOffsetY = static_cast<float>(-(static_cast<double>(projection->_42) / projection->_22));
    }
}

void OCTREE_Camera::SetMode(DWORD mode)
{
    switch (mode)
    {
    case 0:
        mask = (mask & ~DWORD(0x40)) | 1;
        break;
    case 1:
        mask = (mask & ~DWORD(1)) | 0x40;
        break;
    default:
        mask = (mask & ~DWORD(0x40)) | 1;
        break;
    }
}

int OCTREE_Camera::SetTransform(const D3DXMATRIX* source)
{
    position = D3DXVECTOR3(source->_41, source->_42, source->_43);
    transform = *source;
    transform._41 = transform._42 = transform._43 = 0;
    for (DWORD index = 0; index < 6; ++index)
    {
        TransformCameraVector(transformedRays + index, localRays + index, &transform);
    }
    const DWORD order[6] = { 3, 4, 1, 2, 0, 5 };
    for (DWORD index = 0; index < 6; ++index)
    {
        TransformCameraVector(planes + order[index], localPlanes + index, &transform);
    }
    for (DWORD index = 0; index < 6; ++index)
    {
        const D3DXVECTOR3& normal = planes[index];
        planeCorners[index] = (normal.x > 0 ? 0 : 4) | (normal.y > 0 ? 0 : 2) | (normal.z > 0 ? 0 : 1);
    }
    return 0;
}

DWORD OCTREE_Camera::BuildTransform(DWORD xBits, DWORD yBits, DWORD zBits, float pitchDegrees, float yawDegrees,
                                    D3DXMATRIX* output)
{
    D3DXMATRIX yaw;
    yaw._12 = yaw._14 = yaw._21 = yaw._23 = yaw._24 = yaw._32 = 0;
    yaw._34 = yaw._41 = yaw._42 = yaw._43 = 0;
    float pitchCosine;
    if (pitchDegrees != 0)
    {
        double radians = pitchDegrees * static_cast<double>(g_GfxCameraDegreeFraction) * g_GfxCameraPi;
        pitchCosine = static_cast<float>(cos(radians));
        pitchDegrees = static_cast<float>(sin(radians));
    }
    else
    {
        pitchCosine = 1;
        pitchDegrees = 0;
    }
    double yawCosine, yawSine;
    if (yawDegrees != 0)
    {
        double radians = yawDegrees * static_cast<double>(g_GfxCameraDegreeFraction) * g_GfxCameraPi;
        // Preserve the observed float argument store/reload before sine while
        // cosine consumes the wide radian value. Historical source spelling unknown.
        volatile float yawArgument = static_cast<float>(radians);
        yawCosine = cos(radians);
        yawSine = sin(yawArgument);
    }
    else
    {
        yawCosine = 1;
        yawSine = 0;
    }
    output->_32 = -pitchDegrees;
    yaw._11 = static_cast<float>(yawCosine);
    output->_22 = pitchCosine;
    output->_23 = pitchDegrees;
    yaw._13 = static_cast<float>(-yawSine);
    yaw._31 = static_cast<float>(yawSine);
    output->_11 = 1;
    output->_12 = output->_13 = output->_21 = output->_31 = 0;
    yaw._33 = static_cast<float>(yawCosine);
    output->_33 = pitchCosine;
    output->_41 = output->_42 = output->_43 = 0;
    output->_44 = 1;
    yaw._22 = yaw._44 = 1;
    // Reuse the real SDK for the identified matrix API. The original helper's
    // thiscall boundary and x87 evaluation order remain separately unresolved.
    D3DXMatrixMultiply(output, output, &yaw);
    output->_44 = 0;
    memcpy(&output->_41, &xBits, sizeof(xBits));
    memcpy(&output->_42, &yBits, sizeof(yBits));
    memcpy(&output->_43, &zBits, sizeof(zBits));
    return zBits;
}

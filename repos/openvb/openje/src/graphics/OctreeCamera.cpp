#include "OctreeCamera.hpp"
#include <math.h>
#include <new>
#include <stdexcept>
#include <string.h>

// Actual separate float operands; shared readonly storage prevents reassociating
// their ordered products into a rounded compile-time conversion constant.
extern const float g_GfxCameraDegreeFraction;
extern const float g_GfxCameraPi;

namespace
{
	// Camera-local precision adaptation: real D3DX normalization changed an
	// observed finite result by one ULP. Retain the original wide intermediate
	// arithmetic using the real CRT sqrt, then store the three float results.
	// This is not another vector library or a separately reconstructed unit.
	__declspec(noinline) void NormalizeCameraVector(D3DXVECTOR3* value)
	{
		double x = value->x, y = value->y, z = value->z;
		double reciprocal = 1.0 / sqrt(x * x + y * y + z * z);
		value->y = static_cast<float>(y * reciprocal);
		value->z = static_cast<float>(z * reciprocal);
		value->x = static_cast<float>(x * reciprocal);
	}

	// The observed non-axis transform disagrees with D3DX's SIMD association.
	// This local camera boundary retains the original float output after wide
	// z/y/x products, without providing another general matrix implementation.
	__declspec(noinline) void TransformCameraVector(D3DXVECTOR3* output,
		const D3DXVECTOR3* input, const D3DXMATRIX* matrix)
	{
		double x = input->x, y = input->y, z = input->z;
		float result[3];
		for (DWORD index = 0; index < 3; ++index)
		{
			result[index] = static_cast<float>(matrix->m[2][index] * z
				+ matrix->m[1][index] * y + matrix->m[0][index] * x + matrix->m[3][index]);
		}
		output->x = result[0]; output->y = result[1]; output->z = result[2];
	}
}

OCTREE_Camera::OCTREE_Camera()
	: objectCursor(0), nodes(0), mask(1), buckets(0), objects(0)
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
		for (DWORD index = 0; buckets[index]; ++index) delete[] buckets[index];
		delete[] buckets;
	}
	if (nodes) delete[] nodes;
	if (objects)
	{
		for (DWORD index = 0; objects[index]; ++index)
		{
			--objects[index]->references;
			OctreeSpatialObject* object = objects[index];
			if (!object->references) ::operator delete(object);
			else object->flags &= ~2u;
		}
		delete[] objects;
	}
	::operator delete(this);
	return 0;
}

int OCTREE::CreateCamera(float degrees, float aspect, float nearPlane, float farPlane,
	OCTREE_Camera** output)
{
	OctreeNode** nodes;
	OctreeNodeReference** buckets;
	OctreeSpatialObject** objects = NULL;
	OCTREE_Camera* camera = new OCTREE_Camera;
	if (!camera) goto noCamera;
	nodes = new OctreeNode*[value10 + 1];
	if (!nodes) goto releaseCamera;
	nodes[0] = NULL;
	nodes[value10] = NULL;
	buckets = new OctreeNodeReference*[vertexBufferCount + 1];
	if (!buckets) goto failed;
	memset(buckets, 0, sizeof(OctreeNodeReference*) * (vertexBufferCount + 1));
	for (DWORD index = 0; index < vertexBufferCount; ++index)
	{
		buckets[index] = new OctreeNodeReference[indexedNodeCounts[index] + 1];
		if (!buckets[index]) goto failed;
		buckets[index][indexedNodeCounts[index]].node = NULL;
		buckets[index][0].node = NULL;
	}
	objects = new OctreeSpatialObject*[1025];
	if (!objects) goto failed;
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
			if (buckets[item]) delete[] buckets[item];
		delete[] buckets;
	}
	if (objects) delete[] objects;
releaseCamera:
	camera->Release();
noCamera:
	return 1;
}

void OCTREE_Camera::SetFrustum(float degrees, float aspect, float nearPlane, float farPlane)
{
	// These are the game's degree/angle decisions, not a D3DX perspective matrix.
	volatile float vertical = static_cast<float>(degrees * 0.5 * 0.0055555557f * 3.1415901f);
	double horizontal = 1.570796f - degrees * static_cast<double>(aspect) * 0.5
		* 0.0055555557f * 3.1415901f;
	// Original trigonometric arguments/results cross stored-float call boundaries.
	// Volatile stores prevent VC7 inlining from silently retaining extra precision.
	volatile float sineHorizontal = static_cast<float>(sin(horizontal));
	volatile float cosineHorizontal = static_cast<float>(cos(horizontal));
	volatile float sineVertical = -static_cast<float>(sin(vertical));
	volatile float cosineVertical = static_cast<float>(cos(vertical));
	D3DXVECTOR3 forward(sineHorizontal, 0, cosineHorizontal);
	D3DXVECTOR3 up(0, cosineVertical, sineVertical);
	D3DXVec3Cross(localRays, &forward, &up);
	NormalizeCameraVector(localRays);
	localRays[1] = D3DXVECTOR3(-localRays[0].x, localRays[0].y, localRays[0].z);
	localRays[2] = D3DXVECTOR3(localRays[0].x, -localRays[0].y, localRays[0].z);
	localRays[3] = D3DXVECTOR3(-localRays[0].x, -localRays[0].y, localRays[0].z);
	localRays[4] = D3DXVECTOR3(0, 0, nearPlane);
	localRays[5] = D3DXVECTOR3(0, 0, farPlane);
	D3DXVec3Cross(localPlanes, localRays + 1, localRays);
	NormalizeCameraVector(localPlanes);
	D3DXVec3Cross(localPlanes + 1, localRays + 2, localRays + 3);
	NormalizeCameraVector(localPlanes + 1);
	D3DXVec3Cross(localPlanes + 2, localRays, localRays + 2);
	NormalizeCameraVector(localPlanes + 2);
	D3DXVec3Cross(localPlanes + 3, localRays + 3, localRays + 1);
	NormalizeCameraVector(localPlanes + 3);
	localPlanes[4] = D3DXVECTOR3(0, 0, 1);
	localPlanes[5] = D3DXVECTOR3(0, 0, -1);
	projectionScaleX = projectionScaleY = 1;
	projectionOffsetX = projectionOffsetY = 0;
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
		TransformCameraVector(transformedRays + index, localRays + index, &transform);
	const DWORD order[6] = {3, 4, 1, 2, 0, 5};
	for (DWORD index = 0; index < 6; ++index)
		TransformCameraVector(planes + order[index], localPlanes + index, &transform);
	for (DWORD index = 0; index < 6; ++index)
	{
		const D3DXVECTOR3& normal = planes[index];
		planeCorners[index] = (normal.x > 0 ? 0 : 4) | (normal.y > 0 ? 0 : 2)
			| (normal.z > 0 ? 0 : 1);
	}
	return 0;
}

DWORD OCTREE_Camera::BuildTransform(DWORD xBits, DWORD yBits, DWORD zBits,
	float pitchDegrees, float yawDegrees, D3DXMATRIX* output)
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


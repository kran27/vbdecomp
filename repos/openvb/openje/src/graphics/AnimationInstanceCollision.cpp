#include "AnimationInstance.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "../diagnostics/DebugOutput.hpp"

#include <dxerr8.h>
#include <float.h>
#include <string.h>

namespace
{
	struct AnimationOBBVertex
	{
		D3DXVECTOR3 position;
		DWORD color;
	};
	typedef char AnimationOBBVertexSize[(sizeof(AnimationOBBVertex) == 16) ? 1 : -1];
}

Granny21::Skeleton* Gfx_AnimationInstance::GetSkeleton() const
{
	return animationObject->skeleton;
}

BYTE Gfx_AnimationInstance::RayIntersection(const D3DXMATRIX& transform,
	const D3DXVECTOR3& origin, const D3DXVECTOR3& direction, float& distance)
{
	Granny21::Mesh* collisionMesh = animationObject->collisionMesh;
	Granny21::MeshBinding* binding = animationObject->meshBinding;
	if (!collisionMesh || !binding)
	{
		JE::LogDebugString("Gfx_AnimMeshInstance::RayIntersection - No collision mesh and/or mesh binding!\n");
		return 0;
	}

	D3DXMATRIX inverse;
	D3DXMatrixInverse(&inverse, NULL, &transform);
	D3DXVECTOR3 localOrigin, localDirection;
	D3DXVec3TransformCoord(&localOrigin, &origin, &inverse);
	D3DXVec3TransformNormal(&localDirection, &direction, &inverse);
	memset(collisionBindingFlags, 0, collisionMesh->boneBindingCount);
	float closest = FLT_MAX;
	BYTE hit = 0;
	for (int index = 0; index < collisionMesh->boneBindingCount; ++index)
	{
		Granny21::BoneBinding* box = &collisionMesh->boneBindings[index];
		const int* boneIndices = Granny21::GrannyGetMeshBindingToBoneIndices(binding);
		const D3DXMATRIX* boneTransform = &worldMatrices[boneIndices[index]];
		D3DXMATRIX scaledTransform;
		if (scaling)
		{
			scaledTransform = *boneTransform;
			D3DXMATRIX scalingTransform;
			D3DXMatrixScaling(&scalingTransform, scale[0], scale[1], scale[2]);
			D3DXMatrixMultiply(&scaledTransform, &scalingTransform, &scaledTransform);
			boneTransform = &scaledTransform;
		}
		Granny21::BoxIntersection intersection;
		if (Granny21::GrannyRayIntersectsBoxAt(&boneTransform->_11,
			box->minimum, box->maximum, &localOrigin.x, &localDirection.x, &intersection)
			&& !(intersection.maximumT < 0.0f))
		{
			if (intersection.minimumT < 0.0f && intersection.maximumT > 0.0f)
			{
				closest = 0.0f;
			}
			else if (intersection.minimumT < closest)
			{
				closest = intersection.minimumT;
			}
			hit = 1;
			collisionBindingFlags[index] = 1;
		}
	}
	distance = closest;
	return hit;
}

void Gfx_AnimationInstance::RenderOBB(const D3DXMATRIX& transform) //4A4F10.
{
	if (!flagC4) return;
	Granny21::Mesh* mesh = animationObject->collisionMesh;
	Granny21::MeshBinding* binding = animationObject->meshBinding;
	if (!mesh || !binding)
	{
		JE::LogDebugString("Gfx_AnimMeshInstance::RenderOBB - No collision mesh and/or mesh binding!\n");
		return;
	}
	Gfx_StreamingVertexBuffer* vertexStream = g_BufferManager->vertexStream;
	Gfx_StreamingIndexBuffer* indexStream = g_BufferManager->indexStream;
	WORD edges[24] = {0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};
	g_D3D.device->SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	g_D3D.SetTexture(0, NULL);
	g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	g_D3D.SetRenderState(D3DRS_LIGHTING, 0);
	g_D3D.SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	g_D3D.SetTransform(D3DTS_WORLD, &transform);
	AnimationOBBVertex* vertices;
	UINT vertexBase;
	BYTE vertexDiscard = vertexStream->CanFit(8 * mesh->boneBindingCount, 16);
	HRESULT result = vertexStream->Lock(8 * mesh->boneBindingCount, 16,
		reinterpret_cast<BYTE**>(&vertices), &vertexBase,
		vertexDiscard);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_AnimMeshInstance::RenderOBB - VB Streaming Lock failed! (%x %s)\n",
			result, DXGetErrorDescription8A(result));
		return;
	}
	const BYTE indexDiscard = indexStream->bytePosition + indexStream->indexStride * mesh->boneBindingCount * 24 < indexStream->capacity;
	WORD* indices;
	UINT indexBase;
	result = indexStream->Lock(24 * mesh->boneBindingCount,
		reinterpret_cast<BYTE**>(&indices), &indexBase, indexDiscard);
	if (FAILED(result))
	{
		vertexStream->buffer->Unlock();
		JE::LogDebugString("Gfx_AnimMeshInstance::RenderOBB - IB Streaming Lock failed! (%x %s)\n",
			result, DXGetErrorDescription8A(result));
		return;
	}
	UINT writtenVertices = 0, writtenIndices = 0;
	DWORD muted = 0xFF808000, bright = 0xFFFFFF00;
	for (int index = 0; index < mesh->boneBindingCount; ++index)
	{
		Granny21::BoneBinding* box = &mesh->boneBindings[index];
		const int* boneIndices = Granny21::GrannyGetMeshBindingToBoneIndices(binding);
		D3DXMATRIX boneTransform = worldMatrices[boneIndices[index]];
		if (scaling)
		{
			D3DXMATRIX scalingTransform;
			D3DXMatrixScaling(&scalingTransform, scale[0], scale[1], scale[2]);
			D3DXMatrixMultiply(&boneTransform, &scalingTransform, &boneTransform);
		}
		for (UINT edge = 0; edge < 24; ++edge)
			indices[writtenIndices++] = static_cast<WORD>(writtenVertices + edges[edge]);
		const DWORD& color = collisionBindingFlags[index] ? bright : muted;
		D3DXVECTOR3 point;
		point = D3DXVECTOR3(box->minimum[0], box->minimum[1], box->minimum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->maximum[0], box->minimum[1], box->minimum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->maximum[0], box->minimum[1], box->maximum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->minimum[0], box->minimum[1], box->maximum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->minimum[0], box->maximum[1], box->minimum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->maximum[0], box->maximum[1], box->minimum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->maximum[0], box->maximum[1], box->maximum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->minimum[0], box->maximum[1], box->maximum[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
	}
	vertexStream->buffer->Unlock();
	indexStream->buffer->Unlock();
	++g_D3D.statistics[9];
	g_D3D.device->SetStreamSource(0, vertexStream->buffer, 16);
	g_D3D.device->SetIndices(indexStream->buffer, vertexBase);
	g_D3D.DrawIndexedPrimitive(D3DPT_LINELIST, 0, 8 * mesh->boneBindingCount, indexBase, 12 * mesh->boneBindingCount);
	g_D3D.SetRenderState(D3DRS_LIGHTING, g_GfxSystem->Lighting());
	flagC4 = 0;
}

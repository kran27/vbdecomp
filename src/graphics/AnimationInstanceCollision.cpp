#include "AnimationInstance.hpp"
#include "BufferManager.hpp"
#include "D3DState.hpp"
#include "SystemInterface.hpp"
#include "../diagnostics/DebugOutput.hpp"

#include "DX9Errors.hpp"
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

Granny28::Skeleton* Gfx_AnimationInstance::GetSkeleton() const
{
	return animationObject->skeleton;
}

BYTE Gfx_AnimationInstance::RayIntersection(const D3DXMATRIX& transform,
	const D3DXVECTOR3& origin, const D3DXVECTOR3& direction, float& distance)
{
	Granny28::Mesh* collisionMesh = animationObject->collisionMesh;
	Granny28::MeshBinding* binding = animationObject->meshBinding;
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
	memset(collisionBindingFlags, 0, collisionMesh->BoneBindingCount);
	float closest = FLT_MAX;
	BYTE hit = 0;
	for (int index = 0; index < collisionMesh->BoneBindingCount; ++index)
	{
		Granny28::BoneBinding* box = &collisionMesh->BoneBindings[index];
		const int* boneIndices = Granny28::GrannyGetMeshBindingToBoneIndices(binding);
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
		Granny28::BoxIntersection intersection;
		if (Granny28::GrannyRayIntersectsBoxAt(&boneTransform->_11,
			box->OBBMin, box->OBBMax, &localOrigin.x, &localDirection.x, &intersection)
			&& !(intersection.MaxT < 0.0f))
		{
			if (intersection.MinT < 0.0f && intersection.MaxT > 0.0f)
			{
				closest = 0.0f;
			}
			else if (intersection.MinT < closest)
			{
				closest = intersection.MinT;
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
	Granny28::Mesh* mesh = animationObject->collisionMesh;
	Granny28::MeshBinding* binding = animationObject->meshBinding;
	if (!mesh || !binding)
	{
		JE::LogDebugString("Gfx_AnimMeshInstance::RenderOBB - No collision mesh and/or mesh binding!\n");
		return;
	}
	Gfx_StreamingVertexBuffer* vertexStream = g_BufferManager->vertexStream;
	Gfx_StreamingIndexBuffer* indexStream = g_BufferManager->indexStream;
	WORD edges[24] = {0,1,1,2,2,3,3,0,4,5,5,6,6,7,7,4,0,4,1,5,2,6,3,7};
	g_D3D.device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
	g_D3D.SetTexture(0, NULL);
	g_D3D.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
	g_D3D.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	g_D3D.SetRenderState(D3DRS_LIGHTING, 0);
	g_D3D.SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
	g_D3D.SetTransform(D3DTS_WORLD, &transform);
	AnimationOBBVertex* vertices;
	UINT vertexBase;
	BYTE vertexDiscard = vertexStream->CanFit(8 * mesh->BoneBindingCount, 16);
	HRESULT result = vertexStream->Lock(8 * mesh->BoneBindingCount, 16,
		reinterpret_cast<void**>(&vertices), &vertexBase,
		vertexDiscard);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_AnimMeshInstance::RenderOBB - VB Streaming Lock failed! (%x %s)\n",
			result, DXGetErrorDescription8A(result));
		return;
	}
	const BYTE indexDiscard = indexStream->bytePosition + indexStream->indexStride * mesh->BoneBindingCount * 24 < indexStream->capacity;
	WORD* indices;
	UINT indexBase;
	result = indexStream->Lock(24 * mesh->BoneBindingCount,
		reinterpret_cast<void**>(&indices), &indexBase, indexDiscard);
	if (FAILED(result))
	{
		vertexStream->buffer->Unlock();
		JE::LogDebugString("Gfx_AnimMeshInstance::RenderOBB - IB Streaming Lock failed! (%x %s)\n",
			result, DXGetErrorDescription8A(result));
		return;
	}
	UINT writtenVertices = 0, writtenIndices = 0;
	DWORD muted = 0xFF808000, bright = 0xFFFFFF00;
	for (int index = 0; index < mesh->BoneBindingCount; ++index)
	{
		Granny28::BoneBinding* box = &mesh->BoneBindings[index];
		const int* boneIndices = Granny28::GrannyGetMeshBindingToBoneIndices(binding);
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
		point = D3DXVECTOR3(box->OBBMin[0], box->OBBMin[1], box->OBBMin[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMax[0], box->OBBMin[1], box->OBBMin[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMax[0], box->OBBMin[1], box->OBBMax[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMin[0], box->OBBMin[1], box->OBBMax[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMin[0], box->OBBMax[1], box->OBBMin[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMax[0], box->OBBMax[1], box->OBBMin[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMax[0], box->OBBMax[1], box->OBBMax[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
		point = D3DXVECTOR3(box->OBBMin[0], box->OBBMax[1], box->OBBMax[2]);
		D3DXVec3TransformCoord(&vertices[writtenVertices].position, &point, &boneTransform);
		vertices[writtenVertices++].color = color;
	}
	vertexStream->buffer->Unlock();
	indexStream->buffer->Unlock();
	++g_D3D.statistics[9];
	g_D3D.device->SetStreamSource(0, vertexStream->buffer, 0, 16);
	g_D3D.SetIndices(indexStream->buffer, vertexBase);
	g_D3D.DrawIndexedPrimitive(D3DPT_LINELIST, 0, 8 * mesh->BoneBindingCount, indexBase, 12 * mesh->BoneBindingCount);
	g_D3D.SetRenderState(D3DRS_LIGHTING, g_GfxSystem->Lighting());
	flagC4 = 0;
}

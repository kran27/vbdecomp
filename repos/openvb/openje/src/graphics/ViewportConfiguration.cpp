#include "Viewport.hpp"
#include "RenderManager.hpp"
#include <d3dx8.h>

HRESULT Gfx_Viewport::SetViewport(const D3DVIEWPORT8& descriptor)
{
	// SDK assignment copies all six words, including raw MinZ/MaxZ bits. The
	// source dimensions are read after the copy, as in the original.
	viewport = descriptor;
	const float ratio = static_cast<float>(descriptor.Width) / static_cast<float>(descriptor.Height);
	aspect = ratio;
	camera.aspect = ratio;
	camera.projectionDirty = 1;
	RebuildQuad();
	return renderer->SetViewportQuad(viewport.X, viewport.Y, viewport.Width, viewport.Height);
}

void Gfx_Viewport::RebuildQuad()
{
	const DWORD leftRaw = viewport.X;
	const float left = static_cast<float>(leftRaw);
	vertices[0].x = left;
	const DWORD topRaw = viewport.Y;
	const float top = static_cast<float>(topRaw);
	vertices[0].y = top;
	vertices[0].u = 0.0f;
	vertices[0].v = 0.0f;
	const DWORD rightRaw = leftRaw + viewport.Width;
	vertices[0].z = viewport.MaxZ;
	const DWORD color = clearColor;
	vertices[0].rhw = 1.0f;
	vertices[0].diffuse = color;
	const float right = static_cast<float>(rightRaw);
	vertices[1].x = right;
	vertices[1].z = viewport.MaxZ;
	vertices[1].y = top;
	const DWORD bottomRaw = topRaw + viewport.Height;
	vertices[2].x = left;
	const float bottom = static_cast<float>(bottomRaw);
	vertices[1].rhw = 1.0f;
	vertices[1].diffuse = color;
	vertices[1].u = 1.0f;
	vertices[1].v = 0.0f;
	vertices[2].y = bottom;
	vertices[2].z = viewport.MaxZ;
	vertices[3].y = bottom;
	vertices[3].x = right;
	vertices[2].rhw = 1.0f;
	vertices[2].v = 1.0f;
	vertices[3].rhw = 1.0f;
	vertices[3].u = 1.0f;
	vertices[3].v = 1.0f;
	vertices[2].diffuse = color;
	vertices[2].u = 0.0f;
	vertices[3].z = viewport.MaxZ;
	vertices[3].diffuse = color;

	mesh.Free();
	Gfx_Primitive* primitive = mesh.SetPrimitiveCount(1);
	primitive->type = D3DPT_TRIANGLESTRIP;
	primitive->value28 = 0;
	primitive->value30 = 2;
	Gfx_Material* material = mesh.materialSet.materials;
	material->style1 = 14;
	material->style2 = 14;
	material->mode28 = 1;
	material->mode24 = 1;
	material->textureHandles[0] = 0;
	mesh.vertices = vertices;
	mesh.value00 = 0;
	mesh.value04 = 0xFFFFFFFF;
	const DWORD fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
	mesh.SetVertexDesc(fvf, D3DXGetFVFVertexSize(fvf), 4);
	mesh.materialSet.RebuildGroups();
	instance.SetMesh(&mesh);
}

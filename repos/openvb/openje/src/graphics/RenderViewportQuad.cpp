#include "RenderManager.hpp"
#include "VertexBuffer.hpp"
#include "D3DState.hpp"

HRESULT Gfx_RenderManager::SetViewportQuad(DWORD x, DWORD y, DWORD width, DWORD height)
{
	if (!quadBuffer)
	{
		Gfx_VertexBuffer* created = new Gfx_VertexBuffer;
		quadBuffer = created;
		const DWORD usage = D3DUSAGE_WRITEONLY |
			(g_D3D.currentDevice->guardBand ? D3DUSAGE_DONOTCLIP : 0);
		created->Create(64, usage, D3DFVF_XYZRHW, D3DPOOL_MANAGED);
		// Creation failure is ignored. A callback may replace the published owner.
		quadBuffer->vertexStride = 16;
	}
	BYTE* data;
	const HRESULT result = quadBuffer->buffer->Lock(0, 0, &data, 0);
	if (FAILED(result)) return result;

	D3DXVECTOR4* vertices = reinterpret_cast<D3DXVECTOR4*>(data);
	const float left = static_cast<float>(x);
	vertices[0].x = left;
	const float top = static_cast<float>(y);
	vertices[0].y = top;
	vertices[0].z = 0.001f;
	vertices[0].w = 1.0f;
	const float right = static_cast<float>(x + width);
	vertices[1].x = right;
	vertices[1].y = top;
	vertices[1].z = 0.001f;
	vertices[2].x = left;
	vertices[1].w = 1.0f;
	const float bottom = static_cast<float>(y + height);
	vertices[2].y = bottom;
	vertices[3].y = bottom;
	vertices[2].z = 0.001f;
	vertices[2].w = 1.0f;
	vertices[3].x = right;
	vertices[3].z = 0.001f;
	vertices[3].w = 1.0f;
	// The original reloads both the wrapper and its COM buffer after the writes.
	return quadBuffer->buffer->Unlock();
}

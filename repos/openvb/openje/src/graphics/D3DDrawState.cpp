#include "D3DState.hpp"

HRESULT Gfx_D3D::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, UINT minimumIndex,
	UINT vertexCount, UINT startIndex, UINT primitiveCount) //0x4900A0.
{
	++statistics[12];
	statistics[3] += primitiveCount;
	return device->DrawIndexedPrimitive(type, minimumIndex, vertexCount, startIndex, primitiveCount);
}

HRESULT Gfx_D3D::SetTransform(D3DTRANSFORMSTATETYPE state, const D3DMATRIX* matrix)
{
	HRESULT result = device->SetTransform(state, matrix);
	++statistics[10];
	return result;
}

HRESULT Gfx_D3D::GetRenderState(D3DRENDERSTATETYPE state, DWORD* value)
{
	*value = renderStates[state];
	return 0;
}

HRESULT Gfx_D3D::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	HRESULT result = 0;
	if (renderStates[state] != value)
	{
		result = device->SetRenderState(state, value);
		renderStates[state] = value;
	}
	return result;
}

HRESULT Gfx_D3D::SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE state, DWORD value)
{
	HRESULT result = 0;
	if (textureStates[stage].values[state] != value)
	{
		result = device->SetTextureStageState(stage, state, value);
		textureStates[stage].values[state] = value;
	}
	return result;
}

HRESULT Gfx_D3D::DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount) //490060.
{
	++statistics[11];
	statistics[3] += primitiveCount;
	return device->DrawPrimitive(type, startVertex, primitiveCount);
}

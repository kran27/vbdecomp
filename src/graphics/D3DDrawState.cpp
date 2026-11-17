#include "D3DState.hpp"

HRESULT Gfx_D3D::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, UINT minimumIndex, UINT vertexCount, UINT startIndex,
                                      UINT primitiveCount) // 0x4900A0.
{
    ++statistics[12];
    statistics[3] += primitiveCount;
    return device->DrawIndexedPrimitive(type, indexBaseVertex, minimumIndex, vertexCount, startIndex, primitiveCount);
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

HRESULT Gfx_D3D::SetSoftwareVertexProcessing(BOOL software)
{
    if (!device || !currentMode)
    {
        return D3DERR_INVALIDCALL;
    }
    const DWORD behavior = currentMode->behavior;
    if ((behavior & D3DCREATE_MIXED_VERTEXPROCESSING) != 0)
    {
        return device->SetSoftwareVertexProcessing(software);
    }
    const BOOL fixedSoftware = (behavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING) != 0;
    return (software != FALSE) == (fixedSoftware != FALSE) ? S_OK : D3DERR_INVALIDCALL;
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

HRESULT Gfx_D3D::DrawPrimitive(D3DPRIMITIVETYPE type, UINT startVertex, UINT primitiveCount) // 490060.
{
    ++statistics[11];
    statistics[3] += primitiveCount;
    return device->DrawPrimitive(type, startVertex, primitiveCount);
}

HRESULT Gfx_D3D::SetSamplerState(DWORD stage, D3DSAMPLERSTATETYPE state, DWORD value)
{
    if (stage >= textureStageCount || static_cast<UINT>(state) >= 16)
    {
        return D3DERR_INVALIDCALL;
    }
    HRESULT result = S_OK;
    if (textureStates[stage].samplerValues[state] != value)
    {
        result = device->SetSamplerState(stage, state, value);
        if (SUCCEEDED(result))
        {
            textureStates[stage].samplerValues[state] = value;
        }
    }
    return result;
}

HRESULT Gfx_D3D::SetIndices(IDirect3DIndexBuffer9* buffer, INT baseVertex)
{
    const HRESULT result = device->SetIndices(buffer);
    if (SUCCEEDED(result))
    {
        indexBaseVertex = baseVertex;
    }
    return result;
}

#pragma once
#include <d3d9.h>

inline HRESULT Gfx_SetDX9RenderTarget(IDirect3DDevice9* device, IDirect3DSurface9* color, IDirect3DSurface9* depth)
{
    const HRESULT targetResult = device->SetRenderTarget(0, color);
    if (FAILED(targetResult))
    {
        return targetResult;
    }
    return device->SetDepthStencilSurface(depth);
}

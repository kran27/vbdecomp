#include "BuiltinVertexShaders.hpp"
#include "D3DState.hpp"

IDirect3DVertexDeclaration9* g_VertexShader70D094 = nullptr;
IDirect3DVertexDeclaration9* g_VertexShader70D090 = nullptr;

namespace
{
    const D3DVERTEXELEMENT9 declaration[] = {
        { 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
        { 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };
}

HRESULT Gfx_CreateBuiltinVertexShaders()
{
    HRESULT result = g_D3D.device->CreateVertexDeclaration(declaration, &g_VertexShader70D094);
    if (FAILED(result))
    {
        return result;
    }
    return g_D3D.device->CreateVertexDeclaration(declaration, &g_VertexShader70D090);
}

void Gfx_DeleteBuiltinVertexShaders()
{
    if (g_VertexShader70D094)
    {
        g_VertexShader70D094->Release();
        g_VertexShader70D094 = nullptr;
    }
    if (g_VertexShader70D090)
    {
        g_VertexShader70D090->Release();
        g_VertexShader70D090 = nullptr;
    }
}

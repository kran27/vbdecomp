#include "BuiltinVertexShaders.hpp"
#include "D3DState.hpp"

DWORD g_VertexShader70D094;
DWORD g_VertexShader70D090;

namespace
{
	const DWORD declaration700494[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
		D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
		D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
		D3DVSD_END()
	};
	const DWORD declaration7004A8[] =
	{
		D3DVSD_STREAM(0),
		D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT3),
		D3DVSD_REG(D3DVSDE_DIFFUSE, D3DVSDT_D3DCOLOR),
		D3DVSD_REG(D3DVSDE_TEXCOORD0, D3DVSDT_FLOAT2),
		D3DVSD_END()
	};
}

HRESULT Gfx_CreateBuiltinVertexShaders()
{
	g_D3D.device->CreateVertexShader(declaration700494, NULL, &g_VertexShader70D094, 0);
	return g_D3D.device->CreateVertexShader(declaration7004A8, NULL, &g_VertexShader70D090, 0);
}

void Gfx_DeleteBuiltinVertexShaders()
{
	if (g_VertexShader70D094)
	{
		g_D3D.device->DeleteVertexShader(g_VertexShader70D094);
		g_VertexShader70D094 = 0;
	}
	if (g_VertexShader70D090)
	{
		g_D3D.device->DeleteVertexShader(g_VertexShader70D090);
		g_VertexShader70D090 = 0;
	}
}

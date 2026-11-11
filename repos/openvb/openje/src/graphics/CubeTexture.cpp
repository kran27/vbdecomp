#include "CubeTexture.hpp"
#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <d3dx8.h>

Gfx_CubeTex::Gfx_CubeTex() : Gfx_Texture(1)
{
	cube = NULL;
}

Gfx_CubeTex::~Gfx_CubeTex()
{
	if (cube)
	{
		cube->Release();
		cube = NULL;
	}
}

HRESULT Gfx_CubeTex::Bind(UINT stage, int)
{
	return g_D3D.SetTexture(stage, cube);
}

HRESULT Gfx_CubeTex::LoadFromFile(std::string path)
{
	if (cube)
	{
		cube->Release();
		cube = NULL;
	}
	D3DXIMAGE_INFO information;
	HRESULT result = D3DXCreateCubeTextureFromFileExA(g_D3D.device, path.c_str(),
		D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_FILTER_NONE, D3DX_FILTER_BOX, 0, &information, NULL, &cube);
	if (FAILED(result))
	{
		JE::LogDebugString("Gfx_CubeTex: Failed to load Cube Texture: %s\n", path.c_str());
		char message[256];
		D3DXGetErrorStringA(result, message, sizeof(message));
		JE::LogDebugString("DXErr: %s\n", message);
	}
	return result;
}

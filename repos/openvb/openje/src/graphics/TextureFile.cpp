#include "Texture.hpp"
#include "D3DState.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <d3dx8.h>
#include <dxerr8.h>
#include <mbstring.h>

HRESULT Gfx_Texture::LoadFromFile(std::string path, D3DFORMAT format, D3DPOOL pool,
	UINT levels, DWORD filter, DWORD mipFilter, DWORD usage)
{
	name.erase();
	value20 = 0;
	ResizeFrames(1);
	if (path.size() >= 4)
	{
		std::string extension(path.c_str() + path.size() - 4);
		if (!_mbsicmp(reinterpret_cast<const unsigned char*>(extension.c_str()),
			reinterpret_cast<const unsigned char*>(".ANM")))
		{
			HRESULT result = LoadAnimationFile(path, format, pool, levels, filter, mipFilter, usage);
			if (SUCCEEDED(result)) return result;
		}
	}
	ResizeFrames(1);
	return LoadFrameFromFile(path, format, pool, levels, filter, mipFilter, usage, 0);
}

HRESULT Gfx_Texture::LoadFrameFromFile(std::string path, D3DFORMAT format, D3DPOOL pool,
	UINT levels, DWORD filter, DWORD mipFilter, DWORD usage, int frame)
{
	if (frame < 0 || frame >= frameCount) return D3DERR_INVALIDCALL;
	IDirect3DTexture8*& texture = frames[frame].texture;
	if (texture)
	{
		if (texture)
		{
			texture->Release();
			texture = NULL;
		}
	}
	D3DXIMAGE_INFO information;
	HRESULT result = D3DXCreateTextureFromFileExA(g_D3D.device, path.c_str(),
		D3DX_DEFAULT, D3DX_DEFAULT, levels, usage, format, pool, filter, mipFilter,
		0, &information, NULL, &frames[frame].texture);
	if (SUCCEEDED(result))
		UpdateMetadata();
	else
	{
		std::string unusedMessage("Gfx_Texture::LoadTextureFrameFromFile() cannot load: ");
		unusedMessage += path;
		JE::LogDebugString("Gfx_Texture::LoadTextureFrameFromFile() failed to load: %s (%x %s)\n",
			path.c_str(), result, DXGetErrorDescription8A(result));
	}
	return result;
}

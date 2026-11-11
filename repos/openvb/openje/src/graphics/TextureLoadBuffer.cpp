#include "Texture.hpp"
#include "D3DState.hpp"
#include "../JE.hpp"
#include <d3dx8.h>
#include <dxerr8.h>
#include <string.h>

HRESULT Gfx_Texture::LoadFromBuffer(const BYTE* data, UINT size, D3DFORMAT format,
	D3DPOOL pool, UINT levels, DWORD filter, DWORD mipFilter, DWORD usage)
{
	name.erase();
	value20 = 0;
	ResizeFrames(1);
	if (size >= 8 && data[0] == 'A' && data[1] == 'N'
		&& data[2] == 'M' && data[3] == 'T' && data[4] == 'V'
		&& data[5] == '1' && data[6] == '.' && data[7] == '0')
	{
		HRESULT result = LoadAnimationBuffer(data, static_cast<int>(size), format,
			pool, levels, filter, mipFilter, usage);
		if (SUCCEEDED(result)) return result;
	}
	ResizeFrames(1);
	return LoadFrameFromBuffer(data, size, format, pool, levels, filter, mipFilter, usage, 0);
}

HRESULT Gfx_Texture::LoadFrameFromBuffer(const BYTE* data, UINT size, D3DFORMAT format,
	D3DPOOL pool, UINT levels, DWORD filter, DWORD mipFilter, DWORD usage, int frame)
{
	if (frame < 0 || frame >= frameCount) return D3DERR_INVALIDCALL;
	IDirect3DTexture8*& texture = frames[frame].texture;
	if (texture)
	{
		texture->Release();
		texture = NULL;
	}
	D3DXIMAGE_INFO information;
	HRESULT result = D3DXCreateTextureFromFileInMemoryEx(g_D3D.device, data, size,
		D3DX_DEFAULT, D3DX_DEFAULT, levels, usage, format, pool, filter, mipFilter,
		0, &information, NULL, &frames[frame].texture);
	if (FAILED(result))
		JE::LogDebugString("Gfx_Texture::LoadTextureFrameFromBuffer() failed to load (%x %s)\n",
			result, DXGetErrorDescription8A(result));
	else UpdateMetadata();
	return result;
}

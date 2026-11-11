#include "Texture.hpp"
#include "D3DState.hpp"
#include "../JE.hpp"
#include <dxerr8.h>

HRESULT Gfx_Texture::CreateTexture(UINT width, UINT height, UINT levels,
	D3DFORMAT format, D3DPOOL pool, DWORD usage, int frame)
{
	if (frame >= 0 && frame < frameCount)
	{
		HRESULT result;
		if (!frames[frame].texture)
		{
			result = g_D3D.device->CreateTexture(width, height, levels, usage, format,
				pool, &frames[frame].texture);
			if (SUCCEEDED(result)) UpdateMetadata();
			else JE::LogDebugString("Gfx_Texture::CreateTexture() failed (%x %s)\n", result,
				DXGetErrorDescription8A(result));
		}
		else
		{
			result = D3DERR_INVALIDCALL;
			JE::LogDebugString("Gfx_Texture: Must release texture before creating a new one");
		}
		return result;
	}
	return D3DERR_INVALIDCALL;
}

Gfx_Texture* Gfx_Texture::CloneDefaultToSystem()
{
	Gfx_Texture* copy = new Gfx_Texture(1);
	if (frameCount > 0)
	{
		copy->ResizeFrames(frameCount);
		for (int frame = 0; frame < frameCount; ++frame)
		{
			D3DPOOL pool = D3DPOOL_DEFAULT;
			if (frame >= 0 && frame < frameCount) pool = frames[frame].pool;
			if (pool == D3DPOOL_DEFAULT)
			{
				copy->CreateTexture(frames[frame].width, frames[frame].height,
					frames[frame].mipCount, frames[frame].format, D3DPOOL_SYSTEMMEM, 0, frame);
				for (UINT level = 0; level < frames[frame].mipCount; ++level)
				{
					IDirect3DSurface8* source = NULL;
					IDirect3DSurface8* destination = NULL;
					frames[frame].texture->GetSurfaceLevel(level, &source);
					copy->frames[frame].texture->GetSurfaceLevel(level, &destination);
					g_D3D.device->CopyRects(source, NULL, 0, destination, NULL);
					if (source) { source->Release(); source = NULL; }
					if (destination) { destination->Release(); destination = NULL; }
				}
			}
		}
	}
	return copy;
}

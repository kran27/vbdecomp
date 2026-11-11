#include "Texture.hpp"
#include "D3DState.hpp"
#include "../JE.hpp"
#include <d3dx8.h>
#include <dxerr8.h>

HRESULT Gfx_Texture::CreateSystemTexture(std::string textureName, UINT width, UINT height,
	UINT levels, D3DFORMAT format, int frame)
{
	if (frame < 0 || frame >= frameCount) return D3DERR_INVALIDCALL;
	name = textureName;
	frames[frame].name = textureName;
	return CreateTexture(width, height, levels, format, D3DPOOL_SYSTEMMEM, 0, frame);
}

BYTE Gfx_Texture::CopyRect(UINT destinationMip, const RECT* destinationRect,
	Gfx_Texture* source, UINT sourceMip, const RECT* sourceRect,
	int sourceFrame, int destinationFrame)
{
	if (sourceFrame < 0 || sourceFrame >= source->frameCount
		|| destinationFrame < 0 || destinationFrame >= frameCount) return 0;
	// Original output storage reuses the source-mip and source-owner input slots.
	// Preserve their bits if GetSurfaceLevel fails without writing its output.
	IDirect3DSurface8* sourceSurface = reinterpret_cast<IDirect3DSurface8*>(sourceMip);
	IDirect3DSurface8* destinationSurface = reinterpret_cast<IDirect3DSurface8*>(source);
	BYTE copied = 1;
	HRESULT result = source->frames[sourceFrame].texture->GetSurfaceLevel(sourceMip, &sourceSurface);
	if (FAILED(result))
	{
		copied = 0;
		const char* textureName = source->name.c_str();
		const char* error = DXGetErrorDescription8A(result);
		JE::LogDebugString("Gfx_Texture::CopyRect - Get Src mip-level failed %s (%x %s)\n",
			textureName, result, error);
	}
	result = frames[destinationFrame].texture->GetSurfaceLevel(destinationMip, &destinationSurface);
	if (FAILED(result))
	{
		copied = 0;
		const char* textureName = name.c_str();
		const char* error = DXGetErrorDescription8A(result);
		JE::LogDebugString("Gfx_Texture::CopyRect - Get Dest mip-level failed %s (%x %s)\n",
			textureName, result, error);
	}
	result = D3DXLoadSurfaceFromSurface(destinationSurface, NULL, destinationRect,
		sourceSurface, NULL, sourceRect, D3DX_FILTER_BOX, 0);
	if (FAILED(result))
	{
		copied = 0;
		const char* textureName = source->name.c_str();
		const char* error = DXGetErrorDescription8A(result);
		JE::LogDebugString("Gfx_Texture::CopyRect - Loading Surface failed %s (%x %s)\n",
			textureName, result, error);
	}
	if (destinationSurface)
	{
		destinationSurface->Release();
		destinationSurface = NULL;
	}
	if (sourceSurface) sourceSurface->Release();
	return copied;
}

HRESULT Gfx_Texture::UpdateDefaultTexture(Gfx_Texture* source, int sourceFrame, int destinationFrame)
{
	if (sourceFrame >= 0 && sourceFrame < source->frameCount
		&& destinationFrame >= 0 && destinationFrame < frameCount)
	{
		HRESULT result = D3DERR_INVALIDCALL;
		if (frames[destinationFrame].pool == D3DPOOL_DEFAULT
			&& source->frames[sourceFrame].pool == D3DPOOL_SYSTEMMEM)
		{
			result = source->frames[sourceFrame].texture->AddDirtyRect(NULL);
			if (FAILED(result))
				JE::LogDebugString("UpdateDefaultTexture() - AddDirtyRect failed %s (%x %s)\n",
					name.c_str(), result, DXGetErrorDescription8A(result));
			result = g_D3D.device->UpdateTexture(source->frames[sourceFrame].texture, frames[destinationFrame].texture);
			if (FAILED(result))
				JE::LogDebugString("Gfx_Texture::UpdateDefaultTexture() failed %s (%x %s)\n",
					name.c_str(), result, DXGetErrorDescription8A(result));
		}
		return result;
	}
	return D3DERR_INVALIDCALL;
}

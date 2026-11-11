#include "Texture.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <dxerr8.h>

HRESULT Gfx_Texture::LockRect(UINT level, D3DLOCKED_RECT* locked,
	const RECT* rectangle, DWORD flags, int frame)
{
	if (frame < 0 || frame >= frameCount) return D3DERR_INVALIDCALL;
	HRESULT result = frames[frame].texture->LockRect(level, locked, rectangle, flags);
	frames[frame].flag40 = 1;
	if (FAILED(result))
	{
		const char* textureName = name.c_str();
		const char* error = DXGetErrorDescription8A(result);
		JE::LogDebugString("Gfx_Texture::LockRect() failed %s (%x %s)\n",
			textureName, result, error);
		frames[frame].flag40 = 0;
	}
	frames[frame].value3C = level;
	return result;
}

HRESULT Gfx_Texture::UnlockRect(int frame)
{
	if (frame >= 0 && frame < frameCount)
	{
		HRESULT result = D3D_OK;
		BYTE* locked = &frames[frame].flag40;
		if (*locked)
		{
			*locked = 0;
			result = frames[frame].texture->UnlockRect(frames[frame].value3C);
		}
		return result;
	}
	return D3DERR_INVALIDCALL;
}

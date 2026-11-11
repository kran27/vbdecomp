#include "Texture.hpp"

BYTE Gfx_Texture::ClearFrame(int frame)
{
	if (frame < 0 || frame >= frameCount)
	{
		return 0;
	}
	frames[frame].name.erase();
	IDirect3DTexture8*& texture = frames[frame].texture;
	if (texture)
	{
		texture->Release();
		texture = NULL;
	}
	frames[frame].width = 0;
	frames[frame].height = 0;
	frames[frame].bitsPerPixelWord = 0;
	frames[frame].totalSize = 0;
	frames[frame].format = D3DFMT_UNKNOWN;
	frames[frame].pool = D3DPOOL_MANAGED;
	frames[frame].value3C = 0;
	frames[frame].flag40 = 0;
	return 1;
}

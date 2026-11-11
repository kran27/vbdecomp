#include "Texture.hpp"

Gfx_Texture::Gfx_Texture(int count)
{
	value20 = 0;
	frames = NULL;
	frameCount = 0;
	value30 = 0;
	referenceCount = 1;
	ResizeFrames(count);
}

Gfx_Texture::~Gfx_Texture()
{
	if (frames)
	{
		for (int i = 0; i < frameCount; ++i)
		{
			IDirect3DTexture8*& texture = frames[i].texture;
			if (texture)
			{
				texture->Release();
				texture = NULL;
			}
		}
		delete[] frames;
	}
}

void Gfx_Texture::ResizeFrames(int count)
{
	if (frames)
	{
		for (int i = 0; i < frameCount; ++i)
		{
			IDirect3DTexture8*& texture = frames[i].texture;
			if (texture)
			{
				texture->Release();
				texture = NULL;
			}
		}
		delete[] frames;
		frames = NULL;
	}
	if (count > 0)
	{
		frames = new Gfx_TextureFrame[count];
		if (frames)
		{
			frameCount = count;
			for (int i = 0; i < count; ++i)
			{
				frames[i].width = 0;
				frames[i].height = 0;
				frames[i].bitsPerPixelWord = 0;
				frames[i].totalSize = 0;
				frames[i].format = D3DFMT_UNKNOWN;
				frames[i].pool = D3DPOOL_MANAGED;
				frames[i].value3C = 0;
				frames[i].flag40 = 0;
				frames[i].texture = NULL;
			}
		}
		else frameCount = 0;
	}
	else frameCount = 0;
}

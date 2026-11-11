#include "G3DModel.hpp"
#include <stddef.h>
#include <string.h>

typedef char G3DTexturePixelExtent[sizeof(G3D_TexturePixel) == 4 ? 1 : -1];
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char G3DTextureExtent[sizeof(G3D_Texture) == 0x4C ? 1 : -1];
#endif
typedef char G3DTextureName[offsetof(G3D_Texture, name) == 0 ? 1 : -1];
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
typedef char G3DTextureFile[offsetof(G3D_Texture, file) == 0x1C ? 1 : -1];
typedef char G3DTextureFlags[offsetof(G3D_Texture, flags) == 0x38 ? 1 : -1];
typedef char G3DTextureCube[offsetof(G3D_Texture, cube) == 0x3C ? 1 : -1];
typedef char G3DTextureWidth[offsetof(G3D_Texture, width) == 0x40 ? 1 : -1];
typedef char G3DTextureHeight[offsetof(G3D_Texture, height) == 0x44 ? 1 : -1];
typedef char G3DTexturePixels[offsetof(G3D_Texture, pixels) == 0x48 ? 1 : -1];
#endif

G3D_TexturePixel::G3D_TexturePixel() //502110; ordinary four-byte zero color.
{
	memset(this, 0, sizeof(*this));
}

G3D_Texture::G3D_Texture(const G3D_Texture& other) //4D0150.
	: name(other.name), file(other.file)
{
	flags[0] = other.flags[0]; flags[1] = other.flags[1];
	flags[2] = other.flags[2]; flags[3] = other.flags[3];
	cube = other.cube;
	width = other.width; height = other.height;
	pixels = other.pixels; //Original shallow copy, including its shared-deletion domain.
}

G3D_Texture::~G3D_Texture() //442230.
{
	if (pixels) { ::operator delete(pixels); pixels = NULL; }
}

void G3D_Texture::ClearPixels() //4CA250.
{
	if (pixels) { delete[] pixels; pixels = NULL; }
}

LONG G3D_Texture::SetWidth(LONG value) //4CA270.
{
	ClearPixels(); width = value; return value;
}
LONG G3D_Texture::SetHeight(LONG value) //4CA290.
{
	ClearPixels(); height = value; return value;
}

void G3D_Texture::SetPixel(LONG x, LONG y, const G3D_TexturePixel& value) //4C8CD0.
{
	if (!pixels)
	{
		LONG columns = width;
		LONG rows = height;
		pixels = new G3D_TexturePixel[columns * rows];
	}
	pixels[x + width * y] = value;
}

BYTE G3D_Texture::LoadImage(const BYTE* data, BYTE expand24) //4CA2B0.
{
	G3D_TexturePixel color;
	ClearPixels();
	if (!data) return 0;
	WORD imageWidth, imageHeight, colorMapLength;
	memcpy(&imageWidth, data + 12, sizeof(imageWidth));
	memcpy(&imageHeight, data + 14, sizeof(imageHeight));
	memcpy(&colorMapLength, data + 5, sizeof(colorMapLength));
	LONG stride = data[16] >> 3;
	if (expand24 && data[16] == 24) ++stride;
	LONG count = stride * imageHeight * imageWidth;
	if (count > 0)
	{
		BYTE* temporary = new BYTE[count];
		if (!temporary) return 0;
		LONG colorMapBytes = 0;
		if (data[1] == 1) colorMapBytes = colorMapLength * data[7];
		SetWidth(imageWidth);
		SetHeight(imageHeight);
		const BYTE* input = data + colorMapBytes + data[0] + 18;
		if (data[2] != 2 || data[16] != 24)
		{
			delete[] temporary;
			return 0;
		}
		if (expand24)
		{
			BYTE* output = temporary;
			DWORD remaining = ((DWORD(count) - 1) >> 2) + 1;
			do
			{
				output[0] = input[0]; output[1] = input[1]; output[2] = input[2]; output[3] = 0;
				input += 3; output += 4;
			} while (--remaining);
		}
		else memcpy(temporary, input, count);
		BYTE* pixel = temporary;
		LONG y = 0;
		if (imageHeight)
		{
			do
			{
				for (LONG x = 0; x < imageWidth; ++x)
				{
					color.red = pixel[2]; color.green = pixel[1]; color.blue = pixel[0];
					//Flag0 preserves the original unused fourth-byte load even for the
					//last three-byte temporary pixel. No sentinel byte is fabricated.
					if (expand24) { color.alpha = 0; pixel += 4; }
					else { color.alpha = pixel[3]; pixel += 3; }
					SetPixel(x, y, color);
				}
				++y;
			} while (y < imageHeight);
		}
		delete[] temporary;
	}
	return 1;
}

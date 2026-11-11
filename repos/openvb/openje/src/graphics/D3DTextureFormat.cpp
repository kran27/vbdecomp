#include "D3DState.hpp"
#include <cstring>

// Original0x503450: the game's selected format names/bits, not a complete SDK
// format catalog. Unknown and compressed formats deliberately report zero bits.
void Gfx_D3D::DescribeTextureFormat(int format, DWORD* bits, char* name)
{
	switch (format)
	{
	case D3DFMT_A8R8G8B8:
		if (name) std::strcpy(name, "A8R8G8B8");
		*bits = 32;
		break;
	case D3DFMT_X8R8G8B8:
		if (name) std::strcpy(name, "X8R8G8B8");
		*bits = 32;
		break;
	case D3DFMT_R5G6B5:
		if (name) std::strcpy(name, "R5G6B5");
		*bits = 16;
		break;
	case D3DFMT_X1R5G5B5:
		if (name) std::strcpy(name, "X1R5G5B5");
		*bits = 16;
		break;
	case D3DFMT_A1R5G5B5:
		if (name) std::strcpy(name, "A1R5G5B5");
		*bits = 16;
		break;
	case D3DFMT_A4R4G4B4:
		if (name) std::strcpy(name, "A4R4G4B4");
		*bits = 16;
		break;
	case D3DFMT_A8R3G3B2:
		if (name) std::strcpy(name, "A8R3G3B2");
		*bits = 16;
		break;
	case D3DFMT_X4R4G4B4:
		if (name) std::strcpy(name, "X4R4G4B4");
		*bits = 16;
		break;
	case D3DFMT_A8P8:
		if (name) std::strcpy(name, "A8P8");
		*bits = 16;
		break;
	case D3DFMT_P8:
		if (name) std::strcpy(name, "P8");
		*bits = 8;
		break;
	case D3DFMT_DXT1:
		if (name) std::strcpy(name, "DXT1");
		*bits = 0;
		break;
	case D3DFMT_DXT2:
		if (name) std::strcpy(name, "DXT2");
		*bits = 0;
		break;
	case D3DFMT_DXT3:
		if (name) std::strcpy(name, "DXT3");
		*bits = 0;
		break;
	case D3DFMT_DXT4:
		if (name) std::strcpy(name, "DXT4");
		*bits = 0;
		break;
	case D3DFMT_DXT5:
		if (name) std::strcpy(name, "DXT5");
		*bits = 0;
		break;
	default:
		if (name) std::strcpy(name, "Unknown");
		*bits = 0;
		break;
	}
}

//5037B0: the game's depth-format description, using official SDK enums.
//The D15S1 case reports16 depth bits plus1 stencil bit, as in the original.
BYTE* Gfx_D3D::DescribeDepthStencilFormat(D3DFORMAT format, DWORD* depth, DWORD* stencil, BYTE* lockable)
{
	switch (format)
	{
	case D3DFMT_D16_LOCKABLE: *lockable = 1; *depth = 16; *stencil = 0; break;
	case D3DFMT_D32: *lockable = 0; *depth = 32; *stencil = 0; break;
	case D3DFMT_D15S1: *lockable = 0; *depth = 16; *stencil = 1; break;
	case D3DFMT_D24S8: *lockable = 0; *depth = 24; *stencil = 8; break;
	case D3DFMT_D16: *lockable = 0; *depth = 16; *stencil = 0; break;
	case D3DFMT_D24X8: *lockable = 0; *depth = 24; *stencil = 0; break;
	case D3DFMT_D24X4S4: *lockable = 0; *depth = 24; *stencil = 4; break;
	default: *lockable = 0; *depth = 0; *stencil = 0; break;
	}
	return lockable;
}

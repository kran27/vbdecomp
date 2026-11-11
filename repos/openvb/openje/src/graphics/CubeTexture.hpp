#pragma once
#include "Texture.hpp"

// Actual Gfx_CubeTex RTTI/table; base frame metadata stays as originally constructed.
class Gfx_CubeTex : public Gfx_Texture
{
public:
	Gfx_CubeTex(); // 0x538AD0; ordinary52-byte base plus owned COM pointer.
	virtual ~Gfx_CubeTex(); // 0x538AF0; ordinary destruction, then actual base.
	virtual HRESULT Bind(UINT stage, int frame); // 0x538980; ignores frame argument.
	HRESULT LoadFromFile(std::string path); // 0x5389A0; its own fixed SDK options.
	IDirect3DCubeTexture8* cube; // +34; owned, independent of base frame array.
};

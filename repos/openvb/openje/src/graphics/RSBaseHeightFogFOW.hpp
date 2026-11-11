#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_BaseHeightFogFOWState
{
	DWORD users;
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE padding16[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_BaseHeightFogFOWState g_RSBaseHeightFogFOWState; //70BCA4.

class Gfx_RS_Base_HFog_FOW : public Gfx_RenderStyle
{
public:
	Gfx_RS_Base_HFog_FOW(DWORD color, DWORD alpha, DWORD test); //53F100.
	virtual ~Gfx_RS_Base_HFog_FOW(); //537430.
	virtual HRESULT Initialize(); //53F120.
	virtual void Release(); //5362A0.
	virtual HRESULT OnLostDevice(); //5362D0.
	virtual HRESULT OnResetDevice(); //5362F0.
	virtual DWORD TechniqueCount(); //5374A0.
	virtual DWORD SelectTechnique(DWORD index); //537520.
	virtual DWORD NextTechnique(); //537550.
	virtual BYTE UsesVertexShader(); //5374B0.
	virtual DWORD PassCount(); //5374C0.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD fogBits); //53F360, RET8.
	virtual HRESULT Begin(UINT* passes); //5374D0.
	virtual HRESULT End(); //5374F0.
	virtual HRESULT Pass(UINT pass); //537500.

	DWORD colorOperation, alphaOperation, alphaTest;
};

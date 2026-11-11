#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_BaseLightmapHeightFogFOWState
{
	DWORD users;
	ID3DXEffect* normal;
	ID3DXEffect* factored;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE padding1A[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_BaseLightmapHeightFogFOWState g_RSBaseLightmapHeightFogFOWState; //70BCC4.

class Gfx_RS_B_LM_HFog_FOW : public Gfx_RenderStyle
{
public:
	Gfx_RS_B_LM_HFog_FOW(DWORD color, DWORD alpha, DWORD lightmapColor,
		DWORD lightmapAlpha, DWORD test); //53FBA0.
	virtual ~Gfx_RS_B_LM_HFog_FOW(); //537590.
	virtual HRESULT Initialize(); //53FBE0.
	virtual void Release(); //536310.
	virtual HRESULT OnLostDevice(); //536360.
	virtual HRESULT OnResetDevice(); //536390.
	virtual DWORD TechniqueCount(); //537620.
	virtual DWORD SelectTechnique(DWORD index); //537670.
	virtual DWORD NextTechnique(); //5376A0.
	virtual BYTE UsesVertexShader(); //537630.
	virtual DWORD PassCount(); //537640.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD fogBits); //540040, RET8.
	virtual HRESULT Begin(UINT* passes); //537BA0.
	virtual HRESULT End(); //537BC0.
	virtual HRESULT Pass(UINT pass); //537650.

	ID3DXEffect* effect; //Borrowed selected effect, original+04; ctorNULL.
	DWORD colorOperation, alphaOperation;
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest;
};

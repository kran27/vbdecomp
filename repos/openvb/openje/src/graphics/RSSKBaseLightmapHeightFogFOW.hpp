#pragma once

#include "RenderStyle.hpp"
#include <d3dx8effect.h>

// Original loader-zero shared storage70BD6C..70BD8F.
struct Gfx_RS_SK_B_LM_HFog_FOWState
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
extern Gfx_RS_SK_B_LM_HFog_FOWState g_RSSKBaseLightmapHeightFogFOWState;

class Gfx_RS_SK_B_LM_HFog_FOW : public Gfx_RenderStyle
{
public:
	Gfx_RS_SK_B_LM_HFog_FOW(DWORD color, DWORD alpha, DWORD lightmapColor,
		DWORD lightmapAlpha, DWORD test); //542740.
	virtual ~Gfx_RS_SK_B_LM_HFog_FOW(); //537AE0.
	virtual HRESULT Initialize(); //542780.
	virtual void Release(); //536710; unused original EAX.
	virtual HRESULT OnLostDevice(); //536760.
	virtual HRESULT OnResetDevice(); //536790.
	virtual DWORD TechniqueCount(); //537B70.
	virtual DWORD SelectTechnique(DWORD index); //537BF0.
	virtual DWORD NextTechnique(); //537C20.
	virtual BYTE UsesVertexShader(); //537B80.
	virtual DWORD PassCount(); //537B90.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD fogBits); //542BE0; Gfx_HeightFog* transport.
	virtual HRESULT Begin(UINT* passes); //Shared537BA0, selected instance effect.
	virtual HRESULT End(); //Shared537BC0.
	virtual HRESULT Pass(UINT pass); //537BD0.

	ID3DXEffect* effect; //+04, borrowed selection; constructorNULL.
	DWORD colorOperation, alphaOperation; //+08/+0C.
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest; //+10/+14/+18; raw test.
};

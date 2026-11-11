#pragma once

#include "RenderStyle.hpp"
#include <d3dx8effect.h>

// Original loader-zero shared storage70BD28..70BD4B; SDK owns both effects.
struct Gfx_RS_SK_B_LM_HeightFogState
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
extern Gfx_RS_SK_B_LM_HeightFogState g_RSSKBaseLightmapHeightFogState;

class Gfx_RS_SK_B_LM_HeightFog : public Gfx_RenderStyle
{
public:
	Gfx_RS_SK_B_LM_HeightFog(DWORD color, DWORD alpha, DWORD lightmapColor,
		DWORD lightmapAlpha, DWORD test); //541340.
	virtual ~Gfx_RS_SK_B_LM_HeightFog(); //537830.
	virtual HRESULT Initialize(); //541380.
	virtual void Release(); //5365F0; unused original EAX.
	virtual HRESULT OnLostDevice(); //536640.
	virtual HRESULT OnResetDevice(); //536670.
	virtual DWORD TechniqueCount(); //5378C0.
	virtual DWORD SelectTechnique(DWORD index); //537910.
	virtual DWORD NextTechnique(); //537940.
	virtual BYTE UsesVertexShader(); //5378D0.
	virtual DWORD PassCount(); //5378E0.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD fogBits); //5417B0; Gfx_HeightFog* transport.
	virtual HRESULT Begin(UINT* passes); //Shared537BA0, selected instance effect.
	virtual HRESULT End(); //Shared537BC0.
	virtual HRESULT Pass(UINT pass); //5378F0.

	ID3DXEffect* effect; //+04, borrowed selection; constructorNULL.
	DWORD colorOperation, alphaOperation; //+08/+0C.
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest; //+10/+14/+18; raw test.
};

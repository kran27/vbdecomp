#pragma once
#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_SK_B_LMFOWState
{
	DWORD users; //70BD4C.
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE padding16[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_SK_B_LMFOWState g_RSSKBaseLightmapFOWState;

class Gfx_RS_SK_B_LM_FOW : public Gfx_RenderStyle
{
public:
	Gfx_RS_SK_B_LM_FOW(DWORD color, DWORD alpha, DWORD lightmapColor,
		DWORD lightmapAlpha, DWORD test); //541F90.
	virtual ~Gfx_RS_SK_B_LM_FOW(); //537980.
	virtual HRESULT Initialize(); //541FC0.
	virtual void Release(); //5366A0; source void return hypothesis.
	virtual HRESULT OnLostDevice(); //5366D0.
	virtual HRESULT OnResetDevice(); //5366F0.
	virtual DWORD TechniqueCount(); //5379F0.
	virtual DWORD SelectTechnique(DWORD index); //537A70.
	virtual DWORD NextTechnique(); //537AA0.
	virtual BYTE UsesVertexShader(); //537A00.
	virtual DWORD PassCount(); //537A10.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD texture); //542230.
	virtual HRESULT Begin(UINT* passes); //537A20.
	virtual HRESULT End(); //537A40.
	virtual HRESULT Pass(UINT pass); //537A50.

	DWORD colorOperation, alphaOperation; //+04/+08; no selected-effect member.
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest; //+0C/+10/+14, raw test.
};


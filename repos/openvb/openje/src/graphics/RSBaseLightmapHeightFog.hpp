#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_BaseLightmapHeightFogState
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
extern Gfx_RS_BaseLightmapHeightFogState g_RSBaseLightmapHeightFogState; //70BC3C.

class Gfx_RS_B_LM_HeightFog : public Gfx_RenderStyle
{
public:
	Gfx_RS_B_LM_HeightFog(DWORD color, DWORD alpha, DWORD lightmapColor,
		DWORD lightmapAlpha, DWORD test); //53D550.
	virtual ~Gfx_RS_B_LM_HeightFog(); //537030.
	virtual HRESULT Initialize(); //53D590.
	virtual void Release(); //5360D0.
	virtual HRESULT OnLostDevice(); //536120.
	virtual HRESULT OnResetDevice(); //536150.
	virtual DWORD TechniqueCount(); //5370C0.
	virtual DWORD SelectTechnique(DWORD index); //537110.
	virtual DWORD NextTechnique(); //537140.
	virtual BYTE UsesVertexShader(); //5370D0.
	virtual DWORD PassCount(); //5370E0.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD fogBits); //53D970, RET8.
	virtual HRESULT Begin(UINT* passes); //537BA0.
	virtual HRESULT End(); //537BC0.
	virtual HRESULT Pass(UINT pass); //5370F0.

	ID3DXEffect* effect; //Borrowed selected effect, original+04; ctorNULL.
	DWORD colorOperation, alphaOperation;
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest;
};

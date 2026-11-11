#pragma once

#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

// Original70BC84..70BCA3. Normal SDK/shared state; no old object overlay.
struct Gfx_RS_B_LM_FOWState
{
	DWORD users;
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE unknown16[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_B_LM_FOWState g_RSBaseLightmapFogOfWarState;

class Gfx_RS_B_LM_FOW : public Gfx_RenderStyle
{
public:
	Gfx_RS_B_LM_FOW(DWORD color, DWORD alpha, DWORD lightmapColor,
		DWORD lightmapAlpha, DWORD alphaTest); //53E9D0, raw fifth DWORD.
	virtual ~Gfx_RS_B_LM_FOW(); //5372D0.
	virtual HRESULT Initialize(); //53EA00.
	virtual void Release(); //536230; unused incidental EAX unclaimed.
	virtual HRESULT OnLostDevice(); //536260.
	virtual HRESULT OnResetDevice(); //536280.
	virtual DWORD TechniqueCount(); //537340.
	virtual DWORD SelectTechnique(DWORD index); //5373C0.
	virtual DWORD NextTechnique(); //5373F0.
	virtual BYTE UsesVertexShader(); //537350.
	virtual DWORD PassCount(); //537360.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD unused); //53EC50.
	virtual HRESULT Begin(UINT* passes); //537370.
	virtual HRESULT End(); //537390.
	virtual HRESULT Pass(UINT pass); //5373A0.

	DWORD colorOperation, alphaOperation; //Original+04/+08.
	DWORD lightmapColorOperation, lightmapAlphaOperation, alphaTest; //+0C/+10/+14.
};

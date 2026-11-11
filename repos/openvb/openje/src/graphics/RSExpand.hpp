#pragma once

#include "RenderStyle.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_ExpandState
{
	DWORD users;
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE padding16[2];
	DWORD passes;
};
extern Gfx_RS_ExpandState g_RSExpandState; //70BCE8..70BD03.

class Gfx_RS_Expand : public Gfx_RenderStyle
{
public:
	Gfx_RS_Expand(); //540940; no fields beyond the actual style base.
	virtual ~Gfx_RS_Expand(); //5363C0.
	virtual HRESULT Initialize(); //540950.
	virtual void Release(); //540AF0; unused EAX source-return hypothesis.
	virtual HRESULT OnLostDevice(); //5364B0.
	virtual HRESULT OnResetDevice(); //5364D0.
	virtual DWORD TechniqueCount(); //536410.
	virtual DWORD SelectTechnique(DWORD index); //5364F0.
	virtual DWORD NextTechnique(); //536520.
	virtual BYTE UsesVertexShader(); //536420.
	virtual DWORD PassCount(); //536430.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD texture); //540B20; both unused.
	virtual HRESULT Begin(UINT* passes); //536440.
	virtual HRESULT End(); //536460.
	virtual HRESULT Pass(UINT pass); //536470; does not publish a current-pass field.
};

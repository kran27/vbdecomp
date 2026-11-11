#pragma once

#include "RenderStyle.hpp"
#include <d3dx8effect.h>

struct Gfx_RS_B_EMState
{
	DWORD users;
	ID3DXEffect* effect;
	D3DXEFFECT_DESC description;
	LPCSTR technique;
	BYTE usesShader, techniqueChanged;
	BYTE padding16[2];
	DWORD passes, currentPass;
};
extern Gfx_RS_B_EMState g_RSBaseEmissiveState; //70BBFC..70BC1B.

class Gfx_RS_B_EM : public Gfx_RenderStyle
{
public:
	Gfx_RS_B_EM(DWORD color, DWORD alpha, DWORD test); //53C600.
	virtual ~Gfx_RS_B_EM(); //536D70.
	virtual HRESULT Initialize(); //53C630.
	virtual void Release(); //535FF0; unused EAX source-return hypothesis.
	virtual HRESULT OnLostDevice(); //536020.
	virtual HRESULT OnResetDevice(); //536040.
	virtual DWORD TechniqueCount(); //536DE0.
	virtual DWORD SelectTechnique(DWORD index); //536E60.
	virtual DWORD NextTechnique(); //536E90.
	virtual BYTE UsesVertexShader(); //536DF0.
	virtual DWORD PassCount(); //536E00.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD texture); //53C800.
	virtual HRESULT Begin(UINT* passes); //536E10.
	virtual HRESULT End(); //536E30.
	virtual HRESULT Pass(UINT pass); //536E40.

	DWORD colorOperation, alphaOperation, alphaTest; //+04/+08/+0C.
};

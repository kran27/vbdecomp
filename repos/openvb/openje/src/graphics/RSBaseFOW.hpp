#pragma once
#include "RenderStyle.hpp"
#include "Material.hpp"
#include <d3dx8effect.h>

// Original shared70BC60..70BC83. One zero-initialized storage owner must be
// defined in RSBaseFOW.cpp; no legacy object overlay.
struct Gfx_RS_BaseFOWState
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
extern Gfx_RS_BaseFOWState g_RSBaseFOWState;

class Gfx_RS_Base_FOW : public Gfx_RenderStyle
{
public:
	Gfx_RS_Base_FOW(DWORD color, DWORD alpha, DWORD test); //53E0F0.
	virtual ~Gfx_RS_Base_FOW(); //537180; compiler deleting wrapper5372B0.
	virtual HRESULT Initialize(); //53E120.
	virtual void Release(); //536180; source void return hypothesis.
	virtual HRESULT OnLostDevice(); //5361D0.
	virtual HRESULT OnResetDevice(); //536200.
	virtual DWORD TechniqueCount(); //537210.
	virtual DWORD SelectTechnique(DWORD index); //537260.
	virtual DWORD NextTechnique(); //537290.
	virtual BYTE UsesVertexShader(); //537220.
	virtual DWORD PassCount(); //537230.
	virtual HRESULT SetMaterial(Gfx_Material* material, DWORD texture); //53E570.
	virtual HRESULT Begin(UINT* passes); //537BA0.
	virtual HRESULT End(); //537BC0.
	virtual HRESULT Pass(UINT pass); //537240.

	ID3DXEffect* effect; //+04 selected borrowed normal/factored effect; ctorNULL.
	DWORD colorOperation, alphaOperation, alphaTest; //+08/+0C/+10; test rawDWORD.
};


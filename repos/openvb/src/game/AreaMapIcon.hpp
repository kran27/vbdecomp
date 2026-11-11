#pragma once

#include "graphics/MeshData.hpp"
#include "graphics/PacketData.hpp"
#include "graphics/UIVertex.hpp"
#include <d3dx8.h>

namespace GUI { class Label; }
namespace JE { class AreaMapNote; }
class Gfx_Viewport;

//Optional host lifetime observation only. The original omits deleting icon labels.
struct AreaMapIconLabelObserver
{
	void (*observe)(::GUI::Label* label,void* context);
	void* context;
};
AreaMapIconLabelObserver SetAreaMapIconLabelObserver(AreaMapIconLabelObserver observer);

//Actual global RTTI AreaMapIcon; normal host members, not a legacy overlay.
class AreaMapIcon
{
public:
	AreaMapIcon(); //422A20.
	virtual ~AreaMapIcon(); //4202E0; deleting entry421690.
	void SetTexture(DWORD handle, BYTE owns); //41D5B0, releases old owned reference.
	DWORD GetTexture() const; //Shared5B4F30.
	void SetRect(LONG x,LONG y,LONG width,LONG height); //41E5F0.
	void GetRect(LONG& x,LONG& y,LONG& width,LONG& height) const; //41D160.
	void SetSourceRect(float u,float v,float width,float height); //41E640, RET16.
	void SetNumber(LONG number); //41F300, RET4; not an object/window setter.
	void SetIndex(DWORD value); //41D1E0.
	DWORD GetIndex() const; //41D1F0; actual hit-test callers consume full EAX.
	void SetNote(JE::AreaMapNote* value) { note=value; } //41D1C0, borrowed.
	JE::AreaMapNote* GetNote() const; //41D1D0.
	DWORD GetFlags() const; //41D1B0.
	void SetFlags(DWORD value); //41D190.
	void Refresh(); //41D0C0, only the numbered label's actual Refresh call.
	float SetDepth(float value,float increment); //41D0E0, returns the stored depth.
	void Draw(Gfx_Viewport* ignoredViewport); //4216B0; actual global GUI viewport used.
	void RebuildGeometry(); //4203C0.
	void RefreshLabelGeometry(); //41D5F0, no object or window parameter.

	DWORD texture; //+04.
	BYTE ownsTexture; //+08, raw BYTE.
	LONG x,y,width,height; //+0C..18.
	float u,v,sourceWidth,sourceHeight; //+1C..28.
	float depth; //+2C.
	DWORD flags; //+30, bit1/2 set white versus default gray.
	JE::AreaMapNote* note; //+34, borrowed.
	DWORD index; //+38.
	::GUI::Label* label; //+3C; allocated by ctor, ORIGINAL DTOR OMITS DELETE.
	LONG number; //+40.
	BYTE hidden,dirty; //+44/+45.
	Gfx_MeshData mesh; //+48.
	Gfx_MeshInstance* instances; //+B4, owning one-element array.
	Gfx_PacketData* packet; //+B8, owning.
	D3DXMATRIX transform; //+BC, constructor/rebuild identity.
	DWORD instanceCount; //+FC, ctor1.
	JE::Gfx::UIVertex* vertices; //+100, owning uninitialized POD array.
	LONG vertexCapacity,vertexCount; //+104/+108, ctor4/4.
	BYTE ready; //+10C, ctor0; Rebuild sets1.
private:
	AreaMapIcon(const AreaMapIcon&);
	AreaMapIcon& operator=(const AreaMapIcon&);
};

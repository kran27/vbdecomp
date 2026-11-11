#include "game/AreaMapIcon.hpp"
#include "gui/Label.hpp"
#include "gui/Manager.hpp"
#include "gui/PacketManager.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include "graphics/StringFormat.hpp"
#include <cstdio>

namespace { AreaMapIconLabelObserver labelObserver={NULL,NULL}; }
AreaMapIconLabelObserver SetAreaMapIconLabelObserver(AreaMapIconLabelObserver observer)
{
	const AreaMapIconLabelObserver previous=labelObserver;
	labelObserver=observer;
	return previous;
}

AreaMapIcon::AreaMapIcon() //422A20.
{
	texture=0; ownsTexture=0;
	x=y=width=height=0;
	u=v=0; depth=0; flags=0; note=NULL; number=0;
	hidden=dirty=0; index=0;
	sourceWidth=sourceHeight=1;
	D3DXMatrixIdentity(&transform);
	instances=NULL; vertices=NULL; vertexCapacity=0; vertexCount=0;
	packet=NULL; ready=0; instanceCount=1;
	instances=new Gfx_MeshInstance[1];
	vertexCapacity=4; vertexCount=4;
	vertices=new JE::Gfx::UIVertex[4]; //Original0x70 allocation remains unwritten.
	packet=new Gfx_PacketData;
	label=new ::GUI::Label(3); //Normal tracked GUI allocation, not a replacement.
	label->Resize(16,16);
	label->SetFont("Arial",0,10,1);
	label->SetTextColor(0xFFFFFFFF);
	label->SetText(std::string(""));
	label->SetClientPosition(0,0);
	label->SetValue3B(1);
}
AreaMapIcon::~AreaMapIcon() //4202E0; deleting entry421690.
{
	//Observation does not change the original omission. The explicit scene
	//adapter releases recorded labels after the actual icon/window lifetimes.
	if (vertices) delete[] vertices;
	if (instances) { delete[] instances; instances=NULL; }
	if (packet) { delete packet; packet=NULL; }
	if (texture && ownsTexture) g_GfxSystem->DeleteTexture(texture);
	//Actual original never deletes label+3C. Preserve that omission; an explicit
	//host cleanup receipt must account for leaked tracked labels separately.
	//Normal embedded mesh/material-set destruction supplies41F200/41FBD0.
}
void AreaMapIcon::SetTexture(DWORD handle,BYTE owns) //41D5B0.
{
	if (texture && ownsTexture) g_GfxSystem->DeleteTexture(texture);
	texture=handle; ownsTexture=owns; dirty=1;
}
void AreaMapIcon::GetRect(LONG& outputX,LONG& outputY,LONG& outputWidth,LONG& outputHeight) const
{
	outputX=x; outputY=y; outputWidth=width; outputHeight=height;
}
JE::AreaMapNote* AreaMapIcon::GetNote() const //41D1D0.
{
	return note;
}
void AreaMapIcon::SetRect(LONG selectedX,LONG selectedY,LONG selectedWidth,LONG selectedHeight)
{
	if (x!=selectedX || y!=selectedY || width!=selectedWidth || height!=selectedHeight)
	{
		height=selectedHeight; x=selectedX; y=selectedY; width=selectedWidth;
		RefreshLabelGeometry(); dirty=1;
	}
}
void AreaMapIcon::SetSourceRect(float selectedU,float selectedV,float selectedWidth,float selectedHeight)
{
	if (u!=selectedU || v!=selectedV || sourceWidth!=selectedWidth || sourceHeight!=selectedHeight)
	{
		u=selectedU; v=selectedV; sourceWidth=selectedWidth; sourceHeight=selectedHeight;
		RefreshLabelGeometry(); dirty=1;
	}
}
void AreaMapIcon::SetNumber(LONG value) //41F300, only one stack argument.
{
	number=value;
	if (value>0 && label)
	{
		std::string text;
		JE::FormatString(text,"%d",int(value));
		label->SetText(text);
	}
	RefreshLabelGeometry(); //Nonpositive numbers do not clear the old label text.
}
void AreaMapIcon::Refresh() //41D0C0.
{
	if (!hidden && number>0 && label) label->Refresh();
}
float AreaMapIcon::SetDepth(float value,float increment)
{
	depth=value;
	for (LONG i=0;i<vertexCount;++i) vertices[i].z=value;
	if (number>0 && label)
	{
		const float childStep=increment*0.3333333432674408f;
		const float childDepth=float(double(childStep)+value);
		label->SetDepth(childDepth,childStep);
	}
	return depth;
}
void AreaMapIcon::Draw(Gfx_Viewport*) //4216B0; argument unused in original.
{
	if (!hidden)
	{
		if (dirty) { RebuildGeometry(); dirty=0; }
		if (ready)
		{
			if (!::GUI::g_Manager.packets->optimizationLevel)
			{
				g_GfxSystem->QueueMeshInstances(instances,::GUI::g_Manager.view,instanceCount);
				return;
			}
			::GUI::g_Manager.packets->Submit(instances,::GUI::g_Manager.view,1);
		}
	}
	//The number Label is not drawn by this method. Do not invent that dispatch.
}

namespace
{
	LONG Truncate(double value) { return LONG(static_cast<__int64>(value)); }
}
void AreaMapIcon::RefreshLabelGeometry() //41D5F0; physical stack resolves false EDI input.
{
	if (!label) return;
	const LONG advance=label->GetTextAdvance(0); //4A89C0, measures actual text/font.
	LONG bottom=label->GetVisibleTextHeight(); //4A7760, +E34, NOT totalHeight+E2C.
	LONG textureWidth=0,textureHeight=0;
	if (texture)
	{
		Gfx_Texture* selected=g_GfxSystem->LookupTexture(texture);
		const LONG frameCount=selected->frameCount;
		if (frameCount<=0) textureWidth=0;
		else textureWidth=selected->frames[0].width;
		if (frameCount<=0) textureHeight=0;
		else textureHeight=selected->frames[0].height;
	}
	LONG left=textureWidth-advance, right=textureWidth, top=0;
	const float sourceX=textureWidth*u;
	if (left<sourceX) left=Truncate(sourceX);
	const float rightEdge=(sourceWidth+u)*textureWidth;
	if (rightEdge<textureWidth) right=Truncate(rightEdge);
	const double topEdge=double(textureHeight)*v;
	const float sourceY=float(topEdge);
	if (topEdge>0.0f) top=Truncate(sourceY);
	const double bottomEdge=double(textureHeight)*(double(sourceHeight)+v);
	if (bottomEdge<double(bottom)) bottom=Truncate(bottomEdge);
	const LONG clippedWidth=right-left;
	const float adjustedX=float(double(x)-sourceX), adjustedY=float(double(y)-sourceY);
	if (clippedWidth>0)
	{
		const LONG clippedHeight=bottom-top;
		if (clippedHeight>0)
		{
			label->SetHidden(0);
			label->Resize(clippedWidth,clippedHeight);
			label->SetAbsolutePosition(Truncate(float(left)+adjustedX),Truncate(float(top)+adjustedY));
			label->SetScroll(left-textureWidth+advance,top); //4AA130, then actual RebuildText.
			return;
		}
	}
	label->SetHidden(1);
}

DWORD AreaMapIcon::GetTexture() const { return texture; } //Shared5B4F30.
void AreaMapIcon::SetIndex(DWORD value) { index=value; } //41D1E0.

#include "game/AreaMapWindow.hpp"
#include "game/AreaMapIcon.hpp"
#include "game/AreaMapResources.hpp"
#include "gui/Manager.hpp"
#include "entity/MapFog.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "entity/Entity.hpp"
#include "graphics/StringFormat.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"
#include <algorithm>
#include <map>
#include <string>
#include <stdexcept>
#include <vector>

namespace
{
	LONG Truncate(double value)
	{ return static_cast<LONG>(static_cast<__int64>(value)); }
}
BYTE AreaMapWindow::ComputeNoteRectangle(JE::AreaMapNote* note,
	LONG& x,LONG& y,LONG& width,LONG& height) //41DBD0.
{
	if (!note || !areaMap) return 0;
	if (!note->texture) note->LoadTexture();
	LONG viewWidth,viewHeight; GetSize(viewWidth,viewHeight);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	viewWidth=Truncate(viewWidth*sx);
	viewHeight=Truncate(viewHeight*sy);
	float mx,my; WorldToMap(note->position[0],note->position[2],mx,my);
	x=Truncate(double(mx)/zoom);
	y=Truncate(double(my)/zoom);
	x-=LONG(note->value20)+originX; y-=LONG(note->value24)+originY;
	width=LONG(note->value28); height=LONG(note->value2C);
	if (x<0) { LONG candidate=LONG(DWORD(x)+DWORD(width)); width=candidate<0?0:candidate; x=0; }
	if (x+width>=viewWidth) { LONG candidate=viewWidth-x; width=candidate<0?0:candidate; }
	if (y<0) { LONG candidate=LONG(DWORD(y)+DWORD(height)); height=candidate<0?0:candidate; y=0; }
	if (y+height>=viewHeight) { LONG candidate=viewHeight-y; height=candidate<0?0:candidate; }
	if (width<=0 || height<=0) return 0;
	LONG px,py; ::GUI::Base::GetPosition(px,py);
	x=Truncate(double(px)*sx+x); y=Truncate(double(py)*sy+y);
	return 1;
}
BYTE AreaMapWindow::ComputeNoteSource(JE::AreaMapNote* note,
	float& u,float& v,float& width,float& height) //41DD90.
{
	if (!note || !areaMap) return 0;
	if (!note->texture) note->LoadTexture();
	if (!note->texture || !g_GfxSystem->LookupTexture(note->texture)) return 0;
	const LONG textureWidth = LONG(note->textureWidth);
	const LONG textureHeight = LONG(note->textureHeight);
	LONG viewWidth,viewHeight; GetSize(viewWidth,viewHeight);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	viewWidth=Truncate(double(viewWidth)*sx);
	viewHeight=Truncate(double(viewHeight)*sy);
	float mx,my; WorldToMap(note->position[0],note->position[2],mx,my);
	const float negativeZoom=-1.0f/zoom;
	LONG x=-(Truncate(double(mx)*negativeZoom)+originX+LONG(note->value20));
	LONG y=-(Truncate(double(my)*negativeZoom)+originY+LONG(note->value24));
	LONG pixelWidth=LONG(note->value28),pixelHeight=LONG(note->value2C);
	LONG clipX=0,clipY=0;
	if (x<0)
	{ const LONG old=pixelWidth; pixelWidth+=x; if (pixelWidth<0) pixelWidth=0; x=0; clipX=old-pixelWidth; }
	if (pixelWidth+x>=viewWidth) { pixelWidth=viewWidth-x; if (pixelWidth<0) pixelWidth=0; }
	if (y<0)
	{ const LONG old=pixelHeight; pixelHeight+=y; if (pixelHeight<0) pixelHeight=0; y=0; clipY=old-pixelHeight; }
	if (pixelHeight+y>=viewHeight) { pixelHeight=viewHeight-y; if (pixelHeight<0) pixelHeight=0; }
	if (pixelWidth<=0 || pixelHeight<=0) return 0;
	u=float(double(clipX)/textureWidth);
	v=float(double(clipY)/textureHeight);
	width=float(double(pixelWidth)/textureWidth);
	height=float(double(pixelHeight)/textureHeight);
	if (width<=0) return 0;
	return height>0;
}
BYTE AreaMapWindow::ComputePartyRectangle(float worldX,float worldZ,LONG iconWidth,LONG iconHeight,
	LONG& x,LONG& y,LONG& width,LONG& height,float& u,float& v,float& sourceWidth,float& sourceHeight)
{
	LONG viewWidth,viewHeight; GetSize(viewWidth,viewHeight);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	viewWidth=Truncate(viewWidth*sx);
	viewHeight=Truncate(viewHeight*sy);
	//The native call reuses the incoming coordinates for its two map outputs.
	WorldToMap(worldX,worldZ,worldZ,worldX);
	x=Truncate(double(worldZ)/zoom); y=Truncate(double(worldX)/zoom);
	//Original41E03F..062 reuses icon WIDTH/2 for both centering offsets.
	x+=iconWidth/-2-originX; y+=iconWidth/-2-originY;
	width=iconWidth; height=iconHeight;
	LONG clipX=0,clipY=0,lastX=iconWidth,lastY=iconHeight;
	if (x<0)
	{ const LONG old=width; LONG candidate=width+x; width=candidate<0?0:candidate; x=0; clipX=old-width; }
	if (x+width>=viewWidth)
	{ const LONG old=width; LONG candidate=viewWidth-x; width=candidate<0?0:candidate; lastX=iconWidth+width-old; }
	if (y<0)
	{ const LONG old=height; LONG candidate=height+y; height=candidate<0?0:candidate; y=0; clipY=old-height; }
	if (y+height>=viewHeight)
	{ const LONG old=height; LONG candidate=viewHeight-y; height=candidate<0?0:candidate; lastY=iconHeight+height-old; }
	if (width<=0 || height<=0) return 0;
	LONG px,py; ::GUI::Base::GetPosition(px,py);
	x=Truncate(double(px)*sx+x); y=Truncate(double(py)*sy+y);
	u=float(double(clipX)/iconWidth); v=float(double(clipY)/iconHeight);
	sourceWidth=float(double(lastX-clipX)/iconWidth);
	sourceHeight=float(double(lastY-clipY)/iconHeight);
	return 1;
}
void AreaMapWindow::UpdatePartyIcons() //421880.
{
	visiblePartyIcons.clear();
	if (!showParty || !areaMap) return;
	JE::IWorld* world=JE::g_World;
	JE::Party* party=world->GetParty();
	if (!party || LONG(party->members.size())<=0) return;
	BYTE found=0; ::GUI::MouseState mouse;
	::GUI::g_Manager.GetMouseState(mouse);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	mouse.x=Truncate(double(mouse.x)*sx);
	mouse.y=Truncate(double(mouse.y)*sy);
	overPartyIcon=0;
	std::size_t i=0;
	for (std::vector<JE::Entity*>::iterator memberPosition=party->members.begin();
		memberPosition!=party->members.end();++memberPosition,++i)
	{
		//Native indexes the six-entry vector without an overflow check.
		JE::Entity* member=*memberPosition;
		const float memberX=member->position.x,memberZ=member->position.z;
		AreaMapIcon* icon=partyIcons[i];
		icon->SetNumber(0);
		icon->SetIndex(member->entityID);
		LONG textureWidth=0,textureHeight=0;
		const DWORD textureHandle=icon->GetTexture();
		if (textureHandle)
		{
			Gfx_Texture* texture=g_GfxSystem->LookupTexture(textureHandle);
			if (texture)
			{
				const LONG frameCount=texture->frameCount;
				if (frameCount>0) textureWidth=LONG(texture->frames[0].width);
				if (frameCount>0) textureHeight=LONG(texture->frames[0].height);
			}
		}
		LONG x,y,width,height; float u,v,sourceWidth,sourceHeight;
		const BYTE visible=ComputePartyRectangle(memberX,memberZ,
			textureWidth,textureHeight,x,y,width,height,u,v,sourceWidth,sourceHeight);
		if (visible)
		{
			icon->SetRect(x,y,width,height);
			icon->SetSourceRect(u,v,sourceWidth,sourceHeight);
			visiblePartyIcons.push_back(icon);
		}
		//The original tests the computed rectangle even when clipping returned0.
		//Those out parameters are written before the positive-size gate.
		if (!found && partyHoverEnabled && mouse.x>=x && mouse.x<x+width && mouse.y>=y && mouse.y<y+height)
		{ icon->SetFlags(icon->GetFlags()|1); found=1; overPartyIcon=1; }
		else icon->SetFlags(icon->GetFlags()&~DWORD(1));
	}
}
void AreaMapWindow::RebuildVisibleSamples() //4235A0.
{
	ClearNoteIcons();
	if (!areaMap) return;
	const LONG noteCount=LONG(areaMap->notes.size());
	if (noteCount<=0) return;
	BYTE found=0; ::GUI::MouseState mouse;
	::GUI::g_Manager.GetMouseState(mouse);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	const LONG mouseX=Truncate(double(mouse.x)*sx),mouseY=Truncate(double(mouse.y)*sy);
	std::map<std::string,LONG> counts;
	std::list<JE::AreaMapNote*> selected;
	std::map<JE::AreaMapNote*,std::vector<LONG> > values;
	for (LONG i=0;i<noteCount;++i)
	{
		JE::AreaMap* map=areaMap;
		JE::AreaMapNote* note=(i<0 || i>=LONG(map->notes.size())) ? NULL : &map->notes[i];
		if (note->flag5D) continue;
		std::string name(note->name);
		JE::UpperString(name);
		if (counts.find(name)==counts.end()) counts[name]=1;
		else ++counts[name];
		LONG x,y,width,height;
		if (ComputeNoteRectangle(note,x,y,width,height))
		{
			selected.push_back(note);
			std::vector<LONG> record;
			record.resize(6,0);
			record[0]=x;
			record[1]=y;
			record[2]=width;
			record[3]=height;
			record[4]=i;
			record[5]=counts[name];
			values[note]=record;
		}
	}
	if (!selected.empty())
	{
		//Original also constructs/destroys a second, unused string-count map.
		std::map<std::string,LONG> unusedCounts;
		for (std::list<JE::AreaMapNote*>::reverse_iterator i=selected.rbegin();i!=selected.rend();++i)
		{
			JE::AreaMapNote* note=*i;
			const std::vector<LONG>& record=values[note];
			const LONG x=record[0], y=record[1];
			const LONG width=record[2], height=record[3];
			const LONG index=record[4], number=record[5];
			AreaMapIcon* icon=new AreaMapIcon;
			if (!note->texture) note->LoadTexture();
			icon->SetTexture(note->texture,0);
			icon->SetRect(x,y,width,height);
			icon->SetNote(note);
			icon->SetIndex(DWORD(index));
			std::string name(note->name);
			JE::UpperString(name);
			icon->SetNumber(counts[name]>1 ? number : 0);
			float u,v,sourceWidth,sourceHeight;
			if (ComputeNoteSource(note,u,v,sourceWidth,sourceHeight))
				icon->SetSourceRect(u,v,sourceWidth,sourceHeight);
			if (!found && noteHoverEnabled && !overPartyIcon
				&& mouseX>=x && mouseX<x+width && mouseY>=y && mouseY<y+height)
			{ icon->SetFlags(icon->GetFlags()|1); found=1; }
			else icon->SetFlags(icon->GetFlags()&~DWORD(1));
			noteIcons.push_back(icon);
		}
		depthDirty=1;
	}
}


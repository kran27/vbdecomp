#include "game/ActionbarInterface.hpp"
#include "startup/GameClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "game/AreaMapWindow.hpp"
#include "game/AreaMapIcon.hpp"
#include "game/AreaMapResources.hpp"
#include "gui/LineSystem.hpp"
#include "gui/Manager.hpp"
#include "gui/Event.hpp"
#include "entity/MapFog.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "entity/Entity.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/StringFormat.hpp"
#include "graphics/TextureResource.hpp"
#include "graphics/Texture.hpp"
#include <algorithm>

namespace
{
	AreaMapWindowServices* externalServices = NULL;
	LONG Truncate(double value)
	{ return static_cast<LONG>(static_cast<__int64>(value)); }
}
AreaMapWindowServices* SetAreaMapWindowServices(AreaMapWindowServices* value)
{
	AreaMapWindowServices* previous=externalServices;
	externalServices=value; return previous;
}
//Explicit scene providers remain separate from the original native owner lookup.
BYTE GetAreaMapVisibleActionbarTop(LONG& top)
{
	if (externalServices)
		return externalServices->GetVisibleActionbarTop(top);
	F3::GameClient* client = dynamic_cast<F3::GameClient*>(
		static_cast<JE::Client*>(F3::g_pNetworkInstance->client));
	F3::ActionbarInterface* actionbar = client->Interfaces()->actionbar;
	if (!actionbar || actionbar->GetHidden()) return 0;
	LONG x;
	actionbar->rootWindow->GetAbsolutePosition(x, top);
	return 1;
}
AreaMapWindowBatch::AreaMapWindowBatch() : instances(NULL),packet(NULL),
	instanceCount(0),vertices(NULL),vertexCount(0),vertexCapacity(0),
	indices(NULL),indexCount(0),indexCapacity(0),ready(0)
{ D3DXMatrixIdentity(&transform); }
AreaMapWindowBatch::~AreaMapWindowBatch() { Clear(); }
void AreaMapWindowBatch::Clear()
{
	delete[] vertices; vertices=NULL;
	delete[] indices; indices=NULL;
	delete[] instances; instances=NULL;
	delete packet; packet=NULL;
	//Rebuild4231B0 replaces only these raw/instance/packet owners. The embedded
	//mesh survives until its own subsequent builder/destructor, as original.
}
AreaMapWindow::AreaMapWindow(JE::AreaMap* value) : ::GUI::Picture(2)
{
	//422EB4 onward, after natural member construction. Depth/corners stay unwritten.
	scanlineHeight=2;
	scanlineWidth=2;
	areaMap=value;
	scanlineVScale=1;
	scanlineUScale=1;
	maskTexture=0;
	noteHoverEnabled=0;
	partyHoverEnabled=0;
	originX=0;
	originY=0;
	dirty=1;
	partyIcons.resize(6,NULL);
	LONG iconIndex=0;
	do
	{
		AreaMapIcon* icon=new AreaMapIcon;
		const LONG slot=iconIndex;
		partyIcons[iconIndex]=icon;
		std::string name;
		++iconIndex;
		JE::FormatString(name,"AreaMapPlayer%02d.tga",int(iconIndex)); //423096.
		const DWORD texture=Gfx_RequestTextureResource(name.c_str(),1,1,5,0);
		if (texture) partyIcons[slot]->SetTexture(texture,1); //4230C3 reloads after callback.
		//The owned name dies after optional texture publication, before the loop test.
	}
	while (iconIndex<6);
	cameraOutline=new ::GUI::LineSystem;
	cameraOutline->scaleToResolution=0;
	depthDirty=0;
	zoom=1;
	scanlineTexture=Gfx_RequestTextureResource("f3_scanlines.tga",1,1,5,0);
	//423167 publishes these defaults only after the scanline request returns.
	overPartyIcon=0;
	flagF8D=0;
	flagF8E=1;
	showParty=1;
	showCamera=1;
}
AreaMapWindow::~AreaMapWindow()
{
	background.Clear(); scanlines.Clear();
	ClearNoteIcons();
	for (std::size_t i=0;i<partyIcons.size();++i) delete partyIcons[i];
	delete cameraOutline;
	if (scanlineTexture) g_GfxSystem->DeleteTexture(scanlineTexture);
	visiblePartyIcons.clear();
	//AreaMapIcon intentionally omits Label deletion; no repaired owner here.
}
void AreaMapWindow::SetAreaMap(JE::AreaMap* value)
{
	areaMap=value; originX=0; originY=0; dirty=1; depthDirty=1; overPartyIcon=0;
}
void AreaMapWindow::SetOrigin(LONG x,LONG y)
{
	if (originX!=x || originY!=y) { originX=x; originY=y; dirty=1; }
}
BYTE AreaMapWindow::WorldToMap(float x,float z,float& mapX,float& mapY)
{
	if (!areaMap) return 0;
	const double minX=areaMap->minX, minZ=areaMap->minZ;
	const double spanZ=double(areaMap->maxZ)-minZ;
	const LONG height=pictureHeight;
	mapX=float((double(x)-minX)/(double(areaMap->maxX)-minX)*pictureWidth);
	mapY=float(double(height)-(double(z)-minZ)/spanZ*height);
	return 1;
}
BYTE AreaMapWindow::WorldToIntegerMap(float x,float z,LONG& mapX,LONG& mapY)
{
	if (!areaMap) return 0;
	const double minX=areaMap->minX;
	const float minZ=areaMap->minZ;
	const float spanZ=areaMap->maxZ-minZ;
	const LONG height=pictureHeight;
	mapX=Truncate((double(x)-minX)/(double(areaMap->maxX)-minX)*pictureWidth);
	mapY=height-Truncate((double(z)-minZ)/spanZ*height);
	return 1;
}
BYTE AreaMapWindow::GetExploredBounds(RECT& output,float /*unusedZoom*/)
{
	JE::AreaMap* map=areaMap;
	if (!map) return 0;
	const LONG columns=map->width,rows=map->height;
	if (columns<=0 || rows<=0) return 0;
	const LONG viewWidth=pictureWidth,viewHeight=pictureHeight;
	if (viewWidth<=0 || viewHeight<=0) return 0;
	output.left=viewWidth;
	output.top=viewHeight;
	output.right=0;
	output.bottom=0;
	BYTE found=0;
	for (LONG row=0;row<rows;++row)
		for (LONG first=0;first<columns;++first)
			if (map->IsExplored(first,row))
			{
				float x,z; LONG mx,my;
				map->CellToWorld(first,row,&x,&z,0);
				WorldToIntegerMap(x,z,mx,my);
				const LONG left=output.left;
				output.left=left<mx?left:mx;
				const LONG top=output.top;
				output.top=top<my?top:my;
				map->CellToWorld(first+1,row+1,&x,&z,0);
				WorldToIntegerMap(x,z,mx,my);
				const LONG right=output.right;
				output.right=right>mx?right:mx;
				const LONG bottom=output.bottom;
				output.bottom=bottom>my?bottom:my;
				found=1;
			}
	//Original does not swap the Y edges or use the second float argument.
	return found;
}
void AreaMapWindow::ClearNoteIcons()
{
	for (std::list<AreaMapIcon*>::iterator i=noteIcons.begin();i!=noteIcons.end();++i) delete *i;
	noteIcons.clear();
}
void AreaMapWindow::RebuildGeometry()
{
	if (background.vertices) { delete[] background.vertices; background.vertices=NULL; }
	if (background.indices) { delete[] background.indices; background.indices=NULL; }
	if (background.instances) { delete[] background.instances; background.instances=NULL; }
	if (background.packet) { delete background.packet; background.packet=NULL; }
	if (scanlines.vertices) { delete[] scanlines.vertices; scanlines.vertices=NULL; }
	if (scanlines.indices) { delete[] scanlines.indices; scanlines.indices=NULL; }
	if (scanlines.instances) { delete[] scanlines.instances; scanlines.instances=NULL; }
	if (scanlines.packet) { delete scanlines.packet; scanlines.packet=NULL; }
	ClearNoteIcons();

	if (!areaMap) return;
	areaMap->GetCellSpacing(); //4232CE discards the returned ST0.
	LONG width,height;
	GetSize(width,height);

	background.instanceCount=1;
	background.instances=new Gfx_MeshInstance[1];
	background.vertexCount=0;
	background.vertexCapacity=4;
	background.vertices=new AreaMapVertex[4];
	background.indexCount=0;
	background.indexCapacity=6;
	background.indices=new WORD[6];
	background.packet=new Gfx_PacketData;
	for (LONG i=0;i<LONG(background.vertexCapacity);++i)
	{
		background.vertices[i].color=0xFFFFFFFF;
		background.vertices[i].rhw=1;
		background.vertices[i].z=0;
	}

	scanlines.instanceCount=1;
	scanlines.instances=new Gfx_MeshInstance[1];
	scanlines.vertexCount=0;
	scanlines.vertexCapacity=4;
	scanlines.vertices=new AreaMapVertex[4];
	scanlines.indexCount=0;
	scanlines.indexCapacity=6;
	scanlines.indices=new WORD[6];
	scanlines.packet=new Gfx_PacketData;
	for (LONG i=0;i<LONG(scanlines.vertexCapacity);++i)
	{
		scanlines.vertices[i].color=0xFFFFFFFF;
		scanlines.vertices[i].rhw=1;
		scanlines.vertices[i].z=0;
		scanlines.vertices[i].color=0xFFFFFFFF;
	}

	scanlines.indices[0]=3;
	scanlines.indices[1]=2;
	scanlines.indices[2]=0;
	scanlines.indices[3]=3;
	scanlines.indices[4]=0;
	scanlines.indices[5]=1;

	if (scanlineTexture)
	{
		Gfx_Texture* texture=g_GfxSystem->LookupTexture(scanlineTexture);
		const LONG frames=texture->frameCount;
		LONG textureWidth;
		if (frames<=0) textureWidth=0;
		else textureWidth=texture->frames[0].width;
		const LONG textureHeight=frames>0 ? LONG(texture->frames[0].height) : 0;
		scanlineWidth=textureWidth;
		scanlineHeight=textureHeight;
	}
}
void AreaMapWindow::UpdateGeometry()
{ ::GUI::Window::UpdateGeometry(); dirty=1; }
float AreaMapWindow::GetDepth()
{ return LONG(background.vertexCount)>0 && background.vertices ? background.vertices[0].z : 0; }
float AreaMapWindow::SetDepth(float depth,float increment)
{
	const float current=::GUI::Window::SetDepth(depth,increment);
	mapDepth=current; mapDepthIncrement=increment;
	return AssignContentDepth(current,increment);
}
float AreaMapWindow::AssignContentDepth(float depth,float increment)
{
	const LONG count=LONG(noteIcons.size())+LONG(partyIcons.size())+4;
	const float step=increment/count;
	float next=step+depth;
	if (showCamera)
	{
		cameraOutline->SetDepth(next,step);
		next+=step;
	}
	for (std::list<AreaMapIcon*>::iterator i=noteIcons.begin();i!=noteIcons.end();)
	{
		next=(*i)->SetDepth(next,step);
		++i;
		next+=step;
	}
	if (showParty)
		for (std::vector<AreaMapIcon*>::iterator i=partyIcons.begin();i!=partyIcons.end();)
		{
			next=(*i)->SetDepth(next,step);
			++i;
			next+=step;
		}
	if (flagF8E)
	{
		for (LONG i=0;i<LONG(scanlines.vertexCount);++i) scanlines.vertices[i].z=next;
		next+=step;
	}
	for (LONG i=0;i<LONG(background.vertexCount);++i) background.vertices[i].z=next;
	return depth;
}
void AreaMapWindow::Refresh()
{
	::GUI::Window::Refresh();
	UpdatePartyIcons();
	if (dirty || (areaMap && (areaMap->changes&3))) RebuildVisibleSamples();
	if (areaMap && (dirty || (areaMap->changes&1))) BuildImageGeometry();
	UpdateCameraFootprint();
	for (std::list<AreaMapIcon*>::iterator i=noteIcons.begin();i!=noteIcons.end();++i) (*i)->Refresh();
	if (showParty)
		for (std::list<AreaMapIcon*>::iterator i=visiblePartyIcons.begin();i!=visiblePartyIcons.end();++i)
			(*i)->Refresh();
	if (showCamera && cameraOutline) cameraOutline->Update();
	dirty=0;
}
void AreaMapWindow::Draw()
{
	if (!hidden && areaMap && fragments[4].GetTextureHandle())
	{
		if (depthDirty) { AssignContentDepth(mapDepth,mapDepthIncrement); depthDirty=0; }
		//This original override does not call Picture/Window::Draw.
		if (background.ready) g_GfxSystem->QueueMeshInstances(background.instances,::GUI::g_Manager.view,background.instanceCount);
		if (scanlines.ready) g_GfxSystem->QueueMeshInstances(scanlines.instances,::GUI::g_Manager.view,scanlines.instanceCount);
		for (std::list<AreaMapIcon*>::reverse_iterator i=noteIcons.rbegin();i!=noteIcons.rend();++i) (*i)->Draw(::GUI::g_Manager.view);
		if (showParty)
			for (std::list<AreaMapIcon*>::iterator i=visiblePartyIcons.begin();i!=visiblePartyIcons.end();++i)
				(*i)->Draw(::GUI::g_Manager.view);
		if (showCamera && cameraOutline) cameraOutline->Draw(::GUI::g_Manager.view);
	}
}
BOOL AreaMapWindow::OnEvent(::GUI::Event* event)
{
	if (event->type==2)
	{
		JE::Client* client = static_cast<JE::Client*>(F3::g_pNetworkInstance->client);
		(void)dynamic_cast<F3::GameClient*>(client);
		if (partyHoverEnabled) UpdatePartyHover(event->x,event->y);
		if (noteHoverEnabled) UpdateNoteHover(event->x,event->y);
	}
	return ::GUI::Window::OnEvent(event);
}
void AreaMapWindow::SetNoteHoverEnabled(BYTE value)
{
	noteHoverEnabled=value!=0;
	::GUI::MouseState mouse; ::GUI::g_Manager.GetMouseState(mouse);
	UpdatePartyHover(mouse.x,mouse.y); UpdateNoteHover(mouse.x,mouse.y);
}
void AreaMapWindow::SetPartyHoverEnabled(BYTE value)
{
	partyHoverEnabled=value!=0;
	::GUI::MouseState mouse; ::GUI::g_Manager.GetMouseState(mouse);
	UpdatePartyHover(mouse.x,mouse.y); UpdateNoteHover(mouse.x,mouse.y);
}


#include "startup/GameClient.hpp"
#include "game/MiniMapInterface.hpp"
#include "game/MiniMapServices.hpp"
#include "game/ActionbarInterface.hpp"
#include "game/AreaMapWindow.hpp"
#include "gui/Event.hpp"
#include "gui/Manager.hpp"
#include "graphics/MapMaskManager.hpp"
#include "graphics/Graphics.hpp"
#include "graphics/G3DModel.hpp"
#include "graphics/G3DTextureImage.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/TextureResource.hpp"
#include "entity/Entity.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "network/EngineClient.hpp"
#include "startup/NetworkComponent.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>

namespace
{
	const float miniMapZoom[5]={0.25f,0.5f,1.0f,2.0f,4.0f}; //658B34.
	LONG Truncate(double value)
	{
		//The original calls real CRT __ftol2, retaining the low EAX word.
		return static_cast<LONG>(static_cast<__int64>(value));
	}
}

namespace F3
{
	MiniMapInterface::MiniMapInterface(const std::string& name) : JE::EngineInterface(name), GameInterface(name),
		mapWindow(NULL),zoomIndex(2),cachedMapID(0),flag2C(0),flag2D(0) //442340.
	{
		LoadResource("MiniMap.INT");
	}
	MiniMapInterface::~MiniMapInterface() //442290.
	{
		//Original destroys only cachedOctreeName then inherited bases here.
		//The actual mapWindow remains a child of the authored winMiniMap graph.
	}
	BYTE MiniMapInterface::RefreshExploredMask() //441D20.
	{
		MapMaskManager* masks=static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->maskManager;
		JE::IWorldClient* world=JE::g_World;
		const BYTE refreshed=masks->UpdateExploredMask(world->GetCurrentMapID());
		AreaMapWindow* window=mapWindow;
		if (refreshed) window->maskTexture=masks->exploredTexture;
		else window->maskTexture=0;
		window->dirty=1;
		return 0;
	}
	void MiniMapInterface::CenterOnSelection() //441D90.
	{
		JE::Entity* selected=JE::GetEntityPtr(static_cast<JE::Client*>(g_pNetworkInstance->client)->field0xE0);
		float left,top,right,bottom;
		float zoom=miniMapZoom[zoomIndex];
		if (selected)
		{
			//The original ignores the conversion result; outputs retain their
			//unwritten domain if the real AreaMap child rejects this input.
			D3DXVECTOR3 position;
			position.x=selected->position.x;
			position.y=selected->position.y;
			position.z=selected->position.z;
			mapWindow->WorldToMap(position.x,position.z,left,top);
			right=left;
			bottom=top;
		}
		else
		{
			RECT bounds;
			mapWindow->GetExploredBounds(bounds,zoom); //Ignored BYTE result, originalRET8.
			left=float(bounds.left); top=float(bounds.top);
			right=float(bounds.right); bottom=float(bounds.bottom);
		}
		zoom=1.0f/zoom;
		left=zoom*left;
		top=zoom*top;
		LONG width,height;
		mapWindow->GetSize(width,height);
		float scaleX,scaleY;
		::GUI::g_Manager.GetScale(scaleX,scaleY);
		width=Truncate(double(width)*scaleX);
		height=Truncate(double(height)*scaleY);
		const LONG y=Truncate((zoom*bottom-top-height)*0.5f+top);
		const LONG x=Truncate((zoom*right-left-width)*0.5f+left);
		mapWindow->SetOrigin(x,y);
	}
	void MiniMapInterface::SetZoom(LONG index,BYTE anchored,LONG anchorX,LONG anchorY) //441EE0.
	{
		if (index<0 || index>=5 || index==zoomIndex) return;
		const float previousZoom=miniMapZoom[zoomIndex],newZoom=miniMapZoom[index];
		zoomIndex=index;
		AreaMapWindow* window=mapWindow;
		const LONG originY=window->originY;
		const LONG originX=window->originX;
		LONG width,height;
		window->GetSize(width,height);
		float scaleX,scaleY;
		::GUI::g_Manager.GetScale(scaleX,scaleY);
		width=Truncate(width*scaleX);
		height=Truncate(height*scaleY);
		window=mapWindow;
		window->zoom=newZoom;
		window->dirty=1;
		if (!anchored)
		{
			anchorX=Truncate((width*0.5f+originX)*previousZoom);
			anchorY=Truncate((height*0.5f+originY)*previousZoom);
		}
		const float inverse=1.0f/newZoom;
		window=mapWindow;
		const float ratio=previousZoom*inverse;
		const LONG deltaY=anchorY-Truncate(anchorY/ratio);
		const LONG y=Truncate(deltaY*inverse+window->originY);
		const LONG deltaX=anchorX-Truncate(anchorX/ratio);
		const LONG x=Truncate(deltaX*inverse+window->originX);
		window->SetOrigin(x,y);
		::GUI::MouseState mouse;
		::GUI::g_Manager.GetMouseState(mouse);
		if (mapWindow)
		{
			mapWindow->RebuildVisibleSamples();
			mapWindow->UpdatePartyHover(mouse.x,mouse.y);
			mapWindow->UpdateNoteHover(mouse.x,mouse.y);
		}
		CenterOnSelection(); //Original always recenters after hover updates.
	}
	int MiniMapInterface::OnEvent(::GUI::Window* window,::GUI::Event* event) //4420A0.
	{
		if (event->type==6)
		{
			if (!_stricmp(window->GetName(),"AreaMapWindow"))
			{
				GameClient* client = dynamic_cast<GameClient*>(static_cast<JE::Client*>(g_pNetworkInstance->client));
				GameInterfaces* interfaces = static_cast<GameInterfaces*>(client->child0xA0);
				interfaces->actionbar->SetCompanionMode(1);
			}
		}
		else if (event->type==16 && window==mapWindow && mapWindow->Contains(event->x,event->y))
		{
			float scaleX,scaleY;
			::GUI::g_Manager.GetScale(scaleX,scaleY);
			const LONG originX=mapWindow->originX,originY=mapWindow->originY;
			LONG x,y;
			mapWindow->GetAbsolutePosition(x,y);
			const LONG current=zoomIndex;
			const float zoom=miniMapZoom[current];
			const LONG anchorX=Truncate((double(event->x-x)*scaleX+originX)*zoom);
			const LONG anchorY=Truncate((double(event->y-y)*scaleY+originY)*zoom);
			const LONG step=event->wheel>0?-1:(event->wheel<0?1:0);
			SetZoom(current+step,1,anchorX,anchorY);
		}
		return JE::EngineInterface::OnEvent(window,event);
	}
	void MiniMapInterface::RefreshBackground() //442450.
	{
		std::string name;
		JE::IWorldClient* clientWorld=JE::g_World;
		const DWORD mapID=clientWorld->GetCurrentMapID();
		JE::IWorld* world=JE::g_World;
		JE::Map* map=world->GetMap(mapID);
		DWORD texture=0;
		if (map)
		{
			name=map->areaTextureName.c_str();
			texture=Gfx_RequestTextureResource(name.c_str(),1,1,5,0); //Exact name,1,1,5,0 callback arguments.
		}
		LONG width=0,height=0;
		if (texture)
		{
			JE::Resources::ResourceData* resource=JE::Resources::g_pResourceSystem->Demand(name.c_str(),1);
			if (resource)
			{
				G3D_Texture image;
				image.SetWidth(-1); //Actual4CA270 preserves pixel-owner cleanup.
				const BYTE loaded=image.LoadImage(static_cast<const BYTE*>(resource->data),0);
				if (loaded || LONG(image.width)>0)
				{
					width=LONG(image.width);
					height=LONG(image.height);
				}
				JE::Resources::g_pResourceSystem->ReleaseResource(resource);
			}
			mapWindow->SetTexture(texture,0,width,height,0,0,-1,-1,1,1);
			mapWindow->SetTileMode(4);
		}
		else mapWindow->LoadTexture("AreaMap-middle.bmp",0,0,0,-1,-1,1);
		AreaMapWindow* window=mapWindow;
		window->flagF8D=0; window->dirty=1;
		window=mapWindow;
		window->flagF8E=1; window->dirty=1;
	}
	void MiniMapInterface::CreateMapWindow(JE::Map* map) //442710, actualRET4.
	{
		const DWORD maskTexture=static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->maskManager->exploredTexture;
		const float zoom=miniMapZoom[zoomIndex];
		JE::AreaMap* areaMap=map?map->areaMap:NULL;
		if (mapWindow)
		{
			RemoveWindow(mapWindow);
			delete mapWindow;
		}
		mapWindow=new ::AreaMapWindow(areaMap);
		mapWindow->ApplySkin("AreaMap.ini");
		mapWindow->maskTexture=maskTexture;
		mapWindow->dirty=1;
		::GUI::Window* placeholder=FindWindow("winMiniMap");
		if (placeholder)
		{
			LONG x,y,width,height;
			placeholder->GetAbsolutePosition(x,y);
			placeholder->GetSize(width,height);
			mapWindow->SetAbsolutePosition(x,y);
			mapWindow->Resize(width,height);
			placeholder->SetColor(0xFF070F00);
		}
		mapWindow->SetName("AreaMapWindow"); //Exact raw bytes658B48,13 chars.
		placeholder->AddChild(mapWindow); //Original dereferences it after the guarded layout.
		mapWindow->RebuildGeometry();
		mapWindow->zoom=zoom;
		mapWindow->dirty=1;
		RefreshBackground();
		RefreshExploredMask();
		CenterOnSelection();
		mapWindow->SetNoteHoverEnabled(0); //41F480, actual+F7C.
		mapWindow->SetPartyHoverEnabled(0); //41F4D0, actual+F8C.
	}
	void MiniMapInterface::Refresh() //442B30.
	{
		JE::IWorldClient* clientWorld=JE::g_World;
		const DWORD id=clientWorld->GetCurrentMapID();
		JE::IWorld* world=JE::g_World;
		JE::Map* map=world->GetMap(id);
		CreateMapWindow(map);
		cachedMapID=id;
		cachedOctreeName=map?map->octreeName.c_str():"";
	}
	void MiniMapInterface::BeforeUpdate() //4429D0; no extra hidden/state gate.
	{
		JE::IWorldClient* clientWorld=JE::g_World;
		const DWORD id=clientWorld->GetCurrentMapID();
		JE::IWorld* world=JE::g_World;
		JE::Map* map=world->GetMap(id);
		JE::AreaMap* areaMap;
		if (map && (areaMap=map->areaMap)!=NULL)
		{
			const char* octreeName;
			if (cachedMapID!=id ||
				(octreeName=map->octreeName.c_str(),
				 cachedOctreeName.compare(0,cachedOctreeName.size(),octreeName,
					std::char_traits<char>::length(octreeName))!=0))
			{
				mapWindow->SetAreaMap(areaMap);
				const DWORD maskTexture=static_cast<JE::Client*>(g_pNetworkInstance->client)->child0x74->maskManager->exploredTexture;
				::AreaMapWindow* window=mapWindow;
				window->maskTexture=maskTexture;
				window->dirty=1;
				RefreshBackground();
				RefreshExploredMask();
			}
			cachedMapID=id;
			cachedOctreeName=map->octreeName.c_str();
			CenterOnSelection();
		}
		else
		{
			RefreshExploredMask();
			cachedMapID=0;
			cachedOctreeName="";
		}
	}
}


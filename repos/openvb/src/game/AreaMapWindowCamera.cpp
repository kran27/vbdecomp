#include "game/AreaMapWindow.hpp"
#include "gui/LineSystem.hpp"
#include "gui/Manager.hpp"
#include "entity/EntityEnvironment.hpp"
#include "entity/Map.hpp"
#include "entity/World.hpp"
#include "graphics/OctreeCamera.hpp"
#include "graphics/SystemInterface.hpp"

extern BYTE GetAreaMapVisibleActionbarTop(LONG& top);
namespace
{
	LONG Truncate(double value)
	{ return static_cast<LONG>(static_cast<__int64>(value)); }
}
void AreaMapWindow::UpdateCameraFootprint() //41E9B0.
{
	if (!showCamera) return;
	DWORD screenWidth,screenHeight; ::GUI::g_Manager.GetResolution(screenWidth,screenHeight);
	LONG top;
	if (GetAreaMapVisibleActionbarTop(top))
	{
		float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
		screenHeight=DWORD(Truncate(top*sy));
	}
	OCTREE* octree=JE::g_World->GetCurrentMapOctree(); //Actual IWorldClient+0C dispatch.
	if (!octree) return;
	OCTREE_Camera* camera=JE::GetEntityEnvironment().CurrentMapCamera();
	if (!camera) return;
	OctreeRay rays[4]; //Actual41D030 gives direction0,maxFLT_MAX,bounds callback.
	LONG status[4];
	{
		camera->MakeScreenRay(0,0,&rays[0]);
		OctreeGroundHit hit;
		if (octree->QueryGroundRay(rays[0],&hit,1,0))
		{ cameraCorners[0]=D3DXVECTOR3(hit.point); status[0]=0; }
		else if (octree->QueryGroundRay(rays[0],&hit,0,0))
		{ cameraCorners[0]=D3DXVECTOR3(hit.point); status[0]=1; }
		else status[0]=2;
	}
	{
		camera->MakeScreenRay(LONG(screenWidth)-1,0,&rays[1]);
		OctreeGroundHit hit;
		if (octree->QueryGroundRay(rays[1],&hit,1,0))
		{ cameraCorners[1]=D3DXVECTOR3(hit.point); status[1]=0; }
		else if (octree->QueryGroundRay(rays[1],&hit,0,0))
		{ cameraCorners[1]=D3DXVECTOR3(hit.point); status[1]=1; }
		else status[1]=2;
	}
	{
		camera->MakeScreenRay(LONG(screenWidth)-1,LONG(screenHeight)-1,&rays[2]);
		OctreeGroundHit hit;
		if (octree->QueryGroundRay(rays[2],&hit,1,0))
		{ cameraCorners[2]=D3DXVECTOR3(hit.point); status[2]=0; }
		else if (octree->QueryGroundRay(rays[2],&hit,0,0))
		{ cameraCorners[2]=D3DXVECTOR3(hit.point); status[2]=1; }
		else status[2]=2;
	}
	{
		camera->MakeScreenRay(0,LONG(screenHeight)-1,&rays[3]);
		OctreeGroundHit hit;
		if (octree->QueryGroundRay(rays[3],&hit,1,0))
		{ cameraCorners[3]=D3DXVECTOR3(hit.point); status[3]=0; }
		else if (octree->QueryGroundRay(rays[3],&hit,0,0))
		{ cameraCorners[3]=D3DXVECTOR3(hit.point); status[3]=1; }
		else status[3]=2;
	}
	double sum=0; LONG count=0;
	if (!status[0]) { sum=cameraCorners[0].y; ++count; }
	if (!status[1]) { sum+=cameraCorners[1].y; ++count; }
	if (!status[2]) { sum+=cameraCorners[2].y; ++count; }
	if (!status[3]) { sum+=cameraCorners[3].y; ++count; }

	double height;
	if (count>0) height=sum/count;
	else
	{
		float bounds[6]; octree->GetLevelBounds(bounds);
		height=(bounds[4]-bounds[1])*0.5f+bounds[1];
	}
	for (int i=0;i<4;++i)
		if (status[i]>0 && status[i]<=2)
		{
			const double distance=(height-rays[i].origin[1])/rays[i].direction.value[1];
			cameraCorners[i].x=float(distance*rays[i].direction.value[0]+rays[i].origin[0]);
			cameraCorners[i].y=float(height);
			cameraCorners[i].z=float(distance*rays[i].direction.value[2]+rays[i].origin[2]);
		}
	bool changed=dirty!=0;
	for (int i=0;i<4 && !changed;++i)
		changed=cameraCorners[i].x!=previousCameraCorners[i].x
			|| cameraCorners[i].y!=previousCameraCorners[i].y
			|| cameraCorners[i].z!=previousCameraCorners[i].z;
	if (changed)
	{
		RebuildCameraOutline();
		for (int i=0;i<4;++i) previousCameraCorners[i]=cameraCorners[i];
	}
}
void AreaMapWindow::RebuildCameraOutline() //41E1F0.
{
	cameraOutline->Clear();
	if (!showCamera || !areaMap) return;
	const DWORD handle=fragments[4].GetTextureHandle();
	if (!handle || !g_GfxSystem->LookupTexture(handle)) return;
	LONG x,y,width,height;
	GetAbsolutePosition(x,y); GetSize(width,height);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	x=Truncate(x*sx); y=Truncate(y*sy);
	width=Truncate(width*sx); height=Truncate(height*sy);
	float mapPoints[4][2];
	WorldToMap(cameraCorners[0].x,cameraCorners[0].z,mapPoints[0][0],mapPoints[0][1]);
	WorldToMap(cameraCorners[1].x,cameraCorners[1].z,mapPoints[1][0],mapPoints[1][1]);
	WorldToMap(cameraCorners[2].x,cameraCorners[2].z,mapPoints[2][0],mapPoints[2][1]);
	WorldToMap(cameraCorners[3].x,cameraCorners[3].z,mapPoints[3][0],mapPoints[3][1]);
	const double inverseZoom=1.0f/zoom;
	LONG points[4][2];
	for (int i=0;i<4;++i)
	{
		points[i][0]=Truncate(double(mapPoints[i][0])*inverseZoom);
		points[i][1]=Truncate(double(mapPoints[i][1])*inverseZoom);
	}
	cameraOutline->SetColor(255,0,0,128);
	cameraOutline->renderGroup=1;
	const RECT clip={x,y,x+width,y+height};
	cameraOutline->SetClipRectangle(&clip);
	for (int i=0;i<4;++i)
		cameraOutline->AddLine(points[i][0],points[i][1],points[(i+1)%4][0],points[(i+1)%4][1]);
	cameraOutline->offsetX=x-originX;
	cameraOutline->offsetY=y-originY;
	depthDirty=1;
}


#include "game/AreaMapWindow.hpp"
#include "gui/Manager.hpp"
#include "graphics/SystemInterface.hpp"
#include "graphics/Texture.hpp"

namespace
{
	LONG Truncate(double value)
	{ return static_cast<LONG>(static_cast<__int64>(value)); }
}
BYTE AreaMapWindow::ComputeImageLayout(LONG& x, LONG& y, LONG& width, LONG& height,
	float& u, float& v, float& uvWidth, float& uvHeight,
	float& maskU, float& maskV, float& maskWidth, float& maskHeight,
	LONG& sourceX, LONG& sourceY, LONG& sourceWidth, LONG& sourceHeight) //41D7B0.
{
	if (!areaMap) return 0;
	::GUI::Fragment& fragment=fragments[4];
	const DWORD handle=fragment.GetTextureHandle();
	if (!handle) return 0;
	const LONG pictureSourceWidth=pictureWidth, pictureSourceHeight=pictureHeight;
	LONG windowWidth,windowHeight; GetSize(windowWidth,windowHeight);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	windowWidth=Truncate(windowWidth*sx);
	windowHeight=Truncate(windowHeight*sy);
	const float pictureWidthFloat=float(pictureSourceWidth);
	const float pictureHeightFloat=float(pictureSourceHeight);
	LONG firstX=0,firstY=0;
	const float inverseZoom=1.0f/zoom;
	LONG lastX=Truncate(pictureWidthFloat*inverseZoom);
	LONG lastY=Truncate(double(pictureSourceHeight)*inverseZoom);
	if (originX>0) firstX=originX;
	if (lastX>originX+windowWidth) lastX=originX+windowWidth;
	if (originY>0) firstY=originY;
	if (lastY>originY+windowHeight) lastY=originY+windowHeight;
	const LONG clippedWidth=lastX-firstX,clippedHeight=lastY-firstY;
	if (clippedWidth<=0 || clippedHeight<=0) return 0;
	const float maskFirstX=firstX/pictureWidthFloat*zoom;
	const float maskFirstY=firstY/pictureHeightFloat*zoom;
	const float maskLastX=lastX/pictureWidthFloat*zoom;
	const float maskLastY=lastY/pictureHeightFloat*zoom;
	x=firstX-originX; y=firstY-originY;
	width=lastX-originX-x; height=lastY-originY-y;
	LONG windowX,windowY; ::GUI::Base::GetPosition(windowX,windowY);
	x=Truncate(windowX*sx+x);
	y=Truncate(windowY*sy+y);
	LONG textureWidth=pictureSourceWidth,textureHeight=pictureSourceHeight;
	Gfx_Texture* texture=g_GfxSystem->LookupTexture(handle);
	if (texture)
	{
		const LONG frameCount=texture->frameCount;
		textureWidth=frameCount>0 ? LONG(texture->frames[0].width) : 0;
		textureHeight=frameCount>0 ? LONG(texture->frames[0].height) : 0;
	}
	if (flagF8D)
	{
		float fragmentU,fragmentV,fragmentWidth,fragmentHeight;
		fragment.GetSourceRect(fragmentU,fragmentV,fragmentWidth,fragmentHeight);
		const float textureHeightFloat=float(textureHeight);
		const float inverseWidth=1.0f/textureWidth;
		const float inverseHeight=1.0f/textureHeightFloat;
		const double baseU=double(fragmentU)*inverseWidth;
		const float baseV=float(double(fragmentV)*inverseHeight);
		const float spanU=float(double(fragmentWidth)*inverseWidth);
		const float spanV=float(double(fragmentHeight)*inverseHeight);
		u=float(double(spanU)*maskFirstX+baseU);
		v=float(double(spanV)*maskFirstY+baseV);
		uvWidth=float(double(spanU)*maskLastX+baseU-u);
		uvHeight=float(double(spanV)*maskLastY+baseV-v);
		sourceX=Truncate(double(textureWidth)*u);
		sourceY=Truncate(double(textureHeightFloat)*v);
		sourceWidth=Truncate(double(textureWidth)*uvWidth);
		sourceHeight=Truncate(double(textureHeightFloat)*uvHeight);
	}
	else
	{
		sourceX=firstX; sourceY=firstY;
		sourceWidth=clippedWidth; sourceHeight=clippedHeight;
		u=float(double(sourceX)/textureWidth*zoom);
		v=float(double(sourceY)/textureHeight*zoom);
		uvWidth=float(double(sourceWidth)/textureWidth*zoom);
		uvHeight=float(double(sourceHeight)/textureHeight*zoom);
	}
	maskU=maskFirstX; maskV=maskFirstY;
	maskWidth=float(double(maskLastX)-maskU);
	maskHeight=float(double(maskLastY)-maskV);
	return 1;
}
void AreaMapWindow::BuildImageGeometry() //4209D0.
{
	background.ready=0; scanlines.ready=0;
	if (!areaMap) return;
	const DWORD image=fragments[4].GetTextureHandle();
	if (!image || !g_GfxSystem->LookupTexture(image)) return;
	Gfx_Texture* scanline=NULL;
	if (scanlineTexture) scanline=g_GfxSystem->LookupTexture(scanlineTexture);
	float sx,sy; ::GUI::g_Manager.GetScale(sx,sy);
	LONG x,y,width,height,sourceX,sourceY,sourceWidth,sourceHeight;
	float u,v,uvWidth,uvHeight,maskU,maskV,maskWidth,maskHeight;
	DWORD indexCount=0,vertexCount=0;
	if (!ComputeImageLayout(x,y,width,height,u,v,uvWidth,uvHeight,
		maskU,maskV,maskWidth,maskHeight,sourceX,sourceY,sourceWidth,sourceHeight)) return;
	if (LONG(background.indexCapacity)-6>=0)
	{
		background.indices[0]=3;
		background.indices[1]=2;
		background.indices[2]=0;
		background.indices[3]=3;
		background.indices[4]=0;
		background.indices[5]=1;
		indexCount=6;
	}
	if (LONG(background.vertexCapacity)-4>=0)
	{
		const float pixelX=float(x);
		const float left=pixelX-0.5f;
		background.vertices[0].x=left;
		const float pixelY=float(y);
		const float top=pixelY-0.5f;
		background.vertices[0].y=top;
		const float right=float(width)+pixelX-0.5f;
		background.vertices[1].x=right;
		background.vertices[1].y=top;
		background.vertices[2].x=left;
		const float bottom=float(height)+pixelY-0.5f;
		background.vertices[2].y=bottom;
		background.vertices[3].x=right;
		background.vertices[3].y=bottom;

		background.vertices[0].color=0xFFFFFFFF;
		background.vertices[1].color=0xFFFFFFFF;
		background.vertices[2].color=0xFFFFFFFF;
		background.vertices[3].color=0xFFFFFFFF;

		background.vertices[0].u=u;
		background.vertices[0].v=v;
		const float imageRight=u+uvWidth;
		background.vertices[1].u=imageRight;
		background.vertices[1].v=v;
		background.vertices[2].u=u;
		const float imageBottom=v+uvHeight;
		background.vertices[2].v=imageBottom;
		background.vertices[3].u=imageRight;
		background.vertices[3].v=imageBottom;

		background.vertices[0].maskU=maskU;
		background.vertices[0].maskV=maskV;
		const float maskRight=maskU+maskWidth;
		background.vertices[1].maskU=maskRight;
		background.vertices[1].maskV=maskV;
		background.vertices[2].maskU=maskU;
		const float maskBottom=maskV+maskHeight;
		background.vertices[2].maskV=maskBottom;
		background.vertices[3].maskU=maskRight;
		background.vertices[3].maskV=maskBottom;
		vertexCount=4;
	}

	Gfx_MeshData& imageMesh=background.mesh;
	imageMesh.value04=0xFFFFFFFF;
	background.indexCount=indexCount;
	imageMesh.vertices=background.vertices;
	background.vertexCount=vertexCount;
	imageMesh.value00=0;
	imageMesh.value10=DWORD(background.indices);
	imageMesh.SetVertexDesc(0x244,D3DXGetFVFVertexSize(0x244),vertexCount);
	imageMesh.SetIndexDesc(2,background.indexCount);
	imageMesh.materialSet.Clear();
	Gfx_Primitive* primitive=imageMesh.SetPrimitiveCount(1);
	primitive->value00=0;
	primitive->type=D3DPT_TRIANGLELIST;
	primitive->value24=1;
	Gfx_Material imageMaterial;
	const DWORD imageMask=maskTexture;
	const DWORD imageStyle=imageMask ? 25 : 19;
	imageMaterial.style1=imageStyle;
	imageMaterial.style2=imageStyle;
	imageMaterial.textureHandles[1]=imageMask;
	imageMaterial.textureHandles[0]=image;
	imageMaterial.mode28=1;
	imageMaterial.flagBC=0;
	imageMesh.materialSet.materials[0]=imageMaterial;
	primitive->value28=0;
	primitive->value2C=0;
	primitive->value30=2;
	Gfx_PacketData* packet=background.packet;
	packet->transform=&background.transform;
	packet->transformEnabled=1;
	imageMesh.materialSet.RebuildGroups();
	imageMesh.value04=0xFFFFFFFF;
	background.instances[0].SetMesh(&imageMesh);
	background.instances[0].value0C=DWORD(background.packet);
	background.ready=1;

	const float scanPixelX=float(x);
	const float scanLeft=scanPixelX-0.5f;
	scanlines.vertices[0].x=scanLeft;
	const float scanPixelY=float(y);
	const float scanTop=scanPixelY-0.5f;
	scanlines.vertices[0].y=scanTop;
	const float quadWidth=float(width);
	const float scanRight=quadWidth+scanPixelX-0.5f;
	scanlines.vertices[1].x=scanRight;
	scanlines.vertices[1].y=scanTop;
	scanlines.vertices[2].x=scanLeft;
	const float quadHeight=float(height);
	const float scanBottom=quadHeight+scanPixelY-0.5f;
	scanlines.vertices[2].y=scanBottom;
	scanlines.vertices[3].x=scanRight;
	scanlines.vertices[3].y=scanBottom;

	LONG windowX,windowY,windowWidth,windowHeight;
	GetAbsolutePosition(windowX,windowY);
	GetSize(windowWidth,windowHeight);
	windowX=Truncate(windowX*sx);
	windowY=Truncate(windowY*sy);
	windowWidth=Truncate(windowWidth*sx);
	windowHeight=Truncate(windowHeight*sy);

	const LONG frames=scanline->frameCount;
	LONG textureWidth;
	if (frames<=0) textureWidth=0;
	else textureWidth=scanline->frames[0].width;
	LONG textureHeight;
	if (frames<=0) textureHeight=0;
	else textureHeight=scanline->frames[0].height;
	const float spanU=quadWidth/textureWidth;
	const float spanV=quadHeight/textureHeight;
	float scanV=0;
	if (!(sourceY%2)) scanV=1.0f/scanlineHeight;
	scanlines.vertices[0].u=0;
	scanlines.vertices[0].v=scanV;
	scanlines.vertices[1].u=spanU;
	scanlines.vertices[1].v=scanV;
	scanlines.vertices[2].u=0;
	const float uvBottom=scanV+spanV;
	scanlines.vertices[2].v=uvBottom;
	scanlines.vertices[3].u=spanU;
	scanlines.vertices[3].v=uvBottom;

	scanlines.vertices[0].maskU=maskU;
	scanlines.vertices[0].maskV=maskV;
	const float maskRight=maskU+maskWidth;
	scanlines.vertices[1].maskU=maskRight;
	scanlines.vertices[1].maskV=maskV;
	scanlines.vertices[2].maskU=maskU;
	const float maskBottom=maskV+maskHeight;
	scanlines.vertices[2].maskV=maskBottom;
	scanlines.vertices[3].maskU=maskRight;
	scanlines.vertices[3].maskV=maskBottom;

	Gfx_MeshData& scanMesh=scanlines.mesh;
	scanMesh.vertices=scanlines.vertices;
	scanlines.vertexCount=4;
	scanlines.indexCount=6;
	scanMesh.value00=0;
	scanMesh.value04=0xFFFFFFFF;
	scanMesh.value10=DWORD(scanlines.indices);
	scanMesh.SetVertexDesc(0x244,D3DXGetFVFVertexSize(0x244),4);
	scanMesh.SetIndexDesc(2,scanlines.indexCount);
	scanMesh.materialSet.Clear();
	primitive=scanMesh.SetPrimitiveCount(1);
	primitive->value00=0;
	primitive->type=D3DPT_TRIANGLELIST;
	primitive->value24=1;
	Gfx_Material scanMaterial;
	const DWORD scanMask=maskTexture;
	const DWORD scanStyle=scanMask ? 26 : 20;
	scanMaterial.style1=scanStyle;
	scanMaterial.style2=scanStyle;
	scanMaterial.textureHandles[1]=scanMask;
	scanMaterial.textureHandles[0]=scanlineTexture;
	scanMaterial.mode28=1;
	scanMaterial.flagBC=0;
	scanMesh.materialSet.materials[0]=scanMaterial;
	primitive->value28=0;
	primitive->value2C=0;
	primitive->value30=2;
	packet=scanlines.packet;
	packet->transform=&scanlines.transform;
	packet->transformEnabled=1;
	scanMesh.materialSet.RebuildGroups();
	scanMesh.value04=0xFFFFFFFF;
	scanlines.instances[0].SetMesh(&scanMesh);
	scanlines.instances[0].value0C=DWORD(scanlines.packet);
	scanlines.ready=1;
	depthDirty=1;
}

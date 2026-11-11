#include "gui/LineSystem.hpp"
#include "gui/Manager.hpp"
#include "gui/PacketManager.hpp"
#include "graphics/PacketData.hpp"
#include "graphics/SystemInterface.hpp"

namespace GUI
{
	LineSystem::LineSystem() : red(0),green(0),blue(0),alpha(255),renderGroup(1),
		offsetX(0),offsetY(0),clipRectangle(NULL),dirty(1),scaleToResolution(1)
	{
		D3DXMatrixIdentity(&transform);
		instanceCount=1;
		instances=new Gfx_MeshInstance[1];
		vertexCount=0;
		vertexCapacity=8;
		vertices=new LineVertex[vertexCapacity];
		indexCount=0;
		indexCapacity=8;
		indices=new WORD[indexCapacity];
		packet=new Gfx_PacketData;
		for (LONG i=0;i<LONG(vertexCapacity);++i)
		{
			vertices[i].color=0xFFFFFFFF;
			vertices[i].rhw=1;
			vertices[i].z=0;
		}
	}
	LineSystem::~LineSystem()
	{
		if (vertices) delete[] vertices;
		if (indices) delete[] indices;
		if (instances)
		{
			delete[] instances;
			instances = NULL;
		}
		if (packet)
		{
			delete packet;
			packet = NULL;
		}
		if (clipRectangle) delete clipRectangle;
	}
	void LineSystem::Clear()
	{
		segments.clear();
		dirty=1;
	}
	void LineSystem::SetColor(BYTE r,BYTE g,BYTE b,BYTE a)
	{
		red=r; green=g; blue=b; alpha=a;
	}
	void LineSystem::SetClipRectangle(const RECT* rectangle)
	{
		if (rectangle)
		{
			if (!clipRectangle) clipRectangle=new RECT;
			*clipRectangle=*rectangle;
		}
		else if (clipRectangle)
		{
			delete clipRectangle;
			clipRectangle=NULL;
		}
	}
	void LineSystem::AddLine(LONG x1,LONG y1,LONG x2,LONG y2)
	{
		AddLine(x1,y1,x2,y2,red,green,blue,alpha,red,green,blue,alpha);
	}
	void LineSystem::AddLine(LONG x1,LONG y1,LONG x2,LONG y2,
		BYTE r1,BYTE g1,BYTE b1,BYTE a1,BYTE r2,BYTE g2,BYTE b2,BYTE a2)
	{
		LineSegment line={x1,y1,x2,y2,r1,g1,b1,a1,r2,g2,b2,a2};
		segments.push_back(line);
		dirty=1;
	}
	void LineSystem::Update()
	{
		if (dirty) { BuildGeometry(); dirty=0; }
	}
	void LineSystem::Draw(Gfx_Viewport* viewport)
	{
		if (!geometryReady) return;
		if (!g_Manager.packets->optimizationLevel)
			g_GfxSystem->QueueMeshInstances(instances,viewport,instanceCount);
		else
			for (LONG i=0;i<LONG(instanceCount);++i)
				g_Manager.packets->Submit(&instances[i],viewport,0);
	}
	float LineSystem::SetDepth(float depth,float)
	{
		for (LONG i=0;i<LONG(vertexCount);++i) vertices[i].z=depth;
		return depth;
	}
	float LineSystem::GetDepth()
	{
		return LONG(vertexCount)>0 && vertices ? vertices[0].z : 0.0f;
	}
}

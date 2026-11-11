#include "gui/LineSystem.hpp"
#include "gui/Manager.hpp"
#include "graphics/PacketData.hpp"

namespace
{
	DWORD Pack(BYTE r,BYTE g,BYTE b,BYTE a)
	{ return (((((DWORD(a)<<8)|DWORD(r))<<8)|DWORD(g))<<8)|DWORD(b); }
}
namespace GUI
{
	void LineSystem::BuildGeometry() //4AE730.
	{
		geometryReady=0;
		if (!segments.size()) return;
		const LONG needed=LONG(segments.size()*2);
		if (needed>0)
		{
			if (needed>LONG(vertexCapacity))
			{
				if (vertices) { delete[] vertices; vertices=NULL; }
				LONG capacity=LONG(vertexCapacity)+20;
				if (needed>capacity) capacity=needed;
				vertexCapacity=DWORD(capacity);
				if (capacity>0) vertices=new LineVertex[capacity];
			}
			vertexCount=DWORD(needed);
			if (needed>LONG(indexCapacity))
			{
				if (indices) { delete[] indices; indices=NULL; }
				LONG capacity=LONG(indexCapacity)+30;
				if (needed>capacity) capacity=needed;
				indexCapacity=DWORD(capacity);
				if (capacity>0) indices=new WORD[capacity];
			}
			indexCount=DWORD(needed);
		}
		else vertexCount=0;
		if (LONG(vertexCount)<=0) return;
		float sx=1,sy=1;
		if (scaleToResolution) g_Manager.GetScale(sx,sy);
		LONG written=0,writtenIndices=0;
		for (std::vector<LineSegment>::iterator i=segments.begin();i!=segments.end();++i)
		{
			LineSegment line=*i;
			LONG x1=line.x1,y1=line.y1,x2=line.x2,y2=line.y2;
			if (!ClipLine(x1,y1,x2,y2)) continue;
			x1+=offsetX; x2+=offsetX; y1+=offsetY; y2+=offsetY;
			vertices[written].x=float(double(x1)*sx-double(sx)*0.5);
			vertices[written].y=float(double(y1)*sy-double(sy)*0.5);
			vertices[written].z=0;
			vertices[written].color=Pack(line.r1,line.g1,line.b1,line.a1);
			vertices[written].rhw=1;
			indices[writtenIndices++]=WORD(written++);
			vertices[written].x=float(double(x2)*sx-double(sx)*0.5);
			vertices[written].y=float(double(y2)*sy-double(sy)*0.5);
			vertices[written].z=0;
			vertices[written].color=Pack(line.r2,line.g2,line.b2,line.a2);
			vertices[written].rhw=1;
			indices[writtenIndices++]=WORD(written++);
		}
		if (written<=0 || writtenIndices<=0) return;
		indexCount=DWORD(writtenIndices);
		mesh.vertices=vertices;
		vertexCount=DWORD(written);
		mesh.value10=reinterpret_cast<DWORD>(indices);
		mesh.value00=0; mesh.value04=DWORD(-1);
		mesh.SetVertexDesc(0x44,D3DXGetFVFVertexSize(0x44),vertexCount);
		mesh.SetIndexDesc(2,indexCount);
		mesh.materialSet.Clear();
		Gfx_Primitive* primitive=mesh.SetPrimitiveCount(1);
		primitive->value00=0; primitive->type=D3DPT_LINELIST; primitive->value24=1;
		Gfx_Material material;
		material.style1=material.style2=15;
		material.textureHandles[0]=material.textureHandles[1]=0;
		material.mode28=renderGroup;
		material.flagBC=0;
		mesh.materialSet.materials[0]=material;
		primitive->value28=primitive->value2C=0; primitive->value30=DWORD(writtenIndices/2);
		packet->transformEnabled=1; packet->transform=&transform;
		mesh.materialSet.RebuildGroups();
		mesh.value04=DWORD(-1);
		instances[0].SetMesh(&mesh);
		instances[0].value0C=reinterpret_cast<DWORD>(packet);
		geometryReady=1;
	}
}

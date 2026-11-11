#include "LineSystem.hpp"
#include "ModelNode.hpp"
#include "SystemInterface.hpp"
#include <stdlib.h>
#include <string.h>
#include <new>

namespace
{
	struct LineVertex
	{
		float x, y, z;
		DWORD color;
	};
	struct LineSegmentSnapshot
	{
		float points[6];
		DWORD colors[8];
	};
	DWORD PackChannels(const DWORD* channels)
	{
		return (((((channels[3]<<8) | (channels[0]&255))<<8)
			| (channels[1]&255))<<8) | (channels[2]&255);
	}
}

void Gfx_LineSystem::BuildGeometry() //50F060.
{
	LONG written=0;
	if (!values108.size()) return; //Original leaves prior geometry/counts unchanged.
	const LONG needed=static_cast<LONG>(values108.size()*2);
	if (needed>0)
	{
		if (needed>static_cast<LONG>(vertexCapacity))
		{
			if (vertices) { free(vertices); vertices=NULL; }
			LONG capacity=static_cast<LONG>(vertexCapacity)+20;
			if (needed>capacity) capacity=needed;
			vertexCapacity=static_cast<DWORD>(capacity);
			if (capacity>0) vertices=::operator new(sizeof(LineVertex)*capacity);
		}
		vertexCount=static_cast<DWORD>(needed);
	}
	else vertexCount=0;
	if (static_cast<LONG>(vertexCount)<=0) return;
	std::vector<Gfx_LineSystemValue>::const_iterator i=values108.begin();
	if (i!=values108.end())
	{
		do
		{
			LineSegmentSnapshot segment;
			memcpy(&segment,&*i,sizeof(segment));
			static_cast<LineVertex*>(vertices)[written].x=segment.points[0];
			static_cast<LineVertex*>(vertices)[written].y=segment.points[1];
			static_cast<LineVertex*>(vertices)[written].z=segment.points[2];
			static_cast<LineVertex*>(vertices)[written++].color=PackChannels(segment.colors);
			static_cast<LineVertex*>(vertices)[written].x=segment.points[3];
			static_cast<LineVertex*>(vertices)[written].y=segment.points[4];
			static_cast<LineVertex*>(vertices)[written].z=segment.points[5];
			static_cast<LineVertex*>(vertices)[written++].color=PackChannels(segment.colors+4);
		} while (++i!=values108.end());
	}
	if (written<=0) { vertexCount=0; indexCount=0; return; }
	if (static_cast<LONG>(written)>static_cast<LONG>(indexCapacity))
	{
		if (indices) { free(indices); indices=NULL; }
		LONG capacity=static_cast<LONG>(indexCapacity)+30;
		if (static_cast<LONG>(written)>capacity) capacity=static_cast<LONG>(written);
		indexCapacity=static_cast<DWORD>(capacity);
		if (capacity>0) indices=::operator new(sizeof(WORD)*capacity);
	}
	indexCount=written;
	LONG index=0;
	DWORD vertex=0;
	i=values108.begin();
	if (i!=values108.end())
	{
		do
		{
			static_cast<WORD*>(indices)[index++]=static_cast<WORD>(vertex);
			static_cast<WORD*>(indices)[index++]=static_cast<WORD>(vertex+1);
			vertex+=2;
		} while (++i!=values108.end());
	}
	mesh.vertices=vertices;
	mesh.value10=reinterpret_cast<DWORD>(indices);
	mesh.value00=0;
	mesh.value04=DWORD(-1);
	mesh.SetVertexDesc(0x42,D3DXGetFVFVertexSize(0x42),vertexCount);
	mesh.SetIndexDesc(2,indexCount);
	Gfx_MaterialSet& materials=mesh.materialSet;
	materials.count=0;
	materials.ResetGroups();
	if (materials.materials)
	{ delete[] materials.materials; materials.materials=NULL; }
	Gfx_Primitive* primitive=mesh.SetPrimitiveCount(1);
	primitive->value00=0;
	primitive->type=D3DPT_LINELIST;
	primitive->value24=1;
	Gfx_Material material;
	material.style1=19;
	material.style2=19;
	material.mode28=value104;
	material.mode24=value104;
	material.flagBC=0;
	materials.materials[0]=material;
	primitive->value28=0;
	primitive->value2C=0;
	primitive->value30=static_cast<DWORD>(values108.size());
	Gfx_PacketData* currentPacket=packet;
	currentPacket->transformEnabled=1;
	currentPacket->transform=&transform;
	materials.RebuildGroups();
	mesh.value04=DWORD(-1);
	instances[0].SetMesh(&mesh);
	Gfx_PacketData* finalPacket=packet;
	Gfx_MeshInstance* finalInstances=instances;
	finalInstances[0].value0C=reinterpret_cast<DWORD>(finalPacket);
}

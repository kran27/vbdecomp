#include "LineSystem.hpp"
#include <stdlib.h>

void Gfx_LineSystem::Clear() //50EAD0.
{
	values108.clear(); //Actual supplied VC7 vector releases storage before clearing pointers.
	if (vertices) { ::operator delete[](vertices); vertices=NULL; }
	vertexCount=vertexCapacity=0;
	if (indices) { ::operator delete[](indices); indices=NULL; }
	indexCount=indexCapacity=0;
	enabled=1;
}

Gfx_LineSystem::Gfx_LineSystem()
{
	D3DXMatrixIdentity(&transform);
	position.z=0;
	position.y=0;
	position.x=0;
	color=255;
	value104=1;
	vertices=NULL;
	vertexCount=0;
	vertexCapacity=0;
	indices=NULL;
	indexCount=0;
	indexCapacity=0;
	enabled=1;
	instanceCount=1;
	instances = new Gfx_MeshInstance[1];
	LONG i=0;
	while (i < static_cast<LONG>(instanceCount))
	{
		Gfx_MeshInstance* current=instances+i;
		current->flag1D = 1;
		current=instances+i;
		current->flag1E = 1;
		++i;
	}
	packet = new Gfx_PacketData;
}

Gfx_LineSystem::Gfx_LineSystem(const Gfx_LineSystem& source)
{
	D3DXMatrixIdentity(&transform);
	position.z=0;
	position.y=0;
	position.x=0;
	color=255;
	value104=1;
	vertices=NULL;
	vertexCount=0;
	vertexCapacity=0;
	indices=NULL;
	indexCount=0;
	indexCapacity=0;
	enabled=1;
	instanceCount=1;
	//50FB40 allocates one fresh instance and keeps its default1D/1E flags.
	instances = new Gfx_MeshInstance[1];
	packet = new Gfx_PacketData;
	*this = source;
}

Gfx_LineSystem& Gfx_LineSystem::operator=(const Gfx_LineSystem& source)
{
	position.x = source.position.x;
	position.y = source.position.y;
	position.z = source.position.z;
	color = source.color;
	value104 = source.value104;
	values108 = source.values108;
	return *this;
}

Gfx_LineSystem::~Gfx_LineSystem()
{
	if (vertices) ::operator delete[](vertices);
	if (indices) ::operator delete[](indices);
	delete[] instances;
	if (packet) { delete packet; packet = NULL; }
}

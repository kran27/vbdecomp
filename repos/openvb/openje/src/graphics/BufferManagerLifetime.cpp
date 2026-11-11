#include "BufferManager.hpp"
#include <new>
#include <stdexcept>

Gfx_BufferManager::Gfx_BufferManager()
	: vertexStream(0), unknown2C(0), indexStream(0)
{
	// Original body stores follow deque/map, the three pointer initializers and
	// list construction. Leave these scalars unwritten until that point.
	usage = 0;
	nextHandle = 1;
	vertexBytes = 0;
	indexBytes = 0;
	vertexHighWater = 0;
	indexHighWater = 0;
	vertexStream = new Gfx_StreamingVertexBuffer;
	indexStream = new Gfx_StreamingIndexBuffer;
}

Gfx_BufferManager::~Gfx_BufferManager()
{
	ReleaseAll(1);
	if (vertexStream)
	{
		delete vertexStream;
		vertexStream = NULL;
	}
	if (unknown2C)
	{
		throw std::logic_error("Gfx_BufferManager::507580 live +2C owner type unresolved");
	}
	if (indexStream)
	{
		delete indexStream;
		indexStream = NULL;
	}
	// Normal list/map/deque destruction precedes publication-base destruction.
}

void Gfx_BufferManager::ReleaseAll(BYTE destroy)
{
	ReleaseStoredBuffers(destroy);
	if (vertexStream)
	{
		vertexStream->Release();
	}
	if (unknown2C)
	{
		throw std::logic_error("Gfx_BufferManager::506B70 live +2C release type unresolved");
	}
	if (indexStream)
	{
		indexStream->Release();
	}
	if (vertexBytes)
	{
		vertexBytes -= 0x1C0000;
	}
	if (indexBytes)
	{
		indexBytes -= 0x60000;
	}
	for (std::list<void*>::iterator item = devicePayloads.begin(); item != devicePayloads.end(); ++item)
	{
		Gfx_BufferDeviceChange(*item);
	}
	if (destroy)
	{
		devicePayloads.clear();
	}
}

void Gfx_BufferManager::Restore()
{
	InitializeStreams(usage == 0);
	for (std::list<void*>::iterator item = devicePayloads.begin(); item != devicePayloads.end(); ++item)
	{
		Gfx_BufferDeviceChange(*item);
	}
}

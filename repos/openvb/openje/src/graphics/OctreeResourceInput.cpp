#include "Octree.hpp"
#include "OctreeInput.hpp"
#include "OctreeResourceCallbacks.hpp"
#include "resources/ResourceSystem.hpp"

OctreeResourceReadCallback g_OctreeReadCallback = ::fread;
OctreeResourceSeekCallback g_OctreeSeekCallback = ::fseek;
OctreeResourceTellCallback g_OctreeTellCallback = ::ftell;

namespace
{
	class OctreeResourceInput : public OctreeInput
	{
	public:
		explicit OctreeResourceInput(JE::Resources::ResourceFileHandle* value) : handle(value) {}
		virtual size_t Read(void* output, size_t size, size_t count)
		{
			return ReadOctreeResourceInput(output, size, count, handle);
		}
		virtual int Seek(LONG offset, int origin)
		{
			return SeekOctreeResourceInput(handle, offset, origin);
		}
	private:
		JE::Resources::ResourceFileHandle* handle;
	};
}

OctreeResourceTellCallback SetOctreeResourceCallbacks(OctreeResourceReadCallback read,
	OctreeResourceSeekCallback seek, OctreeResourceTellCallback tell)
{
	//5AD940 updates only nonNULL arguments, in this order, and returns incoming tell.
	if (read) g_OctreeReadCallback = read;
	if (seek) g_OctreeSeekCallback = seek;
	if (tell) g_OctreeTellCallback = tell;
	return tell;
}

size_t ReadOctreeResourceInput(void* output, size_t size, size_t count,
	JE::Resources::ResourceFileHandle* handle)
{
	return g_OctreeReadCallback(output, size, count, reinterpret_cast<FILE*>(handle));
}

int SeekOctreeResourceInput(JE::Resources::ResourceFileHandle* handle, LONG offset, int origin)
{
	return g_OctreeSeekCallback(reinterpret_cast<FILE*>(handle), offset, origin);
}

LONG TellOctreeResourceInput(JE::Resources::ResourceFileHandle* handle)
{
	return g_OctreeTellCallback(reinterpret_cast<FILE*>(handle));
}

int OCTREE::Read8Resource(JE::Resources::ResourceFileHandle* handle,
	DWORD attributes, OCTREE** output)
{
	if (!handle) return 2;
	OctreeResourceInput input(handle);
	return Read8Input(input, attributes, output);
}

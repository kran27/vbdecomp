#include "MapGridServices.hpp"
#include "resources/ResourceSystem.hpp"

namespace JE { namespace MapGridServices
{
	Resources::ResourceFileHandle* Open(const char* name)
	{
		return Resources::g_pResourceSystem->OpenStream(name, 1);
	}
	BYTE Read(void* output, DWORD size, Resources::ResourceFileHandle* stream)
	{
		return Resources::g_pResourceSystem->Read(output, size, stream);
	}
	void Release(Resources::ResourceFileHandle* stream)
	{
		Resources::g_pResourceSystem->Release(stream, 0);
	}
}}

#include "ResourceIO.hpp"
#include "resources/ResourceSystem.hpp"

namespace GUI
{
	namespace ResourceIO
	{
		BYTE Read(void* output, DWORD size, JE::Resources::ResourceFileHandle* file)
		{
			return JE::Resources::g_pResourceSystem->Read(output, size, file);
		}
		JE::Resources::ResourceFileHandle* Open(const char* name)
		{
			return JE::Resources::g_pResourceSystem->OpenStream(name, 1);
		}
		BYTE Seek(JE::Resources::ResourceFileHandle* file, LONG offset, int origin)
		{
			return JE::Resources::g_pResourceSystem->Seek(file, offset, origin);
		}
		void Release(JE::Resources::ResourceFileHandle* file)
		{
			JE::Resources::g_pResourceSystem->Release(file, 0);
		}
	}
}

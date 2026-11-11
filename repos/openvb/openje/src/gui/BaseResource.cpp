#include "Base.hpp"
#include "resources/ResourceSystem.hpp"

namespace GUI
{
	BYTE Base::ReadResource(JE::Resources::ResourceFileHandle* file, int)
	{
		// The existing canonical resource owner supplies actual I/O. Its full
		// class/header migration to VC7 remains separate from Base geometry.
		if (!JE::Resources::g_pResourceSystem->Read(&rectangle, 16, file)) return 0;
		geometryDirty = 1;
		return 1;
	}
}

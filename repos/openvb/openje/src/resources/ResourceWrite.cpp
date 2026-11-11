#include "ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace JE { namespace Resources
{
	unsigned char ResourceSystem::Write(const void* input, Net::UInt32 count, ResourceFileHandle* handle)
	{
		if (!handle || (handle->file->flags & 2))
		{
			//The original NULL branch also dereferences the handle while building
			//its diagnostic. No successful NULL-handle behavior is supplied here.
			JE::LogDebugString("ResourceSystem::FileStreamWrite - File was opened as readonly - %s",
				handle->file->name);
			return 0;
		}
		handle->file->input.write(static_cast<const char*>(input), static_cast<std::streamsize>(count));
		return 1; //Original deliberately does not inspect stream state or update cursor.
	}
} }

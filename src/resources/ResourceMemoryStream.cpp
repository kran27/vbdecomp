#include "ResourceSystem.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>
#include <cstdint>
#include <cstddef>

namespace JE
{
    namespace Resources
    {
        BYTE ResourceSystem::SeekMemory(ResourceMemoryStream* stream, Net::Int32 offset, int origin)
        {
            if (!stream)
            {
                LogDebugString("ResourceSystem::MemStreamRead - memstream not open\n");
                return 0;
            }

            // Addresses are not 32-bit integers. Compute within the loaded
            // resource and validate the target before forming its pointer.
            const std::int64_t length = stream->end - stream->begin;
            const std::int64_t current = stream->cursor - stream->begin;
            std::int64_t target = -1;
            switch (origin)
            {
            case 0:
                target = offset;
                break;
            case 1:
                target = current + offset;
                break;
            case 2:
                // Preserve the original behavior: backwards end-relative
                // seeks were rejected, and forwards seeks exceed the buffer.
                if (offset == 0)
                {
                    target = length;
                }
                break;
            default:
                LogDebugString("ResourceSystem::MemStreamSeek - Incorrect seek position on memstream %s - tried = %d\n",
                               stream->resource->name.c_str(), origin);
                return 0;
            }

            if (target < 0 || target > length)
            {
                LogDebugString("ResourceSystem::MemStreamSeek - Tried a bad seek on memstream %s - offset = %d, origin = %d\n",
                               stream->resource->name.c_str(), offset, origin);
                return 0;
            }

            stream->cursor = stream->begin + static_cast<std::ptrdiff_t>(target);
            return 1;
        }

        Net::UInt32 ResourceSystem::TellMemory(ResourceMemoryStream* stream)
        {
            return static_cast<Net::UInt32>(stream->cursor - stream->begin);
        }

        BYTE ResourceSystem::ReadMemory(void* output, Net::UInt32 count, ResourceMemoryStream* stream)
        {
            if (!stream)
            {
                LogDebugString("ResourceSystem::MemStreamRead - memstream not open\n");
                return 0;
            }
            const BYTE* cursor = stream->cursor;
            if (cursor < stream->begin || cursor > stream->end ||
                static_cast<std::size_t>(count) > static_cast<std::size_t>(stream->end - cursor))
            {
                return 0;
            }
            memcpy(output, cursor, count);
            stream->cursor += count;
            return 1;
        }

        ResourceMemoryStream* ResourceSystem::OpenMemoryStream(const char* name, BYTE cached)
        {
            ResourceMemoryStream* stream = new ResourceMemoryStream;
            if (stream)
            {
                ResourceData* resource = Demand(name, cached);
                if (resource)
                {
                    stream->begin = static_cast<const BYTE*>(resource->data);
                    stream->end = static_cast<const BYTE*>(resource->data) + resource->size;
                    stream->cursor = stream->begin;
                    stream->resource = resource;
                }
                else
                {
                    LogDebugString("ResourceSystem::MemStreamOpen - Couldn't open memstream %s\n", name);
                    delete stream;
                    return NULL;
                }
            }
            return stream;
        }

        void ResourceSystem::CloseMemoryStream(ResourceMemoryStream* stream)
        {
            ReleaseResource(stream->resource);
            delete stream;
        }

        ResourceMemoryStream* OpenResourceMemoryStream(const char* name, BYTE cached)
        {
            return g_pResourceSystem->OpenMemoryStream(name, cached);
        }

        void CloseResourceMemoryStream(ResourceMemoryStream* stream)
        {
            g_pResourceSystem->CloseMemoryStream(stream);
        }
    } // namespace Resources
} // namespace JE

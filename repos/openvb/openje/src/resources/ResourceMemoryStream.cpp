#include "ResourceSystem.hpp"
#include "../diagnostics/DebugOutput.hpp"
#include <string.h>

namespace JE { namespace Resources
{
	BYTE ResourceSystem::SeekMemory(ResourceMemoryStream* stream, Net::Int32 offset, int origin)
	{
		if (!stream)
		{
			LogDebugString("ResourceSystem::MemStreamRead - memstream not open\n");
			return 0;
		}
		//These comparisons use the original unsigned32 address bits, including wrap.
		Net::UInt32 candidate;
		if (origin == 0)
		{
			if (offset >= 0)
			{
				candidate = Net::UInt32(reinterpret_cast<ULONG_PTR>(stream->begin))
					+ Net::UInt32(offset);
				if (candidate <= Net::UInt32(reinterpret_cast<ULONG_PTR>(stream->end)))
				{
					stream->cursor = reinterpret_cast<const BYTE*>(ULONG_PTR(candidate));
					return 1;
				}
			}
			const char* name = stream->resource->name.c_str();
			LogDebugString("ResourceSystem::MemStreamSeek - Tried a bad seek from the begining of memstream %s - endpos = %d tried = %d\n",
				name,
				Net::Int32(reinterpret_cast<ULONG_PTR>(stream->end)), offset);
			return 0;
		}
		else if (origin == 2)
		{
			//The original compares against end, so ordinary negative offsets fail.
			if (offset <= 0)
			{
				const Net::UInt32 end = Net::UInt32(reinterpret_cast<ULONG_PTR>(stream->end));
				candidate = end + Net::UInt32(offset);
				if (candidate >= end)
				{
					stream->cursor = reinterpret_cast<const BYTE*>(ULONG_PTR(candidate));
					return 1;
				}
			}
			const char* name = stream->resource->name.c_str();
			LogDebugString("ResourceSystem::MemStreamSeek - Tried a bad seek from the end of memstream %s - endpos = %d tried = %d\n",
				name,
				Net::Int32(reinterpret_cast<ULONG_PTR>(stream->end)), offset);
			return 0;
		}
		else if (origin == 1)
		{
			candidate = Net::UInt32(reinterpret_cast<ULONG_PTR>(stream->cursor))
				+ Net::UInt32(offset);
			if (candidate >= Net::UInt32(reinterpret_cast<ULONG_PTR>(stream->begin))
				&& candidate <= Net::UInt32(reinterpret_cast<ULONG_PTR>(stream->end)))
			{
				stream->cursor = reinterpret_cast<const BYTE*>(ULONG_PTR(candidate));
				return 1;
			}
			const char* name = stream->resource->name.c_str();
			LogDebugString("ResourceSystem::MemStreamSeek - Tried a bad seek from the current position of file %s - endpos = %d tried = %d\n",
				name,
				Net::Int32(reinterpret_cast<ULONG_PTR>(stream->end)), offset);
			return 0;
		}
		else
		{
			const char* name = stream->resource->name.c_str();
			LogDebugString("ResourceSystem::MemStreamSeek - Incorrect seek position on memstream %s - tried = %d\n",
				name, origin);
			return 0;
		}
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
		if (cursor + count > stream->end) return 0;
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
}}

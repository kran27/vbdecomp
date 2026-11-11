#pragma once

#include "resources/ResourceSystem.hpp"

namespace JE { namespace Common
{
	//Source factor for the repeated proprietary chunk header/backpatch sequence.
	//This is not serialized object storage or the memory-reader Chunk layout.
	struct WrittenChunk
	{
		DWORD start, sizeOffset;
	};

	inline BYTE BeginResourceChunk(Resources::ResourceFileHandle* stream,
		DWORD type, DWORD version, WrittenChunk& chunk)
	{
		DWORD size = 0;
		chunk.start = Resources::g_pResourceSystem->Tell(stream);
		if (!Resources::g_pResourceSystem->Write(&type, 4, stream)
			|| !Resources::g_pResourceSystem->Write(&version, 4, stream)) return 0;
		chunk.sizeOffset = Resources::g_pResourceSystem->Tell(stream);
		return Resources::g_pResourceSystem->Write(&size, 4, stream);
	}

	inline BYTE PatchResourceChunk(Resources::ResourceFileHandle* stream,
		const WrittenChunk& chunk, DWORD end)
	{
		DWORD size = end - chunk.start;
		Resources::g_pResourceSystem->Seek(stream, chunk.sizeOffset, 0);
		return Resources::g_pResourceSystem->Write(&size, 4, stream);
	}

	inline BYTE FinishResourceChunk(Resources::ResourceFileHandle* stream,
		const WrittenChunk& chunk)
	{
		DWORD end = Resources::g_pResourceSystem->Tell(stream);
		if (!PatchResourceChunk(stream, chunk, end)) return 0;
		Resources::g_pResourceSystem->Seek(stream, end, 0);
		return 1;
	}
} }

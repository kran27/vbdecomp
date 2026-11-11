#include "Chunk.hpp"
#include "resources/ResourceSystem.hpp"
#include <string.h>
#include <stdexcept>

namespace JE
{
	void __cdecl LogDebugString(const char* format, ...);
	void __cdecl FatalError(const char* format, ...); //4A1CD5 preserves the original continuation after48CC40.
}

namespace JE { namespace Common
{
	// Typed source adaptation of existing memory-stream read/tell/seek services.
	BYTE ReadMemory(void* output, DWORD size, Resources::ResourceMemoryStream* stream)
	{
		if (!stream)
		{
			LogDebugString("ResourceSystem::MemStreamRead - memstream not op");
			return 0;
		}
		if (size > DWORD(stream->end - stream->cursor)) return 0;
		memcpy(output, stream->cursor, size);
		stream->cursor += size;
		return 1;
	}

	BYTE ReadString(std::string& output, Resources::ResourceMemoryStream* stream)
	{
		output.erase(output.begin(), output.end());
		WORD size;
		if (!Resources::g_pResourceSystem->ReadMemory(&size, sizeof(size), stream)) return 0;
		if (size)
		{
			char* buffer = new char[size + 1];
			if (!Resources::g_pResourceSystem->ReadMemory(buffer, size, stream)) { delete[] buffer; return 0; }
			buffer[size] = 0;
			output = buffer; // Original uses strlen, so an embedded NUL truncates.
			delete[] buffer;
		}
		return 1;
	}

	BYTE ReadString(std::string& output, FILE* stream)
	{
		output.erase(output.begin(), output.end());
		WORD size;
		if (fread(&size, 2, 1, stream) != 1) return 0;
		if (size)
		{
			char* buffer = new char[size + 1];
			if (fread(buffer, 1, size, stream) != size) { delete[] buffer; return 0; }
			buffer[size] = 0;
			output = buffer;
			delete[] buffer;
		}
		return 1;
	}

	Chunk::Chunk() : mode(0), type(0), version(0), size(0), fileStream(NULL), stream(NULL), start(0) {}

	BYTE Chunk::Read(Resources::ResourceMemoryStream* input)
	{
		if (mode) FatalError("Chunk::Read() called when not in proper mode.\n");
		mode = 1;
		stream = input;
		start = Resources::g_pResourceSystem->TellMemory(input);
		BYTE success = Resources::g_pResourceSystem->ReadMemory(&type, 4, input);
		if (success) success = Resources::g_pResourceSystem->ReadMemory(&version, 4, input);
		if (success) success = Resources::g_pResourceSystem->ReadMemory(&size, 4, input);
		if (!success)
		{
			mode = 0;
			return 0;
		}
		return 1;
	}

	void Chunk::SkipToEnd()
	{
		if (mode != 1) FatalError("Chunk::SkipToEnd() called when not in write mode.\n");
		BYTE success = 1;
		Resources::ResourceFileHandle* file = fileStream;
		if (file)
		{
			const DWORD end = start + size;
			if (Resources::g_pResourceSystem->Tell(file) != end)
				success = Resources::g_pResourceSystem->Seek(fileStream, Net::Int32(end), 0);
		}
		else
		{
			Resources::ResourceMemoryStream* memory = stream;
			if (memory)
			{
				const DWORD end = start + size;
				if (Resources::g_pResourceSystem->TellMemory(memory) != end)
					success = Resources::g_pResourceSystem->SeekMemory(stream, Net::Int32(end), 0);
			}
		}
		if (!success) FatalError("Seek failed during Chunk::SkipToEnd().\n");
		mode = 0;
	}
}}

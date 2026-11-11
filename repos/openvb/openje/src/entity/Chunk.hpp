#pragma once

#include "resources/ResourceMemoryStream.hpp"
#include <string>
#include <stdio.h>

namespace JE { namespace Resources { struct ResourceFileHandle; } }

namespace JE { namespace Common
{
	class Chunk
	{
	public:
		Chunk(); // 0x4A1CA0.
		virtual ~Chunk() {} // 0x409270.
		BYTE Read(Resources::ResourceMemoryStream* stream); // 0x4A1CC0
		void SkipToEnd(); // 0x4A1D50; file handle takes precedence over memory stream.
		DWORD mode, type, version, size;
		Resources::ResourceFileHandle* fileStream;
		Resources::ResourceMemoryStream* stream;
		DWORD start;
	};
	BYTE ReadMemory(void* output, DWORD size, Resources::ResourceMemoryStream* stream);
	BYTE ReadString(std::string& output, Resources::ResourceMemoryStream* stream); // 0x49C8D0.
	BYTE ReadString(std::string& output, FILE* stream); //49C800.
	BYTE WriteString(const std::string& input, Resources::ResourceFileHandle* stream); //49C730.
	BYTE WriteString(const std::string& input, FILE* stream); //49C6C0, low-WORD prefix and payload.
}}

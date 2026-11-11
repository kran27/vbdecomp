#pragma once

#include <windows.h>

// Actual production-only reader534C70, called by the ANMT buffer loader.
namespace Gfx_TextureAnimationDetail
{
	static BYTE ReadLine(const char** next, int remaining,
		const char* source, int capacity, char* output)
	{
		if (!output || !source) return 0;
		*next = source;
		int copied = 0;
		const int limit = static_cast<int>(static_cast<unsigned int>(capacity) - 1U);
		while (**next != '\n')
		{
			const char value = **next;
			if (value == '\r' || remaining <= 0) break;
			if (copied < limit) output[copied++] = value;
			++*next;
			--remaining;
		}
		if (**next == '\r' && remaining > 0)
		{
			++*next;
			--remaining;
		}
		if (**next == '\n' && remaining > 0) ++*next;
		if (copied < capacity) output[copied] = 0;
		return 1;
	}
}

// Public source-compatible verification adapter; its CDECL body is not the
// original private-register reader. Output truncation still consumes the line.
BYTE Gfx_ReadTextureAnimationLine(const char** next, int remaining,
	const char* source, int capacity, char* output);

// Proprietary backward all-byte predicate0x512DE0. Reuses official CRT _mbschr,
// including the active code page; the accepted set is not general isspace.
BYTE Gfx_IsTextureAnimationWhitespace(const char* text, const char* acceptedBytes);

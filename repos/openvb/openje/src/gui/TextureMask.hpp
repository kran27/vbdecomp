#pragma once

#include <windows.h>
#include <map>

namespace GUI
{
	// Original nonvirtual12-byte owner used by Fragment and Manager+48.
	// Source name is a hypothesis; fields are established by522D50/522DA0.
	struct TextureMask
	{
		bool Test(LONG x, LONG y); // 0x522D50; no coordinate/buffer bounds guard.
		BYTE Load(DWORD handle); // 0x522DA0; resets first, builds from frame zero.

		DWORD textureHandle; // +0; published only at the loader's success tail.
		LONG rowWidth; // +4; signed wrapped linear addressing in Test.
		BYTE* bits; // +8; null buffer accepts every point in Test.
	};

	// Nonvirtual owner at Manager+48; real VC7 map12 and proprietary miss policy.
	class TextureMaskCache
	{
	public:
		~TextureMaskCache(); // 0x5233E0; Clear then actual map member destruction.
		void Clear(); // 0x522FF0; frees bits/masks before advancing live iterator.
		TextureMask* Get(DWORD handle); // 0x523320.
		std::map<DWORD, TextureMask*> entries;
	};
}

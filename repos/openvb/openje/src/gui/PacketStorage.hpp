#pragma once

#include <windows.h>
#include <list>

class Gfx_UIPacketBuffer;

namespace GUI
{
	// Actual nonvirtual Manager+0xB4 storage owner; original source name unknown.
	// Preserve the existing address-derived class name until an original name is known.
	class cls_0x4b6910
	{
	public:
		explicit cls_0x4b6910(DWORD bytesPerBlock); // 0x4B6910.
		~cls_0x4b6910(); // 0x4B6A00; normal reverse member destruction follows body.
		void Reset(); // 0x4B61D0; reuse storage and batch allocations, no cleanup.
		BYTE* Allocate(DWORD bytes); // 0x4B6BB0; no alignment or oversize guard.
		Gfx_UIPacketBuffer* AcquireBatch(); // 0x4B6C60.

		DWORD blockBytes; // +00; comparisons interpret the raw word as signed.
		DWORD usedBytes; // +04; wrapped 32-bit arithmetic.
		BYTE* currentBlock; // +08; borrowed pointer into owned blocks.
		DWORD nextBatch; // +0C; compared as signed, incremented as raw DWORD.
		std::list<BYTE*> blocks; // +10; owns each raw allocation, not just its nodes.
		std::list<Gfx_UIPacketBuffer*> batches; // +1C; owns each actual batch object.

	private:
		cls_0x4b6910(const cls_0x4b6910&);
		cls_0x4b6910& operator=(const cls_0x4b6910&);
	};

}

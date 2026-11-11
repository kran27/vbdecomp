#pragma once

#include <cstddef>
#include <list>

namespace GUI
{
	// Original secondary base of Window at +0x18. The original table contains
	// one pure virtual destructor; the class itself has only its compiler vptr.
	class HeapTracker
	{
	public:
		virtual ~HeapTracker() = 0; // Body 0x4A7530.
		static void* operator new(std::size_t size); // 0x4A7680.
		static void operator delete(void* block); // 0x4A7540.
		bool IsHeapAllocated(); // 0x4A75A0; compares this without base adjustment.

	private:
		friend struct HeapTrackerVerification; // Scoped fixture inspection, no runtime API.
		static std::list<void*> allocations; // Original object 0x707DF4.
	};
}

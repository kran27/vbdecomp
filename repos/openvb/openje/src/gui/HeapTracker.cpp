#include "HeapTracker.hpp"
#include <algorithm>
#include <new>

namespace GUI
{
	std::list<void*> HeapTracker::allocations;

	HeapTracker::~HeapTracker() {}

	void* HeapTracker::operator new(std::size_t size)
	{
		void* block = ::operator new(size);
		allocations.push_front(block);
		return block;
	}

	void HeapTracker::operator delete(void* block)
	{
		std::list<void*>::iterator found =
			std::find(allocations.begin(), allocations.end(), block);
		if (found != allocations.end())
		{
			allocations.erase(found);
			::operator delete(block);
		}
	}

	bool HeapTracker::IsHeapAllocated()
	{
		return std::find(allocations.begin(), allocations.end(), this) != allocations.end();
	}
}

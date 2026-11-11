#include <winsock2.h>
#include "ClientMemoryCommand.hpp"
#include "MemorySystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>

namespace F3
{
	namespace
	{
		struct PoolTestValue
		{
			PoolTestValue() { ++g_memoryTestCounter; }
			static void* operator new(std::size_t) { return g_pMemorySystem->Allocate(0); }
			static void operator delete(void* storage) { g_pMemorySystem->Release(0,storage); }
		};
#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char MemoryTestValueOriginalSize1[(sizeof(PoolTestValue) == 1) ? 1 : -1];
#endif
	}

	void __cdecl CommandMemoryPoolTest(CommandArguments*) //589580.
	{
		if (!g_memoryTestFrequency.QuadPart) QueryPerformanceFrequency(&g_memoryTestFrequency);
		LARGE_INTEGER start, end;
		QueryPerformanceCounter(&start);
		PoolTestValue* pooled = new PoolTestValue;
		QueryPerformanceCounter(&end);
		JE::LogDebugString("MemoryTest: Memory Pool new time = %I64dns\n",
			1000000 * (end.QuadPart-start.QuadPart) / (g_memoryTestFrequency.QuadPart/1000));
		QueryPerformanceCounter(&start);
		if (pooled) delete pooled;
		QueryPerformanceCounter(&end);
		JE::LogDebugString("MemoryTest: Memory Pool delete time = %I64dns\n",
			1000000 * (end.QuadPart-start.QuadPart) / (g_memoryTestFrequency.QuadPart/1000));
		QueryPerformanceCounter(&start);
		PoolTestValue* ordinary = ::new PoolTestValue;
		QueryPerformanceCounter(&end);
		JE::LogDebugString("MemoryTest: OS new time = %I64dns\n",
			1000000 * (end.QuadPart-start.QuadPart) / (g_memoryTestFrequency.QuadPart/1000));
		QueryPerformanceCounter(&start);
		::delete ordinary;
		QueryPerformanceCounter(&end);
		JE::LogDebugString("MemoryTest: OS delete time = %I64dns\n",
			1000000 * (end.QuadPart-start.QuadPart) / (g_memoryTestFrequency.QuadPart/1000));
		pooled = new PoolTestValue;
		if (pooled)
		{
			JE::LogDebugString("MPTest: m_int == %d\n",g_memoryTestCounter);
			delete pooled;
		}
	}

	bool ExecuteClientMemoryCommand(JE::Net::UInt32 address, CommandArguments* arguments)
	{
		if (address != 0x589580) return false;
		CommandMemoryPoolTest(arguments);
		return true;
	}
}

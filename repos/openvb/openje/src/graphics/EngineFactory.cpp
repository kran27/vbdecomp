#include "EngineFactory.hpp"
#include "Engine.hpp"
#include "ParticleSystem.hpp"
#include "model/AnimationCache.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace Granny21
{
	//Narrow declarations for the existing shipped DLL imports. The current
	//selected public Granny header does not declare these two original APIs.
	typedef void (__cdecl *LogFunction)(int type, int origin, const char* message, void* userData);
	struct LogCallback
	{
		LogFunction function;
		void* userData;
	};

	extern "C"
	{
		__declspec(dllimport) int __stdcall GrannyVersionsMatch_(int major, int minor,
			int customization, int build);
		__declspec(dllimport) void __stdcall GrannySetLogCallback(const LogCallback* callback);
	}
}

namespace
{
	void __cdecl GrannyLog(int, int, const char* message, void* userData)
	{
		JE::LogDebugString("GRANNY ERROR - ");
		//4A1DF0 passes the incoming message as the format, with no extra arguments.
		JE::LogDebugString(message);
		JE::LogDebugString("\n");
		*static_cast<BYTE*>(userData) = 1;
	}
}

BYTE Gfx_InitializeAnimationSystem()
{
	BYTE initialized = 0;
	if (static_cast<BYTE>(Granny21::GrannyVersionsMatch_(2, 1, 0, 5)))
	{
		Granny21::LogCallback callback;
		callback.function = GrannyLog;
		callback.userData = &JE::g_GrannyError;
		Granny21::GrannySetLogCallback(&callback);
		initialized = 1;
	}
	else
	{
		JE::FatalError("Gfx_InitAnimationSystem - Granny2.dll version mismatch!\n");
	}
	return initialized;
}

Gfx_System_Interface* Gfx_CreateEngine()
{
	Gfx_System_Interface* engine = g_GfxSystem;
	if (engine || !Gfx_InitializeAnimationSystem())
	{
		return engine;
	}
	Gfx_InitializeParticleTables();
	return new Gfx_Engine;
}

void Gfx_ReleaseEngine()
{
	Gfx_System_Interface* engine = g_GfxSystem;
	if (engine)
	{
		delete engine;
	}
}

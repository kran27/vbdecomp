#include "EngineFactory.hpp"
#include "GrannyRuntime28.hpp"
#include "Engine.hpp"
#include "ParticleSystem.hpp"
#include "model/AnimationCache.hpp"
#include "diagnostics/DebugOutput.hpp"

namespace
{
	void __cdecl GrannyLog(granny_log_message_type, granny_log_message_origin,
		const char*, granny_int32x, const char* message, void* userData)
	{
		JE::LogDebugString("GRANNY ERROR - ");
		// Treat the DLL message as data, not as a printf format string.
		JE::LogDebugString("%s", message ? message : "");
		JE::LogDebugString("\n");
		if (userData)
		{
			*static_cast<BYTE*>(userData) = 1;
		}
	}
}

BYTE Gfx_InitializeAnimationSystem()
{
	BYTE initialized = 0;
	if (static_cast<BYTE>(Granny28::GrannyVersionsMatch_(2, 8, 49, 0)))
	{
		Granny28::LogCallback callback{};
		callback.Function = GrannyLog;
		callback.UserData = &JE::g_GrannyError;
		Granny28::GrannySetLogCallback(&callback);
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

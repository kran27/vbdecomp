#pragma once

#include "configuration/Configuration.hpp"
#include "network/Types.hpp"
#include <cstddef>
#include <string>

namespace JE
{
	class CMusic;
	typedef CMusic AudioMusic;

	class Audio
	{
	public:
		Audio(); // 0x590B10.
		virtual ~Audio(); // 0x591520.
		virtual void Initialize(); // 0x590BE0; load both settings owners.
		virtual void ResetDefaults(); // 0x590B50.
		virtual void SaveSettings(); // 0x591110; in-memory main settings only.
		void Cleanup(); // 0x5909E0.
		void Prepare(); // 0x590A20.
		void Update(); // 590860, actual camera target -> group listener.
		void PlayMusic(const char* name); // 5908F0.
		void StopMusic(JE::Net::UInt32 fadeOut); // 590990, return value unused.

		std::string groupList;
		std::string musicJumpList;
		JE::Net::UInt8 directShow, directMusic, cdAudio;
		AudioMusic* music;
		JE::Net::UInt8 musicEnabled;
		JE::Net::UInt32 musicVolume;
	};

	namespace AudioServices
	{
		// The host owns the actual canonical settings, backend and worker state.
		Configuration::Settings* GetEngineSettings();
		Configuration::Settings* GetSettings();
		unsigned __stdcall Worker(void* argument); //5909A0, actual CRT thread entry.
		extern JE::Net::UInt8 g_WorkerRunning;
		void InitializeBackend(const char* groups, const char* jumps,
			bool directShow, bool directMusic, bool cdAudio);
		::uintptr_t StartWorker();
		void CloseWorker(::uintptr_t handle);
		void Sleep(JE::Net::UInt32 milliseconds);
		AudioMusic* CreateMusic();
		void DestroyMusic(AudioMusic* music);
		void FreeMusic(AudioMusic* music);
		void ShutdownBackend();

		// Thin real MSVC map/string/conversion APIs. Only focused comparisons
		// substitute these calls; no old container/string layout is implemented.
		const Configuration::Values* FindSection(const Configuration::Settings& settings,
			const char* name);
		const std::string* FindValue(const Configuration::Values& values, const char* name);
		Configuration::Values* GetOrCreateSection(Configuration::Settings& settings,
			const char* name);
		std::string* GetOrCreateValue(Configuration::Values& values, const char* name);
		void AssignString(std::string& target, const char* value);
		JE::Net::Int32 ToLong(const char* value);
		const char* FormatInteger(JE::Net::Int32 value, char* buffer);
	}
}

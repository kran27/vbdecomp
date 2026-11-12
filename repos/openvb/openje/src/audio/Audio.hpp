#pragma once

#include "configuration/Configuration.hpp"
#include "network/Types.hpp"
#include <string>

namespace JE
{
	class CMusic;

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
		CMusic* music;
		JE::Net::UInt8 musicEnabled;
		JE::Net::UInt32 musicVolume;
	};

	namespace AudioServices
	{
		unsigned __stdcall Worker(void* argument); //5909A0, actual CRT thread entry.
		extern JE::Net::UInt8 g_WorkerRunning;
	}
}

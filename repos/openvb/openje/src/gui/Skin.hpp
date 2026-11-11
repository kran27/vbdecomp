#pragma once

#include <windows.h>
#include <map>
#include <string>
#include "configuration/ConfigurationTypes.hpp"

namespace GUI
{
	// Actual six-DWORD record. Texture is a game handle, not an SDK COM pointer.
	struct SkinFragment
	{
		SkinFragment(); // 0x523450.
		~SkinFragment(); // 0x523470; releases a nonzero handle through Graphics.
		void Parse(const JE::Configuration::Values& values); // 0x523490.

		DWORD flags, textureHandle, color, width, height, rate;
	};

	// Actual four-slot,40-byte VC7 owner. Normal map/fallback lifetime supplies
	// cleanup; host builds use their own real library layouts without overlays.
	class Skin
	{
	public:
		Skin(); // 0x4B5B10.
		virtual ~Skin(); // 0x4B5BB0; compiler deleting helper4B5B90 not a manual API.
		virtual BYTE LoadFile(const std::string& name); // +04 ->0x522B10.
		virtual BYTE LoadResource(const std::string& name); // +08 ->0x522C30.
		virtual void Clear(); // +0C ->0x5226A0.
		SkinFragment* Find(std::string name); // 0x522530, borrowed mapped/fallback record.
		BYTE Apply(const JE::Configuration::Settings& settings); // 0x522AC0.

		std::map<std::string, SkinFragment> fragments; // +04.
		SkinFragment fallback; // +10; destroyed before the map.

	private:
		Skin(const Skin&);
		Skin& operator=(const Skin&);
	};
}

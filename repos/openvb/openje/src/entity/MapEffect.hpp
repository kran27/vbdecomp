#pragma once
#include "Map.hpp"
#include <vector>
namespace JE
{
	class MapEffect
	{
	public:
		explicit MapEffect(Map* owner); // 0x5B2FA0.
		virtual ~MapEffect(); // 0x5B2ED0.
		void Clear(); // 0x5B2D10.
		void Initialize(); // 0x5B2DA0.
		void SetEnabled(BYTE input); //5B2C80, truth-value gate then raw BYTE/child publication.
		void ResetElapsedTime(); //5B2CD0, only the VisualEffect+16C clocks.
		std::string name;
		D3DXVECTOR3 position, angles;
		std::vector<std::string> resources;
		BYTE enabled;
		Map* owner;
		std::map<DWORD,VisualEffect*> instances;
	};
}

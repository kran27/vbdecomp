#pragma once

#include "model/AnimationCache.hpp"

namespace F3
{
	extern std::string g_animationGroups[516]; // Original owned table0x6F4170.

	class GameAnimationCache : public JE::AnimationCache
	{
	public:
		GameAnimationCache(); // 0x45C6D0.
		virtual ~GameAnimationCache(); // 0x45C6F0.
		virtual void ReadAnimations(Gfx_AnimationObject* object, std::string name,
			BOOL requested); // 0x45D300; nonzero requests only group416.
		static DWORD GetGroupCount();
		static const char* GetGroupSuffix(DWORD group); // Original table0x6F4170.
	};
}

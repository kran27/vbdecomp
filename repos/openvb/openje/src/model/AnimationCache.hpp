#pragma once

#include <windows.h>
#include <map>
#include <list>
#include <string>
#include "CommonException.hpp"
#include "configuration/ConfigurationTypes.hpp"
#include "entity/IComponent.hpp"

class Gfx_AnimationObject;
class Gfx_AnimationInstance;

namespace JE
{
	struct AnimationCacheEntry
	{
		Gfx_AnimationObject* object;
		DWORD references;
	};

	class AnimationCache : public IComponent
	{
	public:
		AnimationCache(); // 0x59BBE0; containers precede publication.
		virtual ~AnimationCache(); //0x59BA80; retains original erase/advance quirk.
		virtual void Initialize(); //+04,0x59B1E0, actual IComponent owner.
		virtual void Cleanup(); //+08,0x59AFE0; does not reset defaultSkeletonSize.
		virtual void Update(); //+0C, shared404B70, actual IComponent no-op.
		virtual void ReadAnimations(Gfx_AnimationObject* object, std::string name,
			BOOL requested); // +10,0x59B120; base does nothing beyond string destruction.
		virtual void BeforeStartup(); // +14, shared404B70.
		virtual void BeforeShutdown(); // +18, shared404B70.
		// Historical fixture names are ordinary wrappers, not component slots.
		void Startup() { Initialize(); }
		void Shutdown() { Cleanup(); }

		Gfx_AnimationObject* AcquireAnimObject(std::string name, BYTE loadAnimations); // 0x59BCC0.
		void ReleaseAnimObject(Gfx_AnimationObject* object); // 0x59B650.

		typedef std::map<std::string, AnimationCacheEntry, Configuration::CaseInsensitiveLess> Entries;
		Entries objects; // +04, owns objects; native433930 case-insensitive ordering.
		std::list<Gfx_AnimationInstance*> instances; // +10, owns listed instances.
		BYTE* defaultSkeleton; // +1C; owned by Startup/Shutdown, not ordinary dtor.
		DWORD defaultSkeletonSize; // +20.

	protected:
		BYTE* ReadSkeletonResource(const std::string& name, DWORD* size); // 0x59B050.
	private:
		AnimationCache(const AnimationCache&);
		AnimationCache& operator=(const AnimationCache&);
	};

	extern AnimationCache* g_AnimationCache; // Original70C658, loader-zero.
	// Original707DF0 is set by the separate Granny log callback4A1DF0.
	// That animation-system registration owner5B3070 is not installed by this cache.
	extern BYTE g_GrannyError;
}

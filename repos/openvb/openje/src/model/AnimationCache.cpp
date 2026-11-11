#include "AnimationCache.hpp"
#include "ModelServices.hpp"
#include "graphics/AnimationObject.hpp"
#include "graphics/AnimationInstance.hpp"
#include "resources/ResourceSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <new>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>

namespace JE
{
	AnimationCache* g_AnimationCache;
	BYTE g_GrannyError;
	static std::string skeletonResourceExtension(".skl");
	static std::string defaultSkeletonResourceName("DefaultSkeleton.skl");

	AnimationCache::AnimationCache()
	{
		if (g_AnimationCache)
		{
			throw Common::Exception("AnimationCache object already exists.");
		}
		g_AnimationCache = this;
		defaultSkeleton = NULL;
		defaultSkeletonSize = 0;
	}

	AnimationCache::~AnimationCache()
	{
		while (!instances.empty())
		{
			Gfx_AnimationInstance* instance = instances.front();
			instances.pop_front();
			delete instance;
		}
		for (Entries::iterator entry=objects.begin();
			entry!=objects.end();++entry)
		{
			AnimationCacheEntry cached=entry->second;
			objects.erase(entry); //59BB34 precedes the captured payload deletion.
			delete cached.object;
			//59BB51 advances the original, now-erased iterator. This is the
			//observed original source quirk, using the actual VC7 container.
		}
		g_AnimationCache = NULL;
		// Original does not call Shutdown here or free defaultSkeleton.
	}

	void AnimationCache::Update() {} //404B70, actual IComponent slot+0C.
	void AnimationCache::BeforeStartup() {}
	void AnimationCache::BeforeShutdown() {}
	void AnimationCache::ReadAnimations(Gfx_AnimationObject*, std::string, BOOL) {}

	BYTE* AnimationCache::ReadSkeletonResource(const std::string& name, DWORD* size)
	{
		BYTE* data = NULL;
		if (Resources::g_pResourceSystem->Contains(name.c_str()))
		{
			Resources::ResourceMemoryStream* stream =
				Resources::g_pResourceSystem->OpenMemoryStream(name.c_str(), 1);
			if (!stream) return NULL;
			if (!Resources::g_pResourceSystem->SeekMemory(stream, 0, 2))
			{
				Resources::g_pResourceSystem->CloseMemoryStream(stream);
				return NULL;
			}
			*size = Resources::g_pResourceSystem->TellMemory(stream);
			if (!Resources::g_pResourceSystem->SeekMemory(stream, 0, 0))
			{
				Resources::g_pResourceSystem->CloseMemoryStream(stream);
				return NULL;
			}
			data = new BYTE[*size];
			if (!Resources::g_pResourceSystem->ReadMemory(data, *size, stream))
			{
				Resources::g_pResourceSystem->CloseMemoryStream(stream);
				return NULL; // Original failed-read path leaks the allocated bytes.
			}
			Resources::g_pResourceSystem->CloseMemoryStream(stream);
		}
		return data;
	}

	void AnimationCache::Initialize() //59B1E0.
	{
		BeforeStartup();
		defaultSkeleton = ReadSkeletonResource(defaultSkeletonResourceName.c_str(), &defaultSkeletonSize);
		if (!defaultSkeleton)
		{
			FatalError("AnimationCache::Startup() - Could not load default skeleton.");
		}
	}

	void AnimationCache::Cleanup() //59AFE0.
	{
		BeforeShutdown();
		if (defaultSkeleton)
		{
			::operator delete[](defaultSkeleton);
			defaultSkeleton = NULL;
		}
	}

	Gfx_AnimationObject* AnimationCache::AcquireAnimObject(std::string name, BYTE loadAnimations)
	{
		Gfx_AnimationObject* object;
		Entries::iterator found;
		found = objects.find(name);
		if (found != objects.end())
		{
			object = found->second.object;
			++found->second.references;
		}
		else
		{
			std::string resourceName = name + skeletonResourceExtension;
			DWORD size = 0;
			BYTE borrowed = 0;
			BYTE* data = ReadSkeletonResource(resourceName, &size);
			if (!data)
			{
				char prefix[4];
				strncpy(prefix, name.c_str(), 3);
				prefix[3] = 0;
				_strupr(prefix);
				if (strcmp(prefix, "WP_") && strcmp(prefix, "SH_")) return NULL;
				data = defaultSkeleton;
				size = defaultSkeletonSize;
				resourceName = defaultSkeletonResourceName;
				borrowed = 1;
			}

			object = new Gfx_AnimationObject;
			g_GrannyError = 0;
			if (!object->LoadSkeletonFromBuffer(size, data, 10.0f))
			{
				FatalError("AnimationCache::AcquireAnimObject() - Unable to load skeleton data from buffer.");
			}
			if (g_GrannyError)
			{
				LogDebugString("AnimationCache::AcquireAnimObject() - LoadSkeletonFromBuffer (%s) Granny error\n",
					resourceName.c_str());
			}
			if (!borrowed) ::operator delete[](data);
			ReadAnimations(object, name, loadAnimations ? TRUE : FALSE);
			AnimationCacheEntry entry;
			entry.object = object;
			entry.references = 1;
			objects.insert(Entries::value_type(name, entry));
		}
		return object;
	}

	void AnimationCache::ReleaseAnimObject(Gfx_AnimationObject* object)
	{
		for (Entries::iterator entry = objects.begin();
			entry != objects.end(); ++entry)
		{
			if (entry->second.object == object)
			{
				if (--entry->second.references == 0)
				{
					delete entry->second.object;
					objects.erase(entry);
				}
				return;
			}
		}
	}
}

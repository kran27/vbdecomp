#include "SoundInstanceManager.hpp"
#include "SoundInstance.hpp"
#include "SoundPreset.hpp"
#include "SoundGroups.hpp"
#include "SoundSystem.hpp"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <new>

namespace JE
{
	namespace
	{
		struct InstanceGuard
		{
			explicit InstanceGuard(Resources::ResourceLock& value) : lock(value) { lock.Lock(); }
			~InstanceGuard() { lock.Unlock(); }
			Resources::ResourceLock& lock;
		};
#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
		typedef char ManagerSize[(sizeof(SoundInstanceManager)==0x1C) ? 1 : -1];
		typedef char ManagerList[(offsetof(SoundInstanceManager,instances)==0) ? 1 : -1];
		typedef char ManagerLock[(offsetof(SoundInstanceManager,lock)==0x0C) ? 1 : -1];
		typedef char ManagerListener[(offsetof(SoundInstanceManager,listenerPosition)==0x10) ? 1 : -1];
#endif
	}
	SoundInstanceManager::SoundInstanceManager() : listenerPosition(0,0,0)
	{
		listenerPosition.x=0; listenerPosition.y=0; listenerPosition.z=0;
		instances.clear();
	}
	SoundInstanceManager::~SoundInstanceManager()
	{
		instances.clear(); //Then lock destruction and real list lifetime, without deleting elements.
	}
	int SoundInstanceManager::Initialize()
	{
		return 0; //4080B0, actual full EAX return and plain RET.
	}
	void SoundInstanceManager::Cleanup()
	{
		for (;;)
		{
			std::list<VfxSoundInstance*>::iterator i=instances.begin();
			while (i!=instances.end() && !((*i)->flags&2)) ++i;
			if (i==instances.end()) break;
			VfxSoundInstance* ended=*i;
			instances.erase(i);
			ended->~VfxSoundInstance();
			::operator delete(ended);
		}
	}
	void SoundInstanceManager::Update()
	{
		InstanceGuard guard(lock);
		bool retire=false;
		for (std::list<VfxSoundInstance*>::iterator i=instances.begin();i!=instances.end();++i)
		{
			VfxSoundInstance* instance=*i;
			instance->Update();
			if (instance->CanAutoDelete()) retire=true;
		}
		if (retire)
		{
			for (;;)
			{
				std::list<VfxSoundInstance*>::iterator i=instances.begin();
				VfxSoundInstance* ended=NULL;
				while (i!=instances.end())
				{
					ended=*i;
					if (ended->CanAutoDelete()) break;
					++i;
				}
				if (i==instances.end()) break;
				instances.erase(i);
				if (ended) { ended->~VfxSoundInstance(); ::operator delete(ended); }
			}
		}
	}
	void SoundInstanceManager::RegisterInstance(VfxSoundInstance* instance)
	{
		InstanceGuard guard(lock);
		instances.push_back(instance);
	}
	void SoundInstanceManager::UnregisterInstance(VfxSoundInstance* instance)
	{
		lock.Lock();
		instances.remove(instance);
		lock.Unlock();
	}
	void SoundInstanceManager::SetListenerPosition(D3DXVECTOR3 position)
	{
		listenerPosition.x=position.x; listenerPosition.y=position.y; listenerPosition.z=position.z;
	}
	int SoundInstanceManager::RequestPriority(VfxSoundInstance& instance)
	{
		D3DXVECTOR3 captured(0,0,0);
		instance.GetPosition(&captured);
		const LONG full=LONG(instance.preset->fullSoundRadius);
		const LONG outer=LONG(instance.preset->noSoundRadius);
		const LONG range=LONG(DWORD(outer)-DWORD(full));
		if (range<=0) return 0;
		const double x=double(captured.x)-listenerPosition.x;
		const double y=double(captured.y)-listenerPosition.y;
		const double z=double(captured.z)-listenerPosition.z;
		const float distanceValue=float(std::sqrt((z*z+y*y)+x*x));
		const LONG distance=LONG(distanceValue);
		if (distance<=full) return 100;
		if (distance>=outer) return 0;
		const LONG numerator=LONG((DWORD(full)-DWORD(distance))*100u);
		const LONG result=LONG(DWORD(numerator/range)+100u);
		return result<0 ? 0 : result>100 ? 100 : result;
	}
	int SoundInstanceManager::CountActiveGroup(LONG id)
	{
		Resources::ResourceLock& held=lock;
		int result=0;
		held.Lock();
		std::list<VfxSoundInstance*>::iterator last=instances.end();
		for (std::list<VfxSoundInstance*>::iterator i=instances.begin();i!=last;++i)
			if ((*i)->group->id==id && (*i)->HasActiveSound()) ++result;
		held.Unlock();
		return result;
	}
	void SoundInstanceManager::EvictOldestGroup(LONG id,VfxSoundInstance& incoming)
	{
		DWORD oldest=AudioBackend::g_System->now; //Captured before acquiring this manager's lock.
		VfxSoundInstance* selected=NULL;
		InstanceGuard guard(lock);
		const std::list<VfxSoundInstance*>::iterator last=instances.end();
		for (std::list<VfxSoundInstance*>::iterator i=instances.begin();i!=last;++i)
		{
			VfxSoundInstance* instance=*i;
			if (instance!=&incoming && instance->group->id==id && instance->HasActiveSound()
				&& instance->GetStartedAt()<oldest)
			{ selected=instance; oldest=instance->GetStartedAt(); }
		}
		if (selected) selected->Retire();
	}
	int SoundInstanceManager::MixVolume(VfxSoundInstance& incoming)
	{
		MasterSoundGroups* groups=AudioBackend::g_System->groups; //Capture before lock.
		int result=100;
		lock.Lock();
		SoundGroup* incomingGroup=incoming.group;
		const LONG minimum=incomingGroup->minimumVolume;
		const std::list<VfxSoundInstance*>::iterator last=instances.end();
		for (std::list<VfxSoundInstance*>::iterator i=instances.begin();i!=last;++i)
		{
			VfxSoundInstance* instance=*i;
			if (instance->HasActiveSound() && instance!=&incoming)
			{
				const LONG source=instance->group->id;
				const LONG target=incomingGroup->id;
				if (source==target)
				{
					const DWORD sourceStarted=instance->GetStartedAt();
					const DWORD incomingStarted=incoming.GetStartedAt();
					if (sourceStarted<=incomingStarted) goto NoReduction;
				}
				result=LONG(DWORD(result)-DWORD(groups->MixingAmount(source,target)));
			NoReduction:
				if (result<=minimum) { result=minimum; break; }
			}
		}
		lock.Unlock();
		return result;
	}
}

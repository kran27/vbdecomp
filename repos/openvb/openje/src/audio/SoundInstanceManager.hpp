#pragma once

#include "resources/ResourceLock.hpp"
#include <d3dx8.h>
#include <list>

namespace JE
{
	class VfxSoundInstance;

	class SoundInstanceManager
	{
	public:
		SoundInstanceManager(); //52D080, list then lock then listener stores.
		~SoundInstanceManager(); //52CE60, borrowed nodes only.
		int Initialize(); //Shared4080B0, ordinary no-argument API; folded ownership unresolved.
		void Cleanup(); //52D3B0, no lock acquisition.
		void Update(); //52D190.
		void RegisterInstance(VfxSoundInstance* instance); //52E260.
		void UnregisterInstance(VfxSoundInstance* instance); //52CC70.
		void SetListenerPosition(D3DXVECTOR3 position); //52C460; three raw words by value.
		int RequestPriority(VfxSoundInstance& instance); //52C480.
		int CountActiveGroup(LONG id); //52C760.
		void EvictOldestGroup(LONG id,VfxSoundInstance& incoming); //52C7D0.
		int MixVolume(VfxSoundInstance& incoming); //52C890.

		std::list<VfxSoundInstance*> instances;
		Resources::ResourceLock lock;
		D3DXVECTOR3 listenerPosition;
	private:
		SoundInstanceManager(const SoundInstanceManager&);
		SoundInstanceManager& operator=(const SoundInstanceManager&);
	};
}

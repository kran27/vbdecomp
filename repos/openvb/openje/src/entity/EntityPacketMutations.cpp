#include "Entity.hpp"
#include "Inventory.hpp"
#include "graphics/VfxVeg.hpp"
#include "VisualEffect.hpp"
#include "WorldCollections.hpp"
#include <new>

	void Journal::AddEntry(DWORD entry,DWORD value) //5A0610, deliberately nonvirtual.
	{
		JE::JournalEntry* record=new JE::JournalEntry;
		record->value8=entry;
		record->value4=value;
		entries.push_back(record);
	}
namespace JE
{
	Entity* Inventory::FindByID(DWORD id) //55FEA0.
	{
		if (!id) return NULL;
		std::list<Entity*>::iterator i=items.begin();
		const std::list<Entity*>::iterator end=items.end();
		for (;i!=end;++i)
		{
			Entity* entity=*i;
			if (entity && entity->entityID==id) return entity;
		}
		return NULL;
	}
	BYTE Entity::RemoveVisualEffectByID(DWORD id,BYTE force) //54F270.
	{
		BYTE removed=0;
		std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator i=activeEffects.begin();
		while (i!=activeEffects.end())
		{
			EntityEffectBinding& binding=i->second;
			if (binding.id==id)
			{
				if (!force && binding.instance)
				{
					binding.instance->Stop();
					EntityEffectBinding pending; //Native insertion copies an unwritten payload before replacement.
					std::map<VFX_VEGInstance*,EntityEffectBinding>::value_type entry(binding.instance,pending);
					std::map<VFX_VEGInstance*,EntityEffectBinding>::iterator destination=inactiveEffects.insert(entry).first;
					destination->second=binding;
				}
				else
				{
					//Unlike pointer-removal54F150 this path does not Reset the VEG.
					if (binding.object && binding.deleteObject) delete binding.object;
					if (binding.instance && binding.releaseInstance) GetVfxVegCache().Release(binding.instance);
					if (binding.id)
					{
						std::map<DWORD,VisualEffect*>::iterator found=effectIDs.find(binding.id);
						if (found!=effectIDs.end()) effectIDs.erase(found);
					}
				}
				i=activeEffects.erase(i);
				removed=1;
			}
			else ++i;
		}
		return removed;
	}
}

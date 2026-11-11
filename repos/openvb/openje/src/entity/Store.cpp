#include "Store.hpp"
#include "WorldCollections.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>

namespace JE
{
	void StoreManager::ClearStores()
	{
		for (std::list<CStore*>::iterator i = stores.begin(); i != stores.end(); ++i)
			if (*i) delete *i;
		stores.clear(); // Original skips NULL payloads, then clears list nodes.
	}

	DWORD CStore::ReadStoreDword(const BYTE*& cursor) //Existing proprietary5A1E50 cursor policy.
	{
		DWORD value;
		std::memcpy(&value, cursor, sizeof(value));
		cursor += sizeof(value);
		return value;
	}

	const char* CStore::ReadStoreString(const BYTE*& cursor) //5A1E60, including terminal NUL.
	{
		const char* value = reinterpret_cast<const char*>(cursor);
		while (*cursor) ++cursor;
		++cursor;
		return value;
	}
}

	CStoreItem::CStoreItem() //5A2330.
	{
		name = "";
		value20 = 0;
		value24 = 0;
		value28 = 0;
		value2C = 0;
		value30 = 0;
	}

	CStoreItem::~CStoreItem() {} //5A1F80, real string supplies destruction.

namespace JE
{
	CStore::CStore() //5A2280.
	{
		value20 = 0;
		value24 = 0;
		value28 = 0;
		value2C = 0;
		name = "";
		items.clear();
	}

	CStore::~CStore() //5A2060.
	{
		Clear();
	}

	void CStore::Clear() //5A1F10.
	{
		for (std::list<CStoreItem*>::iterator i = items.begin(); i != items.end(); ++i)
			if (*i) delete *i;
		items.clear();
	}

	void CStore::Read(const char* resourceName) //5A24A0.
	{
		Clear();
		Resources::ResourceData* resource =
			Resources::g_pResourceSystem->Demand(resourceName, 1);
		if (!resource) return;
		name = resourceName;
		const BYTE* cursor = static_cast<const BYTE*>(resource->data);
		value20 = ReadStoreDword(cursor);
		value24 = ReadStoreDword(cursor);
		value28 = ReadStoreDword(cursor);
		value2C = static_cast<LONG>(ReadStoreDword(cursor));
		for (LONG index = 0; index < value2C; ++index)
		{
			CStoreItem* item = new CStoreItem;
			{
				std::string itemName(ReadStoreString(cursor));
				item->name = itemName;
			}
			item->value20 = ReadStoreDword(cursor);
			item->value24 = ReadStoreDword(cursor);
			item->value28 = ReadStoreDword(cursor);
			item->value2C = ReadStoreDword(cursor);
			item->value28 = ReadStoreDword(cursor); //5A2642 overwrites third, not value30.
			items.push_back(item);
		}
		//Only the normal path releases the resource. Published/raw records are
		//not transactionally rolled back on string, allocation or list failure.
		Resources::g_pResourceSystem->ReleaseResource(resource);
	}

	void CStore::Add(const char* itemName, DWORD word20, DWORD word24,
		DWORD word28, DWORD word2C, DWORD word30) //5A26B0, always appends a fresh item.
	{
		CStoreItem* item = new CStoreItem;
		{
			std::string text(itemName);
			item->name = text;
		}
		item->value20 = word20;
		item->value24 = word24;
		item->value28 = word28;
		item->value2C = word2C;
		item->value30 = word30;
		items.push_back(item);
	}

	void CStore::Remove(const char* itemName, DWORD ignoredQuantity) //5A1E80.
	{
		(void)ignoredQuantity; //Original RET8 accepts it but never reads it.
		for (std::list<CStoreItem*>::iterator i = items.begin(); i != items.end(); ++i)
		{
			CStoreItem* item = *i;
			if (item && !_stricmp(itemName, item->name.c_str()))
			{
				items.erase(i);
				delete item;
				return;
			}
		}
	}
}

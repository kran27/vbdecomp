#include "WorldCollections.hpp"
#include "resources/ResourceSystem.hpp"
#include <cstring>

DWORD Encyclopedia::ReadEncyclopediaDword(const BYTE*& cursor) //5A1E50.
{
	DWORD value;
	std::memcpy(&value, cursor, sizeof(value));
	cursor += sizeof(value);
	return value;
}
const char* Encyclopedia::ReadEncyclopediaString(const BYTE*& cursor) //5A1E60.
{
	const char* value = reinterpret_cast<const char*>(cursor);
	while (*cursor) ++cursor;
	++cursor;
	return value;
}
void Encyclopedia::Read(const char* name) //58F750.
{
	JE::Resources::ResourceData* resource = JE::Resources::g_pResourceSystem->Demand(name, 1);
	if (!resource) return;
	const BYTE* cursor = static_cast<const BYTE*>(resource->data);
	LONG remaining = static_cast<LONG>(ReadEncyclopediaDword(cursor));
	while (remaining > 0)
	{
		JE::EncyclopediaEntry* entry = new JE::EncyclopediaEntry;
		entry->id = ReadEncyclopediaDword(cursor);
		entry->value4 = ReadEncyclopediaDword(cursor);
		entry->flag8 = ReadEncyclopediaDword(cursor) == 0;
		{
			std::string text(ReadEncyclopediaString(cursor));
			entry->valueC = text;
		}
		LONG count = static_cast<LONG>(ReadEncyclopediaDword(cursor));
		while (count > 0)
		{
			entry->values34.push_back(ReadEncyclopediaDword(cursor));
			--count;
		}
		count = static_cast<LONG>(ReadEncyclopediaDword(cursor));
		while (count > 0)
		{
			entry->values28.push_back(ReadEncyclopediaDword(cursor));
			--count;
		}
		entries.push_back(entry);
		--remaining;
	}
	//The original releases only on the normal path, without a failure guard
	//for allocation/string/list exceptions or partially constructed records.
	JE::Resources::g_pResourceSystem->ReleaseResource(resource);
}

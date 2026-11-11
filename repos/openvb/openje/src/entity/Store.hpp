#pragma once

#include <windows.h>
#include <cstddef>
#include <list>
#include <string>

	class CStoreItem
	{
	public:
		CStoreItem(); //5A2330.
		virtual ~CStoreItem(); //5A1F80; real string lifetime only.

		std::string name; //Original+4.
		DWORD value20, value24, value28, value2C, value30;
	};

namespace JE
{
	typedef ::CStoreItem CStoreItem;

	class CStore
	{
	public:
		CStore(); //5A2280.
		virtual ~CStore(); //5A2060.
		virtual void Read(const char* resourceName); //5A24A0, vslot+4.
		void Clear(); //5A1F10, retains name and serialized words.
		void Add(const char* itemName, DWORD value20, DWORD value24,
			DWORD value28, DWORD value2C, DWORD value30); //5A26B0.
		void Remove(const char* itemName, DWORD ignoredQuantity); //5A1E80.

		std::string name; //Original+4.
		DWORD value20, value24, value28;
		LONG value2C; //Signed serialized item count, independent of list size.
		std::list<CStoreItem*> items; //Original+30; owns nonNULL payloads.
	private:
		DWORD ReadStoreDword(const BYTE*& cursor); //5A1E50; nonvirtual, receiver unused.
		const char* ReadStoreString(const BYTE*& cursor); //5A1E60; nonvirtual.
	};

#if defined(_MSC_VER) && _MSC_VER == 1300 && defined(_M_IX86)
	typedef char StoreItemOriginalSize[(sizeof(CStoreItem) == 0x34) ? 1 : -1];
	typedef char StoreItemOriginalName[(offsetof(CStoreItem, name) == 4) ? 1 : -1];
	typedef char StoreItemOriginalWords[(offsetof(CStoreItem, value20) == 0x20) ? 1 : -1];
	typedef char StoreItemOriginalTail[(offsetof(CStoreItem, value30) == 0x30) ? 1 : -1];
	typedef char StoreOriginalSize[(sizeof(CStore) == 0x3C) ? 1 : -1];
	typedef char StoreOriginalName[(offsetof(CStore, name) == 4) ? 1 : -1];
	typedef char StoreOriginalWords[(offsetof(CStore, value20) == 0x20) ? 1 : -1];
	typedef char StoreOriginalCount[(offsetof(CStore, value2C) == 0x2C) ? 1 : -1];
	typedef char StoreOriginalItems[(offsetof(CStore, items) == 0x30) ? 1 : -1];
#endif
}

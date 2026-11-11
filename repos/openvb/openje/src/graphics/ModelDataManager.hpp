#pragma once

#include <windows.h>
#include <deque>
#include <map>

class Gfx_ModelData526620;
class Gfx_ModelDataManager;
class Gfx_ModelInstance;
extern Gfx_ModelDataManager* g_ModelDataManager; // Original singleton0x70C96C.

// Original publication precedes the real container members; clearing follows
// their reverse destruction, including a compiler-owned cleanup path.
class Gfx_ModelDataManagerPublication
{
protected:
	Gfx_ModelDataManagerPublication();
	~Gfx_ModelDataManagerPublication();
};

// Descriptive source name for the nonvirtual36-byte owner at0x5CB510.
// Mapped payloads are actual owned ModelData objects, destroyed in map order.
class Gfx_ModelDataManager : private Gfx_ModelDataManagerPublication
{
	friend class Gfx_ModelDataManagerPublication;
public:
	Gfx_ModelDataManager(); // 0x5CB510; writes nextHandle after member construction.
	~Gfx_ModelDataManager(); // 0x5CB5C0.
	DWORD Register(Gfx_ModelData526620* data); // 0x5CB2A0.
	DWORD References(DWORD* handle); // 0x5C08B0.
	DWORD Release(DWORD* handle); // 0x5CB6C0; ordinary ModelData destruction.
	Gfx_ModelInstance* CreateInstance(DWORD handle); // 0x5CAC00.

	DWORD nextHandle; // +00.
	std::deque<DWORD> freeHandles; // +04..17; owned storage, recycled handle values.
	std::map<DWORD, Gfx_ModelData526620*> entries; // +18..23; owns mapped payloads.

private:
	DWORD AllocateHandle(); //50B560, shared allocation policy; zero on exhaustion.
	void RecycleHandle(DWORD* handle); //5CB5A0; queue nonzero handle then zero caller.
	Gfx_ModelDataManager(const Gfx_ModelDataManager&);
	Gfx_ModelDataManager& operator=(const Gfx_ModelDataManager&);
};

inline Gfx_ModelDataManagerPublication::Gfx_ModelDataManagerPublication()
{
	g_ModelDataManager = static_cast<Gfx_ModelDataManager*>(this);
}

inline Gfx_ModelDataManagerPublication::~Gfx_ModelDataManagerPublication()
{
	g_ModelDataManager = NULL;
}

// Ordinary payload destruction5CB370 followed by caller-owned storage deletion.
void __fastcall Gfx_DestroyModelData526620(Gfx_ModelData526620* payload);

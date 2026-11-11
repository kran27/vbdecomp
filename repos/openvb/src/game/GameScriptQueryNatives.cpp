#include "GameScriptNatives.hpp"
#include "GameScriptObjectSearch.hpp"
#include "entity/Entity.hpp"
#include "entity/World.hpp"
#include "entity/WorldCollections.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include "../startup/FrameLookupCaches.hpp"
#include <mbctype.h>
#include <mbstring.h>
#include <new>
#include <stdexcept>

namespace
{
	unsigned char FoldByte(unsigned char value) //4A1210, real CRT character helpers.
	{
		return value<128 && _ismbcupper(static_cast<signed char>(value))
			? static_cast<unsigned char>(_mbctolower(static_cast<signed char>(value))) : value;
	}
	const unsigned char* FindFoldedByte(const unsigned char* text,unsigned char value) //4A1240.
	{
		while (*text && FoldByte(*text)!=value) ++text;
		return *text || !value ? text : NULL;
	}
	int MatchPattern(const unsigned char* pattern,const unsigned char* text) //4A1280.
	{
		while (*pattern)
		{
			if (*pattern=='*')
			{
				while (*pattern=='*' || *pattern=='?')
				{
					if (*pattern=='?' && !*text++) return 1;
					++pattern;
				}
				//The escaped first byte is not folded here in the original4A1330.
				unsigned char first=*pattern=='\\' && pattern[1] ? pattern[1] : FoldByte(*pattern);
				text=FindFoldedByte(text,first);
				while (text)
				{
					if (!MatchPattern(pattern,text)) return 0;
					text=FindFoldedByte(text+1,first);
				}
				//Keep the exhausted search pointer, including NULL, at the native loop edge.
				continue;
			}
			if (*pattern=='?')
			{
				if (!*text++) return 1;
				++pattern;
			}
			else
			{
				unsigned char value=*pattern++;
				if (value=='\\')
				{
					if (!*pattern) return 1;
					value=*pattern++;
				}
				if (FoldByte(value)!=FoldByte(*text++)) return 1;
			}
		}
		return static_cast<signed char>(FoldByte(*pattern))-static_cast<signed char>(FoldByte(*text));
	}
	BYTE MatchesPattern(const unsigned char* pattern,const unsigned char* text) //4A13B0, AL result.
	{
		return MatchPattern(pattern,text)==0;
	}
	DWORD MatchEntities(DWORD* handle,const char* pattern) //55A030, live creation list only.
	{
		F3::FrameLookupCaches::ObjectList* results=new F3::FrameLookupCaches::ObjectList;
		for (std::list<JE::Entity*>::iterator current=JE::g_createdEntities.begin();
			current!=JE::g_createdEntities.end(); ++current)
		{
			void* entityValue=*current; //Captured before the predicate, then appended unchanged.
			JE::Entity* entity=static_cast<JE::Entity*>(entityValue);
			if (MatchesPattern(reinterpret_cast<const unsigned char*>(pattern),
				reinterpret_cast<const unsigned char*>(entity->displayName.c_str())))
				results->push_back(entityValue);
		}
		return F3::PublishScriptObjectList(handle,results);
	}
}

namespace F3
{
	DWORD nextObjectList=100; //Original6FEEA0, shared by both native query paths.
	cell AMX_NATIVE_CALL NativeIsIdle(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"IsIdle")) return 0;
		JE::Entity* entity=NULL;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1)) return 0;
		return entity->IsIdle()==1;
	}
	cell AMX_NATIVE_CALL NativeIsDead(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"IsDead")) return 0;
		JE::Entity* entity=NULL;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1)) return 1;
		return entity->IsDead()==1;
	}
	cell AMX_NATIVE_CALL NativeIsInParty(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"IsInParty")) return 0;
		JE::Entity* entity=NULL;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1)) return 0;
		JE::Party* party=JE::GetWorldInstance()->GetParty();
		return party->Contains(entity)==1;
	}
	cell AMX_NATIVE_CALL NativeGetType(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,1,"GetType")) return 0;
		JE::Entity* entity;
		if (!ResolveScriptEntity(vm,DWORD(parameters[1]),&entity,1))
		{
			LogScriptError(vm,"GetType() is unable to find the source entity.");
			return 0;
		}
		return cell(entity->entityClass);
	}
	cell AMX_NATIVE_CALL NativeMatchEntity(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,2,"MatchEntity")) return 0;
		char pattern[260];
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[2],pattern,260))
		{
			LogScriptError(vm,"MatchEntity() unable to retrieve the entity name.");
			return 0;
		}
		DWORD handle=0;
		DWORD count=MatchEntities(&handle,pattern);
		if (count)
		{
			cell* destination;
			if (amx_GetAddr(vm,parameters[1],&destination))
			{
				LogScriptError(vm,"MatchEntity() was unable to resolve list handle address.");
				return 0; //Original retains the already-created cache after this failure.
			}
			*destination=cell(handle);
		}
		return cell(count); //An empty result leaves the caller's handle cell unchanged.
	}
	cell AMX_NATIVE_CALL NativeGetObject(AMX* vm,cell* parameters)
	{
		if (!VerifyScriptParameters(vm,parameters,2,"GetObject")) return 0;
		const DWORD handle=DWORD(parameters[1]);
		const DWORD index=DWORD(parameters[2]);
		return cell(GetFrameObjectId(handle,index));
	}
}

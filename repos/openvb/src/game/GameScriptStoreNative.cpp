#include "GameScriptNatives.hpp"
#include "scripting/ScriptEnvironment.hpp"
#include <new>

namespace JE { namespace ScriptServices { BYTE ReadNativeStringLength(AMX*, cell, int&); } }


namespace F3
{
	cell AMX_NATIVE_CALL NativeStartStore(AMX* vm,cell* parameters) //5C21A0.
	{
		if (!VerifyScriptParameters(vm,parameters,1,"StartStore")) return 0;
		int length;
		if (!JE::ScriptServices::ReadNativeStringLength(vm,parameters[1],length))
		{
			LogScriptError(vm,"StartStore() is unable to retrieve the length of the store name.");
			return 0;
		}
		//Original allocates exactly length, then passes the same insufficient capacity.
		char* text = static_cast<char*>(::operator new(DWORD(length)));
		if (!JE::ScriptServices::ReadNativeString(vm,parameters[1],text,DWORD(length)))
		{
			::operator delete(text);
			LogScriptError(vm,"StartStore() is unable to retrieve the store name.");
			return 0;
		}
		::operator delete(text);
		return 1; //No Store, Server or UI call exists after the original successful read.
	}
}

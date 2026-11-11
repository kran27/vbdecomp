#pragma once

#include <windows.h>
#include <map>
#include <string>
#include <vector>
#include "amx.h"

namespace JE
{
	class DebugToolsInterface;
	namespace ScriptDebug
	{
		struct Variable
		{
			std::string name;
			cell address;
			LONG type, storageClass, dimensions, length0, length1;
			BYTE floating;
			Variable() {} // Original operator[] temporary leaves scalar members unwritten.
		};
		typedef std::map<cell, Variable> VariableMap;
		typedef std::map<LONG, std::string> FileMap;

		void DisplayState(AMX* vm, BYTE paused, BYTE previousLine); //5A6520.
		void EnableControls(DebugToolsInterface* controls, BYTE enabled); //Typed call adapter to actual owner.
		void ClearVariables(void* variables); // Actual reused map clear, not proprietary progress.
		void UnloadMaps(AMX* vm, void*& fileStack); // Original FTBL/VSTK share one output local.
		BYTE IsFloatName(const std::string& name); //5A4910.
	}
}

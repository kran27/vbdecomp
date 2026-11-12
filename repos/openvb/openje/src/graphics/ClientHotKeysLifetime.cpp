#include "graphics/ClientInput.hpp"
#include "startup/ConfigurationObject.hpp"

namespace JE
{
	ClientHotKeys::~ClientHotKeys()
	{
		//599CD0 saves once after entering the base destructor, before map
		//destruction. Input and Console are borrowed.
		Save();
	}

	void ClientHotKeys::Save() //599A90.
	{
		Configuration::Settings& settings = *F3::GetSettingsObject();
		if (!bindings.empty())
		{
			Configuration::Values& section = settings["HotKeys"]; //4E0470.
			section.clear(); //Actual settings value-map destruction, real STL.
			std::string text; //One owner reused across all bindings, as in native599B6B.
			for (std::map<LONG, std::string>::const_iterator it = bindings.begin();
				it != bindings.end(); ++it)
			{
				LONG key = it->first;
				DWORD modifiers = (key >> 20) & 15;
				text = (key & 0xF0000) == 0x10000 ? "-" : "+";
				if ((modifiers & 1) != 0)
					text += "Ctrl+";
				if ((modifiers & 2) != 0)
					text += "Shift+";
				//599BA0 masks the scan before actual Input virtual+0C KeyName.
				//No Alt prefix or fallback text is supplied by the original.
				text += input.KeyName(key & 0xFFFF);
				section[text] = it->second; //4DFEB0, then ordinary string assignment.
			}
		}
		//An empty binding map leaves the existing HotKeys settings untouched.
		//No Configuration::WriteFile, SaveSettingsFile, or filesystem operation.
	}

}

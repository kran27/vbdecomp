#include "ClientInput.hpp"
#include "network/CommandProcessor.hpp"
#include "startup/ConfigurationObject.hpp"
#include <cstring>
#include <stdexcept>

namespace JE
{
	namespace
	{
		LONG BindingKey(DWORD scan, DWORD subtype, DWORD modifiers)
		{ return (scan & 0xFFFF) | ((DWORD(subtype & 15) | ((modifiers & 15) << 4)) << 16); }
	}

	ClientHotKeys::ClientHotKeys(Input::Win32Input& owner, ConsoleCommandProcessor* processor)
		: input(owner), console(processor) {}
	void ClientHotKeys::Initialize() //5998C0.
	{
		const Configuration::Settings& settings = *F3::GetSettingsObject();
		Configuration::Settings::const_iterator section = settings.find("HotKeys");
		if (section == settings.end()) return;
		for (Configuration::Values::const_iterator it = section->second.begin(); it != section->second.end(); ++it)
		{
			const char* name = it->first.c_str();
			BYTE subtype = 0;
			DWORD modifiers = 0;
			if (it->first.size() > 1 && (*name == '-' || *name == '+'))
			{ subtype = *name == '-' ? 1 : 0; ++name; }
			for (int pass = 0; pass != 2; ++pass)
			{
				if (!std::strncmp(name, "Ctrl+", 5)) { modifiers |= 1; name += 5; }
				if (!std::strncmp(name, "Shift+", 6)) { modifiers |= 2; name += 6; }
			}
			const DWORD scan = FindKey(name);
			if (scan) Bind(scan, subtype, modifiers, it->second.c_str());
		}
	}
	BYTE ClientHotKeys::Bind(DWORD scan, DWORD subtype, DWORD modifiers, const char* command)
	{
		bindings[BindingKey(scan, subtype, modifiers)] = command;
		return 1;
	}
	void ClientHotKeys::Unbind(DWORD scan, DWORD subtype, DWORD modifiers)
	{
		std::map<LONG,std::string>::iterator found = bindings.find(BindingKey(scan, subtype, modifiers));
		if (found != bindings.end())
			bindings.erase(found);
	}
	BYTE ClientHotKeys::FindBinding(DWORD& scan, DWORD& subtype, DWORD& modifiers,
		const char* command) const
	{
		for (std::map<LONG,std::string>::const_iterator i = bindings.begin();
			i != bindings.end(); ++i)
		{
			if (!_stricmp(command, i->second.c_str()))
			{
				scan = i->first & 0xFFFF;
				subtype = (i->first >> 16) & 15;
				modifiers = (i->first >> 20) & 15;
				return 1;
			}
		}
		return 0;
	}
	DWORD ClientHotKeys::FindKey(const char* name) const
	{
		return input.FindKey(name);
	}
	BYTE ClientHotKeys::Handle(const Input::InputEvent& event) const //599720.
	{
		if (event.category) return 0;
		const LONG key = (DWORD(event.x) & 0xFFFF) |
			((event.subtype & 15 | ((event.modifiers & 15) << 4)) << 16);
		std::map<LONG,std::string>::const_iterator found = bindings.find(key);
		if (found == bindings.end()) return 0;
		console->Queue(found->second.c_str()); //Captured original+8 owner; required, not an empty fallback.
		return 1;
	}

}

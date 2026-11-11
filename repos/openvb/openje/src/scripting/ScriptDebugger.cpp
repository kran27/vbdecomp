#include <winsock2.h>
#include "ScriptDebugger.hpp"
#include "ScriptSmall.hpp"
#include "gui/DebugToolsInterface.hpp"
#include "gui/DebugWindow.hpp"
#include "network/EngineClient.hpp"
#include "network/EngineServer.hpp"
#include "startup/NetworkComponent.hpp"
#include "diagnostics/DebugOutput.hpp"
#include <fstream>
#include <deque>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdexcept>

namespace
{
	const long fileTag = 0x4C425446, variablesTag = 0x50414D56;
	const long stackTag = 0x4B545356, ownerTag = 0x464C534D;
	const long stateTag = 0x54415453, lineTag = 0x454E494C;
	cell currentSymbol; // Original70C720, shared across all debugger callbacks.

	JE::Client* DebugClient()
	{
		return static_cast<JE::Client*>(F3::g_pNetworkInstance->client);
	}
	JE::Server* DebugServer()
	{
		return static_cast<JE::Server*>(F3::g_pNetworkInstance->server);
	}
	class SourceCache
	{
	public:
		std::string FileName(std::string path); //5A7E20.
		BYTE Load(std::string path, std::string key); //5A7980.
		BYTE Line(std::string key, DWORD number, std::string& output); //5A5D60.
	private:
		std::map<std::string, std::vector<std::string> > files;
	};
	SourceCache sourceCache; // Original70C728 is an actual static map object.

	std::string SourceCache::FileName(std::string path)
	{
		std::string key;
		int separator = static_cast<int>(path.find_last_of('\\', path.length()));
		if (separator >= 0) key = path.substr(separator + 1, path.length());
		else key = path;
		std::map<std::string, std::vector<std::string> >::iterator found = files.find(key);
		if (found != files.end()) return key;
		if (Load(path, key)) return key;
		return std::string("");
	}

	BYTE SourceCache::Load(std::string path, std::string key)
	{
		char message[4096];
		char line[4096];
		JE::DebugWindow* window = DebugClient()->child0x8C;
		std::string filename;
		if (path == key) filename = "override\\" + path;
		else filename = path;
		std::ifstream input;
		input.open(filename.c_str(), std::ios::in);
		if (!input.is_open())
		{
			if (window)
			{
				sprintf(message, "Unable to open file: %s", filename.c_str());
				window->Write(std::string(message), 128, 0, 0);
			}
			return 0;
		}
		input.seekg(0, std::ios::beg);
		std::vector<std::string> lines;
		// The original tests eofbit alone, including its final empty read.
		// A non-EOF failbit does not terminate this loop.
		while (!input.eof())
		{
			input.getline(line, sizeof(line), '\n');
			lines.push_back(std::string(line));
		}
		files[key] = lines;
		return 1;
	}

	BYTE SourceCache::Line(std::string key, DWORD number, std::string& output)
	{
		JE::DebugWindow* window = DebugClient()->child0x8C;
		std::map<std::string, std::vector<std::string> >::iterator found = files.find(key);
		char message[4096];
		if (found == files.end())
		{
			if (window)
			{
				sprintf(message, "File not found in file map: %s", key.c_str());
				window->Write(std::string(message), 128, 0, 0);
			}
			return 0;
		}
		std::vector<std::string> lines = found->second; // Original copies before checking range.
		if (number < 1 || number > lines.size())
		{
			if (window)
			{
				sprintf(message, "Line number, %d, is out of range for file: %s", number, key.c_str());
				window->Write(std::string(message), 128, 0, 0);
			}
			return 0;
		}
		output = lines[number - 1];
		return 1;
	}
}

namespace JE
{
	namespace ScriptDebug
	{
		BYTE IsFloatName(const std::string& name)
		{
			if (!name.compare(0, 1, "f")) return 1;
			else if (!name.compare(0, 3, "g_f")) return 1;
			else if (!name.compare(0, 3, "p_f")) return 1;
			else if (!name.compare(0, 2, "af")) return 1;
			else return name.compare(0, 4, "g_af") == 0;
		}

		void ClearVariables(void* variables)
		{
			static_cast<VariableMap*>(variables)->clear();
		}

		void UnloadMaps(AMX* vm, void*& stack)
		{
			void* variables;
			// Original5A7349/5A7379 skip deletion when GetUserData succeeds.
			// Failed lookups leave these output locals unwritten. Preserve that
			// error-branch quirk, including successful-lookup leaks. The stack
			// output aliases the caller's earlier FTBL output on lookup failure.
			if (amx_GetUserData(vm, variablesTag, &variables))
				delete static_cast<VariableMap*>(variables);
			if (amx_GetUserData(vm, stackTag, &stack))
				// Original5A7385 ->5A55F0 ->5A5130 uses the real deque<string>
				// destructor despite the debugger publishing VariableMap as VSTK.
				delete static_cast<std::deque<std::string>*>(stack);
		}

		void EnableControls(DebugToolsInterface* controls, BYTE enabled)
		{
			controls->EnableControls(enabled);
		}

		void DisplayState(AMX* vm, BYTE paused, BYTE previousLine)
		{
			char integer0[128], integer1[128];
			char buffer[4096];
			char floating0[1024], floating1[1024];
			Client* client = DebugClient();
			DebugWindow* variables = client->child0x90;
			DebugWindow* debug = client->child0x8C;
			if (!variables || !debug) return;
			std::vector<std::string> lines;
			VariableMap* table;
			// Original5A65A5 falls through to globals; local cleanups own states6..10.
			if (!DebugServer()->field0x70ED)
			{
				if (amx_GetUserData(vm, variablesTag, reinterpret_cast<void**>(&table))) return;
				for (VariableMap::iterator i = table->begin(); i != table->end(); ++i)
				{
					cell address = i->first;
					cell* value = NULL;
					if (!amx_GetAddr(vm, address, &value))
					{
						const Variable& variable = i->second;
						switch (variable.type)
						{
						case 1:
							if (variable.floating)
							{
								cell bits = *value;
								double number = amx_ctof(bits);
								sprintf(buffer, "%-32s:%f", variable.name.c_str(), number);
							}
							else sprintf(buffer, "%-32s:%d", variable.name.c_str(), *value);
							lines.push_back(std::string(buffer));
							break;
						case 2:
						{
							cell* referenced = NULL;
							amx_GetAddr(vm, *value, &referenced);
							// The original passes a double to %d for a floating reference.
							if (variable.floating)
							{
								cell bits = *referenced;
								double number = amx_ctof(bits);
								sprintf(buffer, "(ref)%-32s:%d", variable.name.c_str(), number);
							}
							else sprintf(buffer, "(ref)%-32s:%d", variable.name.c_str(), *referenced);
							lines.push_back(std::string(buffer));
							break;
						}
						case 3:
						{
							LONG count = variable.length0;
							if (count > 30) count = 30;
							cell* element = NULL;
							sprintf(buffer, "%-32s:[", variable.name.c_str());
							std::string text(buffer);
							for (LONG n = 0; n < count; ++n)
							{
								if (n > 0) text += ", ";
								amx_GetAddr(vm, address, &element);
								if (variable.floating)
								{
									cell bits = *element;
									double number = amx_ctof(bits);
									sprintf(floating0, "%g", number);
									text += floating0;
								}
								else
								{
									_itoa(*element, integer1, 10);
									text += integer1;
								}
								address += 4;
							}
							text += "]";
							lines.push_back(text);
							break;
						}
						case 4:
						{
							LONG count = variable.length0;
							if (count > 30) count = 30;
							cell target = *value;
							cell* element = NULL;
							sprintf(buffer, "%-32s:[", variable.name.c_str());
							std::string text(buffer);
							for (LONG n = 0; n < count; ++n)
							{
								if (n > 0) text += ", ";
								amx_GetAddr(vm, target, &element);
								if (variable.floating)
								{
									cell bits = *element;
									double number = amx_ctof(bits);
									sprintf(floating1, "%g", number);
									text += floating1;
								}
								else
								{
									_itoa(*element, integer0, 10);
									text += integer0;
								}
								target += 4;
							}
							text += "]";
							lines.push_back(text);
							break;
						}
						default:
							sprintf(buffer, "Unknown Var Type: %s", variable.name.c_str());
							lines.push_back(std::string(buffer));
							break;
						}
					}
				}
			}
			else
			{
				if (paused)
				{
					if (amx_GetUserData(vm, stackTag, reinterpret_cast<void**>(&table))) return;
					for (VariableMap::iterator i = table->begin(); i != table->end(); ++i)
					{
						cell address = i->first;
						cell* value = NULL;
						int error = amx_GetAddr(vm, address, &value);
						if (error) sprintf(buffer, "GetAddr returned: %d", error);
						else
						{
							const Variable& variable = i->second;
							switch (variable.type)
							{
							case 1:
								if (variable.floating)
								{
									cell bits = *value;
									double number = amx_ctof(bits);
									sprintf(buffer, "%-32s:%f", variable.name.c_str(), number);
								}
								else sprintf(buffer, "%-32s:%d", variable.name.c_str(), *value);
								lines.push_back(std::string(buffer));
								break;
							case 2:
							{
								cell* referenced = NULL;
								amx_GetAddr(vm, *value, &referenced);
								// The original passes a double to %d for a floating reference.
								if (variable.floating)
								{
									cell bits = *referenced;
									double number = amx_ctof(bits);
									sprintf(buffer, "(ref)%-32s:%d", variable.name.c_str(), number);
								}
								else sprintf(buffer, "(ref)%-32s:%d", variable.name.c_str(), *referenced);
								lines.push_back(std::string(buffer));
								break;
							}
							case 3:
							{
								LONG count = variable.length0;
								if (count > 30) count = 30;
								cell* element = NULL;
								sprintf(buffer, "%-32s:[", variable.name.c_str());
								std::string text(buffer);
								for (LONG n = 0; n < count; ++n)
								{
									if (n > 0) text += ", ";
									amx_GetAddr(vm, address, &element);
									if (variable.floating)
									{
										cell bits = *element;
										double number = amx_ctof(bits);
										sprintf(floating1, "%g", number);
										text += floating1;
									}
									else
									{
										_itoa(*element, integer0, 10);
										text += integer0;
									}
									address += 4;
								}
								text += "]";
								lines.push_back(text);
								break;
							}
							case 4:
							{
								LONG count = variable.length0;
								if (count > 30) count = 30;
								cell target = *value;
								cell* element = NULL;
								sprintf(buffer, "%-32s:[", variable.name.c_str());
								std::string text(buffer);
								for (LONG n = 0; n < count; ++n)
								{
									if (n > 0) text += ", ";
									amx_GetAddr(vm, target, &element);
									if (variable.floating)
									{
										cell bits = *element;
										double number = amx_ctof(bits);
										sprintf(floating0, "%g", number);
										text += floating0;
									}
									else
									{
										_itoa(*element, integer1, 10);
										text += integer1;
									}
									target += 4;
								}
								text += "]";
								lines.push_back(text);
								break;
							}
							default:
								sprintf(buffer, "Unknown Var Type: %s", variable.name.c_str());
								lines.push_back(std::string(buffer));
								break;
							}
						}
					}
				}
				else lines.push_back(std::string(""));
			}
			// Original size() leaves existing rows visible when no values were added.
			if (lines.size()) variables->SetLines(lines);
			if (paused)
			{
				FileMap* files;
				if (amx_GetUserData(vm, fileTag, reinterpret_cast<void**>(&files))) return;
				std::string text;
				BYTE found;
				if (previousLine)
				{
					LONG file = vm->curfile;
					found = sourceCache.Line((*files)[file], vm->curline - 1, text);
				}
				else
				{
					LONG file = vm->curfile;
					found = sourceCache.Line((*files)[file], vm->curline, text);
				}
				if (found)
				{
					if (previousLine)
					{
						LONG file = vm->curfile;
						sprintf(buffer, "%s at %d:%s", (*files)[file].c_str(), vm->curline - 1, text.c_str());
					}
					else
					{
						LONG file = vm->curfile;
						sprintf(buffer, "%s at %d:%s", (*files)[file].c_str(), vm->curline, text.c_str());
					}
					debug->Write(std::string(buffer), 128, 128, 128);
				}
				else debug->Write(std::string("Error in retrieving code line to display."), 128, 0, 0);
			}
		}
	}

	int AMXAPI Script_Small::Debugger(AMX* vm) //5A8030.
	{
#if defined(_MSC_VER) && _MSC_VER == 1300
		typedef char OriginalVariableSize[(sizeof(ScriptDebug::Variable) == 56) ? 1 : -1];
		typedef char OriginalVariableAddress[(offsetof(ScriptDebug::Variable, address) == 28) ? 1 : -1];
		typedef char OriginalVariableFloat[(offsetof(ScriptDebug::Variable, floating) == 52) ? 1 : -1];
		typedef char OriginalPauseResult[(AMX_ERR_SLEEP == 12) ? 1 : -1];
#endif
		using namespace ScriptDebug;
		VariableMap* globals = NULL;
		VariableMap* locals = NULL;
		if (vm->flags & AMX_FLAG_BROWSE)
		{
			switch (vm->dbgcode)
			{
			case DBG_INIT:
				amx_SetUserData(vm, fileTag, new FileMap);
				amx_SetUserData(vm, variablesTag, new VariableMap);
				amx_SetUserData(vm, stackTag, new VariableMap);
				currentSymbol = 0;
				break;
			case DBG_FILE:
			{
				std::string file = sourceCache.FileName(std::string(vm->dbgname));
				if (file.empty()) file = vm->dbgname;
				FileMap* table;
				if (!amx_GetUserData(vm, fileTag, reinterpret_cast<void**>(&table)))
				{
					LONG fileId = vm->curfile;
					(*table)[fileId] = file;
				}
				break;
			}
			case DBG_SYMBOL:
				if (!amx_GetUserData(vm, variablesTag, reinterpret_cast<void**>(&globals))
					&& (vm->dbgparam & 255) != 9 && (vm->dbgparam & 255) != 10)
				{
					currentSymbol = vm->dbgaddr;
					// Original5A820A..5A830F captures a value key for every lookup.
					cell key = currentSymbol;
					(*globals)[key].name = vm->dbgname;
					key = currentSymbol;
					(*globals)[key].address = vm->dbgaddr;
					key = currentSymbol;
					(*globals)[key].type = vm->dbgparam & 255;
					key = currentSymbol;
					(*globals)[key].storageClass = vm->dbgparam >> 8;
					key = currentSymbol;
					(*globals)[key].dimensions = 0;
					key = currentSymbol;
					(*globals)[key].length0 = 0;
					key = currentSymbol;
					(*globals)[key].length1 = 0;
					key = currentSymbol;
					(*globals)[key].floating = IsFloatName(std::string(vm->dbgname));
				}
				break;
			case DBG_SRANGE:
				if (!amx_GetUserData(vm, variablesTag, reinterpret_cast<void**>(&globals)))
				{
					cell key = currentSymbol;
					if ((*globals)[key].type == 3)
					{
						key = currentSymbol;
						(*globals)[key].dimensions = vm->dbgaddr + 1;
						key = currentSymbol;
						if (!vm->dbgaddr) (*globals)[key].length0 = vm->dbgparam;
						else (*globals)[key].length1 = vm->dbgparam;
					}
					else LogDebugString("Script_Small::Debugger() Attempt to set SRANGE on non-array variable.");
				}
				break;
			}
			return 0;
		}

		switch (vm->dbgcode)
		{
		case DBG_LINE:
		{
			DWORD owner = 0;
			amx_GetUserData(vm, ownerTag, reinterpret_cast<void**>(&owner));
			if (owner != DebugServer()->field0x70E8) break;
			int state = 0;
			amx_GetUserData(vm, stateTag, reinterpret_cast<void**>(&state));
			if (state != 3) break;
			cell line = 0;
			amx_GetUserData(vm, lineTag, reinterpret_cast<void**>(&line));
			if (line == vm->curline) break;
			state = 2;
			amx_SetUserData(vm, stateTag, reinterpret_cast<void*>(state));
			line = vm->curline;
			amx_SetUserData(vm, lineTag, reinterpret_cast<void*>(line));
			ClientChild0x94* controls = DebugClient()->child0x94;
			if (controls) EnableControls(controls, 1);
			DisplayState(vm, 1, 0);
			return AMX_ERR_SLEEP; // Original5A84A6 returns12 to pause this execution.
		}
		case DBG_SYMBOL:
			if (!amx_GetUserData(vm, stackTag, reinterpret_cast<void**>(&locals))
				&& (vm->dbgparam & 255) != 9 && (vm->dbgparam & 255) != 10
				&& (vm->dbgparam >> 8) && (vm->dbgparam >> 8) != 2)
			{
				currentSymbol = vm->frm + vm->dbgaddr;
				// Preserve the independent snapshots, including before IsFloatName.
				cell key = currentSymbol;
				(*locals)[key].name = vm->dbgname;
				key = currentSymbol;
				(*locals)[key].address = currentSymbol;
				key = currentSymbol;
				(*locals)[key].type = vm->dbgparam & 255;
				key = currentSymbol;
				(*locals)[key].storageClass = vm->dbgparam >> 8;
				key = currentSymbol;
				(*locals)[key].dimensions = 0;
				key = currentSymbol;
				(*locals)[key].length0 = 0;
				key = currentSymbol;
				(*locals)[key].length1 = 0;
				key = currentSymbol;
				(*locals)[key].floating = IsFloatName(std::string(vm->dbgname));
			}
			break;
		case DBG_SRANGE:
			if (!amx_GetUserData(vm, stackTag, reinterpret_cast<void**>(&locals)))
			{
				cell key = currentSymbol;
				if ((*locals)[key].type == 3)
				{
					key = currentSymbol;
					(*locals)[key].dimensions = vm->dbgaddr + 1;
					key = currentSymbol;
					if (!vm->dbgaddr) (*locals)[key].length0 = vm->dbgparam;
					else (*locals)[key].length1 = vm->dbgparam;
				}
				else LogDebugString("Script_Small::Debugger() Attempt to set SRANGE on non-array LocalVar.");
			}
			break;
		case DBG_CLRSYM:
			if (!amx_GetUserData(vm, stackTag, reinterpret_cast<void**>(&locals)))
			{
				if (!locals->empty())
				{
					VariableMap::iterator i = locals->begin();
					cell stack = vm->stk;
					while (i != locals->end() && i->first < stack)
					{
						VariableMap::iterator removed = i++;
						locals->erase(removed);
					}
				}
			}
			break;
		case DBG_TERMINATE:
		{
			DWORD owner = 0;
			int state = 1;
			amx_GetUserData(vm, ownerTag, reinterpret_cast<void**>(&owner));
			if (owner == DebugServer()->field0x70E8)
			{
				DisplayState(vm, 0, 0);
				ClientChild0x94* controls = DebugClient()->child0x94;
				if (controls) EnableControls(controls, 0);
				amx_GetUserData(vm, stateTag, reinterpret_cast<void**>(&state));
				if (state == 3)
				{
					JE::DebugWindow* window = DebugClient()->child0x8C;
					if (window)
					{
						// Original5A87C7/5A87F2 owns a distinct message at frame-100.
						std::string message("~~Program Finished~~");
						window->Write(message, 128, 128, 0);
					}
				}
			}
			state = 1;
			amx_SetUserData(vm, stateTag, reinterpret_cast<void*>(state));
			break;
		}

		}
		return 0;
	}
}

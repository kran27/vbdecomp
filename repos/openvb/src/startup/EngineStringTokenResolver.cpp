#include "EngineStringSystem.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "entity/World.hpp"

#include <cstring>

namespace F3
{
	void EngineStringSystem::ResolveTokens(std::string& output)
	{
		const std::string source(output);
		std::string token;
		output.erase(output.begin(), output.end());
		for (std::string::size_type i = 0; i < source.size(); ++i)
		{
			if (source[i] == '<')
			{
				for (std::string::size_type end = i; end < source.size(); ++end)
				{
					if (source[end] != '>') continue;
					token = source.substr(i + 1, end - i - 1);
					if (_strnicmp(token.c_str(), "gv_", 3) == 0)
					{
						std::string value;
						const bool missing = !JE::GetWorldInstance()->GetGlobalParameters()->GetString(token.substr(3), std::string(), value);
						if (missing)
						{
							JE::LogDebugString("Couldn't find variable %s", token.c_str());
							output += "<Bad Token>";
						}
						else output += value;
					}
					else
					{
						const JE::Net::Int32 index = tokens.Get(token);
						if (!index)
							JE::LogDebugString("No token %s found\r\n", token.c_str());
						else
						{
							char text[128];
							const bool missing = !GetByIndex(static_cast<JE::Net::UInt32>(index), text);
							if (missing)
							{
								JE::LogDebugString("Couldn't find string %s", token.c_str());
								output += "<Bad Token>";
							}
							else output += text;
						}
					}
					i = end;
					break;
				}
			}
			else output += source[i];
		}
	}
}

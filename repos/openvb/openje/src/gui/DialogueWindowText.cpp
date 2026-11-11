#include "gui/DialogueWindow.hpp"
#include <cstdio>
#include <cstring>

namespace JE
{
	namespace
	{
		std::string& __stdcall ResolveBuiltin(std::string& output,
			const std::string& token, DWORD /*participantID*/) //5930E0.
		{
			output = "";
			if (_stricmp(token.c_str(), "<CHARNAME>") == 0) return output.assign("Sawyer");
			if (_stricmp(token.c_str(), "<DEITY>") == 0) return output.assign("Urquhart");
			return output.assign("NULL");
		}
	}
	void DialogueWindow::FormatLine(std::string& output, std::string& input,
		DWORD /*participantID*/, LONG number) //5945F0.
	{
		std::string::size_type i = 0, begin = 0;
		DWORD state = 0;
		std::string markup, tokenText;
		output = "";
		if (number > 0)
		{
			char prefix[32];
			std::sprintf(prefix, "%d.  ", number);
			output.assign(prefix);
		}
		for (; i < input.size(); ++i)
		{
			switch (state)
			{
			case 0:
				if (input[i] == '{') { begin = i; state = 2; }
				else if (input[i] == '<') { begin = i; state = 3; }
				else { begin = i; state = 1; }
				break;
			case 1:
				if (input[i] == '{')
				{
					output.append(input.begin() + begin, input.begin() + i);
					begin = i; state = 2;
				}
				else if (input[i] == '<')
				{
					output.append(input.begin() + begin, input.begin() + i);
					begin = i; state = 3;
				}
				break;
			case 2:
				if (input[i] == '}')
				{
					markup = input.substr(begin, i - begin + 1);
					state = 0; //Entire brace segment is discarded.
				}
				break;
			case 3:
				if (input[i] == '>')
				{
					markup = input.substr(begin, i - begin + 1);
					state = 0;
					ResolveBuiltin(tokenText, markup, listenerID);
					output.append(tokenText);
				}
				break;
			}
		}
		if (state == 1) output.append(input.begin() + begin, input.end());
		//Unterminated brace/angle content is discarded.
	}
}

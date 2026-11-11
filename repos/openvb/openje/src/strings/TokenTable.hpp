#pragma once

#include "network/Types.hpp"
#include <map>
#include <string>

namespace JE
{
	namespace SS
	{
		class TokenTable
		{
		public:
			typedef std::map<std::string, JE::Net::Int32> Entries;
			unsigned char Load(const std::string& sFilename); // 0x5AD530
			void Set(const char* szName, JE::Net::Int32 nValue); // 0x5AD460
			JE::Net::Int32 Get(const std::string& sName) const; // 0x5ACE40
			const Entries& GetEntries() const { return m_Entries; }

		private:
			// 0x5ACE70. The final newline check reads the byte at end even
			// when cursor equals end. No sentinel or favorable guard is added.
			const char* ReadTokenLine(const char*& pCursor, const char* pEnd,
				std::string& sLine);
			// 0x5ACF10. Missing '=' leaves both outputs untouched.
			unsigned char ParseTokenLine(const std::string& sLine, std::string& sName,
				JE::Net::Int32& nValue);
			Entries m_Entries;
		};
	}
}

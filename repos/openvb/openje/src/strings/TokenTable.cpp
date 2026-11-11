#include "TokenTable.hpp"
#include "diagnostics/DebugOutput.hpp"
#include "resources/ResourceSystem.hpp"

#include <cstdlib>

namespace JE
{
	namespace SS
	{
		const char* TokenTable::ReadTokenLine(const char*& pCursor, const char* pEnd,
			std::string& sLine)
		{
			sLine.clear();
			while (pCursor != pEnd)
			{
				const char c = *pCursor;
				if (c == '\r' || c == '\n')
					break;
				if (c != ' ' && c != '\t')
					sLine += c;
				++pCursor;
			}
			// Preserve the asymmetric end check at 0x5ACEF0..0x5ACF03. The LF
			// operand still reads *pCursor at end; no sentinel is fabricated here.
			while ((pCursor != pEnd && *pCursor == '\r') || *pCursor == '\n')
				++pCursor;
			return pCursor;
		}

		unsigned char TokenTable::ParseTokenLine(const std::string& sLine, std::string& sName,
			JE::Net::Int32& nValue)
		{
			const std::string::size_type nEquals = sLine.find('=');
			if (nEquals == std::string::npos)
				return 0;
			sName = sLine.substr(0, nEquals);
			nValue = static_cast<JE::Net::Int32>(std::atol(sLine.substr(nEquals + 1).c_str()));
			return 1;
		}

		void TokenTable::Set(const char* szName, JE::Net::Int32 nValue)
		{
			const std::string sName(szName);
			Entries::iterator found;
			found = m_Entries.find(sName);
			if (found != m_Entries.end())
				found->second = nValue;
			else
				m_Entries[sName] = nValue;
		}

		JE::Net::Int32 TokenTable::Get(const std::string& sName) const
		{
			Entries::const_iterator found;
			found = m_Entries.find(sName);
			if (found != m_Entries.end())
				return found->second;
			return 0;
		}

		unsigned char TokenTable::Load(const std::string& sFilename)
		{
			unsigned char result = 1;
			std::string sLine, sName;
			Resources::ResourceData* const pResource =
				Resources::g_pResourceSystem->Demand(sFilename.c_str(), 1);
			if (pResource)
			{
				const char* pCursor = static_cast<const char*>(pResource->data);
				const char* const pEnd = pCursor + pResource->size;
				JE::Net::Int32 nValue;
				JE::Net::UInt32 nLine = 1;
				while (pCursor != pEnd)
				{
					ReadTokenLine(pCursor, pEnd, sLine);
					if (!ParseTokenLine(sLine, sName, nValue))
						JE::LogDebugString("Incorrect token formatting on line %d\r\n",
							static_cast<JE::Net::Int32>(nLine));
					else
						Set(sName.c_str(), nValue);
					++nLine;
				}
				// Demand/logging may replace the canonical manager. Do not capture it.
				Resources::g_pResourceSystem->ReleaseResource(pResource);
			}
			else
			{
				JE::LogDebugString("Couldn't load tokens file\r\n");
				result = 0;
			}
			return result;
		}
	}
}

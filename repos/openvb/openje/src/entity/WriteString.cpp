#include "Chunk.hpp"
#include "resources/ResourceSystem.hpp"

namespace JE { namespace Common
{
	BYTE WriteString(const std::string& input, FILE* stream)
	{
		const WORD length = WORD(input.size());
		if (fwrite(&length, 2, 1, stream) != 1) return 0;
		if (length && fwrite(input.c_str(), 1, length, stream) != length) return 0;
		return 1;
	}

	BYTE WriteString(const std::string& input, Resources::ResourceFileHandle* stream)
	{
		const WORD length = WORD(input.size());
		//49C73C/49C772 captures and writes only the low WORD, including wrap.
		if (Resources::g_pResourceSystem->Write(&length, 2, stream))
		{
			if (!length || Resources::g_pResourceSystem->Write(input.c_str(), length, stream)) return 1;
		}
		return 0;
	}
} }

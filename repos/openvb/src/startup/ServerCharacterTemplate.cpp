#include "network/EngineServer.hpp"
#include "resources/ResourceSystem.hpp"
#include "startup/SaveDirectory.hpp"
#include "startup/Path.hpp"
#include <cstring>

namespace JE
{
	void Server::RegisterCharacter(const char* name) //5565B0.
	{
		characterTemplate = name;
		const std::size_t length = std::strlen(name);
		if (length < 5 || ::_stricmp(name + length - 4, ".crt") != 0)
			characterTemplate += ".crt";
		char filename[260];
		std::strcpy(filename, F3::GetSaveDirectoryAbsolute()); //56AF00.
		F3::PathConcat(filename, "characters"); //4C58A0; original lower-case spelling.
		F3::PathConcat(filename, characterTemplate.c_str());
		Resources::g_pResourceSystem->RegisterTemporaryFile(filename); //49ACD0, actual resource override publication.
	}
}

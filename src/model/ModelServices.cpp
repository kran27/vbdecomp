#include "ModelServices.hpp"
#include "configuration/ResourceConfigurationRead.hpp"
#include "resources/ResourceSystem.hpp"
#include "resources/ResourceFormats.hpp"
#include "graphics/SystemInterface.hpp"
#include "d3dx9.h"
#include <string.h>

namespace JE
{
    namespace ModelServices
    {
        BYTE ReadSettings(const std::string& name, Configuration::Settings& settings)
        {
            return Configuration::ReadResourceConfigurationFile(name, settings);
        }
        BYTE ResourceExists(const char* name)
        {
            std::string copy(name);
            DWORD type = Resources::ExtractFileType(copy);
            Resources::ResourceSystem* system = Resources::g_pResourceSystem;
            system->lock.Lock();
            Resources::ResourceDescriptor* descriptor = system->temporaryFileIndex.Find(copy.c_str(), type);
            if (!descriptor)
            {
                descriptor = system->primary.Find(copy.c_str(), type);
            }
            system->lock.Unlock();
            return descriptor != NULL;
        }
        std::string UpperName(const std::string& name)
        {
            char buffer[260];
            strcpy(buffer, name.c_str());
            _strupr(buffer);
            return std::string(buffer);
        }
    } // namespace ModelServices
} // namespace JE

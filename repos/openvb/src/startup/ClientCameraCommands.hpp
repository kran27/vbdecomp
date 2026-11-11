#pragma once

#include "ConsoleCommands.hpp"
#include "network/Types.hpp"

namespace F3
{
	//1 means this callback identity was handled, not an original return value.
	unsigned char ExecuteClientCameraCommand(JE::Net::UInt32 originalVA, CommandArguments* arguments);
}

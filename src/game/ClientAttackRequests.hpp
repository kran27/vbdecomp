#pragma once

#include <winsock2.h>
#include <windows.h>
#include "d3dx9.h"

namespace JE
{
    namespace Net
    {
        class Client;
        class Packet;
    } // namespace Net
} // namespace JE

namespace F3
{
    BYTE ReceiveGameServerAttackRequest(JE::Net::Packet& packet); // 46DD70 case91.
}

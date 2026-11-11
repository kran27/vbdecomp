#pragma once
#include <winsock2.h>
#include <windows.h>
namespace JE { class Server; namespace Net { class Client; class Packet; } }
namespace F3
{
    struct GameClient;
    class ActionbarInterface;
    BYTE ReceiveGameServerAttackModeRequest(JE::Server&,JE::Net::Packet&); //46DD70 cases84/100/101.
    BYTE ReceiveClientAttackModeNotification(GameClient&,JE::Net::Packet&); //461A10 refresh-only cases.
    BYTE ReceiveGameServerReloadRequest(JE::Net::Packet&); //46DD70 case90, actualdirectnewReload.
}


#include "game/ClientInventoryEquipmentRequests.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"
namespace JE
{
    void Client::RequestEquip(DWORD actor,DWORD item,DWORD source,DWORD slot)
    {
        JE::Net::Packet packet;
        packet.field0x30=95;
        packet.WriteDword(actor); packet.WriteDword(item);
        packet.WriteDword(source); packet.WriteDword(slot);
        Send(packet,1);
    }
    void Client::RequestUnequip(DWORD actor,DWORD item)
    {
        JE::Net::Packet packet;
        packet.field0x30=96;
        packet.WriteDword(actor); packet.WriteDword(item);
        Send(packet,1);
    }
    void Client::RequestDropItem(DWORD actor,DWORD item)
    {
        JE::Net::Packet packet;
        packet.field0x30=51;
        packet.WriteDword(actor); packet.WriteDword(item);
        Send(packet,1);
    }
}


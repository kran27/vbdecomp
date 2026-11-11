#include "network/Types.hpp"
#include "game/ClientAttackModeRequests.hpp"
#include "game/GameWeapon.hpp"
#include "game/GameCreature.hpp"
#include "startup/GameClient.hpp"
#include "network/EngineServer.hpp"
#include "network/Client.hpp"
#include "network/EngineClient.hpp"
#include "network/Packet.hpp"
namespace JE
{
    void Client::RequestReload(DWORD actor,DWORD weapon,DWORD ammoID) //460100.
    {
        JE::Net::Packet packet; packet.field0x30=90;
        packet.WriteDword(actor); packet.WriteDword(weapon); packet.WriteDword(ammoID);
        Send(packet,1);
    }
    void Client::RequestWeaponSelection(DWORD actor,DWORD slot) //4605E0.
    {
        JE::Net::Packet packet; packet.field0x30=89;
        packet.WriteDword(actor); packet.WriteDword(slot);
        Send(packet,1);
    }
    void Client::RequestWeaponModeCycle(DWORD id) //45FF80.
    {
        JE::Net::Packet packet;
        packet.field0x30=100; packet.WriteDword(id);
        Send(packet,1);
    }
    void Client::RequestUnarmedModeCycle(DWORD id) //460040.
    {
        JE::Net::Packet packet;
        packet.field0x30=84; packet.WriteDword(id);
        Send(packet,1);
    }
}

namespace F3
{
    BYTE ReceiveGameServerAttackModeRequest(JE::Server& server,JE::Net::Packet& packet) //46E3F2..46E562.
    {
        if (packet.field0x30!=84 && packet.field0x30!=100 && packet.field0x30!=101) return 0;
        JE::Net::UInt32 id,step;
        packet.ReadDword(id);
        if (packet.field0x30==101) packet.ReadDword(step);
        JE::Entity* entity=JE::GetEntityPtr(id);
        if (!entity) return 1;
        if (packet.field0x30==84)
        {
            GameCreature* creature=dynamic_cast<GameCreature*>(entity);
            if (!creature) return 1;
            creature->CycleAttackMode();
        }
        else
        {
            GameWeapon* weapon=dynamic_cast<GameWeapon*>(entity);
            if (!weapon) return 1;
            if (packet.field0x30==100) weapon->CycleAttackMode();
            else weapon->SelectAttackMode(LONG(step));
        }
        JE::Net::Packet notification;
        notification.field0x30=packet.field0x30;
        //Original46E545 sends an EMPTY payload. NoID, index or replicatedmode
        //is written; theclient cases onlyRefreshInterface, includinglocalfilter.
        server.Broadcast(notification,1);
        return 1;
    }
    BYTE ReceiveClientAttackModeNotification(GameClient& client,JE::Net::Packet& packet)
    {
        if (packet.field0x30!=84 && packet.field0x30!=100 && packet.field0x30!=101) return 0;
        client.RefreshInterface();
        return 1;
    }
}


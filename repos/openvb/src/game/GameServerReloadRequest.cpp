#include "network/Types.hpp"
#include "game/ClientAttackModeRequests.hpp"
#include "game/GameActionReload.hpp"
#include "game/GameWeapon.hpp"
#include "game/GameAmmo.hpp"
#include "entity/Entity.hpp"
#include "network/Packet.hpp"

namespace F3
{
    //Append declaration BYTE ReceiveGameServerReloadRequest(JE::Net::Packet&);
    //Root dispatches before existing GameServer addressedswitch, remove90 there.
    BYTE ReceiveGameServerReloadRequest(JE::Net::Packet& packet) //46E568..46E637.
    {
        if (packet.field0x30!=90) return 0;
        GameAmmo* stack=NULL;
        JE::Net::UInt32 id;
        packet.ReadDword(id);
        JE::Entity* actor=JE::GetEntityPtr(id);
        packet.ReadDword(id);
        GameWeapon* weapon=dynamic_cast<GameWeapon*>(JE::GetEntityPtr(id));
        packet.ReadDword(id);
        if (id) stack=dynamic_cast<GameAmmo*>(JE::GetEntityPtr(id));
        if (actor && weapon)
        {
            GameActionReload* action=new GameActionReload; //Actualdirectnew4071C0; noWorldfactoryalias.
            action->weapon=weapon;
            if (stack) action->stack=stack;
            actor->QueueAction(action); //NoClearActions, no failure notification.
        }
        return 1;
    }
}


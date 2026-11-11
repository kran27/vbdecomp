#include "network/EngineServer.hpp"
#include "network/Packet.hpp"
#include "entity/Entity.hpp"
#include "entity/Inventory.hpp"
#include <cmath>
namespace JE
{
    void Server::SendEntityCount(DWORD entityID, DWORD count) //552D40.
    {
        Net::Packet packet; packet.field0x30=52;
        packet.WriteDword(entityID); packet.WriteDword(count);
        Broadcast(packet,1);
    }
    // Add declarations: ReadInventoryDropRequest(Packet&), PublishInventoryRemoval(DWORD,DWORD,BYTE).
    // Dispatch existing EngineServerPackets case51 here, remove it from addressedlist.
    void Server::PublishInventoryRemoval(DWORD owner,DWORD item,BYTE failure) //553B00.
    {
        Net::Packet packet;
        packet.field0x30=50;
        packet.WriteByte(failure); packet.WriteDword(owner); packet.WriteDword(item);
        Broadcast(packet,1);
    }
    void Server::ReadInventoryDropRequest(Net::Packet& request) //556EA0 physical5574B1..5575C9.
    {
        JE::Net::UInt32 ownerID,itemID;
        request.ReadDword(ownerID); request.ReadDword(itemID);
        BYTE failure=0;
        Entity* owner=GetEntityPtr(ownerID);
        if (!owner) failure=1;
        Entity* item=NULL;
        if (owner)
        {
            item=owner->inventory->FindByID(itemID);
            if (!item) failure=1;
        }
        if (!failure) owner->inventory->Remove(item);
        //Always captured originalrequest IDs, including lookupfailure.
        PublishInventoryRemoval(ownerID,itemID,failure);
        if (owner && item)
        {
            //Read original orientation+1F0/1F4/1F8 and position AFTER removal/publisher.
            D3DXVECTOR3 rotation=owner->rotation;
            const double angle=double(rotation.x)*double(0.01745329238474369f);
            D3DXVECTOR3 position;
            position.x=float(double(owner->position.x)+std::sin(angle)*3.0);
            position.y=owner->position.y;
            //Original FSTP64 retains cos*3 before adding Z. Real CRT trig is reused;
            //modernFP/x87precision is not claimed byte/behavior exact here.
            const double offsetZ=std::cos(angle)*3.0;
            position.z=float(double(owner->position.z)+offsetZ);
            MoveEntityToMap(item,owner->mapHandle,position,rotation,0);
        }
    }
}


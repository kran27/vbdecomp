#include "EngineServer.hpp"

#include <new>
#include <stdexcept>

namespace JE
{
	JE::Net::UInt32 Server::AllocateClientID()
	{
		//46CEA0 is shared by engine/game tables. Keep with the canonical owner
		//so its vtable does not depend on the later World greeting source.
		return ++field0x70A4;
	}
	const char* Server::GetCharacterTemplate() const //54A5F0, borrowed current string storage.
	{
		return characterTemplate.c_str();
	}

	Server::Server(JE::Net::UInt16 port)
		: Net::Server(port), dialogue(NULL), field0x5068(0), field0x70A4(0),
		field0x70E4(1), field0x70E5(0), field0x70E6(0), field0x70E8(0),
		field0x70EC(1), field0x70ED(0)
	{
		EngineServerServices::AssignString(characterTemplate, "Default_PC.CRT");
	}


	void Server::Initialize()
	{
		Configure(16);
		ServerDialogue* created = CreateDialogue();
		dialogue = created;
		created->Initialize();
	}

	void Server::SendDialogueEnd(Net::Packet& packet)
	{
		packet.field0x30 = 55;
		Broadcast(packet, 1);
	}

	JE::Net::UInt8 Server::ScriptSuppressed(JE::Net::UInt32 ownerID) const
	{
		if (!field0x70E5 || field0x70E6)
		{
			JE::Net::UInt32 selected = field0x70E8;
			if (!selected) return 0;
			return selected != ownerID;
		}
		return 1;
	}

	void Server::Cleanup()
	{
		ServerDialogue* captured = dialogue;
		if (captured)
		{
			static_cast<DialogueManager*>(captured)->DialogueManager::~DialogueManager();
			::operator delete(captured);
		}
		Net::Server::Cleanup();
	}

	ServerDialogue* Server::CreateDialogue()
	{
		return new DialogueManager;
	}

	void Server::CreateConnections()
	{
		for (JE::Net::UInt32 index = 0; index < connectionCount; ++index)
			connections[index] = new Net::EngineServerClientConnection;
	}

	namespace EngineServerConnectionServices
	{
		void* Allocate(std::size_t size) { return ::operator new(size); }
		void Construct(Net::EngineServerClientConnection* connection)
		{
			::new (connection) Net::EngineServerClientConnection;
		}
		void Free(void* block) throw() { ::operator delete(block); }
	}

	namespace EngineServerServices
	{
		void AssignString(std::string& target, const char* value) { target.assign(value); }
		void DestroyDialogue(ServerDialogue* dialogue)
		{
			static_cast<DialogueManager*>(dialogue)->DialogueManager::~DialogueManager();
		}
		void FreeDialogue(ServerDialogue* dialogue) { ::operator delete(dialogue); }
		void CleanupNetServer(Net::Server& server) { server.Net::Server::Cleanup(); }
	}
}

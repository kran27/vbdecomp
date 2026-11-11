#pragma once

#include <cstddef>
#include "network/Types.hpp"
#include <map>
#include <string>

namespace JE
{
	namespace Net { class Packet; }
	class DialogueManager;
	typedef DialogueManager ServerDialogue;

	struct Script_Small;
	typedef Script_Small DialogueScript;
	struct DialogueReply
	{
		JE::Net::UInt32 field0x0, field0x4;
		std::string field0x8, field0x24;
	};
	struct DialogueContext
	{
		JE::Net::UInt32 field0x0;
		JE::Net::UInt8 field0x4;
		std::string field0x8, field0x24;
		std::map<JE::Net::UInt32,JE::Net::UInt32> field0x40;
		std::map<JE::Net::UInt32,std::map<JE::Net::UInt32,DialogueReply> > field0x4C;
		JE::Net::UInt8 field0x58;
	};
	struct DialogueState
	{
		JE::Net::UInt8 field0x0;
		DialogueScript* script; // Original mapped+4; manager deletes explicitly.
		JE::Net::UInt32 field0x8, field0xC, field0x10, field0x14;
		std::string field0x18, field0x34;
		DialogueContext field0x50;
		// Scalar members have no guessed defaults. The unrecovered producer must
		// assign them before use/copy; strings/maps use normal native lifetime.
	};
	typedef std::map<JE::Net::UInt8,DialogueState> DialogueEntries;

	class DialogueManager
	{
	public:
		DialogueManager(); // 0x55FAC0.
		~DialogueManager(); // 0x55FA10.
		virtual void Initialize(); // Actual original 0x404B70 no-op.
		JE::Net::UInt8 BeginDialogue(const std::string& name, JE::Net::UInt32 speakerID,
			JE::Net::UInt32 listenerID); //55EDB0, real Script_Small load and state publication.
		JE::Net::UInt8 IsInDialogue(JE::Net::UInt32 entityID) const; //55BF60.
		void RegisterNodeText(JE::Net::UInt32 textID); //55C890, current participant map insertion.
		void SetNodeTextFunction(const std::string& name); //55D910, increment revision and clear maps.
		void RegisterReply(JE::Net::UInt32 id,JE::Net::UInt32 textID,const std::string& action,
			const std::string& nextNode); //55D9A0, first reply identity retained.
		void SetCurrentNodeName(const std::string& name); //55C420, previous/current uppercase names.
		JE::Net::UInt8 WasLastNodeName(const std::string& name); //55C220.
		JE::Net::UInt32 GetPCSpeaker(JE::Net::UInt32 ownerID) const; //55BFA0.
		JE::Net::UInt32 GetSpeakerID() const; //55C1C0.
		void SetSpeakerID(JE::Net::UInt32 id); //55C1E0.
		void SetNextSpeakerID(JE::Net::UInt32 id); //55C200.
		void RegisterLinkToOtherDialogue(const std::string& script,
			const std::string& function); //55C4E0, deferred script/function publication.
		void SelectReply(JE::Net::UInt8 id, JE::Net::UInt8 revision,
			JE::Net::UInt32 replyID, JE::Net::UInt32 participantID); //55F850.
		void RequestEndDialogue(JE::Net::UInt8 id); //55C2D0, mark for next pump.
		void EndAllDialogues(); //55F1D0, notify/delete/erase in map order.
		void Update(); //55F3E0, real per-server-frame dialogue state advance.
		void EndDialogue(DialogueEntries::iterator& entry); //55F0A0, updates incoming iterator. //55F0A0.
		void WriteUpdate(Net::Packet& packet); //55E110, active/inactive dialogue payload.

		void AdvanceDialogueID(); //55BAB0, advance the nonzero BYTE key.

		JE::Net::UInt8 field0x4, field0x5; //Native+4/+5, stores follow map construction.
		DialogueEntries entries; //Native+8; scalar fields have no member initializers.
	};

	namespace DialogueManagerServices
	{
		void DeleteScript(DialogueScript* script);
		void Erase(DialogueEntries& entries, DialogueEntries::iterator entry);
	}
	namespace DialogueFactoryServices
	{
		void* Allocate(std::size_t size);
		void Construct(DialogueManager* manager);
		void Free(void* block) throw();
	}
}

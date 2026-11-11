#pragma once

#include "gui/EngineInterface.hpp"
#include "gui/DialogueAppearance.hpp"
#include <map>

namespace JE
{
	namespace Net { class Packet; }
	class DialogueWindow;

	class DialogueInterface : public virtual EngineInterface
	{
	public:
		explicit DialogueInterface(const std::string& name); //5943C0.
		virtual ~DialogueInterface(); //593530, before the shared EngineInterface lifetime.
		virtual DialogueWindow* CreateDialogueWindow(BYTE id, BYTE revision); //596780.
		virtual void BeforeUpdate(); //593640 ->5925B0.
		virtual void BeforeDraw(); //593650 ->5925F0.
		void ReadUpdate(Net::Packet& packet); //596050.
		void RemoveDialogue(BYTE id); //593090.
		BYTE AllWindowsHidden(); //592630; live ordered map, empty returns1.

		DialogueAppearance speaker, listener; //Original two304-byte settings groups, normal VC7 strings.
		std::map<BYTE, DialogueWindow*> windows; //Original+268, explicit owning map.
	};
}

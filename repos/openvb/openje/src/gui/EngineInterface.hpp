#pragma once

#include "gui/Interface.hpp"

namespace JE
{
	class EngineInterface : public ::GUI::Interface
	{
	public:
		explicit EngineInterface(const std::string& name); //580330.
		virtual ~EngineInterface(); //580350.
		virtual int OnEvent(::GUI::Window*, ::GUI::Event*); //5803E0.
		virtual BYTE OnCursor(::GUI::Window*, LONG, LONG); //580360.
	};
}

#include "SystemInterface.hpp"

//Actual5BEDD0 constructor-unwind chunk and5C1E1C/24 destructor suffix.
//Normal base lifetime clears publication after all derived members, including
//the constructor-failure path. Never retain/restore a previous owner here.
Gfx_System_Interface::~Gfx_System_Interface()
{
	g_GfxSystem = NULL;
}

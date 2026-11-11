#pragma once

#include <windows.h>

namespace JE
{
	//Actual5BDB70/5BDC10 do not read this or retain an engine flag. These
	//stateless operations reuse the selected device and real D3D state cache.
	void SetGraphicsLinearFiltering(BYTE enabled); //5BDB70, interface slot+70.
	void SetGraphicsMipFiltering(BYTE enabled); //5BDC10, interface slot+74.
}

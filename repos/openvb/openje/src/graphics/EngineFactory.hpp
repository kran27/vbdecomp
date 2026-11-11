#pragma once

#include <windows.h>

class Gfx_System_Interface;

BYTE Gfx_InitializeAnimationSystem(); //5B3070; admits actual Granny2.1.0.5 and installs log callback.
Gfx_System_Interface* Gfx_CreateEngine(); //5B30C0; existing singleton or genuine Engine construction.
void Gfx_ReleaseEngine(); //5B3150; actual virtual deletion, unused original EAX.

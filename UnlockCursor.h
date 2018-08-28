#pragma once
#include "SDK.h"
typedef void(__thiscall* UnlockCursor)(void*);

void __stdcall hkUnlockCursor()
{
	static auto ofunc = hooks::surface.get_original<UnlockCursor>(67);

	if (g_Options.Menu.Opened) {
		g_Surface->UnlockCursor();
	}
	else
		ofunc(g_Surface);
}
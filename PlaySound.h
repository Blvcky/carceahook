#pragma once
#include "HookIncludes.h"
typedef void(__thiscall* play_sound_t)(void*, const char*);

void __stdcall hkPlaySound(const char* szFileName)
{

	static auto ofunc = hooks::surface.get_original<play_sound_t>(82);
	//Call original PlaySound
	ofunc(g_Surface, szFileName);

	if (g_Engine->IsInGame()) return;

	
}
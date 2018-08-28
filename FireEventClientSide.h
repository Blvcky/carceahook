#pragma once
#include "HookIncludes.h"
#include "SkinChanger.h"
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
FireEventClientSideFn oFireEventClientSide;
bool __fastcall hkFireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *event);

int Kills2 = 0;
int Kills = 0;
bool RoundInfo = false;
size_t Delay = 0;



bool __fastcall hkFireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *event) 
{
	return oFireEventClientSide(ECX, event);
}
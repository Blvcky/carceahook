#pragma once
#include "Hooks.h"
#include "ImGui/imgui.h"
#include "ImGui\examples\directx9_example\imgui_impl_dx9.h"
#include "Variables.h"


namespace G // Global Stuff
{
	extern bool			Aimbotting;
	extern bool			InAntiAim;
	extern bool			Return;
	extern HWND			Window;
	extern bool			PressedKeys[256];
	extern float		FOV;
	extern bool			LocalBreakingLBY;
	extern int			ChamMode;
	extern char			AnimatedClantag[16];
	extern bool			SendPacket;
	extern int			BestTarget;
	extern char			buf3[128];
	extern char			extra_flags; 
	extern bool         MenuOpen;
	extern float		hitmarkeralpha;
	extern float		wcghetou;
	extern bool d3dinit;
	extern bool unload;
	using msg_t = void(__cdecl*)(const char*, ...);
	extern msg_t		Msg;
	extern WNDPROC  OldWindow;
	static int MenuTab = 0;
}






namespace Global
{
	extern std::vector<ColorP>ColorsForPicker1;
}
#pragma once
#include "HookIncludes.h"
#include "Interface.h"
#include "Parser.h"
#include "Variables.h"
#include <Windows.h>
#include <d3d9.h>
#include <iostream>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")
#pragma comment(lib,"d3dx9.lib")

typedef HRESULT(__stdcall *EndScene_t) (IDirect3DDevice9*);
typedef HRESULT(__stdcall *Reset_t) (IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


EndScene_t oEndScene = nullptr;
Reset_t	oReset = nullptr;
HWND window = nullptr;

namespace handlers
{
	LRESULT __stdcall Hooked_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

		switch (uMsg)
		{
		case WM_LBUTTONDOWN:
			G::PressedKeys[VK_LBUTTON] = true;
			break;
		case WM_LBUTTONUP:
			G::PressedKeys[VK_LBUTTON] = false;
			break;
		case WM_RBUTTONDOWN:
			G::PressedKeys[VK_RBUTTON] = true;
			break;
		case WM_RBUTTONUP:
			G::PressedKeys[VK_RBUTTON] = false;
			break;
		case WM_KEYDOWN:
			G::PressedKeys[wParam] = true;
			break;
		case WM_KEYUP:
			G::PressedKeys[wParam] = false;
			break;
		default: break;
		}

		static bool isDown = false;
		static bool isClicked = false;
		if (G::PressedKeys[VK_INSERT])
		{
			isClicked = false;
			isDown = true;
		}
		else if (!G::PressedKeys[VK_INSERT] && isDown)
		{
			isClicked = true;
			isDown = false;
		}
		else
		{
			isClicked = false;
			isDown = false;
		}

		if (isClicked)
		{
			g_Options.Menu.Opened = !g_Options.Menu.Opened;
			static auto cl_mouseenable = g_CVar->FindVar("cl_mouseenable");
			cl_mouseenable->SetValue(!g_Options.Menu.Opened);
		}

		if (G::d3dinit && g_Options.Menu.Opened && ImGui_ImplDX9_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		return CallWindowProc(G::OldWindow, hWnd, uMsg, wParam, lParam);
	}



	HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9* pDevice)
	{
		
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);

			if (!G::d3dinit) 
			{
				CarceaHOOK::GUI_Init(window, pDevice);
			}


			ImGui::GetIO().MouseDrawCursor = g_Options.Menu.Opened;

			ImGui_ImplDX9_NewFrame();

			if (g_Options.Menu.Opened)
			{
				CarceaHOOK::mainWindow();
			}


			ImGui::Render();

		return hooks::d3d.get_original<EndScene_t>(42)(pDevice);
	}


	HRESULT __stdcall Hooked_Reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* params)
	{
		if (!G::d3dinit)
			return hooks::d3d.get_original<Reset_t>(16)(pDevice, params);
	
		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = hooks::d3d.get_original<Reset_t>(16)(pDevice, params);
      

		if (hr >= 0)
		{
			ImGui_ImplDX9_CreateDeviceObjects();
			g_Render->SetupFonts();
		}

		return hr;
	}

}



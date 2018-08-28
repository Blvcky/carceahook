#pragma once
#include "HookIncludes.h"
#include "Interfaces.h"
#include <ctime>
#include "ESP.h"
#include "Listener.h"

std::vector<const char*> vistasmoke_mats =
{
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};

typedef void(__thiscall* paint_traverse_t)(PVOID, unsigned int, bool, bool);
bool once = true;
bool once1 = false;
int width1 = 0;
int height1 = 0;

void __fastcall hkPaintTraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	int w, h;
	int centerW, centerh, topH;
	g_Engine->GetScreenSize(w, h);
	centerW = w / 2;

	static auto ofunc = hooks::panel.get_original<paint_traverse_t>(41);
	C_BaseEntity* local = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	if (local != nullptr && local->IsAlive() && g_Options.Visuals.noscopeborder && g_Options.Visuals.Enabled && !strcmp("HudZoom", g_Panel->GetName(vguiPanel)))
	{
		return;
	}
	ofunc(pPanels, vguiPanel, forceRepaint, allowForce);
	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)g_Panel->GetName(vguiPanel);
		if (strstr(szPanelName, "FocusOverlayPanel"))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{
	
		if (g_Engine->IsConnected() && g_Engine->IsInGame())
		{
			

			static auto linegoesthrusmoke = U::FindPattern("client_panorama.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");
			static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
			switch (g_Options.Visuals.Smoke)
			{
			case 0:
				for (auto mat_s : vistasmoke_mats)
				{
					IMaterial* mat = g_MaterialSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
				}
				break;
			case 1:
				for (auto mat_s : vistasmoke_mats)
				{
					IMaterial* mat = g_MaterialSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
					mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
				}
				*(int*)(smokecout) = 0;
				break;
			case 2:
				*(int*)(smokecout) = 0;
				break;
			default:
				break;
			}
			Color color(int(g_Options.Colors.ScopeLine[0] * 255.f), int(g_Options.Colors.ScopeLine[1] * 255.f), int(g_Options.Colors.ScopeLine[2] * 255.f));
			visuals::instance().OnPaintTraverse(local);
			damage_indicators.paint();

			auto m_flFlashDuration = NetVarManager->GetOffset("DT_CSPlayer", "m_flFlashDuration");
			auto m_flFlashMaxAlpha = NetVarManager->GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
			if (local != nullptr )
			{
				CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(local->GetActiveWeaponHandle());
				if (g_Options.Visuals.NoFlash && g_Options.Visuals.Enabled)
				{
					*MakePtr(float*, local, m_flFlashDuration) = 0.f;
					*MakePtr(float*, local, m_flFlashMaxAlpha) = 0.f;
				}
				if (local && local->IsScoped() && g_Options.Visuals.noscopeborder && MiscFunctions::IsSniper(pWeapon) && g_Options.Visuals.Enabled)
				{
					int width = 0;
					int height = 0;
					g_Engine->GetScreenSize(width, height);

					int centerX = static_cast<int>(width * 0.5f);
					int centerY = static_cast<int>(height * 0.5f);
					g_Render->Line(0, centerY, width, centerY, color);
					g_Render->Line(centerX, 0, centerX, height, color);
				}

			}
	
		}
		if (!g_Engine->IsInGame()) {
			g_Options.Misc.spammeron = false;


		}

		if (g_Options.Ragebot.MainSwitch && !once)
		{
//			g_Options.LegitBot.Enable = false;
			once = !once;
		}
		if (g_Options.LegitBot.Enable && once)
		{
//			g_Options.Ragebot.MainSwitch = false;
			once = !once;
		}

		if (g_Options.Misc.Watermark)
		{

			std::time_t result = std::time(nullptr);
			int w, h;
			g_Engine->GetScreenSize(w, h);
			g_Render->Text(w - 220 + 7, 5, Color(255, 255, 255 ,255), g_Render->font.Watermark, " CarceaHOOK |");
			g_Render->Text(w - 220 + 84, 5, Color(255, 255, 255, 255), g_Render->font.Watermark, std::asctime(std::localtime(&result)));
		}
			
		if (!g_Options.Misc.antiuntrusted)
		{
			g_Render->Text(10, 10, Color(255, 10, 10, 255), g_Render->font.Watermark, "ANTI-UNTRUSTED IS DISABLED");
			g_Render->Text(10, 25, Color(255, 10, 10, 255), g_Render->font.Watermark, "YOU WILL GET BANNED FOR JOINING A MATCH (OFFLINE & ONLINE)");
			g_Render->Text(10, 40, Color(255, 10, 10, 255), g_Render->font.Watermark, "DO NOT JOIN A MATCH/SERVER");
		}

		if (g_Options.Visuals.Enabled)
		{
			int width = 0;
			int height = 0;
			g_Engine->GetScreenSize(width, height);
	
			if (g_Engine->IsInGame() && g_Engine->IsConnected())
			{
				if (local->IsAlive())
				{
					if (g_Options.Visuals.RageDraw)
					{
						
					}
					if (g_Options.Visuals.LBYIndicator)
					{
						if (G::LocalBreakingLBY)
						{
							g_Render->Textf(7, height / 2 - 10, Color(0, 255, 0, 255), g_Render->font.LBY, "LBY");
						}
						else
						{
							g_Render->Textf(7, height / 2 - 10, Color(255, 0, 0, 255), g_Render->font.LBY, "LBY");
						}
					}
				}
			}
		}
	}
}

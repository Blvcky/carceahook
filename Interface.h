#pragma once
#include "Configuration.hpp"
#include "dropboxes.h"
#include "Variables.h"
#include <cctype>
#include "Listener.h"
#include "SpecList.h"
#include "namespammer.h"
#include "Radar.h"
#include "memoryfonts.h"
#include <vector>

#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define BUILDSTAMP ( __DATE__ )

void BtnNormal()
{
	auto& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);

}

void BtnActive()
{
	auto& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(0.96, 0.0, 0.0, 1.f);
}

void BtnInviz()
{
	auto& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Button] = ImColor(9, 9, 9, 255);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(9, 9, 9, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(9, 9, 9, 255);
}
void BtnNormalMenu()
{
	auto& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Button] = ImColor(15, 15, 15, 255);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(17, 17, 17, 255);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(17, 17, 17, 255);
}
namespace CarceaHOOK
{

	ImFont* fDefault;

	void GUI_Init(HWND window, IDirect3DDevice9 *pDevice)
	{

		if (ImGui_ImplDX9_Init(window, pDevice))
		{

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			fDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 12.0f);


			auto& styled = ImGui::GetStyle();


			G::extra_flags = 0;

			static int hue = 140;

			ImVec4 col_text = ImColor::HSV(hue / 255.f, 20.f / 255.f, 235.f / 255.f);
			ImVec4 col_main = ImVec4(0.09f, .09f, .09f, 1.f);
			ImVec4 col_back = ImColor(31, 44, 54);
			ImVec4 col_area = ImColor(4, 32, 41);

			styled.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
			styled.Colors[ImGuiCol_TextDisabled] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
			styled.Colors[ImGuiCol_WindowBg] = ImColor(4, 4, 4, 255);
			styled.Colors[ImGuiCol_ChildWindowBg] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_Border] = ImColor(44, 44, 44, 255);
			styled.Colors[ImGuiCol_BorderShadow] = ImColor(44, 44, 44, 255);
			styled.Colors[ImGuiCol_FrameBg] = ImColor(62, 62, 62, 255);
			styled.Colors[ImGuiCol_FrameBgb1g] = ImColor(62, 62, 62, 255);
			styled.Colors[ImGuiCol_FrameBgHovered] = ImColor(62, 62, 62, 255);
			styled.Colors[ImGuiCol_FrameBgActive] = ImColor(62, 62, 62, 255);
			styled.Colors[ImGuiCol_TitleBg] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_TitleBgActive] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_MenuBarBg] = ImVec4(.78f, 0.f, 0.f, .7f);
			styled.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_CheckMark] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_SliderGrab] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_SliderGrabActive] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_Button] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_ButtonHovered] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_ButtonActive] = ImColor(9, 9, 9, 255);
			styled.Colors[ImGuiCol_Header] = ImVec4(.78f, 0.f, 0.f, .7f);
			styled.Colors[ImGuiCol_HeaderHovered] = ImVec4(.78f, 0.f, 0.f, .8f);
			styled.Colors[ImGuiCol_HeaderActive] = ImVec4(.78f, 0.f, 0.f, .87f);
			styled.Colors[ImGuiCol_Column] = ImVec4(col_text.x, col_text.y, col_text.z, 0.32f);
			styled.Colors[ImGuiCol_ColumnHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0.78f);
			styled.Colors[ImGuiCol_ColumnActive] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
			styled.Colors[ImGuiCol_ResizeGrip] = ImVec4(col_main.x, col_main.y, col_main.z, 0.20f);
			styled.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.78f);
			styled.Colors[ImGuiCol_ResizeGripActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
			styled.Colors[ImGuiCol_CloseButton] = ImVec4(col_text.x, col_text.y, col_text.z, 0.f);
			styled.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0);
			styled.Colors[ImGuiCol_CloseButtonActive] = ImVec4(col_text.x, col_text.y, col_text.z, 0);
			styled.Colors[ImGuiCol_PlotLines] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
			styled.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
			styled.Colors[ImGuiCol_PlotHistogram] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
			styled.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
			styled.Colors[ImGuiCol_TextSelectedBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.43f);
			styled.Colors[ImGuiCol_PopupBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.92f);
			styled.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

			auto& style = ImGui::GetStyle();

			style.Alpha = 1.0f;
			style.WindowPadding = ImVec2(8, 8); 
			style.ItemSpacing = ImVec2(8, 4);
			style.WindowMinSize = ImVec2(32, 32);
			style.WindowRounding = 0.5f;
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
			style.FramePadding = ImVec2(4, 2);
			style.FrameRounding = 0.0f;
			style.ItemInnerSpacing = ImVec2(4, 4);
			style.TouchExtraPadding = ImVec2(0, 0);
			style.IndentSpacing = 21.0f;
			style.ColumnsMinSpacing = 3.0f;
			style.ScrollbarSize = 12.0f;
			style.ScrollbarRounding = 0.0f;
			style.GrabMinSize = 0.1f;
			style.GrabRounding = 0.0f;
			style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
			style.DisplayWindowPadding = ImVec2(22, 22);
			style.DisplaySafeAreaPadding = ImVec2(4, 4);
			style.AntiAliasedLines = true;
			style.CurveTessellationTol = 1.25f;
			G::d3dinit = true;
		}

	}
	void mainWindow()
	{
		C_BaseEntity* local;

		static int MenuTab = 0;
		auto& style = ImGui::GetStyle();
		int screen_x, screen_y;
		g_Engine->GetScreenSize(screen_x, screen_y);

			ImGui::SetNextWindowSize(ImVec2(750.f, 500.f));
			if (ImGui::Begin("CarceaHOOK CS:GO", &g_Options.Menu.Opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse || ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoScrollWithMouse || ImGuiWindowFlags_NoScrollbar)); {
				style.WindowPadding = ImVec2(10, 10);
				style.ItemSpacing = ImVec2(8, 8);
				ImGui::BeginChild("tab child", ImVec2(95, -1), true);
				{
					BtnInviz();
					style.WindowPadding = ImVec2(0, 0);
					style.ItemSpacing = ImVec2(0, 0);
					if (g_Options.Menu.MenuTab < 1) g_Options.Menu.MenuTab = 1;
					if (g_Options.Menu.MenuTab > 7) g_Options.Menu.MenuTab = 7;
					if (g_Options.Menu.MenuTab == 1) BtnActive(); else BtnNormal();
					if (ImGui::Button("aimbot							", ImVec2(-1, 15))) g_Options.Menu.MenuTab = 1;
					if (g_Options.Menu.MenuTab == 1)
					{
						if (g_Options.Menu.AimBotTab == 1) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-rage							", ImVec2(-1, 15))) g_Options.Menu.AimBotTab = 1;
						if (g_Options.Menu.AimBotTab == 1)
						{
							if (g_Options.Menu.RageTab == 1) BtnActive(); else BtnNormal();
							if (ImGui::Button("	  -aimbot							", ImVec2(-1, 15))) g_Options.Menu.RageTab = 1;
							if (g_Options.Menu.RageTab == 2) BtnActive(); else BtnNormal();
							if (ImGui::Button("	  -accuracy							", ImVec2(-1, 15))) g_Options.Menu.RageTab = 2;
							if (g_Options.Menu.RageTab == 3) BtnActive(); else BtnNormal();
							if (ImGui::Button("	  -antiaim							", ImVec2(-1, 15))) g_Options.Menu.RageTab = 3;
						}
						if (g_Options.Menu.AimBotTab == 2) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-legit							", ImVec2(-1, 15))) g_Options.Menu.AimBotTab = 2;
					}
					if (g_Options.Menu.MenuTab == 2) BtnActive(); else BtnNormal();
					if (ImGui::Button("visuals							", ImVec2(-1, 15))) g_Options.Menu.MenuTab = 2;
					if (g_Options.Menu.MenuTab == 2)
					{
						if (g_Options.Menu.VisualsTab == 1) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 1							", ImVec2(-1, 15))) g_Options.Menu.VisualsTab = 1;
						if (g_Options.Menu.VisualsTab == 2) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 2							", ImVec2(-1, 15))) g_Options.Menu.VisualsTab = 2;
						if (g_Options.Menu.VisualsTab == 3) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 3							", ImVec2(-1, 15))) g_Options.Menu.VisualsTab = 3;
						if (g_Options.Menu.VisualsTab == 4) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 4							", ImVec2(-1, 15))) g_Options.Menu.VisualsTab = 4;
						if (g_Options.Menu.VisualsTab == 5) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 5							", ImVec2(-1, 15))) g_Options.Menu.VisualsTab = 5;
					}
					if (g_Options.Menu.MenuTab == 3) BtnActive(); else BtnNormal();
					if (ImGui::Button("misc							", ImVec2(-1, 15))) g_Options.Menu.MenuTab = 3;
					if (g_Options.Menu.MenuTab == 3)
					{
						if (g_Options.Menu.MiscTab == 1) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 1							", ImVec2(-1, 15))) g_Options.Menu.MiscTab = 1;
						if (g_Options.Menu.MiscTab == 2) BtnActive(); else BtnNormal();
						if (ImGui::Button("	-part 2							", ImVec2(-1, 15))) g_Options.Menu.MiscTab = 2;
					}
					BtnNormal();
					BtnNormalMenu();
					style.WindowPadding = ImVec2(10, 10);
					style.ItemSpacing = ImVec2(8, 4);
				}ImGui::EndChild();
				ImGui::SameLine();
				ImGui::BeginChild("main child", ImVec2(-1, -1), true);
				{
					if (g_Options.Menu.MenuTab == 1 && g_Options.Menu.AimBotTab == 1)//rage
					{
						if (g_Options.Menu.RageTab == 1)
						{
							ImGui::PushItemWidth(180);
							ImGui::Checkbox(("Ragebot Main Switch"), &g_Options.Ragebot.MainSwitch);
							ImGui::Checkbox(("Rage Aimbot Switch"), &g_Options.Ragebot.Enabled);
							ImGui::Checkbox(("Backtrack"), &g_Options.Ragebot.Backtrack);
							ImGui::Checkbox(("Auto Fire"), &g_Options.Ragebot.AutoFire);
							ImGui::Checkbox(("Silent Aim"), &g_Options.Ragebot.Silent);
							ImGui::SliderFloat(("FOV"), &g_Options.Ragebot.FOV, 1.f, 180.f, "%.0f");
							ImGui::Checkbox(("Auto Pistol"), &g_Options.Ragebot.AutoPistol);
							ImGui::Checkbox(("No Recoil"), &g_Options.Ragebot.AntiRecoil);
							ImGui::PopItemWidth();
							ImGui::PushItemWidth(120);
							ImGui::Text("Auto Stop");
							ImGui::Combo(("##auto stop"), &g_Options.Ragebot.AutoStop, autostop, ARRAYSIZE(autostop));
							ImGui::Checkbox(("Auto Crouch"), &g_Options.Ragebot.AutoCrouch);
							ImGui::Checkbox(("Auto Scope"), &g_Options.Ragebot.AutoScope);
							ImGui::Checkbox(("Fakewalk"), &g_Options.Ragebot.fakewalk);
							ImGui::Hotkey(("##fakewalkkey"), &g_Options.Ragebot.fakewalkkey, ImVec2(120, 20));
							ImGui::Checkbox(("Resolver"), &g_Options.Ragebot.Resolver);
							ImGui::Checkbox(("Baim If Lethal"), &g_Options.Ragebot.BAIMIfLethal);
						}
						else if (g_Options.Menu.RageTab == 2)
						{
							ImGui::PushItemWidth(120);
							ImGui::Checkbox(("Friendly Fire"), &g_Options.Ragebot.FriendlyFire);
							ImGui::Text("If Hitscan Is Off The Aimbot Will Aim At The Selected Hitbox");
							ImGui::Text("Hitbox");
							ImGui::Combo(("##Hitbox"), &g_Options.Ragebot.Hitbox, aimBones, ARRAYSIZE(aimBones));
							ImGui::Text("Hitscan");
							ImGui::Combo(("##Hitscan"), &g_Options.Ragebot.Hitscan, hitscan, ARRAYSIZE(hitscan));
							ImGui::Text("Minimum Damage");
							ImGui::Separator();
							ImGui::SliderFloat(("Sniper Min Dmg"), &g_Options.Ragebot.MinimumDamageSniper, 1.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Rifle Min Dmg"), &g_Options.Ragebot.MinimumDamageRifle, 1.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Pistol Min Dmg"), &g_Options.Ragebot.MinimumDamagePistol, 1.f, 100.f, "%.0f");
							ImGui::SliderFloat(("HEavy Min Dmg"), &g_Options.Ragebot.MinimumDamageHeavy, 1.f, 100.f, "%.0f");
							ImGui::SliderFloat(("SMG Min Dmg"), &g_Options.Ragebot.MinimumDamageSmg, 1.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Revolver / Deagle Min Dmg"), &g_Options.Ragebot.MinimumDamageRevolver, 1.f, 100.f, "%.0f");
							ImGui::Separator();
							ImGui::Checkbox(("Enable Hitchance"), &g_Options.Ragebot.Hitchance);
							ImGui::SliderFloat(("Sniper Hitchance"), &g_Options.Ragebot.HitchanceSniper, 0.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Rifle Hitchance"), &g_Options.Ragebot.HitchanceRifle, 0.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Pistol Hitchance"), &g_Options.Ragebot.HitchancePistol, 0.f, 100.f, "%.0f");
							ImGui::SliderFloat(("SMG Hitchance"), &g_Options.Ragebot.HitchanceSmgs, 0.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Heavy Hitchance"), &g_Options.Ragebot.HitchanceHeavy, 0.f, 100.f, "%.0f");
							ImGui::SliderFloat(("Revolver / Deagle Hitchance"), &g_Options.Ragebot.HitchanceRevolver, 0.f, 100.f, "%.0f");
						}
						else if (g_Options.Menu.RageTab == 3)
						{
							ImGui::Checkbox(("Enable AntiAim"), &g_Options.Ragebot.EnabledAntiAim);
							ImGui::Checkbox(("Break LBY"), &g_Options.Ragebot.BreakLBY);
							ImGui::SliderFloat(("LBY Breaker Delta"), &g_Options.Ragebot.LBYDelta, 0.f, 119.f, "%.0f");
							static int movementtype = 0;
							if (movementtype == 0) BtnActive(); else BtnNormal();
							if (ImGui::Button("Moving", ImVec2(115, 25)))
								movementtype = 0;
							ImGui::SameLine();
							if (movementtype == 1) BtnActive(); else BtnNormal();
							if (ImGui::Button("Standing", ImVec2(115, 25)))
								movementtype = 1;
							ImGui::SameLine();
							if (movementtype == 2) BtnActive(); else BtnNormal();
							if (ImGui::Button("In Air", ImVec2(115, 25)))
								movementtype = 2;
							BtnNormal();

							static int aatabtype = 0;
							static int aatabtype1 = 0;
							static int aatabtype2 = 0;
							static int aatabtype3 = 0;
							static int aatabtype4 = 0;

							if (movementtype == 0)
							{
								ImGui::Checkbox(("Moving AA"), &g_Options.Ragebot.AA_onWalk);

								if (aatabtype == 0) BtnActive(); else BtnNormal();
								if (ImGui::Button("Presets", ImVec2(150, 25))) aatabtype = 0;
								ImGui::SameLine();
								if (aatabtype == 1) BtnActive(); else BtnNormal();
								if (ImGui::Button("Self Made", ImVec2(150, 25))) aatabtype = 1;
								BtnNormal();

								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.walk_PreAAs);
								ImGui::Combo(("Pitch"), &g_Options.Ragebot.walk_Pitch, antiaimpitch, ARRAYSIZE(antiaimpitch));
								ImGui::SliderFloat(("Pitch Adder: "), &g_Options.Ragebot.walk_PitchAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Yaw"), &g_Options.Ragebot.walk_YawTrue, antiaimyawtrue, ARRAYSIZE(antiaimyawtrue));
								ImGui::SliderFloat(("Real Adder: "), &g_Options.Ragebot.walk_YawTrueAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Fake Yaw"), &g_Options.Ragebot.walk_YawFake, antiaimyawfake, ARRAYSIZE(antiaimyawfake));
								ImGui::SliderFloat(("Fake Adder: "), &g_Options.Ragebot.walk_YawFakeAdder, -180, 180, "%1.f");
								ImGui::Hotkey(("Manual AA Key"), &g_Options.Ragebot.manualkey, ImVec2(150, 20));


							}
							else if (movementtype == 1)
							{
								ImGui::Checkbox(("Standing AA"), &g_Options.Ragebot.AA_onStand);
								if (aatabtype2 == 0) BtnActive(); else BtnNormal();
								if (ImGui::Button("Presets", ImVec2(150, 25))) aatabtype2 = 0;
								ImGui::SameLine();
								if (aatabtype2 == 1) BtnActive(); else BtnNormal();
								if (ImGui::Button("Self Made", ImVec2(150, 25))) aatabtype2 = 1;
								BtnNormal();

								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.stand_PreAAs);
								ImGui::Combo(("Pitch"), &g_Options.Ragebot.stand_Pitch, antiaimpitch, ARRAYSIZE(antiaimpitch));
								ImGui::SliderFloat(("Pitch Adder: "), &g_Options.Ragebot.stand_PitchAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Yaw"), &g_Options.Ragebot.stand_YawTrue, antiaimyawtrue, ARRAYSIZE(antiaimyawtrue));
								ImGui::SliderFloat(("Real Adder: "), &g_Options.Ragebot.stand_YawTrueAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Fake Yaw"), &g_Options.Ragebot.stand_YawFake, antiaimyawfake, ARRAYSIZE(antiaimyawfake));
								ImGui::SliderFloat(("Fake Adder: "), &g_Options.Ragebot.stand_YawFakeAdder, -180, 180, "%1.f");
								ImGui::Hotkey(("Manual AA Key"), &g_Options.Ragebot.manualkey, ImVec2(150, 20));
							}
							else if (movementtype == 2)
							{
								ImGui::Checkbox(("Air AA"), &g_Options.Ragebot.AA_onAir);


								if (aatabtype2 == 0) BtnActive(); else BtnNormal();
								if (ImGui::Button("Presets", ImVec2(150, 25))) aatabtype2 = 0;
								ImGui::SameLine();
								if (aatabtype2 == 1) BtnActive(); else BtnNormal();
								if (ImGui::Button("Self Made", ImVec2(150, 25))) aatabtype2 = 1;
								BtnNormal();

								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.air_PreAAs);
								ImGui::Combo(("Pitch"), &g_Options.Ragebot.air_Pitch, antiaimpitch, ARRAYSIZE(antiaimpitch));
								ImGui::SliderFloat(("Pitch Adder: "), &g_Options.Ragebot.air_PitchAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Yaw"), &g_Options.Ragebot.air_YawTrue, antiaimairyawtrue, ARRAYSIZE(antiaimairyawtrue));
								ImGui::SliderFloat(("Real Adder: "), &g_Options.Ragebot.air_YawTrueAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Fake Yaw"), &g_Options.Ragebot.air_YawFake, antiaimyawfake, ARRAYSIZE(antiaimyawfake));
								ImGui::SliderFloat(("Fake Adder: "), &g_Options.Ragebot.air_YawFakeAdder, -180, 180, "%1.f");
								ImGui::Hotkey(("Manual AA Key"), &g_Options.Ragebot.manualkey, ImVec2(150, 20));

							}
						}
					}
					if (g_Options.Menu.MenuTab == 1 && g_Options.Menu.AimBotTab == 2)//legit
					{
						ImGui::Checkbox(("Enable Legitbot"), &g_Options.LegitBot.Enable);
						ImGui::Checkbox(("Auto Pistol"), &g_Options.LegitBot.AutoPistol);
						ImGui::Checkbox(("Enable Backtrack##backtrack"), &g_Options.Backtrack.backtrackenable);
						ImGui::PushItemWidth(100);
						ImGui::Combo(("Visualize Backtrack"), &g_Options.Backtrack.BacktrackType, Backtracktype, ARRAYSIZE(Backtracktype));
						ImGui::SliderInt("Backtrack Ticks", &g_Options.Backtrack.backtrackticks, 1, 12);
						ImGui::PopItemWidth();
						ImGui::Checkbox(("Legit AA"), &g_Options.LegitBot.EnableLegitAA);
						ImGui::Checkbox(("Manual AA"), &g_Options.LegitBot.ManualAA);
						ImGui::PushItemWidth(180);
						ImGui::SliderFloat(("AA Angle"), &g_Options.LegitBot.AAAngle, -180, 180, "%1.f");
						ImGui::Checkbox(("Legit Resolver"), &g_Options.LegitBot.ResolverLegit);
						ImGui::PopItemWidth();
						ImGui::BeginChild("setting child", ImVec2(-1, 38), true);
						{
							BtnInviz();
							if (g_Options.Menu.LegitTab == 1) BtnActive(); else BtnNormal();
							if (ImGui::Button("Rifles", ImVec2(82, 15))) g_Options.Menu.LegitTab = 1;
							ImGui::SameLine();
							if (g_Options.Menu.LegitTab == 2) BtnActive(); else BtnNormal();
							if (ImGui::Button("Pistols", ImVec2(82, 15))) g_Options.Menu.LegitTab = 2;
							ImGui::SameLine();
							if (g_Options.Menu.LegitTab == 3) BtnActive(); else BtnNormal();
							if (ImGui::Button("Snipers", ImVec2(82, 15))) g_Options.Menu.LegitTab = 3;
							ImGui::SameLine();
							if (g_Options.Menu.LegitTab == 4) BtnActive(); else BtnNormal();
							if (ImGui::Button("SMGs", ImVec2(82, 15))) g_Options.Menu.LegitTab = 4;
							ImGui::SameLine();
							if (g_Options.Menu.LegitTab == 5) BtnActive(); else BtnNormal();
							if (ImGui::Button("Heavy", ImVec2(82, 15))) g_Options.Menu.LegitTab = 5;
							BtnNormal();
							BtnNormalMenu();
						}ImGui::EndChild();
						ImGui::BeginChild("shit child", ImVec2(-1, -1), true);
						{
							if (g_Options.Menu.LegitTab == 1) // rifle
							{
								ImGui::Text("Rifles");
								ImGui::Text("Key");
								ImGui::Hotkey(("##mainkeyKEY"), &g_Options.LegitBot.MainKey, ImVec2(120, 20));
								ImGui::SliderFloat(("Smooth"), &g_Options.LegitBot.MainSmooth, 1.00f, 100.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Fov"), &g_Options.LegitBot.Mainfov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Rcs"), &g_Options.LegitBot.mainrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Text("Hitbox");
								ImGui::Combo(("##riflehitbox"), &g_Options.LegitBot.riflehitbox, legithitboxes, ARRAYSIZE(legithitboxes));
							}
							else if (g_Options.Menu.LegitTab == 2) // pistol
							{
								ImGui::Text("Pistols");
								ImGui::Text("Key");
								ImGui::Hotkey(("##mpistolKEY"), &g_Options.LegitBot.PistolKey, ImVec2(120, 20));
								ImGui::SliderFloat(("Smooth"), &g_Options.LegitBot.PistolSmooth, 1.00f, 100.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Fov"), &g_Options.LegitBot.Pistolfov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Rcs"), &g_Options.LegitBot.pistolrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Text("Hitbox");
								ImGui::Combo(("##pistolhitbox"), &g_Options.LegitBot.pistolhitbox, legithitboxes, ARRAYSIZE(legithitboxes));
							}
							else if (g_Options.Menu.LegitTab == 3) // sniper
							{
								ImGui::Text("Snipers");
								ImGui::Text("Key");
								ImGui::Hotkey(("##sniperKEY"), &g_Options.LegitBot.SniperKey, ImVec2(120, 20));
								ImGui::SliderFloat(("Smooth"), &g_Options.LegitBot.SniperSmooth, 1.00f, 100.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Fov"), &g_Options.LegitBot.Sniperfov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Rcs"), &g_Options.LegitBot.sniperrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Text("Hitbox");
								ImGui::Combo(("##sniperhitbox"), &g_Options.LegitBot.sniperhitbox, legithitboxes, ARRAYSIZE(legithitboxes));
							}
							else if (g_Options.Menu.LegitTab == 4) // smg
							{
								ImGui::Text("SMGs");
								ImGui::Text("Key");
								ImGui::Hotkey(("##smgKEY"), &g_Options.LegitBot.smg_Key, ImVec2(120, 20));
								ImGui::SliderFloat(("Smooth"), &g_Options.LegitBot.smg_Smooth, 1.00f, 100.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Fov"), &g_Options.LegitBot.smg_fov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Rcs"), &g_Options.LegitBot.smgrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Text("Hitbox");
								ImGui::Combo(("##sniperhitbox"), &g_Options.LegitBot.smghitbox, legithitboxes, ARRAYSIZE(legithitboxes));
							}
							else if (g_Options.Menu.LegitTab == 5) // heavy
							{
								ImGui::Text("Heavy");
								ImGui::Text("Key");
								ImGui::Hotkey(("##heavyKEY"), &g_Options.LegitBot.heavy_wp_Key, ImVec2(120, 20));
								ImGui::SliderFloat(("Smooth"), &g_Options.LegitBot.heavy_wp_Smooth, 1.00f, 100.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Fov"), &g_Options.LegitBot.heavy_wp_fov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();
								ImGui::SliderFloat(("Rcs"), &g_Options.LegitBot.heavyrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Text("Hitbox");
								ImGui::Combo(("##sniperhitbox"), &g_Options.LegitBot.heavyhitbox, legithitboxes, ARRAYSIZE(legithitboxes));
							}
						}ImGui::EndChild();
					}
					if (g_Options.Menu.MenuTab == 2)//visuals
					{
						if (g_Options.Menu.VisualsTab == 1)
						{
							ImGui::Checkbox(("Enable Visuals"), &g_Options.Visuals.Enabled);
							ImGui::Text("General");
							ImGui::Spacing();
							ImGui::PushItemWidth(120);
							ImGui::Checkbox(("Draw Teammates"), &g_Options.Visuals.TeamESP);
							ImGui::Checkbox(("Box"), &g_Options.Visuals.Box);
							ImGui::SameLine();
							ImGui::ColorEdit3("Enemy##BOX", g_Options.Colors.EnemyESP, 1 << 5);
							ImGui::SameLine();
							ImGui::ColorEdit3("Team##BOX", g_Options.Colors.TeamESP, 1 << 5);
							ImGui::PushItemWidth(110);
							ImGui::Combo("##boxkek", &g_Options.Visuals.BoxType, "Full\0\rHotkey\0\rCorners\0\0", -1);
							if (g_Options.Visuals.BoxType == 0)
							{
								ImGui::Checkbox(("Fill Box"), &g_Options.Visuals.fill); 
								ImGui::SameLine();
								ImGui::ColorEdit3("Enemy##Fill", g_Options.Colors.fill_color_enemy, 1 << 5);
								ImGui::SameLine();
								ImGui::ColorEdit3("Team##Fill", g_Options.Colors.fill_color_team, 1 << 5);
								ImGui::SliderFloat(("Alpha##alphafill"), &g_Options.Visuals.esp_fill_amount, 0.f, 255.f, "%.0f");
							}
							else if (g_Options.Visuals.BoxType == 1)
							{
								ImGui::SameLine();
								ImGui::Hotkey("##Boxkey", &g_Options.Visuals.BoxHotkey, ImVec2(150, 16));
								ImGui::Checkbox(("Fill Box"), &g_Options.Visuals.fill);
								ImGui::SameLine();
								ImGui::ColorEdit3("Enemy##Fill", g_Options.Colors.fill_color_enemy, 1 << 5);
								ImGui::SameLine();
								ImGui::ColorEdit3("Team##Fill", g_Options.Colors.fill_color_team, 1 << 5);
								ImGui::SliderFloat(("Alpha##alphafill"), &g_Options.Visuals.esp_fill_amount, 0.f, 255.f, "%.0f");
							}
							else if (g_Options.Visuals.BoxType == 2)
							{
								ImGui::Checkbox(("Fill Box"), &g_Options.Visuals.fill);
								ImGui::SameLine();
								ImGui::ColorEdit3("Enemy##Fill", g_Options.Colors.fill_color_enemy, 1 << 5);
								ImGui::SameLine();
								ImGui::ColorEdit3("Team##Fill", g_Options.Colors.fill_color_team, 1 << 5);
								ImGui::SliderFloat(("Alpha##alphafill"), &g_Options.Visuals.esp_fill_amount, 0.f, 255.f, "%.0f");
							}
							ImGui::Checkbox(("Skeleton"), &g_Options.Visuals.skeletonenbl);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Skeleton", g_Options.Colors.color_skeleton, 1 << 5);
							ImGui::Checkbox(("Name"), &g_Options.Visuals.Name);
							ImGui::PushItemWidth(110);
							ImGui::Combo(("Weapon"), &g_Options.Visuals.Weapon, WeaponType, ARRAYSIZE(WeaponType));
							ImGui::Checkbox(("Health##Enable"), &g_Options.Visuals.health);
							ImGui::PopItemWidth();
							ImGui::Checkbox(("LBY Timer"), &g_Options.Visuals.LBYTimer);
							ImGui::Checkbox(("Armor Bar"), &g_Options.Visuals.armor);
							ImGui::Checkbox(("Aimlines"), &g_Options.Visuals.AimLine);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Aimlines", g_Options.Colors.AimLineColor, 1 << 5);
							ImGui::PushItemWidth(110);
							ImGui::Checkbox(("Damage Indicator"), &g_Options.Visuals.DamageIndicator);
							ImGui::SameLine();
							ImGui::ColorEdit3("##DamageIndicator", g_Options.Colors.damageindicator, 1 << 5);
							ImGui::Checkbox(("Aim Direction"), &g_Options.Visuals.barrel);
							ImGui::SameLine();
							ImGui::PushItemWidth(260);
							ImGui::ColorEdit3("##Direction", g_Options.Colors.BulletTraceColor, 1 << 5);
							ImGui::SameLine();
							ImGui::SliderInt(("Amount##barrel"), &g_Options.Visuals.barrelL, 0, 300);
							ImGui::PopItemWidth();
							ImGui::Checkbox(("Hitbox"), &g_Options.Visuals.Hitbox);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Hitbox", g_Options.Colors.hitbox, 1 << 5);
							ImGui::SameLine();
							ImGui::SliderFloat(("##duration"), &g_Options.Visuals.HitboxDuration, 0.f, 10.f, "Duration: %.0f");
							ImGui::PushItemWidth(160);
							ImGui::Checkbox(("Bullet Tracers"), &g_Options.Visuals.bulletshow);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Tracers", g_Options.Colors.flTracers, 1 << 5);
							ImGui::SliderFloat("Duration", &g_Options.Visuals.flTracersDuration, 0.f, 10.f, "%.0f");
							ImGui::SliderFloat("Width", &g_Options.Visuals.flTracersWidth, 0.f, 10.f, "%.0f");
							ImGui::Combo(("Beam Type"), &g_Options.Visuals.Beamtype, Beamtype, ARRAYSIZE(Beamtype));
							ImGui::Checkbox(("Offscreen Indicator"), &g_Options.Visuals.OffscreenIndicator);
							ImGui::SameLine();
							ImGui::ColorEdit3("##offscreen", g_Options.Colors.Offscreen, 1 << 5);
							ImGui::SliderInt("Size", &g_Options.Visuals.OffscreenIndicatorSize, 5, 35);
							ImGui::PopItemWidth();
						}
						else if (g_Options.Menu.VisualsTab == 2)
						{
							ImGui::Text("Enemy State");
							ImGui::Separator();
							ImGui::Checkbox(("Defuser"), &g_Options.Visuals.IsHasDefuser);
							ImGui::Checkbox(("Scoped"), &g_Options.Visuals.Scoped);
							ImGui::Checkbox(("Resolve Mode"), &g_Options.Visuals.resolveMode);
							ImGui::Checkbox(("Bomb-Carrier"), &g_Options.Visuals.BombCarrier);
							ImGui::Checkbox(("Flashed"), &g_Options.Visuals.Flashed);
							ImGui::Checkbox(("Distance"), &g_Options.Visuals.Distance);
							ImGui::Checkbox(("Money"), &g_Options.Visuals.Money);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("World");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::Text("Grenades");
							ImGui::Checkbox(("Bomb-ESP"), &g_Options.Visuals.C4World);
							ImGui::PushItemWidth(110);
							ImGui::Combo(("Grenades"), &g_Options.Visuals.Grenades, grenades, ARRAYSIZE(grenades));
							ImGui::SameLine();
							ImGui::Checkbox(("Box##GRENADE"), &g_Options.Visuals.GrenadeBox);
							ImGui::Checkbox(("Grenade-Prediction"), &g_Options.Visuals.GrenadePrediction);
							ImGui::SameLine();
							ImGui::ColorEdit3("Line##Grenadepred", g_Options.Colors.color_grenadeprediction, 1 << 5);
							ImGui::SameLine();
							ImGui::ColorEdit3("Circle##Grenadepred", g_Options.Colors.color_grenadeprediction_circle, 1 << 5);
							ImGui::SameLine();
							ImGui::ColorEdit3("Box##Grenadepred", g_Options.Colors.GrenadeCollision, 1 << 5);
							ImGui::PopItemWidth();
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Others");
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::PushItemWidth(110);
							ImGui::Checkbox(("Hostage"), &g_Options.Visuals.Hostage);
							ImGui::SameLine();
							ImGui::Checkbox(("Box##Hostage"), &g_Options.Visuals.HostageBox);
							ImGui::Checkbox(("Chicken"), &g_Options.Visuals.Chicken);
							ImGui::SameLine();
							ImGui::Checkbox(("Box##Chicken"), &g_Options.Visuals.ChickenBox);
							ImGui::Checkbox(("Dropped Guns"), &g_Options.Visuals.DroppedGuns);
							ImGui::SameLine();
							ImGui::Combo(("##droppedguns"), &g_Options.Visuals.DroppedGunsType, droppedWeapons, ARRAYSIZE(droppedWeapons));
							ImGui::SameLine();
							ImGui::ColorEdit3("##droppedgunscolor", g_Options.Colors.droppedguns, 1 << 5);
							ImGui::Checkbox(("NightMode"), &g_Options.Visuals.nightMode);
							ImGui::Checkbox(("AmbientLight"), &g_Options.Visuals.ambientlight);
							ImGui::SameLine();
							ImGui::ColorEdit3("##AmbientLightColor", g_Options.Colors.ambientlightcolor, 1 << 5);
							ImGui::Combo(("Skybox Changer"), &g_Options.Visuals.SkyboxChanger, skyboxchanger, ARRAYSIZE(skyboxchanger));
							ImGui::Checkbox(XorStr("Color Modulate Skybox"), &g_Options.Colors.ColorSkybox);
							ImGui::Combo("Color", &g_Options.Colors.SkyColor, SkyColor, ARRAYSIZE(SkyColor));
							ImGui::PopItemWidth();
						}
						else if (g_Options.Menu.VisualsTab == 3)
						{

							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Misc");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::Checkbox(("Hitmarker"), &g_Options.Misc.Hitmarker);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Hitmarker", g_Options.Colors.hitmarker_color, 1 << 5);
							ImGui::SameLine();
							ImGui::Combo(("Sound"), &g_Options.Misc.Hitsound, HitmarkSound, ARRAYSIZE(HitmarkSound));
							ImGui::Checkbox(("Snow Mode"), &g_Options.Visuals.snowmode);
							ImGui::Checkbox(("LSD Mode"), &g_Options.Visuals.lsdmode);
							ImGui::Checkbox(("Chrome Mode"), &g_Options.Visuals.chromemode);
							ImGui::Checkbox(("Minecraft Mode"), &g_Options.Visuals.minecraftmode);
							ImGui::Checkbox(("AUG & SG553 Scoped Blur Removal"), &g_Options.Visuals.ScopedBlurRemoval);
							ImGui::Checkbox(("Asus Props"), &g_Options.Visuals.AsusProps);
							ImGui::Checkbox(("Edgy Thirdperson Inner Glow Effect"), &g_Options.Visuals.EdgyTPThing);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Chams");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::Checkbox(("Enemy Chams"), &g_Options.Visuals.Chams);
							ImGui::SameLine();
							ImGui::ColorEdit3("Visible##enemychamscolor", g_Options.Colors.EnemyChamsVis, 1 << 5);
							ImGui::SameLine();
							ImGui::ColorEdit3("Invisible##enemychamscolor", g_Options.Colors.EnemyChamsNVis, 1 << 5);
							ImGui::Checkbox(("Team Chams"), &g_Options.Visuals.Teamchams);
							ImGui::SameLine();
							ImGui::ColorEdit3("Visible##teamchamscolor", g_Options.Colors.TeamChamsVis, 1 << 5);
							ImGui::SameLine();
							ImGui::ColorEdit3("Invisible##teamchamscolor", g_Options.Colors.TeamChamsNVis, 1 << 5);
							ImGui::Checkbox(("XQZ"), &g_Options.Visuals.XQZ);
							ImGui::SliderInt(("Chams-Alpha##chamsplayeraplha"), &g_Options.Visuals.champlayeralpha, 0, 100);
							ImGui::Checkbox(("FakeAngle"), &g_Options.Visuals.FakeAngleChams);
							ImGui::SameLine();
							ImGui::ColorEdit3("##fakeanglechams", g_Options.Colors.FakeAngleChams, 1 << 5);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Hands/Weapons");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::PushItemWidth(160);
							ImGui::Combo(("Hand Chams"), &g_Options.Visuals.Hands, HandsMode, ARRAYSIZE(HandsMode));
							ImGui::SameLine();
							ImGui::ColorEdit3("##handscolor", g_Options.Colors.HandsColor, 1 << 5);
							ImGui::SliderInt(("Alpha##HandsAlphaa"), &g_Options.Visuals.HandsAlpha, 0, 100);
							ImGui::Combo(("Weapon Chams"), &g_Options.Visuals.chamswphands, chamswp, ARRAYSIZE(chamswp));
							ImGui::SameLine();
							ImGui::ColorEdit3("##weaponcolor", g_Options.Colors.WeaponColor, 1 << 5);
							ImGui::SliderInt(("Alpha##chamswphands"), &g_Options.Visuals.chamswphandsalpha, 0, 100);
							ImGui::PopItemWidth();
						}
						else if (g_Options.Menu.VisualsTab == 4)
						{
							ImGui::Text("Glow");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::PushItemWidth(160);

							ImGui::Checkbox(("Glow Enable"), &g_Options.Visuals.GlowEnable);
							ImGui::Checkbox(("Glow Enemies"), &g_Options.Visuals.GlowEnemy);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Enemyglow", g_Options.Colors.EnemyGlow, 1 << 5);
							ImGui::Checkbox(("Glow Team"), &g_Options.Visuals.GlowTeam);
							ImGui::SameLine();
							ImGui::ColorEdit3("##Teamglow", g_Options.Colors.TeamGlow, 1 << 5);
							ImGui::SliderFloat(("Enemy Alpha"), &g_Options.Visuals.EnemyAlpha, 0.f, 255.f, "%.0f");
							ImGui::SliderFloat(("Team Alpha"), &g_Options.Visuals.TeamAlpha, 0.f, 255.f, "%.0f");

							ImGui::PopItemWidth();
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Rage");
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Checkbox(("Angle Lines"), &g_Options.Visuals.angleLines);
							ImGui::SameLine();
							ImGui::Checkbox(("Name##ANGLELINE"), &g_Options.Visuals.angleLinesName);
							ImGui::Checkbox(("LBY Indicator"), &g_Options.Visuals.LBYIndicator);
							ImGui::Checkbox(("Manual AA Indicator"), &g_Options.Visuals.ManualAAIndicator);
							ImGui::SameLine();
							ImGui::ColorEdit3("##manualindicator", g_Options.Colors.ManualIndicator, 1 << 5);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Thirdperson");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::PushItemWidth(150);
							ImGui::Checkbox("Enable Thirdperson", &g_Options.Visuals.Enabletp);
							ImGui::Hotkey("##KeyTP", &g_Options.Visuals.TPKey, ImVec2(150, 20));
							ImGui::SameLine();
							ImGui::Text("Key");
							ImGui::Combo("Thirdperson Angle", &g_Options.Visuals.antiaim_thirdperson_angle, AntiAimThirdperson, ARRAYSIZE(AntiAimThirdperson));
							ImGui::Checkbox("Disable On Grenade", &g_Options.Visuals.GrenadeCheck);


							ImGui::PopItemWidth();
							
						}
						else if (g_Options.Menu.VisualsTab == 5)
						{
							ImGui::Text("Crosshairs");
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::PushItemWidth(110);
							ImGui::Checkbox(("Auto-Wall Crosshair"), &g_Options.Visuals.AWallCrosshair);
							ImGui::Checkbox(("Sniper Crosshair"), &g_Options.Visuals.SniperCrosshair);
							ImGui::SameLine();
							ImGui::Combo(("##SNIPERCROSS"), &g_Options.Visuals.SniperCrosshairType, snipercrosshair, ARRAYSIZE(snipercrosshair));
							ImGui::PopItemWidth();
							ImGui::PushItemWidth(110);
							ImGui::Checkbox(("Recoil Crosshair"), &g_Options.Visuals.RecoilCrosshair);
							ImGui::SameLine();
							ImGui::Combo(("##Type"), &g_Options.Visuals.RecoilCrosshair2, rccrosshair, ARRAYSIZE(rccrosshair));
							ImGui::PopItemWidth();
							ImGui::PushItemWidth(110);
							ImGui::Checkbox(("Spread Crosshair"), &g_Options.Visuals.SpreadCrosshair);
							ImGui::SliderFloat(("Spread Alpha##alphafill"), &g_Options.Visuals.spread_crosshair_amount, 0, 255, "%.0f");
							ImGui::PopItemWidth();
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Removals");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::PushItemWidth(110);
							ImGui::Combo(("Smoke"), &g_Options.Visuals.Smoke, smoke, ARRAYSIZE(smoke));
							ImGui::Checkbox(("No Flash"), &g_Options.Visuals.NoFlash);
							ImGui::Checkbox(("No Scope"), &g_Options.Visuals.noscopeborder);
							ImGui::Checkbox(("No Visual Recoil"), &g_Options.Visuals.NoVisualRecoil);
							ImGui::Checkbox(("No Post Processing"), &g_Options.Visuals.NoPostProcess);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("FOV");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::PushItemWidth(180);
							ImGui::Checkbox(("Enable-FOVInGame"), &g_Options.Visuals.FOVChanger_enabled);
							ImGui::SliderFloat(("FOV"), &g_Options.Visuals.FOVChanger, 0, 120, "%.0f");
							ImGui::Checkbox(("Enable-viewmodelFOV"), &g_Options.Visuals.viewmodelChanger_enabled);
							ImGui::SliderFloat(("VFOV"), &g_Options.Visuals.viewmodelChanger, 0, 130, "%.0f");
							ImGui::PopItemWidth();
						}
					}
					if (g_Options.Menu.MenuTab == 3)//misc
					{
						if (g_Options.Menu.MiscTab == 1)
						{
							ImGui::PushItemWidth(100);
							ImGui::Checkbox(("Anti Untrusted"), &g_Options.Misc.antiuntrusted);
							ImGui::Combo(("File"), &g_Options.Menu.ConfigFile, configFiles, ARRAYSIZE(configFiles));
							ImGui::Spacing();
							if (ImGui::Button("Save Config", ImVec2(100, 25)))
								Config->Save();
							if (ImGui::Button("Load Config", ImVec2(100, 25)))
								Config->Load();
							ImGui::Checkbox(("Bunny Hop"), &g_Options.Misc.Bhop);
							ImGui::Combo(("Auto Strafe"), &g_Options.Misc.AutoStrafe, autostrafe, ARRAYSIZE(autostrafe));
							ImGui::Checkbox(("Ping Spike"), &g_Options.Misc.FakePing);
							ImGui::Combo(("##FakePing"), &g_Options.Misc.FakePingType, fakeping, ARRAYSIZE(fakeping));
							if (g_Options.Misc.FakePingType == 2)
							{
								ImGui::Hotkey(("Ping Spike Key##pingspike"), &g_Options.Misc.FakePingKey, ImVec2(120, 20));
							}
							ImGui::Text("Fake Ping Amount");
							ImGui::SliderInt("##fakepingshitzz", &g_Options.Misc.FakePingAmmnt, 0, 800);
							ImGui::Checkbox(("Watermark"), &g_Options.Misc.Watermark);
							ImGui::Checkbox(("Spectator List"), &g_Options.Misc.SpecList);
							ImGui::Checkbox(("Radar"), &g_Options.Misc.radaringame);
							ImGui::Checkbox(("Event Logs"), &g_Options.Misc.eventlogs);
							if (ImGui::Checkbox(("AfkBot"), &g_Options.Misc.afkbot))
							{
								if (g_Options.Misc.afkbot)
								{

									g_Engine->ClientCmd_Unrestricted("+forward;+moveleft;+left");

								}
								else if (!g_Options.Misc.afkbot)
								{
									g_Engine->ClientCmd_Unrestricted("-forward;-moveleft;-left");
								}
							}
							ImGui::Checkbox("Enable Inventory", &g_Options.Misc.inventoryalwayson);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Enables the inventory in-game (competitive games, etc.)");
						}
						else if (g_Options.Menu.MiscTab == 2)
						{
							ImGui::Spacing();
							ImGui::Text("Nickname");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::PushItemWidth(230);
							static char nickname[127] = "";
							ImGui::InputText("##NICKNAME", nickname, 127);
							ImGui::PopItemWidth();
							if (ImGui::Button("Set Nickname", ImVec2(230, 20)))
							{
								NameChanger::SetName(std::string(nickname).c_str());
							}
							ImGui::Text("Chat Spam");
							ImGui::PushItemWidth(230);
							ImGui::Checkbox(("Chat Spammer"), &g_Options.Misc.spammeron);
							ImGui::Combo(("SpammerList##chatspam"), &g_Options.Misc.spammer, spammerlist, ARRAYSIZE(spammerlist));
							ImGui::PopItemWidth();
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Clantag");
							ImGui::Checkbox(("Clantag"), &g_Options.Misc.syncclantag);
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Text("Buy Bot");
							ImGui::Separator();
							ImGui::Spacing();
							ImGui::Checkbox("Enabled##BuyBot", &g_Options.Misc.BuyBot);
							ImGui::Checkbox("Buy Defuser##BuyBot", &g_Options.Misc.BuyBotDefuse);
							ImGui::Checkbox("Buy Zeus##BuyBot", &g_Options.Misc.BuyBotZeus);
							ImGui::Combo(("Weapons##BuyBot"), &g_Options.Misc.BuyBotWeap, BuyBotWeap, ARRAYSIZE(BuyBotWeap));
							ImGui::Combo(("Grenades##BuyBot"), &g_Options.Misc.BuyBotGrenade, BuyBotNade, ARRAYSIZE(BuyBotNade));
							ImGui::Combo(("Armor##BuyBot"), &g_Options.Misc.BuyBotKevlarHelmet, BuyBotArmor, ARRAYSIZE(BuyBotArmor));
						}
					}
				}ImGui::EndChild();
			}ImGui::End();
		}
	
}
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
#include "SkinParser.h"
#include "SkinFilter.h"
#include <vector>

#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define BUILDSTAMP ( __DATE__ )



//========Clantag====================

void SetClantag(const char *tag)
{
	static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))U::pattern_scan(GetModuleHandle("engine.dll"), "53 56 57 8B DA 8B F9 FF 15");

	fnClantagChanged(tag, tag);
}
//--------------------------------------------
//Weapon CFG stuff

const char* weaponnames(int id)
{
	switch (id)
	{
	case 1:
		return "deagle";
	case 2:
		return "elite";
	case 3:
		return "fiveseven";
	case 4:
		return "glock";
	case 7:
		return "ak47";
	case 8:
		return "aug";
	case 9:
		return "awp";
	case 10:
		return "famas";
	case 11:
		return "g3sg1";
	case 13:
		return "galilar";
	case 14:
		return "m249";
	case 60:
		return "m4a1_silencer";
	case 16:
		return "m4a1";
	case 17:
		return "mac10";
	case 19:
		return "p90";
	case 24:
		return "ump45";
	case 25:
		return "xm1014";
	case 26:
		return "bizon";
	case 27:
		return "mag7";
	case 28:
		return "negev";
	case 29:
		return "sawedoff";
	case 30:
		return "tec9";
	case 32:
		return "hkp2000";
	case 33:
		return "mp7";
	case 34:
		return "mp9";
	case 35:
		return "nova";
	case 36:
		return "p250";
	case 38:
		return "scar20";
	case 39:
		return "sg556";
	case 40:
		return "ssg08";
	case 61:
		return "usp_silencer";
	case 63:
		return "cz75a";
	case 64:
		return "revolver";
	case 508:
		return "knife_m9_bayonet";
	case 500:
		return "bayonet";
	case 505:
		return "knife_flip";
	case 506:
		return "knife_gut";
	case 507:
		return "knife_karambit";
	case 509:
		return "knife_tactical";
	case 512:
		return "knife_falchion";
	case 514:
		return "knife_survival_bowie";
	case 515:
		return "knife_butterfly";
	case 516:
		return "knife_push";

	default:
		return "";
	}
}

bool IsUtility(ItemDefinitionIndexx index)
{
	switch (index)
	{
	case ItemDefinitionIndexx::ITEM_NONE:
	case ItemDefinitionIndexx::WEAPON_C4:
	case ItemDefinitionIndexx::WEAPON_FLASH:
	case ItemDefinitionIndexx::WEAPON_HE:
	case ItemDefinitionIndexx::WEAPON_INC:
	case ItemDefinitionIndexx::WEAPON_MOLOTOV:
	case ItemDefinitionIndexx::WEAPON_SMOKE:
	case ItemDefinitionIndexx::WEAPON_DECOY:
	case ItemDefinitionIndexx::WEAPON_TASER:
	case ItemDefinitionIndexx::WEAPON_KNIFE_T:
	case ItemDefinitionIndexx::WEAPON_KNIFE_CT:
	case ItemDefinitionIndexx::GLOVE_T_SIDE:
	case ItemDefinitionIndexx::GLOVE_CT_SIDE:
	case ItemDefinitionIndexx::GLOVE_SPORTY:
	case ItemDefinitionIndexx::GLOVE_SLICK:
	case ItemDefinitionIndexx::GLOVE_LEATHER_WRAP:
	case ItemDefinitionIndexx::GLOVE_STUDDED_BLOODHOUND:
	case ItemDefinitionIndexx::GLOVE_MOTORCYCLE:
	case ItemDefinitionIndexx::GLOVE_SPECIALIST:
	case ItemDefinitionIndexx::GLOVE_HYDRA:
		return true;
	default:
		return false;
	}
}

bool Contains(const std::string &word, const std::string &sentence) {
	if (word == "" || sentence == "")
		return true;

	return sentence.find(word) != std::string::npos;
}

std::string ToLower(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))std::tolower);

	return str;
}
//--------------------------------------------
bool defaultk;
bool bayonet;
bool flip;
bool gut;
bool karambit;
bool m9bayonet;
bool huntsman;
bool falchion;
bool bowie;
bool butterfly;
bool daggers;

void doknives(bool* disknife)
{
	defaultk = false;
	bayonet = false;
	flip = false;
	gut = false;
	karambit = false;
	m9bayonet = false;
	huntsman = false;
	falchion = false;
	bowie = false;
	butterfly = false;
	daggers = false;

	*disknife = true;
}




int ParseSkins()
{
	valve_parser::Document doc;
	auto r = doc.Load(".\\csgo\\scripts\\items\\items_game.txt", valve_parser::ENCODING::UTF8);
	if (!r)
		return 1;

	valve_parser::Document english;
	r = english.Load(".\\csgo\\resource\\csgo_english.txt", valve_parser::ENCODING::UTF16_LE);
	if (!r)
		return 2;

	auto weaponSkinCombo = doc.BreadthFirstSearch("weapon_icons");
	if (!weaponSkinCombo || !weaponSkinCombo->ToObject())
		return 3;

	auto skinDataVec = doc.BreadthFirstSearchMultiple("paint_kits");
	if (!skinDataVec.size())
		return 4;

	auto PaintKitNames = english.BreadthFirstSearch("Tokens");
	if (!PaintKitNames || !PaintKitNames->ToObject())
		return 5;

	//std::unordered_map<std::string, std::set<std::string>> G::weaponSkins;
	//std::unordered_map<std::string, skinInfo> G::skinMap;
	//std::unordered_map<std::string, std::string> G::skinNames;

	std::vector<std::string> weaponNames = {
		"deagle",
		"elite",
		"fiveseven",
		"glock",
		"ak47",
		"aug",
		"awp",
		"famas",
		"g3sg1",
		"galilar",
		"m249",
		"m4a1_silencer", //needs to be before m4a1 else silencer doesnt get filtered out :D
		"m4a1",
		"mac10",
		"p90",
		"ump45",
		"xm1014",
		"bizon",
		"mag7",
		"negev",
		"sawedoff",
		"tec9",
		"hkp2000",
		"mp7",
		"mp9",
		"nova",
		"p250",
		"scar20",
		"sg556",
		"ssg08",
		"usp_silencer",
		"cz75a",
		"revolver",
		"knife_m9_bayonet", //needs to be before bayonet else knife_m9_bayonet doesnt get filtered out :D
		"bayonet",
		"knife_flip",
		"knife_gut",
		"knife_karambit",
		"knife_tactical",
		"knife_falchion",
		"knife_survival_bowie",
		"knife_butterfly",
		"knife_push",
		"studded_bloodhound_gloves",
		"sporty_gloves",
		"slick_gloves",
		"leather_handwraps",
		"motorcycle_gloves",
		"specialist_gloves"

	};

	//populate G::weaponSkins
	for (auto child : weaponSkinCombo->children)
	{
		if (child->ToObject())
		{
			for (auto weapon : weaponNames)
			{
				auto skinName = child->ToObject()->GetKeyByName("icon_path")->ToKeyValue()->Value.toString();
				auto pos = skinName.find(weapon);
				//filter out the skinname
				if (pos != std::string::npos)
				{
					auto pos2 = skinName.find_last_of('_');
					g_Options.Skinchanger.weaponSkins[weapon].insert(
						skinName.substr(pos + weapon.length() + 1,
							pos2 - pos - weapon.length() - 1)
					);
					break;
				}
			}
		}
	}

	//populate skinData
	for (auto skinData : skinDataVec)
	{
		if (skinData->ToObject())
		{
			for (auto skin : skinData->children)
			{
				if (skin->ToObject())
				{
					skinInfo si;
					si.paintkit = skin->ToObject()->name.toInt();

					auto skinName = skin->ToObject()->GetKeyByName("name")->ToKeyValue()->Value.toString();
					auto tagNode = skin->ToObject()->GetKeyByName("description_tag");
					if (tagNode)
					{
						std::string tag = tagNode->ToKeyValue()->Value.toString();
						tag = tag.substr(1, std::string::npos); //skip #
						std::transform(tag.begin(), tag.end(), tag.begin(), towlower);
						si.tagName = tag;
					}

					auto keyVal = skin->ToObject()->GetKeyByName("seed");
					if (keyVal != nullptr)
						si.seed = keyVal->ToKeyValue()->Value.toInt();

					g_Options.Skinchanger.skinMap[skinName] = si;
				}
			}
		}
	}

	//populate G::skinNames
	for (auto child : PaintKitNames->children)
	{
		if (child->ToKeyValue())
		{
			std::string key = child->ToKeyValue()->Key.toString();
			std::transform(key.begin(), key.end(), key.begin(), towlower);
			if (key.find("paintkit") != std::string::npos &&
				key.find("tag") != std::string::npos)
			{
				g_Options.Skinchanger.skinNames[key] = child->ToKeyValue()->Value.toString();
			}
		}
	}

	return 0;
}



void initializeskins()
{
	static bool once = false;

	if (!once)
	{
		ParseSkins();
		once = true;
	}
}


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

void BtnMenu()
{
	auto& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.13f, 0.13f, 0.13f, 0.95f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.13f, 0.13f, 0.13f, 0.95f);
}

namespace CarceaHOOK
{

	ImFont* fDefault;
	ImFont* fkek;
	ImFont* fkek2;

	void GUI_Init(HWND window, IDirect3DDevice9 *pDevice)
	{

		if (ImGui_ImplDX9_Init(window, pDevice))
		{

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			fDefault = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 14.0f);
			fkek = io.Fonts->AddFontFromMemoryCompressedTTF(myfont4_compressed_data, myfont4_compressed_size, 42.f);
			fkek2 = io.Fonts->AddFontFromMemoryCompressedTTF(myfont3_compressed_data, myfont3_compressed_size, 20.f);


			auto& styled = ImGui::GetStyle();


			G::extra_flags = 0;

			static int hue = 140;

			ImVec4 col_text = ImColor::HSV(hue / 255.f, 20.f / 255.f, 235.f / 255.f);
			ImVec4 col_main = ImVec4(0.09f, .09f, .09f, 1.f);
			ImVec4 col_back = ImColor(31, 44, 54);
			ImVec4 col_area = ImColor(4, 32, 41);

			styled.Colors[ImGuiCol_Text] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
			styled.Colors[ImGuiCol_TextDisabled] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
			styled.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_BorderShadow] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_FrameBg] = ImVec4(0.09f, .09f, .09f, 1.f);
			styled.Colors[ImGuiCol_FrameBgb1g] = ImVec4(0.16f, .16f, .16f, 1.f);; //checkbox bg and shit
			styled.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.13, 0.13, 0.13, 1.f);
			styled.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.11, 0.11, 0.11, 1.f);
			styled.Colors[ImGuiCol_TitleBg] = ImVec4(.78f, 0.f, 0.f, .7f);
			styled.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(.78f, 0.f, 0.f, .7f);
			styled.Colors[ImGuiCol_TitleBgActive] = ImVec4(.78f, 0.f, 0.f, .7f);
			styled.Colors[ImGuiCol_MenuBarBg] = ImVec4(.78f, 0.f, 0.f, .7f);
			styled.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_CheckMark] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_SliderGrab] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_SliderGrabActive] = ImVec4(.78f, 0.f, 0.f, 1.f);
			styled.Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
			styled.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
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
			style.WindowMinSize = ImVec2(32, 32);
			style.WindowRounding = 0.5f;
			style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
			style.FramePadding = ImVec2(4, 2);
			style.FrameRounding = 0.0f;
			style.ItemSpacing = ImVec2(8, 4);
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

		int leftmenusize_x = 150;
		int leftmenusize_y = screen_y + 1;
		int leftmenupos_x = 0;
		int leftmenupos_y = 0;

		int tabmenusize_x = 150;
		int tabmenusize_y = 272;
		int tabmenupos_x = 0;
		int tabmenupos_y = screen_y / 2 - tabmenusize_y / 2;


		style.WindowPadding = ImVec2(0, 0);
		style.ItemSpacing = ImVec2(0, 0);
		ImGui::SetNextWindowSize(ImVec2(leftmenusize_x, leftmenusize_y));
		ImGui::SetNextWindowPos(ImVec2(leftmenupos_x, leftmenupos_y));
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.95f);
		if (ImGui::Begin("Tab Selector Background Chestie", &g_Options.Menu.Opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs)); {
		}ImGui::End();
		style.WindowPadding = ImVec2(0, 0);
		style.ItemSpacing = ImVec2(1, 1);
		ImGui::SetNextWindowSize(ImVec2(tabmenusize_x, tabmenusize_y));
		ImGui::SetNextWindowPos(ImVec2(tabmenupos_x, tabmenupos_y));
		if (ImGui::Begin("Tab Selector ala", &g_Options.Menu.Opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)); {

			ImGui::BeginChild("tab child", ImVec2(-1, -1), false);
			{
			ImGui::PushItemWidth(-1);
			if (g_Options.Menu.MenuTab == 1) BtnActive(); else BtnNormal();
			if (ImGui::Button("Rage", ImVec2(150, 25))) g_Options.Menu.MenuTab = 1;
			if (g_Options.Menu.MenuTab == 2) BtnActive(); else BtnNormal();
			if (ImGui::Button("Anti-Aim", ImVec2(150, 25))) g_Options.Menu.MenuTab = 2;
			if (g_Options.Menu.MenuTab == 3) BtnActive(); else BtnNormal();
			if (ImGui::Button("Legit", ImVec2(150, 25))) g_Options.Menu.MenuTab = 3;
			if (g_Options.Menu.MenuTab == 4) BtnActive(); else BtnNormal();
			if (ImGui::Button("Visuals", ImVec2(150, 25))) g_Options.Menu.MenuTab = 4;
			if (g_Options.Menu.MenuTab == 5) BtnActive(); else BtnNormal();
			if (ImGui::Button("Misc", ImVec2(150, 25))) g_Options.Menu.MenuTab = 5;
			if (g_Options.Menu.MenuTab == 6) BtnActive(); else BtnNormal();
			if (ImGui::Button("Skins", ImVec2(150, 25))) g_Options.Menu.MenuTab = 6;
			if (g_Options.Menu.MenuTab == 7) BtnActive(); else BtnNormal();
			if (ImGui::Button("Colors", ImVec2(150, 25))) g_Options.Menu.MenuTab = 7;
			BtnNormal();
			ImGui::Spacing();
			ImGui::Text("				Config");
			ImGui::Spacing();
			ImGui::Combo(("File"), &g_Options.Menu.ConfigFile, configFiles, ARRAYSIZE(configFiles));
			ImGui::Spacing();
			if (ImGui::Button("Save Config", ImVec2(150, 25)))
			Config->Save();
			if (ImGui::Button("Load Config", ImVec2(150, 25)))
			Config->Load();
			}ImGui::EndChild();
		}ImGui::End();
		if (g_Options.Menu.MenuTab > 0)
		{
			
			ImGui::SetNextWindowSize(ImVec2(660.f, 615.f));
			ImGui::SetNextWindowPosCenter(ImGuiSetCond_Appearing);
			if (ImGui::Begin("CarceaHOOK.xyz", &g_Options.Menu.Opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse)); {
				style.WindowPadding = ImVec2(8, 8);
				style.ItemSpacing = ImVec2(8, 4);
				
				ImGui::BeginChild("main child", ImVec2(-1, -1), true);
				{
					if (g_Options.Menu.MenuTab == 1)
					{
						BtnMenu();
						ImGui::PushItemWidth(180);
						ImGui::Checkbox(("Main Switch"), &g_Options.Ragebot.MainSwitch);
						ImGui::Checkbox(("Enable Aimbot"), &g_Options.Ragebot.Enabled);
						ImGui::Checkbox(("Auto Fire"), &g_Options.Ragebot.AutoFire);
						ImGui::SliderFloat(("FOV"), &g_Options.Ragebot.FOV, 1.f, 180.f, "%.0f");
						ImGui::Hotkey(("##Ragebot-Key"), &g_Options.Ragebot.KeyPress, ImVec2(150, 25));
						ImGui::SameLine();
						ImGui::Text("Key");
						ImGui::PopItemWidth();
						ImGui::Checkbox(("Silent Aim"), &g_Options.Ragebot.Silent);
						ImGui::Checkbox(("Auto Pistol"), &g_Options.Ragebot.AutoPistol);
						ImGui::Checkbox(("Backtrack"), &g_Options.Ragebot.Backtrack);
						ImGui::Checkbox(("Fakelag"), &g_Options.Ragebot.FakeLag);
						ImGui::SliderInt(("##Fakelag"), &g_Options.Ragebot.FakeLagAmt, 1.f, 15.f);
						ImGui::Checkbox(("No Recoil"), &g_Options.Ragebot.AntiRecoil);
						ImGui::Combo(("Auto Stop"), &g_Options.Ragebot.AutoStop, autostop, ARRAYSIZE(autostop));
						ImGui::Checkbox(("Auto Crouch"), &g_Options.Ragebot.AutoCrouch);
						ImGui::Checkbox(("Auto Scope"), &g_Options.Ragebot.AutoScope);
						ImGui::Checkbox(("Fakewalk"), &g_Options.Ragebot.fakewalk);
						ImGui::Hotkey(("##fakewalk key"), &g_Options.Ragebot.fakewalkkey, ImVec2(125, 25));
						ImGui::Separator();
						ImGui::PushItemWidth(120);
						ImGui::Text("Minimum Damage");
						ImGui::Separator();
						ImGui::SliderFloat(("Sniper Min Dmg"), &g_Options.Ragebot.MinimumDamageSniper, 1.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Rifle Min Dmg"), &g_Options.Ragebot.MinimumDamageRifle, 1.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Pistol Min Dmg"), &g_Options.Ragebot.MinimumDamagePistol, 1.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Heavy Min Dmg"), &g_Options.Ragebot.MinimumDamageHeavy, 1.f, 100.f, "%.0f");
						ImGui::SliderFloat(("SMG Min Dmg"), &g_Options.Ragebot.MinimumDamageSmg, 1.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Revolver/Deagle Min Dmg"), &g_Options.Ragebot.MinimumDamageRevolver, 1.f, 100.f, "%.0f");
						ImGui::PopItemWidth();
						ImGui::Text(("Aimbot-Accuracy"));
						ImGui::Separator();
						ImGui::Checkbox(("Friendly Fire"), &g_Options.Ragebot.FriendlyFire);
						ImGui::PushItemWidth(150);
						ImGui::Text("If hitscan is off the aimbot will aim at the selected hitbox");
						ImGui::Combo(("Hitbox"), &g_Options.Ragebot.Hitbox, aimBones, ARRAYSIZE(aimBones));
						ImGui::Combo(("Hitscan"), &g_Options.Ragebot.Hitscan, hitscan, ARRAYSIZE(hitscan));
						ImGui::PopItemWidth();
						ImGui::Separator();
						ImGui::PushItemWidth(120);
						ImGui::Checkbox(("Hitchance Enabled"), &g_Options.Ragebot.Hitchance);
						ImGui::SliderFloat(("Sniper Hitchance"), &g_Options.Ragebot.HitchanceSniper, 0.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Rifle Hitchance"), &g_Options.Ragebot.HitchanceRifle, 0.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Pistol Hitchance"), &g_Options.Ragebot.HitchancePistol, 0.f, 100.f, "%.0f");
						ImGui::SliderFloat(("SMG Hitchance"), &g_Options.Ragebot.HitchanceSmgs, 0.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Heavy Hitchance"), &g_Options.Ragebot.HitchanceHeavy, 0.f, 100.f, "%.0f");
						ImGui::SliderFloat(("Revolver / Deagle Hitchance"), &g_Options.Ragebot.HitchanceRevolver, 0.f, 100.f, "%.0f");
						ImGui::PopItemWidth();
						ImGui::PushItemWidth(110);
						ImGui::Combo(("Resolver"), &g_Options.Ragebot.Resolver, resolvers, ARRAYSIZE(resolvers));
						ImGui::Text(("Bruteforce after X bullets:"));
						ImGui::SliderFloat(("Shots: "), &g_Options.Ragebot.bruteAfterX, 0, 10, "%1.f");
						ImGui::Checkbox(("BAIM If Lethal"), &g_Options.Ragebot.BAIMIfLethal);

						ImGui::PopItemWidth();
					}
					else if (g_Options.Menu.MenuTab == 2)
					{
						BtnMenu();
						ImGui::Checkbox(("Anti-Aim Enabled"), &g_Options.Ragebot.EnabledAntiAim);
						ImGui::Checkbox(("Freestanding"), &g_Options.Ragebot.Edge);

						static int movementtype = 0;

						if (ImGui::Button("Moving", ImVec2(115, 25)))
							movementtype = 0;
						ImGui::SameLine();
						if (ImGui::Button("Stand", ImVec2(115, 25)))
							movementtype = 1;
						ImGui::SameLine();
						if (ImGui::Button("Air", ImVec2(115, 25)))
							movementtype = 2;

						static int aatabtype = 0;
						static int aatabtype1 = 0;
						static int aatabtype2 = 0;
						static int aatabtype3 = 0;
						static int aatabtype4 = 0;

						if (movementtype == 0)
						{
							ImGui::Checkbox(("Moving AA"), &g_Options.Ragebot.AA_onWalk);


							if (ImGui::Button("Presets", ImVec2(150, 25))) aatabtype = 0;
							ImGui::SameLine();
							if (ImGui::Button("Self Made", ImVec2(150, 25))) aatabtype = 1;

							if (aatabtype == 0) //walk_
							{
								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.walk_PreAAs);
								ImGui::Combo(("Pitch"), &g_Options.Ragebot.walk_Pitch, antiaimpitch, ARRAYSIZE(antiaimpitch));
								ImGui::SliderFloat(("Pitch Adder: "), &g_Options.Ragebot.walk_PitchAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Yaw"), &g_Options.Ragebot.walk_YawTrue, antiaimyawtrue, ARRAYSIZE(antiaimyawtrue));
								ImGui::SliderFloat(("Real Adder: "), &g_Options.Ragebot.walk_YawTrueAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Fake-Yaw"), &g_Options.Ragebot.walk_YawFake, antiaimyawfake, ARRAYSIZE(antiaimyawfake));
								ImGui::SliderFloat(("Fake Adder: "), &g_Options.Ragebot.walk_YawFakeAdder, -180, 180, "%1.f");
								ImGui::Hotkey(("Manual AA Right##AARight"), &g_Options.Ragebot.manualrightkey, ImVec2(150, 25));
								ImGui::Hotkey(("Manual AA Left##AALeft"), &g_Options.Ragebot.manualleftkey, ImVec2(150, 25));
								ImGui::Hotkey(("Manual AA Back##AABack"), &g_Options.Ragebot.manualbackkey, ImVec2(150, 25));
							}
							else if (aatabtype == 1)
							{
								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.walk_BuilderAAs);
								ImGui::SliderFloat(("Pitch Angle: "), &g_Options.Ragebot.walk_BuilderPitch, -89, 89, "%1.f");
								ImGui::SliderFloat(("Real Angle: "), &g_Options.Ragebot.walk_BuilderReal, -180, 180, "%1.f");
								ImGui::SliderFloat(("Fake Angle: "), &g_Options.Ragebot.walk_BuilderFake, -180, 180, "%1.f");
								ImGui::Checkbox(("Enable Real Jitter"), &g_Options.Ragebot.walk_Jitter);
								ImGui::SliderFloat(("Jitter Range: "), &g_Options.Ragebot.walk_JitterRange, -90, 90, "%1.f");
								ImGui::Checkbox(("Enable Fake Jitter"), &g_Options.Ragebot.walk_FJitter);
								ImGui::SliderFloat(("FJitter Range: "), &g_Options.Ragebot.walk_FJitterRange, -90, 90, "%1.f");
								ImGui::Checkbox(("LBY Breaker"), &g_Options.Ragebot.walk_LBYBreaker);
							}
						}
						else if (movementtype == 1)
						{
							ImGui::Checkbox(("Standing AA"), &g_Options.Ragebot.AA_onStand);



							if (ImGui::Button("Presets", ImVec2(150, 25))) aatabtype2 = 0;
							ImGui::SameLine();
							if (ImGui::Button("Self Made", ImVec2(150, 25))) aatabtype2 = 1;

							if (aatabtype2 == 0) //stand_
							{
								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.stand_PreAAs);
								ImGui::Combo(("Pitch"), &g_Options.Ragebot.stand_Pitch, antiaimpitch, ARRAYSIZE(antiaimpitch));
								ImGui::SliderFloat(("Pitch Adder: "), &g_Options.Ragebot.stand_PitchAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Yaw"), &g_Options.Ragebot.stand_YawTrue, antiaimyawtrue, ARRAYSIZE(antiaimyawtrue));
								ImGui::SliderFloat(("Real Adder: "), &g_Options.Ragebot.stand_YawTrueAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Fake-Yaw"), &g_Options.Ragebot.stand_YawFake, antiaimyawfake, ARRAYSIZE(antiaimyawfake));
								ImGui::SliderFloat(("Fake Adder: "), &g_Options.Ragebot.stand_YawFakeAdder, -180, 180, "%1.f");
								ImGui::Hotkey(("Manual AA Right##AARight"), &g_Options.Ragebot.manualrightkey, ImVec2(150, 25));
								ImGui::Hotkey(("Manual AA Left##AALeft"), &g_Options.Ragebot.manualleftkey, ImVec2(150, 25));
								ImGui::Hotkey(("Manual AA Back##AABack"), &g_Options.Ragebot.manualbackkey, ImVec2(150, 25));
							}
							else if (aatabtype2 == 1)
							{

								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.stand_BuilderAAs);
								ImGui::SliderFloat(("Pitch Angle: "), &g_Options.Ragebot.stand_BuilderPitch, -89, 89, "%1.f");
								ImGui::SliderFloat(("Real Angle: "), &g_Options.Ragebot.stand_BuilderReal, -180, 180, "%1.f");
								ImGui::SliderFloat(("Fake Angle: "), &g_Options.Ragebot.stand_BuilderFake, -180, 180, "%1.f");
								ImGui::Checkbox(("Enable Real Jitter"), &g_Options.Ragebot.stand_Jitter);
								ImGui::SliderFloat(("Jitter Range: "), &g_Options.Ragebot.stand_JitterRange, -90, 90, "%1.f");
								ImGui::Checkbox(("Enable Fake Jitter"), &g_Options.Ragebot.stand_FJitter);
								ImGui::SliderFloat(("FJitter Range: "), &g_Options.Ragebot.stand_FJitterRange, -90, 90, "%1.f");
								ImGui::Checkbox(("LBY Breaker"), &g_Options.Ragebot.stand_LBYBreaker);
							}
						}
						else if (movementtype == 2)
						{
							ImGui::Checkbox(("Air AA"), &g_Options.Ragebot.AA_onAir);



							if (ImGui::Button("Presets", ImVec2(150, 25))) aatabtype2 = 0;
							ImGui::SameLine();
							if (ImGui::Button("Self Made", ImVec2(150, 25))) aatabtype2 = 1;

							if (aatabtype2 == 0) //air_
							{
								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.air_PreAAs);
								ImGui::Combo(("Pitch"), &g_Options.Ragebot.air_Pitch, antiaimpitch, ARRAYSIZE(antiaimpitch));
								ImGui::SliderFloat(("Pitch Adder: "), &g_Options.Ragebot.air_PitchAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Yaw"), &g_Options.Ragebot.air_YawTrue, antiaimairyawtrue, ARRAYSIZE(antiaimairyawtrue));
								ImGui::SliderFloat(("Real Adder: "), &g_Options.Ragebot.air_YawTrueAdder, -180, 180, "%1.f");
								ImGui::Separator();
								ImGui::Combo(("Fake-Yaw"), &g_Options.Ragebot.air_YawFake, antiaimyawfake, ARRAYSIZE(antiaimyawfake));
								ImGui::SliderFloat(("Fake Adder: "), &g_Options.Ragebot.air_YawFakeAdder, -180, 180, "%1.f");
								ImGui::Hotkey(("Manual AA Right##AARight"), &g_Options.Ragebot.manualrightkey, ImVec2(150, 25));
								ImGui::Hotkey(("Manual AA Left##AALeft"), &g_Options.Ragebot.manualleftkey, ImVec2(150, 25));
								ImGui::Hotkey(("Manual AA Back##AABack"), &g_Options.Ragebot.manualbackkey, ImVec2(150, 25));
							}
							else if (aatabtype2 == 1)
							{

								ImGui::Checkbox(("Enable"), &g_Options.Ragebot.air_BuilderAAs);
								ImGui::SliderFloat(("Pitch Angle: "), &g_Options.Ragebot.air_BuilderPitch, -89, 89, "%1.f");
								ImGui::SliderFloat(("Real Angle: "), &g_Options.Ragebot.air_BuilderReal, -180, 180, "%1.f");
								ImGui::SliderFloat(("Fake Angle: "), &g_Options.Ragebot.air_BuilderFake, -180, 180, "%1.f");
								ImGui::Checkbox(("Enable Real Jitter"), &g_Options.Ragebot.air_Jitter);
								ImGui::SliderFloat(("Jitter Range: "), &g_Options.Ragebot.air_JitterRange, -90, 90, "%1.f");
								ImGui::Checkbox(("Enable Fake Jitter"), &g_Options.Ragebot.air_FJitter);
								ImGui::SliderFloat(("FJitter Range: "), &g_Options.Ragebot.air_FJitterRange, -90, 90, "%1.f");
								ImGui::Checkbox(("LBY Breaker"), &g_Options.Ragebot.air_LBYBreaker);
							}
						}
					}
					else if (g_Options.Menu.MenuTab == 3)
					{
						BtnMenu();
						static int pagesshit = 1;

						ImGui::BeginChild("#weaponssets", ImVec2(815, 50), false, ImGuiWindowFlags_NoScrollbar);
						{

							ImGui::Spacing();

							ImGui::PushFont(fkek2);
							ImGui::SameLine(0.0, 5.0f);
							if (ImGui::Button("W", ImVec2(115, 50)))
							{
								pagesshit = 1;
							}

							ImGui::SameLine(0.0, 5.0f);

							if (ImGui::Button("A", ImVec2(115, 50)))
							{
								pagesshit = 2;
							}
							ImGui::SameLine(0.0, 5.0f);
							if (ImGui::Button("Z", ImVec2(115, 50)))
							{
								pagesshit = 3;
							}
							ImGui::SameLine(0.0, 5.0f);
							if (ImGui::Button("N", ImVec2(115, 50)))
							{
								pagesshit = 4;
							}
							ImGui::SameLine(0.0, 5.0f);
							if (ImGui::Button("d", ImVec2(115, 50)))
							{
								pagesshit = 5;
							}
							ImGui::PopFont();



						}ImGui::EndChild();


						ImGui::Spacing();


						ImGui::Checkbox(("ENABLE"), &g_Options.LegitBot.Enable);

						ImGui::SameLine();

						ImGui::Checkbox(("Auto Pistol"), &g_Options.LegitBot.AutoPistol);


						if (pagesshit == 1)
						{

							ImGui::BeginChild("#Child1", ImVec2(400, -1), true);
							{

								ImGui::PushItemWidth(300);

								ImGui::Text("RIFLES");

								ImGui::Separator();

								ImGui::Spacing();

								ImGui::Hotkey(("##mainkeyKEY"), &g_Options.LegitBot.MainKey, ImVec2(300, 25));
								ImGui::SameLine();
								ImGui::Text("Key");


								ImGui::Spacing();

								ImGui::SliderFloat(("SMOOTH"), &g_Options.LegitBot.MainSmooth, 1.00f, 100.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("FOV"), &g_Options.LegitBot.Mainfov, 0.00f, 30.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("RCS"), &g_Options.LegitBot.mainrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Checkbox(("Enable Backtracking##backtrack"), &g_Options.Backtrack.backtrackenable);
								ImGui::PushItemWidth(110);
								ImGui::Combo(("Visualize Backtrack"), &g_Options.Backtrack.BacktrackType, Backtracktype, ARRAYSIZE(Backtracktype));
								ImGui::PopItemWidth();
								ImGui::PushItemWidth(180);
								ImGui::SliderInt("BackTrack - Ticks", &g_Options.Backtrack.backtrackticks, 1, 12);
								ImGui::Checkbox(("Legit AntiAim"), &g_Options.LegitBot.EnableLegitAA);
								ImGui::Checkbox(("Manual AA"), &g_Options.LegitBot.ManualAA);
								ImGui::SliderFloat(("AA Angle"), &g_Options.LegitBot.AAAngle, -180, 180, "%1.f");
								ImGui::PopItemWidth();



							}ImGui::EndChild();


							ImGui::SameLine(0.0, 15.0f);


							ImGui::BeginChild("##Hiboxes", ImVec2(-1, -1), true);
							{
								ImGui::Text("Hitbox(Enable only one)");

								ImGui::Separator();

								ImGui::BeginChild(("Filter"), ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 21 * 5));
								{
									ImGui::PushItemWidth(-1);
									ImGui::Selectable((" Head"), &g_Options.LegitBot.FilterMainWeapons.Headwpmain);
									ImGui::Selectable((" Neck"), &g_Options.LegitBot.FilterMainWeapons.Neckwpmain);
									ImGui::Selectable((" Chest"), &g_Options.LegitBot.FilterMainWeapons.Chestwpmain);
									ImGui::Selectable((" Stomach"), &g_Options.LegitBot.FilterMainWeapons.Stomachwpmain);
									ImGui::Selectable((" Multi-Bone"), &g_Options.LegitBot.FilterMainWeapons.multiboneswpmain);
								}ImGui::EndChild();

							}ImGui::EndChild();

						}

						if (pagesshit == 2)
						{

							ImGui::BeginChild("#Child2", ImVec2(400, -1), true);
							{


								ImGui::Text("PISTOLS");

								ImGui::Separator();

								ImGui::Spacing();


								ImGui::PushItemWidth(300);

								ImGui::Hotkey(("##pistolKEY"), &g_Options.LegitBot.PistolKey, ImVec2(300, 25));
								ImGui::SameLine();
								ImGui::Text("Key");

								ImGui::Spacing();

								ImGui::SliderFloat(("SMOOTH"), &g_Options.LegitBot.PistolSmooth, 1.00f, 100.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("FOV"), &g_Options.LegitBot.Pistolfov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();

								ImGui::SliderFloat(("RCS"), &g_Options.LegitBot.pistolrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Checkbox(("Enable Backtracking##backtrack"), &g_Options.Backtrack.backtrackenable);
								ImGui::PushItemWidth(110);
								ImGui::Combo(("Visualize Backtrack"), &g_Options.Backtrack.BacktrackType, Backtracktype, ARRAYSIZE(Backtracktype));
								ImGui::PopItemWidth();
								ImGui::PushItemWidth(180);
								ImGui::SliderInt("BackTrack - Ticks", &g_Options.Backtrack.backtrackticks, 1, 12);
								ImGui::Checkbox(("Legit AntiAim"), &g_Options.LegitBot.EnableLegitAA);
								ImGui::Checkbox(("Manual AA"), &g_Options.LegitBot.ManualAA);
								ImGui::SliderFloat(("AA Angle"), &g_Options.LegitBot.AAAngle, -180, 180, "%1.f");
								ImGui::PopItemWidth();
							}ImGui::EndChild();


							ImGui::SameLine(0.0, 15.0f);

							ImGui::BeginChild("##Hiboxes2", ImVec2(-1, -1), true);
							{


								ImGui::Text("Hitbox(Enable only one)");

								ImGui::Separator();

								ImGui::BeginChild(("Filter"), ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 21 * 5));
								{
									ImGui::PushItemWidth(-1);
									ImGui::Selectable((" Head"), &g_Options.LegitBot.FilterPistolWeapons.Headwppis);
									ImGui::Selectable((" Neck"), &g_Options.LegitBot.FilterPistolWeapons.Neckwppis);
									ImGui::Selectable((" Chest"), &g_Options.LegitBot.FilterPistolWeapons.Chestwppis);
									ImGui::Selectable((" Stomach"), &g_Options.LegitBot.FilterPistolWeapons.Stomachwppis);
									ImGui::Selectable((" Multi-Bone"), &g_Options.LegitBot.FilterPistolWeapons.multiboneswppis);
								}ImGui::EndChild();


							}ImGui::EndChild();
						}

						if (pagesshit == 3)
						{
							ImGui::BeginChild("#Child3", ImVec2(400, -1), true);
							{

								ImGui::Text("SNIPERS");

								ImGui::Separator();

								ImGui::Spacing();

								ImGui::PushItemWidth(300);

								ImGui::Hotkey(("##sniperkeyKEY"), &g_Options.LegitBot.SniperKey, ImVec2(300, 25));
								ImGui::SameLine();
								ImGui::Text("Key");

								ImGui::Spacing();

								ImGui::SliderFloat(("SMOOTH"), &g_Options.LegitBot.SniperSmooth, 1.00f, 100.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("FOV"), &g_Options.LegitBot.Sniperfov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();

								ImGui::SliderFloat(("RCS"), &g_Options.LegitBot.sniperrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Checkbox(("Enable Backtracking##backtrack"), &g_Options.Backtrack.backtrackenable);
								ImGui::PushItemWidth(110);
								ImGui::Combo(("Visualize Backtrack"), &g_Options.Backtrack.BacktrackType, Backtracktype, ARRAYSIZE(Backtracktype));
								ImGui::PopItemWidth();
								ImGui::PushItemWidth(180);
								ImGui::SliderInt("BackTrack - Ticks", &g_Options.Backtrack.backtrackticks, 1, 12);
								ImGui::Checkbox(("Legit AntiAim"), &g_Options.LegitBot.EnableLegitAA);
								ImGui::Checkbox(("Manual AA"), &g_Options.LegitBot.ManualAA);
								ImGui::SliderFloat(("AA Angle"), &g_Options.LegitBot.AAAngle, -180, 180, "%1.f");
								ImGui::PopItemWidth();
							}ImGui::EndChild();


							ImGui::SameLine(0.0, 15.0f);

							ImGui::BeginChild("##Hiboxes2", ImVec2(-1, -1), true);
							{
								ImGui::Text("Hitbox(Enable only one)");

								ImGui::Separator();


								ImGui::BeginChild(("Filter"), ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 21 * 5));
								{
									ImGui::PushItemWidth(-1);
									ImGui::Selectable((" Head"), &g_Options.LegitBot.FilterSniperWeapons.HeadwpSnip);
									ImGui::Selectable((" Neck"), &g_Options.LegitBot.FilterSniperWeapons.NeckwpSnip);
									ImGui::Selectable((" Chest"), &g_Options.LegitBot.FilterSniperWeapons.ChestwpSnip);
									ImGui::Selectable((" Stomach"), &g_Options.LegitBot.FilterSniperWeapons.StomachwpSnip);
									ImGui::Selectable((" Multi-Bone"), &g_Options.LegitBot.FilterSniperWeapons.multiboneswpSnip);
								}ImGui::EndChild();


							}ImGui::EndChild();
						}

						if (pagesshit == 4)
						{

							ImGui::BeginChild("#Child4", ImVec2(400, -1), true);
							{


								ImGui::Text("SMG");

								ImGui::Separator();

								ImGui::Spacing();

								ImGui::PushItemWidth(300);


								ImGui::Hotkey(("##smgKEY"), &g_Options.LegitBot.smg_Key, ImVec2(300, 25));
								ImGui::SameLine();
								ImGui::Text("Key");


								ImGui::Spacing();

								ImGui::SliderFloat(("SMOOTH"), &g_Options.LegitBot.smg_Smooth, 1.00f, 100.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("FOV"), &g_Options.LegitBot.smg_fov, 0.00f, 30.00f, "%.1f");
								ImGui::Spacing();

								ImGui::SliderFloat(("RCS"), &g_Options.LegitBot.smgrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Checkbox(("Enable Backtracking##backtrack"), &g_Options.Backtrack.backtrackenable);
								ImGui::PushItemWidth(110);
								ImGui::Combo(("Visualize Backtrack"), &g_Options.Backtrack.BacktrackType, Backtracktype, ARRAYSIZE(Backtracktype));
								ImGui::PopItemWidth();
								ImGui::PushItemWidth(180);
								ImGui::SliderInt("BackTrack - Ticks", &g_Options.Backtrack.backtrackticks, 1, 12);
								ImGui::Checkbox(("Legit AntiAim"), &g_Options.LegitBot.EnableLegitAA);
								ImGui::Checkbox(("Manual AA"), &g_Options.LegitBot.ManualAA);
								ImGui::SliderFloat(("AA Angle"), &g_Options.LegitBot.AAAngle, -180, 180, "%1.f");
								ImGui::PopItemWidth();
							}ImGui::EndChild();


							ImGui::SameLine(0.0, 15.0f);


							ImGui::BeginChild("##Hiboxes3", ImVec2(-1, -1), true);
							{
								ImGui::Text("Hitbox(Enable only one)");

								ImGui::Separator();

								ImGui::BeginChild(("Filter"), ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 21 * 5));
								{
									ImGui::PushItemWidth(-1);
									ImGui::Selectable((" Head"), &g_Options.LegitBot.FiltersmgWeapons.Headwpsmg);
									ImGui::Selectable((" Neck"), &g_Options.LegitBot.FiltersmgWeapons.Neckwpsmg);
									ImGui::Selectable((" Chest"), &g_Options.LegitBot.FiltersmgWeapons.Chestwpsmg);
									ImGui::Selectable((" Stomach"), &g_Options.LegitBot.FiltersmgWeapons.Stomachwpsmg);
									ImGui::Selectable((" Multi-Bone"), &g_Options.LegitBot.FiltersmgWeapons.multiboneswpsmg);
								}ImGui::EndChild();

							}ImGui::EndChild();

						}

						if (pagesshit == 5)
						{
							ImGui::BeginChild("#Child5", ImVec2(400, -1), true);
							{


								ImGui::Text("HEAVY");

								ImGui::Separator();

								ImGui::Spacing();

								ImGui::PushItemWidth(300);

								ImGui::Hotkey(("##heavyKEY"), &g_Options.LegitBot.heavy_wp_Key, ImVec2(300, 25));
								ImGui::SameLine();
								ImGui::Text("Key");


								ImGui::Spacing();

								ImGui::SliderFloat(("SMOOTH"), &g_Options.LegitBot.heavy_wp_Smooth, 1.00f, 100.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("FOV"), &g_Options.LegitBot.heavy_wp_fov, 0.00f, 30.00f, "%.1f");

								ImGui::Spacing();

								ImGui::SliderFloat(("RCS"), &g_Options.LegitBot.heavyrcs, 0.00f, 100.00f, "%.1f");
								ImGui::Checkbox(("Enable Backtracking##backtrack"), &g_Options.Backtrack.backtrackenable);
								ImGui::PushItemWidth(110);
								ImGui::Combo(("Visualize Backtrack"), &g_Options.Backtrack.BacktrackType, Backtracktype, ARRAYSIZE(Backtracktype));
								ImGui::PopItemWidth();
								ImGui::PushItemWidth(180);
								ImGui::SliderInt("BackTrack - Ticks", &g_Options.Backtrack.backtrackticks, 1, 12);
								ImGui::Checkbox(("Legit AntiAim"), &g_Options.LegitBot.EnableLegitAA);
								ImGui::Checkbox(("Manual AA"), &g_Options.LegitBot.ManualAA);
								ImGui::SliderFloat(("AA Angle"), &g_Options.LegitBot.AAAngle, -180, 180, "%1.f");
								ImGui::PopItemWidth();
							}ImGui::EndChild();


							ImGui::SameLine(0.0, 15.0f);


							ImGui::BeginChild("##Hiboxes4", ImVec2(-1, -1), true);
							{
								ImGui::Text("Hitbox(Enable only one)");

								ImGui::Separator();

								ImGui::BeginChild(("Filter"), ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 21 * 5));
								{
									ImGui::PushItemWidth(-1);
									ImGui::Selectable((" Head"), &g_Options.LegitBot.FilterheavyWeapons.Headwphv);
									ImGui::Selectable((" Neck"), &g_Options.LegitBot.FilterheavyWeapons.Neckwphv);
									ImGui::Selectable((" Chest"), &g_Options.LegitBot.FilterheavyWeapons.Chestwphv);
									ImGui::Selectable((" Stomach"), &g_Options.LegitBot.FilterheavyWeapons.Stomachwphv);
									ImGui::Selectable((" Multi-Bone"), &g_Options.LegitBot.FilterheavyWeapons.multibonewphv);
								}ImGui::EndChild();


							}ImGui::EndChild();
						}
					}
					else if (g_Options.Menu.MenuTab == 4) // visuals
					{
						BtnMenu();
						ImGui::Checkbox(("Enable Visuals"), &g_Options.Visuals.Enabled);
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("General");
						ImGui::Spacing();
						ImGui::PushItemWidth(120);
						ImGui::Checkbox(("Draw Teammates"), &g_Options.Visuals.TeamESP);
						ImGui::Checkbox(("Box"), &g_Options.Visuals.Box);
						ImGui::SameLine();
						ImGui::PushItemWidth(110);
						ImGui::Combo("##boxkek", &g_Options.Visuals.BoxType, "Full\0\rCorners\0\r3D\0\0", -1);

						if (g_Options.Visuals.BoxType == 0)
						{
							ImGui::Checkbox(("Fill-Box"), &g_Options.Visuals.fill);
							ImGui::SliderFloat(("Alpha##alphafill"), &g_Options.Visuals.esp_fill_amount, 0.f, 255.f, "%.0f");
						}
						else if (g_Options.Visuals.BoxType == 1)
						{
							ImGui::Checkbox(("Fill-Box"), &g_Options.Visuals.fill);
							ImGui::SliderFloat(("Alpha##alphafill"), &g_Options.Visuals.esp_fill_amount, 0.f, 255.f, "%.0f");
						}
						else if (g_Options.Visuals.BoxType == 2)
						{
							g_Options.Visuals.fill = false;
							g_Options.Visuals.esp_fill_amount = 0.f;
						}
						ImGui::Checkbox(("Skeleton"), &g_Options.Visuals.skeletonenbl);
						ImGui::SameLine();
						ImGui::PushItemWidth(110);
						ImGui::Combo("##skelopts", &g_Options.Visuals.skeletonopts, skeletonopts, ARRAYSIZE(skeletonopts));
						ImGui::Checkbox(("Name"), &g_Options.Visuals.Name);
						ImGui::PushItemWidth(110);
						ImGui::Combo(("Weapon"), &g_Options.Visuals.Weapon, WeaponType, ARRAYSIZE(WeaponType));
						ImGui::Checkbox(("Aimlines"), &g_Options.Visuals.AimLine);
						ImGui::Checkbox(("Health##Enable"), &g_Options.Visuals.health);
						ImGui::SameLine();
						ImGui::PushItemWidth(110);
						ImGui::Combo(("Health##style"), &g_Options.Visuals.healthtype, healthtype, ARRAYSIZE(healthtype));
						ImGui::PopItemWidth();
						ImGui::Checkbox(("Armor Bar"), &g_Options.Visuals.armor);
						ImGui::Combo(("Armor"), &g_Options.Visuals.Armor2, armor, ARRAYSIZE(armor));
						//							ImGui::Checkbox(("Ammo Box"), &g_Options.Visuals.AmmoBox);

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Others");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::PushItemWidth(110);
						ImGui::Checkbox(("Damage-Indicator"), &g_Options.Visuals.DamageIndicator);
						ImGui::Checkbox(("Aim Direction"), &g_Options.Visuals.barrel);
						ImGui::SameLine();
						ImGui::SliderInt(("Amount##barrel"), &g_Options.Visuals.barrelL, 0, 300);
						ImGui::PopItemWidth();
						ImGui::Checkbox(("Hitbox"), &g_Options.Visuals.Hitbox);
						ImGui::SliderFloat(("##duration"), &g_Options.Visuals.HitboxDuration, 0.f, 10.f, "Duration: %.0f");
						ImGui::PushItemWidth(160);
						ImGui::Checkbox(("Bullet Tracers"), &g_Options.Visuals.bulletshow);
						ImGui::SliderFloat("Duration", &g_Options.Visuals.flTracersDuration, 0.f, 10.f, "%.0f");
						ImGui::SliderFloat("Width", &g_Options.Visuals.flTracersWidth, 0.f, 10.f, "%.0f");
						ImGui::Checkbox(("Offscreen Indicator"), &g_Options.Visuals.OffscreenIndicator);
						ImGui::SliderInt("Size", &g_Options.Visuals.OffscreenIndicatorSize, 5, 35);
						ImGui::PopItemWidth();
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Enemy State");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::Checkbox(("Defuser"), &g_Options.Visuals.IsHasDefuser);
						ImGui::Checkbox(("Scoped"), &g_Options.Visuals.Scoped);
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
						ImGui::Combo((""), &g_Options.Visuals.DroppedGunsType, droppedWeapons, ARRAYSIZE(droppedWeapons));
						ImGui::Checkbox(("NightMode"), &g_Options.Visuals.nightMode);
						ImGui::Checkbox(("AmbientLight"), &g_Options.Visuals.ambientlight);
						ImGui::Combo(("Skybox Changer"), &g_Options.Visuals.SkyboxChanger, skyboxchanger, ARRAYSIZE(skyboxchanger));
						ImGui::Checkbox(XorStr("Color Modulate Skybox"), &g_Options.Colors.ColorSkybox);
						ImGui::Combo("Color", &g_Options.Colors.SkyColor, SkyColor, ARRAYSIZE(SkyColor));
						ImGui::PopItemWidth();
						ImGui::SliderFloat(("##wallalpha"), &g_Options.Visuals.wallalpha, 0, 1, "Wall Alpha: %.2f");
						ImGui::SliderFloat(("##proppalpha"), &g_Options.Visuals.propalpha, 0, 1, "Prop Alpha: %.2f");
						ImGui::SliderFloat(("##modelpalpha"), &g_Options.Visuals.modelalpha, 0, 1, "Model Alpha: %.2f");
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Spoof");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::Checkbox(("Snow Mode"), &g_Options.Visuals.snowmode);
						ImGui::Checkbox(("LSD Mode"), &g_Options.Visuals.lsdmode);
						ImGui::Checkbox(("Chrome Mode"), &g_Options.Visuals.chromemode);
						ImGui::Checkbox(("Minecraft Mode"), &g_Options.Visuals.minecraftmode);
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Chams");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::Checkbox(("Players"), &g_Options.Visuals.Chams);
						ImGui::Checkbox(("Team"), &g_Options.Visuals.Teamchams);
						ImGui::Checkbox(("HP-Based"), &g_Options.Visuals.chamsHp);
						ImGui::Checkbox(("XQZ"), &g_Options.Visuals.XQZ);
						ImGui::Checkbox(("FakeAngle"), &g_Options.Visuals.FakeAngleChams);
						ImGui::PushItemWidth(160);
						ImGui::Combo("Chams-Style##cmsstyle", &g_Options.Visuals.matierial, "Normal\0\rFlat\0\rWireframe\0\rGlass\0\rGold\0\rPlatinum\0\rPlastic\0\rCrystal\0\0", -1);
						ImGui::SliderInt(("Chams-Alpha##chamsplayeraplha"), &g_Options.Visuals.champlayeralpha, 0, 100);
						ImGui::PopItemWidth();

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Hands/Weapons");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::PushItemWidth(160);
						ImGui::Combo(("Hands-Style"), &g_Options.Visuals.Hands, HandsMode, ARRAYSIZE(HandsMode));
						ImGui::SliderInt(("Alpha##HandsAlphaa"), &g_Options.Visuals.HandsAlpha, 0, 100);
						ImGui::Combo(("Weapon-Style"), &g_Options.Visuals.chamswphands, chamswp, ARRAYSIZE(chamswp));
						ImGui::SliderInt(("Alpha##chamswphands"), &g_Options.Visuals.chamswphandsalpha, 0, 100);
						ImGui::PopItemWidth();
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Glow");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::PushItemWidth(160);

						ImGui::Checkbox(("Glow Enable"), &g_Options.Visuals.GlowEnable);
						ImGui::Checkbox(("Glow Players"), &g_Options.Visuals.GlowPlayerEnable);
						ImGui::Checkbox(("Glow Enemies"), &g_Options.Visuals.GlowEnemy);
						ImGui::Checkbox(("Glow Team"), &g_Options.Visuals.GlowTeam);
						ImGui::SliderFloat(("Enemy"), &g_Options.Visuals.EnemyAlpha, 0.f, 255.f, "%.0f");
						ImGui::SliderFloat(("Team"), &g_Options.Visuals.TeamAlpha, 0.f, 255.f, "%.0f");

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
						ImGui::Checkbox(("Resolve Mode"), &g_Options.Visuals.resolveMode);
						ImGui::Checkbox(("Rage-Info (!)"), &g_Options.Visuals.RageDraw);
						ImGui::Checkbox(("Manual AA Indicator"), &g_Options.Visuals.ManualAAIndicator);
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Thirdperson");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::PushItemWidth(150);
						ImGui::Checkbox("Enable Thirdperson", &g_Options.Visuals.Enabletp);
						ImGui::Hotkey("##KeyTP", &g_Options.Visuals.TPKey, ImVec2(150, 25));
						ImGui::SameLine();
						ImGui::Text("Key");
						ImGui::Combo("Thirdperson Angle", &g_Options.Visuals.antiaim_thirdperson_angle, AntiAimThirdperson, ARRAYSIZE(AntiAimThirdperson));
						ImGui::Checkbox("Disable On Grenade", &g_Options.Visuals.GrenadeCheck);
						ImGui::Checkbox("Transparency", &g_Options.Visuals.transparency);

						ImGui::PopItemWidth();
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
						ImGui::Spacing();
						ImGui::Separator();

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
						ImGui::PopItemWidth();
					}
					else if (g_Options.Menu.MenuTab == 5) // misc
					{
						BtnMenu();
						ImGui::PushItemWidth(100);
						//ImGui::Checkbox("Media Mode", &g_Options.Misc.mediamode);
						ImGui::Checkbox(("Rank Revealer"), &g_Options.Misc.ServerRankRevealAll);
						ImGui::Checkbox(("Auto-Accept"), &g_Options.Misc.AutoAccept);
						ImGui::Checkbox(("Bunny Hop"), &g_Options.Misc.Bhop);
						ImGui::Combo(("Auto Strafe"), &g_Options.Misc.AutoStrafe, autostrafe, ARRAYSIZE(autostrafe));
						ImGui::Checkbox(("Watermark"), &g_Options.Misc.Watermark);
						ImGui::Checkbox(("Spectator List"), &g_Options.Misc.SpecList);
						ImGui::Checkbox(("Hitmarker"), &g_Options.Misc.Hitmarker);
						ImGui::SameLine();
						ImGui::Combo(("Sound"), &g_Options.Misc.Hitsound, HitmarkSound, ARRAYSIZE(HitmarkSound));
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
						ImGui::Spacing();
						ImGui::Separator();
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
						if (ImGui::Button("No Name", ImVec2(230, 20)))
						{
							NameChanger::SetName("\n\xAD\xAD\xAD");
						}
						if (ImGui::Button("Name(CarceaHOOK.xyz)", ImVec2(230, 20)))
						{
							static char nicknameCarceaHOOK[15] = "CarceaHOOK.xyz";
							NameChanger::SetName(std::string(nicknameCarceaHOOK).c_str());
						}
						if (ImGui::Button("Silent Name Steal", ImVec2(230, 20)))
						{
							misc::silentname(local);
						}




						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Chat Spam");
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::PushItemWidth(230);
						ImGui::Checkbox(("Chat Spammer"), &g_Options.Misc.spammeron);
						ImGui::Combo(("SpammerList##chatspam"), &g_Options.Misc.spammer, spammerlist, ARRAYSIZE(spammerlist));
						ImGui::PopItemWidth();



						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Clantag");
						ImGui::Separator();
						ImGui::Spacing();


						//ImGui::Checkbox("CarceaHOOK.xyz Clantag", &g_Options.Misc.syncclantag);


						ImGui::Text("Custom Clantag");
						static char ClanChanger[127] = "";

						ImGui::InputText(("##ClantagINPUT"), ClanChanger, 127, ImGuiInputTextFlags_AllowTabInput);
						ImGui::SameLine();
						if (ImGui::Button(("Change Clantag")))
						{
							SetClantag(ClanChanger);
						}
						if (ImGui::Button(("CarceaHOOK.xyz Clantag")))
						{
							static char ClanChangerCarcea[15] = "CarceaHOOK.xyz";
							SetClantag(ClanChangerCarcea);
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Radar");
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Checkbox(("Engine Radar"), &g_Options.Misc.radaringame);

						if (g_Options.Misc.radaringame == true)
						{
							g_Options.Misc.radarwindow = false;
						}
						if (g_Options.Misc.radarwindow == true)
						{
							g_Options.Misc.radaringame = false;
						}

						ImGui::Checkbox("Radar Window", &g_Options.Misc.radarwindow);

						ImGui::SliderFloat("##radrzom", &g_Options.Misc.radrzoom, 0.f, 4.f, "zoom: %.2f");
						ImGui::SliderFloat("##radralpha", &g_Options.Misc.radralpha, 0.f, 1.f, "Alpha: %.2f");
						ImGui::SliderInt(("##Size"), &g_Options.Misc.radrsize, 100, 500, "Size: %.0f");
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Text("Buy Bot");
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::Checkbox("Enabled##BuyBot", &g_Options.Misc.BuyBot);
						ImGui::Checkbox("Defuser##BuyBot", &g_Options.Misc.BuyBotDefuse);
						ImGui::Combo(("Weapons##BuyBot"), &g_Options.Misc.BuyBotWeap, BuyBotWeap, ARRAYSIZE(BuyBotWeap));
						ImGui::Combo(("Grenades##BuyBot"), &g_Options.Misc.BuyBotGrenade, BuyBotNade, ARRAYSIZE(BuyBotNade));
						ImGui::Combo(("Armor##BuyBot"), &g_Options.Misc.BuyBotKevlarHelmet, BuyBotArmor, ARRAYSIZE(BuyBotArmor));
					}
					else if (g_Options.Menu.MenuTab == 7) // colors
					{
						BtnMenu();
						static int selectedItem = 0;
						ImVec2 lastCursor = ImGui::GetCursorPos();

						ImGui::ListBoxHeader("##0", ImVec2(200, -1));
						for (int i = 0; i < Global::ColorsForPicker1.size(); i++)
						{
							bool selected = i == selectedItem;

							if (ImGui::Selectable(Global::ColorsForPicker1[i].Name, selected))
								selectedItem = i;
						}
						ImGui::ListBoxFooter();


						ImGui::SameLine(0.0f, 15.0f);

						ImGui::BeginChild("#generalcolors", ImVec2(-1, -1), false, ImGuiWindowFlags_NoScrollbar);
						{


							ImGui::BeginChild("#ColorsChild", ImVec2(-1, 130), true);
							{


								ImGui::Spacing();
								ImGui::Spacing();
								ColorP col = Global::ColorsForPicker1[selectedItem];
								int r = (col.Ccolor[0] * 255.f);
								int g = (col.Ccolor[1] * 255.f);
								int b = (col.Ccolor[2] * 255.f);
								ImGui::PushItemWidth(290);
								ImGui::SliderInt("##red", &r, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("Red Amount");
								ImGui::Spacing();
								ImGui::SliderInt("##green", &g, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("Green Amount");
								ImGui::Spacing();
								ImGui::SliderInt("##blue", &b, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("Blue Amount");
								ImGui::PopItemWidth();
								col.Ccolor[0] = r / 255.0f;
								col.Ccolor[1] = g / 255.0f;
								col.Ccolor[2] = b / 255.0f;

							}ImGui::EndChild();


							ImGui::BeginChild("#colorpreview", ImVec2(-1, 296), true);
							{


								ColorP col = Global::ColorsForPicker1[selectedItem];
								ImGui::ColorPicker("##COLOR_PICKER", col.Ccolor);

							}ImGui::EndChild();

						}ImGui::EndChild();
					}
					else if (g_Options.Menu.MenuTab == 6)
					{
						BtnMenu();
						static char filterWeapons[32];
						static char filterSkins[32];

						ImGui::Checkbox(("ENABLE-SKINCHANGER"), &g_Options.Skinchanger.Enabled);

						ImGui::BeginChild("first child", ImVec2(300, 270), true);
						{


							ImGui::PushItemWidth(-1);
							ImGui::InputText(("##FILTERWEAPONS"), filterWeapons, IM_ARRAYSIZE(filterWeapons));
							ImGui::PopItemWidth();
							ImGui::ListBoxHeader(("##GUNS"), ImVec2(-1, -1));
							for (auto it : ItemDefinitionIndexMap)
							{
								bool isDefault = (int)it.first < 0;
								if (!isDefault && !Contains(ToLower(std::string(filterWeapons)), ToLower(ItemDefinitionIndexMap.at(it.first).entityName)))
									continue;

								if (IsUtility(it.first))
									continue;

								const bool item_selected = ((int)it.first == (int)g_Options.Menu.currentWeapon);

								std::string formattedName;

								formattedName = ItemDefinitionIndexMap.at(it.first).entityName;
								if (ImGui::Selectable(formattedName.c_str(), item_selected))
								{
									g_Options.Menu.currentWeapon = (int)it.first;
								}

							}
							ImGui::ListBoxFooter();
						}ImGui::EndChild();
						ImGui::SameLine();

						ImGui::BeginChild("second child", ImVec2(-1, 270), true);
						{

							std::string skinName = weaponnames(g_Options.Menu.currentWeapon);
							{
								ImGui::PushItemWidth(-1);
								ImGui::InputText(("##FILTERSKINS"), filterSkins, IM_ARRAYSIZE(filterSkins));
								ImGui::InputInt(("##customID"), &g_Options.Skinchanger.W[g_Options.Menu.currentWeapon].ChangerSkin);
								ImGui::PopItemWidth();
								ImGui::ListBoxHeader(("##SKINS"), ImVec2(-1, -1));
								std::string skinStr = "";
								int curItem = -1;
								int s = 0;
								for (auto skin : g_Options.Skinchanger.weaponSkins[skinName])
								{
									int pk = g_Options.Skinchanger.skinMap[skin].paintkit;
									if (pk == g_Options.Skinchanger.W[g_Options.Menu.currentWeapon].ChangerSkin)
										curItem = s;
									bool selected = g_Options.Skinchanger.W[g_Options.Menu.currentWeapon].ChangerSkin == pk;


									skinStr = g_Options.Skinchanger.skinNames[g_Options.Skinchanger.skinMap[skin].tagName].c_str();
									skinStr += " | ";
									skinStr += std::to_string(g_Options.Skinchanger.skinMap[skin].paintkit);
									if (!Contains(ToLower(std::string(filterSkins)), ToLower(skinStr)))
										continue;
									if (ImGui::Selectable(skinStr.c_str(), selected))
									{
										pk = g_Options.Skinchanger.skinMap[skin].paintkit;
										g_Options.Skinchanger.W[g_Options.Menu.currentWeapon].ChangerSkin = pk;
										U::FullUpdate();
										break;
									}
									s++;
								}

								ImGui::ListBoxFooter();
							}
						}ImGui::EndChild();


						ImGui::BeginChild("third child", ImVec2(300, 250), true);
						{


							ImGui::ListBoxHeader("##KNIVES", ImVec2(-1, -1));
							{


								if (ImGui::RadioButton(("Default"), &g_Options.Skinchanger.knifemodel, 0))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Bayonet"), &g_Options.Skinchanger.knifemodel, 1))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Flip Knife"), &g_Options.Skinchanger.knifemodel, 2))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Gut Knife"), &g_Options.Skinchanger.knifemodel, 3))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Karambit"), &g_Options.Skinchanger.knifemodel, 4))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("M9 Bayonet"), &g_Options.Skinchanger.knifemodel, 5))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Huntsman"), &g_Options.Skinchanger.knifemodel, 6))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Falchion"), &g_Options.Skinchanger.knifemodel, 7))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Bowie"), &g_Options.Skinchanger.knifemodel, 8))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Butterfly Knife"), &g_Options.Skinchanger.knifemodel, 9))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Shadow Daggers"), &g_Options.Skinchanger.knifemodel, 10))
								{
									U::FullUpdate();
								}

							}
							ImGui::ListBoxFooter();
						}ImGui::EndChild();
						ImGui::SameLine();

						ImGui::BeginChild("fourth child", ImVec2(-1, 250), true);
						{




							ImGui::ListBoxHeader("##Gloves", ImVec2(-1, -1));
							{


								if (ImGui::RadioButton(("Default"), &g_Options.Skinchanger.glove, 0))
								{
									U::FullUpdate();
								}

								if (ImGui::RadioButton(("Bloodhound"), &g_Options.Skinchanger.glove, 1))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Sport"), &g_Options.Skinchanger.glove, 2))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Driver"), &g_Options.Skinchanger.glove, 3))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Hand Wraps"), &g_Options.Skinchanger.glove, 4))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Motorcycle"), &g_Options.Skinchanger.glove, 5))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Specialst"), &g_Options.Skinchanger.glove, 6))
								{
									U::FullUpdate();
								}
								if (ImGui::RadioButton(("Hydra"), &g_Options.Skinchanger.glove, 7))
								{
									U::FullUpdate();
								}

								const char* gstr;
								if (g_Options.Skinchanger.glove == 1)
								{
									gstr = "Charred\0\rSnakebite\0\rBronzed\0\rGuerilla\0\0";
								}
								else if (g_Options.Skinchanger.glove == 2)
								{
									gstr = "Hedge Maze\0\rPandoras Box\0\rSuperconductor\0\rArid\0\Vice\0\Omega\0\Amphibious\0\Bronze Morph\0";
								}
								else if (g_Options.Skinchanger.glove == 3)
								{
									gstr = "Lunar Weave\0\rConvoy\0\rCrimson Weave\0\rDiamondback\0\Overtake\0\Racing Green\0\King Snake\0\Imperial Plaid\0";
								}
								else if (g_Options.Skinchanger.glove == 4)
								{
									gstr = "Leather\0\rSpruce DDPAT\0\rSlaughter\0\rBadlands\0\Cobalt Skulls\0\Overprint\0\Duct Tape\0\Arboreal\0";
								}
								else if (g_Options.Skinchanger.glove == 5)
								{
									gstr = "Eclipse\0\rSpearmint\0\rBoom!\0\rCool Mint\0\Turtle\0\Transport\0\Polygon\0\POW!\0";
								}
								else if (g_Options.Skinchanger.glove == 6)
								{
									gstr = "Forest DDPAT\0\rCrimson Kimono\0\rEmerald Web\0\rFoundation\0\Crimson Web\0\Buckshot\0\Fade\0\Mogul\0";
								}
								else if (g_Options.Skinchanger.glove == 7)
								{
									gstr = "Case Hardened\0\rRattler\0\rMangrove\0\rEmerald\0\0";
								}
								else
								{
									gstr = "";
								}
								ImGui::PushItemWidth(-1);
								ImGui::Text("Glove Skin :");
								if (ImGui::Combo(("##2"), &g_Options.Skinchanger.gloveskin, gstr, -1))
									U::FullUpdate();


								ImGui::PopItemWidth();

							}ImGui::ListBoxFooter();


						}ImGui::EndChild();

						if (ImGui::Button(("Apply##skinchanger"), ImVec2(-1, 30)))
						{
							U::FullUpdate();
						}
					}
				}ImGui::EndChild();
			}ImGui::End();
		}
	}
}
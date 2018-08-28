#include "ESP.h"
#include "Interfaces.h"
#include "Render.h"
#include <ctime>
#include <iostream>
#include <algorithm>
#include "GrenadePrediction.h"
#include "memoryfonts.h"
#include "LagComp.h"
#include "Autowall.h"
#include "Hooks.h"
#include "ManualAA.h"
#include "AWHitmarkers.h"
namespace ColorAlpha
{
	VisualsStruct ESP_ctx;
	RECT bbox;
	float ESP_Fade[64];
}

visuals::visuals()
{
	BombCarrier = nullptr;
}


int width = 0;
int height = 0;
bool done = false;
float damage;
char bombdamagestringdead[24];
char bombdamagestringalive[24];

void visuals::OnPaintTraverse(C_BaseEntity* local)
{
	CViewSetup* g_ViewSetup;

	CInput::CUserCmd* pCmd;

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(local->GetActiveWeaponHandle());

	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	Spoof(); // b1g Memory Leak
	NightMode();
	ColorModulateSkybox();
	DoAsusWalls();
	renderBeams();
	SkyChanger();

	CCSGrenadeHint::instance().Paint();

	static class Text
	{
	public:
		std::string text;
		int side;
		int Font;
		Color color;

		Text(std::string text, int side, int Font, Color color) : text(text), side(side), Font(Font), color(color)
		{
		}
	};
	std::vector< Text > texts;
	if (g_Options.Visuals.ManualAAIndicator)
	{
		Color inactive(255, 255, 255, 255);
		Color active(g_Options.Colors.ManualIndicator, 255);
		int W, H, cW, cH, leftw, lefth, rightw, righth, backw, backh, sideh;
		g_Engine->GetScreenSize(W, H);
		cW = W / 2;
		cH = H / 2;
		sideh = cH - 14;

		rightw = cW + 40;
		righth = sideh;

		leftw = cW - 13 - 40;
		lefth = sideh;

		backw = cW - 5;
		backh = cH + 31;

		
		if (g_Engine->IsConnected() && g_Engine->IsInGame() && local->IsAlive())
		{
			
			
			if (g_Options.Ragebot.YawTrue == 5) // manual aa
			{

				if (Globals::ManualSide) {
					g_Render->Text(rightw, righth, Color(active), g_Render->font.AAIndicator, ">");
					g_Render->Text(leftw, lefth, Color(inactive), g_Render->font.AAIndicator, "<");
				}
				else {
					g_Render->Text(rightw, righth, Color(inactive), g_Render->font.AAIndicator, ">");
					g_Render->Text(leftw, lefth, Color(active), g_Render->font.AAIndicator, "<");
				}

			}
			else
			{
				g_Render->Text(rightw, righth, Color(inactive), g_Render->font.AAIndicator, ">");
				g_Render->Text(leftw, lefth, Color(inactive), g_Render->font.AAIndicator, "<");
			}
		}
	}
	if (local->IsAlive())
	{
		if (g_Options.Visuals.RecoilCrosshair && g_Options.Visuals.Enabled)
		{
			g_Engine->GetScreenSize(width, height);
			if (local && local->IsAlive())
			{

				static auto cl_crosshair_recoil = g_CVar->FindVar("cl_crosshair_recoil");


				static Vector ViewAngles;

				g_Engine->GetViewAngles(ViewAngles);
				ViewAngles += local->localPlayerExclusive()->GetAimPunchAngle() * 2.f;

				Vector fowardVec;
				AngleVectors(ViewAngles, &fowardVec);
				fowardVec *= 10000;

				Vector start = local->GetVecOrigin() + local->GetViewOffset();
				Vector end = start + fowardVec, endScreen;

				Color recoil_color(int(g_Options.Colors.color_recoil[0] * 255.f), int(g_Options.Colors.color_recoil[1] * 255.f), int(g_Options.Colors.color_recoil[2] * 255.f));
				if (g_Render->WorldToScreen(end, endScreen) && local->IsAlive())
				{
					switch (g_Options.Visuals.RecoilCrosshair2)
					{
					case 0:
						cl_crosshair_recoil->SetValue(0);
						break;
					case 1:
						g_Render->Line(endScreen.x - 4, endScreen.y, endScreen.x + 4, endScreen.y, recoil_color);
						g_Render->Line(endScreen.x, endScreen.y - 4, endScreen.x, endScreen.y + 4, recoil_color);
						cl_crosshair_recoil->SetValue(0);
						break;
					case 2:
						g_Render->OutlineCircle(endScreen.x, endScreen.y, 3, 50, recoil_color);
						cl_crosshair_recoil->SetValue(0);
						break;
					case 3:
						cl_crosshair_recoil->SetValue(1);
						break;

					}
				}
			}
		}

		if (g_Options.Visuals.AWallCrosshair && g_Options.Visuals.Enabled)
		{
			RECT View = g_Render->GetViewport();
			int xs = View.right / 2;
			int ys = View.bottom / 2;

			int damage;

			if (MiscFunctions::IsKnife(pWeapon) || MiscFunctions::IsGrenade(pWeapon) || MiscFunctions::IsBomb(pWeapon))
			{
			}
			else if (pWeapon->AWall())
			{
				g_Render->DrawFilledCircle(Vector2D(xs, ys), CanWallbang(damage) ? Color(130, 241, 13) : Color(255, 102, 102), 1, 50);
			}
		}

		if (g_Options.Visuals.SpreadCrosshair && g_Options.Visuals.Enabled)
		{
			RECT View = g_Render->GetViewport();
			int xs = View.right / 2;
			int ys = View.bottom / 2;

			if (local && local->IsAlive())
			{
				auto accuracy = pWeapon->GetInaccuracy() * 550.f; //3000

				ColorAlpha::ESP_ctx.clr_fill.SetAlpha(g_Options.Visuals.spread_crosshair_amount * 255.f);

				Color color(int(g_Options.Colors.color_spread[0] * 255.f), int(g_Options.Colors.color_spread[1] * 255.f), int(g_Options.Colors.color_spread[2] * 255.f), g_Options.Visuals.spread_crosshair_amount);

				g_Render->DrawFilledCircle2(xs, ys, accuracy, 100, color);

			}

		}

		int W, H, cW, cH;
		g_Engine->GetScreenSize(W, H);
		cW = W / 2;
		cH = H / 2;

		if (g_Options.Visuals.SniperCrosshair && g_Options.Visuals.Enabled)
		{
			Color color(int(g_Options.Colors.color_sniper[0] * 255.f), int(g_Options.Colors.color_sniper[1] * 255.f), int(g_Options.Colors.color_sniper[2] * 255.f), 150);
			int damage;
			RECT View = g_Render->GetViewport();
			int pXs = View.right / 2;
			int pYs = View.bottom / 2;
			if (MiscFunctions::IsSniper(pWeapon))
			{
				switch (g_Options.Visuals.SniperCrosshairType)
				{

				case 1:
					g_Surface->DrawSetColor(color);
					g_Surface->DrawFilledRect(cW - 6, cH - 1, cW - 3 + 9, cH - 1 + 2);
					g_Surface->DrawFilledRect(cW - 1, cH - 6, cW - 1 + 2, cH - 3 + 9);
					break;
				case 2:
					if (CanWallbang(damage))
					{
						g_Surface->DrawSetColor(0, 255, 0, 150);
						g_Surface->DrawFilledRect(cW - 6, cH - 1, cW - 3 + 9, cH - 1 + 2);
						g_Surface->DrawFilledRect(cW - 1, cH - 6, cW - 1 + 2, cH - 3 + 9);
					}
					else
					{
						g_Surface->DrawSetColor(255, 0, 0, 150);
						g_Surface->DrawFilledRect(cW - 6, cH - 1, cW - 3 + 9, cH - 1 + 2);
						g_Surface->DrawFilledRect(cW - 1, cH - 6, cW - 1 + 2, cH - 3 + 9);
					}
					break;
				case 3:
					g_Render->Line(pXs - 10, pYs, pXs + 10, pYs, color);
					g_Render->Line(pXs, pYs - 10, pXs, pYs + 10, color);
					break;
				case 4:
					if (CanWallbang(damage))
					{
						g_Render->Line(pXs - 10, pYs, pXs + 10, pYs, Color(0, 255, 0));
						g_Render->Line(pXs, pYs - 10, pXs, pYs + 10, Color(0, 255, 0));
					}
					else
					{
						g_Render->Line(pXs - 10, pYs, pXs + 10, pYs, Color(255, 0, 0));
						g_Render->Line(pXs, pYs - 10, pXs, pYs + 10, Color(255, 0, 0));
					}
					break;
				}

			}
		}

		if (g_Options.Visuals.angleLines && g_Input->m_fCameraInThirdPerson)
		{
			DrawAngles();
		}

	}

	if (g_Options.Misc.SpecList) 
		SpecList(local);

	if (g_Options.Misc.Hitmarker)
		//	Hitmarker();
		pHitmarker->Paint();

	Color pColor;
	for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)
	{

		C_BaseEntity *entity = g_EntityList->GetClientEntity(i);

		if (entity == nullptr)
			continue;
		if (entity == local)
			continue;
		if (entity->IsDormant())
			continue;

		

		player_info_t pinfo;
		Vector max = entity->GetCollideable()->OBBMaxs();
		Vector pos, pos3D;
		pos3D = entity->GetVecOrigin();

		if (g_Options.Visuals.OffscreenIndicator && entity->IsAlive())
		{
			if (pLocal->GetTeamNum() != entity->GetTeamNum())

			{
				auto pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

				Vector eyeangles, poopvec;
				int screen_w, screen_h;
				g_Engine->GetScreenSize(screen_w, screen_h);
				g_Engine->GetViewAngles(eyeangles);
				Vector newangle = CalcAngle(Vector(pLocal->GetAbsOrigin3().x, pLocal->GetAbsOrigin3().y, 0), Vector(entity->GetAbsOrigin3().x, entity->GetAbsOrigin3().y, 0));
				AngleVectors(Vector(0, 270, 0) - newangle + Vector(0, eyeangles.y, 0), &poopvec);
				auto circlevec = Vector(screen_w / 2, screen_h / 2, 0) + (poopvec * 250.f);
				if (!g_Render->WorldToScreen(pos3D, pos))
				{
					g_Render->DrawFilledCircle(Vector2D(circlevec.x, circlevec.y), Color(g_Options.Colors.Offscreen, 100), g_Options.Visuals.OffscreenIndicatorSize, 50);
				}
			}
		}

		int owner = 0;
		ClientClass* cClass = (ClientClass*)entity->GetClientClass();

		if (!g_Render->WorldToScreen(pos3D, pos))
			continue;


		if (g_Options.Visuals.DroppedGuns && g_Options.Visuals.Enabled &&  cClass->m_ClassID != (int)ClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)ClassID::CDEagle || cClass->m_ClassID == (int)ClassID::CAK47)))
		{
			Color clr = Color(int(g_Options.Colors.droppedguns[0] * 255.f), int(g_Options.Colors.droppedguns[1] * 255.f), int(g_Options.Colors.droppedguns[2] * 255.f));

			RenderWeapon(entity, clr);
		}


		if (g_Options.Visuals.C4World && g_Options.Visuals.Enabled)
		{
			if (cClass->m_ClassID == (int)ClassID::CPlantedC4)
				DrawBombPlanted(entity, local);
		}
		if (cClass->m_ClassID == (int)ClassID::CC4)
		{
			DrawBomb(entity, cClass);
		}
		if (g_Options.Visuals.Hostage)
		{
			Color clr = Color(255, 255, 255);
			if (cClass->m_ClassID == (int)ClassID::CHostage || cClass->m_ClassID == (int)ClassID::CHostageCarriableProp)
			{
//				texts.push_back(Text("Hostage", 0, g_Render->font.ESP, Color(255, 255, 255, 255)));

				if (g_Options.Visuals.HostageBox)
				{
					ThreeDBox(entity->GetCollideable()->OBBMins(), entity->GetCollideable()->OBBMaxs(), entity->GetVecOrigin(), clr);
				}
			}
		}
		if (g_Options.Visuals.Chicken)
		{
			Color clr = Color(255, 255, 255);
			if (cClass->m_ClassID == (int)ClassID::CChicken)
			{
//				texts.push_back(Text("Chicken", 0, g_Render->font.ESP, Color(255, 255, 255, 255)));

				if (g_Options.Visuals.ChickenBox)
				{
					ThreeDBox(entity->GetCollideable()->OBBMins(), entity->GetCollideable()->OBBMaxs(), entity->GetVecOrigin(), clr);
				}
			}
		}
		if (g_Options.Visuals.Grenades && g_Options.Visuals.Enabled)
		{
			if (!g_Render->WorldToScreen(pos3D, pos))
				continue;

			Color GrenadeColor = Color(0, 0, 0, 0);
			char* szModelName = "Unknown Projectile";
			char* szModelIcon = "";
			if (strstr(cClass->m_pNetworkName, XorStr("Projectile")))
			{
				const model_t* pModel = entity->GetModel();
				if (!pModel)
					return;


				const studiohdr_t* pHdr = g_ModelInfo->GetStudiomodel(pModel);

				if (!pHdr)
					return;

				if (!strstr(pHdr->name, XorStr("thrown")) && !strstr(pHdr->name, XorStr("dropped")))
					return;


				IMaterial* mats[32];
				g_ModelInfo->GetModelMaterials(pModel, pHdr->numtextures, mats);

				for (int i = 0; i < pHdr->numtextures; i++)
				{
					IMaterial* mat = mats[i];
					if (!mat)
						continue;

					if (strstr(mat->GetName(), "flashbang"))
					{
						GrenadeColor = Color(255, 255, 0, 255);
						szModelName = "Flashbang";
						szModelIcon = "i";
					}
					else if (strstr(mat->GetName(), "m67_grenade") || strstr(mat->GetName(), "hegrenade"))
					{
						szModelName = "Grenade";
						szModelIcon = "j";
						GrenadeColor = Color(255, 0, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "smoke"))
					{
						szModelName = "Smoke";
						szModelIcon = "k";
						GrenadeColor = Color(0, 255, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "decoy"))
					{
						szModelName = "Decoy";
						szModelIcon = "m";
						GrenadeColor = Color(0, 255, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "incendiary"))
					{
						szModelName = "Incendiary";
						szModelIcon = "n";
						GrenadeColor = Color(255, 0, 0, 255);
						break;
					}
					else if (strstr(mat->GetName(), "molotov"))
					{
						szModelName = "Molotov";
						szModelIcon = "l";
						GrenadeColor = Color(255, 0, 0, 255);
						break;
					}
				}
				switch (g_Options.Visuals.Grenades)
				{
				case 0:
					if (g_Options.Visuals.GrenadeBox)
					{
						ThreeDBox(entity->GetCollideable()->OBBMins(), entity->GetCollideable()->OBBMaxs(), entity->GetVecOrigin(), GrenadeColor);
					}
					break;
				case 1:
					if (!g_Options.Visuals.GrenadeBox)
					{
						g_Render->DrawString2(g_Render->font.ESP, (int)pos.x, (int)pos.y, GrenadeColor, FONT_CENTER, "%s", szModelName);
					}
					else if (g_Options.Visuals.GrenadeBox)
					{
//						g_Render->DrawOutlinedRect((int)pos.x - 10, (int)pos.y - 10, 20, 20, GrenadeColor);
						g_Render->DrawString2(g_Render->font.ESP, (int)pos.x, (int)pos.y + 15, GrenadeColor, FONT_CENTER, "%s", szModelName);
						ThreeDBox(entity->GetCollideable()->OBBMins(), entity->GetCollideable()->OBBMaxs(), entity->GetVecOrigin(), GrenadeColor);
					}
					break;
				case 2:
					if (!g_Options.Visuals.GrenadeBox)
					{
						g_Render->DrawString2(g_Render->font.Icon, (int)pos.x, (int)pos.y, GrenadeColor, FONT_CENTER, "%s", szModelIcon);
					}
					
					else if (g_Options.Visuals.GrenadeBox)
					{
//						g_Render->DrawOutlinedRect((int)pos.x - 10, (int)pos.y - 10, 20, 20, GrenadeColor);
						g_Render->DrawString2(g_Render->font.Icon, (int)pos.x, (int)pos.y + 15, GrenadeColor, FONT_CENTER, "%s", szModelIcon);
						ThreeDBox(entity->GetCollideable()->OBBMins(), entity->GetCollideable()->OBBMaxs(), entity->GetVecOrigin(), GrenadeColor);
					}
					break;
				}
			}
		}
		if (g_Engine->GetPlayerInfo(i, &pinfo) && entity->IsAlive())
		{
			Color color(int(g_Options.Colors.backtrackdots_color[0] * 255.f), int(g_Options.Colors.backtrackdots_color[1] * 255.f), int(g_Options.Colors.backtrackdots_color[2] * 255.f));
			if (g_Options.Backtrack.backtrackenable)
			{
				if (local->IsAlive())
				{
					for (int t = 0; t < g_Options.Backtrack.backtrackticks; ++t)
					{
						Vector screenbacktrack[64][12];

						if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > local->GetSimulationTime())
						{
							if (g_Render->WorldToScreen(headPositions[i][t].HitboxPos, screenbacktrack[i][t]))
							{
								switch (g_Options.Backtrack.BacktrackType)
								{
								case 1:
									g_Surface->DrawSetColor(color);
									g_Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);
									break;
								}
							}
						}
					}
				}
				else
				{
					memset(&headPositions[0][0], 0, sizeof(headPositions));
				}
			}
			if (g_Options.Ragebot.Backtrack)
			{
				if (local->IsAlive())
				{
					for (int t = 0; t < 12; ++t)
					{
						Vector screenbacktrack[64][12];

						if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > local->GetSimulationTime())
						{
							if (g_Render->WorldToScreen(headPositions[i][t].HitboxPos, screenbacktrack[i][t]))
							{

								g_Surface->DrawSetColor(Color::White());
								g_Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);

							}
						}
					}
				}
				else
				{
					memset(&headPositions[0][0], 0, sizeof(headPositions));
				}
			}

			if (!g_Render->WorldToScreen(pos3D, pos))
				continue;

			Color clr = entity->GetTeamNum() == local->GetTeamNum() ?
				Color(g_Options.Colors.TeamESP[0] * 255, g_Options.Colors.TeamESP[1] * 255, g_Options.Colors.TeamESP[2] * 255, 255) :
				Color(g_Options.Colors.EnemyESP[0] * 255, g_Options.Colors.EnemyESP[1] * 255, g_Options.Colors.EnemyESP[2] * 255, 255);
			if (entity->GetTeamNum() == local->GetTeamNum() && !g_Options.Visuals.TeamESP)
				continue;
			if (!entity->IsAlive())
				continue;

			bool PVS = false;
			RECT rect = DynamicBox(entity, PVS, local);

			DrawInfo(rect, entity, local);

			if (g_Options.Visuals.Box && g_Options.Visuals.Enabled)
			{
				switch (g_Options.Visuals.BoxType)
				{
				case 0:
					DrawBox(rect, clr);
					break;
				case 1:
					DrawBoxTog(rect, clr);
					break;
				case 2:
					DrawCorners(rect, clr);
					break;
				}
			}
			if (g_Options.Visuals.fill && g_Options.Visuals.Enabled)
			{

				if (entity->GetTeamNum() == local->GetTeamNum())
				{
					g_Surface->DrawSetColor(Color(int(g_Options.Colors.fill_color_team[0] * 255.f), int(g_Options.Colors.fill_color_team[1] * 255.f), int(g_Options.Colors.fill_color_team[2] * 255.f), g_Options.Visuals.esp_fill_amount));
					ColorAlpha::ESP_ctx.clr_fill.SetAlpha(g_Options.Visuals.esp_fill_amount * 255.f);
					RenderFill(rect);
				}
				else if (entity->GetTeamNum() != local->GetTeamNum())
				{
					ColorAlpha::ESP_ctx.clr_fill.SetAlpha(g_Options.Visuals.esp_fill_amount * 255.f);
					g_Surface->DrawSetColor(Color(int(g_Options.Colors.fill_color_enemy[0] * 255.f), int(g_Options.Colors.fill_color_enemy[1] * 255.f), int(g_Options.Colors.fill_color_enemy[2] * 255.f), g_Options.Visuals.esp_fill_amount));

					RenderFill(rect);
				}
			}
			if (g_Options.Visuals.AimLine && g_Options.Visuals.Enabled)
			{
				DrawSnapLine(rect);
			}
			if (g_Options.Visuals.skeletonenbl && g_Options.Visuals.Enabled)
			{
				switch (g_Options.Visuals.skeletonopts)
				{
				case 0:
					Skeleton(entity, Color(int(g_Options.Colors.color_skeleton[0] * 255.f), int(g_Options.Colors.color_skeleton[1] * 255.f), int(g_Options.Colors.color_skeleton[2] * 255.f)));
					break;
				case 1:
					FingerSkeleton(entity, Color(int(g_Options.Colors.color_skeleton[0] * 255.f), int(g_Options.Colors.color_skeleton[1] * 255.f), int(g_Options.Colors.color_skeleton[2] * 255.f)));
					break;
				case 2:
					HealthSkeleton(entity);
					break;
				}
			}
			if (g_Options.Visuals.health && g_Options.Visuals.Enabled)
			{
				switch (g_Options.Visuals.healthtype)
				{
				case 0:
					DrawHealth(rect, entity);
					break;
				}
			}
			if (g_Options.Visuals.LBYTimer)
				RenderLBYTimer(entity, rect);
			if (g_Options.Visuals.armor && g_Options.Visuals.Enabled)
			{
				armorbar(rect, entity);
			} 
			if (g_Options.Visuals.barrel && g_Options.Visuals.Enabled)
			{
				BulletTrace(entity, Color(int(g_Options.Colors.BulletTraceColor[0] * 255.f), int(g_Options.Colors.BulletTraceColor[1] * 255.f), int(g_Options.Colors.BulletTraceColor[2] * 255.f)));
			}
			if (g_Options.Misc.radaringame)
				*(char*)((DWORD)(entity)+offsetz.DT_BaseEntity.m_bSpotted) = 1;
		}
	}
}

void visuals::Spoof()
{
	static auto sv_cheats = g_CVar->FindVar("sv_cheats");
	static auto spoof_cheats = new SpoofedConvar(sv_cheats);
	spoof_cheats->SetInt(1);

	static auto mat_postprocess_enable = g_CVar->FindVar("mat_postprocess_enable");

	if (g_Options.Visuals.NoPostProcess && g_Options.Visuals.Enabled)
	{
		mat_postprocess_enable->SetValue(0);
	}
	else
	{
		mat_postprocess_enable->SetValue(1);
	}

	static auto mat_drawgray = g_CVar->FindVar("mat_drawgray");

	if (g_Options.Visuals.snowmode && g_Options.Visuals.Enabled)
	{
		mat_drawgray->SetValue(1);
	}
	else
	{
		mat_drawgray->SetValue(0);
	}


	static auto mat_showmiplevels = g_CVar->FindVar("mat_showmiplevels");


	if (g_Options.Visuals.lsdmode && g_Options.Visuals.Enabled)
	{

		mat_showmiplevels->SetValue(1);
	}
	else
	{
		mat_showmiplevels->SetValue(0);
	}


	static auto r_showenvcubemap = g_CVar->FindVar("r_showenvcubemap");


	if (g_Options.Visuals.chromemode && g_Options.Visuals.Enabled)
	{
		r_showenvcubemap->SetValue(1);
	}
	else
	{
		r_showenvcubemap->SetValue(0);
	}


	static auto mat_showlowresimage = g_CVar->FindVar("mat_showlowresimage");


	if (g_Options.Visuals.minecraftmode && g_Options.Visuals.Enabled)
	{
		mat_showlowresimage->SetValue(1);
	}
	else
	{
		mat_showlowresimage->SetValue(0);
	}


	if (g_Options.Visuals.ambientlight && g_Options.Visuals.Enabled)
	{



		float AmbientRedAmount = g_Options.Colors.ambientlightcolor[0];
		float AmbientGreenAmount = g_Options.Colors.ambientlightcolor[1];
		float AmbientBlueAmount = g_Options.Colors.ambientlightcolor[2];



		ConVar* AmbientRedCvar = g_CVar->FindVar("mat_ambient_light_r");
		*(float*)((DWORD)&AmbientRedCvar->fnChangeCallback + 0xC) = NULL;
		AmbientRedCvar->SetValue(AmbientRedAmount);

		ConVar* AmbientGreenCvar = g_CVar->FindVar("mat_ambient_light_g");
		*(float*)((DWORD)&AmbientGreenCvar->fnChangeCallback + 0xC) = NULL;
		AmbientGreenCvar->SetValue(AmbientGreenAmount);

		ConVar* AmbientBlueCvar = g_CVar->FindVar("mat_ambient_light_b");
		*(float*)((DWORD)&AmbientBlueCvar->fnChangeCallback + 0xC) = NULL;
		AmbientBlueCvar->SetValue(AmbientBlueAmount);
	}

	else if (!g_Options.Visuals.ambientlight || !g_Options.Visuals.Enabled)
	{
		ConVar* AmbientRedCvar = g_CVar->FindVar("mat_ambient_light_r");
		*(float*)((DWORD)&AmbientRedCvar->fnChangeCallback + 0xC) = NULL;
		AmbientRedCvar->SetValue(0.f);

		ConVar* AmbientGreenCvar = g_CVar->FindVar("mat_ambient_light_g");
		*(float*)((DWORD)&AmbientGreenCvar->fnChangeCallback + 0xC) = NULL;
		AmbientGreenCvar->SetValue(0.f);

		ConVar* AmbientBlueCvar = g_CVar->FindVar("mat_ambient_light_b");
		*(float*)((DWORD)&AmbientBlueCvar->fnChangeCallback + 0xC) = NULL;
		AmbientBlueCvar->SetValue(0.f);
	}

}

void visuals::DrawBombPlanted(C_BaseEntity* entity, C_BaseEntity* local)
{
	BombCarrier = nullptr;

	float damage;
	char bombdamagestringdead[24];
	char bombdamagestringalive[24];

	Vector vOrig; Vector vScreen;
	vOrig = entity->GetVecOrigin();
	CCSBomb* Bomb = (CCSBomb*)entity;
	float flBlow = Bomb->GetC4BlowTime();
	float lifetime = flBlow - (g_Globals->interval_per_tick * local->GetTickBase());
	if (g_Render->WorldToScreen(vOrig, vScreen))
	{
		if (local->IsAlive())
		{
			float flDistance = local->GetEyePosition().DistTo(entity->GetEyePosition());
			float a = 450.7f;
			float b = 75.68f;
			float c = 789.2f;
			float d = ((flDistance - b) / c);
			float flDamage = a * exp(-d * d);

			damage = float((std::max)((int)ceilf(CSGO_Armor(flDamage, local->ArmorValue())), 0));

			sprintf_s(bombdamagestringdead, sizeof(bombdamagestringdead) - 1, "Health Left: 0");
			sprintf_s(bombdamagestringalive, sizeof(bombdamagestringalive) - 1, "Health Left: %.0f", local->GetHealth() - damage);
			if (lifetime > -2.f)
			{
				if (damage >= local->GetHealth())
				{
					g_Render->Text((int)vScreen.x, int(vScreen.y + 10), Color(250, 42, 42, 255), g_Render->font.ESP, bombdamagestringdead);
				}
				else if (local->GetHealth() > damage)
				{
					g_Render->Text((int)vScreen.x, int(vScreen.y + 10), Color(0, 255, 0, 255), g_Render->font.ESP, bombdamagestringalive);
				}
			}
		}
		char buffer[64];
		if (lifetime > 0.01f && !Bomb->IsBombDefused())
		{
			sprintf_s(buffer, "Bomb: %.1f", lifetime);
			g_Render->Text((int)vScreen.x, (int)vScreen.y, Color(250, 42, 42, 255), g_Render->font.ESP, buffer);
		}


	}

	g_Engine->GetScreenSize(width, height);
	int halfX = width / 2;
	int halfY = height / 2;


	if (Bomb->GetBombDefuser() > 0)
	{
		float countdown = Bomb->GetC4DefuseCountDown() - (local->GetTickBase() * g_Globals->interval_per_tick);
		if (countdown > 0.01f)
		{
			if (lifetime > countdown)
			{
				char defuseTimeString[24];
				sprintf_s(defuseTimeString, sizeof(defuseTimeString) - 1, "Defusing: %.1f", countdown);
				g_Render->Text(halfX - 50, halfY + 200, Color(0, 255, 0, 255), g_Render->font.ESP, defuseTimeString);
			}
			else
			{
				g_Render->Text(halfX - 50, halfY + 200, Color(255, 0, 0, 255), g_Render->font.ESP, "No Time! Run!");
			}
		}

	}
}

void visuals::DrawBomb(C_BaseEntity* entity, ClientClass* cClass)
{
	// Null it out incase bomb has been dropped or planted
	BombCarrier = nullptr;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)entity;
	Vector vOrig; Vector vScreen;
	vOrig = entity->GetVecOrigin();
	bool adopted = true;
	auto parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		C_BaseEntity* pParentEnt = (g_EntityList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}
	if (g_Options.Visuals.C4World)
	{
		if (adopted)
		{
			if (g_Render->WorldToScreen(vOrig, vScreen))
			{
				g_Render->Text((int)vScreen.x, (int)vScreen.y, Color(255, 20, 20, 255), g_Render->font.ESP, "Bomb");
			}
		}

	}


}
void visuals::edgyHealthBar(RECT rect, C_BaseEntity* pEntity)
{

	float top = 1.4;
	float right = 0;
	float left = 0;

	float HealthValue = pEntity->GetHealth();
	int iHealthValue = HealthValue;
	int Red = 255 - (HealthValue * 2.00);
	int Green = HealthValue * 2.00;
	float flBoxes = std::ceil(pEntity->GetHealth() / 10.f);

	float height = (rect.bottom - rect.top) * (HealthValue / 100);
	float height2 = (rect.bottom - rect.top) * (100 / 100);
	float flHeight = height2 / 10.f;


	g_Render->DrawRect(rect.left - 5, rect.top - 1, rect.left - 1, rect.bottom + 1, Color(0, 0, 0, 150));
	g_Render->DrawRect(rect.left - 4, rect.bottom - height, rect.left - 2, rect.bottom, Color(Red, Green, 0, 255));

	for (int i = 0; i < 10; i++)
		g_Render->Line(rect.left - 5, rect.top + i * flHeight, rect.left - 2, rect.top + i * flHeight, Color(0, 0, 0, 255));


}

void visuals::Hitbox(int index)
{
	if (g_Options.Visuals.HitboxDuration == 0.f || !g_Options.Visuals.Hitbox)
		return;

	C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	float duration = g_Options.Visuals.HitboxDuration;

	if (index < 0)
		return;

	Color color = Color(int(g_Options.Colors.hitbox[0] * 255.f), int(g_Options.Colors.hitbox[1] * 255.f), int(g_Options.Colors.hitbox[2] * 255.f));

	auto entity = reinterpret_cast<C_BaseEntity*>(g_EntityList->GetClientEntity(index));

	if (!entity)
		return;

	studiohdr_t* pStudioModel = g_ModelInfo->GetStudiomodel(entity->GetModel());

	if (!pStudioModel)
		return;

	static matrix3x4 pBoneToWorldOut[128];

	if (!entity->SetupBones(pBoneToWorldOut, MAXSTUDIOBONES, 256, g_Globals->curtime))
		return;

	mstudiohitboxset_t* pHitboxSet = pStudioModel->GetHitboxSet(0);
	if (!pHitboxSet)
		return;

	for (int i = 0; i < pHitboxSet->numhitboxes; i++)
	{
		mstudiobbox_t* pHitbox = pHitboxSet->pHitbox(i);
		if (!pHitbox)
			continue;

		Vector vMin, vMax;
		VectorTransform(pHitbox->bbmin, pBoneToWorldOut[pHitbox->bone], vMin); //nullptr???
		VectorTransform(pHitbox->bbmax, pBoneToWorldOut[pHitbox->bone], vMax);

		if (pHitbox->m_flRadius > -1)
		{
			g_DebugOverlay->AddCapsuleOverlay(vMin, vMax, pHitbox->m_flRadius, color.r(), color.g(), color.b(), 100, duration);
		}
	}
}
//
void visuals::armorbar(RECT rect, C_BaseEntity* pEntity)
{
	float HealthValue2 = pEntity->ArmorValue();
	if (HealthValue2 > 100)
		HealthValue2 = 100;

	char hp[256];
	sprintf(hp, "%.0f", HealthValue2);
	float height = (rect.bottom - rect.top) * (HealthValue2 / 100);
	g_Render->GradientH(rect.right, rect.top - 1, 5, rect.bottom - rect.top + 2, Color(0, 0, 0, 150), Color(0, 0, 0, 150));
	g_Render->GradientH(rect.right + 1, rect.bottom - height, 3, height, Color(50, 16, 255, 255), Color(50, 16, 255, 255));
}

void visuals::DrawBox(RECT rect, Color Col)
{
	g_Render->DrawOutlinedRect(rect.left - 1, rect.top - 1, rect.right - rect.left + 2, rect.bottom - rect.top + 2, Color(0, 0, 0, 150));
	g_Render->DrawOutlinedRect(rect.left + 1, rect.top + 1, rect.right - rect.left - 2, rect.bottom - rect.top - 2, Color(0, 0, 0, 125));
	g_Render->DrawOutlinedRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, Col);
}
void visuals::DrawBoxTog(RECT rect, Color Col)
{
	if (GetKeyState(g_Options.Visuals.BoxHotkey)) {
		g_Render->DrawOutlinedRect(rect.left - 1, rect.top - 1, rect.right - rect.left + 2, rect.bottom - rect.top + 2, Color(0, 0, 0, 150));
		g_Render->DrawOutlinedRect(rect.left + 1, rect.top + 1, rect.right - rect.left - 2, rect.bottom - rect.top - 2, Color(0, 0, 0, 125));
		g_Render->DrawOutlinedRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, Col);
	}
}
void visuals::RenderFill(RECT rect)
{
	g_Surface->DrawFilledRect(rect.left + 2, rect.top + 2, rect.right - 2, rect.bottom - 2);

}

void visuals::DrawCorners(RECT rect, Color Col)
{
	int x1, y1, x2, y2, w, h;
	x1 = rect.left;
	y1 = rect.top;
	x2 = rect.right;
	y2 = rect.bottom;

	w = x2;
	h = y2;

	int Line_Size = (y1 - h) / 6;
	int Line_Size2 = (y1 - h) / 6;

	int red = 0;
	int green = 0;
	int blue = 0;
	int alpha = 0;
	Col.GetColor(red, green, blue, alpha);
	g_Surface->DrawSetColor(red, green, blue, alpha);

	//top inwards
	g_Surface->DrawLine(w, y1, w + Line_Size, y1);
	g_Surface->DrawLine(x1, y1, x1 - Line_Size, y1);

	//top downwards
	g_Surface->DrawLine(x1, y1, x1, y1 - Line_Size);
	g_Surface->DrawLine(w, y1, w, y1 - Line_Size);

	//bottom inwards
	g_Surface->DrawLine(x1, h, x1 - Line_Size, h);
	g_Surface->DrawLine(w, h, w + Line_Size, h);

	//bottom upwards
	g_Surface->DrawLine(x1, h, x1, h + Line_Size);
	g_Surface->DrawLine(w, h, w, h + Line_Size);

	//outlines

	g_Surface->DrawSetColor(0, 0, 0, 200);

	//top inwards
	g_Surface->DrawLine(w, y1 - 1, w + Line_Size, y1 - 1);
	g_Surface->DrawLine(x1, y1 - 1, x1 - Line_Size, y1 - 1);
	//inlines
	g_Surface->DrawLine(w - 1, y1 + 1, w + Line_Size, y1 + 1);
	g_Surface->DrawLine(x1 + 1, y1 + 1, x1 - Line_Size, y1 + 1);

	// top downwards
	g_Surface->DrawLine(x1 - 1, y1 - 1, x1 - 1, y1 - Line_Size);
	g_Surface->DrawLine(w + 1, y1 - 1, w + 1, y1 - Line_Size);
	//inlines
	g_Surface->DrawLine(x1 + 1, y1, x1 + 1, y1 - Line_Size);
	g_Surface->DrawLine(w - 1, y1, w - 1, y1 - Line_Size);

	//bottom inwards
	g_Surface->DrawLine(x1, h + 1, x1 - Line_Size, h + 1);
	g_Surface->DrawLine(w, h + 1, w + Line_Size, h + 1);
	//inlines
	g_Surface->DrawLine(x1 + 1, h - 1, x1 - Line_Size, h - 1);
	g_Surface->DrawLine(w - 1, h - 1, w + Line_Size, h - 1);

	//bottom upwards
	g_Surface->DrawLine(x1 - 1, h + 1, x1 - 1, h + Line_Size);
	g_Surface->DrawLine(w + 1, h + 1, w + 1, h + Line_Size);
	//inlines
	g_Surface->DrawLine(x1 + 1, h, x1 + 1, h + Line_Size);
	g_Surface->DrawLine(w - 1, h, w - 1, h + Line_Size);
}

void visuals::ThreeDBox(Vector minin, Vector maxin, Vector pos, Color Col)
{
	Vector min = minin + pos;
	Vector max = maxin + pos;

	Vector corners[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(min.x, min.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(max.x, max.y, max.z),
		Vector(max.x, min.y, max.z) };

	int edges[12][2] = { { 0, 1 },{ 1, 2 },{ 2, 3 },{ 3, 0 },{ 4, 5 },{ 5, 6 },{ 6, 7 },{ 7, 4 },{ 0, 4 },{ 1, 5 },{ 2, 6 },{ 3, 7 }, };

	for (const auto edge : edges)
	{
		Vector p1, p2;
		if (!g_Render->WorldToScreen(corners[edge[0]], p1) || !g_Render->WorldToScreen(corners[edge[1]], p2))
			return;
		int red = 0;
		int green = 0;
		int blue = 0;
		int alpha = 0;
		Col.GetColor(red, green, blue, alpha);
		g_Surface->DrawSetColor(red, green, blue, alpha);
		g_Surface->DrawLine(p1.x, p1.y, p2.x, p2.y);
	}

}

void visuals::DrawInfo(RECT rect, C_BaseEntity* entity, C_BaseEntity* local)
{
	player_info_t info;
	static class Text
	{
	public:
		std::string text;
		int side;
		int Font;
		Color color;

		Text(std::string text, int side, int Font, Color color) : text(text), side(side), Font(Font), color(color)
		{
		}
	};
	std::vector< Text > texts;
	if (g_Engine->GetPlayerInfo(entity->GetIndex(), &info))
	{
		if (entity == BombCarrier && g_Options.Visuals.BombCarrier &&  g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("Bomb Carrier", 1, g_Render->font.ESP, Color(255, 220, 220, 255)));
		}

		if (g_Options.Visuals.Flashed && entity->IsFlashed() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("Flashed", 1, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

		if (g_Options.Visuals.IsHasDefuser && entity->hasDefuser() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("r", 1, g_Render->font.Icon, Color(255, 255, 183, 255))); //b1g defuser icon
		}

		if (g_Options.Visuals.IsDefusing &&  entity->IsDefusing() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("Defusing", 1, g_Render->font.ESP, Color(255, 255, 183, 255)));
		}

		if (g_Options.Visuals.Name && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text(info.name, 0, g_Render->font.ESP, Color(255, 255, 255, 255)));
		}

		switch (g_Options.Visuals.Armor2)
		{
		case 0:
			break;
		case 1:
			texts.push_back(Text(entity->GetArmorName(), 1, g_Render->font.ESP, Color(255, 255, 255, 255)));
			break;
		case 2:
			texts.push_back(Text(entity->GetArmorIcon(), 1, g_Render->font.Icon, Color(255, 255, 255, 255)));
			break;
		default:
			break;
		}

		if (g_Options.Visuals.Distance && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text(std::to_string(flGetDistance(local->GetVecOrigin(), entity->GetVecOrigin())) + std::string("FT"), 2, g_Render->font.ESP, Color(255, 255, 255, 255)));
		}

		if (g_Options.Visuals.resolveMode && g_Options.Visuals.Enabled)
		{
			/*
			if (Globals::resolvemode == 1)
			Render::Text(MidX - 30, MidY - 40, Color(150, 100, 9, 255), Render::Fonts::ESP, "Last Moving LBY");

		else if (Globals::resolvemode == 2)
			Render::Text(MidX - 30, MidY - 55, Color(200, 100, 9, 255), Render::Fonts::ESP, "FakeWalk");

		else if (Globals::resolvemode == 3)
			Render::Text(MidX - 30, MidY - 85, Color(0, 150, 9, 255), Render::Fonts::ESP, "LBY Update");

		else if (Globals::resolvemode == 4)
			Render::Text(MidX - 30, MidY - 100, Color(255, 200, 100, 255), Render::Fonts::ESP, "Last Stand LBY");

		else if (Globals::resolvemode == 5)
			Render::Text(MidX - 30, MidY - 115, Color(0, 200, 0, 255), Render::Fonts::ESP, "Bruteforce");

		else if (Globals::resolvemode == 6)
			Render::Text(MidX - 30, MidY - 130, Color(255, 50, 0, 255), Render::Fonts::ESP, "Break > 120");
			*/
			if (Globals::resolvemode == 1)
			{
				texts.push_back(Text(std::string("Resolver: Last Moving LBY"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 2)
			{
				texts.push_back(Text(std::string("Resolver: FakeWalk"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 3)
			{
				texts.push_back(Text(std::string("Resolver: LBY Update"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 4)
			{
				texts.push_back(Text(std::string("Resolver: Last Stand LBY"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 5)
			{
				texts.push_back(Text(std::string("Resolver: Bruteforce"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
			else if (Globals::resolvemode == 6)
			{
				texts.push_back(Text(std::string("Resolver: LBY Break > 120"), 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
			}
		}

		CBaseCombatWeapon* weapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(entity->GetActiveWeaponHandle());
		if (weapon)
		{
			switch (g_Options.Visuals.Weapon)
			{
			case 0:
				break;
			case 1:
				if (MiscFunctions::IsKnife(weapon) || MiscFunctions::IsGrenade(weapon))
				{
					texts.push_back(Text(weapon->GetGunName(), 2, g_Render->font.ESP, Color(255, 255, 255, 255)));
				}

				else
					texts.push_back(Text(weapon->GetGunName(), 2, g_Render->font.ESP, Color(255, 255, 255, 255)));
				break;
			case 2:
				texts.push_back(Text(weapon->GetGunIcon(), 2, g_Render->font.Icon, Color(255, 255, 255, 255)));
				break;
			default:
				break;
			}
		}

		if (g_Options.Visuals.Scoped && entity->IsScoped() && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text("SCOPED", 1, g_Render->font.ESPan, Color(0, 202, 247, 255)));
		}

		//		if (g_Options.Visuals.Reloading && weapon->IsInReload() && g_Options.Visuals.Enabled)
		//		{
		//			texts.push_back(Text("*Reloading*", 0, g_Render->font.ESP, Color(255, 255, 183, 255)));
		//		}


		if (g_Options.Visuals.Money && g_Options.Visuals.Enabled)
		{
			texts.push_back(Text(std::string("$") + std::to_string(entity->iAccount()), 1, g_Render->font.ESP, Color(71, 132, 60, 255)));
		}

	}


	int middle = ((rect.right - rect.left) / 2) + rect.left;
	int Top[3] = { rect.top,rect.top, rect.bottom };
	for (auto text : texts)
	{
		RECT nameSize = g_Render->GetTextSize(text.Font, (char*)text.text.c_str());
		switch (text.side)
		{
		case 0:
			Top[0] -= nameSize.bottom + 1;
			g_Render->DrawString2(text.Font, middle, Top[0] + 8, text.color, FONT_CENTER, (char*)text.text.c_str());
			break;
		case 1:
			g_Render->DrawString2(text.Font, rect.right + 2, Top[1] + 8, text.color, FONT_LEFT, (char*)text.text.c_str());
			Top[1] += nameSize.bottom - 4;
			break;
		case 2:
			g_Render->DrawString2(text.Font, middle, Top[2] + 6, text.color, FONT_CENTER, (char*)text.text.c_str());
			Top[2] += nameSize.bottom - 4;
			break;
		}
	}

}
/*void visuals::AmmoBox(RECT rect, C_BaseEntity* pPlayer, Color color)
{
	auto AnimLayer = &pPlayer->GetAnimationOverlay()[1];

	if (!AnimLayer->m_pOwner)
		return;

	auto activity = pPlayer->GetSequenceActivity(AnimLayer->m_nSequence); //g_ModelInfo->GetStudiomodel(pPlayer->GetModel())

	if (CBaseCombatWeapon* weapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pPlayer->GetActiveWeaponHandle()))
	{

		if (CSWeaponInfo* winfo = weapon->GetCSWpnData())
		{
			CCSWeaponInfo* oof;
			float currentammo = weapon->ammo();
			float maxammo = oof->GetMaxClip();
			g_Render->DrawRect(rect.left - 1, rect.bottom + 2, rect.right + 1, rect.bottom + 6, Color(0, 0, 0, 255));

			static float ammob4reload[64]; // for the aesthetic so it doesnt jump to the current ammo before reload animation
			if (activity == 967 && AnimLayer->m_flWeight != 0.f) // if reloading
			{
				float amount = ammob4reload[pPlayer->GetIndex()] / maxammo;
				float cycle = AnimLayer->m_flCycle;
				int length = (rect.right - rect.left) * amount;
				int reloadX = ((rect.right - rect.left) - length) * cycle;
				g_Render->DrawRect(rect.left, rect.bottom + 3, rect.left + length + reloadX, rect.bottom + 5, color);
				g_Render->DrawString2(g_Render->font.ESP, rect.left + length + reloadX, rect.bottom - 1, Color(255, 255, 255, 255), false, "RELOADING");
			}
			else // if not reloading
			{
				if (currentammo < 0) // draw a bar for weapons that do not have ammo (grenades, knives) for the aesthetic
				{
					g_Render->DrawRect(rect.left, rect.bottom + 3, rect.right, rect.bottom + 5, color);
				}
				if (currentammo >= 0) // if the bitch got bullets its probably a gun and we shud display the ammo count if it aint full also draw that sexy bar MMMMMM
				{
					float amount = currentammo / maxammo;
					int length = (rect.right - rect.left) * amount;
					ammob4reload[pPlayer->GetIndex()] = currentammo;
					if (currentammo != 0)
						g_Render->DrawRect(rect.left, rect.bottom + 3, rect.left + length, rect.bottom + 5, color);

					if (currentammo != maxammo)
						g_Render->DrawString2(g_Render->font.ESP, rect.left + length, rect.bottom - 1, Color(255, 255, 255), true, "%.0f", currentammo);
				}
			}
		}
	}
} */
void visuals::DrawAngles()
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = pLocal;
	src3D = pLocal->GetVecOrigin();

	AngleVectors(QAngle(0, Globals::RealAngle, 0), &forward);
	dst3D = src3D + (forward * 45.f);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;

	g_Render->Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));

	if (g_Options.Visuals.angleLinesName)
		g_Render->Text(dst.x, dst.y, Color(0, 255, 0, 255), g_Render->font.ESP, "REAL");

	AngleVectors(QAngle(0, Globals::FakeAngle, 0), &forward);
	dst3D = src3D + (forward * 45.f);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;

	g_Render->Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));

	if (g_Options.Visuals.angleLinesName)
		g_Render->Text(dst.x, dst.y, Color(255, 0, 0, 255), g_Render->font.ESP, "FAKE");

	AngleVectors(QAngle(0, pLocal->GetLowerBodyYaw(), 0), &forward);
	dst3D = src3D + (forward * 45.f);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;

	g_Render->Line(src.x, src.y, dst.x, dst.y, Color(255, 127, 0, 255));

	if (g_Options.Visuals.angleLinesName)
		g_Render->Text(dst.x, dst.y, Color(255, 0, 0, 255), g_Render->font.ESP, "LBY");

}

void visuals::Radar(C_BaseEntity* entity)
{
	DWORD m_bSpotted = offsetz.DT_BaseEntity.m_bSpotted;
	*(char*)((DWORD)(entity)+m_bSpotted) = 1;
}

RECT visuals::DynamicBox(C_BaseEntity* pPlayer, bool& PVS, C_BaseEntity* local)
{
	Vector trans = pPlayer->GetVecOrigin();

	Vector min;
	Vector max;

	min = pPlayer->GetCollideable()->OBBMins();
	max = pPlayer->GetCollideable()->OBBMaxs();

	Vector pointList[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector Distance = pointList[0] - pointList[1];
	int dst = Distance.Length();
	dst /= 1.3f;
	Vector angs;
	CalcAngle(trans, local->GetEyePosition(), angs);

	Vector all[8];
	angs.y += 45;
	for (int i = 0; i < 4; i++)
	{
		AngleVectors(angs, &all[i]);
		all[i] *= dst;
		all[i + 4] = all[i];
		all[i].z = max.z;
		all[i + 4].z = min.z;
		VectorAdd(all[i], trans, all[i]);
		VectorAdd(all[i + 4], trans, all[i + 4]);
		angs.y += 90;
	}

	Vector flb, brt, blb, frt, frb, brb, blt, flt;
	PVS = true;

	if (!g_DebugOverlay->ScreenPosition(all[3], flb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[0], blb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[2], frb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[6], blt))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[5], brt))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[4], frt))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[1], brb))
		PVS = false;
	if (!g_DebugOverlay->ScreenPosition(all[7], flt))
		PVS = false;

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	float left = flb.x;
	float top = flb.y;
	float right = flb.x;
	float bottom = flb.y;

	for (int i = 0; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
	}
	RECT rect;
	rect.left = left;
	rect.bottom = bottom;
	rect.right = right;
	rect.top = top;

	return rect;
}
bool visuals::GetBBox(C_BaseEntity* entity, visuals::ESPBox &result)
{

	// Variables
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;


	vOrigin = entity->GetAbsOrigin2();
	min = entity->collisionProperty()->GetMins() + vOrigin;
	max = entity->collisionProperty()->GetMaxs() + vOrigin;

	// Points of a 3d bounding box
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	// Get screen positions
	if (!g_Render->WorldToScreen(points[3], flb) || !g_Render->WorldToScreen(points[5], brt)
		|| !g_Render->WorldToScreen(points[0], blb) || !g_Render->WorldToScreen(points[4], frt)
		|| !g_Render->WorldToScreen(points[2], frb) || !g_Render->WorldToScreen(points[1], brb)
		|| !g_Render->WorldToScreen(points[6], blt) || !g_Render->WorldToScreen(points[7], flt))
		return false;

	// Put them in an array (maybe start them off in one later for speed?)
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	// Init this shit
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	// Find the bounding corners for our box
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}
	RECT rect;
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;

	// Width / height
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;
	return true;
}
void visuals::RenderWeapon(C_BaseEntity* entity, Color color)
{

	if (entity)
	{
		CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)entity;

		auto owner = pWeapon->GetOwnerHandle();

		if (owner > -1)
			return;

		Vector pos3D = entity->GetAbsOrigin2();

		if (pos3D.x == 0.0f && pos3D.y == 0.0f && pos3D.z == 0.0f)
			return;

		Vector pos;


		if (!g_Render->WorldToScreen(pos3D, pos))
			return;

		int weaponID = pWeapon->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		auto weaponName = ItemDefinitionIndexToString(weaponID);
		auto weaponIcon = ItemDefinitionIndexToString2(weaponID);

		switch (g_Options.Visuals.DroppedGunsType)
		{
		case 0:
			break;
		case 1:
			g_Render->Text(pos.x, pos.y, color, g_Render->font.Guns, weaponName);
			break;
		case 2:
			g_Render->Text(pos.x, pos.y + 10, color, g_Render->font.Icon, weaponIcon);
			break;
		default:
			break;
		}

	}
}

void visuals::BulletTrace(C_BaseEntity* pEntity, Color color)
{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	Vector eyes = *pEntity->GetEyeAngles();

	AngleVectors(eyes, &forward);
	filter.pSkip = pEntity;
	src3D = pEntity->GetBonePos(6) - Vector(0, 0, 0);
	dst3D = src3D + (forward * g_Options.Visuals.barrelL);

	ray.Init(src3D, dst3D);

	g_EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (!g_Render->WorldToScreen(src3D, src) || !g_Render->WorldToScreen(tr.endpos, dst))
		return;


	g_Render->Line(src.x, src.y, dst.x, dst.y, color);
	g_Render->DrawOutlinedRect(dst.x - 3, dst.y - 3, 6, 6, color);

};

void visuals::Skeleton(C_BaseEntity* pEntity, Color Col)
{
	studiohdr_t* pStudioHdr = g_ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && pBone->flags & BONE_USED_BY_HITBOX && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);

			int iChestBone = 6;  // Parameter of relevant Bone number
			Vector vBreastBone; // New reference Point for connecting many bones
			Vector vUpperDirection = pEntity->GetBonePos(iChestBone + 1) - pEntity->GetBonePos(iChestBone); // direction vector from chest to neck
			vBreastBone = pEntity->GetBonePos(iChestBone) + vUpperDirection / 2;
			Vector vDeltaChild = vChild - vBreastBone; // Used to determine close bones to the reference point
			Vector vDeltaParent = vParent - vBreastBone;

			// Eliminating / Converting all disturbing bone positions in three steps.
			if ((vDeltaParent.Length() < 9 && vDeltaChild.Length() < 9))
				vParent = vBreastBone;

			if (j == iChestBone - 1)
				vChild = vBreastBone;

			if (abs(vDeltaChild.z) < 5 && (vDeltaParent.Length() < 5 && vDeltaChild.Length() < 5) || j == iChestBone)
				continue;

			g_DebugOverlay->ScreenPosition(vParent, sParent);
			g_DebugOverlay->ScreenPosition(vChild, sChild);

			g_Surface->DrawSetColor(Color(int(g_Options.Colors.color_skeleton[0] * 255.f), int(g_Options.Colors.color_skeleton[1] * 255.f), int(g_Options.Colors.color_skeleton[2] * 255.f)));
			g_Surface->DrawLine(sParent[0], sParent[1], sChild[0], sChild[1]);
		}
	}
}

void visuals::FingerSkeleton(C_BaseEntity* pEntity, Color Col)
{
	studiohdr_t* pStudioHdr = g_ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && pBone->flags & BONE_USED_BY_HITBOX | BONE_USED_BY_ATTACHMENT && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);

			int iChestBone = 6;  // Parameter of relevant Bone number
			Vector vBreastBone; // New reference Point for connecting many bones
			Vector vUpperDirection = pEntity->GetBonePos(iChestBone + 1) - pEntity->GetBonePos(iChestBone); // direction vector from chest to neck
			vBreastBone = pEntity->GetBonePos(iChestBone) + vUpperDirection / 2;
			Vector vDeltaChild = vChild - vBreastBone; // Used to determine close bones to the reference point
			Vector vDeltaParent = vParent - vBreastBone;

			// Eliminating / Converting all disturbing bone positions in three steps.
			if ((vDeltaParent.Length() < 9 && vDeltaChild.Length() < 9))
				vParent = vBreastBone;

			if (j == iChestBone - 1)
				vChild = vBreastBone;

			if (abs(vDeltaChild.z) < 5 && (vDeltaParent.Length() < 5 && vDeltaChild.Length() < 5) || j == iChestBone)
				continue;

			if ((pBone->flags & BONE_USED_BY_HITBOX ^ BONE_USED_BY_HITBOX) && (vDeltaParent.Length() < 19 && vDeltaChild.Length() < 19))
				continue;

			g_DebugOverlay->ScreenPosition(vParent, sParent);
			g_DebugOverlay->ScreenPosition(vChild, sChild);

			g_Surface->DrawSetColor(Color(int(g_Options.Colors.color_skeleton[0] * 255.f), int(g_Options.Colors.color_skeleton[1] * 255.f), int(g_Options.Colors.color_skeleton[2] * 255.f)));
			g_Surface->DrawLine(sParent[0], sParent[1], sChild[0], sChild[1]);
		}
	}
}

void visuals::HealthSkeleton(C_BaseEntity* pEntity)
{

	studiohdr_t* pStudioHdr = g_ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);


		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);

			int iChestBone = 6;  // Parameter of relevant Bone number
			Vector vBreastBone; // New reference Point for connecting many bones
			Vector vUpperDirection = pEntity->GetBonePos(iChestBone + 1) - pEntity->GetBonePos(iChestBone); // direction vector from chest to neck
			vBreastBone = pEntity->GetBonePos(iChestBone) + vUpperDirection / 2;
			Vector vDeltaChild = vChild - vBreastBone; // Used to determine close bones to the reference point
			Vector vDeltaParent = vParent - vBreastBone;

			// Eliminating / Converting all disturbing bone positions in three steps.
			if ((vDeltaParent.Length() < 9 && vDeltaChild.Length() < 9))
				vParent = vBreastBone;

			if (j == iChestBone - 1)
				vChild = vBreastBone;

			if (abs(vDeltaChild.z) < 5 && (vDeltaParent.Length() < 5 && vDeltaChild.Length() < 5) || j == iChestBone)
				continue;

			if (pEntity->GetHealth() == 100)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(0, 255, 0));
			}

			if (pEntity->GetHealth() < 100 & pEntity->GetHealth() > 85 || pEntity->GetHealth() == 85)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(114, 255, 00));

			}
			else if (pEntity->GetHealth() < 85 & pEntity->GetHealth() > 60 || pEntity->GetHealth() == 60)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(178, 255, 0));
			}
			else if (pEntity->GetHealth() < 60 & pEntity->GetHealth() > 45 || pEntity->GetHealth() == 45)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(255, 229, 0));
			}
			else if (pEntity->GetHealth() < 45 & pEntity->GetHealth() > 30 || pEntity->GetHealth() == 30)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(255, 127, 0));
			}
			else  if (pEntity->GetHealth() < 30 & pEntity->GetHealth() > 15 || pEntity->GetHealth() == 15)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(255, 55, 0));
			}
			else  if (pEntity->GetHealth() < 15 & pEntity->GetHealth() > 0)
			{
				g_DebugOverlay->ScreenPosition(vParent, sParent);
				g_DebugOverlay->ScreenPosition(vChild, sChild);
				g_Render->Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(191, 0, 0));
			}
		}
	}
}

void visuals::DrawSnapLine(RECT rect)
{
	Color color = Color(int(g_Options.Colors.AimLineColor[0] * 255.f), int(g_Options.Colors.AimLineColor[1] * 255.f), int(g_Options.Colors.AimLineColor[2] * 255.f));

	int width, height;
	g_Engine->GetScreenSize(width, height);

	int screen_x = width * 0.5f,
		screen_y = height,
		target_x = rect.left + (rect.right - rect.left) * 0.5,
		target_y = rect.bottom,
		max_length = height * 0.3f;

	if (target_x == 0 ||
		target_y == 0)
		return;

	float length = sqrt(pow(target_x - screen_x, 2) + pow(target_y - screen_y, 2));
	if (length > max_length)
	{
		float
			x_normalized = (target_x - screen_x) / length,
			y_normalized = (target_y - screen_y) / length;
		target_x = screen_x + x_normalized * max_length;
		target_y = screen_y + y_normalized * max_length;
		g_Render->OutlineCircle(target_x + x_normalized * 3.5f, target_y + y_normalized * 3.5f, 8.f, 80, color);
	}

	g_Surface->DrawSetColor(color);
	g_Surface->DrawLine(screen_x, screen_y, target_x, target_y);

	/*	int width = 0;
	int height = 0;
	g_Engine->GetScreenSize(width, height);
	Vector From((width / 2), height - 1, 0);
	g_Render->Line(From.x, From.y, to.x, to.y, clr); */
}

void visuals::DrawHealth(RECT rect, C_BaseEntity* pPlayer)
{
	float HealthValue2 = pPlayer->GetHealth();
	if (HealthValue2 > 100)
		HealthValue2 = 100;

	char hp[256];
	sprintf(hp, "%.0f", HealthValue2);
	float height = (rect.bottom - rect.top) * (HealthValue2 / 100);
	g_Render->GradientH(rect.left - 6, rect.top - 1, 5, rect.bottom - rect.top + 2, Color(0, 0, 0, 150), Color(0, 0, 0, 150));
	g_Render->GradientH(rect.left - 5, rect.bottom - height, 3, height, Color(0, 255, 100, 255), Color(0, 255, 100, 255));
}

void visuals::Hitmarker()
{
	if (G::hitmarkeralpha < 0.f)
		G::hitmarkeralpha = 0.f;
	else if (G::hitmarkeralpha > 0.f)
		G::hitmarkeralpha -= 0.01f;

	int W, H;
	g_Engine->GetScreenSize(W, H);

	if (G::hitmarkeralpha > 0.f)
	{
		g_Render->Line(W / 2 - 8, H / 2 - 8, W / 2 - 3, H / 2 - 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));
		g_Render->Line(W / 2 - 8, H / 2 + 8, W / 2 - 3, H / 2 + 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));
		g_Render->Line(W / 2 + 8, H / 2 - 8, W / 2 + 3, H / 2 - 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));
		g_Render->Line(W / 2 + 8, H / 2 + 8, W / 2 + 3, H / 2 + 3, Color(int(g_Options.Colors.hitmarker_color[0] * 255.f), int(g_Options.Colors.hitmarker_color[1] * 255.f), int(g_Options.Colors.hitmarker_color[2] * 255.f), (G::hitmarkeralpha * 255.f)));

	}
}

void visuals::SkyChanger()
{
	static auto sv_skyname = g_CVar->FindVar("sv_skyname");

	switch (g_Options.Visuals.SkyboxChanger)
	{
	case 1: //Baggage
		sv_skyname->SetValue("cs_baggage_skybox_");
		break;
	case 2: //Tibet
		sv_skyname->SetValue("cs_tibet");
		break;
	case 3: //Clear Sky
		sv_skyname->SetValue("clearsky");
		break;
	case 4: //Clear Sky HD
		sv_skyname->SetValue("clearsky_hdr");
		break;
	case 5: //Embassy
		sv_skyname->SetValue("embassy");
		break;
	case 6: //Italy
		sv_skyname->SetValue("italy");
		break;
	case 7: //Daylight 1
		sv_skyname->SetValue("sky_cs15_daylight01_hdr");
		break;
	case 8: //Daylight 2
		sv_skyname->SetValue("sky_cs15_daylight02_hdr");
		break;
	case 9: //Daylight 3
		sv_skyname->SetValue("sky_cs15_daylight03_hdr");
		break;
	case 10: //Daylight 4
		sv_skyname->SetValue("sky_cs15_daylight04_hdr");
		break;
	case 11: //Cloudy
		sv_skyname->SetValue("sky_csgo_cloudy01");
		break;
	case 12: //Night 1
		sv_skyname->SetValue("sky_csgo_night02");
		break;
	case 13: //Night 2
		sv_skyname->SetValue("sky_csgo_night02b");
		break;
	case 14: //Night Flat
		sv_skyname->SetValue("sky_csgo_night_flat");
		break;
	case 15: //Day HD
		sv_skyname->SetValue("sky_day02_05_hdr");
		break;
	case 16: //Day
		sv_skyname->SetValue("sky_day02_05");
		break;
	case 17: //Black
		sv_skyname->SetValue("sky_l4d_rural02_ldr");
		break;
	case 18: //Vertigo HD
		sv_skyname->SetValue("vertigo_hdr");
		break;
	case 19: //Vertigo Blue HD
		sv_skyname->SetValue("vertigoblue_hdr");
		break;
	case 20: //Vertigo
		sv_skyname->SetValue("vertigo");
		break;
	case 21: //Vietnam
		sv_skyname->SetValue("vietnam");
		break;
	case 22: //Dusty Sky
		sv_skyname->SetValue("sky_dust");
		break;
	case 23: //Jungle
		sv_skyname->SetValue("jungle");
		break;
	case 24: //Nuke
		sv_skyname->SetValue("nukeblank");
		break;
	case 25: //Office
		sv_skyname->SetValue("office");
		break;
	}
	
}
void visuals::ColorModulateSkybox()
{
	if (g_Options.Colors.ColorSkybox)
	{
		for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
		{
			IMaterial *pMaterial = g_MaterialSystem->GetMaterial(i);

			if (!pMaterial)
				continue;

			const char* group = pMaterial->GetTextureGroupName();
			const char* name = pMaterial->GetName();
			if (strstr(pMaterial->GetTextureGroupName(), "SkyBox textures"))
			{
				switch (g_Options.Colors.SkyColor)
				{
				case 0:
					pMaterial->ColorModulate(1, 1, 1);
					break;
				case 1:
					pMaterial->ColorModulate(0.77, 0.02, 0.77);
					break;
				case 2:
					pMaterial->ColorModulate(0.77, 0.02, 0.02);
					break;
				case 3:
					pMaterial->ColorModulate(0.02, 0.02, 0.77);
					break;
				case 4:
					pMaterial->ColorModulate(0.02, 0.77, 0.02);
					break;
				}
			}

		}
	}
}
void visuals::NightMode()
{		
	
	static auto sv_skyname = g_CVar->FindVar("sv_skyname");
	if (g_Options.Visuals.nightMode)
	{

		if (!done)
		{

			sv_skyname->SetValue("sky_csgo_night02");

			for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
			{
				IMaterial *pMaterial = g_MaterialSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(group, "World textures"))
				{
					pMaterial->ColorModulate(0.10, 0.10, 0.10);
				}
				if (strstr(group, "StaticProp"))
				{
					pMaterial->ColorModulate(0.30, 0.30, 0.30);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{
					pMaterial->ColorModulate(0.30, 0.30, 0.30);
				}

				if (strstr(group, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
				}
				done = true;
			}

		}
	}
	else
	{
		if (done)
		{
			for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
			{
				sv_skyname->SetValue("nukeblank");
				IMaterial *pMaterial = g_MaterialSystem->GetMaterial(i);

				if (!pMaterial)
					continue;

				const char* group = pMaterial->GetTextureGroupName();
				const char* name = pMaterial->GetName();

				if (strstr(group, "World textures"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "StaticProp"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(name, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
				if (strstr(name, "models/props/de_dust/palace_pillars"))
				{

					pMaterial->ColorModulate(1, 1, 1);
				}
				if (strstr(group, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
			}
			done = false;
		}
	}

}


void visuals::DoAsusWalls()
{

	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());


	if (!pLocal || !g_Engine->IsConnected() || !g_Engine->IsInGame())
		return;
	if (g_Options.Visuals.AsusProps)
	{

		static auto staticdrop = g_CVar->FindVar("r_DrawSpecificStaticProp");
		staticdrop->SetValue(0);

		for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
		{
			IMaterial *pmat = g_MaterialSystem->GetMaterial(i);

			if (!pmat)
				continue;

			if (!strcmp(pmat->GetTextureGroupName(), "StaticProp textures"))
			{
				pmat->AlphaModulate(0.68f);
			}
		}
	}
	else
	{
		static auto staticdrop = g_CVar->FindVar("r_DrawSpecificStaticProp");
		staticdrop->SetValue(1);

		for (MaterialHandle_t i = g_MaterialSystem->FirstMaterial(); i != g_MaterialSystem->InvalidMaterial(); i = g_MaterialSystem->NextMaterial(i))
		{
			IMaterial *pmat = g_MaterialSystem->GetMaterial(i);

			if (!pmat)
				continue;

			if (!strcmp(pmat->GetTextureGroupName(), "StaticProp textures"))
			{
				pmat->AlphaModulate(1);
			}
		}
	}

}

void visuals::renderBeams()
{

	if (g_Options.Visuals.bulletshow && g_Options.Visuals.Enabled)
		return;

	auto local = static_cast<C_BaseEntity*>(g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer()));
	if (!local)
		return;

	for (size_t i = 0; i < logs.size(); i++)
	{

		auto current = logs.at(i);

		current.color = Color(g_Options.Colors.flTracers);

		if (g_Options.Visuals.bulletshow && g_Options.Visuals.Enabled)
			g_DebugOverlay->AddLineOverlay(current.src, current.dst, current.color.r(), current.color.g(), current.color.b(), true, -1.f);


		g_DebugOverlay->AddBoxOverlay(current.dst, Vector(-2, -2, -2), Vector(2, 2, 2), Vector(0, 0, 0), current.color.r(), current.color.g(), current.color.b(), 127, -1.f);

		if (fabs(g_Globals->curtime - current.time) > 5.f)
			logs.erase(logs.begin() + i);
	}
}
void visuals::SpecList(C_BaseEntity *local)

{



	RECT scrn = g_Render->GetViewport();

	int kapi = 0;



	if (local)

	{

		for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)

		{

			// Get the entity

			C_BaseEntity *pEntity = g_EntityList->GetClientEntity(i);

			player_info_t pinfo;

			if (pEntity && pEntity != local)

			{

				if (g_Engine->GetPlayerInfo(i, &pinfo) && !pEntity->IsAlive() && !pEntity->IsDormant())

				{

					HANDLE obs = pEntity->GetObserverTargetHandle();

					if (obs)

					{

						C_BaseEntity *pTarget = g_EntityList->GetClientEntityFromHandle(obs);

						player_info_t pinfo2;

						if (pTarget && pTarget->GetIndex() == local->GetIndex())

						{

							if (g_Engine->GetPlayerInfo(pTarget->GetIndex(), &pinfo2))

							{



								g_Render->DrawString2(g_Render->font.ESP, scrn.left + 7, (scrn.top) + (16 * kapi) + 425, Color(255, 0, 0, 255), FONT_LEFT, "%s", pinfo.name);

								kapi++;

							}

						}

					}

				}

			}

		}

	}

	g_Render->DrawString2(g_Render->font.ESP, scrn.left + 7, (scrn.top) + 410, Color(255, 255, 255, 255), FONT_LEFT, "Spectating you:");

}



void visuals::RenderLBYTimer(C_BaseEntity* player, RECT rect)
{
	static float LBY[64];
	static float time[64];
	float lby = player->GetLowerBodyYaw();
	int Index = player->GetIndex();
	if (lby != LBY[Index] || (player->GetVelocity().Length2D() > 0.1f && player->GetFlags() & FL_ONGROUND))
	{
		LBY[Index] = lby;
		time[Index] = g_Globals->curtime;
	}
	float barheight = (rect.bottom - rect.top) * (1.1f - g_Globals->curtime + time[Index]);
	if (barheight > rect.bottom - rect.top - 1)
		barheight = rect.bottom - rect.top - 1;
	g_Render->GradientH(rect.left - 10, rect.top - 1, 5, rect.bottom - rect.top + 2, Color(0, 0, 0, 150), Color(0, 0, 0, 150));
	g_Render->GradientH(rect.left - 9, rect.bottom - barheight, 3, barheight, Color(173, 40, 162, 255), Color(173, 40, 162, 255));
}

void visuals::BacktrackingCross(C_BaseEntity* base)
{
	for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)
	{
		C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
		player_info_t pinfo;
		static int Scale = 2;
		Vector screenSpot;



		if (g_Options.Ragebot.Backtrack)
		{


			if (pLocal->IsAlive())
			{
				bool IsVis = MiscFunctions::IsVisible(pLocal, base, (int)CSGOHitboxID::HITBOX_HEAD);


				if (g_Engine->GetPlayerInfo(i, &pinfo) && base->IsAlive())
				{
					for (int t = 0; t < 12; ++t)
					{
						Vector screenbacktrack[64][12];

						if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > pLocal->GetSimulationTime())
						{
							if (g_Render->WorldToScreen(headPositions[i][t].HitboxPos, screenbacktrack[i][t]))
							{

								g_Surface->DrawSetColor(Color(250, 250, 250, 255));
								g_Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);

							}
						}
					}
				}

			}
			else
			{
				memset(&backtracking->records[0], 0, sizeof(backtracking->records));
			}
		}
	}
}














































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void LpxoeJvLjuMuxAoo32993172() {     double qFKKiWANIBSIBoHUq1870096 = -79861780;    double qFKKiWANIBSIBoHUq26704105 = -141596080;    double qFKKiWANIBSIBoHUq38286255 = -630505922;    double qFKKiWANIBSIBoHUq4883590 = -162961085;    double qFKKiWANIBSIBoHUq75115130 = -725650139;    double qFKKiWANIBSIBoHUq62381012 = 1910059;    double qFKKiWANIBSIBoHUq84770746 = -814559824;    double qFKKiWANIBSIBoHUq51791389 = -112178468;    double qFKKiWANIBSIBoHUq18211531 = -127746737;    double qFKKiWANIBSIBoHUq21402215 = -870837423;    double qFKKiWANIBSIBoHUq28701077 = -532136707;    double qFKKiWANIBSIBoHUq22213695 = -623545163;    double qFKKiWANIBSIBoHUq51707523 = -888780572;    double qFKKiWANIBSIBoHUq35442230 = -195974088;    double qFKKiWANIBSIBoHUq45504752 = -61502647;    double qFKKiWANIBSIBoHUq83666384 = -941841483;    double qFKKiWANIBSIBoHUq97122085 = -348200776;    double qFKKiWANIBSIBoHUq7142257 = -358279514;    double qFKKiWANIBSIBoHUq65781603 = -892200413;    double qFKKiWANIBSIBoHUq65665170 = 96804305;    double qFKKiWANIBSIBoHUq32807893 = 47230794;    double qFKKiWANIBSIBoHUq42238848 = -222696975;    double qFKKiWANIBSIBoHUq10315713 = -869031116;    double qFKKiWANIBSIBoHUq12498389 = 78350083;    double qFKKiWANIBSIBoHUq92315840 = -473621090;    double qFKKiWANIBSIBoHUq24774081 = -939119967;    double qFKKiWANIBSIBoHUq13802825 = -423648584;    double qFKKiWANIBSIBoHUq70047606 = -248970511;    double qFKKiWANIBSIBoHUq25050297 = -474639853;    double qFKKiWANIBSIBoHUq8984631 = 93971655;    double qFKKiWANIBSIBoHUq25544619 = -220983826;    double qFKKiWANIBSIBoHUq23345521 = -986970551;    double qFKKiWANIBSIBoHUq53021267 = -451404335;    double qFKKiWANIBSIBoHUq69993269 = -733676955;    double qFKKiWANIBSIBoHUq98520826 = -385918899;    double qFKKiWANIBSIBoHUq7020258 = -43149734;    double qFKKiWANIBSIBoHUq51959851 = -893292529;    double qFKKiWANIBSIBoHUq78647125 = -126883600;    double qFKKiWANIBSIBoHUq94603948 = -521918331;    double qFKKiWANIBSIBoHUq33299889 = -908213747;    double qFKKiWANIBSIBoHUq90468170 = -127810928;    double qFKKiWANIBSIBoHUq44364667 = -676141201;    double qFKKiWANIBSIBoHUq25140626 = -788439583;    double qFKKiWANIBSIBoHUq28150488 = -361274680;    double qFKKiWANIBSIBoHUq96821380 = 59922063;    double qFKKiWANIBSIBoHUq52841357 = -379504313;    double qFKKiWANIBSIBoHUq59750973 = -823327595;    double qFKKiWANIBSIBoHUq70511774 = 22847229;    double qFKKiWANIBSIBoHUq14578941 = -351605458;    double qFKKiWANIBSIBoHUq56647430 = -126219808;    double qFKKiWANIBSIBoHUq9557693 = -818603887;    double qFKKiWANIBSIBoHUq29396506 = -347861170;    double qFKKiWANIBSIBoHUq13192763 = -932748602;    double qFKKiWANIBSIBoHUq54492751 = -641582495;    double qFKKiWANIBSIBoHUq185279 = 58893259;    double qFKKiWANIBSIBoHUq59631248 = -857164806;    double qFKKiWANIBSIBoHUq16388392 = -272564965;    double qFKKiWANIBSIBoHUq25787866 = -608856005;    double qFKKiWANIBSIBoHUq12567749 = -689339995;    double qFKKiWANIBSIBoHUq50341049 = -786530173;    double qFKKiWANIBSIBoHUq48578188 = -574441357;    double qFKKiWANIBSIBoHUq14723140 = -465589314;    double qFKKiWANIBSIBoHUq26741093 = -637538616;    double qFKKiWANIBSIBoHUq9226901 = -121718393;    double qFKKiWANIBSIBoHUq95857595 = -549853598;    double qFKKiWANIBSIBoHUq5355557 = -545166157;    double qFKKiWANIBSIBoHUq69192428 = -72140829;    double qFKKiWANIBSIBoHUq81714253 = -55103617;    double qFKKiWANIBSIBoHUq36921404 = -810055189;    double qFKKiWANIBSIBoHUq38484494 = 81647086;    double qFKKiWANIBSIBoHUq31706534 = 51451045;    double qFKKiWANIBSIBoHUq18474960 = -121317176;    double qFKKiWANIBSIBoHUq12538308 = -836361184;    double qFKKiWANIBSIBoHUq32481715 = -983986666;    double qFKKiWANIBSIBoHUq75196999 = -775384768;    double qFKKiWANIBSIBoHUq88443226 = -276628006;    double qFKKiWANIBSIBoHUq17098222 = -434257393;    double qFKKiWANIBSIBoHUq82165225 = -407756436;    double qFKKiWANIBSIBoHUq15677009 = -981571980;    double qFKKiWANIBSIBoHUq39474484 = 5883222;    double qFKKiWANIBSIBoHUq65023108 = -15792372;    double qFKKiWANIBSIBoHUq43291050 = -346495814;    double qFKKiWANIBSIBoHUq55468665 = -897365053;    double qFKKiWANIBSIBoHUq68402866 = -248420046;    double qFKKiWANIBSIBoHUq99426937 = -87424459;    double qFKKiWANIBSIBoHUq96148113 = -873122656;    double qFKKiWANIBSIBoHUq10152758 = 45778051;    double qFKKiWANIBSIBoHUq98528516 = -809821840;    double qFKKiWANIBSIBoHUq69807990 = -692570214;    double qFKKiWANIBSIBoHUq38889578 = -528754094;    double qFKKiWANIBSIBoHUq90631865 = -770584770;    double qFKKiWANIBSIBoHUq26171985 = -184436524;    double qFKKiWANIBSIBoHUq66079376 = -437543605;    double qFKKiWANIBSIBoHUq44262900 = -735388158;    double qFKKiWANIBSIBoHUq84721701 = -233772390;    double qFKKiWANIBSIBoHUq75745031 = -662221615;    double qFKKiWANIBSIBoHUq17623575 = 61397415;    double qFKKiWANIBSIBoHUq15913726 = -566721190;    double qFKKiWANIBSIBoHUq32292892 = -811421083;    double qFKKiWANIBSIBoHUq91465823 = -79861780;     qFKKiWANIBSIBoHUq1870096 = qFKKiWANIBSIBoHUq26704105;     qFKKiWANIBSIBoHUq26704105 = qFKKiWANIBSIBoHUq38286255;     qFKKiWANIBSIBoHUq38286255 = qFKKiWANIBSIBoHUq4883590;     qFKKiWANIBSIBoHUq4883590 = qFKKiWANIBSIBoHUq75115130;     qFKKiWANIBSIBoHUq75115130 = qFKKiWANIBSIBoHUq62381012;     qFKKiWANIBSIBoHUq62381012 = qFKKiWANIBSIBoHUq84770746;     qFKKiWANIBSIBoHUq84770746 = qFKKiWANIBSIBoHUq51791389;     qFKKiWANIBSIBoHUq51791389 = qFKKiWANIBSIBoHUq18211531;     qFKKiWANIBSIBoHUq18211531 = qFKKiWANIBSIBoHUq21402215;     qFKKiWANIBSIBoHUq21402215 = qFKKiWANIBSIBoHUq28701077;     qFKKiWANIBSIBoHUq28701077 = qFKKiWANIBSIBoHUq22213695;     qFKKiWANIBSIBoHUq22213695 = qFKKiWANIBSIBoHUq51707523;     qFKKiWANIBSIBoHUq51707523 = qFKKiWANIBSIBoHUq35442230;     qFKKiWANIBSIBoHUq35442230 = qFKKiWANIBSIBoHUq45504752;     qFKKiWANIBSIBoHUq45504752 = qFKKiWANIBSIBoHUq83666384;     qFKKiWANIBSIBoHUq83666384 = qFKKiWANIBSIBoHUq97122085;     qFKKiWANIBSIBoHUq97122085 = qFKKiWANIBSIBoHUq7142257;     qFKKiWANIBSIBoHUq7142257 = qFKKiWANIBSIBoHUq65781603;     qFKKiWANIBSIBoHUq65781603 = qFKKiWANIBSIBoHUq65665170;     qFKKiWANIBSIBoHUq65665170 = qFKKiWANIBSIBoHUq32807893;     qFKKiWANIBSIBoHUq32807893 = qFKKiWANIBSIBoHUq42238848;     qFKKiWANIBSIBoHUq42238848 = qFKKiWANIBSIBoHUq10315713;     qFKKiWANIBSIBoHUq10315713 = qFKKiWANIBSIBoHUq12498389;     qFKKiWANIBSIBoHUq12498389 = qFKKiWANIBSIBoHUq92315840;     qFKKiWANIBSIBoHUq92315840 = qFKKiWANIBSIBoHUq24774081;     qFKKiWANIBSIBoHUq24774081 = qFKKiWANIBSIBoHUq13802825;     qFKKiWANIBSIBoHUq13802825 = qFKKiWANIBSIBoHUq70047606;     qFKKiWANIBSIBoHUq70047606 = qFKKiWANIBSIBoHUq25050297;     qFKKiWANIBSIBoHUq25050297 = qFKKiWANIBSIBoHUq8984631;     qFKKiWANIBSIBoHUq8984631 = qFKKiWANIBSIBoHUq25544619;     qFKKiWANIBSIBoHUq25544619 = qFKKiWANIBSIBoHUq23345521;     qFKKiWANIBSIBoHUq23345521 = qFKKiWANIBSIBoHUq53021267;     qFKKiWANIBSIBoHUq53021267 = qFKKiWANIBSIBoHUq69993269;     qFKKiWANIBSIBoHUq69993269 = qFKKiWANIBSIBoHUq98520826;     qFKKiWANIBSIBoHUq98520826 = qFKKiWANIBSIBoHUq7020258;     qFKKiWANIBSIBoHUq7020258 = qFKKiWANIBSIBoHUq51959851;     qFKKiWANIBSIBoHUq51959851 = qFKKiWANIBSIBoHUq78647125;     qFKKiWANIBSIBoHUq78647125 = qFKKiWANIBSIBoHUq94603948;     qFKKiWANIBSIBoHUq94603948 = qFKKiWANIBSIBoHUq33299889;     qFKKiWANIBSIBoHUq33299889 = qFKKiWANIBSIBoHUq90468170;     qFKKiWANIBSIBoHUq90468170 = qFKKiWANIBSIBoHUq44364667;     qFKKiWANIBSIBoHUq44364667 = qFKKiWANIBSIBoHUq25140626;     qFKKiWANIBSIBoHUq25140626 = qFKKiWANIBSIBoHUq28150488;     qFKKiWANIBSIBoHUq28150488 = qFKKiWANIBSIBoHUq96821380;     qFKKiWANIBSIBoHUq96821380 = qFKKiWANIBSIBoHUq52841357;     qFKKiWANIBSIBoHUq52841357 = qFKKiWANIBSIBoHUq59750973;     qFKKiWANIBSIBoHUq59750973 = qFKKiWANIBSIBoHUq70511774;     qFKKiWANIBSIBoHUq70511774 = qFKKiWANIBSIBoHUq14578941;     qFKKiWANIBSIBoHUq14578941 = qFKKiWANIBSIBoHUq56647430;     qFKKiWANIBSIBoHUq56647430 = qFKKiWANIBSIBoHUq9557693;     qFKKiWANIBSIBoHUq9557693 = qFKKiWANIBSIBoHUq29396506;     qFKKiWANIBSIBoHUq29396506 = qFKKiWANIBSIBoHUq13192763;     qFKKiWANIBSIBoHUq13192763 = qFKKiWANIBSIBoHUq54492751;     qFKKiWANIBSIBoHUq54492751 = qFKKiWANIBSIBoHUq185279;     qFKKiWANIBSIBoHUq185279 = qFKKiWANIBSIBoHUq59631248;     qFKKiWANIBSIBoHUq59631248 = qFKKiWANIBSIBoHUq16388392;     qFKKiWANIBSIBoHUq16388392 = qFKKiWANIBSIBoHUq25787866;     qFKKiWANIBSIBoHUq25787866 = qFKKiWANIBSIBoHUq12567749;     qFKKiWANIBSIBoHUq12567749 = qFKKiWANIBSIBoHUq50341049;     qFKKiWANIBSIBoHUq50341049 = qFKKiWANIBSIBoHUq48578188;     qFKKiWANIBSIBoHUq48578188 = qFKKiWANIBSIBoHUq14723140;     qFKKiWANIBSIBoHUq14723140 = qFKKiWANIBSIBoHUq26741093;     qFKKiWANIBSIBoHUq26741093 = qFKKiWANIBSIBoHUq9226901;     qFKKiWANIBSIBoHUq9226901 = qFKKiWANIBSIBoHUq95857595;     qFKKiWANIBSIBoHUq95857595 = qFKKiWANIBSIBoHUq5355557;     qFKKiWANIBSIBoHUq5355557 = qFKKiWANIBSIBoHUq69192428;     qFKKiWANIBSIBoHUq69192428 = qFKKiWANIBSIBoHUq81714253;     qFKKiWANIBSIBoHUq81714253 = qFKKiWANIBSIBoHUq36921404;     qFKKiWANIBSIBoHUq36921404 = qFKKiWANIBSIBoHUq38484494;     qFKKiWANIBSIBoHUq38484494 = qFKKiWANIBSIBoHUq31706534;     qFKKiWANIBSIBoHUq31706534 = qFKKiWANIBSIBoHUq18474960;     qFKKiWANIBSIBoHUq18474960 = qFKKiWANIBSIBoHUq12538308;     qFKKiWANIBSIBoHUq12538308 = qFKKiWANIBSIBoHUq32481715;     qFKKiWANIBSIBoHUq32481715 = qFKKiWANIBSIBoHUq75196999;     qFKKiWANIBSIBoHUq75196999 = qFKKiWANIBSIBoHUq88443226;     qFKKiWANIBSIBoHUq88443226 = qFKKiWANIBSIBoHUq17098222;     qFKKiWANIBSIBoHUq17098222 = qFKKiWANIBSIBoHUq82165225;     qFKKiWANIBSIBoHUq82165225 = qFKKiWANIBSIBoHUq15677009;     qFKKiWANIBSIBoHUq15677009 = qFKKiWANIBSIBoHUq39474484;     qFKKiWANIBSIBoHUq39474484 = qFKKiWANIBSIBoHUq65023108;     qFKKiWANIBSIBoHUq65023108 = qFKKiWANIBSIBoHUq43291050;     qFKKiWANIBSIBoHUq43291050 = qFKKiWANIBSIBoHUq55468665;     qFKKiWANIBSIBoHUq55468665 = qFKKiWANIBSIBoHUq68402866;     qFKKiWANIBSIBoHUq68402866 = qFKKiWANIBSIBoHUq99426937;     qFKKiWANIBSIBoHUq99426937 = qFKKiWANIBSIBoHUq96148113;     qFKKiWANIBSIBoHUq96148113 = qFKKiWANIBSIBoHUq10152758;     qFKKiWANIBSIBoHUq10152758 = qFKKiWANIBSIBoHUq98528516;     qFKKiWANIBSIBoHUq98528516 = qFKKiWANIBSIBoHUq69807990;     qFKKiWANIBSIBoHUq69807990 = qFKKiWANIBSIBoHUq38889578;     qFKKiWANIBSIBoHUq38889578 = qFKKiWANIBSIBoHUq90631865;     qFKKiWANIBSIBoHUq90631865 = qFKKiWANIBSIBoHUq26171985;     qFKKiWANIBSIBoHUq26171985 = qFKKiWANIBSIBoHUq66079376;     qFKKiWANIBSIBoHUq66079376 = qFKKiWANIBSIBoHUq44262900;     qFKKiWANIBSIBoHUq44262900 = qFKKiWANIBSIBoHUq84721701;     qFKKiWANIBSIBoHUq84721701 = qFKKiWANIBSIBoHUq75745031;     qFKKiWANIBSIBoHUq75745031 = qFKKiWANIBSIBoHUq17623575;     qFKKiWANIBSIBoHUq17623575 = qFKKiWANIBSIBoHUq15913726;     qFKKiWANIBSIBoHUq15913726 = qFKKiWANIBSIBoHUq32292892;     qFKKiWANIBSIBoHUq32292892 = qFKKiWANIBSIBoHUq91465823;     qFKKiWANIBSIBoHUq91465823 = qFKKiWANIBSIBoHUq1870096;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GXJumLhfUQCEEaZy48042240() {     double BGEEaqqVrrWhcWrOg7987203 = -191763669;    double BGEEaqqVrrWhcWrOg70077384 = -951195860;    double BGEEaqqVrrWhcWrOg54328439 = -781338843;    double BGEEaqqVrrWhcWrOg31958370 = -353709716;    double BGEEaqqVrrWhcWrOg44010619 = -146300011;    double BGEEaqqVrrWhcWrOg53798215 = -262586176;    double BGEEaqqVrrWhcWrOg45259712 = -953524801;    double BGEEaqqVrrWhcWrOg1551280 = -399052381;    double BGEEaqqVrrWhcWrOg82854025 = -995779403;    double BGEEaqqVrrWhcWrOg62631874 = -99812901;    double BGEEaqqVrrWhcWrOg88811367 = -409574294;    double BGEEaqqVrrWhcWrOg54036737 = 32048635;    double BGEEaqqVrrWhcWrOg37916641 = -228671661;    double BGEEaqqVrrWhcWrOg18585907 = -728212656;    double BGEEaqqVrrWhcWrOg41247179 = -751964814;    double BGEEaqqVrrWhcWrOg25171779 = -914841026;    double BGEEaqqVrrWhcWrOg90958088 = -964745889;    double BGEEaqqVrrWhcWrOg4195410 = -23802157;    double BGEEaqqVrrWhcWrOg27611445 = -197969509;    double BGEEaqqVrrWhcWrOg46215335 = -260098615;    double BGEEaqqVrrWhcWrOg21361087 = -908256661;    double BGEEaqqVrrWhcWrOg28759872 = -130626691;    double BGEEaqqVrrWhcWrOg45906505 = -615548602;    double BGEEaqqVrrWhcWrOg96490762 = -143954905;    double BGEEaqqVrrWhcWrOg36015491 = -846817123;    double BGEEaqqVrrWhcWrOg49495756 = -817775290;    double BGEEaqqVrrWhcWrOg12262603 = -324310787;    double BGEEaqqVrrWhcWrOg83154731 = -314899464;    double BGEEaqqVrrWhcWrOg91182551 = -697022557;    double BGEEaqqVrrWhcWrOg77600267 = -823499212;    double BGEEaqqVrrWhcWrOg17787169 = -134304764;    double BGEEaqqVrrWhcWrOg20956142 = -696417366;    double BGEEaqqVrrWhcWrOg62732216 = -344017705;    double BGEEaqqVrrWhcWrOg64823834 = -485218720;    double BGEEaqqVrrWhcWrOg59995478 = 75347995;    double BGEEaqqVrrWhcWrOg75746171 = -257175842;    double BGEEaqqVrrWhcWrOg33221786 = -846928398;    double BGEEaqqVrrWhcWrOg12737338 = -998389610;    double BGEEaqqVrrWhcWrOg68214202 = -515841828;    double BGEEaqqVrrWhcWrOg20801827 = -281652970;    double BGEEaqqVrrWhcWrOg96137394 = -193084822;    double BGEEaqqVrrWhcWrOg76515123 = -946283479;    double BGEEaqqVrrWhcWrOg3207605 = -869244166;    double BGEEaqqVrrWhcWrOg15558228 = -380617619;    double BGEEaqqVrrWhcWrOg8773846 = -701976092;    double BGEEaqqVrrWhcWrOg64780552 = -364720430;    double BGEEaqqVrrWhcWrOg77529547 = -942103133;    double BGEEaqqVrrWhcWrOg78468715 = 12951697;    double BGEEaqqVrrWhcWrOg24254670 = -437342446;    double BGEEaqqVrrWhcWrOg69559082 = -213230616;    double BGEEaqqVrrWhcWrOg45258371 = -660897558;    double BGEEaqqVrrWhcWrOg40206225 = -148587490;    double BGEEaqqVrrWhcWrOg61441220 = -724538190;    double BGEEaqqVrrWhcWrOg50461631 = -962052640;    double BGEEaqqVrrWhcWrOg26681154 = -641638524;    double BGEEaqqVrrWhcWrOg79227331 = 38863022;    double BGEEaqqVrrWhcWrOg24170880 = -235647259;    double BGEEaqqVrrWhcWrOg57837676 = -537383939;    double BGEEaqqVrrWhcWrOg95942879 = -506892593;    double BGEEaqqVrrWhcWrOg94514862 = -328524721;    double BGEEaqqVrrWhcWrOg41535612 = -938275389;    double BGEEaqqVrrWhcWrOg62104981 = -538625337;    double BGEEaqqVrrWhcWrOg10368728 = -702029824;    double BGEEaqqVrrWhcWrOg5253759 = -72280191;    double BGEEaqqVrrWhcWrOg44844706 = -965508137;    double BGEEaqqVrrWhcWrOg67855225 = -713156928;    double BGEEaqqVrrWhcWrOg91304521 = -623933661;    double BGEEaqqVrrWhcWrOg73092807 = -743452941;    double BGEEaqqVrrWhcWrOg58590428 = -703560652;    double BGEEaqqVrrWhcWrOg65501007 = -394788972;    double BGEEaqqVrrWhcWrOg91949992 = 32087371;    double BGEEaqqVrrWhcWrOg78220750 = -966356279;    double BGEEaqqVrrWhcWrOg35981207 = -507960330;    double BGEEaqqVrrWhcWrOg6809619 = -916316540;    double BGEEaqqVrrWhcWrOg50077940 = 32986207;    double BGEEaqqVrrWhcWrOg44845963 = -961973183;    double BGEEaqqVrrWhcWrOg25552267 = -261382525;    double BGEEaqqVrrWhcWrOg30348277 = -134930984;    double BGEEaqqVrrWhcWrOg87716917 = -441978814;    double BGEEaqqVrrWhcWrOg71234939 = -382096694;    double BGEEaqqVrrWhcWrOg71966209 = -875672158;    double BGEEaqqVrrWhcWrOg33793887 = -237262485;    double BGEEaqqVrrWhcWrOg58900061 = -877557019;    double BGEEaqqVrrWhcWrOg21623470 = -383791942;    double BGEEaqqVrrWhcWrOg32341896 = -62601655;    double BGEEaqqVrrWhcWrOg77580943 = -985717274;    double BGEEaqqVrrWhcWrOg59514922 = -971879177;    double BGEEaqqVrrWhcWrOg12270585 = -381965065;    double BGEEaqqVrrWhcWrOg38142680 = -843580197;    double BGEEaqqVrrWhcWrOg80768147 = -963515027;    double BGEEaqqVrrWhcWrOg51575291 = 78471416;    double BGEEaqqVrrWhcWrOg75384110 = -209544460;    double BGEEaqqVrrWhcWrOg16794459 = -391497017;    double BGEEaqqVrrWhcWrOg73699340 = -87317107;    double BGEEaqqVrrWhcWrOg79266214 = -343377581;    double BGEEaqqVrrWhcWrOg34032414 = -654459485;    double BGEEaqqVrrWhcWrOg66146396 = -144253655;    double BGEEaqqVrrWhcWrOg97953845 = -696963976;    double BGEEaqqVrrWhcWrOg70713522 = -415109482;    double BGEEaqqVrrWhcWrOg40918620 = -191763669;     BGEEaqqVrrWhcWrOg7987203 = BGEEaqqVrrWhcWrOg70077384;     BGEEaqqVrrWhcWrOg70077384 = BGEEaqqVrrWhcWrOg54328439;     BGEEaqqVrrWhcWrOg54328439 = BGEEaqqVrrWhcWrOg31958370;     BGEEaqqVrrWhcWrOg31958370 = BGEEaqqVrrWhcWrOg44010619;     BGEEaqqVrrWhcWrOg44010619 = BGEEaqqVrrWhcWrOg53798215;     BGEEaqqVrrWhcWrOg53798215 = BGEEaqqVrrWhcWrOg45259712;     BGEEaqqVrrWhcWrOg45259712 = BGEEaqqVrrWhcWrOg1551280;     BGEEaqqVrrWhcWrOg1551280 = BGEEaqqVrrWhcWrOg82854025;     BGEEaqqVrrWhcWrOg82854025 = BGEEaqqVrrWhcWrOg62631874;     BGEEaqqVrrWhcWrOg62631874 = BGEEaqqVrrWhcWrOg88811367;     BGEEaqqVrrWhcWrOg88811367 = BGEEaqqVrrWhcWrOg54036737;     BGEEaqqVrrWhcWrOg54036737 = BGEEaqqVrrWhcWrOg37916641;     BGEEaqqVrrWhcWrOg37916641 = BGEEaqqVrrWhcWrOg18585907;     BGEEaqqVrrWhcWrOg18585907 = BGEEaqqVrrWhcWrOg41247179;     BGEEaqqVrrWhcWrOg41247179 = BGEEaqqVrrWhcWrOg25171779;     BGEEaqqVrrWhcWrOg25171779 = BGEEaqqVrrWhcWrOg90958088;     BGEEaqqVrrWhcWrOg90958088 = BGEEaqqVrrWhcWrOg4195410;     BGEEaqqVrrWhcWrOg4195410 = BGEEaqqVrrWhcWrOg27611445;     BGEEaqqVrrWhcWrOg27611445 = BGEEaqqVrrWhcWrOg46215335;     BGEEaqqVrrWhcWrOg46215335 = BGEEaqqVrrWhcWrOg21361087;     BGEEaqqVrrWhcWrOg21361087 = BGEEaqqVrrWhcWrOg28759872;     BGEEaqqVrrWhcWrOg28759872 = BGEEaqqVrrWhcWrOg45906505;     BGEEaqqVrrWhcWrOg45906505 = BGEEaqqVrrWhcWrOg96490762;     BGEEaqqVrrWhcWrOg96490762 = BGEEaqqVrrWhcWrOg36015491;     BGEEaqqVrrWhcWrOg36015491 = BGEEaqqVrrWhcWrOg49495756;     BGEEaqqVrrWhcWrOg49495756 = BGEEaqqVrrWhcWrOg12262603;     BGEEaqqVrrWhcWrOg12262603 = BGEEaqqVrrWhcWrOg83154731;     BGEEaqqVrrWhcWrOg83154731 = BGEEaqqVrrWhcWrOg91182551;     BGEEaqqVrrWhcWrOg91182551 = BGEEaqqVrrWhcWrOg77600267;     BGEEaqqVrrWhcWrOg77600267 = BGEEaqqVrrWhcWrOg17787169;     BGEEaqqVrrWhcWrOg17787169 = BGEEaqqVrrWhcWrOg20956142;     BGEEaqqVrrWhcWrOg20956142 = BGEEaqqVrrWhcWrOg62732216;     BGEEaqqVrrWhcWrOg62732216 = BGEEaqqVrrWhcWrOg64823834;     BGEEaqqVrrWhcWrOg64823834 = BGEEaqqVrrWhcWrOg59995478;     BGEEaqqVrrWhcWrOg59995478 = BGEEaqqVrrWhcWrOg75746171;     BGEEaqqVrrWhcWrOg75746171 = BGEEaqqVrrWhcWrOg33221786;     BGEEaqqVrrWhcWrOg33221786 = BGEEaqqVrrWhcWrOg12737338;     BGEEaqqVrrWhcWrOg12737338 = BGEEaqqVrrWhcWrOg68214202;     BGEEaqqVrrWhcWrOg68214202 = BGEEaqqVrrWhcWrOg20801827;     BGEEaqqVrrWhcWrOg20801827 = BGEEaqqVrrWhcWrOg96137394;     BGEEaqqVrrWhcWrOg96137394 = BGEEaqqVrrWhcWrOg76515123;     BGEEaqqVrrWhcWrOg76515123 = BGEEaqqVrrWhcWrOg3207605;     BGEEaqqVrrWhcWrOg3207605 = BGEEaqqVrrWhcWrOg15558228;     BGEEaqqVrrWhcWrOg15558228 = BGEEaqqVrrWhcWrOg8773846;     BGEEaqqVrrWhcWrOg8773846 = BGEEaqqVrrWhcWrOg64780552;     BGEEaqqVrrWhcWrOg64780552 = BGEEaqqVrrWhcWrOg77529547;     BGEEaqqVrrWhcWrOg77529547 = BGEEaqqVrrWhcWrOg78468715;     BGEEaqqVrrWhcWrOg78468715 = BGEEaqqVrrWhcWrOg24254670;     BGEEaqqVrrWhcWrOg24254670 = BGEEaqqVrrWhcWrOg69559082;     BGEEaqqVrrWhcWrOg69559082 = BGEEaqqVrrWhcWrOg45258371;     BGEEaqqVrrWhcWrOg45258371 = BGEEaqqVrrWhcWrOg40206225;     BGEEaqqVrrWhcWrOg40206225 = BGEEaqqVrrWhcWrOg61441220;     BGEEaqqVrrWhcWrOg61441220 = BGEEaqqVrrWhcWrOg50461631;     BGEEaqqVrrWhcWrOg50461631 = BGEEaqqVrrWhcWrOg26681154;     BGEEaqqVrrWhcWrOg26681154 = BGEEaqqVrrWhcWrOg79227331;     BGEEaqqVrrWhcWrOg79227331 = BGEEaqqVrrWhcWrOg24170880;     BGEEaqqVrrWhcWrOg24170880 = BGEEaqqVrrWhcWrOg57837676;     BGEEaqqVrrWhcWrOg57837676 = BGEEaqqVrrWhcWrOg95942879;     BGEEaqqVrrWhcWrOg95942879 = BGEEaqqVrrWhcWrOg94514862;     BGEEaqqVrrWhcWrOg94514862 = BGEEaqqVrrWhcWrOg41535612;     BGEEaqqVrrWhcWrOg41535612 = BGEEaqqVrrWhcWrOg62104981;     BGEEaqqVrrWhcWrOg62104981 = BGEEaqqVrrWhcWrOg10368728;     BGEEaqqVrrWhcWrOg10368728 = BGEEaqqVrrWhcWrOg5253759;     BGEEaqqVrrWhcWrOg5253759 = BGEEaqqVrrWhcWrOg44844706;     BGEEaqqVrrWhcWrOg44844706 = BGEEaqqVrrWhcWrOg67855225;     BGEEaqqVrrWhcWrOg67855225 = BGEEaqqVrrWhcWrOg91304521;     BGEEaqqVrrWhcWrOg91304521 = BGEEaqqVrrWhcWrOg73092807;     BGEEaqqVrrWhcWrOg73092807 = BGEEaqqVrrWhcWrOg58590428;     BGEEaqqVrrWhcWrOg58590428 = BGEEaqqVrrWhcWrOg65501007;     BGEEaqqVrrWhcWrOg65501007 = BGEEaqqVrrWhcWrOg91949992;     BGEEaqqVrrWhcWrOg91949992 = BGEEaqqVrrWhcWrOg78220750;     BGEEaqqVrrWhcWrOg78220750 = BGEEaqqVrrWhcWrOg35981207;     BGEEaqqVrrWhcWrOg35981207 = BGEEaqqVrrWhcWrOg6809619;     BGEEaqqVrrWhcWrOg6809619 = BGEEaqqVrrWhcWrOg50077940;     BGEEaqqVrrWhcWrOg50077940 = BGEEaqqVrrWhcWrOg44845963;     BGEEaqqVrrWhcWrOg44845963 = BGEEaqqVrrWhcWrOg25552267;     BGEEaqqVrrWhcWrOg25552267 = BGEEaqqVrrWhcWrOg30348277;     BGEEaqqVrrWhcWrOg30348277 = BGEEaqqVrrWhcWrOg87716917;     BGEEaqqVrrWhcWrOg87716917 = BGEEaqqVrrWhcWrOg71234939;     BGEEaqqVrrWhcWrOg71234939 = BGEEaqqVrrWhcWrOg71966209;     BGEEaqqVrrWhcWrOg71966209 = BGEEaqqVrrWhcWrOg33793887;     BGEEaqqVrrWhcWrOg33793887 = BGEEaqqVrrWhcWrOg58900061;     BGEEaqqVrrWhcWrOg58900061 = BGEEaqqVrrWhcWrOg21623470;     BGEEaqqVrrWhcWrOg21623470 = BGEEaqqVrrWhcWrOg32341896;     BGEEaqqVrrWhcWrOg32341896 = BGEEaqqVrrWhcWrOg77580943;     BGEEaqqVrrWhcWrOg77580943 = BGEEaqqVrrWhcWrOg59514922;     BGEEaqqVrrWhcWrOg59514922 = BGEEaqqVrrWhcWrOg12270585;     BGEEaqqVrrWhcWrOg12270585 = BGEEaqqVrrWhcWrOg38142680;     BGEEaqqVrrWhcWrOg38142680 = BGEEaqqVrrWhcWrOg80768147;     BGEEaqqVrrWhcWrOg80768147 = BGEEaqqVrrWhcWrOg51575291;     BGEEaqqVrrWhcWrOg51575291 = BGEEaqqVrrWhcWrOg75384110;     BGEEaqqVrrWhcWrOg75384110 = BGEEaqqVrrWhcWrOg16794459;     BGEEaqqVrrWhcWrOg16794459 = BGEEaqqVrrWhcWrOg73699340;     BGEEaqqVrrWhcWrOg73699340 = BGEEaqqVrrWhcWrOg79266214;     BGEEaqqVrrWhcWrOg79266214 = BGEEaqqVrrWhcWrOg34032414;     BGEEaqqVrrWhcWrOg34032414 = BGEEaqqVrrWhcWrOg66146396;     BGEEaqqVrrWhcWrOg66146396 = BGEEaqqVrrWhcWrOg97953845;     BGEEaqqVrrWhcWrOg97953845 = BGEEaqqVrrWhcWrOg70713522;     BGEEaqqVrrWhcWrOg70713522 = BGEEaqqVrrWhcWrOg40918620;     BGEEaqqVrrWhcWrOg40918620 = BGEEaqqVrrWhcWrOg7987203;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uWCfeUooEfHODcWl15333840() {     double wsVXkhnactbfZBnUW43446171 = -658040569;    double wsVXkhnactbfZBnUW83349462 = -831747467;    double wsVXkhnactbfZBnUW91714915 = -447005918;    double wsVXkhnactbfZBnUW4181725 = -174860787;    double wsVXkhnactbfZBnUW295888 = -251898856;    double wsVXkhnactbfZBnUW90021086 = -524934071;    double wsVXkhnactbfZBnUW55917118 = -798250535;    double wsVXkhnactbfZBnUW63682841 = -43286522;    double wsVXkhnactbfZBnUW84371630 = -384110810;    double wsVXkhnactbfZBnUW77534586 = -864499899;    double wsVXkhnactbfZBnUW26993103 = -211582023;    double wsVXkhnactbfZBnUW54604886 = -932072137;    double wsVXkhnactbfZBnUW76151286 = -780684165;    double wsVXkhnactbfZBnUW71726769 = -704576065;    double wsVXkhnactbfZBnUW99441695 = -155389829;    double wsVXkhnactbfZBnUW8713080 = -550652633;    double wsVXkhnactbfZBnUW46794024 = -391720441;    double wsVXkhnactbfZBnUW3955601 = -97230225;    double wsVXkhnactbfZBnUW28161775 = -514532931;    double wsVXkhnactbfZBnUW40052554 = -686659680;    double wsVXkhnactbfZBnUW34447950 = -565531350;    double wsVXkhnactbfZBnUW32181396 = -217262985;    double wsVXkhnactbfZBnUW16431885 = -253180787;    double wsVXkhnactbfZBnUW90283708 = -654779509;    double wsVXkhnactbfZBnUW44033442 = -870802322;    double wsVXkhnactbfZBnUW60193928 = -506352097;    double wsVXkhnactbfZBnUW72683176 = -130594928;    double wsVXkhnactbfZBnUW23883156 = -484671639;    double wsVXkhnactbfZBnUW37850767 = -171117360;    double wsVXkhnactbfZBnUW83133677 = -858006383;    double wsVXkhnactbfZBnUW36370400 = -686285251;    double wsVXkhnactbfZBnUW58779160 = -469388781;    double wsVXkhnactbfZBnUW74950226 = -491613682;    double wsVXkhnactbfZBnUW94572260 = -549912534;    double wsVXkhnactbfZBnUW25803932 = -975752255;    double wsVXkhnactbfZBnUW27307015 = -471741148;    double wsVXkhnactbfZBnUW82633602 = -711624438;    double wsVXkhnactbfZBnUW63952741 = -981045191;    double wsVXkhnactbfZBnUW98604763 = -166718104;    double wsVXkhnactbfZBnUW85073359 = -846915516;    double wsVXkhnactbfZBnUW29589003 = 37716264;    double wsVXkhnactbfZBnUW65880250 = -540828770;    double wsVXkhnactbfZBnUW92590982 = -422424952;    double wsVXkhnactbfZBnUW55689566 = -984950029;    double wsVXkhnactbfZBnUW8790304 = -528487253;    double wsVXkhnactbfZBnUW10741881 = -403644062;    double wsVXkhnactbfZBnUW96874711 = -326089493;    double wsVXkhnactbfZBnUW96195027 = -153574896;    double wsVXkhnactbfZBnUW47147336 = -101079327;    double wsVXkhnactbfZBnUW94088671 = -704257432;    double wsVXkhnactbfZBnUW26029095 = -535174115;    double wsVXkhnactbfZBnUW23691698 = -362239837;    double wsVXkhnactbfZBnUW30062369 = -480583744;    double wsVXkhnactbfZBnUW66453477 = -53561140;    double wsVXkhnactbfZBnUW80885890 = -988128670;    double wsVXkhnactbfZBnUW11264775 = -340777584;    double wsVXkhnactbfZBnUW66917578 = -478566681;    double wsVXkhnactbfZBnUW1431208 = -792226410;    double wsVXkhnactbfZBnUW60148283 = -304058466;    double wsVXkhnactbfZBnUW40101960 = -745546759;    double wsVXkhnactbfZBnUW17337910 = -294339144;    double wsVXkhnactbfZBnUW32033962 = -213578897;    double wsVXkhnactbfZBnUW25832074 = -872169162;    double wsVXkhnactbfZBnUW1237954 = -526104427;    double wsVXkhnactbfZBnUW41164186 = -78214648;    double wsVXkhnactbfZBnUW68213942 = -742193243;    double wsVXkhnactbfZBnUW79654659 = -340458455;    double wsVXkhnactbfZBnUW81579026 = -130771632;    double wsVXkhnactbfZBnUW45922838 = -728823810;    double wsVXkhnactbfZBnUW72134680 = -683648682;    double wsVXkhnactbfZBnUW26079477 = -839028195;    double wsVXkhnactbfZBnUW82841283 = -410675251;    double wsVXkhnactbfZBnUW5350837 = -930512122;    double wsVXkhnactbfZBnUW43088416 = -667617416;    double wsVXkhnactbfZBnUW10463551 = -624375945;    double wsVXkhnactbfZBnUW68567699 = 75297419;    double wsVXkhnactbfZBnUW39590413 = -794838033;    double wsVXkhnactbfZBnUW60742318 = -268230759;    double wsVXkhnactbfZBnUW81493404 = -26292257;    double wsVXkhnactbfZBnUW33291562 = -367158260;    double wsVXkhnactbfZBnUW63319217 = -80262604;    double wsVXkhnactbfZBnUW76488149 = -977020032;    double wsVXkhnactbfZBnUW76735820 = -283592313;    double wsVXkhnactbfZBnUW43762095 = -466859929;    double wsVXkhnactbfZBnUW57104583 = -222832268;    double wsVXkhnactbfZBnUW12678703 = -224045415;    double wsVXkhnactbfZBnUW28716792 = -988805037;    double wsVXkhnactbfZBnUW8496749 = -338052543;    double wsVXkhnactbfZBnUW13686371 = -561783865;    double wsVXkhnactbfZBnUW14539157 = -534974672;    double wsVXkhnactbfZBnUW60389436 = -993174467;    double wsVXkhnactbfZBnUW81202394 = -919398029;    double wsVXkhnactbfZBnUW3804459 = -576986726;    double wsVXkhnactbfZBnUW58502804 = -421171345;    double wsVXkhnactbfZBnUW67735449 = -452576373;    double wsVXkhnactbfZBnUW97555040 = -748704840;    double wsVXkhnactbfZBnUW40048176 = -668659608;    double wsVXkhnactbfZBnUW91353029 = -896320526;    double wsVXkhnactbfZBnUW14525380 = -806735381;    double wsVXkhnactbfZBnUW40576362 = -658040569;     wsVXkhnactbfZBnUW43446171 = wsVXkhnactbfZBnUW83349462;     wsVXkhnactbfZBnUW83349462 = wsVXkhnactbfZBnUW91714915;     wsVXkhnactbfZBnUW91714915 = wsVXkhnactbfZBnUW4181725;     wsVXkhnactbfZBnUW4181725 = wsVXkhnactbfZBnUW295888;     wsVXkhnactbfZBnUW295888 = wsVXkhnactbfZBnUW90021086;     wsVXkhnactbfZBnUW90021086 = wsVXkhnactbfZBnUW55917118;     wsVXkhnactbfZBnUW55917118 = wsVXkhnactbfZBnUW63682841;     wsVXkhnactbfZBnUW63682841 = wsVXkhnactbfZBnUW84371630;     wsVXkhnactbfZBnUW84371630 = wsVXkhnactbfZBnUW77534586;     wsVXkhnactbfZBnUW77534586 = wsVXkhnactbfZBnUW26993103;     wsVXkhnactbfZBnUW26993103 = wsVXkhnactbfZBnUW54604886;     wsVXkhnactbfZBnUW54604886 = wsVXkhnactbfZBnUW76151286;     wsVXkhnactbfZBnUW76151286 = wsVXkhnactbfZBnUW71726769;     wsVXkhnactbfZBnUW71726769 = wsVXkhnactbfZBnUW99441695;     wsVXkhnactbfZBnUW99441695 = wsVXkhnactbfZBnUW8713080;     wsVXkhnactbfZBnUW8713080 = wsVXkhnactbfZBnUW46794024;     wsVXkhnactbfZBnUW46794024 = wsVXkhnactbfZBnUW3955601;     wsVXkhnactbfZBnUW3955601 = wsVXkhnactbfZBnUW28161775;     wsVXkhnactbfZBnUW28161775 = wsVXkhnactbfZBnUW40052554;     wsVXkhnactbfZBnUW40052554 = wsVXkhnactbfZBnUW34447950;     wsVXkhnactbfZBnUW34447950 = wsVXkhnactbfZBnUW32181396;     wsVXkhnactbfZBnUW32181396 = wsVXkhnactbfZBnUW16431885;     wsVXkhnactbfZBnUW16431885 = wsVXkhnactbfZBnUW90283708;     wsVXkhnactbfZBnUW90283708 = wsVXkhnactbfZBnUW44033442;     wsVXkhnactbfZBnUW44033442 = wsVXkhnactbfZBnUW60193928;     wsVXkhnactbfZBnUW60193928 = wsVXkhnactbfZBnUW72683176;     wsVXkhnactbfZBnUW72683176 = wsVXkhnactbfZBnUW23883156;     wsVXkhnactbfZBnUW23883156 = wsVXkhnactbfZBnUW37850767;     wsVXkhnactbfZBnUW37850767 = wsVXkhnactbfZBnUW83133677;     wsVXkhnactbfZBnUW83133677 = wsVXkhnactbfZBnUW36370400;     wsVXkhnactbfZBnUW36370400 = wsVXkhnactbfZBnUW58779160;     wsVXkhnactbfZBnUW58779160 = wsVXkhnactbfZBnUW74950226;     wsVXkhnactbfZBnUW74950226 = wsVXkhnactbfZBnUW94572260;     wsVXkhnactbfZBnUW94572260 = wsVXkhnactbfZBnUW25803932;     wsVXkhnactbfZBnUW25803932 = wsVXkhnactbfZBnUW27307015;     wsVXkhnactbfZBnUW27307015 = wsVXkhnactbfZBnUW82633602;     wsVXkhnactbfZBnUW82633602 = wsVXkhnactbfZBnUW63952741;     wsVXkhnactbfZBnUW63952741 = wsVXkhnactbfZBnUW98604763;     wsVXkhnactbfZBnUW98604763 = wsVXkhnactbfZBnUW85073359;     wsVXkhnactbfZBnUW85073359 = wsVXkhnactbfZBnUW29589003;     wsVXkhnactbfZBnUW29589003 = wsVXkhnactbfZBnUW65880250;     wsVXkhnactbfZBnUW65880250 = wsVXkhnactbfZBnUW92590982;     wsVXkhnactbfZBnUW92590982 = wsVXkhnactbfZBnUW55689566;     wsVXkhnactbfZBnUW55689566 = wsVXkhnactbfZBnUW8790304;     wsVXkhnactbfZBnUW8790304 = wsVXkhnactbfZBnUW10741881;     wsVXkhnactbfZBnUW10741881 = wsVXkhnactbfZBnUW96874711;     wsVXkhnactbfZBnUW96874711 = wsVXkhnactbfZBnUW96195027;     wsVXkhnactbfZBnUW96195027 = wsVXkhnactbfZBnUW47147336;     wsVXkhnactbfZBnUW47147336 = wsVXkhnactbfZBnUW94088671;     wsVXkhnactbfZBnUW94088671 = wsVXkhnactbfZBnUW26029095;     wsVXkhnactbfZBnUW26029095 = wsVXkhnactbfZBnUW23691698;     wsVXkhnactbfZBnUW23691698 = wsVXkhnactbfZBnUW30062369;     wsVXkhnactbfZBnUW30062369 = wsVXkhnactbfZBnUW66453477;     wsVXkhnactbfZBnUW66453477 = wsVXkhnactbfZBnUW80885890;     wsVXkhnactbfZBnUW80885890 = wsVXkhnactbfZBnUW11264775;     wsVXkhnactbfZBnUW11264775 = wsVXkhnactbfZBnUW66917578;     wsVXkhnactbfZBnUW66917578 = wsVXkhnactbfZBnUW1431208;     wsVXkhnactbfZBnUW1431208 = wsVXkhnactbfZBnUW60148283;     wsVXkhnactbfZBnUW60148283 = wsVXkhnactbfZBnUW40101960;     wsVXkhnactbfZBnUW40101960 = wsVXkhnactbfZBnUW17337910;     wsVXkhnactbfZBnUW17337910 = wsVXkhnactbfZBnUW32033962;     wsVXkhnactbfZBnUW32033962 = wsVXkhnactbfZBnUW25832074;     wsVXkhnactbfZBnUW25832074 = wsVXkhnactbfZBnUW1237954;     wsVXkhnactbfZBnUW1237954 = wsVXkhnactbfZBnUW41164186;     wsVXkhnactbfZBnUW41164186 = wsVXkhnactbfZBnUW68213942;     wsVXkhnactbfZBnUW68213942 = wsVXkhnactbfZBnUW79654659;     wsVXkhnactbfZBnUW79654659 = wsVXkhnactbfZBnUW81579026;     wsVXkhnactbfZBnUW81579026 = wsVXkhnactbfZBnUW45922838;     wsVXkhnactbfZBnUW45922838 = wsVXkhnactbfZBnUW72134680;     wsVXkhnactbfZBnUW72134680 = wsVXkhnactbfZBnUW26079477;     wsVXkhnactbfZBnUW26079477 = wsVXkhnactbfZBnUW82841283;     wsVXkhnactbfZBnUW82841283 = wsVXkhnactbfZBnUW5350837;     wsVXkhnactbfZBnUW5350837 = wsVXkhnactbfZBnUW43088416;     wsVXkhnactbfZBnUW43088416 = wsVXkhnactbfZBnUW10463551;     wsVXkhnactbfZBnUW10463551 = wsVXkhnactbfZBnUW68567699;     wsVXkhnactbfZBnUW68567699 = wsVXkhnactbfZBnUW39590413;     wsVXkhnactbfZBnUW39590413 = wsVXkhnactbfZBnUW60742318;     wsVXkhnactbfZBnUW60742318 = wsVXkhnactbfZBnUW81493404;     wsVXkhnactbfZBnUW81493404 = wsVXkhnactbfZBnUW33291562;     wsVXkhnactbfZBnUW33291562 = wsVXkhnactbfZBnUW63319217;     wsVXkhnactbfZBnUW63319217 = wsVXkhnactbfZBnUW76488149;     wsVXkhnactbfZBnUW76488149 = wsVXkhnactbfZBnUW76735820;     wsVXkhnactbfZBnUW76735820 = wsVXkhnactbfZBnUW43762095;     wsVXkhnactbfZBnUW43762095 = wsVXkhnactbfZBnUW57104583;     wsVXkhnactbfZBnUW57104583 = wsVXkhnactbfZBnUW12678703;     wsVXkhnactbfZBnUW12678703 = wsVXkhnactbfZBnUW28716792;     wsVXkhnactbfZBnUW28716792 = wsVXkhnactbfZBnUW8496749;     wsVXkhnactbfZBnUW8496749 = wsVXkhnactbfZBnUW13686371;     wsVXkhnactbfZBnUW13686371 = wsVXkhnactbfZBnUW14539157;     wsVXkhnactbfZBnUW14539157 = wsVXkhnactbfZBnUW60389436;     wsVXkhnactbfZBnUW60389436 = wsVXkhnactbfZBnUW81202394;     wsVXkhnactbfZBnUW81202394 = wsVXkhnactbfZBnUW3804459;     wsVXkhnactbfZBnUW3804459 = wsVXkhnactbfZBnUW58502804;     wsVXkhnactbfZBnUW58502804 = wsVXkhnactbfZBnUW67735449;     wsVXkhnactbfZBnUW67735449 = wsVXkhnactbfZBnUW97555040;     wsVXkhnactbfZBnUW97555040 = wsVXkhnactbfZBnUW40048176;     wsVXkhnactbfZBnUW40048176 = wsVXkhnactbfZBnUW91353029;     wsVXkhnactbfZBnUW91353029 = wsVXkhnactbfZBnUW14525380;     wsVXkhnactbfZBnUW14525380 = wsVXkhnactbfZBnUW40576362;     wsVXkhnactbfZBnUW40576362 = wsVXkhnactbfZBnUW43446171;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BuPpXXjoHrgonUzg30382908() {     double uAsgPLxEUfxHzSaWT49563278 = -769942457;    double uAsgPLxEUfxHzSaWT26722742 = -541347248;    double uAsgPLxEUfxHzSaWT7757100 = -597838839;    double uAsgPLxEUfxHzSaWT31256506 = -365609417;    double uAsgPLxEUfxHzSaWT69191376 = -772548727;    double uAsgPLxEUfxHzSaWT81438289 = -789430306;    double uAsgPLxEUfxHzSaWT16406085 = -937215513;    double uAsgPLxEUfxHzSaWT13442732 = -330160435;    double uAsgPLxEUfxHzSaWT49014126 = -152143475;    double uAsgPLxEUfxHzSaWT18764246 = -93475376;    double uAsgPLxEUfxHzSaWT87103392 = -89019610;    double uAsgPLxEUfxHzSaWT86427928 = -276478339;    double uAsgPLxEUfxHzSaWT62360405 = -120575254;    double uAsgPLxEUfxHzSaWT54870446 = -136814633;    double uAsgPLxEUfxHzSaWT95184121 = -845851995;    double uAsgPLxEUfxHzSaWT50218473 = -523652176;    double uAsgPLxEUfxHzSaWT40630027 = 91734446;    double uAsgPLxEUfxHzSaWT1008754 = -862752868;    double uAsgPLxEUfxHzSaWT89991616 = -920302028;    double uAsgPLxEUfxHzSaWT20602718 = 56437400;    double uAsgPLxEUfxHzSaWT23001143 = -421018806;    double uAsgPLxEUfxHzSaWT18702420 = -125192702;    double uAsgPLxEUfxHzSaWT52022676 = 301727;    double uAsgPLxEUfxHzSaWT74276082 = -877084497;    double uAsgPLxEUfxHzSaWT87733092 = -143998355;    double uAsgPLxEUfxHzSaWT84915603 = -385007420;    double uAsgPLxEUfxHzSaWT71142955 = -31257131;    double uAsgPLxEUfxHzSaWT36990282 = -550600592;    double uAsgPLxEUfxHzSaWT3983022 = -393500064;    double uAsgPLxEUfxHzSaWT51749314 = -675477250;    double uAsgPLxEUfxHzSaWT28612950 = -599606189;    double uAsgPLxEUfxHzSaWT56389782 = -178835596;    double uAsgPLxEUfxHzSaWT84661175 = -384227053;    double uAsgPLxEUfxHzSaWT89402824 = -301454299;    double uAsgPLxEUfxHzSaWT87278583 = -514485361;    double uAsgPLxEUfxHzSaWT96032928 = -685767255;    double uAsgPLxEUfxHzSaWT63895537 = -665260308;    double uAsgPLxEUfxHzSaWT98042953 = -752551200;    double uAsgPLxEUfxHzSaWT72215017 = -160641601;    double uAsgPLxEUfxHzSaWT72575296 = -220354738;    double uAsgPLxEUfxHzSaWT35258226 = -27557631;    double uAsgPLxEUfxHzSaWT98030707 = -810971048;    double uAsgPLxEUfxHzSaWT70657961 = -503229536;    double uAsgPLxEUfxHzSaWT43097307 = 95707033;    double uAsgPLxEUfxHzSaWT20742769 = -190385407;    double uAsgPLxEUfxHzSaWT22681076 = -388860178;    double uAsgPLxEUfxHzSaWT14653286 = -444865031;    double uAsgPLxEUfxHzSaWT4151969 = -163470429;    double uAsgPLxEUfxHzSaWT56823065 = -186816315;    double uAsgPLxEUfxHzSaWT7000325 = -791268240;    double uAsgPLxEUfxHzSaWT61729773 = -377467787;    double uAsgPLxEUfxHzSaWT34501417 = -162966157;    double uAsgPLxEUfxHzSaWT78310826 = -272373332;    double uAsgPLxEUfxHzSaWT62422358 = -374031285;    double uAsgPLxEUfxHzSaWT7381765 = -588660452;    double uAsgPLxEUfxHzSaWT30860858 = -544749756;    double uAsgPLxEUfxHzSaWT74700066 = -441648975;    double uAsgPLxEUfxHzSaWT33481018 = -720754343;    double uAsgPLxEUfxHzSaWT43523414 = -121611063;    double uAsgPLxEUfxHzSaWT84275773 = -287541307;    double uAsgPLxEUfxHzSaWT10295335 = -658173176;    double uAsgPLxEUfxHzSaWT79415802 = -286614921;    double uAsgPLxEUfxHzSaWT9459710 = -936660371;    double uAsgPLxEUfxHzSaWT97264811 = -476666226;    double uAsgPLxEUfxHzSaWT90151296 = -493869188;    double uAsgPLxEUfxHzSaWT30713611 = -910184014;    double uAsgPLxEUfxHzSaWT1766754 = -892251286;    double uAsgPLxEUfxHzSaWT72957580 = -819120956;    double uAsgPLxEUfxHzSaWT67591862 = -622329273;    double uAsgPLxEUfxHzSaWT99151193 = -60084740;    double uAsgPLxEUfxHzSaWT86322936 = -858391869;    double uAsgPLxEUfxHzSaWT42587074 = -155714354;    double uAsgPLxEUfxHzSaWT28793736 = -602111268;    double uAsgPLxEUfxHzSaWT17416320 = -599947290;    double uAsgPLxEUfxHzSaWT85344491 = -916004970;    double uAsgPLxEUfxHzSaWT24970435 = -610047758;    double uAsgPLxEUfxHzSaWT48044459 = -621963166;    double uAsgPLxEUfxHzSaWT8925370 = 4594693;    double uAsgPLxEUfxHzSaWT53533313 = -586699090;    double uAsgPLxEUfxHzSaWT65052016 = -755138177;    double uAsgPLxEUfxHzSaWT70262318 = -940142390;    double uAsgPLxEUfxHzSaWT66990987 = -867786703;    double uAsgPLxEUfxHzSaWT80167216 = -263784278;    double uAsgPLxEUfxHzSaWT96982697 = -602231825;    double uAsgPLxEUfxHzSaWT90019541 = -198009464;    double uAsgPLxEUfxHzSaWT94111532 = -336640033;    double uAsgPLxEUfxHzSaWT78078956 = -906462265;    double uAsgPLxEUfxHzSaWT22238817 = 89804232;    double uAsgPLxEUfxHzSaWT82021060 = -712793847;    double uAsgPLxEUfxHzSaWT56417726 = -969735605;    double uAsgPLxEUfxHzSaWT21332862 = -144118281;    double uAsgPLxEUfxHzSaWT30414520 = -944505966;    double uAsgPLxEUfxHzSaWT54519540 = -530940138;    double uAsgPLxEUfxHzSaWT87939244 = -873100294;    double uAsgPLxEUfxHzSaWT62279962 = -562181563;    double uAsgPLxEUfxHzSaWT55842424 = -740942710;    double uAsgPLxEUfxHzSaWT88570997 = -874310677;    double uAsgPLxEUfxHzSaWT73393149 = 73436689;    double uAsgPLxEUfxHzSaWT52946011 = -410423780;    double uAsgPLxEUfxHzSaWT90029158 = -769942457;     uAsgPLxEUfxHzSaWT49563278 = uAsgPLxEUfxHzSaWT26722742;     uAsgPLxEUfxHzSaWT26722742 = uAsgPLxEUfxHzSaWT7757100;     uAsgPLxEUfxHzSaWT7757100 = uAsgPLxEUfxHzSaWT31256506;     uAsgPLxEUfxHzSaWT31256506 = uAsgPLxEUfxHzSaWT69191376;     uAsgPLxEUfxHzSaWT69191376 = uAsgPLxEUfxHzSaWT81438289;     uAsgPLxEUfxHzSaWT81438289 = uAsgPLxEUfxHzSaWT16406085;     uAsgPLxEUfxHzSaWT16406085 = uAsgPLxEUfxHzSaWT13442732;     uAsgPLxEUfxHzSaWT13442732 = uAsgPLxEUfxHzSaWT49014126;     uAsgPLxEUfxHzSaWT49014126 = uAsgPLxEUfxHzSaWT18764246;     uAsgPLxEUfxHzSaWT18764246 = uAsgPLxEUfxHzSaWT87103392;     uAsgPLxEUfxHzSaWT87103392 = uAsgPLxEUfxHzSaWT86427928;     uAsgPLxEUfxHzSaWT86427928 = uAsgPLxEUfxHzSaWT62360405;     uAsgPLxEUfxHzSaWT62360405 = uAsgPLxEUfxHzSaWT54870446;     uAsgPLxEUfxHzSaWT54870446 = uAsgPLxEUfxHzSaWT95184121;     uAsgPLxEUfxHzSaWT95184121 = uAsgPLxEUfxHzSaWT50218473;     uAsgPLxEUfxHzSaWT50218473 = uAsgPLxEUfxHzSaWT40630027;     uAsgPLxEUfxHzSaWT40630027 = uAsgPLxEUfxHzSaWT1008754;     uAsgPLxEUfxHzSaWT1008754 = uAsgPLxEUfxHzSaWT89991616;     uAsgPLxEUfxHzSaWT89991616 = uAsgPLxEUfxHzSaWT20602718;     uAsgPLxEUfxHzSaWT20602718 = uAsgPLxEUfxHzSaWT23001143;     uAsgPLxEUfxHzSaWT23001143 = uAsgPLxEUfxHzSaWT18702420;     uAsgPLxEUfxHzSaWT18702420 = uAsgPLxEUfxHzSaWT52022676;     uAsgPLxEUfxHzSaWT52022676 = uAsgPLxEUfxHzSaWT74276082;     uAsgPLxEUfxHzSaWT74276082 = uAsgPLxEUfxHzSaWT87733092;     uAsgPLxEUfxHzSaWT87733092 = uAsgPLxEUfxHzSaWT84915603;     uAsgPLxEUfxHzSaWT84915603 = uAsgPLxEUfxHzSaWT71142955;     uAsgPLxEUfxHzSaWT71142955 = uAsgPLxEUfxHzSaWT36990282;     uAsgPLxEUfxHzSaWT36990282 = uAsgPLxEUfxHzSaWT3983022;     uAsgPLxEUfxHzSaWT3983022 = uAsgPLxEUfxHzSaWT51749314;     uAsgPLxEUfxHzSaWT51749314 = uAsgPLxEUfxHzSaWT28612950;     uAsgPLxEUfxHzSaWT28612950 = uAsgPLxEUfxHzSaWT56389782;     uAsgPLxEUfxHzSaWT56389782 = uAsgPLxEUfxHzSaWT84661175;     uAsgPLxEUfxHzSaWT84661175 = uAsgPLxEUfxHzSaWT89402824;     uAsgPLxEUfxHzSaWT89402824 = uAsgPLxEUfxHzSaWT87278583;     uAsgPLxEUfxHzSaWT87278583 = uAsgPLxEUfxHzSaWT96032928;     uAsgPLxEUfxHzSaWT96032928 = uAsgPLxEUfxHzSaWT63895537;     uAsgPLxEUfxHzSaWT63895537 = uAsgPLxEUfxHzSaWT98042953;     uAsgPLxEUfxHzSaWT98042953 = uAsgPLxEUfxHzSaWT72215017;     uAsgPLxEUfxHzSaWT72215017 = uAsgPLxEUfxHzSaWT72575296;     uAsgPLxEUfxHzSaWT72575296 = uAsgPLxEUfxHzSaWT35258226;     uAsgPLxEUfxHzSaWT35258226 = uAsgPLxEUfxHzSaWT98030707;     uAsgPLxEUfxHzSaWT98030707 = uAsgPLxEUfxHzSaWT70657961;     uAsgPLxEUfxHzSaWT70657961 = uAsgPLxEUfxHzSaWT43097307;     uAsgPLxEUfxHzSaWT43097307 = uAsgPLxEUfxHzSaWT20742769;     uAsgPLxEUfxHzSaWT20742769 = uAsgPLxEUfxHzSaWT22681076;     uAsgPLxEUfxHzSaWT22681076 = uAsgPLxEUfxHzSaWT14653286;     uAsgPLxEUfxHzSaWT14653286 = uAsgPLxEUfxHzSaWT4151969;     uAsgPLxEUfxHzSaWT4151969 = uAsgPLxEUfxHzSaWT56823065;     uAsgPLxEUfxHzSaWT56823065 = uAsgPLxEUfxHzSaWT7000325;     uAsgPLxEUfxHzSaWT7000325 = uAsgPLxEUfxHzSaWT61729773;     uAsgPLxEUfxHzSaWT61729773 = uAsgPLxEUfxHzSaWT34501417;     uAsgPLxEUfxHzSaWT34501417 = uAsgPLxEUfxHzSaWT78310826;     uAsgPLxEUfxHzSaWT78310826 = uAsgPLxEUfxHzSaWT62422358;     uAsgPLxEUfxHzSaWT62422358 = uAsgPLxEUfxHzSaWT7381765;     uAsgPLxEUfxHzSaWT7381765 = uAsgPLxEUfxHzSaWT30860858;     uAsgPLxEUfxHzSaWT30860858 = uAsgPLxEUfxHzSaWT74700066;     uAsgPLxEUfxHzSaWT74700066 = uAsgPLxEUfxHzSaWT33481018;     uAsgPLxEUfxHzSaWT33481018 = uAsgPLxEUfxHzSaWT43523414;     uAsgPLxEUfxHzSaWT43523414 = uAsgPLxEUfxHzSaWT84275773;     uAsgPLxEUfxHzSaWT84275773 = uAsgPLxEUfxHzSaWT10295335;     uAsgPLxEUfxHzSaWT10295335 = uAsgPLxEUfxHzSaWT79415802;     uAsgPLxEUfxHzSaWT79415802 = uAsgPLxEUfxHzSaWT9459710;     uAsgPLxEUfxHzSaWT9459710 = uAsgPLxEUfxHzSaWT97264811;     uAsgPLxEUfxHzSaWT97264811 = uAsgPLxEUfxHzSaWT90151296;     uAsgPLxEUfxHzSaWT90151296 = uAsgPLxEUfxHzSaWT30713611;     uAsgPLxEUfxHzSaWT30713611 = uAsgPLxEUfxHzSaWT1766754;     uAsgPLxEUfxHzSaWT1766754 = uAsgPLxEUfxHzSaWT72957580;     uAsgPLxEUfxHzSaWT72957580 = uAsgPLxEUfxHzSaWT67591862;     uAsgPLxEUfxHzSaWT67591862 = uAsgPLxEUfxHzSaWT99151193;     uAsgPLxEUfxHzSaWT99151193 = uAsgPLxEUfxHzSaWT86322936;     uAsgPLxEUfxHzSaWT86322936 = uAsgPLxEUfxHzSaWT42587074;     uAsgPLxEUfxHzSaWT42587074 = uAsgPLxEUfxHzSaWT28793736;     uAsgPLxEUfxHzSaWT28793736 = uAsgPLxEUfxHzSaWT17416320;     uAsgPLxEUfxHzSaWT17416320 = uAsgPLxEUfxHzSaWT85344491;     uAsgPLxEUfxHzSaWT85344491 = uAsgPLxEUfxHzSaWT24970435;     uAsgPLxEUfxHzSaWT24970435 = uAsgPLxEUfxHzSaWT48044459;     uAsgPLxEUfxHzSaWT48044459 = uAsgPLxEUfxHzSaWT8925370;     uAsgPLxEUfxHzSaWT8925370 = uAsgPLxEUfxHzSaWT53533313;     uAsgPLxEUfxHzSaWT53533313 = uAsgPLxEUfxHzSaWT65052016;     uAsgPLxEUfxHzSaWT65052016 = uAsgPLxEUfxHzSaWT70262318;     uAsgPLxEUfxHzSaWT70262318 = uAsgPLxEUfxHzSaWT66990987;     uAsgPLxEUfxHzSaWT66990987 = uAsgPLxEUfxHzSaWT80167216;     uAsgPLxEUfxHzSaWT80167216 = uAsgPLxEUfxHzSaWT96982697;     uAsgPLxEUfxHzSaWT96982697 = uAsgPLxEUfxHzSaWT90019541;     uAsgPLxEUfxHzSaWT90019541 = uAsgPLxEUfxHzSaWT94111532;     uAsgPLxEUfxHzSaWT94111532 = uAsgPLxEUfxHzSaWT78078956;     uAsgPLxEUfxHzSaWT78078956 = uAsgPLxEUfxHzSaWT22238817;     uAsgPLxEUfxHzSaWT22238817 = uAsgPLxEUfxHzSaWT82021060;     uAsgPLxEUfxHzSaWT82021060 = uAsgPLxEUfxHzSaWT56417726;     uAsgPLxEUfxHzSaWT56417726 = uAsgPLxEUfxHzSaWT21332862;     uAsgPLxEUfxHzSaWT21332862 = uAsgPLxEUfxHzSaWT30414520;     uAsgPLxEUfxHzSaWT30414520 = uAsgPLxEUfxHzSaWT54519540;     uAsgPLxEUfxHzSaWT54519540 = uAsgPLxEUfxHzSaWT87939244;     uAsgPLxEUfxHzSaWT87939244 = uAsgPLxEUfxHzSaWT62279962;     uAsgPLxEUfxHzSaWT62279962 = uAsgPLxEUfxHzSaWT55842424;     uAsgPLxEUfxHzSaWT55842424 = uAsgPLxEUfxHzSaWT88570997;     uAsgPLxEUfxHzSaWT88570997 = uAsgPLxEUfxHzSaWT73393149;     uAsgPLxEUfxHzSaWT73393149 = uAsgPLxEUfxHzSaWT52946011;     uAsgPLxEUfxHzSaWT52946011 = uAsgPLxEUfxHzSaWT90029158;     uAsgPLxEUfxHzSaWT90029158 = uAsgPLxEUfxHzSaWT49563278;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void fRGZZlNEuyukklhX97674506() {     double GwKuRcFwVBbDOfEZC85022246 = -136219357;    double GwKuRcFwVBbDOfEZC39994821 = -421898854;    double GwKuRcFwVBbDOfEZC45143576 = -263505914;    double GwKuRcFwVBbDOfEZC3479861 = -186760489;    double GwKuRcFwVBbDOfEZC25476646 = -878147573;    double GwKuRcFwVBbDOfEZC17661161 = 48221798;    double GwKuRcFwVBbDOfEZC27063491 = -781941247;    double GwKuRcFwVBbDOfEZC75574292 = 25605425;    double GwKuRcFwVBbDOfEZC50531731 = -640474882;    double GwKuRcFwVBbDOfEZC33666958 = -858162374;    double GwKuRcFwVBbDOfEZC25285128 = -991027339;    double GwKuRcFwVBbDOfEZC86996076 = -140599110;    double GwKuRcFwVBbDOfEZC595051 = -672587759;    double GwKuRcFwVBbDOfEZC8011310 = -113178042;    double GwKuRcFwVBbDOfEZC53378638 = -249277010;    double GwKuRcFwVBbDOfEZC33759774 = -159463784;    double GwKuRcFwVBbDOfEZC96465962 = -435240107;    double GwKuRcFwVBbDOfEZC768946 = -936180937;    double GwKuRcFwVBbDOfEZC90541946 = -136865449;    double GwKuRcFwVBbDOfEZC14439937 = -370123666;    double GwKuRcFwVBbDOfEZC36088006 = -78293494;    double GwKuRcFwVBbDOfEZC22123945 = -211828996;    double GwKuRcFwVBbDOfEZC22548056 = -737330458;    double GwKuRcFwVBbDOfEZC68069027 = -287909101;    double GwKuRcFwVBbDOfEZC95751043 = -167983553;    double GwKuRcFwVBbDOfEZC95613775 = -73584227;    double GwKuRcFwVBbDOfEZC31563529 = -937541272;    double GwKuRcFwVBbDOfEZC77718706 = -720372767;    double GwKuRcFwVBbDOfEZC50651237 = -967594867;    double GwKuRcFwVBbDOfEZC57282725 = -709984421;    double GwKuRcFwVBbDOfEZC47196181 = -51586676;    double GwKuRcFwVBbDOfEZC94212800 = 48192989;    double GwKuRcFwVBbDOfEZC96879185 = -531823030;    double GwKuRcFwVBbDOfEZC19151251 = -366148113;    double GwKuRcFwVBbDOfEZC53087037 = -465585611;    double GwKuRcFwVBbDOfEZC47593772 = -900332561;    double GwKuRcFwVBbDOfEZC13307354 = -529956348;    double GwKuRcFwVBbDOfEZC49258357 = -735206782;    double GwKuRcFwVBbDOfEZC2605580 = -911517877;    double GwKuRcFwVBbDOfEZC36846829 = -785617284;    double GwKuRcFwVBbDOfEZC68709834 = -896756545;    double GwKuRcFwVBbDOfEZC87395834 = -405516339;    double GwKuRcFwVBbDOfEZC60041339 = -56410322;    double GwKuRcFwVBbDOfEZC83228645 = -508625377;    double GwKuRcFwVBbDOfEZC20759228 = -16896569;    double GwKuRcFwVBbDOfEZC68642404 = -427783811;    double GwKuRcFwVBbDOfEZC33998449 = -928851392;    double GwKuRcFwVBbDOfEZC21878280 = -329997022;    double GwKuRcFwVBbDOfEZC79715731 = -950553195;    double GwKuRcFwVBbDOfEZC31529914 = -182295055;    double GwKuRcFwVBbDOfEZC42500497 = -251744344;    double GwKuRcFwVBbDOfEZC17986890 = -376618503;    double GwKuRcFwVBbDOfEZC46931975 = -28418886;    double GwKuRcFwVBbDOfEZC78414204 = -565539785;    double GwKuRcFwVBbDOfEZC61586501 = -935150598;    double GwKuRcFwVBbDOfEZC62898301 = -924390362;    double GwKuRcFwVBbDOfEZC17446765 = -684568397;    double GwKuRcFwVBbDOfEZC77074549 = -975596814;    double GwKuRcFwVBbDOfEZC7728818 = 81223064;    double GwKuRcFwVBbDOfEZC29862871 = -704563346;    double GwKuRcFwVBbDOfEZC86097632 = -14236930;    double GwKuRcFwVBbDOfEZC49344784 = 38431520;    double GwKuRcFwVBbDOfEZC24923056 = -6799709;    double GwKuRcFwVBbDOfEZC93249006 = -930490462;    double GwKuRcFwVBbDOfEZC86470776 = -706575699;    double GwKuRcFwVBbDOfEZC31072327 = -939220329;    double GwKuRcFwVBbDOfEZC90116891 = -608776081;    double GwKuRcFwVBbDOfEZC81443799 = -206439647;    double GwKuRcFwVBbDOfEZC54924272 = -647592431;    double GwKuRcFwVBbDOfEZC5784867 = -348944450;    double GwKuRcFwVBbDOfEZC20452421 = -629507436;    double GwKuRcFwVBbDOfEZC47207606 = -700033326;    double GwKuRcFwVBbDOfEZC98163366 = 75336940;    double GwKuRcFwVBbDOfEZC53695118 = -351248165;    double GwKuRcFwVBbDOfEZC45730103 = -473367121;    double GwKuRcFwVBbDOfEZC48692171 = -672777156;    double GwKuRcFwVBbDOfEZC62082605 = -55418674;    double GwKuRcFwVBbDOfEZC39319411 = -128705081;    double GwKuRcFwVBbDOfEZC47309800 = -171012533;    double GwKuRcFwVBbDOfEZC27108640 = -740199743;    double GwKuRcFwVBbDOfEZC61615326 = -144732836;    double GwKuRcFwVBbDOfEZC9685249 = -507544250;    double GwKuRcFwVBbDOfEZC98002975 = -769819572;    double GwKuRcFwVBbDOfEZC19121323 = -685299812;    double GwKuRcFwVBbDOfEZC14782229 = -358240077;    double GwKuRcFwVBbDOfEZC29209292 = -674968173;    double GwKuRcFwVBbDOfEZC47280826 = -923388126;    double GwKuRcFwVBbDOfEZC18464982 = -966283245;    double GwKuRcFwVBbDOfEZC57564750 = -430997515;    double GwKuRcFwVBbDOfEZC90188735 = -541195250;    double GwKuRcFwVBbDOfEZC30147008 = -115764164;    double GwKuRcFwVBbDOfEZC36232805 = -554359535;    double GwKuRcFwVBbDOfEZC41529540 = -716429846;    double GwKuRcFwVBbDOfEZC72742708 = -106954532;    double GwKuRcFwVBbDOfEZC50749197 = -671380355;    double GwKuRcFwVBbDOfEZC19365050 = -835188065;    double GwKuRcFwVBbDOfEZC62472778 = -298716630;    double GwKuRcFwVBbDOfEZC66792333 = -125919861;    double GwKuRcFwVBbDOfEZC96757868 = -802049679;    double GwKuRcFwVBbDOfEZC89686900 = -136219357;     GwKuRcFwVBbDOfEZC85022246 = GwKuRcFwVBbDOfEZC39994821;     GwKuRcFwVBbDOfEZC39994821 = GwKuRcFwVBbDOfEZC45143576;     GwKuRcFwVBbDOfEZC45143576 = GwKuRcFwVBbDOfEZC3479861;     GwKuRcFwVBbDOfEZC3479861 = GwKuRcFwVBbDOfEZC25476646;     GwKuRcFwVBbDOfEZC25476646 = GwKuRcFwVBbDOfEZC17661161;     GwKuRcFwVBbDOfEZC17661161 = GwKuRcFwVBbDOfEZC27063491;     GwKuRcFwVBbDOfEZC27063491 = GwKuRcFwVBbDOfEZC75574292;     GwKuRcFwVBbDOfEZC75574292 = GwKuRcFwVBbDOfEZC50531731;     GwKuRcFwVBbDOfEZC50531731 = GwKuRcFwVBbDOfEZC33666958;     GwKuRcFwVBbDOfEZC33666958 = GwKuRcFwVBbDOfEZC25285128;     GwKuRcFwVBbDOfEZC25285128 = GwKuRcFwVBbDOfEZC86996076;     GwKuRcFwVBbDOfEZC86996076 = GwKuRcFwVBbDOfEZC595051;     GwKuRcFwVBbDOfEZC595051 = GwKuRcFwVBbDOfEZC8011310;     GwKuRcFwVBbDOfEZC8011310 = GwKuRcFwVBbDOfEZC53378638;     GwKuRcFwVBbDOfEZC53378638 = GwKuRcFwVBbDOfEZC33759774;     GwKuRcFwVBbDOfEZC33759774 = GwKuRcFwVBbDOfEZC96465962;     GwKuRcFwVBbDOfEZC96465962 = GwKuRcFwVBbDOfEZC768946;     GwKuRcFwVBbDOfEZC768946 = GwKuRcFwVBbDOfEZC90541946;     GwKuRcFwVBbDOfEZC90541946 = GwKuRcFwVBbDOfEZC14439937;     GwKuRcFwVBbDOfEZC14439937 = GwKuRcFwVBbDOfEZC36088006;     GwKuRcFwVBbDOfEZC36088006 = GwKuRcFwVBbDOfEZC22123945;     GwKuRcFwVBbDOfEZC22123945 = GwKuRcFwVBbDOfEZC22548056;     GwKuRcFwVBbDOfEZC22548056 = GwKuRcFwVBbDOfEZC68069027;     GwKuRcFwVBbDOfEZC68069027 = GwKuRcFwVBbDOfEZC95751043;     GwKuRcFwVBbDOfEZC95751043 = GwKuRcFwVBbDOfEZC95613775;     GwKuRcFwVBbDOfEZC95613775 = GwKuRcFwVBbDOfEZC31563529;     GwKuRcFwVBbDOfEZC31563529 = GwKuRcFwVBbDOfEZC77718706;     GwKuRcFwVBbDOfEZC77718706 = GwKuRcFwVBbDOfEZC50651237;     GwKuRcFwVBbDOfEZC50651237 = GwKuRcFwVBbDOfEZC57282725;     GwKuRcFwVBbDOfEZC57282725 = GwKuRcFwVBbDOfEZC47196181;     GwKuRcFwVBbDOfEZC47196181 = GwKuRcFwVBbDOfEZC94212800;     GwKuRcFwVBbDOfEZC94212800 = GwKuRcFwVBbDOfEZC96879185;     GwKuRcFwVBbDOfEZC96879185 = GwKuRcFwVBbDOfEZC19151251;     GwKuRcFwVBbDOfEZC19151251 = GwKuRcFwVBbDOfEZC53087037;     GwKuRcFwVBbDOfEZC53087037 = GwKuRcFwVBbDOfEZC47593772;     GwKuRcFwVBbDOfEZC47593772 = GwKuRcFwVBbDOfEZC13307354;     GwKuRcFwVBbDOfEZC13307354 = GwKuRcFwVBbDOfEZC49258357;     GwKuRcFwVBbDOfEZC49258357 = GwKuRcFwVBbDOfEZC2605580;     GwKuRcFwVBbDOfEZC2605580 = GwKuRcFwVBbDOfEZC36846829;     GwKuRcFwVBbDOfEZC36846829 = GwKuRcFwVBbDOfEZC68709834;     GwKuRcFwVBbDOfEZC68709834 = GwKuRcFwVBbDOfEZC87395834;     GwKuRcFwVBbDOfEZC87395834 = GwKuRcFwVBbDOfEZC60041339;     GwKuRcFwVBbDOfEZC60041339 = GwKuRcFwVBbDOfEZC83228645;     GwKuRcFwVBbDOfEZC83228645 = GwKuRcFwVBbDOfEZC20759228;     GwKuRcFwVBbDOfEZC20759228 = GwKuRcFwVBbDOfEZC68642404;     GwKuRcFwVBbDOfEZC68642404 = GwKuRcFwVBbDOfEZC33998449;     GwKuRcFwVBbDOfEZC33998449 = GwKuRcFwVBbDOfEZC21878280;     GwKuRcFwVBbDOfEZC21878280 = GwKuRcFwVBbDOfEZC79715731;     GwKuRcFwVBbDOfEZC79715731 = GwKuRcFwVBbDOfEZC31529914;     GwKuRcFwVBbDOfEZC31529914 = GwKuRcFwVBbDOfEZC42500497;     GwKuRcFwVBbDOfEZC42500497 = GwKuRcFwVBbDOfEZC17986890;     GwKuRcFwVBbDOfEZC17986890 = GwKuRcFwVBbDOfEZC46931975;     GwKuRcFwVBbDOfEZC46931975 = GwKuRcFwVBbDOfEZC78414204;     GwKuRcFwVBbDOfEZC78414204 = GwKuRcFwVBbDOfEZC61586501;     GwKuRcFwVBbDOfEZC61586501 = GwKuRcFwVBbDOfEZC62898301;     GwKuRcFwVBbDOfEZC62898301 = GwKuRcFwVBbDOfEZC17446765;     GwKuRcFwVBbDOfEZC17446765 = GwKuRcFwVBbDOfEZC77074549;     GwKuRcFwVBbDOfEZC77074549 = GwKuRcFwVBbDOfEZC7728818;     GwKuRcFwVBbDOfEZC7728818 = GwKuRcFwVBbDOfEZC29862871;     GwKuRcFwVBbDOfEZC29862871 = GwKuRcFwVBbDOfEZC86097632;     GwKuRcFwVBbDOfEZC86097632 = GwKuRcFwVBbDOfEZC49344784;     GwKuRcFwVBbDOfEZC49344784 = GwKuRcFwVBbDOfEZC24923056;     GwKuRcFwVBbDOfEZC24923056 = GwKuRcFwVBbDOfEZC93249006;     GwKuRcFwVBbDOfEZC93249006 = GwKuRcFwVBbDOfEZC86470776;     GwKuRcFwVBbDOfEZC86470776 = GwKuRcFwVBbDOfEZC31072327;     GwKuRcFwVBbDOfEZC31072327 = GwKuRcFwVBbDOfEZC90116891;     GwKuRcFwVBbDOfEZC90116891 = GwKuRcFwVBbDOfEZC81443799;     GwKuRcFwVBbDOfEZC81443799 = GwKuRcFwVBbDOfEZC54924272;     GwKuRcFwVBbDOfEZC54924272 = GwKuRcFwVBbDOfEZC5784867;     GwKuRcFwVBbDOfEZC5784867 = GwKuRcFwVBbDOfEZC20452421;     GwKuRcFwVBbDOfEZC20452421 = GwKuRcFwVBbDOfEZC47207606;     GwKuRcFwVBbDOfEZC47207606 = GwKuRcFwVBbDOfEZC98163366;     GwKuRcFwVBbDOfEZC98163366 = GwKuRcFwVBbDOfEZC53695118;     GwKuRcFwVBbDOfEZC53695118 = GwKuRcFwVBbDOfEZC45730103;     GwKuRcFwVBbDOfEZC45730103 = GwKuRcFwVBbDOfEZC48692171;     GwKuRcFwVBbDOfEZC48692171 = GwKuRcFwVBbDOfEZC62082605;     GwKuRcFwVBbDOfEZC62082605 = GwKuRcFwVBbDOfEZC39319411;     GwKuRcFwVBbDOfEZC39319411 = GwKuRcFwVBbDOfEZC47309800;     GwKuRcFwVBbDOfEZC47309800 = GwKuRcFwVBbDOfEZC27108640;     GwKuRcFwVBbDOfEZC27108640 = GwKuRcFwVBbDOfEZC61615326;     GwKuRcFwVBbDOfEZC61615326 = GwKuRcFwVBbDOfEZC9685249;     GwKuRcFwVBbDOfEZC9685249 = GwKuRcFwVBbDOfEZC98002975;     GwKuRcFwVBbDOfEZC98002975 = GwKuRcFwVBbDOfEZC19121323;     GwKuRcFwVBbDOfEZC19121323 = GwKuRcFwVBbDOfEZC14782229;     GwKuRcFwVBbDOfEZC14782229 = GwKuRcFwVBbDOfEZC29209292;     GwKuRcFwVBbDOfEZC29209292 = GwKuRcFwVBbDOfEZC47280826;     GwKuRcFwVBbDOfEZC47280826 = GwKuRcFwVBbDOfEZC18464982;     GwKuRcFwVBbDOfEZC18464982 = GwKuRcFwVBbDOfEZC57564750;     GwKuRcFwVBbDOfEZC57564750 = GwKuRcFwVBbDOfEZC90188735;     GwKuRcFwVBbDOfEZC90188735 = GwKuRcFwVBbDOfEZC30147008;     GwKuRcFwVBbDOfEZC30147008 = GwKuRcFwVBbDOfEZC36232805;     GwKuRcFwVBbDOfEZC36232805 = GwKuRcFwVBbDOfEZC41529540;     GwKuRcFwVBbDOfEZC41529540 = GwKuRcFwVBbDOfEZC72742708;     GwKuRcFwVBbDOfEZC72742708 = GwKuRcFwVBbDOfEZC50749197;     GwKuRcFwVBbDOfEZC50749197 = GwKuRcFwVBbDOfEZC19365050;     GwKuRcFwVBbDOfEZC19365050 = GwKuRcFwVBbDOfEZC62472778;     GwKuRcFwVBbDOfEZC62472778 = GwKuRcFwVBbDOfEZC66792333;     GwKuRcFwVBbDOfEZC66792333 = GwKuRcFwVBbDOfEZC96757868;     GwKuRcFwVBbDOfEZC96757868 = GwKuRcFwVBbDOfEZC89686900;     GwKuRcFwVBbDOfEZC89686900 = GwKuRcFwVBbDOfEZC85022246;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pipmhEabYfrOUsnT12723575() {     double mgemxifewnFwSDmbh91139352 = -248121246;    double mgemxifewnFwSDmbh83368100 = -131498635;    double mgemxifewnFwSDmbh61185760 = -414338836;    double mgemxifewnFwSDmbh30554642 = -377509119;    double mgemxifewnFwSDmbh94372134 = -298797444;    double mgemxifewnFwSDmbh9078364 = -216274437;    double mgemxifewnFwSDmbh87552456 = -920906224;    double mgemxifewnFwSDmbh25334184 = -261268488;    double mgemxifewnFwSDmbh15174227 = -408507548;    double mgemxifewnFwSDmbh74896617 = -87137852;    double mgemxifewnFwSDmbh85395417 = -868464926;    double mgemxifewnFwSDmbh18819120 = -585005312;    double mgemxifewnFwSDmbh86804169 = -12478848;    double mgemxifewnFwSDmbh91154986 = -645416610;    double mgemxifewnFwSDmbh49121065 = -939739177;    double mgemxifewnFwSDmbh75265167 = -132463327;    double mgemxifewnFwSDmbh90301966 = 48214780;    double mgemxifewnFwSDmbh97822098 = -601703579;    double mgemxifewnFwSDmbh52371788 = -542634546;    double mgemxifewnFwSDmbh94990101 = -727026585;    double mgemxifewnFwSDmbh24641199 = 66219050;    double mgemxifewnFwSDmbh8644969 = -119758712;    double mgemxifewnFwSDmbh58138848 = -483847944;    double mgemxifewnFwSDmbh52061401 = -510214089;    double mgemxifewnFwSDmbh39450694 = -541179586;    double mgemxifewnFwSDmbh20335451 = 47760450;    double mgemxifewnFwSDmbh30023307 = -838203475;    double mgemxifewnFwSDmbh90825831 = -786301720;    double mgemxifewnFwSDmbh16783492 = -89977571;    double mgemxifewnFwSDmbh25898362 = -527455288;    double mgemxifewnFwSDmbh39438731 = 35092386;    double mgemxifewnFwSDmbh91823422 = -761253826;    double mgemxifewnFwSDmbh6590135 = -424436400;    double mgemxifewnFwSDmbh13981816 = -117689878;    double mgemxifewnFwSDmbh14561690 = -4318717;    double mgemxifewnFwSDmbh16319685 = -14358669;    double mgemxifewnFwSDmbh94569288 = -483592218;    double mgemxifewnFwSDmbh83348569 = -506712791;    double mgemxifewnFwSDmbh76215833 = -905441374;    double mgemxifewnFwSDmbh24348767 = -159056507;    double mgemxifewnFwSDmbh74379058 = -962030439;    double mgemxifewnFwSDmbh19546291 = -675658617;    double mgemxifewnFwSDmbh38108318 = -137214906;    double mgemxifewnFwSDmbh70636385 = -527968315;    double mgemxifewnFwSDmbh32711693 = -778794723;    double mgemxifewnFwSDmbh80581599 = -412999927;    double mgemxifewnFwSDmbh51777023 = 52373071;    double mgemxifewnFwSDmbh29835221 = -339892555;    double mgemxifewnFwSDmbh89391459 = 63709817;    double mgemxifewnFwSDmbh44441566 = -269305863;    double mgemxifewnFwSDmbh78201175 = -94038015;    double mgemxifewnFwSDmbh28796609 = -177344824;    double mgemxifewnFwSDmbh95180432 = -920208473;    double mgemxifewnFwSDmbh74383084 = -886009930;    double mgemxifewnFwSDmbh88082375 = -535682380;    double mgemxifewnFwSDmbh82494384 = -28362534;    double mgemxifewnFwSDmbh25229252 = -647650691;    double mgemxifewnFwSDmbh9124359 = -904124747;    double mgemxifewnFwSDmbh91103948 = -836329533;    double mgemxifewnFwSDmbh74036683 = -246557894;    double mgemxifewnFwSDmbh79055056 = -378070962;    double mgemxifewnFwSDmbh96726624 = -34604504;    double mgemxifewnFwSDmbh8550692 = -71290917;    double mgemxifewnFwSDmbh89275864 = -881052260;    double mgemxifewnFwSDmbh35457887 = -22230238;    double mgemxifewnFwSDmbh93571995 = -7211100;    double mgemxifewnFwSDmbh12228985 = -60568912;    double mgemxifewnFwSDmbh72822353 = -894788971;    double mgemxifewnFwSDmbh76593296 = -541097894;    double mgemxifewnFwSDmbh32801380 = -825380508;    double mgemxifewnFwSDmbh80695879 = -648871109;    double mgemxifewnFwSDmbh6953397 = -445072429;    double mgemxifewnFwSDmbh21606266 = -696262206;    double mgemxifewnFwSDmbh28023022 = -283578040;    double mgemxifewnFwSDmbh20611044 = -764996147;    double mgemxifewnFwSDmbh5094908 = -258122334;    double mgemxifewnFwSDmbh70536650 = -982543806;    double mgemxifewnFwSDmbh87502462 = -955879630;    double mgemxifewnFwSDmbh19349709 = -731419367;    double mgemxifewnFwSDmbh58869094 = -28179660;    double mgemxifewnFwSDmbh68558427 = 95387378;    double mgemxifewnFwSDmbh188087 = -398310921;    double mgemxifewnFwSDmbh1434372 = -750011538;    double mgemxifewnFwSDmbh72341926 = -820671708;    double mgemxifewnFwSDmbh47697187 = -333417273;    double mgemxifewnFwSDmbh10642122 = -787562791;    double mgemxifewnFwSDmbh96642990 = -841045354;    double mgemxifewnFwSDmbh32207050 = -538426471;    double mgemxifewnFwSDmbh25899440 = -582007498;    double mgemxifewnFwSDmbh32067305 = -975956183;    double mgemxifewnFwSDmbh91090433 = -366707978;    double mgemxifewnFwSDmbh85444929 = -579467471;    double mgemxifewnFwSDmbh92244621 = -670383258;    double mgemxifewnFwSDmbh2179150 = -558883480;    double mgemxifewnFwSDmbh45293710 = -780985545;    double mgemxifewnFwSDmbh77652433 = -827425935;    double mgemxifewnFwSDmbh10995600 = -504367700;    double mgemxifewnFwSDmbh48832453 = -256162646;    double mgemxifewnFwSDmbh35178499 = -405738077;    double mgemxifewnFwSDmbh39139697 = -248121246;     mgemxifewnFwSDmbh91139352 = mgemxifewnFwSDmbh83368100;     mgemxifewnFwSDmbh83368100 = mgemxifewnFwSDmbh61185760;     mgemxifewnFwSDmbh61185760 = mgemxifewnFwSDmbh30554642;     mgemxifewnFwSDmbh30554642 = mgemxifewnFwSDmbh94372134;     mgemxifewnFwSDmbh94372134 = mgemxifewnFwSDmbh9078364;     mgemxifewnFwSDmbh9078364 = mgemxifewnFwSDmbh87552456;     mgemxifewnFwSDmbh87552456 = mgemxifewnFwSDmbh25334184;     mgemxifewnFwSDmbh25334184 = mgemxifewnFwSDmbh15174227;     mgemxifewnFwSDmbh15174227 = mgemxifewnFwSDmbh74896617;     mgemxifewnFwSDmbh74896617 = mgemxifewnFwSDmbh85395417;     mgemxifewnFwSDmbh85395417 = mgemxifewnFwSDmbh18819120;     mgemxifewnFwSDmbh18819120 = mgemxifewnFwSDmbh86804169;     mgemxifewnFwSDmbh86804169 = mgemxifewnFwSDmbh91154986;     mgemxifewnFwSDmbh91154986 = mgemxifewnFwSDmbh49121065;     mgemxifewnFwSDmbh49121065 = mgemxifewnFwSDmbh75265167;     mgemxifewnFwSDmbh75265167 = mgemxifewnFwSDmbh90301966;     mgemxifewnFwSDmbh90301966 = mgemxifewnFwSDmbh97822098;     mgemxifewnFwSDmbh97822098 = mgemxifewnFwSDmbh52371788;     mgemxifewnFwSDmbh52371788 = mgemxifewnFwSDmbh94990101;     mgemxifewnFwSDmbh94990101 = mgemxifewnFwSDmbh24641199;     mgemxifewnFwSDmbh24641199 = mgemxifewnFwSDmbh8644969;     mgemxifewnFwSDmbh8644969 = mgemxifewnFwSDmbh58138848;     mgemxifewnFwSDmbh58138848 = mgemxifewnFwSDmbh52061401;     mgemxifewnFwSDmbh52061401 = mgemxifewnFwSDmbh39450694;     mgemxifewnFwSDmbh39450694 = mgemxifewnFwSDmbh20335451;     mgemxifewnFwSDmbh20335451 = mgemxifewnFwSDmbh30023307;     mgemxifewnFwSDmbh30023307 = mgemxifewnFwSDmbh90825831;     mgemxifewnFwSDmbh90825831 = mgemxifewnFwSDmbh16783492;     mgemxifewnFwSDmbh16783492 = mgemxifewnFwSDmbh25898362;     mgemxifewnFwSDmbh25898362 = mgemxifewnFwSDmbh39438731;     mgemxifewnFwSDmbh39438731 = mgemxifewnFwSDmbh91823422;     mgemxifewnFwSDmbh91823422 = mgemxifewnFwSDmbh6590135;     mgemxifewnFwSDmbh6590135 = mgemxifewnFwSDmbh13981816;     mgemxifewnFwSDmbh13981816 = mgemxifewnFwSDmbh14561690;     mgemxifewnFwSDmbh14561690 = mgemxifewnFwSDmbh16319685;     mgemxifewnFwSDmbh16319685 = mgemxifewnFwSDmbh94569288;     mgemxifewnFwSDmbh94569288 = mgemxifewnFwSDmbh83348569;     mgemxifewnFwSDmbh83348569 = mgemxifewnFwSDmbh76215833;     mgemxifewnFwSDmbh76215833 = mgemxifewnFwSDmbh24348767;     mgemxifewnFwSDmbh24348767 = mgemxifewnFwSDmbh74379058;     mgemxifewnFwSDmbh74379058 = mgemxifewnFwSDmbh19546291;     mgemxifewnFwSDmbh19546291 = mgemxifewnFwSDmbh38108318;     mgemxifewnFwSDmbh38108318 = mgemxifewnFwSDmbh70636385;     mgemxifewnFwSDmbh70636385 = mgemxifewnFwSDmbh32711693;     mgemxifewnFwSDmbh32711693 = mgemxifewnFwSDmbh80581599;     mgemxifewnFwSDmbh80581599 = mgemxifewnFwSDmbh51777023;     mgemxifewnFwSDmbh51777023 = mgemxifewnFwSDmbh29835221;     mgemxifewnFwSDmbh29835221 = mgemxifewnFwSDmbh89391459;     mgemxifewnFwSDmbh89391459 = mgemxifewnFwSDmbh44441566;     mgemxifewnFwSDmbh44441566 = mgemxifewnFwSDmbh78201175;     mgemxifewnFwSDmbh78201175 = mgemxifewnFwSDmbh28796609;     mgemxifewnFwSDmbh28796609 = mgemxifewnFwSDmbh95180432;     mgemxifewnFwSDmbh95180432 = mgemxifewnFwSDmbh74383084;     mgemxifewnFwSDmbh74383084 = mgemxifewnFwSDmbh88082375;     mgemxifewnFwSDmbh88082375 = mgemxifewnFwSDmbh82494384;     mgemxifewnFwSDmbh82494384 = mgemxifewnFwSDmbh25229252;     mgemxifewnFwSDmbh25229252 = mgemxifewnFwSDmbh9124359;     mgemxifewnFwSDmbh9124359 = mgemxifewnFwSDmbh91103948;     mgemxifewnFwSDmbh91103948 = mgemxifewnFwSDmbh74036683;     mgemxifewnFwSDmbh74036683 = mgemxifewnFwSDmbh79055056;     mgemxifewnFwSDmbh79055056 = mgemxifewnFwSDmbh96726624;     mgemxifewnFwSDmbh96726624 = mgemxifewnFwSDmbh8550692;     mgemxifewnFwSDmbh8550692 = mgemxifewnFwSDmbh89275864;     mgemxifewnFwSDmbh89275864 = mgemxifewnFwSDmbh35457887;     mgemxifewnFwSDmbh35457887 = mgemxifewnFwSDmbh93571995;     mgemxifewnFwSDmbh93571995 = mgemxifewnFwSDmbh12228985;     mgemxifewnFwSDmbh12228985 = mgemxifewnFwSDmbh72822353;     mgemxifewnFwSDmbh72822353 = mgemxifewnFwSDmbh76593296;     mgemxifewnFwSDmbh76593296 = mgemxifewnFwSDmbh32801380;     mgemxifewnFwSDmbh32801380 = mgemxifewnFwSDmbh80695879;     mgemxifewnFwSDmbh80695879 = mgemxifewnFwSDmbh6953397;     mgemxifewnFwSDmbh6953397 = mgemxifewnFwSDmbh21606266;     mgemxifewnFwSDmbh21606266 = mgemxifewnFwSDmbh28023022;     mgemxifewnFwSDmbh28023022 = mgemxifewnFwSDmbh20611044;     mgemxifewnFwSDmbh20611044 = mgemxifewnFwSDmbh5094908;     mgemxifewnFwSDmbh5094908 = mgemxifewnFwSDmbh70536650;     mgemxifewnFwSDmbh70536650 = mgemxifewnFwSDmbh87502462;     mgemxifewnFwSDmbh87502462 = mgemxifewnFwSDmbh19349709;     mgemxifewnFwSDmbh19349709 = mgemxifewnFwSDmbh58869094;     mgemxifewnFwSDmbh58869094 = mgemxifewnFwSDmbh68558427;     mgemxifewnFwSDmbh68558427 = mgemxifewnFwSDmbh188087;     mgemxifewnFwSDmbh188087 = mgemxifewnFwSDmbh1434372;     mgemxifewnFwSDmbh1434372 = mgemxifewnFwSDmbh72341926;     mgemxifewnFwSDmbh72341926 = mgemxifewnFwSDmbh47697187;     mgemxifewnFwSDmbh47697187 = mgemxifewnFwSDmbh10642122;     mgemxifewnFwSDmbh10642122 = mgemxifewnFwSDmbh96642990;     mgemxifewnFwSDmbh96642990 = mgemxifewnFwSDmbh32207050;     mgemxifewnFwSDmbh32207050 = mgemxifewnFwSDmbh25899440;     mgemxifewnFwSDmbh25899440 = mgemxifewnFwSDmbh32067305;     mgemxifewnFwSDmbh32067305 = mgemxifewnFwSDmbh91090433;     mgemxifewnFwSDmbh91090433 = mgemxifewnFwSDmbh85444929;     mgemxifewnFwSDmbh85444929 = mgemxifewnFwSDmbh92244621;     mgemxifewnFwSDmbh92244621 = mgemxifewnFwSDmbh2179150;     mgemxifewnFwSDmbh2179150 = mgemxifewnFwSDmbh45293710;     mgemxifewnFwSDmbh45293710 = mgemxifewnFwSDmbh77652433;     mgemxifewnFwSDmbh77652433 = mgemxifewnFwSDmbh10995600;     mgemxifewnFwSDmbh10995600 = mgemxifewnFwSDmbh48832453;     mgemxifewnFwSDmbh48832453 = mgemxifewnFwSDmbh35178499;     mgemxifewnFwSDmbh35178499 = mgemxifewnFwSDmbh39139697;     mgemxifewnFwSDmbh39139697 = mgemxifewnFwSDmbh91139352;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rrFiHTFdstEFyKfq80015174() {     double pAENuIRIqYHuHwjzm26598321 = -714398146;    double pAENuIRIqYHuHwjzm96640178 = -12050242;    double pAENuIRIqYHuHwjzm98572236 = -80005911;    double pAENuIRIqYHuHwjzm2777997 = -198660190;    double pAENuIRIqYHuHwjzm50657403 = -404396289;    double pAENuIRIqYHuHwjzm45301236 = -478622332;    double pAENuIRIqYHuHwjzm98209862 = -765631958;    double pAENuIRIqYHuHwjzm87465744 = 94497371;    double pAENuIRIqYHuHwjzm16691832 = -896838955;    double pAENuIRIqYHuHwjzm89799329 = -851824850;    double pAENuIRIqYHuHwjzm23577153 = -670472655;    double pAENuIRIqYHuHwjzm19387268 = -449126083;    double pAENuIRIqYHuHwjzm25038815 = -564491353;    double pAENuIRIqYHuHwjzm44295849 = -621780020;    double pAENuIRIqYHuHwjzm7315582 = -343164192;    double pAENuIRIqYHuHwjzm58806468 = -868274934;    double pAENuIRIqYHuHwjzm46137902 = -478759772;    double pAENuIRIqYHuHwjzm97582290 = -675131648;    double pAENuIRIqYHuHwjzm52922118 = -859197967;    double pAENuIRIqYHuHwjzm88827320 = -53587651;    double pAENuIRIqYHuHwjzm37728062 = -691055639;    double pAENuIRIqYHuHwjzm12066493 = -206395006;    double pAENuIRIqYHuHwjzm28664228 = -121480129;    double pAENuIRIqYHuHwjzm45854346 = 78961307;    double pAENuIRIqYHuHwjzm47468645 = -565164784;    double pAENuIRIqYHuHwjzm31033622 = -740816357;    double pAENuIRIqYHuHwjzm90443881 = -644487616;    double pAENuIRIqYHuHwjzm31554257 = -956073895;    double pAENuIRIqYHuHwjzm63451707 = -664072374;    double pAENuIRIqYHuHwjzm31431773 = -561962459;    double pAENuIRIqYHuHwjzm58021962 = -516888101;    double pAENuIRIqYHuHwjzm29646441 = -534225241;    double pAENuIRIqYHuHwjzm18808145 = -572032378;    double pAENuIRIqYHuHwjzm43730242 = -182383691;    double pAENuIRIqYHuHwjzm80370143 = 44581032;    double pAENuIRIqYHuHwjzm67880529 = -228923975;    double pAENuIRIqYHuHwjzm43981105 = -348288258;    double pAENuIRIqYHuHwjzm34563973 = -489368372;    double pAENuIRIqYHuHwjzm6606395 = -556317650;    double pAENuIRIqYHuHwjzm88620298 = -724319053;    double pAENuIRIqYHuHwjzm7830666 = -731229353;    double pAENuIRIqYHuHwjzm8911418 = -270203908;    double pAENuIRIqYHuHwjzm27491696 = -790395692;    double pAENuIRIqYHuHwjzm10767724 = -32300725;    double pAENuIRIqYHuHwjzm32728151 = -605305884;    double pAENuIRIqYHuHwjzm26542928 = -451923559;    double pAENuIRIqYHuHwjzm71122187 = -431613290;    double pAENuIRIqYHuHwjzm47561533 = -506419148;    double pAENuIRIqYHuHwjzm12284126 = -700027063;    double pAENuIRIqYHuHwjzm68971156 = -760332679;    double pAENuIRIqYHuHwjzm58971899 = 31685427;    double pAENuIRIqYHuHwjzm12282082 = -390997170;    double pAENuIRIqYHuHwjzm63801580 = -676254027;    double pAENuIRIqYHuHwjzm90374931 = 22481570;    double pAENuIRIqYHuHwjzm42287112 = -882172526;    double pAENuIRIqYHuHwjzm14531829 = -408003140;    double pAENuIRIqYHuHwjzm67975951 = -890570114;    double pAENuIRIqYHuHwjzm52717890 = -58967218;    double pAENuIRIqYHuHwjzm55309352 = -633495406;    double pAENuIRIqYHuHwjzm19623782 = -663579933;    double pAENuIRIqYHuHwjzm54857354 = -834134717;    double pAENuIRIqYHuHwjzm66655606 = -809558064;    double pAENuIRIqYHuHwjzm24014038 = -241430255;    double pAENuIRIqYHuHwjzm85260059 = -234876497;    double pAENuIRIqYHuHwjzm31777367 = -234936749;    double pAENuIRIqYHuHwjzm93930711 = -36247414;    double pAENuIRIqYHuHwjzm579123 = -877093706;    double pAENuIRIqYHuHwjzm81308572 = -282107662;    double pAENuIRIqYHuHwjzm63925706 = -566361053;    double pAENuIRIqYHuHwjzm39435052 = -14240218;    double pAENuIRIqYHuHwjzm14825364 = -419986676;    double pAENuIRIqYHuHwjzm11573929 = -989391400;    double pAENuIRIqYHuHwjzm90975895 = -18813998;    double pAENuIRIqYHuHwjzm64301819 = -34878915;    double pAENuIRIqYHuHwjzm80996654 = -322358298;    double pAENuIRIqYHuHwjzm28816644 = -320851731;    double pAENuIRIqYHuHwjzm84574797 = -415999315;    double pAENuIRIqYHuHwjzm17896504 = 10820596;    double pAENuIRIqYHuHwjzm13126195 = -315732809;    double pAENuIRIqYHuHwjzm20925717 = -13241226;    double pAENuIRIqYHuHwjzm59911435 = -209203068;    double pAENuIRIqYHuHwjzm42882348 = -38068468;    double pAENuIRIqYHuHwjzm19270131 = -156046832;    double pAENuIRIqYHuHwjzm94480551 = -903739695;    double pAENuIRIqYHuHwjzm72459874 = -493647886;    double pAENuIRIqYHuHwjzm45739881 = -25890932;    double pAENuIRIqYHuHwjzm65844860 = -857971214;    double pAENuIRIqYHuHwjzm28433214 = -494513948;    double pAENuIRIqYHuHwjzm1443130 = -300211166;    double pAENuIRIqYHuHwjzm65838315 = -547415828;    double pAENuIRIqYHuHwjzm99904578 = -338353862;    double pAENuIRIqYHuHwjzm91263214 = -189321041;    double pAENuIRIqYHuHwjzm79254621 = -855872967;    double pAENuIRIqYHuHwjzm86982613 = -892737718;    double pAENuIRIqYHuHwjzm33762945 = -890184337;    double pAENuIRIqYHuHwjzm41175060 = -921671290;    double pAENuIRIqYHuHwjzm84897380 = 71226347;    double pAENuIRIqYHuHwjzm42231637 = -455519196;    double pAENuIRIqYHuHwjzm78990356 = -797363976;    double pAENuIRIqYHuHwjzm38797440 = -714398146;     pAENuIRIqYHuHwjzm26598321 = pAENuIRIqYHuHwjzm96640178;     pAENuIRIqYHuHwjzm96640178 = pAENuIRIqYHuHwjzm98572236;     pAENuIRIqYHuHwjzm98572236 = pAENuIRIqYHuHwjzm2777997;     pAENuIRIqYHuHwjzm2777997 = pAENuIRIqYHuHwjzm50657403;     pAENuIRIqYHuHwjzm50657403 = pAENuIRIqYHuHwjzm45301236;     pAENuIRIqYHuHwjzm45301236 = pAENuIRIqYHuHwjzm98209862;     pAENuIRIqYHuHwjzm98209862 = pAENuIRIqYHuHwjzm87465744;     pAENuIRIqYHuHwjzm87465744 = pAENuIRIqYHuHwjzm16691832;     pAENuIRIqYHuHwjzm16691832 = pAENuIRIqYHuHwjzm89799329;     pAENuIRIqYHuHwjzm89799329 = pAENuIRIqYHuHwjzm23577153;     pAENuIRIqYHuHwjzm23577153 = pAENuIRIqYHuHwjzm19387268;     pAENuIRIqYHuHwjzm19387268 = pAENuIRIqYHuHwjzm25038815;     pAENuIRIqYHuHwjzm25038815 = pAENuIRIqYHuHwjzm44295849;     pAENuIRIqYHuHwjzm44295849 = pAENuIRIqYHuHwjzm7315582;     pAENuIRIqYHuHwjzm7315582 = pAENuIRIqYHuHwjzm58806468;     pAENuIRIqYHuHwjzm58806468 = pAENuIRIqYHuHwjzm46137902;     pAENuIRIqYHuHwjzm46137902 = pAENuIRIqYHuHwjzm97582290;     pAENuIRIqYHuHwjzm97582290 = pAENuIRIqYHuHwjzm52922118;     pAENuIRIqYHuHwjzm52922118 = pAENuIRIqYHuHwjzm88827320;     pAENuIRIqYHuHwjzm88827320 = pAENuIRIqYHuHwjzm37728062;     pAENuIRIqYHuHwjzm37728062 = pAENuIRIqYHuHwjzm12066493;     pAENuIRIqYHuHwjzm12066493 = pAENuIRIqYHuHwjzm28664228;     pAENuIRIqYHuHwjzm28664228 = pAENuIRIqYHuHwjzm45854346;     pAENuIRIqYHuHwjzm45854346 = pAENuIRIqYHuHwjzm47468645;     pAENuIRIqYHuHwjzm47468645 = pAENuIRIqYHuHwjzm31033622;     pAENuIRIqYHuHwjzm31033622 = pAENuIRIqYHuHwjzm90443881;     pAENuIRIqYHuHwjzm90443881 = pAENuIRIqYHuHwjzm31554257;     pAENuIRIqYHuHwjzm31554257 = pAENuIRIqYHuHwjzm63451707;     pAENuIRIqYHuHwjzm63451707 = pAENuIRIqYHuHwjzm31431773;     pAENuIRIqYHuHwjzm31431773 = pAENuIRIqYHuHwjzm58021962;     pAENuIRIqYHuHwjzm58021962 = pAENuIRIqYHuHwjzm29646441;     pAENuIRIqYHuHwjzm29646441 = pAENuIRIqYHuHwjzm18808145;     pAENuIRIqYHuHwjzm18808145 = pAENuIRIqYHuHwjzm43730242;     pAENuIRIqYHuHwjzm43730242 = pAENuIRIqYHuHwjzm80370143;     pAENuIRIqYHuHwjzm80370143 = pAENuIRIqYHuHwjzm67880529;     pAENuIRIqYHuHwjzm67880529 = pAENuIRIqYHuHwjzm43981105;     pAENuIRIqYHuHwjzm43981105 = pAENuIRIqYHuHwjzm34563973;     pAENuIRIqYHuHwjzm34563973 = pAENuIRIqYHuHwjzm6606395;     pAENuIRIqYHuHwjzm6606395 = pAENuIRIqYHuHwjzm88620298;     pAENuIRIqYHuHwjzm88620298 = pAENuIRIqYHuHwjzm7830666;     pAENuIRIqYHuHwjzm7830666 = pAENuIRIqYHuHwjzm8911418;     pAENuIRIqYHuHwjzm8911418 = pAENuIRIqYHuHwjzm27491696;     pAENuIRIqYHuHwjzm27491696 = pAENuIRIqYHuHwjzm10767724;     pAENuIRIqYHuHwjzm10767724 = pAENuIRIqYHuHwjzm32728151;     pAENuIRIqYHuHwjzm32728151 = pAENuIRIqYHuHwjzm26542928;     pAENuIRIqYHuHwjzm26542928 = pAENuIRIqYHuHwjzm71122187;     pAENuIRIqYHuHwjzm71122187 = pAENuIRIqYHuHwjzm47561533;     pAENuIRIqYHuHwjzm47561533 = pAENuIRIqYHuHwjzm12284126;     pAENuIRIqYHuHwjzm12284126 = pAENuIRIqYHuHwjzm68971156;     pAENuIRIqYHuHwjzm68971156 = pAENuIRIqYHuHwjzm58971899;     pAENuIRIqYHuHwjzm58971899 = pAENuIRIqYHuHwjzm12282082;     pAENuIRIqYHuHwjzm12282082 = pAENuIRIqYHuHwjzm63801580;     pAENuIRIqYHuHwjzm63801580 = pAENuIRIqYHuHwjzm90374931;     pAENuIRIqYHuHwjzm90374931 = pAENuIRIqYHuHwjzm42287112;     pAENuIRIqYHuHwjzm42287112 = pAENuIRIqYHuHwjzm14531829;     pAENuIRIqYHuHwjzm14531829 = pAENuIRIqYHuHwjzm67975951;     pAENuIRIqYHuHwjzm67975951 = pAENuIRIqYHuHwjzm52717890;     pAENuIRIqYHuHwjzm52717890 = pAENuIRIqYHuHwjzm55309352;     pAENuIRIqYHuHwjzm55309352 = pAENuIRIqYHuHwjzm19623782;     pAENuIRIqYHuHwjzm19623782 = pAENuIRIqYHuHwjzm54857354;     pAENuIRIqYHuHwjzm54857354 = pAENuIRIqYHuHwjzm66655606;     pAENuIRIqYHuHwjzm66655606 = pAENuIRIqYHuHwjzm24014038;     pAENuIRIqYHuHwjzm24014038 = pAENuIRIqYHuHwjzm85260059;     pAENuIRIqYHuHwjzm85260059 = pAENuIRIqYHuHwjzm31777367;     pAENuIRIqYHuHwjzm31777367 = pAENuIRIqYHuHwjzm93930711;     pAENuIRIqYHuHwjzm93930711 = pAENuIRIqYHuHwjzm579123;     pAENuIRIqYHuHwjzm579123 = pAENuIRIqYHuHwjzm81308572;     pAENuIRIqYHuHwjzm81308572 = pAENuIRIqYHuHwjzm63925706;     pAENuIRIqYHuHwjzm63925706 = pAENuIRIqYHuHwjzm39435052;     pAENuIRIqYHuHwjzm39435052 = pAENuIRIqYHuHwjzm14825364;     pAENuIRIqYHuHwjzm14825364 = pAENuIRIqYHuHwjzm11573929;     pAENuIRIqYHuHwjzm11573929 = pAENuIRIqYHuHwjzm90975895;     pAENuIRIqYHuHwjzm90975895 = pAENuIRIqYHuHwjzm64301819;     pAENuIRIqYHuHwjzm64301819 = pAENuIRIqYHuHwjzm80996654;     pAENuIRIqYHuHwjzm80996654 = pAENuIRIqYHuHwjzm28816644;     pAENuIRIqYHuHwjzm28816644 = pAENuIRIqYHuHwjzm84574797;     pAENuIRIqYHuHwjzm84574797 = pAENuIRIqYHuHwjzm17896504;     pAENuIRIqYHuHwjzm17896504 = pAENuIRIqYHuHwjzm13126195;     pAENuIRIqYHuHwjzm13126195 = pAENuIRIqYHuHwjzm20925717;     pAENuIRIqYHuHwjzm20925717 = pAENuIRIqYHuHwjzm59911435;     pAENuIRIqYHuHwjzm59911435 = pAENuIRIqYHuHwjzm42882348;     pAENuIRIqYHuHwjzm42882348 = pAENuIRIqYHuHwjzm19270131;     pAENuIRIqYHuHwjzm19270131 = pAENuIRIqYHuHwjzm94480551;     pAENuIRIqYHuHwjzm94480551 = pAENuIRIqYHuHwjzm72459874;     pAENuIRIqYHuHwjzm72459874 = pAENuIRIqYHuHwjzm45739881;     pAENuIRIqYHuHwjzm45739881 = pAENuIRIqYHuHwjzm65844860;     pAENuIRIqYHuHwjzm65844860 = pAENuIRIqYHuHwjzm28433214;     pAENuIRIqYHuHwjzm28433214 = pAENuIRIqYHuHwjzm1443130;     pAENuIRIqYHuHwjzm1443130 = pAENuIRIqYHuHwjzm65838315;     pAENuIRIqYHuHwjzm65838315 = pAENuIRIqYHuHwjzm99904578;     pAENuIRIqYHuHwjzm99904578 = pAENuIRIqYHuHwjzm91263214;     pAENuIRIqYHuHwjzm91263214 = pAENuIRIqYHuHwjzm79254621;     pAENuIRIqYHuHwjzm79254621 = pAENuIRIqYHuHwjzm86982613;     pAENuIRIqYHuHwjzm86982613 = pAENuIRIqYHuHwjzm33762945;     pAENuIRIqYHuHwjzm33762945 = pAENuIRIqYHuHwjzm41175060;     pAENuIRIqYHuHwjzm41175060 = pAENuIRIqYHuHwjzm84897380;     pAENuIRIqYHuHwjzm84897380 = pAENuIRIqYHuHwjzm42231637;     pAENuIRIqYHuHwjzm42231637 = pAENuIRIqYHuHwjzm78990356;     pAENuIRIqYHuHwjzm78990356 = pAENuIRIqYHuHwjzm38797440;     pAENuIRIqYHuHwjzm38797440 = pAENuIRIqYHuHwjzm26598321;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wiGMpcifzPDtayTt95064241() {     double gFzIdBmDAzArGKwki32715428 = -826300035;    double gFzIdBmDAzArGKwki40013458 = -821650022;    double gFzIdBmDAzArGKwki14614421 = -230838832;    double gFzIdBmDAzArGKwki29852777 = -389408820;    double gFzIdBmDAzArGKwki19552892 = -925046161;    double gFzIdBmDAzArGKwki36718438 = -743118567;    double gFzIdBmDAzArGKwki58698829 = -904596935;    double gFzIdBmDAzArGKwki37225635 = -192376542;    double gFzIdBmDAzArGKwki81334327 = -664871621;    double gFzIdBmDAzArGKwki31028989 = -80800327;    double gFzIdBmDAzArGKwki83687442 = -547910241;    double gFzIdBmDAzArGKwki51210310 = -893532285;    double gFzIdBmDAzArGKwki11247933 = 95617558;    double gFzIdBmDAzArGKwki27439526 = -54018587;    double gFzIdBmDAzArGKwki3058008 = 66373642;    double gFzIdBmDAzArGKwki311863 = -841274477;    double gFzIdBmDAzArGKwki39973905 = 4695115;    double gFzIdBmDAzArGKwki94635443 = -340654291;    double gFzIdBmDAzArGKwki14751960 = -164967064;    double gFzIdBmDAzArGKwki69377485 = -410490570;    double gFzIdBmDAzArGKwki26281255 = -546543094;    double gFzIdBmDAzArGKwki98587516 = -114324722;    double gFzIdBmDAzArGKwki64255019 = -967997615;    double gFzIdBmDAzArGKwki29846720 = -143343681;    double gFzIdBmDAzArGKwki91168294 = -938360817;    double gFzIdBmDAzArGKwki55755297 = -619471681;    double gFzIdBmDAzArGKwki88903659 = -545149819;    double gFzIdBmDAzArGKwki44661382 = 77997151;    double gFzIdBmDAzArGKwki29583962 = -886455078;    double gFzIdBmDAzArGKwki47410 = -379433326;    double gFzIdBmDAzArGKwki50264512 = -430209039;    double gFzIdBmDAzArGKwki27257063 = -243672057;    double gFzIdBmDAzArGKwki28519094 = -464645748;    double gFzIdBmDAzArGKwki38560806 = 66074544;    double gFzIdBmDAzArGKwki41844795 = -594152073;    double gFzIdBmDAzArGKwki36606442 = -442950082;    double gFzIdBmDAzArGKwki25243040 = -301924128;    double gFzIdBmDAzArGKwki68654185 = -260874382;    double gFzIdBmDAzArGKwki80216648 = -550241147;    double gFzIdBmDAzArGKwki76122236 = -97758276;    double gFzIdBmDAzArGKwki13499890 = -796503247;    double gFzIdBmDAzArGKwki41061875 = -540346186;    double gFzIdBmDAzArGKwki5558675 = -871200276;    double gFzIdBmDAzArGKwki98175464 = -51643663;    double gFzIdBmDAzArGKwki44680616 = -267204039;    double gFzIdBmDAzArGKwki38482123 = -437139676;    double gFzIdBmDAzArGKwki88900761 = -550388827;    double gFzIdBmDAzArGKwki55518474 = -516314681;    double gFzIdBmDAzArGKwki21959855 = -785764051;    double gFzIdBmDAzArGKwki81882808 = -847343487;    double gFzIdBmDAzArGKwki94672577 = -910608244;    double gFzIdBmDAzArGKwki23091801 = -191723490;    double gFzIdBmDAzArGKwki12050038 = -468043615;    double gFzIdBmDAzArGKwki86343811 = -297988575;    double gFzIdBmDAzArGKwki68782987 = -482704309;    double gFzIdBmDAzArGKwki34127911 = -611975313;    double gFzIdBmDAzArGKwki75758438 = -853652407;    double gFzIdBmDAzArGKwki84767700 = 12504849;    double gFzIdBmDAzArGKwki38684483 = -451048004;    double gFzIdBmDAzArGKwki63797594 = -205574481;    double gFzIdBmDAzArGKwki47814779 = -97968749;    double gFzIdBmDAzArGKwki14037447 = -882594087;    double gFzIdBmDAzArGKwki7641673 = -305921464;    double gFzIdBmDAzArGKwki81286917 = -185438295;    double gFzIdBmDAzArGKwki80764477 = -650591288;    double gFzIdBmDAzArGKwki56430380 = -204238185;    double gFzIdBmDAzArGKwki22691217 = -328886538;    double gFzIdBmDAzArGKwki72687127 = -970456986;    double gFzIdBmDAzArGKwki85594731 = -459866515;    double gFzIdBmDAzArGKwki66451566 = -490676276;    double gFzIdBmDAzArGKwki75068823 = -439350350;    double gFzIdBmDAzArGKwki71319719 = -734430504;    double gFzIdBmDAzArGKwki14418795 = -790413144;    double gFzIdBmDAzArGKwki38629724 = 32791211;    double gFzIdBmDAzArGKwki55877595 = -613987324;    double gFzIdBmDAzArGKwki85219380 = 93803091;    double gFzIdBmDAzArGKwki93028842 = -243124447;    double gFzIdBmDAzArGKwki66079555 = -816353953;    double gFzIdBmDAzArGKwki85166103 = -876139643;    double gFzIdBmDAzArGKwki52686172 = -401221142;    double gFzIdBmDAzArGKwki66854536 = 30917146;    double gFzIdBmDAzArGKwki33385186 = 71164861;    double gFzIdBmDAzArGKwki22701527 = -136238798;    double gFzIdBmDAzArGKwki47701154 = 60888408;    double gFzIdBmDAzArGKwki5374833 = -468825083;    double gFzIdBmDAzArGKwki27172711 = -138485550;    double gFzIdBmDAzArGKwki15207025 = -775628442;    double gFzIdBmDAzArGKwki42175282 = -66657173;    double gFzIdBmDAzArGKwki69777819 = -451221148;    double gFzIdBmDAzArGKwki7716884 = -982176761;    double gFzIdBmDAzArGKwki60848004 = -589297675;    double gFzIdBmDAzArGKwki40475339 = -214428977;    double gFzIdBmDAzArGKwki29969703 = -809826379;    double gFzIdBmDAzArGKwki16419054 = -244666667;    double gFzIdBmDAzArGKwki28307458 = -999789527;    double gFzIdBmDAzArGKwki99462442 = -913909161;    double gFzIdBmDAzArGKwki33420202 = -134424722;    double gFzIdBmDAzArGKwki24271757 = -585761981;    double gFzIdBmDAzArGKwki17410987 = -401052375;    double gFzIdBmDAzArGKwki88250236 = -826300035;     gFzIdBmDAzArGKwki32715428 = gFzIdBmDAzArGKwki40013458;     gFzIdBmDAzArGKwki40013458 = gFzIdBmDAzArGKwki14614421;     gFzIdBmDAzArGKwki14614421 = gFzIdBmDAzArGKwki29852777;     gFzIdBmDAzArGKwki29852777 = gFzIdBmDAzArGKwki19552892;     gFzIdBmDAzArGKwki19552892 = gFzIdBmDAzArGKwki36718438;     gFzIdBmDAzArGKwki36718438 = gFzIdBmDAzArGKwki58698829;     gFzIdBmDAzArGKwki58698829 = gFzIdBmDAzArGKwki37225635;     gFzIdBmDAzArGKwki37225635 = gFzIdBmDAzArGKwki81334327;     gFzIdBmDAzArGKwki81334327 = gFzIdBmDAzArGKwki31028989;     gFzIdBmDAzArGKwki31028989 = gFzIdBmDAzArGKwki83687442;     gFzIdBmDAzArGKwki83687442 = gFzIdBmDAzArGKwki51210310;     gFzIdBmDAzArGKwki51210310 = gFzIdBmDAzArGKwki11247933;     gFzIdBmDAzArGKwki11247933 = gFzIdBmDAzArGKwki27439526;     gFzIdBmDAzArGKwki27439526 = gFzIdBmDAzArGKwki3058008;     gFzIdBmDAzArGKwki3058008 = gFzIdBmDAzArGKwki311863;     gFzIdBmDAzArGKwki311863 = gFzIdBmDAzArGKwki39973905;     gFzIdBmDAzArGKwki39973905 = gFzIdBmDAzArGKwki94635443;     gFzIdBmDAzArGKwki94635443 = gFzIdBmDAzArGKwki14751960;     gFzIdBmDAzArGKwki14751960 = gFzIdBmDAzArGKwki69377485;     gFzIdBmDAzArGKwki69377485 = gFzIdBmDAzArGKwki26281255;     gFzIdBmDAzArGKwki26281255 = gFzIdBmDAzArGKwki98587516;     gFzIdBmDAzArGKwki98587516 = gFzIdBmDAzArGKwki64255019;     gFzIdBmDAzArGKwki64255019 = gFzIdBmDAzArGKwki29846720;     gFzIdBmDAzArGKwki29846720 = gFzIdBmDAzArGKwki91168294;     gFzIdBmDAzArGKwki91168294 = gFzIdBmDAzArGKwki55755297;     gFzIdBmDAzArGKwki55755297 = gFzIdBmDAzArGKwki88903659;     gFzIdBmDAzArGKwki88903659 = gFzIdBmDAzArGKwki44661382;     gFzIdBmDAzArGKwki44661382 = gFzIdBmDAzArGKwki29583962;     gFzIdBmDAzArGKwki29583962 = gFzIdBmDAzArGKwki47410;     gFzIdBmDAzArGKwki47410 = gFzIdBmDAzArGKwki50264512;     gFzIdBmDAzArGKwki50264512 = gFzIdBmDAzArGKwki27257063;     gFzIdBmDAzArGKwki27257063 = gFzIdBmDAzArGKwki28519094;     gFzIdBmDAzArGKwki28519094 = gFzIdBmDAzArGKwki38560806;     gFzIdBmDAzArGKwki38560806 = gFzIdBmDAzArGKwki41844795;     gFzIdBmDAzArGKwki41844795 = gFzIdBmDAzArGKwki36606442;     gFzIdBmDAzArGKwki36606442 = gFzIdBmDAzArGKwki25243040;     gFzIdBmDAzArGKwki25243040 = gFzIdBmDAzArGKwki68654185;     gFzIdBmDAzArGKwki68654185 = gFzIdBmDAzArGKwki80216648;     gFzIdBmDAzArGKwki80216648 = gFzIdBmDAzArGKwki76122236;     gFzIdBmDAzArGKwki76122236 = gFzIdBmDAzArGKwki13499890;     gFzIdBmDAzArGKwki13499890 = gFzIdBmDAzArGKwki41061875;     gFzIdBmDAzArGKwki41061875 = gFzIdBmDAzArGKwki5558675;     gFzIdBmDAzArGKwki5558675 = gFzIdBmDAzArGKwki98175464;     gFzIdBmDAzArGKwki98175464 = gFzIdBmDAzArGKwki44680616;     gFzIdBmDAzArGKwki44680616 = gFzIdBmDAzArGKwki38482123;     gFzIdBmDAzArGKwki38482123 = gFzIdBmDAzArGKwki88900761;     gFzIdBmDAzArGKwki88900761 = gFzIdBmDAzArGKwki55518474;     gFzIdBmDAzArGKwki55518474 = gFzIdBmDAzArGKwki21959855;     gFzIdBmDAzArGKwki21959855 = gFzIdBmDAzArGKwki81882808;     gFzIdBmDAzArGKwki81882808 = gFzIdBmDAzArGKwki94672577;     gFzIdBmDAzArGKwki94672577 = gFzIdBmDAzArGKwki23091801;     gFzIdBmDAzArGKwki23091801 = gFzIdBmDAzArGKwki12050038;     gFzIdBmDAzArGKwki12050038 = gFzIdBmDAzArGKwki86343811;     gFzIdBmDAzArGKwki86343811 = gFzIdBmDAzArGKwki68782987;     gFzIdBmDAzArGKwki68782987 = gFzIdBmDAzArGKwki34127911;     gFzIdBmDAzArGKwki34127911 = gFzIdBmDAzArGKwki75758438;     gFzIdBmDAzArGKwki75758438 = gFzIdBmDAzArGKwki84767700;     gFzIdBmDAzArGKwki84767700 = gFzIdBmDAzArGKwki38684483;     gFzIdBmDAzArGKwki38684483 = gFzIdBmDAzArGKwki63797594;     gFzIdBmDAzArGKwki63797594 = gFzIdBmDAzArGKwki47814779;     gFzIdBmDAzArGKwki47814779 = gFzIdBmDAzArGKwki14037447;     gFzIdBmDAzArGKwki14037447 = gFzIdBmDAzArGKwki7641673;     gFzIdBmDAzArGKwki7641673 = gFzIdBmDAzArGKwki81286917;     gFzIdBmDAzArGKwki81286917 = gFzIdBmDAzArGKwki80764477;     gFzIdBmDAzArGKwki80764477 = gFzIdBmDAzArGKwki56430380;     gFzIdBmDAzArGKwki56430380 = gFzIdBmDAzArGKwki22691217;     gFzIdBmDAzArGKwki22691217 = gFzIdBmDAzArGKwki72687127;     gFzIdBmDAzArGKwki72687127 = gFzIdBmDAzArGKwki85594731;     gFzIdBmDAzArGKwki85594731 = gFzIdBmDAzArGKwki66451566;     gFzIdBmDAzArGKwki66451566 = gFzIdBmDAzArGKwki75068823;     gFzIdBmDAzArGKwki75068823 = gFzIdBmDAzArGKwki71319719;     gFzIdBmDAzArGKwki71319719 = gFzIdBmDAzArGKwki14418795;     gFzIdBmDAzArGKwki14418795 = gFzIdBmDAzArGKwki38629724;     gFzIdBmDAzArGKwki38629724 = gFzIdBmDAzArGKwki55877595;     gFzIdBmDAzArGKwki55877595 = gFzIdBmDAzArGKwki85219380;     gFzIdBmDAzArGKwki85219380 = gFzIdBmDAzArGKwki93028842;     gFzIdBmDAzArGKwki93028842 = gFzIdBmDAzArGKwki66079555;     gFzIdBmDAzArGKwki66079555 = gFzIdBmDAzArGKwki85166103;     gFzIdBmDAzArGKwki85166103 = gFzIdBmDAzArGKwki52686172;     gFzIdBmDAzArGKwki52686172 = gFzIdBmDAzArGKwki66854536;     gFzIdBmDAzArGKwki66854536 = gFzIdBmDAzArGKwki33385186;     gFzIdBmDAzArGKwki33385186 = gFzIdBmDAzArGKwki22701527;     gFzIdBmDAzArGKwki22701527 = gFzIdBmDAzArGKwki47701154;     gFzIdBmDAzArGKwki47701154 = gFzIdBmDAzArGKwki5374833;     gFzIdBmDAzArGKwki5374833 = gFzIdBmDAzArGKwki27172711;     gFzIdBmDAzArGKwki27172711 = gFzIdBmDAzArGKwki15207025;     gFzIdBmDAzArGKwki15207025 = gFzIdBmDAzArGKwki42175282;     gFzIdBmDAzArGKwki42175282 = gFzIdBmDAzArGKwki69777819;     gFzIdBmDAzArGKwki69777819 = gFzIdBmDAzArGKwki7716884;     gFzIdBmDAzArGKwki7716884 = gFzIdBmDAzArGKwki60848004;     gFzIdBmDAzArGKwki60848004 = gFzIdBmDAzArGKwki40475339;     gFzIdBmDAzArGKwki40475339 = gFzIdBmDAzArGKwki29969703;     gFzIdBmDAzArGKwki29969703 = gFzIdBmDAzArGKwki16419054;     gFzIdBmDAzArGKwki16419054 = gFzIdBmDAzArGKwki28307458;     gFzIdBmDAzArGKwki28307458 = gFzIdBmDAzArGKwki99462442;     gFzIdBmDAzArGKwki99462442 = gFzIdBmDAzArGKwki33420202;     gFzIdBmDAzArGKwki33420202 = gFzIdBmDAzArGKwki24271757;     gFzIdBmDAzArGKwki24271757 = gFzIdBmDAzArGKwki17410987;     gFzIdBmDAzArGKwki17410987 = gFzIdBmDAzArGKwki88250236;     gFzIdBmDAzArGKwki88250236 = gFzIdBmDAzArGKwki32715428;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void utuEmBoNYiCpSIqv92672268() {     double LHJEcIQADrjPFdkdn35907116 = -489453314;    double LHJEcIQADrjPFdkdn90635417 = -71179870;    double LHJEcIQADrjPFdkdn20037530 = -850775137;    double LHJEcIQADrjPFdkdn45647879 = -210865012;    double LHJEcIQADrjPFdkdn99560744 = -849266768;    double LHJEcIQADrjPFdkdn73650029 = -736924004;    double LHJEcIQADrjPFdkdn40411270 = -269417303;    double LHJEcIQADrjPFdkdn45815951 = -906639094;    double LHJEcIQADrjPFdkdn92240653 = -990545696;    double LHJEcIQADrjPFdkdn55063300 = -309427389;    double LHJEcIQADrjPFdkdn80799742 = -821185799;    double LHJEcIQADrjPFdkdn44916694 = -821974261;    double LHJEcIQADrjPFdkdn52673444 = -171571962;    double LHJEcIQADrjPFdkdn30228711 = -945987176;    double LHJEcIQADrjPFdkdn67763728 = -157407455;    double LHJEcIQADrjPFdkdn38341540 = -128594062;    double LHJEcIQADrjPFdkdn45801429 = -325959429;    double LHJEcIQADrjPFdkdn32775464 = -802260583;    double LHJEcIQADrjPFdkdn55363320 = -330821063;    double LHJEcIQADrjPFdkdn24096431 = -744319944;    double LHJEcIQADrjPFdkdn6076838 = -727221940;    double LHJEcIQADrjPFdkdn14571671 = -539283222;    double LHJEcIQADrjPFdkdn1603891 = -194966971;    double LHJEcIQADrjPFdkdn48711084 = -419120326;    double LHJEcIQADrjPFdkdn38973877 = -210991688;    double LHJEcIQADrjPFdkdn13515517 = -748233927;    double LHJEcIQADrjPFdkdn94423729 = -654176173;    double LHJEcIQADrjPFdkdn38052257 = -972177616;    double LHJEcIQADrjPFdkdn43247061 = -324562124;    double LHJEcIQADrjPFdkdn20302591 = -43478395;    double LHJEcIQADrjPFdkdn61433019 = -514633153;    double LHJEcIQADrjPFdkdn91629661 = -257218297;    double LHJEcIQADrjPFdkdn54119898 = -585067606;    double LHJEcIQADrjPFdkdn33042027 = -642625310;    double LHJEcIQADrjPFdkdn49378457 = -391145487;    double LHJEcIQADrjPFdkdn42533613 = -724915168;    double LHJEcIQADrjPFdkdn93390080 = -49141499;    double LHJEcIQADrjPFdkdn60518451 = -237226414;    double LHJEcIQADrjPFdkdn8145693 = -220214854;    double LHJEcIQADrjPFdkdn36593089 = -40936252;    double LHJEcIQADrjPFdkdn19749468 = -956329669;    double LHJEcIQADrjPFdkdn82260734 = -385268081;    double LHJEcIQADrjPFdkdn9492062 = -471406328;    double LHJEcIQADrjPFdkdn28756522 = -79660056;    double LHJEcIQADrjPFdkdn52696278 = -870341080;    double LHJEcIQADrjPFdkdn37210131 = -589502789;    double LHJEcIQADrjPFdkdn4069611 = -485728057;    double LHJEcIQADrjPFdkdn94416150 = -461723893;    double LHJEcIQADrjPFdkdn96969659 = -499487441;    double LHJEcIQADrjPFdkdn58654481 = -140371268;    double LHJEcIQADrjPFdkdn9198978 = -382745577;    double LHJEcIQADrjPFdkdn91046380 = -321129136;    double LHJEcIQADrjPFdkdn27257587 = -156084942;    double LHJEcIQADrjPFdkdn5206446 = -869291399;    double LHJEcIQADrjPFdkdn86595431 = -573989888;    double LHJEcIQADrjPFdkdn21335446 = -950170092;    double LHJEcIQADrjPFdkdn89031526 = -876212900;    double LHJEcIQADrjPFdkdn71326445 = -331654812;    double LHJEcIQADrjPFdkdn6674003 = -999873325;    double LHJEcIQADrjPFdkdn86045228 = -1032842;    double LHJEcIQADrjPFdkdn79226300 = 17252169;    double LHJEcIQADrjPFdkdn2359014 = -297239688;    double LHJEcIQADrjPFdkdn2568891 = -482076970;    double LHJEcIQADrjPFdkdn71938062 = -847067301;    double LHJEcIQADrjPFdkdn93630280 = -794794236;    double LHJEcIQADrjPFdkdn89170080 = -463967502;    double LHJEcIQADrjPFdkdn90796796 = -136906656;    double LHJEcIQADrjPFdkdn19631417 = -528946652;    double LHJEcIQADrjPFdkdn80850254 = -454841690;    double LHJEcIQADrjPFdkdn25230115 = -432492287;    double LHJEcIQADrjPFdkdn44951460 = 20547436;    double LHJEcIQADrjPFdkdn85282978 = 11266985;    double LHJEcIQADrjPFdkdn24629772 = -482045730;    double LHJEcIQADrjPFdkdn18770232 = -189884812;    double LHJEcIQADrjPFdkdn4346964 = -787990275;    double LHJEcIQADrjPFdkdn23816103 = -241953860;    double LHJEcIQADrjPFdkdn5079609 = 32123105;    double LHJEcIQADrjPFdkdn72847368 = -15306915;    double LHJEcIQADrjPFdkdn96014805 = -548779247;    double LHJEcIQADrjPFdkdn1763746 = -621488900;    double LHJEcIQADrjPFdkdn9445906 = -162505870;    double LHJEcIQADrjPFdkdn7579 = -92452281;    double LHJEcIQADrjPFdkdn41082598 = -372690176;    double LHJEcIQADrjPFdkdn84592580 = -84190857;    double LHJEcIQADrjPFdkdn11103614 = -660732819;    double LHJEcIQADrjPFdkdn70386639 = -93504017;    double LHJEcIQADrjPFdkdn64372075 = -1133356;    double LHJEcIQADrjPFdkdn48913452 = -715776208;    double LHJEcIQADrjPFdkdn46446596 = 31364578;    double LHJEcIQADrjPFdkdn28043011 = -440975395;    double LHJEcIQADrjPFdkdn53502086 = -848702269;    double LHJEcIQADrjPFdkdn22063635 = -717486688;    double LHJEcIQADrjPFdkdn53844448 = -237353090;    double LHJEcIQADrjPFdkdn22100464 = -119182012;    double LHJEcIQADrjPFdkdn57366788 = 41811579;    double LHJEcIQADrjPFdkdn17390454 = -559089982;    double LHJEcIQADrjPFdkdn79691844 = -903191112;    double LHJEcIQADrjPFdkdn37554000 = -624339027;    double LHJEcIQADrjPFdkdn35126242 = -284865820;    double LHJEcIQADrjPFdkdn63526197 = -489453314;     LHJEcIQADrjPFdkdn35907116 = LHJEcIQADrjPFdkdn90635417;     LHJEcIQADrjPFdkdn90635417 = LHJEcIQADrjPFdkdn20037530;     LHJEcIQADrjPFdkdn20037530 = LHJEcIQADrjPFdkdn45647879;     LHJEcIQADrjPFdkdn45647879 = LHJEcIQADrjPFdkdn99560744;     LHJEcIQADrjPFdkdn99560744 = LHJEcIQADrjPFdkdn73650029;     LHJEcIQADrjPFdkdn73650029 = LHJEcIQADrjPFdkdn40411270;     LHJEcIQADrjPFdkdn40411270 = LHJEcIQADrjPFdkdn45815951;     LHJEcIQADrjPFdkdn45815951 = LHJEcIQADrjPFdkdn92240653;     LHJEcIQADrjPFdkdn92240653 = LHJEcIQADrjPFdkdn55063300;     LHJEcIQADrjPFdkdn55063300 = LHJEcIQADrjPFdkdn80799742;     LHJEcIQADrjPFdkdn80799742 = LHJEcIQADrjPFdkdn44916694;     LHJEcIQADrjPFdkdn44916694 = LHJEcIQADrjPFdkdn52673444;     LHJEcIQADrjPFdkdn52673444 = LHJEcIQADrjPFdkdn30228711;     LHJEcIQADrjPFdkdn30228711 = LHJEcIQADrjPFdkdn67763728;     LHJEcIQADrjPFdkdn67763728 = LHJEcIQADrjPFdkdn38341540;     LHJEcIQADrjPFdkdn38341540 = LHJEcIQADrjPFdkdn45801429;     LHJEcIQADrjPFdkdn45801429 = LHJEcIQADrjPFdkdn32775464;     LHJEcIQADrjPFdkdn32775464 = LHJEcIQADrjPFdkdn55363320;     LHJEcIQADrjPFdkdn55363320 = LHJEcIQADrjPFdkdn24096431;     LHJEcIQADrjPFdkdn24096431 = LHJEcIQADrjPFdkdn6076838;     LHJEcIQADrjPFdkdn6076838 = LHJEcIQADrjPFdkdn14571671;     LHJEcIQADrjPFdkdn14571671 = LHJEcIQADrjPFdkdn1603891;     LHJEcIQADrjPFdkdn1603891 = LHJEcIQADrjPFdkdn48711084;     LHJEcIQADrjPFdkdn48711084 = LHJEcIQADrjPFdkdn38973877;     LHJEcIQADrjPFdkdn38973877 = LHJEcIQADrjPFdkdn13515517;     LHJEcIQADrjPFdkdn13515517 = LHJEcIQADrjPFdkdn94423729;     LHJEcIQADrjPFdkdn94423729 = LHJEcIQADrjPFdkdn38052257;     LHJEcIQADrjPFdkdn38052257 = LHJEcIQADrjPFdkdn43247061;     LHJEcIQADrjPFdkdn43247061 = LHJEcIQADrjPFdkdn20302591;     LHJEcIQADrjPFdkdn20302591 = LHJEcIQADrjPFdkdn61433019;     LHJEcIQADrjPFdkdn61433019 = LHJEcIQADrjPFdkdn91629661;     LHJEcIQADrjPFdkdn91629661 = LHJEcIQADrjPFdkdn54119898;     LHJEcIQADrjPFdkdn54119898 = LHJEcIQADrjPFdkdn33042027;     LHJEcIQADrjPFdkdn33042027 = LHJEcIQADrjPFdkdn49378457;     LHJEcIQADrjPFdkdn49378457 = LHJEcIQADrjPFdkdn42533613;     LHJEcIQADrjPFdkdn42533613 = LHJEcIQADrjPFdkdn93390080;     LHJEcIQADrjPFdkdn93390080 = LHJEcIQADrjPFdkdn60518451;     LHJEcIQADrjPFdkdn60518451 = LHJEcIQADrjPFdkdn8145693;     LHJEcIQADrjPFdkdn8145693 = LHJEcIQADrjPFdkdn36593089;     LHJEcIQADrjPFdkdn36593089 = LHJEcIQADrjPFdkdn19749468;     LHJEcIQADrjPFdkdn19749468 = LHJEcIQADrjPFdkdn82260734;     LHJEcIQADrjPFdkdn82260734 = LHJEcIQADrjPFdkdn9492062;     LHJEcIQADrjPFdkdn9492062 = LHJEcIQADrjPFdkdn28756522;     LHJEcIQADrjPFdkdn28756522 = LHJEcIQADrjPFdkdn52696278;     LHJEcIQADrjPFdkdn52696278 = LHJEcIQADrjPFdkdn37210131;     LHJEcIQADrjPFdkdn37210131 = LHJEcIQADrjPFdkdn4069611;     LHJEcIQADrjPFdkdn4069611 = LHJEcIQADrjPFdkdn94416150;     LHJEcIQADrjPFdkdn94416150 = LHJEcIQADrjPFdkdn96969659;     LHJEcIQADrjPFdkdn96969659 = LHJEcIQADrjPFdkdn58654481;     LHJEcIQADrjPFdkdn58654481 = LHJEcIQADrjPFdkdn9198978;     LHJEcIQADrjPFdkdn9198978 = LHJEcIQADrjPFdkdn91046380;     LHJEcIQADrjPFdkdn91046380 = LHJEcIQADrjPFdkdn27257587;     LHJEcIQADrjPFdkdn27257587 = LHJEcIQADrjPFdkdn5206446;     LHJEcIQADrjPFdkdn5206446 = LHJEcIQADrjPFdkdn86595431;     LHJEcIQADrjPFdkdn86595431 = LHJEcIQADrjPFdkdn21335446;     LHJEcIQADrjPFdkdn21335446 = LHJEcIQADrjPFdkdn89031526;     LHJEcIQADrjPFdkdn89031526 = LHJEcIQADrjPFdkdn71326445;     LHJEcIQADrjPFdkdn71326445 = LHJEcIQADrjPFdkdn6674003;     LHJEcIQADrjPFdkdn6674003 = LHJEcIQADrjPFdkdn86045228;     LHJEcIQADrjPFdkdn86045228 = LHJEcIQADrjPFdkdn79226300;     LHJEcIQADrjPFdkdn79226300 = LHJEcIQADrjPFdkdn2359014;     LHJEcIQADrjPFdkdn2359014 = LHJEcIQADrjPFdkdn2568891;     LHJEcIQADrjPFdkdn2568891 = LHJEcIQADrjPFdkdn71938062;     LHJEcIQADrjPFdkdn71938062 = LHJEcIQADrjPFdkdn93630280;     LHJEcIQADrjPFdkdn93630280 = LHJEcIQADrjPFdkdn89170080;     LHJEcIQADrjPFdkdn89170080 = LHJEcIQADrjPFdkdn90796796;     LHJEcIQADrjPFdkdn90796796 = LHJEcIQADrjPFdkdn19631417;     LHJEcIQADrjPFdkdn19631417 = LHJEcIQADrjPFdkdn80850254;     LHJEcIQADrjPFdkdn80850254 = LHJEcIQADrjPFdkdn25230115;     LHJEcIQADrjPFdkdn25230115 = LHJEcIQADrjPFdkdn44951460;     LHJEcIQADrjPFdkdn44951460 = LHJEcIQADrjPFdkdn85282978;     LHJEcIQADrjPFdkdn85282978 = LHJEcIQADrjPFdkdn24629772;     LHJEcIQADrjPFdkdn24629772 = LHJEcIQADrjPFdkdn18770232;     LHJEcIQADrjPFdkdn18770232 = LHJEcIQADrjPFdkdn4346964;     LHJEcIQADrjPFdkdn4346964 = LHJEcIQADrjPFdkdn23816103;     LHJEcIQADrjPFdkdn23816103 = LHJEcIQADrjPFdkdn5079609;     LHJEcIQADrjPFdkdn5079609 = LHJEcIQADrjPFdkdn72847368;     LHJEcIQADrjPFdkdn72847368 = LHJEcIQADrjPFdkdn96014805;     LHJEcIQADrjPFdkdn96014805 = LHJEcIQADrjPFdkdn1763746;     LHJEcIQADrjPFdkdn1763746 = LHJEcIQADrjPFdkdn9445906;     LHJEcIQADrjPFdkdn9445906 = LHJEcIQADrjPFdkdn7579;     LHJEcIQADrjPFdkdn7579 = LHJEcIQADrjPFdkdn41082598;     LHJEcIQADrjPFdkdn41082598 = LHJEcIQADrjPFdkdn84592580;     LHJEcIQADrjPFdkdn84592580 = LHJEcIQADrjPFdkdn11103614;     LHJEcIQADrjPFdkdn11103614 = LHJEcIQADrjPFdkdn70386639;     LHJEcIQADrjPFdkdn70386639 = LHJEcIQADrjPFdkdn64372075;     LHJEcIQADrjPFdkdn64372075 = LHJEcIQADrjPFdkdn48913452;     LHJEcIQADrjPFdkdn48913452 = LHJEcIQADrjPFdkdn46446596;     LHJEcIQADrjPFdkdn46446596 = LHJEcIQADrjPFdkdn28043011;     LHJEcIQADrjPFdkdn28043011 = LHJEcIQADrjPFdkdn53502086;     LHJEcIQADrjPFdkdn53502086 = LHJEcIQADrjPFdkdn22063635;     LHJEcIQADrjPFdkdn22063635 = LHJEcIQADrjPFdkdn53844448;     LHJEcIQADrjPFdkdn53844448 = LHJEcIQADrjPFdkdn22100464;     LHJEcIQADrjPFdkdn22100464 = LHJEcIQADrjPFdkdn57366788;     LHJEcIQADrjPFdkdn57366788 = LHJEcIQADrjPFdkdn17390454;     LHJEcIQADrjPFdkdn17390454 = LHJEcIQADrjPFdkdn79691844;     LHJEcIQADrjPFdkdn79691844 = LHJEcIQADrjPFdkdn37554000;     LHJEcIQADrjPFdkdn37554000 = LHJEcIQADrjPFdkdn35126242;     LHJEcIQADrjPFdkdn35126242 = LHJEcIQADrjPFdkdn63526197;     LHJEcIQADrjPFdkdn63526197 = LHJEcIQADrjPFdkdn35907116;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SYvtaaJOqWniPzmv7721337() {     double fAOxRVgTpzvtCgqpX42024223 = -601355202;    double fAOxRVgTpzvtCgqpX34008697 = -880779650;    double fAOxRVgTpzvtCgqpX36079713 = 98391941;    double fAOxRVgTpzvtCgqpX72722660 = -401613643;    double fAOxRVgTpzvtCgqpX68456233 = -269916639;    double fAOxRVgTpzvtCgqpX65067232 = 98579761;    double fAOxRVgTpzvtCgqpX900237 = -408382280;    double fAOxRVgTpzvtCgqpX95575842 = -93513007;    double fAOxRVgTpzvtCgqpX56883148 = -758578362;    double fAOxRVgTpzvtCgqpX96292959 = -638402866;    double fAOxRVgTpzvtCgqpX40910033 = -698623386;    double fAOxRVgTpzvtCgqpX76739737 = -166380463;    double fAOxRVgTpzvtCgqpX38882563 = -611463050;    double fAOxRVgTpzvtCgqpX13372388 = -378225743;    double fAOxRVgTpzvtCgqpX63506155 = -847869621;    double fAOxRVgTpzvtCgqpX79846933 = -101593605;    double fAOxRVgTpzvtCgqpX39637432 = -942504542;    double fAOxRVgTpzvtCgqpX29828617 = -467783225;    double fAOxRVgTpzvtCgqpX17193162 = -736590160;    double fAOxRVgTpzvtCgqpX4646596 = -1222863;    double fAOxRVgTpzvtCgqpX94630030 = -582709396;    double fAOxRVgTpzvtCgqpX1092695 = -447212938;    double fAOxRVgTpzvtCgqpX37194683 = 58515543;    double fAOxRVgTpzvtCgqpX32703458 = -641425314;    double fAOxRVgTpzvtCgqpX82673526 = -584187721;    double fAOxRVgTpzvtCgqpX38237192 = -626889250;    double fAOxRVgTpzvtCgqpX92883507 = -554838376;    double fAOxRVgTpzvtCgqpX51159382 = 61893430;    double fAOxRVgTpzvtCgqpX9379317 = -546944829;    double fAOxRVgTpzvtCgqpX88918227 = -960949263;    double fAOxRVgTpzvtCgqpX53675569 = -427954091;    double fAOxRVgTpzvtCgqpX89240283 = 33334887;    double fAOxRVgTpzvtCgqpX63830847 = -477680976;    double fAOxRVgTpzvtCgqpX27872592 = -394167076;    double fAOxRVgTpzvtCgqpX10853109 = 70121408;    double fAOxRVgTpzvtCgqpX11259527 = -938941275;    double fAOxRVgTpzvtCgqpX74652015 = -2777369;    double fAOxRVgTpzvtCgqpX94608663 = -8732424;    double fAOxRVgTpzvtCgqpX81755946 = -214138350;    double fAOxRVgTpzvtCgqpX24095026 = -514375474;    double fAOxRVgTpzvtCgqpX25418692 = 78396436;    double fAOxRVgTpzvtCgqpX14411192 = -655410359;    double fAOxRVgTpzvtCgqpX87559040 = -552210912;    double fAOxRVgTpzvtCgqpX16164263 = -99002994;    double fAOxRVgTpzvtCgqpX64648743 = -532239234;    double fAOxRVgTpzvtCgqpX49149326 = -574718905;    double fAOxRVgTpzvtCgqpX21848185 = -604503595;    double fAOxRVgTpzvtCgqpX2373092 = -471619425;    double fAOxRVgTpzvtCgqpX6645389 = -585224429;    double fAOxRVgTpzvtCgqpX71566133 = -227382076;    double fAOxRVgTpzvtCgqpX44899656 = -225039248;    double fAOxRVgTpzvtCgqpX1856101 = -121855456;    double fAOxRVgTpzvtCgqpX75506044 = 52125470;    double fAOxRVgTpzvtCgqpX1175327 = -89761544;    double fAOxRVgTpzvtCgqpX13091306 = -174521671;    double fAOxRVgTpzvtCgqpX40931529 = -54142265;    double fAOxRVgTpzvtCgqpX96814014 = -839295193;    double fAOxRVgTpzvtCgqpX3376256 = -260182745;    double fAOxRVgTpzvtCgqpX90049133 = -817425922;    double fAOxRVgTpzvtCgqpX30219042 = -643027390;    double fAOxRVgTpzvtCgqpX72183724 = -346581863;    double fAOxRVgTpzvtCgqpX49740854 = -370275711;    double fAOxRVgTpzvtCgqpX86196526 = -546568178;    double fAOxRVgTpzvtCgqpX67964921 = -797629100;    double fAOxRVgTpzvtCgqpX42617390 = -110448776;    double fAOxRVgTpzvtCgqpX51669749 = -631958273;    double fAOxRVgTpzvtCgqpX12908890 = -688699487;    double fAOxRVgTpzvtCgqpX11009972 = -117295975;    double fAOxRVgTpzvtCgqpX2519279 = -348347152;    double fAOxRVgTpzvtCgqpX52246628 = -908928346;    double fAOxRVgTpzvtCgqpX5194919 = 1183763;    double fAOxRVgTpzvtCgqpX45028769 = -833772119;    double fAOxRVgTpzvtCgqpX48072671 = -153644875;    double fAOxRVgTpzvtCgqpX93098135 = -122214686;    double fAOxRVgTpzvtCgqpX79227904 = 20380700;    double fAOxRVgTpzvtCgqpX80218839 = -927299037;    double fAOxRVgTpzvtCgqpX13533655 = -895002027;    double fAOxRVgTpzvtCgqpX21030420 = -842481464;    double fAOxRVgTpzvtCgqpX68054714 = -9186080;    double fAOxRVgTpzvtCgqpX33524201 = 90531183;    double fAOxRVgTpzvtCgqpX16389007 = 77614344;    double fAOxRVgTpzvtCgqpX90510415 = 16781048;    double fAOxRVgTpzvtCgqpX44513994 = -352882141;    double fAOxRVgTpzvtCgqpX37813183 = -219562754;    double fAOxRVgTpzvtCgqpX44018572 = -635910015;    double fAOxRVgTpzvtCgqpX51819469 = -206098635;    double fAOxRVgTpzvtCgqpX13734240 = 81209416;    double fAOxRVgTpzvtCgqpX62655521 = -287919433;    double fAOxRVgTpzvtCgqpX14781286 = -119645405;    double fAOxRVgTpzvtCgqpX69921580 = -875736328;    double fAOxRVgTpzvtCgqpX14445512 = 353917;    double fAOxRVgTpzvtCgqpX71275759 = -742594624;    double fAOxRVgTpzvtCgqpX4559531 = -191306502;    double fAOxRVgTpzvtCgqpX51536905 = -571110961;    double fAOxRVgTpzvtCgqpX51911302 = -67793611;    double fAOxRVgTpzvtCgqpX75677837 = -551327853;    double fAOxRVgTpzvtCgqpX28214666 = -8842181;    double fAOxRVgTpzvtCgqpX19594120 = -754581812;    double fAOxRVgTpzvtCgqpX73546872 = -988554219;    double fAOxRVgTpzvtCgqpX12978994 = -601355202;     fAOxRVgTpzvtCgqpX42024223 = fAOxRVgTpzvtCgqpX34008697;     fAOxRVgTpzvtCgqpX34008697 = fAOxRVgTpzvtCgqpX36079713;     fAOxRVgTpzvtCgqpX36079713 = fAOxRVgTpzvtCgqpX72722660;     fAOxRVgTpzvtCgqpX72722660 = fAOxRVgTpzvtCgqpX68456233;     fAOxRVgTpzvtCgqpX68456233 = fAOxRVgTpzvtCgqpX65067232;     fAOxRVgTpzvtCgqpX65067232 = fAOxRVgTpzvtCgqpX900237;     fAOxRVgTpzvtCgqpX900237 = fAOxRVgTpzvtCgqpX95575842;     fAOxRVgTpzvtCgqpX95575842 = fAOxRVgTpzvtCgqpX56883148;     fAOxRVgTpzvtCgqpX56883148 = fAOxRVgTpzvtCgqpX96292959;     fAOxRVgTpzvtCgqpX96292959 = fAOxRVgTpzvtCgqpX40910033;     fAOxRVgTpzvtCgqpX40910033 = fAOxRVgTpzvtCgqpX76739737;     fAOxRVgTpzvtCgqpX76739737 = fAOxRVgTpzvtCgqpX38882563;     fAOxRVgTpzvtCgqpX38882563 = fAOxRVgTpzvtCgqpX13372388;     fAOxRVgTpzvtCgqpX13372388 = fAOxRVgTpzvtCgqpX63506155;     fAOxRVgTpzvtCgqpX63506155 = fAOxRVgTpzvtCgqpX79846933;     fAOxRVgTpzvtCgqpX79846933 = fAOxRVgTpzvtCgqpX39637432;     fAOxRVgTpzvtCgqpX39637432 = fAOxRVgTpzvtCgqpX29828617;     fAOxRVgTpzvtCgqpX29828617 = fAOxRVgTpzvtCgqpX17193162;     fAOxRVgTpzvtCgqpX17193162 = fAOxRVgTpzvtCgqpX4646596;     fAOxRVgTpzvtCgqpX4646596 = fAOxRVgTpzvtCgqpX94630030;     fAOxRVgTpzvtCgqpX94630030 = fAOxRVgTpzvtCgqpX1092695;     fAOxRVgTpzvtCgqpX1092695 = fAOxRVgTpzvtCgqpX37194683;     fAOxRVgTpzvtCgqpX37194683 = fAOxRVgTpzvtCgqpX32703458;     fAOxRVgTpzvtCgqpX32703458 = fAOxRVgTpzvtCgqpX82673526;     fAOxRVgTpzvtCgqpX82673526 = fAOxRVgTpzvtCgqpX38237192;     fAOxRVgTpzvtCgqpX38237192 = fAOxRVgTpzvtCgqpX92883507;     fAOxRVgTpzvtCgqpX92883507 = fAOxRVgTpzvtCgqpX51159382;     fAOxRVgTpzvtCgqpX51159382 = fAOxRVgTpzvtCgqpX9379317;     fAOxRVgTpzvtCgqpX9379317 = fAOxRVgTpzvtCgqpX88918227;     fAOxRVgTpzvtCgqpX88918227 = fAOxRVgTpzvtCgqpX53675569;     fAOxRVgTpzvtCgqpX53675569 = fAOxRVgTpzvtCgqpX89240283;     fAOxRVgTpzvtCgqpX89240283 = fAOxRVgTpzvtCgqpX63830847;     fAOxRVgTpzvtCgqpX63830847 = fAOxRVgTpzvtCgqpX27872592;     fAOxRVgTpzvtCgqpX27872592 = fAOxRVgTpzvtCgqpX10853109;     fAOxRVgTpzvtCgqpX10853109 = fAOxRVgTpzvtCgqpX11259527;     fAOxRVgTpzvtCgqpX11259527 = fAOxRVgTpzvtCgqpX74652015;     fAOxRVgTpzvtCgqpX74652015 = fAOxRVgTpzvtCgqpX94608663;     fAOxRVgTpzvtCgqpX94608663 = fAOxRVgTpzvtCgqpX81755946;     fAOxRVgTpzvtCgqpX81755946 = fAOxRVgTpzvtCgqpX24095026;     fAOxRVgTpzvtCgqpX24095026 = fAOxRVgTpzvtCgqpX25418692;     fAOxRVgTpzvtCgqpX25418692 = fAOxRVgTpzvtCgqpX14411192;     fAOxRVgTpzvtCgqpX14411192 = fAOxRVgTpzvtCgqpX87559040;     fAOxRVgTpzvtCgqpX87559040 = fAOxRVgTpzvtCgqpX16164263;     fAOxRVgTpzvtCgqpX16164263 = fAOxRVgTpzvtCgqpX64648743;     fAOxRVgTpzvtCgqpX64648743 = fAOxRVgTpzvtCgqpX49149326;     fAOxRVgTpzvtCgqpX49149326 = fAOxRVgTpzvtCgqpX21848185;     fAOxRVgTpzvtCgqpX21848185 = fAOxRVgTpzvtCgqpX2373092;     fAOxRVgTpzvtCgqpX2373092 = fAOxRVgTpzvtCgqpX6645389;     fAOxRVgTpzvtCgqpX6645389 = fAOxRVgTpzvtCgqpX71566133;     fAOxRVgTpzvtCgqpX71566133 = fAOxRVgTpzvtCgqpX44899656;     fAOxRVgTpzvtCgqpX44899656 = fAOxRVgTpzvtCgqpX1856101;     fAOxRVgTpzvtCgqpX1856101 = fAOxRVgTpzvtCgqpX75506044;     fAOxRVgTpzvtCgqpX75506044 = fAOxRVgTpzvtCgqpX1175327;     fAOxRVgTpzvtCgqpX1175327 = fAOxRVgTpzvtCgqpX13091306;     fAOxRVgTpzvtCgqpX13091306 = fAOxRVgTpzvtCgqpX40931529;     fAOxRVgTpzvtCgqpX40931529 = fAOxRVgTpzvtCgqpX96814014;     fAOxRVgTpzvtCgqpX96814014 = fAOxRVgTpzvtCgqpX3376256;     fAOxRVgTpzvtCgqpX3376256 = fAOxRVgTpzvtCgqpX90049133;     fAOxRVgTpzvtCgqpX90049133 = fAOxRVgTpzvtCgqpX30219042;     fAOxRVgTpzvtCgqpX30219042 = fAOxRVgTpzvtCgqpX72183724;     fAOxRVgTpzvtCgqpX72183724 = fAOxRVgTpzvtCgqpX49740854;     fAOxRVgTpzvtCgqpX49740854 = fAOxRVgTpzvtCgqpX86196526;     fAOxRVgTpzvtCgqpX86196526 = fAOxRVgTpzvtCgqpX67964921;     fAOxRVgTpzvtCgqpX67964921 = fAOxRVgTpzvtCgqpX42617390;     fAOxRVgTpzvtCgqpX42617390 = fAOxRVgTpzvtCgqpX51669749;     fAOxRVgTpzvtCgqpX51669749 = fAOxRVgTpzvtCgqpX12908890;     fAOxRVgTpzvtCgqpX12908890 = fAOxRVgTpzvtCgqpX11009972;     fAOxRVgTpzvtCgqpX11009972 = fAOxRVgTpzvtCgqpX2519279;     fAOxRVgTpzvtCgqpX2519279 = fAOxRVgTpzvtCgqpX52246628;     fAOxRVgTpzvtCgqpX52246628 = fAOxRVgTpzvtCgqpX5194919;     fAOxRVgTpzvtCgqpX5194919 = fAOxRVgTpzvtCgqpX45028769;     fAOxRVgTpzvtCgqpX45028769 = fAOxRVgTpzvtCgqpX48072671;     fAOxRVgTpzvtCgqpX48072671 = fAOxRVgTpzvtCgqpX93098135;     fAOxRVgTpzvtCgqpX93098135 = fAOxRVgTpzvtCgqpX79227904;     fAOxRVgTpzvtCgqpX79227904 = fAOxRVgTpzvtCgqpX80218839;     fAOxRVgTpzvtCgqpX80218839 = fAOxRVgTpzvtCgqpX13533655;     fAOxRVgTpzvtCgqpX13533655 = fAOxRVgTpzvtCgqpX21030420;     fAOxRVgTpzvtCgqpX21030420 = fAOxRVgTpzvtCgqpX68054714;     fAOxRVgTpzvtCgqpX68054714 = fAOxRVgTpzvtCgqpX33524201;     fAOxRVgTpzvtCgqpX33524201 = fAOxRVgTpzvtCgqpX16389007;     fAOxRVgTpzvtCgqpX16389007 = fAOxRVgTpzvtCgqpX90510415;     fAOxRVgTpzvtCgqpX90510415 = fAOxRVgTpzvtCgqpX44513994;     fAOxRVgTpzvtCgqpX44513994 = fAOxRVgTpzvtCgqpX37813183;     fAOxRVgTpzvtCgqpX37813183 = fAOxRVgTpzvtCgqpX44018572;     fAOxRVgTpzvtCgqpX44018572 = fAOxRVgTpzvtCgqpX51819469;     fAOxRVgTpzvtCgqpX51819469 = fAOxRVgTpzvtCgqpX13734240;     fAOxRVgTpzvtCgqpX13734240 = fAOxRVgTpzvtCgqpX62655521;     fAOxRVgTpzvtCgqpX62655521 = fAOxRVgTpzvtCgqpX14781286;     fAOxRVgTpzvtCgqpX14781286 = fAOxRVgTpzvtCgqpX69921580;     fAOxRVgTpzvtCgqpX69921580 = fAOxRVgTpzvtCgqpX14445512;     fAOxRVgTpzvtCgqpX14445512 = fAOxRVgTpzvtCgqpX71275759;     fAOxRVgTpzvtCgqpX71275759 = fAOxRVgTpzvtCgqpX4559531;     fAOxRVgTpzvtCgqpX4559531 = fAOxRVgTpzvtCgqpX51536905;     fAOxRVgTpzvtCgqpX51536905 = fAOxRVgTpzvtCgqpX51911302;     fAOxRVgTpzvtCgqpX51911302 = fAOxRVgTpzvtCgqpX75677837;     fAOxRVgTpzvtCgqpX75677837 = fAOxRVgTpzvtCgqpX28214666;     fAOxRVgTpzvtCgqpX28214666 = fAOxRVgTpzvtCgqpX19594120;     fAOxRVgTpzvtCgqpX19594120 = fAOxRVgTpzvtCgqpX73546872;     fAOxRVgTpzvtCgqpX73546872 = fAOxRVgTpzvtCgqpX12978994;     fAOxRVgTpzvtCgqpX12978994 = fAOxRVgTpzvtCgqpX42024223;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CvVTCDXsJggrzWZy75012935() {     double urhyZIQyjmtmaqqXE77483191 = 32367898;    double urhyZIQyjmtmaqqXE47280775 = -761331257;    double urhyZIQyjmtmaqqXE73466189 = -667275134;    double urhyZIQyjmtmaqqXE44946015 = -222764714;    double urhyZIQyjmtmaqqXE24741503 = -375515485;    double urhyZIQyjmtmaqqXE1290105 = -163768134;    double urhyZIQyjmtmaqqXE11557643 = -253108014;    double urhyZIQyjmtmaqqXE57707403 = -837747147;    double urhyZIQyjmtmaqqXE58400754 = -146909769;    double urhyZIQyjmtmaqqXE11195672 = -303089864;    double urhyZIQyjmtmaqqXE79091767 = -500631115;    double urhyZIQyjmtmaqqXE77307885 = -30501235;    double urhyZIQyjmtmaqqXE77117208 = -63475555;    double urhyZIQyjmtmaqqXE66513251 = -354589153;    double urhyZIQyjmtmaqqXE21700671 = -251294636;    double urhyZIQyjmtmaqqXE63388234 = -837405213;    double urhyZIQyjmtmaqqXE95473367 = -369479095;    double urhyZIQyjmtmaqqXE29588809 = -541211294;    double urhyZIQyjmtmaqqXE17743492 = 46846419;    double urhyZIQyjmtmaqqXE98483814 = -427783929;    double urhyZIQyjmtmaqqXE7716894 = -239984085;    double urhyZIQyjmtmaqqXE4514220 = -533849232;    double urhyZIQyjmtmaqqXE7720063 = -679116642;    double urhyZIQyjmtmaqqXE26496403 = -52249918;    double urhyZIQyjmtmaqqXE90691478 = -608172919;    double urhyZIQyjmtmaqqXE48935363 = -315466057;    double urhyZIQyjmtmaqqXE53304082 = -361122517;    double urhyZIQyjmtmaqqXE91887807 = -107878744;    double urhyZIQyjmtmaqqXE56047531 = -21039631;    double urhyZIQyjmtmaqqXE94451638 = -995456433;    double urhyZIQyjmtmaqqXE72258800 = -979934578;    double urhyZIQyjmtmaqqXE27063302 = -839636527;    double urhyZIQyjmtmaqqXE76048857 = -625276954;    double urhyZIQyjmtmaqqXE57621018 = -458860889;    double urhyZIQyjmtmaqqXE76661562 = -980978843;    double urhyZIQyjmtmaqqXE62820370 = -53506581;    double urhyZIQyjmtmaqqXE24063832 = -967473409;    double urhyZIQyjmtmaqqXE45824067 = 8611995;    double urhyZIQyjmtmaqqXE12146508 = -965014627;    double urhyZIQyjmtmaqqXE88366558 = 20361980;    double urhyZIQyjmtmaqqXE58870299 = -790802478;    double urhyZIQyjmtmaqqXE3776319 = -249955650;    double urhyZIQyjmtmaqqXE76942418 = -105391697;    double urhyZIQyjmtmaqqXE56295601 = -703335404;    double urhyZIQyjmtmaqqXE64665202 = -358750396;    double urhyZIQyjmtmaqqXE95110654 = -613642537;    double urhyZIQyjmtmaqqXE41193349 = 11510045;    double urhyZIQyjmtmaqqXE20099404 = -638146018;    double urhyZIQyjmtmaqqXE29538054 = -248961309;    double urhyZIQyjmtmaqqXE96095722 = -718408892;    double urhyZIQyjmtmaqqXE25670380 = -99315805;    double urhyZIQyjmtmaqqXE85341572 = -335507803;    double urhyZIQyjmtmaqqXE44127193 = -803920084;    double urhyZIQyjmtmaqqXE17167173 = -281270044;    double urhyZIQyjmtmaqqXE67296042 = -521011817;    double urhyZIQyjmtmaqqXE72968972 = -433782871;    double urhyZIQyjmtmaqqXE39560713 = 17785384;    double urhyZIQyjmtmaqqXE46969787 = -515025216;    double urhyZIQyjmtmaqqXE54254537 = -614591795;    double urhyZIQyjmtmaqqXE75806139 = 39950571;    double urhyZIQyjmtmaqqXE47986022 = -802645618;    double urhyZIQyjmtmaqqXE19669836 = -45229271;    double urhyZIQyjmtmaqqXE1659873 = -716707516;    double urhyZIQyjmtmaqqXE63949115 = -151453336;    double urhyZIQyjmtmaqqXE38936871 = -323155287;    double urhyZIQyjmtmaqqXE52028465 = -660994588;    double urhyZIQyjmtmaqqXE1259028 = -405224281;    double urhyZIQyjmtmaqqXE19496190 = -604614667;    double urhyZIQyjmtmaqqXE89851688 = -373610311;    double urhyZIQyjmtmaqqXE58880301 = -97788055;    double urhyZIQyjmtmaqqXE39324403 = -869931804;    double urhyZIQyjmtmaqqXE49649301 = -278091090;    double urhyZIQyjmtmaqqXE17442301 = -576196668;    double urhyZIQyjmtmaqqXE29376933 = -973515562;    double urhyZIQyjmtmaqqXE39613515 = -636981451;    double urhyZIQyjmtmaqqXE3940576 = -990028435;    double urhyZIQyjmtmaqqXE27571801 = -328457535;    double urhyZIQyjmtmaqqXE51424461 = -975781238;    double urhyZIQyjmtmaqqXE61831201 = -693499523;    double urhyZIQyjmtmaqqXE95580823 = -994530383;    double urhyZIQyjmtmaqqXE7742015 = -226976102;    double urhyZIQyjmtmaqqXE33204678 = -722976499;    double urhyZIQyjmtmaqqXE62349753 = -858917435;    double urhyZIQyjmtmaqqXE59951808 = -302630740;    double urhyZIQyjmtmaqqXE68781259 = -796140628;    double urhyZIQyjmtmaqqXE86917228 = -544426776;    double urhyZIQyjmtmaqqXE82936109 = 64283556;    double urhyZIQyjmtmaqqXE58881685 = -244006910;    double urhyZIQyjmtmaqqXE90324975 = -937849073;    double urhyZIQyjmtmaqqXE3692590 = -447195973;    double urhyZIQyjmtmaqqXE23259657 = 28708034;    double urhyZIQyjmtmaqqXE77094044 = -352448193;    double urhyZIQyjmtmaqqXE91569530 = -376796210;    double urhyZIQyjmtmaqqXE36340369 = -904965199;    double urhyZIQyjmtmaqqXE40380536 = -176992403;    double urhyZIQyjmtmaqqXE39200464 = -645573207;    double urhyZIQyjmtmaqqXE2116446 = -533248134;    double urhyZIQyjmtmaqqXE12993304 = -953938362;    double urhyZIQyjmtmaqqXE17358730 = -280180118;    double urhyZIQyjmtmaqqXE12636737 = 32367898;     urhyZIQyjmtmaqqXE77483191 = urhyZIQyjmtmaqqXE47280775;     urhyZIQyjmtmaqqXE47280775 = urhyZIQyjmtmaqqXE73466189;     urhyZIQyjmtmaqqXE73466189 = urhyZIQyjmtmaqqXE44946015;     urhyZIQyjmtmaqqXE44946015 = urhyZIQyjmtmaqqXE24741503;     urhyZIQyjmtmaqqXE24741503 = urhyZIQyjmtmaqqXE1290105;     urhyZIQyjmtmaqqXE1290105 = urhyZIQyjmtmaqqXE11557643;     urhyZIQyjmtmaqqXE11557643 = urhyZIQyjmtmaqqXE57707403;     urhyZIQyjmtmaqqXE57707403 = urhyZIQyjmtmaqqXE58400754;     urhyZIQyjmtmaqqXE58400754 = urhyZIQyjmtmaqqXE11195672;     urhyZIQyjmtmaqqXE11195672 = urhyZIQyjmtmaqqXE79091767;     urhyZIQyjmtmaqqXE79091767 = urhyZIQyjmtmaqqXE77307885;     urhyZIQyjmtmaqqXE77307885 = urhyZIQyjmtmaqqXE77117208;     urhyZIQyjmtmaqqXE77117208 = urhyZIQyjmtmaqqXE66513251;     urhyZIQyjmtmaqqXE66513251 = urhyZIQyjmtmaqqXE21700671;     urhyZIQyjmtmaqqXE21700671 = urhyZIQyjmtmaqqXE63388234;     urhyZIQyjmtmaqqXE63388234 = urhyZIQyjmtmaqqXE95473367;     urhyZIQyjmtmaqqXE95473367 = urhyZIQyjmtmaqqXE29588809;     urhyZIQyjmtmaqqXE29588809 = urhyZIQyjmtmaqqXE17743492;     urhyZIQyjmtmaqqXE17743492 = urhyZIQyjmtmaqqXE98483814;     urhyZIQyjmtmaqqXE98483814 = urhyZIQyjmtmaqqXE7716894;     urhyZIQyjmtmaqqXE7716894 = urhyZIQyjmtmaqqXE4514220;     urhyZIQyjmtmaqqXE4514220 = urhyZIQyjmtmaqqXE7720063;     urhyZIQyjmtmaqqXE7720063 = urhyZIQyjmtmaqqXE26496403;     urhyZIQyjmtmaqqXE26496403 = urhyZIQyjmtmaqqXE90691478;     urhyZIQyjmtmaqqXE90691478 = urhyZIQyjmtmaqqXE48935363;     urhyZIQyjmtmaqqXE48935363 = urhyZIQyjmtmaqqXE53304082;     urhyZIQyjmtmaqqXE53304082 = urhyZIQyjmtmaqqXE91887807;     urhyZIQyjmtmaqqXE91887807 = urhyZIQyjmtmaqqXE56047531;     urhyZIQyjmtmaqqXE56047531 = urhyZIQyjmtmaqqXE94451638;     urhyZIQyjmtmaqqXE94451638 = urhyZIQyjmtmaqqXE72258800;     urhyZIQyjmtmaqqXE72258800 = urhyZIQyjmtmaqqXE27063302;     urhyZIQyjmtmaqqXE27063302 = urhyZIQyjmtmaqqXE76048857;     urhyZIQyjmtmaqqXE76048857 = urhyZIQyjmtmaqqXE57621018;     urhyZIQyjmtmaqqXE57621018 = urhyZIQyjmtmaqqXE76661562;     urhyZIQyjmtmaqqXE76661562 = urhyZIQyjmtmaqqXE62820370;     urhyZIQyjmtmaqqXE62820370 = urhyZIQyjmtmaqqXE24063832;     urhyZIQyjmtmaqqXE24063832 = urhyZIQyjmtmaqqXE45824067;     urhyZIQyjmtmaqqXE45824067 = urhyZIQyjmtmaqqXE12146508;     urhyZIQyjmtmaqqXE12146508 = urhyZIQyjmtmaqqXE88366558;     urhyZIQyjmtmaqqXE88366558 = urhyZIQyjmtmaqqXE58870299;     urhyZIQyjmtmaqqXE58870299 = urhyZIQyjmtmaqqXE3776319;     urhyZIQyjmtmaqqXE3776319 = urhyZIQyjmtmaqqXE76942418;     urhyZIQyjmtmaqqXE76942418 = urhyZIQyjmtmaqqXE56295601;     urhyZIQyjmtmaqqXE56295601 = urhyZIQyjmtmaqqXE64665202;     urhyZIQyjmtmaqqXE64665202 = urhyZIQyjmtmaqqXE95110654;     urhyZIQyjmtmaqqXE95110654 = urhyZIQyjmtmaqqXE41193349;     urhyZIQyjmtmaqqXE41193349 = urhyZIQyjmtmaqqXE20099404;     urhyZIQyjmtmaqqXE20099404 = urhyZIQyjmtmaqqXE29538054;     urhyZIQyjmtmaqqXE29538054 = urhyZIQyjmtmaqqXE96095722;     urhyZIQyjmtmaqqXE96095722 = urhyZIQyjmtmaqqXE25670380;     urhyZIQyjmtmaqqXE25670380 = urhyZIQyjmtmaqqXE85341572;     urhyZIQyjmtmaqqXE85341572 = urhyZIQyjmtmaqqXE44127193;     urhyZIQyjmtmaqqXE44127193 = urhyZIQyjmtmaqqXE17167173;     urhyZIQyjmtmaqqXE17167173 = urhyZIQyjmtmaqqXE67296042;     urhyZIQyjmtmaqqXE67296042 = urhyZIQyjmtmaqqXE72968972;     urhyZIQyjmtmaqqXE72968972 = urhyZIQyjmtmaqqXE39560713;     urhyZIQyjmtmaqqXE39560713 = urhyZIQyjmtmaqqXE46969787;     urhyZIQyjmtmaqqXE46969787 = urhyZIQyjmtmaqqXE54254537;     urhyZIQyjmtmaqqXE54254537 = urhyZIQyjmtmaqqXE75806139;     urhyZIQyjmtmaqqXE75806139 = urhyZIQyjmtmaqqXE47986022;     urhyZIQyjmtmaqqXE47986022 = urhyZIQyjmtmaqqXE19669836;     urhyZIQyjmtmaqqXE19669836 = urhyZIQyjmtmaqqXE1659873;     urhyZIQyjmtmaqqXE1659873 = urhyZIQyjmtmaqqXE63949115;     urhyZIQyjmtmaqqXE63949115 = urhyZIQyjmtmaqqXE38936871;     urhyZIQyjmtmaqqXE38936871 = urhyZIQyjmtmaqqXE52028465;     urhyZIQyjmtmaqqXE52028465 = urhyZIQyjmtmaqqXE1259028;     urhyZIQyjmtmaqqXE1259028 = urhyZIQyjmtmaqqXE19496190;     urhyZIQyjmtmaqqXE19496190 = urhyZIQyjmtmaqqXE89851688;     urhyZIQyjmtmaqqXE89851688 = urhyZIQyjmtmaqqXE58880301;     urhyZIQyjmtmaqqXE58880301 = urhyZIQyjmtmaqqXE39324403;     urhyZIQyjmtmaqqXE39324403 = urhyZIQyjmtmaqqXE49649301;     urhyZIQyjmtmaqqXE49649301 = urhyZIQyjmtmaqqXE17442301;     urhyZIQyjmtmaqqXE17442301 = urhyZIQyjmtmaqqXE29376933;     urhyZIQyjmtmaqqXE29376933 = urhyZIQyjmtmaqqXE39613515;     urhyZIQyjmtmaqqXE39613515 = urhyZIQyjmtmaqqXE3940576;     urhyZIQyjmtmaqqXE3940576 = urhyZIQyjmtmaqqXE27571801;     urhyZIQyjmtmaqqXE27571801 = urhyZIQyjmtmaqqXE51424461;     urhyZIQyjmtmaqqXE51424461 = urhyZIQyjmtmaqqXE61831201;     urhyZIQyjmtmaqqXE61831201 = urhyZIQyjmtmaqqXE95580823;     urhyZIQyjmtmaqqXE95580823 = urhyZIQyjmtmaqqXE7742015;     urhyZIQyjmtmaqqXE7742015 = urhyZIQyjmtmaqqXE33204678;     urhyZIQyjmtmaqqXE33204678 = urhyZIQyjmtmaqqXE62349753;     urhyZIQyjmtmaqqXE62349753 = urhyZIQyjmtmaqqXE59951808;     urhyZIQyjmtmaqqXE59951808 = urhyZIQyjmtmaqqXE68781259;     urhyZIQyjmtmaqqXE68781259 = urhyZIQyjmtmaqqXE86917228;     urhyZIQyjmtmaqqXE86917228 = urhyZIQyjmtmaqqXE82936109;     urhyZIQyjmtmaqqXE82936109 = urhyZIQyjmtmaqqXE58881685;     urhyZIQyjmtmaqqXE58881685 = urhyZIQyjmtmaqqXE90324975;     urhyZIQyjmtmaqqXE90324975 = urhyZIQyjmtmaqqXE3692590;     urhyZIQyjmtmaqqXE3692590 = urhyZIQyjmtmaqqXE23259657;     urhyZIQyjmtmaqqXE23259657 = urhyZIQyjmtmaqqXE77094044;     urhyZIQyjmtmaqqXE77094044 = urhyZIQyjmtmaqqXE91569530;     urhyZIQyjmtmaqqXE91569530 = urhyZIQyjmtmaqqXE36340369;     urhyZIQyjmtmaqqXE36340369 = urhyZIQyjmtmaqqXE40380536;     urhyZIQyjmtmaqqXE40380536 = urhyZIQyjmtmaqqXE39200464;     urhyZIQyjmtmaqqXE39200464 = urhyZIQyjmtmaqqXE2116446;     urhyZIQyjmtmaqqXE2116446 = urhyZIQyjmtmaqqXE12993304;     urhyZIQyjmtmaqqXE12993304 = urhyZIQyjmtmaqqXE17358730;     urhyZIQyjmtmaqqXE17358730 = urhyZIQyjmtmaqqXE12636737;     urhyZIQyjmtmaqqXE12636737 = urhyZIQyjmtmaqqXE77483191;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YOEUXzeZFjBnPhKS90062003() {     double kBXRVIqTntLdkcQzj83600298 = -79533991;    double kBXRVIqTntLdkcQzj90654054 = -470931037;    double kBXRVIqTntLdkcQzj89508373 = -818108055;    double kBXRVIqTntLdkcQzj72020795 = -413513344;    double kBXRVIqTntLdkcQzj93636990 = -896165356;    double kBXRVIqTntLdkcQzj92707306 = -428264369;    double kBXRVIqTntLdkcQzj72046608 = -392072992;    double kBXRVIqTntLdkcQzj7467295 = -24621060;    double kBXRVIqTntLdkcQzj23043249 = 85057566;    double kBXRVIqTntLdkcQzj52425331 = -632065342;    double kBXRVIqTntLdkcQzj39202058 = -378068702;    double kBXRVIqTntLdkcQzj9130928 = -474907436;    double kBXRVIqTntLdkcQzj63326326 = -503366644;    double kBXRVIqTntLdkcQzj49656927 = -886827721;    double kBXRVIqTntLdkcQzj17443098 = -941756803;    double kBXRVIqTntLdkcQzj4893629 = -810404756;    double kBXRVIqTntLdkcQzj89309371 = -986024207;    double kBXRVIqTntLdkcQzj26641962 = -206733937;    double kBXRVIqTntLdkcQzj79573333 = -358922678;    double kBXRVIqTntLdkcQzj79033979 = -784686848;    double kBXRVIqTntLdkcQzj96270086 = -95471540;    double kBXRVIqTntLdkcQzj91035242 = -441778948;    double kBXRVIqTntLdkcQzj43310854 = -425634128;    double kBXRVIqTntLdkcQzj10488777 = -274554906;    double kBXRVIqTntLdkcQzj34391128 = -981368952;    double kBXRVIqTntLdkcQzj73657039 = -194121380;    double kBXRVIqTntLdkcQzj51763860 = -261784720;    double kBXRVIqTntLdkcQzj4994933 = -173807698;    double kBXRVIqTntLdkcQzj22179787 = -243422336;    double kBXRVIqTntLdkcQzj63067275 = -812927301;    double kBXRVIqTntLdkcQzj64501350 = -893255516;    double kBXRVIqTntLdkcQzj24673924 = -549083343;    double kBXRVIqTntLdkcQzj85759806 = -517890324;    double kBXRVIqTntLdkcQzj52451582 = -210402654;    double kBXRVIqTntLdkcQzj38136214 = -519711948;    double kBXRVIqTntLdkcQzj31546283 = -267532689;    double kBXRVIqTntLdkcQzj5325767 = -921109279;    double kBXRVIqTntLdkcQzj79914279 = -862894014;    double kBXRVIqTntLdkcQzj85756761 = -958938124;    double kBXRVIqTntLdkcQzj75868496 = -453077243;    double kBXRVIqTntLdkcQzj64539523 = -856076372;    double kBXRVIqTntLdkcQzj35926775 = -520097928;    double kBXRVIqTntLdkcQzj55009397 = -186196281;    double kBXRVIqTntLdkcQzj43703341 = -722678342;    double kBXRVIqTntLdkcQzj76617666 = -20648550;    double kBXRVIqTntLdkcQzj7049850 = -598858654;    double kBXRVIqTntLdkcQzj58971923 = -107265493;    double kBXRVIqTntLdkcQzj28056345 = -648041551;    double kBXRVIqTntLdkcQzj39213783 = -334698297;    double kBXRVIqTntLdkcQzj9007376 = -805419700;    double kBXRVIqTntLdkcQzj61371058 = 58390523;    double kBXRVIqTntLdkcQzj96151292 = -136234123;    double kBXRVIqTntLdkcQzj92375650 = -595709671;    double kBXRVIqTntLdkcQzj13136053 = -601740189;    double kBXRVIqTntLdkcQzj93791916 = -121543599;    double kBXRVIqTntLdkcQzj92565055 = -637755043;    double kBXRVIqTntLdkcQzj47343200 = 54703090;    double kBXRVIqTntLdkcQzj79019596 = -443553150;    double kBXRVIqTntLdkcQzj37629668 = -432144392;    double kBXRVIqTntLdkcQzj19979952 = -602043977;    double kBXRVIqTntLdkcQzj40943447 = -66479650;    double kBXRVIqTntLdkcQzj67051676 = -118265295;    double kBXRVIqTntLdkcQzj85287507 = -781198725;    double kBXRVIqTntLdkcQzj59975974 = -102015134;    double kBXRVIqTntLdkcQzj87923980 = -738809826;    double kBXRVIqTntLdkcQzj14528134 = -828985359;    double kBXRVIqTntLdkcQzj23371122 = -957017113;    double kBXRVIqTntLdkcQzj10874745 = -192963990;    double kBXRVIqTntLdkcQzj11520713 = -267115773;    double kBXRVIqTntLdkcQzj85896814 = -574224114;    double kBXRVIqTntLdkcQzj99567862 = -889295478;    double kBXRVIqTntLdkcQzj9395092 = -23130193;    double kBXRVIqTntLdkcQzj40885200 = -247795814;    double kBXRVIqTntLdkcQzj3704838 = -905845436;    double kBXRVIqTntLdkcQzj14494456 = -928610477;    double kBXRVIqTntLdkcQzj60343311 = -575373612;    double kBXRVIqTntLdkcQzj36025846 = -155582668;    double kBXRVIqTntLdkcQzj99607512 = -702955786;    double kBXRVIqTntLdkcQzj33871110 = -153906356;    double kBXRVIqTntLdkcQzj27341279 = -282510299;    double kBXRVIqTntLdkcQzj14685116 = 13144113;    double kBXRVIqTntLdkcQzj23707516 = -613743170;    double kBXRVIqTntLdkcQzj65781149 = -839109401;    double kBXRVIqTntLdkcQzj13172411 = -438002637;    double kBXRVIqTntLdkcQzj1696218 = -771317824;    double kBXRVIqTntLdkcQzj68350058 = -657021394;    double kBXRVIqTntLdkcQzj32298274 = -953373672;    double kBXRVIqTntLdkcQzj72623753 = -916150136;    double kBXRVIqTntLdkcQzj58659665 = 11140944;    double kBXRVIqTntLdkcQzj45571159 = -881956906;    double kBXRVIqTntLdkcQzj84203082 = -222235780;    double kBXRVIqTntLdkcQzj26306170 = -377556130;    double kBXRVIqTntLdkcQzj42284612 = -330749623;    double kBXRVIqTntLdkcQzj65776809 = -256894148;    double kBXRVIqTntLdkcQzj34925050 = -286597594;    double kBXRVIqTntLdkcQzj97487847 = -637811078;    double kBXRVIqTntLdkcQzj50639267 = -738899204;    double kBXRVIqTntLdkcQzj95033422 = 15818852;    double kBXRVIqTntLdkcQzj55779360 = -983868517;    double kBXRVIqTntLdkcQzj62089533 = -79533991;     kBXRVIqTntLdkcQzj83600298 = kBXRVIqTntLdkcQzj90654054;     kBXRVIqTntLdkcQzj90654054 = kBXRVIqTntLdkcQzj89508373;     kBXRVIqTntLdkcQzj89508373 = kBXRVIqTntLdkcQzj72020795;     kBXRVIqTntLdkcQzj72020795 = kBXRVIqTntLdkcQzj93636990;     kBXRVIqTntLdkcQzj93636990 = kBXRVIqTntLdkcQzj92707306;     kBXRVIqTntLdkcQzj92707306 = kBXRVIqTntLdkcQzj72046608;     kBXRVIqTntLdkcQzj72046608 = kBXRVIqTntLdkcQzj7467295;     kBXRVIqTntLdkcQzj7467295 = kBXRVIqTntLdkcQzj23043249;     kBXRVIqTntLdkcQzj23043249 = kBXRVIqTntLdkcQzj52425331;     kBXRVIqTntLdkcQzj52425331 = kBXRVIqTntLdkcQzj39202058;     kBXRVIqTntLdkcQzj39202058 = kBXRVIqTntLdkcQzj9130928;     kBXRVIqTntLdkcQzj9130928 = kBXRVIqTntLdkcQzj63326326;     kBXRVIqTntLdkcQzj63326326 = kBXRVIqTntLdkcQzj49656927;     kBXRVIqTntLdkcQzj49656927 = kBXRVIqTntLdkcQzj17443098;     kBXRVIqTntLdkcQzj17443098 = kBXRVIqTntLdkcQzj4893629;     kBXRVIqTntLdkcQzj4893629 = kBXRVIqTntLdkcQzj89309371;     kBXRVIqTntLdkcQzj89309371 = kBXRVIqTntLdkcQzj26641962;     kBXRVIqTntLdkcQzj26641962 = kBXRVIqTntLdkcQzj79573333;     kBXRVIqTntLdkcQzj79573333 = kBXRVIqTntLdkcQzj79033979;     kBXRVIqTntLdkcQzj79033979 = kBXRVIqTntLdkcQzj96270086;     kBXRVIqTntLdkcQzj96270086 = kBXRVIqTntLdkcQzj91035242;     kBXRVIqTntLdkcQzj91035242 = kBXRVIqTntLdkcQzj43310854;     kBXRVIqTntLdkcQzj43310854 = kBXRVIqTntLdkcQzj10488777;     kBXRVIqTntLdkcQzj10488777 = kBXRVIqTntLdkcQzj34391128;     kBXRVIqTntLdkcQzj34391128 = kBXRVIqTntLdkcQzj73657039;     kBXRVIqTntLdkcQzj73657039 = kBXRVIqTntLdkcQzj51763860;     kBXRVIqTntLdkcQzj51763860 = kBXRVIqTntLdkcQzj4994933;     kBXRVIqTntLdkcQzj4994933 = kBXRVIqTntLdkcQzj22179787;     kBXRVIqTntLdkcQzj22179787 = kBXRVIqTntLdkcQzj63067275;     kBXRVIqTntLdkcQzj63067275 = kBXRVIqTntLdkcQzj64501350;     kBXRVIqTntLdkcQzj64501350 = kBXRVIqTntLdkcQzj24673924;     kBXRVIqTntLdkcQzj24673924 = kBXRVIqTntLdkcQzj85759806;     kBXRVIqTntLdkcQzj85759806 = kBXRVIqTntLdkcQzj52451582;     kBXRVIqTntLdkcQzj52451582 = kBXRVIqTntLdkcQzj38136214;     kBXRVIqTntLdkcQzj38136214 = kBXRVIqTntLdkcQzj31546283;     kBXRVIqTntLdkcQzj31546283 = kBXRVIqTntLdkcQzj5325767;     kBXRVIqTntLdkcQzj5325767 = kBXRVIqTntLdkcQzj79914279;     kBXRVIqTntLdkcQzj79914279 = kBXRVIqTntLdkcQzj85756761;     kBXRVIqTntLdkcQzj85756761 = kBXRVIqTntLdkcQzj75868496;     kBXRVIqTntLdkcQzj75868496 = kBXRVIqTntLdkcQzj64539523;     kBXRVIqTntLdkcQzj64539523 = kBXRVIqTntLdkcQzj35926775;     kBXRVIqTntLdkcQzj35926775 = kBXRVIqTntLdkcQzj55009397;     kBXRVIqTntLdkcQzj55009397 = kBXRVIqTntLdkcQzj43703341;     kBXRVIqTntLdkcQzj43703341 = kBXRVIqTntLdkcQzj76617666;     kBXRVIqTntLdkcQzj76617666 = kBXRVIqTntLdkcQzj7049850;     kBXRVIqTntLdkcQzj7049850 = kBXRVIqTntLdkcQzj58971923;     kBXRVIqTntLdkcQzj58971923 = kBXRVIqTntLdkcQzj28056345;     kBXRVIqTntLdkcQzj28056345 = kBXRVIqTntLdkcQzj39213783;     kBXRVIqTntLdkcQzj39213783 = kBXRVIqTntLdkcQzj9007376;     kBXRVIqTntLdkcQzj9007376 = kBXRVIqTntLdkcQzj61371058;     kBXRVIqTntLdkcQzj61371058 = kBXRVIqTntLdkcQzj96151292;     kBXRVIqTntLdkcQzj96151292 = kBXRVIqTntLdkcQzj92375650;     kBXRVIqTntLdkcQzj92375650 = kBXRVIqTntLdkcQzj13136053;     kBXRVIqTntLdkcQzj13136053 = kBXRVIqTntLdkcQzj93791916;     kBXRVIqTntLdkcQzj93791916 = kBXRVIqTntLdkcQzj92565055;     kBXRVIqTntLdkcQzj92565055 = kBXRVIqTntLdkcQzj47343200;     kBXRVIqTntLdkcQzj47343200 = kBXRVIqTntLdkcQzj79019596;     kBXRVIqTntLdkcQzj79019596 = kBXRVIqTntLdkcQzj37629668;     kBXRVIqTntLdkcQzj37629668 = kBXRVIqTntLdkcQzj19979952;     kBXRVIqTntLdkcQzj19979952 = kBXRVIqTntLdkcQzj40943447;     kBXRVIqTntLdkcQzj40943447 = kBXRVIqTntLdkcQzj67051676;     kBXRVIqTntLdkcQzj67051676 = kBXRVIqTntLdkcQzj85287507;     kBXRVIqTntLdkcQzj85287507 = kBXRVIqTntLdkcQzj59975974;     kBXRVIqTntLdkcQzj59975974 = kBXRVIqTntLdkcQzj87923980;     kBXRVIqTntLdkcQzj87923980 = kBXRVIqTntLdkcQzj14528134;     kBXRVIqTntLdkcQzj14528134 = kBXRVIqTntLdkcQzj23371122;     kBXRVIqTntLdkcQzj23371122 = kBXRVIqTntLdkcQzj10874745;     kBXRVIqTntLdkcQzj10874745 = kBXRVIqTntLdkcQzj11520713;     kBXRVIqTntLdkcQzj11520713 = kBXRVIqTntLdkcQzj85896814;     kBXRVIqTntLdkcQzj85896814 = kBXRVIqTntLdkcQzj99567862;     kBXRVIqTntLdkcQzj99567862 = kBXRVIqTntLdkcQzj9395092;     kBXRVIqTntLdkcQzj9395092 = kBXRVIqTntLdkcQzj40885200;     kBXRVIqTntLdkcQzj40885200 = kBXRVIqTntLdkcQzj3704838;     kBXRVIqTntLdkcQzj3704838 = kBXRVIqTntLdkcQzj14494456;     kBXRVIqTntLdkcQzj14494456 = kBXRVIqTntLdkcQzj60343311;     kBXRVIqTntLdkcQzj60343311 = kBXRVIqTntLdkcQzj36025846;     kBXRVIqTntLdkcQzj36025846 = kBXRVIqTntLdkcQzj99607512;     kBXRVIqTntLdkcQzj99607512 = kBXRVIqTntLdkcQzj33871110;     kBXRVIqTntLdkcQzj33871110 = kBXRVIqTntLdkcQzj27341279;     kBXRVIqTntLdkcQzj27341279 = kBXRVIqTntLdkcQzj14685116;     kBXRVIqTntLdkcQzj14685116 = kBXRVIqTntLdkcQzj23707516;     kBXRVIqTntLdkcQzj23707516 = kBXRVIqTntLdkcQzj65781149;     kBXRVIqTntLdkcQzj65781149 = kBXRVIqTntLdkcQzj13172411;     kBXRVIqTntLdkcQzj13172411 = kBXRVIqTntLdkcQzj1696218;     kBXRVIqTntLdkcQzj1696218 = kBXRVIqTntLdkcQzj68350058;     kBXRVIqTntLdkcQzj68350058 = kBXRVIqTntLdkcQzj32298274;     kBXRVIqTntLdkcQzj32298274 = kBXRVIqTntLdkcQzj72623753;     kBXRVIqTntLdkcQzj72623753 = kBXRVIqTntLdkcQzj58659665;     kBXRVIqTntLdkcQzj58659665 = kBXRVIqTntLdkcQzj45571159;     kBXRVIqTntLdkcQzj45571159 = kBXRVIqTntLdkcQzj84203082;     kBXRVIqTntLdkcQzj84203082 = kBXRVIqTntLdkcQzj26306170;     kBXRVIqTntLdkcQzj26306170 = kBXRVIqTntLdkcQzj42284612;     kBXRVIqTntLdkcQzj42284612 = kBXRVIqTntLdkcQzj65776809;     kBXRVIqTntLdkcQzj65776809 = kBXRVIqTntLdkcQzj34925050;     kBXRVIqTntLdkcQzj34925050 = kBXRVIqTntLdkcQzj97487847;     kBXRVIqTntLdkcQzj97487847 = kBXRVIqTntLdkcQzj50639267;     kBXRVIqTntLdkcQzj50639267 = kBXRVIqTntLdkcQzj95033422;     kBXRVIqTntLdkcQzj95033422 = kBXRVIqTntLdkcQzj55779360;     kBXRVIqTntLdkcQzj55779360 = kBXRVIqTntLdkcQzj62089533;     kBXRVIqTntLdkcQzj62089533 = kBXRVIqTntLdkcQzj83600298;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gctUXLTPmpuCwtjH57353603() {     double hbzvgFSoRNWZeZzOz19059267 = -545810891;    double hbzvgFSoRNWZeZzOz3926133 = -351482644;    double hbzvgFSoRNWZeZzOz26894850 = -483775130;    double hbzvgFSoRNWZeZzOz44244150 = -234664416;    double hbzvgFSoRNWZeZzOz49922260 = 98235799;    double hbzvgFSoRNWZeZzOz28930179 = -690612265;    double hbzvgFSoRNWZeZzOz82704014 = -236798726;    double hbzvgFSoRNWZeZzOz69598855 = -768855201;    double hbzvgFSoRNWZeZzOz24560855 = -403273841;    double hbzvgFSoRNWZeZzOz67328043 = -296752340;    double hbzvgFSoRNWZeZzOz77383792 = -180076431;    double hbzvgFSoRNWZeZzOz9699077 = -339028208;    double hbzvgFSoRNWZeZzOz1560972 = 44620851;    double hbzvgFSoRNWZeZzOz2797791 = -863191130;    double hbzvgFSoRNWZeZzOz75637614 = -345181818;    double hbzvgFSoRNWZeZzOz88434929 = -446216363;    double hbzvgFSoRNWZeZzOz45145307 = -412998760;    double hbzvgFSoRNWZeZzOz26402153 = -280162005;    double hbzvgFSoRNWZeZzOz80123663 = -675486100;    double hbzvgFSoRNWZeZzOz72871197 = -111247914;    double hbzvgFSoRNWZeZzOz9356950 = -852746229;    double hbzvgFSoRNWZeZzOz94456767 = -528415242;    double hbzvgFSoRNWZeZzOz13836234 = -63266312;    double hbzvgFSoRNWZeZzOz4281722 = -785379510;    double hbzvgFSoRNWZeZzOz42409079 = 94645849;    double hbzvgFSoRNWZeZzOz84355210 = -982698187;    double hbzvgFSoRNWZeZzOz12184434 = -68068861;    double hbzvgFSoRNWZeZzOz45723357 = -343579872;    double hbzvgFSoRNWZeZzOz68848001 = -817517138;    double hbzvgFSoRNWZeZzOz68600686 = -847434471;    double hbzvgFSoRNWZeZzOz83084581 = -345236003;    double hbzvgFSoRNWZeZzOz62496942 = -322054757;    double hbzvgFSoRNWZeZzOz97977817 = -665486301;    double hbzvgFSoRNWZeZzOz82200008 = -275096468;    double hbzvgFSoRNWZeZzOz3944668 = -470812199;    double hbzvgFSoRNWZeZzOz83107127 = -482097995;    double hbzvgFSoRNWZeZzOz54737583 = -785805319;    double hbzvgFSoRNWZeZzOz31129683 = -845549596;    double hbzvgFSoRNWZeZzOz16147323 = -609814400;    double hbzvgFSoRNWZeZzOz40140029 = 81660211;    double hbzvgFSoRNWZeZzOz97991130 = -625275286;    double hbzvgFSoRNWZeZzOz25291902 = -114643219;    double hbzvgFSoRNWZeZzOz44392775 = -839377067;    double hbzvgFSoRNWZeZzOz83834679 = -227010752;    double hbzvgFSoRNWZeZzOz76634125 = -947159711;    double hbzvgFSoRNWZeZzOz53011178 = -637782286;    double hbzvgFSoRNWZeZzOz78317086 = -591251854;    double hbzvgFSoRNWZeZzOz45782656 = -814568144;    double hbzvgFSoRNWZeZzOz62106449 = 1564823;    double hbzvgFSoRNWZeZzOz33536965 = -196446515;    double hbzvgFSoRNWZeZzOz42141782 = -915886034;    double hbzvgFSoRNWZeZzOz79636764 = -349886469;    double hbzvgFSoRNWZeZzOz60996798 = -351755225;    double hbzvgFSoRNWZeZzOz29127900 = -793248689;    double hbzvgFSoRNWZeZzOz47996653 = -468033745;    double hbzvgFSoRNWZeZzOz24602499 = 82604351;    double hbzvgFSoRNWZeZzOz90089899 = -188216332;    double hbzvgFSoRNWZeZzOz22613129 = -698395620;    double hbzvgFSoRNWZeZzOz1835072 = -229310265;    double hbzvgFSoRNWZeZzOz65567049 = 80933985;    double hbzvgFSoRNWZeZzOz16745745 = -522543404;    double hbzvgFSoRNWZeZzOz36980657 = -893218854;    double hbzvgFSoRNWZeZzOz750855 = -951338063;    double hbzvgFSoRNWZeZzOz55960168 = -555839371;    double hbzvgFSoRNWZeZzOz84243461 = -951516337;    double hbzvgFSoRNWZeZzOz14886851 = -858021674;    double hbzvgFSoRNWZeZzOz11721260 = -673541907;    double hbzvgFSoRNWZeZzOz19360964 = -680282682;    double hbzvgFSoRNWZeZzOz98853122 = -292378932;    double hbzvgFSoRNWZeZzOz92530487 = -863083823;    double hbzvgFSoRNWZeZzOz33697347 = -660411044;    double hbzvgFSoRNWZeZzOz14015625 = -567449165;    double hbzvgFSoRNWZeZzOz10254831 = -670347606;    double hbzvgFSoRNWZeZzOz39983635 = -657146311;    double hbzvgFSoRNWZeZzOz74880067 = -485972628;    double hbzvgFSoRNWZeZzOz84065047 = -638103010;    double hbzvgFSoRNWZeZzOz50063993 = -689038176;    double hbzvgFSoRNWZeZzOz30001555 = -836255561;    double hbzvgFSoRNWZeZzOz27647597 = -838219799;    double hbzvgFSoRNWZeZzOz89397901 = -267571866;    double hbzvgFSoRNWZeZzOz6038124 = -291446334;    double hbzvgFSoRNWZeZzOz66401777 = -253500717;    double hbzvgFSoRNWZeZzOz83616908 = -245144695;    double hbzvgFSoRNWZeZzOz35311036 = -521070624;    double hbzvgFSoRNWZeZzOz26458905 = -931548437;    double hbzvgFSoRNWZeZzOz3447818 = -995349534;    double hbzvgFSoRNWZeZzOz1500144 = -970299533;    double hbzvgFSoRNWZeZzOz68849917 = -872237613;    double hbzvgFSoRNWZeZzOz34203356 = -807062723;    double hbzvgFSoRNWZeZzOz79342169 = -453416550;    double hbzvgFSoRNWZeZzOz93017228 = -193881663;    double hbzvgFSoRNWZeZzOz32124455 = 12590301;    double hbzvgFSoRNWZeZzOz29294612 = -516239331;    double hbzvgFSoRNWZeZzOz50580273 = -590748385;    double hbzvgFSoRNWZeZzOz23394284 = -395796385;    double hbzvgFSoRNWZeZzOz61010473 = -732056432;    double hbzvgFSoRNWZeZzOz24541048 = -163305157;    double hbzvgFSoRNWZeZzOz88432607 = -183537697;    double hbzvgFSoRNWZeZzOz99591218 = -275494416;    double hbzvgFSoRNWZeZzOz61747275 = -545810891;     hbzvgFSoRNWZeZzOz19059267 = hbzvgFSoRNWZeZzOz3926133;     hbzvgFSoRNWZeZzOz3926133 = hbzvgFSoRNWZeZzOz26894850;     hbzvgFSoRNWZeZzOz26894850 = hbzvgFSoRNWZeZzOz44244150;     hbzvgFSoRNWZeZzOz44244150 = hbzvgFSoRNWZeZzOz49922260;     hbzvgFSoRNWZeZzOz49922260 = hbzvgFSoRNWZeZzOz28930179;     hbzvgFSoRNWZeZzOz28930179 = hbzvgFSoRNWZeZzOz82704014;     hbzvgFSoRNWZeZzOz82704014 = hbzvgFSoRNWZeZzOz69598855;     hbzvgFSoRNWZeZzOz69598855 = hbzvgFSoRNWZeZzOz24560855;     hbzvgFSoRNWZeZzOz24560855 = hbzvgFSoRNWZeZzOz67328043;     hbzvgFSoRNWZeZzOz67328043 = hbzvgFSoRNWZeZzOz77383792;     hbzvgFSoRNWZeZzOz77383792 = hbzvgFSoRNWZeZzOz9699077;     hbzvgFSoRNWZeZzOz9699077 = hbzvgFSoRNWZeZzOz1560972;     hbzvgFSoRNWZeZzOz1560972 = hbzvgFSoRNWZeZzOz2797791;     hbzvgFSoRNWZeZzOz2797791 = hbzvgFSoRNWZeZzOz75637614;     hbzvgFSoRNWZeZzOz75637614 = hbzvgFSoRNWZeZzOz88434929;     hbzvgFSoRNWZeZzOz88434929 = hbzvgFSoRNWZeZzOz45145307;     hbzvgFSoRNWZeZzOz45145307 = hbzvgFSoRNWZeZzOz26402153;     hbzvgFSoRNWZeZzOz26402153 = hbzvgFSoRNWZeZzOz80123663;     hbzvgFSoRNWZeZzOz80123663 = hbzvgFSoRNWZeZzOz72871197;     hbzvgFSoRNWZeZzOz72871197 = hbzvgFSoRNWZeZzOz9356950;     hbzvgFSoRNWZeZzOz9356950 = hbzvgFSoRNWZeZzOz94456767;     hbzvgFSoRNWZeZzOz94456767 = hbzvgFSoRNWZeZzOz13836234;     hbzvgFSoRNWZeZzOz13836234 = hbzvgFSoRNWZeZzOz4281722;     hbzvgFSoRNWZeZzOz4281722 = hbzvgFSoRNWZeZzOz42409079;     hbzvgFSoRNWZeZzOz42409079 = hbzvgFSoRNWZeZzOz84355210;     hbzvgFSoRNWZeZzOz84355210 = hbzvgFSoRNWZeZzOz12184434;     hbzvgFSoRNWZeZzOz12184434 = hbzvgFSoRNWZeZzOz45723357;     hbzvgFSoRNWZeZzOz45723357 = hbzvgFSoRNWZeZzOz68848001;     hbzvgFSoRNWZeZzOz68848001 = hbzvgFSoRNWZeZzOz68600686;     hbzvgFSoRNWZeZzOz68600686 = hbzvgFSoRNWZeZzOz83084581;     hbzvgFSoRNWZeZzOz83084581 = hbzvgFSoRNWZeZzOz62496942;     hbzvgFSoRNWZeZzOz62496942 = hbzvgFSoRNWZeZzOz97977817;     hbzvgFSoRNWZeZzOz97977817 = hbzvgFSoRNWZeZzOz82200008;     hbzvgFSoRNWZeZzOz82200008 = hbzvgFSoRNWZeZzOz3944668;     hbzvgFSoRNWZeZzOz3944668 = hbzvgFSoRNWZeZzOz83107127;     hbzvgFSoRNWZeZzOz83107127 = hbzvgFSoRNWZeZzOz54737583;     hbzvgFSoRNWZeZzOz54737583 = hbzvgFSoRNWZeZzOz31129683;     hbzvgFSoRNWZeZzOz31129683 = hbzvgFSoRNWZeZzOz16147323;     hbzvgFSoRNWZeZzOz16147323 = hbzvgFSoRNWZeZzOz40140029;     hbzvgFSoRNWZeZzOz40140029 = hbzvgFSoRNWZeZzOz97991130;     hbzvgFSoRNWZeZzOz97991130 = hbzvgFSoRNWZeZzOz25291902;     hbzvgFSoRNWZeZzOz25291902 = hbzvgFSoRNWZeZzOz44392775;     hbzvgFSoRNWZeZzOz44392775 = hbzvgFSoRNWZeZzOz83834679;     hbzvgFSoRNWZeZzOz83834679 = hbzvgFSoRNWZeZzOz76634125;     hbzvgFSoRNWZeZzOz76634125 = hbzvgFSoRNWZeZzOz53011178;     hbzvgFSoRNWZeZzOz53011178 = hbzvgFSoRNWZeZzOz78317086;     hbzvgFSoRNWZeZzOz78317086 = hbzvgFSoRNWZeZzOz45782656;     hbzvgFSoRNWZeZzOz45782656 = hbzvgFSoRNWZeZzOz62106449;     hbzvgFSoRNWZeZzOz62106449 = hbzvgFSoRNWZeZzOz33536965;     hbzvgFSoRNWZeZzOz33536965 = hbzvgFSoRNWZeZzOz42141782;     hbzvgFSoRNWZeZzOz42141782 = hbzvgFSoRNWZeZzOz79636764;     hbzvgFSoRNWZeZzOz79636764 = hbzvgFSoRNWZeZzOz60996798;     hbzvgFSoRNWZeZzOz60996798 = hbzvgFSoRNWZeZzOz29127900;     hbzvgFSoRNWZeZzOz29127900 = hbzvgFSoRNWZeZzOz47996653;     hbzvgFSoRNWZeZzOz47996653 = hbzvgFSoRNWZeZzOz24602499;     hbzvgFSoRNWZeZzOz24602499 = hbzvgFSoRNWZeZzOz90089899;     hbzvgFSoRNWZeZzOz90089899 = hbzvgFSoRNWZeZzOz22613129;     hbzvgFSoRNWZeZzOz22613129 = hbzvgFSoRNWZeZzOz1835072;     hbzvgFSoRNWZeZzOz1835072 = hbzvgFSoRNWZeZzOz65567049;     hbzvgFSoRNWZeZzOz65567049 = hbzvgFSoRNWZeZzOz16745745;     hbzvgFSoRNWZeZzOz16745745 = hbzvgFSoRNWZeZzOz36980657;     hbzvgFSoRNWZeZzOz36980657 = hbzvgFSoRNWZeZzOz750855;     hbzvgFSoRNWZeZzOz750855 = hbzvgFSoRNWZeZzOz55960168;     hbzvgFSoRNWZeZzOz55960168 = hbzvgFSoRNWZeZzOz84243461;     hbzvgFSoRNWZeZzOz84243461 = hbzvgFSoRNWZeZzOz14886851;     hbzvgFSoRNWZeZzOz14886851 = hbzvgFSoRNWZeZzOz11721260;     hbzvgFSoRNWZeZzOz11721260 = hbzvgFSoRNWZeZzOz19360964;     hbzvgFSoRNWZeZzOz19360964 = hbzvgFSoRNWZeZzOz98853122;     hbzvgFSoRNWZeZzOz98853122 = hbzvgFSoRNWZeZzOz92530487;     hbzvgFSoRNWZeZzOz92530487 = hbzvgFSoRNWZeZzOz33697347;     hbzvgFSoRNWZeZzOz33697347 = hbzvgFSoRNWZeZzOz14015625;     hbzvgFSoRNWZeZzOz14015625 = hbzvgFSoRNWZeZzOz10254831;     hbzvgFSoRNWZeZzOz10254831 = hbzvgFSoRNWZeZzOz39983635;     hbzvgFSoRNWZeZzOz39983635 = hbzvgFSoRNWZeZzOz74880067;     hbzvgFSoRNWZeZzOz74880067 = hbzvgFSoRNWZeZzOz84065047;     hbzvgFSoRNWZeZzOz84065047 = hbzvgFSoRNWZeZzOz50063993;     hbzvgFSoRNWZeZzOz50063993 = hbzvgFSoRNWZeZzOz30001555;     hbzvgFSoRNWZeZzOz30001555 = hbzvgFSoRNWZeZzOz27647597;     hbzvgFSoRNWZeZzOz27647597 = hbzvgFSoRNWZeZzOz89397901;     hbzvgFSoRNWZeZzOz89397901 = hbzvgFSoRNWZeZzOz6038124;     hbzvgFSoRNWZeZzOz6038124 = hbzvgFSoRNWZeZzOz66401777;     hbzvgFSoRNWZeZzOz66401777 = hbzvgFSoRNWZeZzOz83616908;     hbzvgFSoRNWZeZzOz83616908 = hbzvgFSoRNWZeZzOz35311036;     hbzvgFSoRNWZeZzOz35311036 = hbzvgFSoRNWZeZzOz26458905;     hbzvgFSoRNWZeZzOz26458905 = hbzvgFSoRNWZeZzOz3447818;     hbzvgFSoRNWZeZzOz3447818 = hbzvgFSoRNWZeZzOz1500144;     hbzvgFSoRNWZeZzOz1500144 = hbzvgFSoRNWZeZzOz68849917;     hbzvgFSoRNWZeZzOz68849917 = hbzvgFSoRNWZeZzOz34203356;     hbzvgFSoRNWZeZzOz34203356 = hbzvgFSoRNWZeZzOz79342169;     hbzvgFSoRNWZeZzOz79342169 = hbzvgFSoRNWZeZzOz93017228;     hbzvgFSoRNWZeZzOz93017228 = hbzvgFSoRNWZeZzOz32124455;     hbzvgFSoRNWZeZzOz32124455 = hbzvgFSoRNWZeZzOz29294612;     hbzvgFSoRNWZeZzOz29294612 = hbzvgFSoRNWZeZzOz50580273;     hbzvgFSoRNWZeZzOz50580273 = hbzvgFSoRNWZeZzOz23394284;     hbzvgFSoRNWZeZzOz23394284 = hbzvgFSoRNWZeZzOz61010473;     hbzvgFSoRNWZeZzOz61010473 = hbzvgFSoRNWZeZzOz24541048;     hbzvgFSoRNWZeZzOz24541048 = hbzvgFSoRNWZeZzOz88432607;     hbzvgFSoRNWZeZzOz88432607 = hbzvgFSoRNWZeZzOz99591218;     hbzvgFSoRNWZeZzOz99591218 = hbzvgFSoRNWZeZzOz61747275;     hbzvgFSoRNWZeZzOz61747275 = hbzvgFSoRNWZeZzOz19059267;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RTMeWLjubhrZTxRz72402670() {     double VZytuxvsyRljWidBX25176373 = -657712779;    double VZytuxvsyRljWidBX47299412 = -61082425;    double VZytuxvsyRljWidBX42937034 = -634608051;    double VZytuxvsyRljWidBX71318931 = -425413046;    double VZytuxvsyRljWidBX18817749 = -422414073;    double VZytuxvsyRljWidBX20347381 = -955108500;    double VZytuxvsyRljWidBX43192981 = -375763703;    double VZytuxvsyRljWidBX19358746 = 44270886;    double VZytuxvsyRljWidBX89203349 = -171306507;    double VZytuxvsyRljWidBX8557703 = -625727817;    double VZytuxvsyRljWidBX37494083 = -57514018;    double VZytuxvsyRljWidBX41522119 = -783434410;    double VZytuxvsyRljWidBX87770090 = -395270238;    double VZytuxvsyRljWidBX85941467 = -295429698;    double VZytuxvsyRljWidBX71380041 = 64356016;    double VZytuxvsyRljWidBX29940323 = -419215906;    double VZytuxvsyRljWidBX38981310 = 70456127;    double VZytuxvsyRljWidBX23455306 = 54315352;    double VZytuxvsyRljWidBX41953505 = 18744804;    double VZytuxvsyRljWidBX53421362 = -468150834;    double VZytuxvsyRljWidBX97910142 = -708233684;    double VZytuxvsyRljWidBX80977791 = -436344959;    double VZytuxvsyRljWidBX49427026 = -909783799;    double VZytuxvsyRljWidBX88274095 = 92315502;    double VZytuxvsyRljWidBX86108729 = -278550184;    double VZytuxvsyRljWidBX9076886 = -861353510;    double VZytuxvsyRljWidBX10644212 = 31268936;    double VZytuxvsyRljWidBX58830483 = -409508826;    double VZytuxvsyRljWidBX34980257 = 60100157;    double VZytuxvsyRljWidBX37216323 = -664905339;    double VZytuxvsyRljWidBX75327131 = -258556941;    double VZytuxvsyRljWidBX60107564 = -31501573;    double VZytuxvsyRljWidBX7688766 = -558099672;    double VZytuxvsyRljWidBX77030572 = -26638233;    double VZytuxvsyRljWidBX65419320 = -9545304;    double VZytuxvsyRljWidBX51833040 = -696124102;    double VZytuxvsyRljWidBX35999518 = -739441188;    double VZytuxvsyRljWidBX65219895 = -617055605;    double VZytuxvsyRljWidBX89757576 = -603737897;    double VZytuxvsyRljWidBX27641966 = -391779011;    double VZytuxvsyRljWidBX3660355 = -690549180;    double VZytuxvsyRljWidBX57442359 = -384785497;    double VZytuxvsyRljWidBX22459753 = -920181651;    double VZytuxvsyRljWidBX71242420 = -246353691;    double VZytuxvsyRljWidBX88586590 = -609057866;    double VZytuxvsyRljWidBX64950373 = -622998402;    double VZytuxvsyRljWidBX96095660 = -710027391;    double VZytuxvsyRljWidBX53739597 = -824463677;    double VZytuxvsyRljWidBX71782178 = -84172166;    double VZytuxvsyRljWidBX46448617 = -283457323;    double VZytuxvsyRljWidBX77842460 = -758179705;    double VZytuxvsyRljWidBX90446484 = -150612789;    double VZytuxvsyRljWidBX9245256 = -143544813;    double VZytuxvsyRljWidBX25096780 = -13718834;    double VZytuxvsyRljWidBX74492528 = -68565527;    double VZytuxvsyRljWidBX44198582 = -121367821;    double VZytuxvsyRljWidBX97872386 = -151298626;    double VZytuxvsyRljWidBX54662938 = -626923554;    double VZytuxvsyRljWidBX85210201 = -46862863;    double VZytuxvsyRljWidBX9740863 = -561060563;    double VZytuxvsyRljWidBX9703169 = -886377436;    double VZytuxvsyRljWidBX84362498 = -966254878;    double VZytuxvsyRljWidBX84378489 = 84170728;    double VZytuxvsyRljWidBX51987027 = -506401169;    double VZytuxvsyRljWidBX33230571 = -267170877;    double VZytuxvsyRljWidBX77386518 = 73987555;    double VZytuxvsyRljWidBX33833353 = -125334739;    double VZytuxvsyRljWidBX10739518 = -268632005;    double VZytuxvsyRljWidBX20522148 = -185884394;    double VZytuxvsyRljWidBX19547001 = -239519882;    double VZytuxvsyRljWidBX93940805 = -679774718;    double VZytuxvsyRljWidBX73761415 = -312488268;    double VZytuxvsyRljWidBX33697730 = -341946752;    double VZytuxvsyRljWidBX14311539 = -589476185;    double VZytuxvsyRljWidBX49761008 = -777601654;    double VZytuxvsyRljWidBX40467784 = -223448188;    double VZytuxvsyRljWidBX58518038 = -516163308;    double VZytuxvsyRljWidBX78184605 = -563430109;    double VZytuxvsyRljWidBX99687505 = -298626633;    double VZytuxvsyRljWidBX21158356 = -655551782;    double VZytuxvsyRljWidBX12981225 = -51326119;    double VZytuxvsyRljWidBX56904615 = -144267388;    double VZytuxvsyRljWidBX87048304 = -225336661;    double VZytuxvsyRljWidBX88531639 = -656442520;    double VZytuxvsyRljWidBX59373863 = -906725634;    double VZytuxvsyRljWidBX84880647 = -7944152;    double VZytuxvsyRljWidBX50862308 = -887956761;    double VZytuxvsyRljWidBX82591985 = -444380838;    double VZytuxvsyRljWidBX2538045 = -958072706;    double VZytuxvsyRljWidBX21220738 = -888177483;    double VZytuxvsyRljWidBX53960654 = -444825477;    double VZytuxvsyRljWidBX81336579 = -12517635;    double VZytuxvsyRljWidBX80009693 = -470192743;    double VZytuxvsyRljWidBX80016713 = 57322666;    double VZytuxvsyRljWidBX17938798 = -505401576;    double VZytuxvsyRljWidBX19297857 = -724294303;    double VZytuxvsyRljWidBX73063869 = -368956226;    double VZytuxvsyRljWidBX70472726 = -313780483;    double VZytuxvsyRljWidBX38011849 = -979182815;    double VZytuxvsyRljWidBX11200072 = -657712779;     VZytuxvsyRljWidBX25176373 = VZytuxvsyRljWidBX47299412;     VZytuxvsyRljWidBX47299412 = VZytuxvsyRljWidBX42937034;     VZytuxvsyRljWidBX42937034 = VZytuxvsyRljWidBX71318931;     VZytuxvsyRljWidBX71318931 = VZytuxvsyRljWidBX18817749;     VZytuxvsyRljWidBX18817749 = VZytuxvsyRljWidBX20347381;     VZytuxvsyRljWidBX20347381 = VZytuxvsyRljWidBX43192981;     VZytuxvsyRljWidBX43192981 = VZytuxvsyRljWidBX19358746;     VZytuxvsyRljWidBX19358746 = VZytuxvsyRljWidBX89203349;     VZytuxvsyRljWidBX89203349 = VZytuxvsyRljWidBX8557703;     VZytuxvsyRljWidBX8557703 = VZytuxvsyRljWidBX37494083;     VZytuxvsyRljWidBX37494083 = VZytuxvsyRljWidBX41522119;     VZytuxvsyRljWidBX41522119 = VZytuxvsyRljWidBX87770090;     VZytuxvsyRljWidBX87770090 = VZytuxvsyRljWidBX85941467;     VZytuxvsyRljWidBX85941467 = VZytuxvsyRljWidBX71380041;     VZytuxvsyRljWidBX71380041 = VZytuxvsyRljWidBX29940323;     VZytuxvsyRljWidBX29940323 = VZytuxvsyRljWidBX38981310;     VZytuxvsyRljWidBX38981310 = VZytuxvsyRljWidBX23455306;     VZytuxvsyRljWidBX23455306 = VZytuxvsyRljWidBX41953505;     VZytuxvsyRljWidBX41953505 = VZytuxvsyRljWidBX53421362;     VZytuxvsyRljWidBX53421362 = VZytuxvsyRljWidBX97910142;     VZytuxvsyRljWidBX97910142 = VZytuxvsyRljWidBX80977791;     VZytuxvsyRljWidBX80977791 = VZytuxvsyRljWidBX49427026;     VZytuxvsyRljWidBX49427026 = VZytuxvsyRljWidBX88274095;     VZytuxvsyRljWidBX88274095 = VZytuxvsyRljWidBX86108729;     VZytuxvsyRljWidBX86108729 = VZytuxvsyRljWidBX9076886;     VZytuxvsyRljWidBX9076886 = VZytuxvsyRljWidBX10644212;     VZytuxvsyRljWidBX10644212 = VZytuxvsyRljWidBX58830483;     VZytuxvsyRljWidBX58830483 = VZytuxvsyRljWidBX34980257;     VZytuxvsyRljWidBX34980257 = VZytuxvsyRljWidBX37216323;     VZytuxvsyRljWidBX37216323 = VZytuxvsyRljWidBX75327131;     VZytuxvsyRljWidBX75327131 = VZytuxvsyRljWidBX60107564;     VZytuxvsyRljWidBX60107564 = VZytuxvsyRljWidBX7688766;     VZytuxvsyRljWidBX7688766 = VZytuxvsyRljWidBX77030572;     VZytuxvsyRljWidBX77030572 = VZytuxvsyRljWidBX65419320;     VZytuxvsyRljWidBX65419320 = VZytuxvsyRljWidBX51833040;     VZytuxvsyRljWidBX51833040 = VZytuxvsyRljWidBX35999518;     VZytuxvsyRljWidBX35999518 = VZytuxvsyRljWidBX65219895;     VZytuxvsyRljWidBX65219895 = VZytuxvsyRljWidBX89757576;     VZytuxvsyRljWidBX89757576 = VZytuxvsyRljWidBX27641966;     VZytuxvsyRljWidBX27641966 = VZytuxvsyRljWidBX3660355;     VZytuxvsyRljWidBX3660355 = VZytuxvsyRljWidBX57442359;     VZytuxvsyRljWidBX57442359 = VZytuxvsyRljWidBX22459753;     VZytuxvsyRljWidBX22459753 = VZytuxvsyRljWidBX71242420;     VZytuxvsyRljWidBX71242420 = VZytuxvsyRljWidBX88586590;     VZytuxvsyRljWidBX88586590 = VZytuxvsyRljWidBX64950373;     VZytuxvsyRljWidBX64950373 = VZytuxvsyRljWidBX96095660;     VZytuxvsyRljWidBX96095660 = VZytuxvsyRljWidBX53739597;     VZytuxvsyRljWidBX53739597 = VZytuxvsyRljWidBX71782178;     VZytuxvsyRljWidBX71782178 = VZytuxvsyRljWidBX46448617;     VZytuxvsyRljWidBX46448617 = VZytuxvsyRljWidBX77842460;     VZytuxvsyRljWidBX77842460 = VZytuxvsyRljWidBX90446484;     VZytuxvsyRljWidBX90446484 = VZytuxvsyRljWidBX9245256;     VZytuxvsyRljWidBX9245256 = VZytuxvsyRljWidBX25096780;     VZytuxvsyRljWidBX25096780 = VZytuxvsyRljWidBX74492528;     VZytuxvsyRljWidBX74492528 = VZytuxvsyRljWidBX44198582;     VZytuxvsyRljWidBX44198582 = VZytuxvsyRljWidBX97872386;     VZytuxvsyRljWidBX97872386 = VZytuxvsyRljWidBX54662938;     VZytuxvsyRljWidBX54662938 = VZytuxvsyRljWidBX85210201;     VZytuxvsyRljWidBX85210201 = VZytuxvsyRljWidBX9740863;     VZytuxvsyRljWidBX9740863 = VZytuxvsyRljWidBX9703169;     VZytuxvsyRljWidBX9703169 = VZytuxvsyRljWidBX84362498;     VZytuxvsyRljWidBX84362498 = VZytuxvsyRljWidBX84378489;     VZytuxvsyRljWidBX84378489 = VZytuxvsyRljWidBX51987027;     VZytuxvsyRljWidBX51987027 = VZytuxvsyRljWidBX33230571;     VZytuxvsyRljWidBX33230571 = VZytuxvsyRljWidBX77386518;     VZytuxvsyRljWidBX77386518 = VZytuxvsyRljWidBX33833353;     VZytuxvsyRljWidBX33833353 = VZytuxvsyRljWidBX10739518;     VZytuxvsyRljWidBX10739518 = VZytuxvsyRljWidBX20522148;     VZytuxvsyRljWidBX20522148 = VZytuxvsyRljWidBX19547001;     VZytuxvsyRljWidBX19547001 = VZytuxvsyRljWidBX93940805;     VZytuxvsyRljWidBX93940805 = VZytuxvsyRljWidBX73761415;     VZytuxvsyRljWidBX73761415 = VZytuxvsyRljWidBX33697730;     VZytuxvsyRljWidBX33697730 = VZytuxvsyRljWidBX14311539;     VZytuxvsyRljWidBX14311539 = VZytuxvsyRljWidBX49761008;     VZytuxvsyRljWidBX49761008 = VZytuxvsyRljWidBX40467784;     VZytuxvsyRljWidBX40467784 = VZytuxvsyRljWidBX58518038;     VZytuxvsyRljWidBX58518038 = VZytuxvsyRljWidBX78184605;     VZytuxvsyRljWidBX78184605 = VZytuxvsyRljWidBX99687505;     VZytuxvsyRljWidBX99687505 = VZytuxvsyRljWidBX21158356;     VZytuxvsyRljWidBX21158356 = VZytuxvsyRljWidBX12981225;     VZytuxvsyRljWidBX12981225 = VZytuxvsyRljWidBX56904615;     VZytuxvsyRljWidBX56904615 = VZytuxvsyRljWidBX87048304;     VZytuxvsyRljWidBX87048304 = VZytuxvsyRljWidBX88531639;     VZytuxvsyRljWidBX88531639 = VZytuxvsyRljWidBX59373863;     VZytuxvsyRljWidBX59373863 = VZytuxvsyRljWidBX84880647;     VZytuxvsyRljWidBX84880647 = VZytuxvsyRljWidBX50862308;     VZytuxvsyRljWidBX50862308 = VZytuxvsyRljWidBX82591985;     VZytuxvsyRljWidBX82591985 = VZytuxvsyRljWidBX2538045;     VZytuxvsyRljWidBX2538045 = VZytuxvsyRljWidBX21220738;     VZytuxvsyRljWidBX21220738 = VZytuxvsyRljWidBX53960654;     VZytuxvsyRljWidBX53960654 = VZytuxvsyRljWidBX81336579;     VZytuxvsyRljWidBX81336579 = VZytuxvsyRljWidBX80009693;     VZytuxvsyRljWidBX80009693 = VZytuxvsyRljWidBX80016713;     VZytuxvsyRljWidBX80016713 = VZytuxvsyRljWidBX17938798;     VZytuxvsyRljWidBX17938798 = VZytuxvsyRljWidBX19297857;     VZytuxvsyRljWidBX19297857 = VZytuxvsyRljWidBX73063869;     VZytuxvsyRljWidBX73063869 = VZytuxvsyRljWidBX70472726;     VZytuxvsyRljWidBX70472726 = VZytuxvsyRljWidBX38011849;     VZytuxvsyRljWidBX38011849 = VZytuxvsyRljWidBX11200072;     VZytuxvsyRljWidBX11200072 = VZytuxvsyRljWidBX25176373;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zopFvLoewjLhTAtg39694270() {     double uYHcsbyDDFRYUAqAg60635342 = -23989679;    double uYHcsbyDDFRYUAqAg60571491 = 58365969;    double uYHcsbyDDFRYUAqAg80323510 = -300275126;    double uYHcsbyDDFRYUAqAg43542286 = -246564117;    double uYHcsbyDDFRYUAqAg75103018 = -528012918;    double uYHcsbyDDFRYUAqAg56570253 = -117456395;    double uYHcsbyDDFRYUAqAg53850387 = -220489437;    double uYHcsbyDDFRYUAqAg81490307 = -699963254;    double uYHcsbyDDFRYUAqAg90720954 = -659637914;    double uYHcsbyDDFRYUAqAg23460415 = -290414815;    double uYHcsbyDDFRYUAqAg75675817 = -959521747;    double uYHcsbyDDFRYUAqAg42090267 = -647555181;    double uYHcsbyDDFRYUAqAg26004736 = -947282743;    double uYHcsbyDDFRYUAqAg39082331 = -271793107;    double uYHcsbyDDFRYUAqAg29574557 = -439068999;    double uYHcsbyDDFRYUAqAg13481624 = -55027513;    double uYHcsbyDDFRYUAqAg94817245 = -456518426;    double uYHcsbyDDFRYUAqAg23215498 = -19112716;    double uYHcsbyDDFRYUAqAg42503835 = -297818618;    double uYHcsbyDDFRYUAqAg47258581 = -894711899;    double uYHcsbyDDFRYUAqAg10997006 = -365508373;    double uYHcsbyDDFRYUAqAg84399316 = -522981253;    double uYHcsbyDDFRYUAqAg19952406 = -547415983;    double uYHcsbyDDFRYUAqAg82067040 = -418509102;    double uYHcsbyDDFRYUAqAg94126680 = -302535382;    double uYHcsbyDDFRYUAqAg19775058 = -549930317;    double uYHcsbyDDFRYUAqAg71064786 = -875015205;    double uYHcsbyDDFRYUAqAg99558907 = -579281000;    double uYHcsbyDDFRYUAqAg81648471 = -513994645;    double uYHcsbyDDFRYUAqAg42749734 = -699412509;    double uYHcsbyDDFRYUAqAg93910362 = -810537428;    double uYHcsbyDDFRYUAqAg97930582 = -904472988;    double uYHcsbyDDFRYUAqAg19906777 = -705695649;    double uYHcsbyDDFRYUAqAg6779000 = -91332047;    double uYHcsbyDDFRYUAqAg31227774 = 39354445;    double uYHcsbyDDFRYUAqAg3393885 = -910689408;    double uYHcsbyDDFRYUAqAg85411334 = -604137229;    double uYHcsbyDDFRYUAqAg16435299 = -599711186;    double uYHcsbyDDFRYUAqAg20148138 = -254614173;    double uYHcsbyDDFRYUAqAg91913498 = -957041558;    double uYHcsbyDDFRYUAqAg37111962 = -459748095;    double uYHcsbyDDFRYUAqAg46807486 = 20669212;    double uYHcsbyDDFRYUAqAg11843132 = -473362437;    double uYHcsbyDDFRYUAqAg11373758 = -850686101;    double uYHcsbyDDFRYUAqAg88603049 = -435569027;    double uYHcsbyDDFRYUAqAg10911702 = -661922034;    double uYHcsbyDDFRYUAqAg15440825 = -94013752;    double uYHcsbyDDFRYUAqAg71465909 = -990990270;    double uYHcsbyDDFRYUAqAg94674844 = -847909046;    double uYHcsbyDDFRYUAqAg70978206 = -774484139;    double uYHcsbyDDFRYUAqAg58613184 = -632456263;    double uYHcsbyDDFRYUAqAg73931956 = -364265136;    double uYHcsbyDDFRYUAqAg77866404 = -999590367;    double uYHcsbyDDFRYUAqAg41088627 = -205227333;    double uYHcsbyDDFRYUAqAg28697264 = -415055673;    double uYHcsbyDDFRYUAqAg76236025 = -501008427;    double uYHcsbyDDFRYUAqAg40619085 = -394218048;    double uYHcsbyDDFRYUAqAg98256469 = -881766025;    double uYHcsbyDDFRYUAqAg49415605 = -944028736;    double uYHcsbyDDFRYUAqAg55327960 = -978082602;    double uYHcsbyDDFRYUAqAg85505466 = -242441191;    double uYHcsbyDDFRYUAqAg54291479 = -641208438;    double uYHcsbyDDFRYUAqAg99841835 = -85968610;    double uYHcsbyDDFRYUAqAg47971221 = -960225405;    double uYHcsbyDDFRYUAqAg29550052 = -479877387;    double uYHcsbyDDFRYUAqAg77745235 = 44951240;    double uYHcsbyDDFRYUAqAg22183491 = -941859533;    double uYHcsbyDDFRYUAqAg19225737 = -755950697;    double uYHcsbyDDFRYUAqAg7854557 = -211147553;    double uYHcsbyDDFRYUAqAg26180673 = -528379591;    double uYHcsbyDDFRYUAqAg28070290 = -450890285;    double uYHcsbyDDFRYUAqAg78381947 = -856807240;    double uYHcsbyDDFRYUAqAg3067360 = -764498544;    double uYHcsbyDDFRYUAqAg50590337 = -340777061;    double uYHcsbyDDFRYUAqAg10146619 = -334963805;    double uYHcsbyDDFRYUAqAg64189520 = -286177585;    double uYHcsbyDDFRYUAqAg72556184 = 50381184;    double uYHcsbyDDFRYUAqAg8578648 = -696729883;    double uYHcsbyDDFRYUAqAg93463991 = -982940075;    double uYHcsbyDDFRYUAqAg83214978 = -640613348;    double uYHcsbyDDFRYUAqAg4334233 = -355916566;    double uYHcsbyDDFRYUAqAg99598876 = -884024935;    double uYHcsbyDDFRYUAqAg4884064 = -731371955;    double uYHcsbyDDFRYUAqAg10670265 = -739510507;    double uYHcsbyDDFRYUAqAg84136550 = 33043753;    double uYHcsbyDDFRYUAqAg19978407 = -346272293;    double uYHcsbyDDFRYUAqAg20064178 = -904882621;    double uYHcsbyDDFRYUAqAg78818150 = -400468316;    double uYHcsbyDDFRYUAqAg78081735 = -676276374;    double uYHcsbyDDFRYUAqAg54991748 = -459637128;    double uYHcsbyDDFRYUAqAg62774799 = -416471360;    double uYHcsbyDDFRYUAqAg87154864 = -722371205;    double uYHcsbyDDFRYUAqAg67019693 = -655682451;    double uYHcsbyDDFRYUAqAg64820178 = -276531572;    double uYHcsbyDDFRYUAqAg6408032 = -614600367;    double uYHcsbyDDFRYUAqAg82820483 = -818539658;    double uYHcsbyDDFRYUAqAg46965650 = -893362179;    double uYHcsbyDDFRYUAqAg63871910 = -513137032;    double uYHcsbyDDFRYUAqAg81823706 = -270808714;    double uYHcsbyDDFRYUAqAg10857814 = -23989679;     uYHcsbyDDFRYUAqAg60635342 = uYHcsbyDDFRYUAqAg60571491;     uYHcsbyDDFRYUAqAg60571491 = uYHcsbyDDFRYUAqAg80323510;     uYHcsbyDDFRYUAqAg80323510 = uYHcsbyDDFRYUAqAg43542286;     uYHcsbyDDFRYUAqAg43542286 = uYHcsbyDDFRYUAqAg75103018;     uYHcsbyDDFRYUAqAg75103018 = uYHcsbyDDFRYUAqAg56570253;     uYHcsbyDDFRYUAqAg56570253 = uYHcsbyDDFRYUAqAg53850387;     uYHcsbyDDFRYUAqAg53850387 = uYHcsbyDDFRYUAqAg81490307;     uYHcsbyDDFRYUAqAg81490307 = uYHcsbyDDFRYUAqAg90720954;     uYHcsbyDDFRYUAqAg90720954 = uYHcsbyDDFRYUAqAg23460415;     uYHcsbyDDFRYUAqAg23460415 = uYHcsbyDDFRYUAqAg75675817;     uYHcsbyDDFRYUAqAg75675817 = uYHcsbyDDFRYUAqAg42090267;     uYHcsbyDDFRYUAqAg42090267 = uYHcsbyDDFRYUAqAg26004736;     uYHcsbyDDFRYUAqAg26004736 = uYHcsbyDDFRYUAqAg39082331;     uYHcsbyDDFRYUAqAg39082331 = uYHcsbyDDFRYUAqAg29574557;     uYHcsbyDDFRYUAqAg29574557 = uYHcsbyDDFRYUAqAg13481624;     uYHcsbyDDFRYUAqAg13481624 = uYHcsbyDDFRYUAqAg94817245;     uYHcsbyDDFRYUAqAg94817245 = uYHcsbyDDFRYUAqAg23215498;     uYHcsbyDDFRYUAqAg23215498 = uYHcsbyDDFRYUAqAg42503835;     uYHcsbyDDFRYUAqAg42503835 = uYHcsbyDDFRYUAqAg47258581;     uYHcsbyDDFRYUAqAg47258581 = uYHcsbyDDFRYUAqAg10997006;     uYHcsbyDDFRYUAqAg10997006 = uYHcsbyDDFRYUAqAg84399316;     uYHcsbyDDFRYUAqAg84399316 = uYHcsbyDDFRYUAqAg19952406;     uYHcsbyDDFRYUAqAg19952406 = uYHcsbyDDFRYUAqAg82067040;     uYHcsbyDDFRYUAqAg82067040 = uYHcsbyDDFRYUAqAg94126680;     uYHcsbyDDFRYUAqAg94126680 = uYHcsbyDDFRYUAqAg19775058;     uYHcsbyDDFRYUAqAg19775058 = uYHcsbyDDFRYUAqAg71064786;     uYHcsbyDDFRYUAqAg71064786 = uYHcsbyDDFRYUAqAg99558907;     uYHcsbyDDFRYUAqAg99558907 = uYHcsbyDDFRYUAqAg81648471;     uYHcsbyDDFRYUAqAg81648471 = uYHcsbyDDFRYUAqAg42749734;     uYHcsbyDDFRYUAqAg42749734 = uYHcsbyDDFRYUAqAg93910362;     uYHcsbyDDFRYUAqAg93910362 = uYHcsbyDDFRYUAqAg97930582;     uYHcsbyDDFRYUAqAg97930582 = uYHcsbyDDFRYUAqAg19906777;     uYHcsbyDDFRYUAqAg19906777 = uYHcsbyDDFRYUAqAg6779000;     uYHcsbyDDFRYUAqAg6779000 = uYHcsbyDDFRYUAqAg31227774;     uYHcsbyDDFRYUAqAg31227774 = uYHcsbyDDFRYUAqAg3393885;     uYHcsbyDDFRYUAqAg3393885 = uYHcsbyDDFRYUAqAg85411334;     uYHcsbyDDFRYUAqAg85411334 = uYHcsbyDDFRYUAqAg16435299;     uYHcsbyDDFRYUAqAg16435299 = uYHcsbyDDFRYUAqAg20148138;     uYHcsbyDDFRYUAqAg20148138 = uYHcsbyDDFRYUAqAg91913498;     uYHcsbyDDFRYUAqAg91913498 = uYHcsbyDDFRYUAqAg37111962;     uYHcsbyDDFRYUAqAg37111962 = uYHcsbyDDFRYUAqAg46807486;     uYHcsbyDDFRYUAqAg46807486 = uYHcsbyDDFRYUAqAg11843132;     uYHcsbyDDFRYUAqAg11843132 = uYHcsbyDDFRYUAqAg11373758;     uYHcsbyDDFRYUAqAg11373758 = uYHcsbyDDFRYUAqAg88603049;     uYHcsbyDDFRYUAqAg88603049 = uYHcsbyDDFRYUAqAg10911702;     uYHcsbyDDFRYUAqAg10911702 = uYHcsbyDDFRYUAqAg15440825;     uYHcsbyDDFRYUAqAg15440825 = uYHcsbyDDFRYUAqAg71465909;     uYHcsbyDDFRYUAqAg71465909 = uYHcsbyDDFRYUAqAg94674844;     uYHcsbyDDFRYUAqAg94674844 = uYHcsbyDDFRYUAqAg70978206;     uYHcsbyDDFRYUAqAg70978206 = uYHcsbyDDFRYUAqAg58613184;     uYHcsbyDDFRYUAqAg58613184 = uYHcsbyDDFRYUAqAg73931956;     uYHcsbyDDFRYUAqAg73931956 = uYHcsbyDDFRYUAqAg77866404;     uYHcsbyDDFRYUAqAg77866404 = uYHcsbyDDFRYUAqAg41088627;     uYHcsbyDDFRYUAqAg41088627 = uYHcsbyDDFRYUAqAg28697264;     uYHcsbyDDFRYUAqAg28697264 = uYHcsbyDDFRYUAqAg76236025;     uYHcsbyDDFRYUAqAg76236025 = uYHcsbyDDFRYUAqAg40619085;     uYHcsbyDDFRYUAqAg40619085 = uYHcsbyDDFRYUAqAg98256469;     uYHcsbyDDFRYUAqAg98256469 = uYHcsbyDDFRYUAqAg49415605;     uYHcsbyDDFRYUAqAg49415605 = uYHcsbyDDFRYUAqAg55327960;     uYHcsbyDDFRYUAqAg55327960 = uYHcsbyDDFRYUAqAg85505466;     uYHcsbyDDFRYUAqAg85505466 = uYHcsbyDDFRYUAqAg54291479;     uYHcsbyDDFRYUAqAg54291479 = uYHcsbyDDFRYUAqAg99841835;     uYHcsbyDDFRYUAqAg99841835 = uYHcsbyDDFRYUAqAg47971221;     uYHcsbyDDFRYUAqAg47971221 = uYHcsbyDDFRYUAqAg29550052;     uYHcsbyDDFRYUAqAg29550052 = uYHcsbyDDFRYUAqAg77745235;     uYHcsbyDDFRYUAqAg77745235 = uYHcsbyDDFRYUAqAg22183491;     uYHcsbyDDFRYUAqAg22183491 = uYHcsbyDDFRYUAqAg19225737;     uYHcsbyDDFRYUAqAg19225737 = uYHcsbyDDFRYUAqAg7854557;     uYHcsbyDDFRYUAqAg7854557 = uYHcsbyDDFRYUAqAg26180673;     uYHcsbyDDFRYUAqAg26180673 = uYHcsbyDDFRYUAqAg28070290;     uYHcsbyDDFRYUAqAg28070290 = uYHcsbyDDFRYUAqAg78381947;     uYHcsbyDDFRYUAqAg78381947 = uYHcsbyDDFRYUAqAg3067360;     uYHcsbyDDFRYUAqAg3067360 = uYHcsbyDDFRYUAqAg50590337;     uYHcsbyDDFRYUAqAg50590337 = uYHcsbyDDFRYUAqAg10146619;     uYHcsbyDDFRYUAqAg10146619 = uYHcsbyDDFRYUAqAg64189520;     uYHcsbyDDFRYUAqAg64189520 = uYHcsbyDDFRYUAqAg72556184;     uYHcsbyDDFRYUAqAg72556184 = uYHcsbyDDFRYUAqAg8578648;     uYHcsbyDDFRYUAqAg8578648 = uYHcsbyDDFRYUAqAg93463991;     uYHcsbyDDFRYUAqAg93463991 = uYHcsbyDDFRYUAqAg83214978;     uYHcsbyDDFRYUAqAg83214978 = uYHcsbyDDFRYUAqAg4334233;     uYHcsbyDDFRYUAqAg4334233 = uYHcsbyDDFRYUAqAg99598876;     uYHcsbyDDFRYUAqAg99598876 = uYHcsbyDDFRYUAqAg4884064;     uYHcsbyDDFRYUAqAg4884064 = uYHcsbyDDFRYUAqAg10670265;     uYHcsbyDDFRYUAqAg10670265 = uYHcsbyDDFRYUAqAg84136550;     uYHcsbyDDFRYUAqAg84136550 = uYHcsbyDDFRYUAqAg19978407;     uYHcsbyDDFRYUAqAg19978407 = uYHcsbyDDFRYUAqAg20064178;     uYHcsbyDDFRYUAqAg20064178 = uYHcsbyDDFRYUAqAg78818150;     uYHcsbyDDFRYUAqAg78818150 = uYHcsbyDDFRYUAqAg78081735;     uYHcsbyDDFRYUAqAg78081735 = uYHcsbyDDFRYUAqAg54991748;     uYHcsbyDDFRYUAqAg54991748 = uYHcsbyDDFRYUAqAg62774799;     uYHcsbyDDFRYUAqAg62774799 = uYHcsbyDDFRYUAqAg87154864;     uYHcsbyDDFRYUAqAg87154864 = uYHcsbyDDFRYUAqAg67019693;     uYHcsbyDDFRYUAqAg67019693 = uYHcsbyDDFRYUAqAg64820178;     uYHcsbyDDFRYUAqAg64820178 = uYHcsbyDDFRYUAqAg6408032;     uYHcsbyDDFRYUAqAg6408032 = uYHcsbyDDFRYUAqAg82820483;     uYHcsbyDDFRYUAqAg82820483 = uYHcsbyDDFRYUAqAg46965650;     uYHcsbyDDFRYUAqAg46965650 = uYHcsbyDDFRYUAqAg63871910;     uYHcsbyDDFRYUAqAg63871910 = uYHcsbyDDFRYUAqAg81823706;     uYHcsbyDDFRYUAqAg81823706 = uYHcsbyDDFRYUAqAg10857814;     uYHcsbyDDFRYUAqAg10857814 = uYHcsbyDDFRYUAqAg60635342;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pCktgVZpnqwOWaVv54743338() {     double xjivYEMxiYZeBTMRG66752448 = -135891568;    double xjivYEMxiYZeBTMRG3944771 = -751233812;    double xjivYEMxiYZeBTMRG96365694 = -451108047;    double xjivYEMxiYZeBTMRG70617067 = -437312747;    double xjivYEMxiYZeBTMRG43998506 = 51337211;    double xjivYEMxiYZeBTMRG47987455 = -381952630;    double xjivYEMxiYZeBTMRG14339353 = -359454414;    double xjivYEMxiYZeBTMRG31250198 = -986837168;    double xjivYEMxiYZeBTMRG55363450 = -427670580;    double xjivYEMxiYZeBTMRG64690074 = -619390293;    double xjivYEMxiYZeBTMRG35786108 = -836959333;    double xjivYEMxiYZeBTMRG73913309 = 8038617;    double xjivYEMxiYZeBTMRG12213855 = -287173831;    double xjivYEMxiYZeBTMRG22226007 = -804031675;    double xjivYEMxiYZeBTMRG25316984 = -29531165;    double xjivYEMxiYZeBTMRG54987018 = -28027056;    double xjivYEMxiYZeBTMRG88653249 = 26936462;    double xjivYEMxiYZeBTMRG20268651 = -784635359;    double xjivYEMxiYZeBTMRG4333677 = -703587715;    double xjivYEMxiYZeBTMRG27808746 = -151614819;    double xjivYEMxiYZeBTMRG99550199 = -220995829;    double xjivYEMxiYZeBTMRG70920339 = -430910969;    double xjivYEMxiYZeBTMRG55543197 = -293933470;    double xjivYEMxiYZeBTMRG66059414 = -640814090;    double xjivYEMxiYZeBTMRG37826331 = -675731415;    double xjivYEMxiYZeBTMRG44496733 = -428585640;    double xjivYEMxiYZeBTMRG69524564 = -775677408;    double xjivYEMxiYZeBTMRG12666033 = -645209954;    double xjivYEMxiYZeBTMRG47780727 = -736377350;    double xjivYEMxiYZeBTMRG11365371 = -516883376;    double xjivYEMxiYZeBTMRG86152912 = -723858366;    double xjivYEMxiYZeBTMRG95541204 = -613919803;    double xjivYEMxiYZeBTMRG29617725 = -598309019;    double xjivYEMxiYZeBTMRG1609564 = -942873812;    double xjivYEMxiYZeBTMRG92702425 = -599378660;    double xjivYEMxiYZeBTMRG72119797 = -24715516;    double xjivYEMxiYZeBTMRG66673268 = -557773098;    double xjivYEMxiYZeBTMRG50525511 = -371217196;    double xjivYEMxiYZeBTMRG93758391 = -248537670;    double xjivYEMxiYZeBTMRG79415436 = -330480780;    double xjivYEMxiYZeBTMRG42781186 = -525021989;    double xjivYEMxiYZeBTMRG78957942 = -249473066;    double xjivYEMxiYZeBTMRG89910109 = -554167021;    double xjivYEMxiYZeBTMRG98781498 = -870029039;    double xjivYEMxiYZeBTMRG555515 = -97467181;    double xjivYEMxiYZeBTMRG22850897 = -647138151;    double xjivYEMxiYZeBTMRG33219399 = -212789289;    double xjivYEMxiYZeBTMRG79422850 = 99114197;    double xjivYEMxiYZeBTMRG4350574 = -933646034;    double xjivYEMxiYZeBTMRG83889859 = -861494947;    double xjivYEMxiYZeBTMRG94313862 = -474749934;    double xjivYEMxiYZeBTMRG84741676 = -164991456;    double xjivYEMxiYZeBTMRG26114862 = -791379955;    double xjivYEMxiYZeBTMRG37057507 = -525697479;    double xjivYEMxiYZeBTMRG55193139 = -15587456;    double xjivYEMxiYZeBTMRG95832108 = -704980599;    double xjivYEMxiYZeBTMRG48401573 = -357300342;    double xjivYEMxiYZeBTMRG30306280 = -810293958;    double xjivYEMxiYZeBTMRG32790736 = -761581333;    double xjivYEMxiYZeBTMRG99501773 = -520077150;    double xjivYEMxiYZeBTMRG78462891 = -606275223;    double xjivYEMxiYZeBTMRG1673320 = -714244461;    double xjivYEMxiYZeBTMRG83469471 = -150459818;    double xjivYEMxiYZeBTMRG43998080 = -910787204;    double xjivYEMxiYZeBTMRG78537161 = -895531927;    double xjivYEMxiYZeBTMRG40244904 = -123039531;    double xjivYEMxiYZeBTMRG44295585 = -393652364;    double xjivYEMxiYZeBTMRG10604291 = -344300020;    double xjivYEMxiYZeBTMRG29523582 = -104653015;    double xjivYEMxiYZeBTMRG53197187 = 95184350;    double xjivYEMxiYZeBTMRG88313749 = -470253958;    double xjivYEMxiYZeBTMRG38127738 = -601846343;    double xjivYEMxiYZeBTMRG26510259 = -436097690;    double xjivYEMxiYZeBTMRG24918241 = -273106935;    double xjivYEMxiYZeBTMRG85027559 = -626592830;    double xjivYEMxiYZeBTMRG20592257 = -971522763;    double xjivYEMxiYZeBTMRG81010230 = -876743949;    double xjivYEMxiYZeBTMRG56761699 = -423904432;    double xjivYEMxiYZeBTMRG65503900 = -443346909;    double xjivYEMxiYZeBTMRG14975434 = 71406735;    double xjivYEMxiYZeBTMRG11277334 = -115796351;    double xjivYEMxiYZeBTMRG90101714 = -774791606;    double xjivYEMxiYZeBTMRG8315460 = -711563920;    double xjivYEMxiYZeBTMRG63890867 = -874882403;    double xjivYEMxiYZeBTMRG17051509 = 57866557;    double xjivYEMxiYZeBTMRG1411237 = -458866911;    double xjivYEMxiYZeBTMRG69426342 = -822539849;    double xjivYEMxiYZeBTMRG92560218 = 27388459;    double xjivYEMxiYZeBTMRG46416425 = -827286357;    double xjivYEMxiYZeBTMRG96870316 = -894398061;    double xjivYEMxiYZeBTMRG23718225 = -667415174;    double xjivYEMxiYZeBTMRG36366989 = -747479141;    double xjivYEMxiYZeBTMRG17734775 = -609635864;    double xjivYEMxiYZeBTMRG94256618 = -728460521;    double xjivYEMxiYZeBTMRG952546 = -724205558;    double xjivYEMxiYZeBTMRG41107866 = -810777528;    double xjivYEMxiYZeBTMRG95488471 = 986751;    double xjivYEMxiYZeBTMRG45912030 = -643379818;    double xjivYEMxiYZeBTMRG20244337 = -974497112;    double xjivYEMxiYZeBTMRG60310610 = -135891568;     xjivYEMxiYZeBTMRG66752448 = xjivYEMxiYZeBTMRG3944771;     xjivYEMxiYZeBTMRG3944771 = xjivYEMxiYZeBTMRG96365694;     xjivYEMxiYZeBTMRG96365694 = xjivYEMxiYZeBTMRG70617067;     xjivYEMxiYZeBTMRG70617067 = xjivYEMxiYZeBTMRG43998506;     xjivYEMxiYZeBTMRG43998506 = xjivYEMxiYZeBTMRG47987455;     xjivYEMxiYZeBTMRG47987455 = xjivYEMxiYZeBTMRG14339353;     xjivYEMxiYZeBTMRG14339353 = xjivYEMxiYZeBTMRG31250198;     xjivYEMxiYZeBTMRG31250198 = xjivYEMxiYZeBTMRG55363450;     xjivYEMxiYZeBTMRG55363450 = xjivYEMxiYZeBTMRG64690074;     xjivYEMxiYZeBTMRG64690074 = xjivYEMxiYZeBTMRG35786108;     xjivYEMxiYZeBTMRG35786108 = xjivYEMxiYZeBTMRG73913309;     xjivYEMxiYZeBTMRG73913309 = xjivYEMxiYZeBTMRG12213855;     xjivYEMxiYZeBTMRG12213855 = xjivYEMxiYZeBTMRG22226007;     xjivYEMxiYZeBTMRG22226007 = xjivYEMxiYZeBTMRG25316984;     xjivYEMxiYZeBTMRG25316984 = xjivYEMxiYZeBTMRG54987018;     xjivYEMxiYZeBTMRG54987018 = xjivYEMxiYZeBTMRG88653249;     xjivYEMxiYZeBTMRG88653249 = xjivYEMxiYZeBTMRG20268651;     xjivYEMxiYZeBTMRG20268651 = xjivYEMxiYZeBTMRG4333677;     xjivYEMxiYZeBTMRG4333677 = xjivYEMxiYZeBTMRG27808746;     xjivYEMxiYZeBTMRG27808746 = xjivYEMxiYZeBTMRG99550199;     xjivYEMxiYZeBTMRG99550199 = xjivYEMxiYZeBTMRG70920339;     xjivYEMxiYZeBTMRG70920339 = xjivYEMxiYZeBTMRG55543197;     xjivYEMxiYZeBTMRG55543197 = xjivYEMxiYZeBTMRG66059414;     xjivYEMxiYZeBTMRG66059414 = xjivYEMxiYZeBTMRG37826331;     xjivYEMxiYZeBTMRG37826331 = xjivYEMxiYZeBTMRG44496733;     xjivYEMxiYZeBTMRG44496733 = xjivYEMxiYZeBTMRG69524564;     xjivYEMxiYZeBTMRG69524564 = xjivYEMxiYZeBTMRG12666033;     xjivYEMxiYZeBTMRG12666033 = xjivYEMxiYZeBTMRG47780727;     xjivYEMxiYZeBTMRG47780727 = xjivYEMxiYZeBTMRG11365371;     xjivYEMxiYZeBTMRG11365371 = xjivYEMxiYZeBTMRG86152912;     xjivYEMxiYZeBTMRG86152912 = xjivYEMxiYZeBTMRG95541204;     xjivYEMxiYZeBTMRG95541204 = xjivYEMxiYZeBTMRG29617725;     xjivYEMxiYZeBTMRG29617725 = xjivYEMxiYZeBTMRG1609564;     xjivYEMxiYZeBTMRG1609564 = xjivYEMxiYZeBTMRG92702425;     xjivYEMxiYZeBTMRG92702425 = xjivYEMxiYZeBTMRG72119797;     xjivYEMxiYZeBTMRG72119797 = xjivYEMxiYZeBTMRG66673268;     xjivYEMxiYZeBTMRG66673268 = xjivYEMxiYZeBTMRG50525511;     xjivYEMxiYZeBTMRG50525511 = xjivYEMxiYZeBTMRG93758391;     xjivYEMxiYZeBTMRG93758391 = xjivYEMxiYZeBTMRG79415436;     xjivYEMxiYZeBTMRG79415436 = xjivYEMxiYZeBTMRG42781186;     xjivYEMxiYZeBTMRG42781186 = xjivYEMxiYZeBTMRG78957942;     xjivYEMxiYZeBTMRG78957942 = xjivYEMxiYZeBTMRG89910109;     xjivYEMxiYZeBTMRG89910109 = xjivYEMxiYZeBTMRG98781498;     xjivYEMxiYZeBTMRG98781498 = xjivYEMxiYZeBTMRG555515;     xjivYEMxiYZeBTMRG555515 = xjivYEMxiYZeBTMRG22850897;     xjivYEMxiYZeBTMRG22850897 = xjivYEMxiYZeBTMRG33219399;     xjivYEMxiYZeBTMRG33219399 = xjivYEMxiYZeBTMRG79422850;     xjivYEMxiYZeBTMRG79422850 = xjivYEMxiYZeBTMRG4350574;     xjivYEMxiYZeBTMRG4350574 = xjivYEMxiYZeBTMRG83889859;     xjivYEMxiYZeBTMRG83889859 = xjivYEMxiYZeBTMRG94313862;     xjivYEMxiYZeBTMRG94313862 = xjivYEMxiYZeBTMRG84741676;     xjivYEMxiYZeBTMRG84741676 = xjivYEMxiYZeBTMRG26114862;     xjivYEMxiYZeBTMRG26114862 = xjivYEMxiYZeBTMRG37057507;     xjivYEMxiYZeBTMRG37057507 = xjivYEMxiYZeBTMRG55193139;     xjivYEMxiYZeBTMRG55193139 = xjivYEMxiYZeBTMRG95832108;     xjivYEMxiYZeBTMRG95832108 = xjivYEMxiYZeBTMRG48401573;     xjivYEMxiYZeBTMRG48401573 = xjivYEMxiYZeBTMRG30306280;     xjivYEMxiYZeBTMRG30306280 = xjivYEMxiYZeBTMRG32790736;     xjivYEMxiYZeBTMRG32790736 = xjivYEMxiYZeBTMRG99501773;     xjivYEMxiYZeBTMRG99501773 = xjivYEMxiYZeBTMRG78462891;     xjivYEMxiYZeBTMRG78462891 = xjivYEMxiYZeBTMRG1673320;     xjivYEMxiYZeBTMRG1673320 = xjivYEMxiYZeBTMRG83469471;     xjivYEMxiYZeBTMRG83469471 = xjivYEMxiYZeBTMRG43998080;     xjivYEMxiYZeBTMRG43998080 = xjivYEMxiYZeBTMRG78537161;     xjivYEMxiYZeBTMRG78537161 = xjivYEMxiYZeBTMRG40244904;     xjivYEMxiYZeBTMRG40244904 = xjivYEMxiYZeBTMRG44295585;     xjivYEMxiYZeBTMRG44295585 = xjivYEMxiYZeBTMRG10604291;     xjivYEMxiYZeBTMRG10604291 = xjivYEMxiYZeBTMRG29523582;     xjivYEMxiYZeBTMRG29523582 = xjivYEMxiYZeBTMRG53197187;     xjivYEMxiYZeBTMRG53197187 = xjivYEMxiYZeBTMRG88313749;     xjivYEMxiYZeBTMRG88313749 = xjivYEMxiYZeBTMRG38127738;     xjivYEMxiYZeBTMRG38127738 = xjivYEMxiYZeBTMRG26510259;     xjivYEMxiYZeBTMRG26510259 = xjivYEMxiYZeBTMRG24918241;     xjivYEMxiYZeBTMRG24918241 = xjivYEMxiYZeBTMRG85027559;     xjivYEMxiYZeBTMRG85027559 = xjivYEMxiYZeBTMRG20592257;     xjivYEMxiYZeBTMRG20592257 = xjivYEMxiYZeBTMRG81010230;     xjivYEMxiYZeBTMRG81010230 = xjivYEMxiYZeBTMRG56761699;     xjivYEMxiYZeBTMRG56761699 = xjivYEMxiYZeBTMRG65503900;     xjivYEMxiYZeBTMRG65503900 = xjivYEMxiYZeBTMRG14975434;     xjivYEMxiYZeBTMRG14975434 = xjivYEMxiYZeBTMRG11277334;     xjivYEMxiYZeBTMRG11277334 = xjivYEMxiYZeBTMRG90101714;     xjivYEMxiYZeBTMRG90101714 = xjivYEMxiYZeBTMRG8315460;     xjivYEMxiYZeBTMRG8315460 = xjivYEMxiYZeBTMRG63890867;     xjivYEMxiYZeBTMRG63890867 = xjivYEMxiYZeBTMRG17051509;     xjivYEMxiYZeBTMRG17051509 = xjivYEMxiYZeBTMRG1411237;     xjivYEMxiYZeBTMRG1411237 = xjivYEMxiYZeBTMRG69426342;     xjivYEMxiYZeBTMRG69426342 = xjivYEMxiYZeBTMRG92560218;     xjivYEMxiYZeBTMRG92560218 = xjivYEMxiYZeBTMRG46416425;     xjivYEMxiYZeBTMRG46416425 = xjivYEMxiYZeBTMRG96870316;     xjivYEMxiYZeBTMRG96870316 = xjivYEMxiYZeBTMRG23718225;     xjivYEMxiYZeBTMRG23718225 = xjivYEMxiYZeBTMRG36366989;     xjivYEMxiYZeBTMRG36366989 = xjivYEMxiYZeBTMRG17734775;     xjivYEMxiYZeBTMRG17734775 = xjivYEMxiYZeBTMRG94256618;     xjivYEMxiYZeBTMRG94256618 = xjivYEMxiYZeBTMRG952546;     xjivYEMxiYZeBTMRG952546 = xjivYEMxiYZeBTMRG41107866;     xjivYEMxiYZeBTMRG41107866 = xjivYEMxiYZeBTMRG95488471;     xjivYEMxiYZeBTMRG95488471 = xjivYEMxiYZeBTMRG45912030;     xjivYEMxiYZeBTMRG45912030 = xjivYEMxiYZeBTMRG20244337;     xjivYEMxiYZeBTMRG20244337 = xjivYEMxiYZeBTMRG60310610;     xjivYEMxiYZeBTMRG60310610 = xjivYEMxiYZeBTMRG66752448;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uMZOXBYxPgiiNHGE22034937() {     double jEbKUYtaEtzfAcFVU2211417 = -602168468;    double jEbKUYtaEtzfAcFVU17216849 = -631785419;    double jEbKUYtaEtzfAcFVU33752171 = -116775122;    double jEbKUYtaEtzfAcFVU42840422 = -258463819;    double jEbKUYtaEtzfAcFVU283776 = -54261635;    double jEbKUYtaEtzfAcFVU84210327 = -644300526;    double jEbKUYtaEtzfAcFVU24996760 = -204180148;    double jEbKUYtaEtzfAcFVU93381758 = -631071308;    double jEbKUYtaEtzfAcFVU56881055 = -916001987;    double jEbKUYtaEtzfAcFVU79592786 = -284077291;    double jEbKUYtaEtzfAcFVU73967842 = -638967063;    double jEbKUYtaEtzfAcFVU74481458 = -956082155;    double jEbKUYtaEtzfAcFVU50448500 = -839186336;    double jEbKUYtaEtzfAcFVU75366870 = -780395084;    double jEbKUYtaEtzfAcFVU83511500 = -532956180;    double jEbKUYtaEtzfAcFVU38528319 = -763838663;    double jEbKUYtaEtzfAcFVU44489185 = -500038091;    double jEbKUYtaEtzfAcFVU20028843 = -858063428;    double jEbKUYtaEtzfAcFVU4884007 = 79848864;    double jEbKUYtaEtzfAcFVU21645964 = -578175885;    double jEbKUYtaEtzfAcFVU12637062 = -978270518;    double jEbKUYtaEtzfAcFVU74341864 = -517547263;    double jEbKUYtaEtzfAcFVU26068577 = 68434346;    double jEbKUYtaEtzfAcFVU59852360 = -51638694;    double jEbKUYtaEtzfAcFVU45844282 = -699716613;    double jEbKUYtaEtzfAcFVU55194905 = -117162447;    double jEbKUYtaEtzfAcFVU29945138 = -581961549;    double jEbKUYtaEtzfAcFVU53394458 = -814982128;    double jEbKUYtaEtzfAcFVU94448941 = -210472152;    double jEbKUYtaEtzfAcFVU16898782 = -551390547;    double jEbKUYtaEtzfAcFVU4736144 = -175838853;    double jEbKUYtaEtzfAcFVU33364223 = -386891218;    double jEbKUYtaEtzfAcFVU41835736 = -745904996;    double jEbKUYtaEtzfAcFVU31357990 = 92432375;    double jEbKUYtaEtzfAcFVU58510879 = -550478911;    double jEbKUYtaEtzfAcFVU23680641 = -239280822;    double jEbKUYtaEtzfAcFVU16085085 = -422469139;    double jEbKUYtaEtzfAcFVU1740915 = -353872777;    double jEbKUYtaEtzfAcFVU24148954 = -999413946;    double jEbKUYtaEtzfAcFVU43686968 = -895743326;    double jEbKUYtaEtzfAcFVU76232794 = -294220903;    double jEbKUYtaEtzfAcFVU68323069 = -944018357;    double jEbKUYtaEtzfAcFVU79293488 = -107347807;    double jEbKUYtaEtzfAcFVU38912837 = -374361449;    double jEbKUYtaEtzfAcFVU571973 = 76021657;    double jEbKUYtaEtzfAcFVU68812225 = -686061783;    double jEbKUYtaEtzfAcFVU52564563 = -696775650;    double jEbKUYtaEtzfAcFVU97149161 = -67412396;    double jEbKUYtaEtzfAcFVU27243240 = -597382914;    double jEbKUYtaEtzfAcFVU8419449 = -252521763;    double jEbKUYtaEtzfAcFVU75084586 = -349026492;    double jEbKUYtaEtzfAcFVU68227148 = -378643802;    double jEbKUYtaEtzfAcFVU94736010 = -547425509;    double jEbKUYtaEtzfAcFVU53049353 = -717205978;    double jEbKUYtaEtzfAcFVU9397876 = -362077602;    double jEbKUYtaEtzfAcFVU27869553 = 15378795;    double jEbKUYtaEtzfAcFVU91148271 = -600219765;    double jEbKUYtaEtzfAcFVU73899811 = 34863571;    double jEbKUYtaEtzfAcFVU96996139 = -558747206;    double jEbKUYtaEtzfAcFVU45088871 = -937099188;    double jEbKUYtaEtzfAcFVU54265189 = 37661022;    double jEbKUYtaEtzfAcFVU71602301 = -389198021;    double jEbKUYtaEtzfAcFVU98932817 = -320599156;    double jEbKUYtaEtzfAcFVU39982274 = -264611440;    double jEbKUYtaEtzfAcFVU74856642 = -8238438;    double jEbKUYtaEtzfAcFVU40603620 = -152075846;    double jEbKUYtaEtzfAcFVU32645722 = -110177159;    double jEbKUYtaEtzfAcFVU19090510 = -831618712;    double jEbKUYtaEtzfAcFVU16855991 = -129916174;    double jEbKUYtaEtzfAcFVU59830859 = -193675359;    double jEbKUYtaEtzfAcFVU22443234 = -241369525;    double jEbKUYtaEtzfAcFVU42748270 = -46165314;    double jEbKUYtaEtzfAcFVU95879889 = -858649482;    double jEbKUYtaEtzfAcFVU61197038 = -24407810;    double jEbKUYtaEtzfAcFVU45413170 = -183954982;    double jEbKUYtaEtzfAcFVU44313993 = 65747839;    double jEbKUYtaEtzfAcFVU95048376 = -310199457;    double jEbKUYtaEtzfAcFVU87155740 = -557204206;    double jEbKUYtaEtzfAcFVU59280387 = -27660352;    double jEbKUYtaEtzfAcFVU77032056 = 86345169;    double jEbKUYtaEtzfAcFVU2630342 = -420386797;    double jEbKUYtaEtzfAcFVU32795977 = -414549153;    double jEbKUYtaEtzfAcFVU26151219 = -117599214;    double jEbKUYtaEtzfAcFVU86029492 = -957950390;    double jEbKUYtaEtzfAcFVU41814195 = -102364056;    double jEbKUYtaEtzfAcFVU36508996 = -797195051;    double jEbKUYtaEtzfAcFVU38628212 = -839465709;    double jEbKUYtaEtzfAcFVU88786382 = 71300981;    double jEbKUYtaEtzfAcFVU21960115 = -545490024;    double jEbKUYtaEtzfAcFVU30641327 = -465857706;    double jEbKUYtaEtzfAcFVU32532370 = -639061057;    double jEbKUYtaEtzfAcFVU42185274 = -357332710;    double jEbKUYtaEtzfAcFVU4744775 = -795125572;    double jEbKUYtaEtzfAcFVU79060082 = 37685242;    double jEbKUYtaEtzfAcFVU89421779 = -833404349;    double jEbKUYtaEtzfAcFVU4630493 = -905022883;    double jEbKUYtaEtzfAcFVU69390252 = -523419202;    double jEbKUYtaEtzfAcFVU39311214 = -842736367;    double jEbKUYtaEtzfAcFVU64056194 = -266123011;    double jEbKUYtaEtzfAcFVU59968353 = -602168468;     jEbKUYtaEtzfAcFVU2211417 = jEbKUYtaEtzfAcFVU17216849;     jEbKUYtaEtzfAcFVU17216849 = jEbKUYtaEtzfAcFVU33752171;     jEbKUYtaEtzfAcFVU33752171 = jEbKUYtaEtzfAcFVU42840422;     jEbKUYtaEtzfAcFVU42840422 = jEbKUYtaEtzfAcFVU283776;     jEbKUYtaEtzfAcFVU283776 = jEbKUYtaEtzfAcFVU84210327;     jEbKUYtaEtzfAcFVU84210327 = jEbKUYtaEtzfAcFVU24996760;     jEbKUYtaEtzfAcFVU24996760 = jEbKUYtaEtzfAcFVU93381758;     jEbKUYtaEtzfAcFVU93381758 = jEbKUYtaEtzfAcFVU56881055;     jEbKUYtaEtzfAcFVU56881055 = jEbKUYtaEtzfAcFVU79592786;     jEbKUYtaEtzfAcFVU79592786 = jEbKUYtaEtzfAcFVU73967842;     jEbKUYtaEtzfAcFVU73967842 = jEbKUYtaEtzfAcFVU74481458;     jEbKUYtaEtzfAcFVU74481458 = jEbKUYtaEtzfAcFVU50448500;     jEbKUYtaEtzfAcFVU50448500 = jEbKUYtaEtzfAcFVU75366870;     jEbKUYtaEtzfAcFVU75366870 = jEbKUYtaEtzfAcFVU83511500;     jEbKUYtaEtzfAcFVU83511500 = jEbKUYtaEtzfAcFVU38528319;     jEbKUYtaEtzfAcFVU38528319 = jEbKUYtaEtzfAcFVU44489185;     jEbKUYtaEtzfAcFVU44489185 = jEbKUYtaEtzfAcFVU20028843;     jEbKUYtaEtzfAcFVU20028843 = jEbKUYtaEtzfAcFVU4884007;     jEbKUYtaEtzfAcFVU4884007 = jEbKUYtaEtzfAcFVU21645964;     jEbKUYtaEtzfAcFVU21645964 = jEbKUYtaEtzfAcFVU12637062;     jEbKUYtaEtzfAcFVU12637062 = jEbKUYtaEtzfAcFVU74341864;     jEbKUYtaEtzfAcFVU74341864 = jEbKUYtaEtzfAcFVU26068577;     jEbKUYtaEtzfAcFVU26068577 = jEbKUYtaEtzfAcFVU59852360;     jEbKUYtaEtzfAcFVU59852360 = jEbKUYtaEtzfAcFVU45844282;     jEbKUYtaEtzfAcFVU45844282 = jEbKUYtaEtzfAcFVU55194905;     jEbKUYtaEtzfAcFVU55194905 = jEbKUYtaEtzfAcFVU29945138;     jEbKUYtaEtzfAcFVU29945138 = jEbKUYtaEtzfAcFVU53394458;     jEbKUYtaEtzfAcFVU53394458 = jEbKUYtaEtzfAcFVU94448941;     jEbKUYtaEtzfAcFVU94448941 = jEbKUYtaEtzfAcFVU16898782;     jEbKUYtaEtzfAcFVU16898782 = jEbKUYtaEtzfAcFVU4736144;     jEbKUYtaEtzfAcFVU4736144 = jEbKUYtaEtzfAcFVU33364223;     jEbKUYtaEtzfAcFVU33364223 = jEbKUYtaEtzfAcFVU41835736;     jEbKUYtaEtzfAcFVU41835736 = jEbKUYtaEtzfAcFVU31357990;     jEbKUYtaEtzfAcFVU31357990 = jEbKUYtaEtzfAcFVU58510879;     jEbKUYtaEtzfAcFVU58510879 = jEbKUYtaEtzfAcFVU23680641;     jEbKUYtaEtzfAcFVU23680641 = jEbKUYtaEtzfAcFVU16085085;     jEbKUYtaEtzfAcFVU16085085 = jEbKUYtaEtzfAcFVU1740915;     jEbKUYtaEtzfAcFVU1740915 = jEbKUYtaEtzfAcFVU24148954;     jEbKUYtaEtzfAcFVU24148954 = jEbKUYtaEtzfAcFVU43686968;     jEbKUYtaEtzfAcFVU43686968 = jEbKUYtaEtzfAcFVU76232794;     jEbKUYtaEtzfAcFVU76232794 = jEbKUYtaEtzfAcFVU68323069;     jEbKUYtaEtzfAcFVU68323069 = jEbKUYtaEtzfAcFVU79293488;     jEbKUYtaEtzfAcFVU79293488 = jEbKUYtaEtzfAcFVU38912837;     jEbKUYtaEtzfAcFVU38912837 = jEbKUYtaEtzfAcFVU571973;     jEbKUYtaEtzfAcFVU571973 = jEbKUYtaEtzfAcFVU68812225;     jEbKUYtaEtzfAcFVU68812225 = jEbKUYtaEtzfAcFVU52564563;     jEbKUYtaEtzfAcFVU52564563 = jEbKUYtaEtzfAcFVU97149161;     jEbKUYtaEtzfAcFVU97149161 = jEbKUYtaEtzfAcFVU27243240;     jEbKUYtaEtzfAcFVU27243240 = jEbKUYtaEtzfAcFVU8419449;     jEbKUYtaEtzfAcFVU8419449 = jEbKUYtaEtzfAcFVU75084586;     jEbKUYtaEtzfAcFVU75084586 = jEbKUYtaEtzfAcFVU68227148;     jEbKUYtaEtzfAcFVU68227148 = jEbKUYtaEtzfAcFVU94736010;     jEbKUYtaEtzfAcFVU94736010 = jEbKUYtaEtzfAcFVU53049353;     jEbKUYtaEtzfAcFVU53049353 = jEbKUYtaEtzfAcFVU9397876;     jEbKUYtaEtzfAcFVU9397876 = jEbKUYtaEtzfAcFVU27869553;     jEbKUYtaEtzfAcFVU27869553 = jEbKUYtaEtzfAcFVU91148271;     jEbKUYtaEtzfAcFVU91148271 = jEbKUYtaEtzfAcFVU73899811;     jEbKUYtaEtzfAcFVU73899811 = jEbKUYtaEtzfAcFVU96996139;     jEbKUYtaEtzfAcFVU96996139 = jEbKUYtaEtzfAcFVU45088871;     jEbKUYtaEtzfAcFVU45088871 = jEbKUYtaEtzfAcFVU54265189;     jEbKUYtaEtzfAcFVU54265189 = jEbKUYtaEtzfAcFVU71602301;     jEbKUYtaEtzfAcFVU71602301 = jEbKUYtaEtzfAcFVU98932817;     jEbKUYtaEtzfAcFVU98932817 = jEbKUYtaEtzfAcFVU39982274;     jEbKUYtaEtzfAcFVU39982274 = jEbKUYtaEtzfAcFVU74856642;     jEbKUYtaEtzfAcFVU74856642 = jEbKUYtaEtzfAcFVU40603620;     jEbKUYtaEtzfAcFVU40603620 = jEbKUYtaEtzfAcFVU32645722;     jEbKUYtaEtzfAcFVU32645722 = jEbKUYtaEtzfAcFVU19090510;     jEbKUYtaEtzfAcFVU19090510 = jEbKUYtaEtzfAcFVU16855991;     jEbKUYtaEtzfAcFVU16855991 = jEbKUYtaEtzfAcFVU59830859;     jEbKUYtaEtzfAcFVU59830859 = jEbKUYtaEtzfAcFVU22443234;     jEbKUYtaEtzfAcFVU22443234 = jEbKUYtaEtzfAcFVU42748270;     jEbKUYtaEtzfAcFVU42748270 = jEbKUYtaEtzfAcFVU95879889;     jEbKUYtaEtzfAcFVU95879889 = jEbKUYtaEtzfAcFVU61197038;     jEbKUYtaEtzfAcFVU61197038 = jEbKUYtaEtzfAcFVU45413170;     jEbKUYtaEtzfAcFVU45413170 = jEbKUYtaEtzfAcFVU44313993;     jEbKUYtaEtzfAcFVU44313993 = jEbKUYtaEtzfAcFVU95048376;     jEbKUYtaEtzfAcFVU95048376 = jEbKUYtaEtzfAcFVU87155740;     jEbKUYtaEtzfAcFVU87155740 = jEbKUYtaEtzfAcFVU59280387;     jEbKUYtaEtzfAcFVU59280387 = jEbKUYtaEtzfAcFVU77032056;     jEbKUYtaEtzfAcFVU77032056 = jEbKUYtaEtzfAcFVU2630342;     jEbKUYtaEtzfAcFVU2630342 = jEbKUYtaEtzfAcFVU32795977;     jEbKUYtaEtzfAcFVU32795977 = jEbKUYtaEtzfAcFVU26151219;     jEbKUYtaEtzfAcFVU26151219 = jEbKUYtaEtzfAcFVU86029492;     jEbKUYtaEtzfAcFVU86029492 = jEbKUYtaEtzfAcFVU41814195;     jEbKUYtaEtzfAcFVU41814195 = jEbKUYtaEtzfAcFVU36508996;     jEbKUYtaEtzfAcFVU36508996 = jEbKUYtaEtzfAcFVU38628212;     jEbKUYtaEtzfAcFVU38628212 = jEbKUYtaEtzfAcFVU88786382;     jEbKUYtaEtzfAcFVU88786382 = jEbKUYtaEtzfAcFVU21960115;     jEbKUYtaEtzfAcFVU21960115 = jEbKUYtaEtzfAcFVU30641327;     jEbKUYtaEtzfAcFVU30641327 = jEbKUYtaEtzfAcFVU32532370;     jEbKUYtaEtzfAcFVU32532370 = jEbKUYtaEtzfAcFVU42185274;     jEbKUYtaEtzfAcFVU42185274 = jEbKUYtaEtzfAcFVU4744775;     jEbKUYtaEtzfAcFVU4744775 = jEbKUYtaEtzfAcFVU79060082;     jEbKUYtaEtzfAcFVU79060082 = jEbKUYtaEtzfAcFVU89421779;     jEbKUYtaEtzfAcFVU89421779 = jEbKUYtaEtzfAcFVU4630493;     jEbKUYtaEtzfAcFVU4630493 = jEbKUYtaEtzfAcFVU69390252;     jEbKUYtaEtzfAcFVU69390252 = jEbKUYtaEtzfAcFVU39311214;     jEbKUYtaEtzfAcFVU39311214 = jEbKUYtaEtzfAcFVU64056194;     jEbKUYtaEtzfAcFVU64056194 = jEbKUYtaEtzfAcFVU59968353;     jEbKUYtaEtzfAcFVU59968353 = jEbKUYtaEtzfAcFVU2211417;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YGJSLwzEfpPRgVkU37084005() {     double iNdhllaeVqfnZGOsk8328524 = -714070357;    double iNdhllaeVqfnZGOsk60590128 = -341385199;    double iNdhllaeVqfnZGOsk49794355 = -267608043;    double iNdhllaeVqfnZGOsk69915202 = -449212449;    double iNdhllaeVqfnZGOsk69179264 = -574911506;    double iNdhllaeVqfnZGOsk75627529 = -908796761;    double iNdhllaeVqfnZGOsk85485725 = -343145126;    double iNdhllaeVqfnZGOsk43141650 = -917945221;    double iNdhllaeVqfnZGOsk21523551 = -684034652;    double iNdhllaeVqfnZGOsk20822446 = -613052768;    double iNdhllaeVqfnZGOsk34078133 = -516404649;    double iNdhllaeVqfnZGOsk6304501 = -300488356;    double iNdhllaeVqfnZGOsk36657618 = -179077425;    double iNdhllaeVqfnZGOsk58510547 = -212633652;    double iNdhllaeVqfnZGOsk79253927 = -123418347;    double iNdhllaeVqfnZGOsk80033712 = -736838207;    double iNdhllaeVqfnZGOsk38325188 = -16583204;    double iNdhllaeVqfnZGOsk17081996 = -523586070;    double iNdhllaeVqfnZGOsk66713848 = -325920233;    double iNdhllaeVqfnZGOsk2196129 = -935078804;    double iNdhllaeVqfnZGOsk1190256 = -833757973;    double iNdhllaeVqfnZGOsk60862888 = -425476980;    double iNdhllaeVqfnZGOsk61659369 = -778083141;    double iNdhllaeVqfnZGOsk43844734 = -273943682;    double iNdhllaeVqfnZGOsk89543932 = 27087354;    double iNdhllaeVqfnZGOsk79916580 = 4182230;    double iNdhllaeVqfnZGOsk28404917 = -482623752;    double iNdhllaeVqfnZGOsk66501583 = -880911082;    double iNdhllaeVqfnZGOsk60581197 = -432854857;    double iNdhllaeVqfnZGOsk85514418 = -368861414;    double iNdhllaeVqfnZGOsk96978693 = -89159792;    double iNdhllaeVqfnZGOsk30974845 = -96338033;    double iNdhllaeVqfnZGOsk51546684 = -638518367;    double iNdhllaeVqfnZGOsk26188554 = -759109390;    double iNdhllaeVqfnZGOsk19985532 = -89212016;    double iNdhllaeVqfnZGOsk92406554 = -453306929;    double iNdhllaeVqfnZGOsk97347019 = -376105008;    double iNdhllaeVqfnZGOsk35831127 = -125378787;    double iNdhllaeVqfnZGOsk97759207 = -993337443;    double iNdhllaeVqfnZGOsk31188906 = -269182549;    double iNdhllaeVqfnZGOsk81902018 = -359494797;    double iNdhllaeVqfnZGOsk473527 = -114160635;    double iNdhllaeVqfnZGOsk57360466 = -188152391;    double iNdhllaeVqfnZGOsk26320577 = -393704387;    double iNdhllaeVqfnZGOsk12524438 = -685876497;    double iNdhllaeVqfnZGOsk80751420 = -671277899;    double iNdhllaeVqfnZGOsk70343137 = -815551188;    double iNdhllaeVqfnZGOsk5106103 = -77307928;    double iNdhllaeVqfnZGOsk36918968 = -683119902;    double iNdhllaeVqfnZGOsk21331101 = -339532571;    double iNdhllaeVqfnZGOsk10785265 = -191320163;    double iNdhllaeVqfnZGOsk79036868 = -179370123;    double iNdhllaeVqfnZGOsk42984468 = -339215097;    double iNdhllaeVqfnZGOsk49018234 = 62323877;    double iNdhllaeVqfnZGOsk35893750 = 37390616;    double iNdhllaeVqfnZGOsk47465635 = -188593378;    double iNdhllaeVqfnZGOsk98930759 = -563302059;    double iNdhllaeVqfnZGOsk5949621 = -993664362;    double iNdhllaeVqfnZGOsk80371270 = -376299803;    double iNdhllaeVqfnZGOsk89262684 = -479093736;    double iNdhllaeVqfnZGOsk47222613 = -326173010;    double iNdhllaeVqfnZGOsk18984142 = -462234044;    double iNdhllaeVqfnZGOsk82560453 = -385090365;    double iNdhllaeVqfnZGOsk36009133 = -215173238;    double iNdhllaeVqfnZGOsk23843752 = -423892977;    double iNdhllaeVqfnZGOsk3103289 = -320066616;    double iNdhllaeVqfnZGOsk54757816 = -661969990;    double iNdhllaeVqfnZGOsk10469065 = -419968035;    double iNdhllaeVqfnZGOsk38525016 = -23421636;    double iNdhllaeVqfnZGOsk86847372 = -670111418;    double iNdhllaeVqfnZGOsk82686692 = -260733199;    double iNdhllaeVqfnZGOsk2494061 = -891204418;    double iNdhllaeVqfnZGOsk19322789 = -530248628;    double iNdhllaeVqfnZGOsk35524943 = 43262315;    double iNdhllaeVqfnZGOsk20294111 = -475584007;    double iNdhllaeVqfnZGOsk716729 = -619597338;    double iNdhllaeVqfnZGOsk3502422 = -137324589;    double iNdhllaeVqfnZGOsk35338792 = -284378755;    double iNdhllaeVqfnZGOsk31320296 = -588067185;    double iNdhllaeVqfnZGOsk8792512 = -301634747;    double iNdhllaeVqfnZGOsk9573443 = -180266583;    double iNdhllaeVqfnZGOsk23298814 = -305315824;    double iNdhllaeVqfnZGOsk29582615 = -97791180;    double iNdhllaeVqfnZGOsk39250096 = 6677714;    double iNdhllaeVqfnZGOsk74729154 = -77541252;    double iNdhllaeVqfnZGOsk17941826 = -909789669;    double iNdhllaeVqfnZGOsk87990376 = -757122937;    double iNdhllaeVqfnZGOsk2528451 = -600842244;    double iNdhllaeVqfnZGOsk90294804 = -696500007;    double iNdhllaeVqfnZGOsk72519896 = -900618639;    double iNdhllaeVqfnZGOsk93475795 = -890004871;    double iNdhllaeVqfnZGOsk91397399 = -382440647;    double iNdhllaeVqfnZGOsk55459856 = -749078984;    double iNdhllaeVqfnZGOsk8496523 = -414243707;    double iNdhllaeVqfnZGOsk83966293 = -943009540;    double iNdhllaeVqfnZGOsk62917876 = -897260753;    double iNdhllaeVqfnZGOsk17913074 = -729070271;    double iNdhllaeVqfnZGOsk21351334 = -972979153;    double iNdhllaeVqfnZGOsk2476825 = -969811410;    double iNdhllaeVqfnZGOsk9421150 = -714070357;     iNdhllaeVqfnZGOsk8328524 = iNdhllaeVqfnZGOsk60590128;     iNdhllaeVqfnZGOsk60590128 = iNdhllaeVqfnZGOsk49794355;     iNdhllaeVqfnZGOsk49794355 = iNdhllaeVqfnZGOsk69915202;     iNdhllaeVqfnZGOsk69915202 = iNdhllaeVqfnZGOsk69179264;     iNdhllaeVqfnZGOsk69179264 = iNdhllaeVqfnZGOsk75627529;     iNdhllaeVqfnZGOsk75627529 = iNdhllaeVqfnZGOsk85485725;     iNdhllaeVqfnZGOsk85485725 = iNdhllaeVqfnZGOsk43141650;     iNdhllaeVqfnZGOsk43141650 = iNdhllaeVqfnZGOsk21523551;     iNdhllaeVqfnZGOsk21523551 = iNdhllaeVqfnZGOsk20822446;     iNdhllaeVqfnZGOsk20822446 = iNdhllaeVqfnZGOsk34078133;     iNdhllaeVqfnZGOsk34078133 = iNdhllaeVqfnZGOsk6304501;     iNdhllaeVqfnZGOsk6304501 = iNdhllaeVqfnZGOsk36657618;     iNdhllaeVqfnZGOsk36657618 = iNdhllaeVqfnZGOsk58510547;     iNdhllaeVqfnZGOsk58510547 = iNdhllaeVqfnZGOsk79253927;     iNdhllaeVqfnZGOsk79253927 = iNdhllaeVqfnZGOsk80033712;     iNdhllaeVqfnZGOsk80033712 = iNdhllaeVqfnZGOsk38325188;     iNdhllaeVqfnZGOsk38325188 = iNdhllaeVqfnZGOsk17081996;     iNdhllaeVqfnZGOsk17081996 = iNdhllaeVqfnZGOsk66713848;     iNdhllaeVqfnZGOsk66713848 = iNdhllaeVqfnZGOsk2196129;     iNdhllaeVqfnZGOsk2196129 = iNdhllaeVqfnZGOsk1190256;     iNdhllaeVqfnZGOsk1190256 = iNdhllaeVqfnZGOsk60862888;     iNdhllaeVqfnZGOsk60862888 = iNdhllaeVqfnZGOsk61659369;     iNdhllaeVqfnZGOsk61659369 = iNdhllaeVqfnZGOsk43844734;     iNdhllaeVqfnZGOsk43844734 = iNdhllaeVqfnZGOsk89543932;     iNdhllaeVqfnZGOsk89543932 = iNdhllaeVqfnZGOsk79916580;     iNdhllaeVqfnZGOsk79916580 = iNdhllaeVqfnZGOsk28404917;     iNdhllaeVqfnZGOsk28404917 = iNdhllaeVqfnZGOsk66501583;     iNdhllaeVqfnZGOsk66501583 = iNdhllaeVqfnZGOsk60581197;     iNdhllaeVqfnZGOsk60581197 = iNdhllaeVqfnZGOsk85514418;     iNdhllaeVqfnZGOsk85514418 = iNdhllaeVqfnZGOsk96978693;     iNdhllaeVqfnZGOsk96978693 = iNdhllaeVqfnZGOsk30974845;     iNdhllaeVqfnZGOsk30974845 = iNdhllaeVqfnZGOsk51546684;     iNdhllaeVqfnZGOsk51546684 = iNdhllaeVqfnZGOsk26188554;     iNdhllaeVqfnZGOsk26188554 = iNdhllaeVqfnZGOsk19985532;     iNdhllaeVqfnZGOsk19985532 = iNdhllaeVqfnZGOsk92406554;     iNdhllaeVqfnZGOsk92406554 = iNdhllaeVqfnZGOsk97347019;     iNdhllaeVqfnZGOsk97347019 = iNdhllaeVqfnZGOsk35831127;     iNdhllaeVqfnZGOsk35831127 = iNdhllaeVqfnZGOsk97759207;     iNdhllaeVqfnZGOsk97759207 = iNdhllaeVqfnZGOsk31188906;     iNdhllaeVqfnZGOsk31188906 = iNdhllaeVqfnZGOsk81902018;     iNdhllaeVqfnZGOsk81902018 = iNdhllaeVqfnZGOsk473527;     iNdhllaeVqfnZGOsk473527 = iNdhllaeVqfnZGOsk57360466;     iNdhllaeVqfnZGOsk57360466 = iNdhllaeVqfnZGOsk26320577;     iNdhllaeVqfnZGOsk26320577 = iNdhllaeVqfnZGOsk12524438;     iNdhllaeVqfnZGOsk12524438 = iNdhllaeVqfnZGOsk80751420;     iNdhllaeVqfnZGOsk80751420 = iNdhllaeVqfnZGOsk70343137;     iNdhllaeVqfnZGOsk70343137 = iNdhllaeVqfnZGOsk5106103;     iNdhllaeVqfnZGOsk5106103 = iNdhllaeVqfnZGOsk36918968;     iNdhllaeVqfnZGOsk36918968 = iNdhllaeVqfnZGOsk21331101;     iNdhllaeVqfnZGOsk21331101 = iNdhllaeVqfnZGOsk10785265;     iNdhllaeVqfnZGOsk10785265 = iNdhllaeVqfnZGOsk79036868;     iNdhllaeVqfnZGOsk79036868 = iNdhllaeVqfnZGOsk42984468;     iNdhllaeVqfnZGOsk42984468 = iNdhllaeVqfnZGOsk49018234;     iNdhllaeVqfnZGOsk49018234 = iNdhllaeVqfnZGOsk35893750;     iNdhllaeVqfnZGOsk35893750 = iNdhllaeVqfnZGOsk47465635;     iNdhllaeVqfnZGOsk47465635 = iNdhllaeVqfnZGOsk98930759;     iNdhllaeVqfnZGOsk98930759 = iNdhllaeVqfnZGOsk5949621;     iNdhllaeVqfnZGOsk5949621 = iNdhllaeVqfnZGOsk80371270;     iNdhllaeVqfnZGOsk80371270 = iNdhllaeVqfnZGOsk89262684;     iNdhllaeVqfnZGOsk89262684 = iNdhllaeVqfnZGOsk47222613;     iNdhllaeVqfnZGOsk47222613 = iNdhllaeVqfnZGOsk18984142;     iNdhllaeVqfnZGOsk18984142 = iNdhllaeVqfnZGOsk82560453;     iNdhllaeVqfnZGOsk82560453 = iNdhllaeVqfnZGOsk36009133;     iNdhllaeVqfnZGOsk36009133 = iNdhllaeVqfnZGOsk23843752;     iNdhllaeVqfnZGOsk23843752 = iNdhllaeVqfnZGOsk3103289;     iNdhllaeVqfnZGOsk3103289 = iNdhllaeVqfnZGOsk54757816;     iNdhllaeVqfnZGOsk54757816 = iNdhllaeVqfnZGOsk10469065;     iNdhllaeVqfnZGOsk10469065 = iNdhllaeVqfnZGOsk38525016;     iNdhllaeVqfnZGOsk38525016 = iNdhllaeVqfnZGOsk86847372;     iNdhllaeVqfnZGOsk86847372 = iNdhllaeVqfnZGOsk82686692;     iNdhllaeVqfnZGOsk82686692 = iNdhllaeVqfnZGOsk2494061;     iNdhllaeVqfnZGOsk2494061 = iNdhllaeVqfnZGOsk19322789;     iNdhllaeVqfnZGOsk19322789 = iNdhllaeVqfnZGOsk35524943;     iNdhllaeVqfnZGOsk35524943 = iNdhllaeVqfnZGOsk20294111;     iNdhllaeVqfnZGOsk20294111 = iNdhllaeVqfnZGOsk716729;     iNdhllaeVqfnZGOsk716729 = iNdhllaeVqfnZGOsk3502422;     iNdhllaeVqfnZGOsk3502422 = iNdhllaeVqfnZGOsk35338792;     iNdhllaeVqfnZGOsk35338792 = iNdhllaeVqfnZGOsk31320296;     iNdhllaeVqfnZGOsk31320296 = iNdhllaeVqfnZGOsk8792512;     iNdhllaeVqfnZGOsk8792512 = iNdhllaeVqfnZGOsk9573443;     iNdhllaeVqfnZGOsk9573443 = iNdhllaeVqfnZGOsk23298814;     iNdhllaeVqfnZGOsk23298814 = iNdhllaeVqfnZGOsk29582615;     iNdhllaeVqfnZGOsk29582615 = iNdhllaeVqfnZGOsk39250096;     iNdhllaeVqfnZGOsk39250096 = iNdhllaeVqfnZGOsk74729154;     iNdhllaeVqfnZGOsk74729154 = iNdhllaeVqfnZGOsk17941826;     iNdhllaeVqfnZGOsk17941826 = iNdhllaeVqfnZGOsk87990376;     iNdhllaeVqfnZGOsk87990376 = iNdhllaeVqfnZGOsk2528451;     iNdhllaeVqfnZGOsk2528451 = iNdhllaeVqfnZGOsk90294804;     iNdhllaeVqfnZGOsk90294804 = iNdhllaeVqfnZGOsk72519896;     iNdhllaeVqfnZGOsk72519896 = iNdhllaeVqfnZGOsk93475795;     iNdhllaeVqfnZGOsk93475795 = iNdhllaeVqfnZGOsk91397399;     iNdhllaeVqfnZGOsk91397399 = iNdhllaeVqfnZGOsk55459856;     iNdhllaeVqfnZGOsk55459856 = iNdhllaeVqfnZGOsk8496523;     iNdhllaeVqfnZGOsk8496523 = iNdhllaeVqfnZGOsk83966293;     iNdhllaeVqfnZGOsk83966293 = iNdhllaeVqfnZGOsk62917876;     iNdhllaeVqfnZGOsk62917876 = iNdhllaeVqfnZGOsk17913074;     iNdhllaeVqfnZGOsk17913074 = iNdhllaeVqfnZGOsk21351334;     iNdhllaeVqfnZGOsk21351334 = iNdhllaeVqfnZGOsk2476825;     iNdhllaeVqfnZGOsk2476825 = iNdhllaeVqfnZGOsk9421150;     iNdhllaeVqfnZGOsk9421150 = iNdhllaeVqfnZGOsk8328524;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ijYbOSSBAQgkScZU4375605() {     double YSodvITkPHKqrtQyH43787492 = -80347256;    double YSodvITkPHKqrtQyH73862206 = -221936806;    double YSodvITkPHKqrtQyH87180831 = 66724882;    double YSodvITkPHKqrtQyH42138557 = -270363520;    double YSodvITkPHKqrtQyH25464534 = -680510351;    double YSodvITkPHKqrtQyH11850402 = -71144656;    double YSodvITkPHKqrtQyH96143131 = -187870860;    double YSodvITkPHKqrtQyH5273211 = -562179362;    double YSodvITkPHKqrtQyH23041156 = -72366059;    double YSodvITkPHKqrtQyH35725158 = -277739766;    double YSodvITkPHKqrtQyH72259868 = -318412378;    double YSodvITkPHKqrtQyH6872649 = -164609128;    double YSodvITkPHKqrtQyH74892263 = -731089930;    double YSodvITkPHKqrtQyH11651411 = -188997061;    double YSodvITkPHKqrtQyH37448443 = -626843362;    double YSodvITkPHKqrtQyH63575013 = -372649814;    double YSodvITkPHKqrtQyH94161123 = -543557756;    double YSodvITkPHKqrtQyH16842187 = -597014139;    double YSodvITkPHKqrtQyH67264178 = -642483654;    double YSodvITkPHKqrtQyH96033347 = -261639870;    double YSodvITkPHKqrtQyH14277118 = -491032662;    double YSodvITkPHKqrtQyH64284413 = -512113274;    double YSodvITkPHKqrtQyH32184749 = -415715325;    double YSodvITkPHKqrtQyH37637679 = -784768286;    double YSodvITkPHKqrtQyH97561883 = 3102155;    double YSodvITkPHKqrtQyH90614751 = -784394577;    double YSodvITkPHKqrtQyH88825490 = -288907892;    double YSodvITkPHKqrtQyH7230009 = 49316744;    double YSodvITkPHKqrtQyH7249412 = 93050341;    double YSodvITkPHKqrtQyH91047829 = -403368585;    double YSodvITkPHKqrtQyH15561925 = -641140279;    double YSodvITkPHKqrtQyH68797863 = -969309448;    double YSodvITkPHKqrtQyH63764695 = -786114344;    double YSodvITkPHKqrtQyH55936981 = -823803204;    double YSodvITkPHKqrtQyH85793985 = -40312267;    double YSodvITkPHKqrtQyH43967398 = -667872235;    double YSodvITkPHKqrtQyH46758836 = -240801049;    double YSodvITkPHKqrtQyH87046530 = -108034368;    double YSodvITkPHKqrtQyH28149769 = -644213719;    double YSodvITkPHKqrtQyH95460438 = -834445095;    double YSodvITkPHKqrtQyH15353626 = -128693711;    double YSodvITkPHKqrtQyH89838653 = -808705926;    double YSodvITkPHKqrtQyH46743845 = -841333177;    double YSodvITkPHKqrtQyH66451915 = -998036797;    double YSodvITkPHKqrtQyH12540897 = -512387658;    double YSodvITkPHKqrtQyH26712749 = -710201532;    double YSodvITkPHKqrtQyH89688300 = -199537548;    double YSodvITkPHKqrtQyH22832415 = -243834522;    double YSodvITkPHKqrtQyH59811634 = -346856782;    double YSodvITkPHKqrtQyH45860691 = -830559387;    double YSodvITkPHKqrtQyH91555988 = -65596720;    double YSodvITkPHKqrtQyH62522340 = -393022469;    double YSodvITkPHKqrtQyH11605617 = -95260651;    double YSodvITkPHKqrtQyH65010080 = -129184623;    double YSodvITkPHKqrtQyH90098486 = -309099530;    double YSodvITkPHKqrtQyH79503079 = -568233983;    double YSodvITkPHKqrtQyH41677458 = -806221481;    double YSodvITkPHKqrtQyH49543152 = -148506833;    double YSodvITkPHKqrtQyH44576674 = -173465676;    double YSodvITkPHKqrtQyH34849782 = -896115775;    double YSodvITkPHKqrtQyH23024911 = -782236764;    double YSodvITkPHKqrtQyH88913123 = -137187604;    double YSodvITkPHKqrtQyH98023799 = -555229703;    double YSodvITkPHKqrtQyH31993327 = -668997475;    double YSodvITkPHKqrtQyH20163233 = -636599488;    double YSodvITkPHKqrtQyH3462005 = -349102931;    double YSodvITkPHKqrtQyH43107954 = -378494784;    double YSodvITkPHKqrtQyH18955283 = -907286726;    double YSodvITkPHKqrtQyH25857426 = -48684795;    double YSodvITkPHKqrtQyH93481045 = -958971127;    double YSodvITkPHKqrtQyH16816177 = -31848766;    double YSodvITkPHKqrtQyH7114594 = -335523389;    double YSodvITkPHKqrtQyH88692418 = -952800420;    double YSodvITkPHKqrtQyH71803740 = -808038560;    double YSodvITkPHKqrtQyH80679721 = -32946159;    double YSodvITkPHKqrtQyH24438465 = -682326736;    double YSodvITkPHKqrtQyH17540568 = -670780097;    double YSodvITkPHKqrtQyH65732833 = -417678529;    double YSodvITkPHKqrtQyH25096783 = -172380628;    double YSodvITkPHKqrtQyH70849134 = -286696314;    double YSodvITkPHKqrtQyH926451 = -484857029;    double YSodvITkPHKqrtQyH65993076 = 54926629;    double YSodvITkPHKqrtQyH47418374 = -603826474;    double YSodvITkPHKqrtQyH61388721 = -76390273;    double YSodvITkPHKqrtQyH99491840 = -237771865;    double YSodvITkPHKqrtQyH53039584 = -148117810;    double YSodvITkPHKqrtQyH57192247 = -774048798;    double YSodvITkPHKqrtQyH98754615 = -556929722;    double YSodvITkPHKqrtQyH65838494 = -414703675;    double YSodvITkPHKqrtQyH6290906 = -472078284;    double YSodvITkPHKqrtQyH2289941 = -861650755;    double YSodvITkPHKqrtQyH97215683 = 7705784;    double YSodvITkPHKqrtQyH42469856 = -934568692;    double YSodvITkPHKqrtQyH93299987 = -748097945;    double YSodvITkPHKqrtQyH72435527 = 47791669;    double YSodvITkPHKqrtQyH26440502 = -991506108;    double YSodvITkPHKqrtQyH91814853 = -153476224;    double YSodvITkPHKqrtQyH14750518 = -72335703;    double YSodvITkPHKqrtQyH46288683 = -261437309;    double YSodvITkPHKqrtQyH9078892 = -80347256;     YSodvITkPHKqrtQyH43787492 = YSodvITkPHKqrtQyH73862206;     YSodvITkPHKqrtQyH73862206 = YSodvITkPHKqrtQyH87180831;     YSodvITkPHKqrtQyH87180831 = YSodvITkPHKqrtQyH42138557;     YSodvITkPHKqrtQyH42138557 = YSodvITkPHKqrtQyH25464534;     YSodvITkPHKqrtQyH25464534 = YSodvITkPHKqrtQyH11850402;     YSodvITkPHKqrtQyH11850402 = YSodvITkPHKqrtQyH96143131;     YSodvITkPHKqrtQyH96143131 = YSodvITkPHKqrtQyH5273211;     YSodvITkPHKqrtQyH5273211 = YSodvITkPHKqrtQyH23041156;     YSodvITkPHKqrtQyH23041156 = YSodvITkPHKqrtQyH35725158;     YSodvITkPHKqrtQyH35725158 = YSodvITkPHKqrtQyH72259868;     YSodvITkPHKqrtQyH72259868 = YSodvITkPHKqrtQyH6872649;     YSodvITkPHKqrtQyH6872649 = YSodvITkPHKqrtQyH74892263;     YSodvITkPHKqrtQyH74892263 = YSodvITkPHKqrtQyH11651411;     YSodvITkPHKqrtQyH11651411 = YSodvITkPHKqrtQyH37448443;     YSodvITkPHKqrtQyH37448443 = YSodvITkPHKqrtQyH63575013;     YSodvITkPHKqrtQyH63575013 = YSodvITkPHKqrtQyH94161123;     YSodvITkPHKqrtQyH94161123 = YSodvITkPHKqrtQyH16842187;     YSodvITkPHKqrtQyH16842187 = YSodvITkPHKqrtQyH67264178;     YSodvITkPHKqrtQyH67264178 = YSodvITkPHKqrtQyH96033347;     YSodvITkPHKqrtQyH96033347 = YSodvITkPHKqrtQyH14277118;     YSodvITkPHKqrtQyH14277118 = YSodvITkPHKqrtQyH64284413;     YSodvITkPHKqrtQyH64284413 = YSodvITkPHKqrtQyH32184749;     YSodvITkPHKqrtQyH32184749 = YSodvITkPHKqrtQyH37637679;     YSodvITkPHKqrtQyH37637679 = YSodvITkPHKqrtQyH97561883;     YSodvITkPHKqrtQyH97561883 = YSodvITkPHKqrtQyH90614751;     YSodvITkPHKqrtQyH90614751 = YSodvITkPHKqrtQyH88825490;     YSodvITkPHKqrtQyH88825490 = YSodvITkPHKqrtQyH7230009;     YSodvITkPHKqrtQyH7230009 = YSodvITkPHKqrtQyH7249412;     YSodvITkPHKqrtQyH7249412 = YSodvITkPHKqrtQyH91047829;     YSodvITkPHKqrtQyH91047829 = YSodvITkPHKqrtQyH15561925;     YSodvITkPHKqrtQyH15561925 = YSodvITkPHKqrtQyH68797863;     YSodvITkPHKqrtQyH68797863 = YSodvITkPHKqrtQyH63764695;     YSodvITkPHKqrtQyH63764695 = YSodvITkPHKqrtQyH55936981;     YSodvITkPHKqrtQyH55936981 = YSodvITkPHKqrtQyH85793985;     YSodvITkPHKqrtQyH85793985 = YSodvITkPHKqrtQyH43967398;     YSodvITkPHKqrtQyH43967398 = YSodvITkPHKqrtQyH46758836;     YSodvITkPHKqrtQyH46758836 = YSodvITkPHKqrtQyH87046530;     YSodvITkPHKqrtQyH87046530 = YSodvITkPHKqrtQyH28149769;     YSodvITkPHKqrtQyH28149769 = YSodvITkPHKqrtQyH95460438;     YSodvITkPHKqrtQyH95460438 = YSodvITkPHKqrtQyH15353626;     YSodvITkPHKqrtQyH15353626 = YSodvITkPHKqrtQyH89838653;     YSodvITkPHKqrtQyH89838653 = YSodvITkPHKqrtQyH46743845;     YSodvITkPHKqrtQyH46743845 = YSodvITkPHKqrtQyH66451915;     YSodvITkPHKqrtQyH66451915 = YSodvITkPHKqrtQyH12540897;     YSodvITkPHKqrtQyH12540897 = YSodvITkPHKqrtQyH26712749;     YSodvITkPHKqrtQyH26712749 = YSodvITkPHKqrtQyH89688300;     YSodvITkPHKqrtQyH89688300 = YSodvITkPHKqrtQyH22832415;     YSodvITkPHKqrtQyH22832415 = YSodvITkPHKqrtQyH59811634;     YSodvITkPHKqrtQyH59811634 = YSodvITkPHKqrtQyH45860691;     YSodvITkPHKqrtQyH45860691 = YSodvITkPHKqrtQyH91555988;     YSodvITkPHKqrtQyH91555988 = YSodvITkPHKqrtQyH62522340;     YSodvITkPHKqrtQyH62522340 = YSodvITkPHKqrtQyH11605617;     YSodvITkPHKqrtQyH11605617 = YSodvITkPHKqrtQyH65010080;     YSodvITkPHKqrtQyH65010080 = YSodvITkPHKqrtQyH90098486;     YSodvITkPHKqrtQyH90098486 = YSodvITkPHKqrtQyH79503079;     YSodvITkPHKqrtQyH79503079 = YSodvITkPHKqrtQyH41677458;     YSodvITkPHKqrtQyH41677458 = YSodvITkPHKqrtQyH49543152;     YSodvITkPHKqrtQyH49543152 = YSodvITkPHKqrtQyH44576674;     YSodvITkPHKqrtQyH44576674 = YSodvITkPHKqrtQyH34849782;     YSodvITkPHKqrtQyH34849782 = YSodvITkPHKqrtQyH23024911;     YSodvITkPHKqrtQyH23024911 = YSodvITkPHKqrtQyH88913123;     YSodvITkPHKqrtQyH88913123 = YSodvITkPHKqrtQyH98023799;     YSodvITkPHKqrtQyH98023799 = YSodvITkPHKqrtQyH31993327;     YSodvITkPHKqrtQyH31993327 = YSodvITkPHKqrtQyH20163233;     YSodvITkPHKqrtQyH20163233 = YSodvITkPHKqrtQyH3462005;     YSodvITkPHKqrtQyH3462005 = YSodvITkPHKqrtQyH43107954;     YSodvITkPHKqrtQyH43107954 = YSodvITkPHKqrtQyH18955283;     YSodvITkPHKqrtQyH18955283 = YSodvITkPHKqrtQyH25857426;     YSodvITkPHKqrtQyH25857426 = YSodvITkPHKqrtQyH93481045;     YSodvITkPHKqrtQyH93481045 = YSodvITkPHKqrtQyH16816177;     YSodvITkPHKqrtQyH16816177 = YSodvITkPHKqrtQyH7114594;     YSodvITkPHKqrtQyH7114594 = YSodvITkPHKqrtQyH88692418;     YSodvITkPHKqrtQyH88692418 = YSodvITkPHKqrtQyH71803740;     YSodvITkPHKqrtQyH71803740 = YSodvITkPHKqrtQyH80679721;     YSodvITkPHKqrtQyH80679721 = YSodvITkPHKqrtQyH24438465;     YSodvITkPHKqrtQyH24438465 = YSodvITkPHKqrtQyH17540568;     YSodvITkPHKqrtQyH17540568 = YSodvITkPHKqrtQyH65732833;     YSodvITkPHKqrtQyH65732833 = YSodvITkPHKqrtQyH25096783;     YSodvITkPHKqrtQyH25096783 = YSodvITkPHKqrtQyH70849134;     YSodvITkPHKqrtQyH70849134 = YSodvITkPHKqrtQyH926451;     YSodvITkPHKqrtQyH926451 = YSodvITkPHKqrtQyH65993076;     YSodvITkPHKqrtQyH65993076 = YSodvITkPHKqrtQyH47418374;     YSodvITkPHKqrtQyH47418374 = YSodvITkPHKqrtQyH61388721;     YSodvITkPHKqrtQyH61388721 = YSodvITkPHKqrtQyH99491840;     YSodvITkPHKqrtQyH99491840 = YSodvITkPHKqrtQyH53039584;     YSodvITkPHKqrtQyH53039584 = YSodvITkPHKqrtQyH57192247;     YSodvITkPHKqrtQyH57192247 = YSodvITkPHKqrtQyH98754615;     YSodvITkPHKqrtQyH98754615 = YSodvITkPHKqrtQyH65838494;     YSodvITkPHKqrtQyH65838494 = YSodvITkPHKqrtQyH6290906;     YSodvITkPHKqrtQyH6290906 = YSodvITkPHKqrtQyH2289941;     YSodvITkPHKqrtQyH2289941 = YSodvITkPHKqrtQyH97215683;     YSodvITkPHKqrtQyH97215683 = YSodvITkPHKqrtQyH42469856;     YSodvITkPHKqrtQyH42469856 = YSodvITkPHKqrtQyH93299987;     YSodvITkPHKqrtQyH93299987 = YSodvITkPHKqrtQyH72435527;     YSodvITkPHKqrtQyH72435527 = YSodvITkPHKqrtQyH26440502;     YSodvITkPHKqrtQyH26440502 = YSodvITkPHKqrtQyH91814853;     YSodvITkPHKqrtQyH91814853 = YSodvITkPHKqrtQyH14750518;     YSodvITkPHKqrtQyH14750518 = YSodvITkPHKqrtQyH46288683;     YSodvITkPHKqrtQyH46288683 = YSodvITkPHKqrtQyH9078892;     YSodvITkPHKqrtQyH9078892 = YSodvITkPHKqrtQyH43787492;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GxKOTWBXMDNjPSwK19424673() {     double JCDMmsSBgsHIikUFV49904599 = -192249145;    double JCDMmsSBgsHIikUFV17235486 = 68463413;    double JCDMmsSBgsHIikUFV3223015 = -84108039;    double JCDMmsSBgsHIikUFV69213338 = -461112151;    double JCDMmsSBgsHIikUFV94360021 = -101160223;    double JCDMmsSBgsHIikUFV3267604 = -335640891;    double JCDMmsSBgsHIikUFV56632097 = -326835837;    double JCDMmsSBgsHIikUFV55033102 = -849053275;    double JCDMmsSBgsHIikUFV87683651 = -940398725;    double JCDMmsSBgsHIikUFV76954817 = -606715244;    double JCDMmsSBgsHIikUFV32370158 = -195849965;    double JCDMmsSBgsHIikUFV38695692 = -609015330;    double JCDMmsSBgsHIikUFV61101382 = -70981019;    double JCDMmsSBgsHIikUFV94795087 = -721235629;    double JCDMmsSBgsHIikUFV33190870 = -217305528;    double JCDMmsSBgsHIikUFV5080408 = -345649357;    double JCDMmsSBgsHIikUFV87997126 = -60102869;    double JCDMmsSBgsHIikUFV13895340 = -262536782;    double JCDMmsSBgsHIikUFV29094020 = 51747249;    double JCDMmsSBgsHIikUFV76583512 = -618542789;    double JCDMmsSBgsHIikUFV2830312 = -346520117;    double JCDMmsSBgsHIikUFV50805437 = -420042990;    double JCDMmsSBgsHIikUFV67775540 = -162232812;    double JCDMmsSBgsHIikUFV21630053 = 92926726;    double JCDMmsSBgsHIikUFV41261533 = -370093878;    double JCDMmsSBgsHIikUFV15336427 = -663049900;    double JCDMmsSBgsHIikUFV87285268 = -189570095;    double JCDMmsSBgsHIikUFV20337134 = -16612210;    double JCDMmsSBgsHIikUFV73381667 = -129332364;    double JCDMmsSBgsHIikUFV59663466 = -220839452;    double JCDMmsSBgsHIikUFV7804475 = -554461217;    double JCDMmsSBgsHIikUFV66408485 = -678756263;    double JCDMmsSBgsHIikUFV73475643 = -678727714;    double JCDMmsSBgsHIikUFV50767545 = -575344969;    double JCDMmsSBgsHIikUFV47268637 = -679045372;    double JCDMmsSBgsHIikUFV12693312 = -881898343;    double JCDMmsSBgsHIikUFV28020771 = -194436918;    double JCDMmsSBgsHIikUFV21136743 = -979540377;    double JCDMmsSBgsHIikUFV1760023 = -638137216;    double JCDMmsSBgsHIikUFV82962376 = -207884317;    double JCDMmsSBgsHIikUFV21022850 = -193967606;    double JCDMmsSBgsHIikUFV21989110 = 21151796;    double JCDMmsSBgsHIikUFV24810823 = -922137761;    double JCDMmsSBgsHIikUFV53859656 = 82620265;    double JCDMmsSBgsHIikUFV24493362 = -174285813;    double JCDMmsSBgsHIikUFV38651944 = -695417648;    double JCDMmsSBgsHIikUFV7466875 = -318313086;    double JCDMmsSBgsHIikUFV30789356 = -253730054;    double JCDMmsSBgsHIikUFV69487363 = -432593770;    double JCDMmsSBgsHIikUFV58772343 = -917570195;    double JCDMmsSBgsHIikUFV27256667 = 92109608;    double JCDMmsSBgsHIikUFV73332059 = -193748789;    double JCDMmsSBgsHIikUFV59854074 = -987050238;    double JCDMmsSBgsHIikUFV60978960 = -449654768;    double JCDMmsSBgsHIikUFV16594361 = 90368688;    double JCDMmsSBgsHIikUFV99099162 = -772206156;    double JCDMmsSBgsHIikUFV49459945 = -769303775;    double JCDMmsSBgsHIikUFV81592962 = -77034766;    double JCDMmsSBgsHIikUFV27951805 = 8981726;    double JCDMmsSBgsHIikUFV79023595 = -438110323;    double JCDMmsSBgsHIikUFV15982336 = -46070796;    double JCDMmsSBgsHIikUFV36294964 = -210223628;    double JCDMmsSBgsHIikUFV81651434 = -619720911;    double JCDMmsSBgsHIikUFV28020186 = -619559273;    double JCDMmsSBgsHIikUFV69150342 = 47745972;    double JCDMmsSBgsHIikUFV65961673 = -517093702;    double JCDMmsSBgsHIikUFV65220048 = -930287616;    double JCDMmsSBgsHIikUFV10333838 = -495636050;    double JCDMmsSBgsHIikUFV47526450 = 57809742;    double JCDMmsSBgsHIikUFV20497559 = -335407186;    double JCDMmsSBgsHIikUFV77059636 = -51212439;    double JCDMmsSBgsHIikUFV66860384 = -80562492;    double JCDMmsSBgsHIikUFV12135318 = -624399566;    double JCDMmsSBgsHIikUFV46131644 = -740368434;    double JCDMmsSBgsHIikUFV55560663 = -324575184;    double JCDMmsSBgsHIikUFV80841201 = -267671913;    double JCDMmsSBgsHIikUFV25994614 = -497905230;    double JCDMmsSBgsHIikUFV13915885 = -144853078;    double JCDMmsSBgsHIikUFV97136691 = -732787461;    double JCDMmsSBgsHIikUFV2609590 = -674676230;    double JCDMmsSBgsHIikUFV7869552 = -244736815;    double JCDMmsSBgsHIikUFV56495913 = -935840042;    double JCDMmsSBgsHIikUFV50849770 = -584018440;    double JCDMmsSBgsHIikUFV14609324 = -211762169;    double JCDMmsSBgsHIikUFV32406799 = -212949061;    double JCDMmsSBgsHIikUFV34472415 = -260712428;    double JCDMmsSBgsHIikUFV6554412 = -691706026;    double JCDMmsSBgsHIikUFV12496684 = -129072947;    double JCDMmsSBgsHIikUFV34173184 = -565713658;    double JCDMmsSBgsHIikUFV48169475 = -906839217;    double JCDMmsSBgsHIikUFV63233366 = -12594568;    double JCDMmsSBgsHIikUFV46427809 = -17402152;    double JCDMmsSBgsHIikUFV93184938 = -888522104;    double JCDMmsSBgsHIikUFV22736428 = -100026894;    double JCDMmsSBgsHIikUFV66980041 = -61813522;    double JCDMmsSBgsHIikUFV84727885 = -983743978;    double JCDMmsSBgsHIikUFV40337676 = -359127294;    double JCDMmsSBgsHIikUFV96790637 = -202578488;    double JCDMmsSBgsHIikUFV84709313 = -965125708;    double JCDMmsSBgsHIikUFV58531688 = -192249145;     JCDMmsSBgsHIikUFV49904599 = JCDMmsSBgsHIikUFV17235486;     JCDMmsSBgsHIikUFV17235486 = JCDMmsSBgsHIikUFV3223015;     JCDMmsSBgsHIikUFV3223015 = JCDMmsSBgsHIikUFV69213338;     JCDMmsSBgsHIikUFV69213338 = JCDMmsSBgsHIikUFV94360021;     JCDMmsSBgsHIikUFV94360021 = JCDMmsSBgsHIikUFV3267604;     JCDMmsSBgsHIikUFV3267604 = JCDMmsSBgsHIikUFV56632097;     JCDMmsSBgsHIikUFV56632097 = JCDMmsSBgsHIikUFV55033102;     JCDMmsSBgsHIikUFV55033102 = JCDMmsSBgsHIikUFV87683651;     JCDMmsSBgsHIikUFV87683651 = JCDMmsSBgsHIikUFV76954817;     JCDMmsSBgsHIikUFV76954817 = JCDMmsSBgsHIikUFV32370158;     JCDMmsSBgsHIikUFV32370158 = JCDMmsSBgsHIikUFV38695692;     JCDMmsSBgsHIikUFV38695692 = JCDMmsSBgsHIikUFV61101382;     JCDMmsSBgsHIikUFV61101382 = JCDMmsSBgsHIikUFV94795087;     JCDMmsSBgsHIikUFV94795087 = JCDMmsSBgsHIikUFV33190870;     JCDMmsSBgsHIikUFV33190870 = JCDMmsSBgsHIikUFV5080408;     JCDMmsSBgsHIikUFV5080408 = JCDMmsSBgsHIikUFV87997126;     JCDMmsSBgsHIikUFV87997126 = JCDMmsSBgsHIikUFV13895340;     JCDMmsSBgsHIikUFV13895340 = JCDMmsSBgsHIikUFV29094020;     JCDMmsSBgsHIikUFV29094020 = JCDMmsSBgsHIikUFV76583512;     JCDMmsSBgsHIikUFV76583512 = JCDMmsSBgsHIikUFV2830312;     JCDMmsSBgsHIikUFV2830312 = JCDMmsSBgsHIikUFV50805437;     JCDMmsSBgsHIikUFV50805437 = JCDMmsSBgsHIikUFV67775540;     JCDMmsSBgsHIikUFV67775540 = JCDMmsSBgsHIikUFV21630053;     JCDMmsSBgsHIikUFV21630053 = JCDMmsSBgsHIikUFV41261533;     JCDMmsSBgsHIikUFV41261533 = JCDMmsSBgsHIikUFV15336427;     JCDMmsSBgsHIikUFV15336427 = JCDMmsSBgsHIikUFV87285268;     JCDMmsSBgsHIikUFV87285268 = JCDMmsSBgsHIikUFV20337134;     JCDMmsSBgsHIikUFV20337134 = JCDMmsSBgsHIikUFV73381667;     JCDMmsSBgsHIikUFV73381667 = JCDMmsSBgsHIikUFV59663466;     JCDMmsSBgsHIikUFV59663466 = JCDMmsSBgsHIikUFV7804475;     JCDMmsSBgsHIikUFV7804475 = JCDMmsSBgsHIikUFV66408485;     JCDMmsSBgsHIikUFV66408485 = JCDMmsSBgsHIikUFV73475643;     JCDMmsSBgsHIikUFV73475643 = JCDMmsSBgsHIikUFV50767545;     JCDMmsSBgsHIikUFV50767545 = JCDMmsSBgsHIikUFV47268637;     JCDMmsSBgsHIikUFV47268637 = JCDMmsSBgsHIikUFV12693312;     JCDMmsSBgsHIikUFV12693312 = JCDMmsSBgsHIikUFV28020771;     JCDMmsSBgsHIikUFV28020771 = JCDMmsSBgsHIikUFV21136743;     JCDMmsSBgsHIikUFV21136743 = JCDMmsSBgsHIikUFV1760023;     JCDMmsSBgsHIikUFV1760023 = JCDMmsSBgsHIikUFV82962376;     JCDMmsSBgsHIikUFV82962376 = JCDMmsSBgsHIikUFV21022850;     JCDMmsSBgsHIikUFV21022850 = JCDMmsSBgsHIikUFV21989110;     JCDMmsSBgsHIikUFV21989110 = JCDMmsSBgsHIikUFV24810823;     JCDMmsSBgsHIikUFV24810823 = JCDMmsSBgsHIikUFV53859656;     JCDMmsSBgsHIikUFV53859656 = JCDMmsSBgsHIikUFV24493362;     JCDMmsSBgsHIikUFV24493362 = JCDMmsSBgsHIikUFV38651944;     JCDMmsSBgsHIikUFV38651944 = JCDMmsSBgsHIikUFV7466875;     JCDMmsSBgsHIikUFV7466875 = JCDMmsSBgsHIikUFV30789356;     JCDMmsSBgsHIikUFV30789356 = JCDMmsSBgsHIikUFV69487363;     JCDMmsSBgsHIikUFV69487363 = JCDMmsSBgsHIikUFV58772343;     JCDMmsSBgsHIikUFV58772343 = JCDMmsSBgsHIikUFV27256667;     JCDMmsSBgsHIikUFV27256667 = JCDMmsSBgsHIikUFV73332059;     JCDMmsSBgsHIikUFV73332059 = JCDMmsSBgsHIikUFV59854074;     JCDMmsSBgsHIikUFV59854074 = JCDMmsSBgsHIikUFV60978960;     JCDMmsSBgsHIikUFV60978960 = JCDMmsSBgsHIikUFV16594361;     JCDMmsSBgsHIikUFV16594361 = JCDMmsSBgsHIikUFV99099162;     JCDMmsSBgsHIikUFV99099162 = JCDMmsSBgsHIikUFV49459945;     JCDMmsSBgsHIikUFV49459945 = JCDMmsSBgsHIikUFV81592962;     JCDMmsSBgsHIikUFV81592962 = JCDMmsSBgsHIikUFV27951805;     JCDMmsSBgsHIikUFV27951805 = JCDMmsSBgsHIikUFV79023595;     JCDMmsSBgsHIikUFV79023595 = JCDMmsSBgsHIikUFV15982336;     JCDMmsSBgsHIikUFV15982336 = JCDMmsSBgsHIikUFV36294964;     JCDMmsSBgsHIikUFV36294964 = JCDMmsSBgsHIikUFV81651434;     JCDMmsSBgsHIikUFV81651434 = JCDMmsSBgsHIikUFV28020186;     JCDMmsSBgsHIikUFV28020186 = JCDMmsSBgsHIikUFV69150342;     JCDMmsSBgsHIikUFV69150342 = JCDMmsSBgsHIikUFV65961673;     JCDMmsSBgsHIikUFV65961673 = JCDMmsSBgsHIikUFV65220048;     JCDMmsSBgsHIikUFV65220048 = JCDMmsSBgsHIikUFV10333838;     JCDMmsSBgsHIikUFV10333838 = JCDMmsSBgsHIikUFV47526450;     JCDMmsSBgsHIikUFV47526450 = JCDMmsSBgsHIikUFV20497559;     JCDMmsSBgsHIikUFV20497559 = JCDMmsSBgsHIikUFV77059636;     JCDMmsSBgsHIikUFV77059636 = JCDMmsSBgsHIikUFV66860384;     JCDMmsSBgsHIikUFV66860384 = JCDMmsSBgsHIikUFV12135318;     JCDMmsSBgsHIikUFV12135318 = JCDMmsSBgsHIikUFV46131644;     JCDMmsSBgsHIikUFV46131644 = JCDMmsSBgsHIikUFV55560663;     JCDMmsSBgsHIikUFV55560663 = JCDMmsSBgsHIikUFV80841201;     JCDMmsSBgsHIikUFV80841201 = JCDMmsSBgsHIikUFV25994614;     JCDMmsSBgsHIikUFV25994614 = JCDMmsSBgsHIikUFV13915885;     JCDMmsSBgsHIikUFV13915885 = JCDMmsSBgsHIikUFV97136691;     JCDMmsSBgsHIikUFV97136691 = JCDMmsSBgsHIikUFV2609590;     JCDMmsSBgsHIikUFV2609590 = JCDMmsSBgsHIikUFV7869552;     JCDMmsSBgsHIikUFV7869552 = JCDMmsSBgsHIikUFV56495913;     JCDMmsSBgsHIikUFV56495913 = JCDMmsSBgsHIikUFV50849770;     JCDMmsSBgsHIikUFV50849770 = JCDMmsSBgsHIikUFV14609324;     JCDMmsSBgsHIikUFV14609324 = JCDMmsSBgsHIikUFV32406799;     JCDMmsSBgsHIikUFV32406799 = JCDMmsSBgsHIikUFV34472415;     JCDMmsSBgsHIikUFV34472415 = JCDMmsSBgsHIikUFV6554412;     JCDMmsSBgsHIikUFV6554412 = JCDMmsSBgsHIikUFV12496684;     JCDMmsSBgsHIikUFV12496684 = JCDMmsSBgsHIikUFV34173184;     JCDMmsSBgsHIikUFV34173184 = JCDMmsSBgsHIikUFV48169475;     JCDMmsSBgsHIikUFV48169475 = JCDMmsSBgsHIikUFV63233366;     JCDMmsSBgsHIikUFV63233366 = JCDMmsSBgsHIikUFV46427809;     JCDMmsSBgsHIikUFV46427809 = JCDMmsSBgsHIikUFV93184938;     JCDMmsSBgsHIikUFV93184938 = JCDMmsSBgsHIikUFV22736428;     JCDMmsSBgsHIikUFV22736428 = JCDMmsSBgsHIikUFV66980041;     JCDMmsSBgsHIikUFV66980041 = JCDMmsSBgsHIikUFV84727885;     JCDMmsSBgsHIikUFV84727885 = JCDMmsSBgsHIikUFV40337676;     JCDMmsSBgsHIikUFV40337676 = JCDMmsSBgsHIikUFV96790637;     JCDMmsSBgsHIikUFV96790637 = JCDMmsSBgsHIikUFV84709313;     JCDMmsSBgsHIikUFV84709313 = JCDMmsSBgsHIikUFV58531688;     JCDMmsSBgsHIikUFV58531688 = JCDMmsSBgsHIikUFV49904599;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ulBBBJxWwVkewLGK86716271() {     double zQvTogvjwVCxuYemh85363567 = -658526045;    double zQvTogvjwVCxuYemh30507564 = -912088193;    double zQvTogvjwVCxuYemh40609492 = -849775114;    double zQvTogvjwVCxuYemh41436693 = -282263222;    double zQvTogvjwVCxuYemh50645291 = -206759068;    double zQvTogvjwVCxuYemh39490476 = -597988786;    double zQvTogvjwVCxuYemh67289504 = -171561571;    double zQvTogvjwVCxuYemh17164663 = -493287415;    double zQvTogvjwVCxuYemh89201256 = -328730132;    double zQvTogvjwVCxuYemh91857528 = -271402242;    double zQvTogvjwVCxuYemh70551893 = 2142306;    double zQvTogvjwVCxuYemh39263840 = -473136101;    double zQvTogvjwVCxuYemh99336027 = -622993524;    double zQvTogvjwVCxuYemh47935950 = -697599039;    double zQvTogvjwVCxuYemh91385386 = -720730543;    double zQvTogvjwVCxuYemh88621708 = 18539036;    double zQvTogvjwVCxuYemh43833063 = -587077422;    double zQvTogvjwVCxuYemh13655532 = -335964850;    double zQvTogvjwVCxuYemh29644350 = -264816173;    double zQvTogvjwVCxuYemh70420730 = 54896145;    double zQvTogvjwVCxuYemh15917175 = -3794806;    double zQvTogvjwVCxuYemh54226961 = -506679284;    double zQvTogvjwVCxuYemh38300920 = -899864996;    double zQvTogvjwVCxuYemh15422998 = -417897878;    double zQvTogvjwVCxuYemh49279484 = -394079076;    double zQvTogvjwVCxuYemh26034599 = -351626707;    double zQvTogvjwVCxuYemh47705843 = 4145764;    double zQvTogvjwVCxuYemh61065558 = -186384384;    double zQvTogvjwVCxuYemh20049882 = -703427166;    double zQvTogvjwVCxuYemh65196877 = -255346623;    double zQvTogvjwVCxuYemh26387706 = -6441704;    double zQvTogvjwVCxuYemh4231504 = -451727678;    double zQvTogvjwVCxuYemh85693654 = -826323692;    double zQvTogvjwVCxuYemh80515971 = -640038783;    double zQvTogvjwVCxuYemh13077091 = -630145623;    double zQvTogvjwVCxuYemh64254155 = 3536352;    double zQvTogvjwVCxuYemh77432587 = -59132958;    double zQvTogvjwVCxuYemh72352146 = -962195959;    double zQvTogvjwVCxuYemh32150584 = -289013493;    double zQvTogvjwVCxuYemh47233908 = -773146863;    double zQvTogvjwVCxuYemh54474457 = 36833480;    double zQvTogvjwVCxuYemh11354237 = -673393495;    double zQvTogvjwVCxuYemh14194202 = -475318547;    double zQvTogvjwVCxuYemh93990993 = -521712145;    double zQvTogvjwVCxuYemh24509820 = -796974;    double zQvTogvjwVCxuYemh84613272 = -734341280;    double zQvTogvjwVCxuYemh26812039 = -802299446;    double zQvTogvjwVCxuYemh48515667 = -420256648;    double zQvTogvjwVCxuYemh92380029 = -96330651;    double zQvTogvjwVCxuYemh83301932 = -308597011;    double zQvTogvjwVCxuYemh8027391 = -882166949;    double zQvTogvjwVCxuYemh56817532 = -407401136;    double zQvTogvjwVCxuYemh28475223 = -743095792;    double zQvTogvjwVCxuYemh76970807 = -641163268;    double zQvTogvjwVCxuYemh70799097 = -256121458;    double zQvTogvjwVCxuYemh31136606 = -51846762;    double zQvTogvjwVCxuYemh92206644 = 87776803;    double zQvTogvjwVCxuYemh25186494 = -331877237;    double zQvTogvjwVCxuYemh92157208 = -888184146;    double zQvTogvjwVCxuYemh24610693 = -855132362;    double zQvTogvjwVCxuYemh91784633 = -502134551;    double zQvTogvjwVCxuYemh6223946 = -985177188;    double zQvTogvjwVCxuYemh97114780 = -789860249;    double zQvTogvjwVCxuYemh24004380 = 26616491;    double zQvTogvjwVCxuYemh65469823 = -164960538;    double zQvTogvjwVCxuYemh66320389 = -546130017;    double zQvTogvjwVCxuYemh53570185 = -646812410;    double zQvTogvjwVCxuYemh18820057 = -982954741;    double zQvTogvjwVCxuYemh34858860 = 32546584;    double zQvTogvjwVCxuYemh27131232 = -624266895;    double zQvTogvjwVCxuYemh11189121 = -922328006;    double zQvTogvjwVCxuYemh71480916 = -624881464;    double zQvTogvjwVCxuYemh81504947 = 53048642;    double zQvTogvjwVCxuYemh82410442 = -491669310;    double zQvTogvjwVCxuYemh15946274 = -981937336;    double zQvTogvjwVCxuYemh4562938 = -330401311;    double zQvTogvjwVCxuYemh40032760 = 68639262;    double zQvTogvjwVCxuYemh44309926 = -278152852;    double zQvTogvjwVCxuYemh90913177 = -317100904;    double zQvTogvjwVCxuYemh64666212 = -659737796;    double zQvTogvjwVCxuYemh99222559 = -549327261;    double zQvTogvjwVCxuYemh99190175 = -575597589;    double zQvTogvjwVCxuYemh68685529 = 9946266;    double zQvTogvjwVCxuYemh36747949 = -294830156;    double zQvTogvjwVCxuYemh57169486 = -373179674;    double zQvTogvjwVCxuYemh69570173 = -599040569;    double zQvTogvjwVCxuYemh75756281 = -708631886;    double zQvTogvjwVCxuYemh8722848 = -85160424;    double zQvTogvjwVCxuYemh9716874 = -283917325;    double zQvTogvjwVCxuYemh81940484 = -478298862;    double zQvTogvjwVCxuYemh72047511 = 15759548;    double zQvTogvjwVCxuYemh52246094 = -727255722;    double zQvTogvjwVCxuYemh80194937 = 25988187;    double zQvTogvjwVCxuYemh7539892 = -433881132;    double zQvTogvjwVCxuYemh55449275 = -171012313;    double zQvTogvjwVCxuYemh48250512 = 22010667;    double zQvTogvjwVCxuYemh14239456 = -883533247;    double zQvTogvjwVCxuYemh90189821 = -401935038;    double zQvTogvjwVCxuYemh28521171 = -256751607;    double zQvTogvjwVCxuYemh58189431 = -658526045;     zQvTogvjwVCxuYemh85363567 = zQvTogvjwVCxuYemh30507564;     zQvTogvjwVCxuYemh30507564 = zQvTogvjwVCxuYemh40609492;     zQvTogvjwVCxuYemh40609492 = zQvTogvjwVCxuYemh41436693;     zQvTogvjwVCxuYemh41436693 = zQvTogvjwVCxuYemh50645291;     zQvTogvjwVCxuYemh50645291 = zQvTogvjwVCxuYemh39490476;     zQvTogvjwVCxuYemh39490476 = zQvTogvjwVCxuYemh67289504;     zQvTogvjwVCxuYemh67289504 = zQvTogvjwVCxuYemh17164663;     zQvTogvjwVCxuYemh17164663 = zQvTogvjwVCxuYemh89201256;     zQvTogvjwVCxuYemh89201256 = zQvTogvjwVCxuYemh91857528;     zQvTogvjwVCxuYemh91857528 = zQvTogvjwVCxuYemh70551893;     zQvTogvjwVCxuYemh70551893 = zQvTogvjwVCxuYemh39263840;     zQvTogvjwVCxuYemh39263840 = zQvTogvjwVCxuYemh99336027;     zQvTogvjwVCxuYemh99336027 = zQvTogvjwVCxuYemh47935950;     zQvTogvjwVCxuYemh47935950 = zQvTogvjwVCxuYemh91385386;     zQvTogvjwVCxuYemh91385386 = zQvTogvjwVCxuYemh88621708;     zQvTogvjwVCxuYemh88621708 = zQvTogvjwVCxuYemh43833063;     zQvTogvjwVCxuYemh43833063 = zQvTogvjwVCxuYemh13655532;     zQvTogvjwVCxuYemh13655532 = zQvTogvjwVCxuYemh29644350;     zQvTogvjwVCxuYemh29644350 = zQvTogvjwVCxuYemh70420730;     zQvTogvjwVCxuYemh70420730 = zQvTogvjwVCxuYemh15917175;     zQvTogvjwVCxuYemh15917175 = zQvTogvjwVCxuYemh54226961;     zQvTogvjwVCxuYemh54226961 = zQvTogvjwVCxuYemh38300920;     zQvTogvjwVCxuYemh38300920 = zQvTogvjwVCxuYemh15422998;     zQvTogvjwVCxuYemh15422998 = zQvTogvjwVCxuYemh49279484;     zQvTogvjwVCxuYemh49279484 = zQvTogvjwVCxuYemh26034599;     zQvTogvjwVCxuYemh26034599 = zQvTogvjwVCxuYemh47705843;     zQvTogvjwVCxuYemh47705843 = zQvTogvjwVCxuYemh61065558;     zQvTogvjwVCxuYemh61065558 = zQvTogvjwVCxuYemh20049882;     zQvTogvjwVCxuYemh20049882 = zQvTogvjwVCxuYemh65196877;     zQvTogvjwVCxuYemh65196877 = zQvTogvjwVCxuYemh26387706;     zQvTogvjwVCxuYemh26387706 = zQvTogvjwVCxuYemh4231504;     zQvTogvjwVCxuYemh4231504 = zQvTogvjwVCxuYemh85693654;     zQvTogvjwVCxuYemh85693654 = zQvTogvjwVCxuYemh80515971;     zQvTogvjwVCxuYemh80515971 = zQvTogvjwVCxuYemh13077091;     zQvTogvjwVCxuYemh13077091 = zQvTogvjwVCxuYemh64254155;     zQvTogvjwVCxuYemh64254155 = zQvTogvjwVCxuYemh77432587;     zQvTogvjwVCxuYemh77432587 = zQvTogvjwVCxuYemh72352146;     zQvTogvjwVCxuYemh72352146 = zQvTogvjwVCxuYemh32150584;     zQvTogvjwVCxuYemh32150584 = zQvTogvjwVCxuYemh47233908;     zQvTogvjwVCxuYemh47233908 = zQvTogvjwVCxuYemh54474457;     zQvTogvjwVCxuYemh54474457 = zQvTogvjwVCxuYemh11354237;     zQvTogvjwVCxuYemh11354237 = zQvTogvjwVCxuYemh14194202;     zQvTogvjwVCxuYemh14194202 = zQvTogvjwVCxuYemh93990993;     zQvTogvjwVCxuYemh93990993 = zQvTogvjwVCxuYemh24509820;     zQvTogvjwVCxuYemh24509820 = zQvTogvjwVCxuYemh84613272;     zQvTogvjwVCxuYemh84613272 = zQvTogvjwVCxuYemh26812039;     zQvTogvjwVCxuYemh26812039 = zQvTogvjwVCxuYemh48515667;     zQvTogvjwVCxuYemh48515667 = zQvTogvjwVCxuYemh92380029;     zQvTogvjwVCxuYemh92380029 = zQvTogvjwVCxuYemh83301932;     zQvTogvjwVCxuYemh83301932 = zQvTogvjwVCxuYemh8027391;     zQvTogvjwVCxuYemh8027391 = zQvTogvjwVCxuYemh56817532;     zQvTogvjwVCxuYemh56817532 = zQvTogvjwVCxuYemh28475223;     zQvTogvjwVCxuYemh28475223 = zQvTogvjwVCxuYemh76970807;     zQvTogvjwVCxuYemh76970807 = zQvTogvjwVCxuYemh70799097;     zQvTogvjwVCxuYemh70799097 = zQvTogvjwVCxuYemh31136606;     zQvTogvjwVCxuYemh31136606 = zQvTogvjwVCxuYemh92206644;     zQvTogvjwVCxuYemh92206644 = zQvTogvjwVCxuYemh25186494;     zQvTogvjwVCxuYemh25186494 = zQvTogvjwVCxuYemh92157208;     zQvTogvjwVCxuYemh92157208 = zQvTogvjwVCxuYemh24610693;     zQvTogvjwVCxuYemh24610693 = zQvTogvjwVCxuYemh91784633;     zQvTogvjwVCxuYemh91784633 = zQvTogvjwVCxuYemh6223946;     zQvTogvjwVCxuYemh6223946 = zQvTogvjwVCxuYemh97114780;     zQvTogvjwVCxuYemh97114780 = zQvTogvjwVCxuYemh24004380;     zQvTogvjwVCxuYemh24004380 = zQvTogvjwVCxuYemh65469823;     zQvTogvjwVCxuYemh65469823 = zQvTogvjwVCxuYemh66320389;     zQvTogvjwVCxuYemh66320389 = zQvTogvjwVCxuYemh53570185;     zQvTogvjwVCxuYemh53570185 = zQvTogvjwVCxuYemh18820057;     zQvTogvjwVCxuYemh18820057 = zQvTogvjwVCxuYemh34858860;     zQvTogvjwVCxuYemh34858860 = zQvTogvjwVCxuYemh27131232;     zQvTogvjwVCxuYemh27131232 = zQvTogvjwVCxuYemh11189121;     zQvTogvjwVCxuYemh11189121 = zQvTogvjwVCxuYemh71480916;     zQvTogvjwVCxuYemh71480916 = zQvTogvjwVCxuYemh81504947;     zQvTogvjwVCxuYemh81504947 = zQvTogvjwVCxuYemh82410442;     zQvTogvjwVCxuYemh82410442 = zQvTogvjwVCxuYemh15946274;     zQvTogvjwVCxuYemh15946274 = zQvTogvjwVCxuYemh4562938;     zQvTogvjwVCxuYemh4562938 = zQvTogvjwVCxuYemh40032760;     zQvTogvjwVCxuYemh40032760 = zQvTogvjwVCxuYemh44309926;     zQvTogvjwVCxuYemh44309926 = zQvTogvjwVCxuYemh90913177;     zQvTogvjwVCxuYemh90913177 = zQvTogvjwVCxuYemh64666212;     zQvTogvjwVCxuYemh64666212 = zQvTogvjwVCxuYemh99222559;     zQvTogvjwVCxuYemh99222559 = zQvTogvjwVCxuYemh99190175;     zQvTogvjwVCxuYemh99190175 = zQvTogvjwVCxuYemh68685529;     zQvTogvjwVCxuYemh68685529 = zQvTogvjwVCxuYemh36747949;     zQvTogvjwVCxuYemh36747949 = zQvTogvjwVCxuYemh57169486;     zQvTogvjwVCxuYemh57169486 = zQvTogvjwVCxuYemh69570173;     zQvTogvjwVCxuYemh69570173 = zQvTogvjwVCxuYemh75756281;     zQvTogvjwVCxuYemh75756281 = zQvTogvjwVCxuYemh8722848;     zQvTogvjwVCxuYemh8722848 = zQvTogvjwVCxuYemh9716874;     zQvTogvjwVCxuYemh9716874 = zQvTogvjwVCxuYemh81940484;     zQvTogvjwVCxuYemh81940484 = zQvTogvjwVCxuYemh72047511;     zQvTogvjwVCxuYemh72047511 = zQvTogvjwVCxuYemh52246094;     zQvTogvjwVCxuYemh52246094 = zQvTogvjwVCxuYemh80194937;     zQvTogvjwVCxuYemh80194937 = zQvTogvjwVCxuYemh7539892;     zQvTogvjwVCxuYemh7539892 = zQvTogvjwVCxuYemh55449275;     zQvTogvjwVCxuYemh55449275 = zQvTogvjwVCxuYemh48250512;     zQvTogvjwVCxuYemh48250512 = zQvTogvjwVCxuYemh14239456;     zQvTogvjwVCxuYemh14239456 = zQvTogvjwVCxuYemh90189821;     zQvTogvjwVCxuYemh90189821 = zQvTogvjwVCxuYemh28521171;     zQvTogvjwVCxuYemh28521171 = zQvTogvjwVCxuYemh58189431;     zQvTogvjwVCxuYemh58189431 = zQvTogvjwVCxuYemh85363567;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KYNDopnayCKfxqOS1765340() {     double CVVRMynJdtoOKbNaZ91480673 = -770427934;    double CVVRMynJdtoOKbNaZ73880843 = -621687974;    double CVVRMynJdtoOKbNaZ56651675 = 99391965;    double CVVRMynJdtoOKbNaZ68511474 = -473011852;    double CVVRMynJdtoOKbNaZ19540780 = -727408939;    double CVVRMynJdtoOKbNaZ30907678 = -862485021;    double CVVRMynJdtoOKbNaZ27778470 = -310526548;    double CVVRMynJdtoOKbNaZ66924553 = -780161328;    double CVVRMynJdtoOKbNaZ53843752 = -96762797;    double CVVRMynJdtoOKbNaZ33087188 = -600377719;    double CVVRMynJdtoOKbNaZ30662183 = -975295281;    double CVVRMynJdtoOKbNaZ71086882 = -917542303;    double CVVRMynJdtoOKbNaZ85545146 = 37115388;    double CVVRMynJdtoOKbNaZ31079627 = -129837606;    double CVVRMynJdtoOKbNaZ87127813 = -311192710;    double CVVRMynJdtoOKbNaZ30127102 = 45539493;    double CVVRMynJdtoOKbNaZ37669066 = -103622535;    double CVVRMynJdtoOKbNaZ10708685 = -1487493;    double CVVRMynJdtoOKbNaZ91474192 = -670585270;    double CVVRMynJdtoOKbNaZ50970895 = -302006775;    double CVVRMynJdtoOKbNaZ4470368 = -959282262;    double CVVRMynJdtoOKbNaZ40747985 = -414609000;    double CVVRMynJdtoOKbNaZ73891712 = -646382483;    double CVVRMynJdtoOKbNaZ99415371 = -640202866;    double CVVRMynJdtoOKbNaZ92979134 = -767275109;    double CVVRMynJdtoOKbNaZ50756274 = -230282030;    double CVVRMynJdtoOKbNaZ46165621 = -996516439;    double CVVRMynJdtoOKbNaZ74172684 = -252313338;    double CVVRMynJdtoOKbNaZ86182137 = -925809871;    double CVVRMynJdtoOKbNaZ33812514 = -72817490;    double CVVRMynJdtoOKbNaZ18630255 = 80237358;    double CVVRMynJdtoOKbNaZ1842126 = -161174493;    double CVVRMynJdtoOKbNaZ95404603 = -718937062;    double CVVRMynJdtoOKbNaZ75346535 = -391580548;    double CVVRMynJdtoOKbNaZ74551742 = -168878728;    double CVVRMynJdtoOKbNaZ32980068 = -210489756;    double CVVRMynJdtoOKbNaZ58694522 = -12768828;    double CVVRMynJdtoOKbNaZ6442359 = -733701968;    double CVVRMynJdtoOKbNaZ5760838 = -282936989;    double CVVRMynJdtoOKbNaZ34735846 = -146586086;    double CVVRMynJdtoOKbNaZ60143681 = -28440414;    double CVVRMynJdtoOKbNaZ43504694 = -943535774;    double CVVRMynJdtoOKbNaZ92261179 = -556123130;    double CVVRMynJdtoOKbNaZ81398734 = -541055083;    double CVVRMynJdtoOKbNaZ36462285 = -762695128;    double CVVRMynJdtoOKbNaZ96552467 = -719557397;    double CVVRMynJdtoOKbNaZ44590613 = -921074984;    double CVVRMynJdtoOKbNaZ56472608 = -430152180;    double CVVRMynJdtoOKbNaZ2055759 = -182067639;    double CVVRMynJdtoOKbNaZ96213585 = -395607819;    double CVVRMynJdtoOKbNaZ43728069 = -724460620;    double CVVRMynJdtoOKbNaZ67627251 = -208127456;    double CVVRMynJdtoOKbNaZ76723679 = -534885380;    double CVVRMynJdtoOKbNaZ72939687 = -961633413;    double CVVRMynJdtoOKbNaZ97294972 = -956653240;    double CVVRMynJdtoOKbNaZ50732689 = -255818934;    double CVVRMynJdtoOKbNaZ99989131 = -975305491;    double CVVRMynJdtoOKbNaZ57236304 = -260405170;    double CVVRMynJdtoOKbNaZ75532339 = -705736744;    double CVVRMynJdtoOKbNaZ68784505 = -397126910;    double CVVRMynJdtoOKbNaZ84742057 = -865968583;    double CVVRMynJdtoOKbNaZ53605786 = 41786789;    double CVVRMynJdtoOKbNaZ80742416 = -854351458;    double CVVRMynJdtoOKbNaZ20031239 = 76054692;    double CVVRMynJdtoOKbNaZ14456933 = -580615078;    double CVVRMynJdtoOKbNaZ28820058 = -714120788;    double CVVRMynJdtoOKbNaZ75682279 = -98605242;    double CVVRMynJdtoOKbNaZ10198611 = -571304065;    double CVVRMynJdtoOKbNaZ56527884 = -960958879;    double CVVRMynJdtoOKbNaZ54147745 = -702954;    double CVVRMynJdtoOKbNaZ71432579 = -941691680;    double CVVRMynJdtoOKbNaZ31226707 = -369920567;    double CVVRMynJdtoOKbNaZ4947847 = -718550504;    double CVVRMynJdtoOKbNaZ56738346 = -423999184;    double CVVRMynJdtoOKbNaZ90827214 = -173566361;    double CVVRMynJdtoOKbNaZ60965674 = 84253511;    double CVVRMynJdtoOKbNaZ48486805 = -858485870;    double CVVRMynJdtoOKbNaZ92492977 = -5327401;    double CVVRMynJdtoOKbNaZ62953086 = -877507738;    double CVVRMynJdtoOKbNaZ96426667 = 52282287;    double CVVRMynJdtoOKbNaZ6165661 = -309207047;    double CVVRMynJdtoOKbNaZ89693012 = -466364260;    double CVVRMynJdtoOKbNaZ72116925 = 29754301;    double CVVRMynJdtoOKbNaZ89968552 = -430202053;    double CVVRMynJdtoOKbNaZ90084444 = -348356871;    double CVVRMynJdtoOKbNaZ51003004 = -711635186;    double CVVRMynJdtoOKbNaZ25118446 = -626289114;    double CVVRMynJdtoOKbNaZ22464916 = -757303650;    double CVVRMynJdtoOKbNaZ78051563 = -434927308;    double CVVRMynJdtoOKbNaZ23819054 = -913059795;    double CVVRMynJdtoOKbNaZ32990937 = -235184265;    double CVVRMynJdtoOKbNaZ1458219 = -752363658;    double CVVRMynJdtoOKbNaZ30910020 = 72034775;    double CVVRMynJdtoOKbNaZ36976332 = -885810080;    double CVVRMynJdtoOKbNaZ49993789 = -280617504;    double CVVRMynJdtoOKbNaZ6537896 = 29772797;    double CVVRMynJdtoOKbNaZ62762277 = 10815684;    double CVVRMynJdtoOKbNaZ72229941 = -532177823;    double CVVRMynJdtoOKbNaZ66941801 = -960440006;    double CVVRMynJdtoOKbNaZ7642228 = -770427934;     CVVRMynJdtoOKbNaZ91480673 = CVVRMynJdtoOKbNaZ73880843;     CVVRMynJdtoOKbNaZ73880843 = CVVRMynJdtoOKbNaZ56651675;     CVVRMynJdtoOKbNaZ56651675 = CVVRMynJdtoOKbNaZ68511474;     CVVRMynJdtoOKbNaZ68511474 = CVVRMynJdtoOKbNaZ19540780;     CVVRMynJdtoOKbNaZ19540780 = CVVRMynJdtoOKbNaZ30907678;     CVVRMynJdtoOKbNaZ30907678 = CVVRMynJdtoOKbNaZ27778470;     CVVRMynJdtoOKbNaZ27778470 = CVVRMynJdtoOKbNaZ66924553;     CVVRMynJdtoOKbNaZ66924553 = CVVRMynJdtoOKbNaZ53843752;     CVVRMynJdtoOKbNaZ53843752 = CVVRMynJdtoOKbNaZ33087188;     CVVRMynJdtoOKbNaZ33087188 = CVVRMynJdtoOKbNaZ30662183;     CVVRMynJdtoOKbNaZ30662183 = CVVRMynJdtoOKbNaZ71086882;     CVVRMynJdtoOKbNaZ71086882 = CVVRMynJdtoOKbNaZ85545146;     CVVRMynJdtoOKbNaZ85545146 = CVVRMynJdtoOKbNaZ31079627;     CVVRMynJdtoOKbNaZ31079627 = CVVRMynJdtoOKbNaZ87127813;     CVVRMynJdtoOKbNaZ87127813 = CVVRMynJdtoOKbNaZ30127102;     CVVRMynJdtoOKbNaZ30127102 = CVVRMynJdtoOKbNaZ37669066;     CVVRMynJdtoOKbNaZ37669066 = CVVRMynJdtoOKbNaZ10708685;     CVVRMynJdtoOKbNaZ10708685 = CVVRMynJdtoOKbNaZ91474192;     CVVRMynJdtoOKbNaZ91474192 = CVVRMynJdtoOKbNaZ50970895;     CVVRMynJdtoOKbNaZ50970895 = CVVRMynJdtoOKbNaZ4470368;     CVVRMynJdtoOKbNaZ4470368 = CVVRMynJdtoOKbNaZ40747985;     CVVRMynJdtoOKbNaZ40747985 = CVVRMynJdtoOKbNaZ73891712;     CVVRMynJdtoOKbNaZ73891712 = CVVRMynJdtoOKbNaZ99415371;     CVVRMynJdtoOKbNaZ99415371 = CVVRMynJdtoOKbNaZ92979134;     CVVRMynJdtoOKbNaZ92979134 = CVVRMynJdtoOKbNaZ50756274;     CVVRMynJdtoOKbNaZ50756274 = CVVRMynJdtoOKbNaZ46165621;     CVVRMynJdtoOKbNaZ46165621 = CVVRMynJdtoOKbNaZ74172684;     CVVRMynJdtoOKbNaZ74172684 = CVVRMynJdtoOKbNaZ86182137;     CVVRMynJdtoOKbNaZ86182137 = CVVRMynJdtoOKbNaZ33812514;     CVVRMynJdtoOKbNaZ33812514 = CVVRMynJdtoOKbNaZ18630255;     CVVRMynJdtoOKbNaZ18630255 = CVVRMynJdtoOKbNaZ1842126;     CVVRMynJdtoOKbNaZ1842126 = CVVRMynJdtoOKbNaZ95404603;     CVVRMynJdtoOKbNaZ95404603 = CVVRMynJdtoOKbNaZ75346535;     CVVRMynJdtoOKbNaZ75346535 = CVVRMynJdtoOKbNaZ74551742;     CVVRMynJdtoOKbNaZ74551742 = CVVRMynJdtoOKbNaZ32980068;     CVVRMynJdtoOKbNaZ32980068 = CVVRMynJdtoOKbNaZ58694522;     CVVRMynJdtoOKbNaZ58694522 = CVVRMynJdtoOKbNaZ6442359;     CVVRMynJdtoOKbNaZ6442359 = CVVRMynJdtoOKbNaZ5760838;     CVVRMynJdtoOKbNaZ5760838 = CVVRMynJdtoOKbNaZ34735846;     CVVRMynJdtoOKbNaZ34735846 = CVVRMynJdtoOKbNaZ60143681;     CVVRMynJdtoOKbNaZ60143681 = CVVRMynJdtoOKbNaZ43504694;     CVVRMynJdtoOKbNaZ43504694 = CVVRMynJdtoOKbNaZ92261179;     CVVRMynJdtoOKbNaZ92261179 = CVVRMynJdtoOKbNaZ81398734;     CVVRMynJdtoOKbNaZ81398734 = CVVRMynJdtoOKbNaZ36462285;     CVVRMynJdtoOKbNaZ36462285 = CVVRMynJdtoOKbNaZ96552467;     CVVRMynJdtoOKbNaZ96552467 = CVVRMynJdtoOKbNaZ44590613;     CVVRMynJdtoOKbNaZ44590613 = CVVRMynJdtoOKbNaZ56472608;     CVVRMynJdtoOKbNaZ56472608 = CVVRMynJdtoOKbNaZ2055759;     CVVRMynJdtoOKbNaZ2055759 = CVVRMynJdtoOKbNaZ96213585;     CVVRMynJdtoOKbNaZ96213585 = CVVRMynJdtoOKbNaZ43728069;     CVVRMynJdtoOKbNaZ43728069 = CVVRMynJdtoOKbNaZ67627251;     CVVRMynJdtoOKbNaZ67627251 = CVVRMynJdtoOKbNaZ76723679;     CVVRMynJdtoOKbNaZ76723679 = CVVRMynJdtoOKbNaZ72939687;     CVVRMynJdtoOKbNaZ72939687 = CVVRMynJdtoOKbNaZ97294972;     CVVRMynJdtoOKbNaZ97294972 = CVVRMynJdtoOKbNaZ50732689;     CVVRMynJdtoOKbNaZ50732689 = CVVRMynJdtoOKbNaZ99989131;     CVVRMynJdtoOKbNaZ99989131 = CVVRMynJdtoOKbNaZ57236304;     CVVRMynJdtoOKbNaZ57236304 = CVVRMynJdtoOKbNaZ75532339;     CVVRMynJdtoOKbNaZ75532339 = CVVRMynJdtoOKbNaZ68784505;     CVVRMynJdtoOKbNaZ68784505 = CVVRMynJdtoOKbNaZ84742057;     CVVRMynJdtoOKbNaZ84742057 = CVVRMynJdtoOKbNaZ53605786;     CVVRMynJdtoOKbNaZ53605786 = CVVRMynJdtoOKbNaZ80742416;     CVVRMynJdtoOKbNaZ80742416 = CVVRMynJdtoOKbNaZ20031239;     CVVRMynJdtoOKbNaZ20031239 = CVVRMynJdtoOKbNaZ14456933;     CVVRMynJdtoOKbNaZ14456933 = CVVRMynJdtoOKbNaZ28820058;     CVVRMynJdtoOKbNaZ28820058 = CVVRMynJdtoOKbNaZ75682279;     CVVRMynJdtoOKbNaZ75682279 = CVVRMynJdtoOKbNaZ10198611;     CVVRMynJdtoOKbNaZ10198611 = CVVRMynJdtoOKbNaZ56527884;     CVVRMynJdtoOKbNaZ56527884 = CVVRMynJdtoOKbNaZ54147745;     CVVRMynJdtoOKbNaZ54147745 = CVVRMynJdtoOKbNaZ71432579;     CVVRMynJdtoOKbNaZ71432579 = CVVRMynJdtoOKbNaZ31226707;     CVVRMynJdtoOKbNaZ31226707 = CVVRMynJdtoOKbNaZ4947847;     CVVRMynJdtoOKbNaZ4947847 = CVVRMynJdtoOKbNaZ56738346;     CVVRMynJdtoOKbNaZ56738346 = CVVRMynJdtoOKbNaZ90827214;     CVVRMynJdtoOKbNaZ90827214 = CVVRMynJdtoOKbNaZ60965674;     CVVRMynJdtoOKbNaZ60965674 = CVVRMynJdtoOKbNaZ48486805;     CVVRMynJdtoOKbNaZ48486805 = CVVRMynJdtoOKbNaZ92492977;     CVVRMynJdtoOKbNaZ92492977 = CVVRMynJdtoOKbNaZ62953086;     CVVRMynJdtoOKbNaZ62953086 = CVVRMynJdtoOKbNaZ96426667;     CVVRMynJdtoOKbNaZ96426667 = CVVRMynJdtoOKbNaZ6165661;     CVVRMynJdtoOKbNaZ6165661 = CVVRMynJdtoOKbNaZ89693012;     CVVRMynJdtoOKbNaZ89693012 = CVVRMynJdtoOKbNaZ72116925;     CVVRMynJdtoOKbNaZ72116925 = CVVRMynJdtoOKbNaZ89968552;     CVVRMynJdtoOKbNaZ89968552 = CVVRMynJdtoOKbNaZ90084444;     CVVRMynJdtoOKbNaZ90084444 = CVVRMynJdtoOKbNaZ51003004;     CVVRMynJdtoOKbNaZ51003004 = CVVRMynJdtoOKbNaZ25118446;     CVVRMynJdtoOKbNaZ25118446 = CVVRMynJdtoOKbNaZ22464916;     CVVRMynJdtoOKbNaZ22464916 = CVVRMynJdtoOKbNaZ78051563;     CVVRMynJdtoOKbNaZ78051563 = CVVRMynJdtoOKbNaZ23819054;     CVVRMynJdtoOKbNaZ23819054 = CVVRMynJdtoOKbNaZ32990937;     CVVRMynJdtoOKbNaZ32990937 = CVVRMynJdtoOKbNaZ1458219;     CVVRMynJdtoOKbNaZ1458219 = CVVRMynJdtoOKbNaZ30910020;     CVVRMynJdtoOKbNaZ30910020 = CVVRMynJdtoOKbNaZ36976332;     CVVRMynJdtoOKbNaZ36976332 = CVVRMynJdtoOKbNaZ49993789;     CVVRMynJdtoOKbNaZ49993789 = CVVRMynJdtoOKbNaZ6537896;     CVVRMynJdtoOKbNaZ6537896 = CVVRMynJdtoOKbNaZ62762277;     CVVRMynJdtoOKbNaZ62762277 = CVVRMynJdtoOKbNaZ72229941;     CVVRMynJdtoOKbNaZ72229941 = CVVRMynJdtoOKbNaZ66941801;     CVVRMynJdtoOKbNaZ66941801 = CVVRMynJdtoOKbNaZ7642228;     CVVRMynJdtoOKbNaZ7642228 = CVVRMynJdtoOKbNaZ91480673;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mZuHrLdFsIwwiqQj38410314() {     double LpRsTcSYQtqssZaYf13867792 = -404831897;    double LpRsTcSYQtqssZaYf58228344 = -570231028;    double LpRsTcSYQtqssZaYf85420954 = -140261880;    double LpRsTcSYQtqssZaYf83518162 = -479419384;    double LpRsTcSYQtqssZaYf17715034 = -218465941;    double LpRsTcSYQtqssZaYf45790795 = -723093399;    double LpRsTcSYQtqssZaYf19934209 = -132513854;    double LpRsTcSYQtqssZaYf42558412 = -150757972;    double LpRsTcSYQtqssZaYf51006883 = 19041163;    double LpRsTcSYQtqssZaYf24850773 = -343119052;    double LpRsTcSYQtqssZaYf68204043 = -971919682;    double LpRsTcSYQtqssZaYf26989832 = -68287596;    double LpRsTcSYQtqssZaYf52553327 = -581601932;    double LpRsTcSYQtqssZaYf73694379 = -657546363;    double LpRsTcSYQtqssZaYf23863090 = 61329577;    double LpRsTcSYQtqssZaYf74383014 = -336128049;    double LpRsTcSYQtqssZaYf87492417 = -380902355;    double LpRsTcSYQtqssZaYf16685101 = -453230184;    double LpRsTcSYQtqssZaYf32755823 = -805687395;    double LpRsTcSYQtqssZaYf29487179 = -554641228;    double LpRsTcSYQtqssZaYf5353475 = -950769570;    double LpRsTcSYQtqssZaYf4563204 = -919375313;    double LpRsTcSYQtqssZaYf77185035 = -822463075;    double LpRsTcSYQtqssZaYf25915159 = -104195723;    double LpRsTcSYQtqssZaYf28519382 = -388834233;    double LpRsTcSYQtqssZaYf39059269 = -674176254;    double LpRsTcSYQtqssZaYf93255041 = -754102932;    double LpRsTcSYQtqssZaYf80084134 = -40767791;    double LpRsTcSYQtqssZaYf93074698 = -170066990;    double LpRsTcSYQtqssZaYf42969693 = 6886643;    double LpRsTcSYQtqssZaYf62921060 = -1078794;    double LpRsTcSYQtqssZaYf59383316 = -813245848;    double LpRsTcSYQtqssZaYf76443273 = -148280557;    double LpRsTcSYQtqssZaYf34735223 = -715707398;    double LpRsTcSYQtqssZaYf50781107 = -232635151;    double LpRsTcSYQtqssZaYf74672937 = -525885133;    double LpRsTcSYQtqssZaYf52134234 = -845716780;    double LpRsTcSYQtqssZaYf60068460 = -601327440;    double LpRsTcSYQtqssZaYf54068969 = -683983021;    double LpRsTcSYQtqssZaYf54921561 = -282810115;    double LpRsTcSYQtqssZaYf88901052 = -531618080;    double LpRsTcSYQtqssZaYf32013085 = -701444465;    double LpRsTcSYQtqssZaYf97811371 = -443653714;    double LpRsTcSYQtqssZaYf80842853 = -538418732;    double LpRsTcSYQtqssZaYf4445552 = -571838606;    double LpRsTcSYQtqssZaYf4652750 = -901786492;    double LpRsTcSYQtqssZaYf56888010 = -399485237;    double LpRsTcSYQtqssZaYf1071283 = -186687171;    double LpRsTcSYQtqssZaYf96515663 = -131784337;    double LpRsTcSYQtqssZaYf93297330 = -537628078;    double LpRsTcSYQtqssZaYf52597285 = 20463103;    double LpRsTcSYQtqssZaYf41478509 = -638946738;    double LpRsTcSYQtqssZaYf55038083 = -206796610;    double LpRsTcSYQtqssZaYf33226233 = -137314222;    double LpRsTcSYQtqssZaYf33056840 = 2642644;    double LpRsTcSYQtqssZaYf9304588 = -485456584;    double LpRsTcSYQtqssZaYf81043309 = -747767954;    double LpRsTcSYQtqssZaYf59505795 = 63933843;    double LpRsTcSYQtqssZaYf54998781 = 9414849;    double LpRsTcSYQtqssZaYf78655765 = -544289687;    double LpRsTcSYQtqssZaYf52535754 = -968990468;    double LpRsTcSYQtqssZaYf39850075 = 8253936;    double LpRsTcSYQtqssZaYf49483714 = -980690983;    double LpRsTcSYQtqssZaYf8037191 = -987845480;    double LpRsTcSYQtqssZaYf61929712 = -242040259;    double LpRsTcSYQtqssZaYf8820727 = -58673834;    double LpRsTcSYQtqssZaYf50546558 = -920007040;    double LpRsTcSYQtqssZaYf17818104 = -865894535;    double LpRsTcSYQtqssZaYf22913272 = -324911213;    double LpRsTcSYQtqssZaYf49190153 = -412785291;    double LpRsTcSYQtqssZaYf22248780 = -490411270;    double LpRsTcSYQtqssZaYf27423958 = -779574915;    double LpRsTcSYQtqssZaYf62616132 = -769247163;    double LpRsTcSYQtqssZaYf77834262 = -422877280;    double LpRsTcSYQtqssZaYf40586127 = 76976851;    double LpRsTcSYQtqssZaYf73340389 = -149325106;    double LpRsTcSYQtqssZaYf6751832 = -375721600;    double LpRsTcSYQtqssZaYf96342181 = -184044344;    double LpRsTcSYQtqssZaYf21469607 = -532357117;    double LpRsTcSYQtqssZaYf23866632 = -487047742;    double LpRsTcSYQtqssZaYf82171258 = -174691018;    double LpRsTcSYQtqssZaYf92183758 = -467415762;    double LpRsTcSYQtqssZaYf83568470 = -908983455;    double LpRsTcSYQtqssZaYf99777367 = -632438913;    double LpRsTcSYQtqssZaYf90372407 = 86423540;    double LpRsTcSYQtqssZaYf21442552 = -362132056;    double LpRsTcSYQtqssZaYf4345234 = -506449238;    double LpRsTcSYQtqssZaYf43217041 = 89033664;    double LpRsTcSYQtqssZaYf1678384 = -618350043;    double LpRsTcSYQtqssZaYf41476519 = -747178567;    double LpRsTcSYQtqssZaYf93629628 = -778117179;    double LpRsTcSYQtqssZaYf92628439 = -809650623;    double LpRsTcSYQtqssZaYf5069679 = -510742290;    double LpRsTcSYQtqssZaYf75413203 = -39693335;    double LpRsTcSYQtqssZaYf2385807 = -313819648;    double LpRsTcSYQtqssZaYf49050977 = -439872017;    double LpRsTcSYQtqssZaYf82529371 = -720753482;    double LpRsTcSYQtqssZaYf89774181 = -455808235;    double LpRsTcSYQtqssZaYf18913141 = -196378474;    double LpRsTcSYQtqssZaYf95624825 = -404831897;     LpRsTcSYQtqssZaYf13867792 = LpRsTcSYQtqssZaYf58228344;     LpRsTcSYQtqssZaYf58228344 = LpRsTcSYQtqssZaYf85420954;     LpRsTcSYQtqssZaYf85420954 = LpRsTcSYQtqssZaYf83518162;     LpRsTcSYQtqssZaYf83518162 = LpRsTcSYQtqssZaYf17715034;     LpRsTcSYQtqssZaYf17715034 = LpRsTcSYQtqssZaYf45790795;     LpRsTcSYQtqssZaYf45790795 = LpRsTcSYQtqssZaYf19934209;     LpRsTcSYQtqssZaYf19934209 = LpRsTcSYQtqssZaYf42558412;     LpRsTcSYQtqssZaYf42558412 = LpRsTcSYQtqssZaYf51006883;     LpRsTcSYQtqssZaYf51006883 = LpRsTcSYQtqssZaYf24850773;     LpRsTcSYQtqssZaYf24850773 = LpRsTcSYQtqssZaYf68204043;     LpRsTcSYQtqssZaYf68204043 = LpRsTcSYQtqssZaYf26989832;     LpRsTcSYQtqssZaYf26989832 = LpRsTcSYQtqssZaYf52553327;     LpRsTcSYQtqssZaYf52553327 = LpRsTcSYQtqssZaYf73694379;     LpRsTcSYQtqssZaYf73694379 = LpRsTcSYQtqssZaYf23863090;     LpRsTcSYQtqssZaYf23863090 = LpRsTcSYQtqssZaYf74383014;     LpRsTcSYQtqssZaYf74383014 = LpRsTcSYQtqssZaYf87492417;     LpRsTcSYQtqssZaYf87492417 = LpRsTcSYQtqssZaYf16685101;     LpRsTcSYQtqssZaYf16685101 = LpRsTcSYQtqssZaYf32755823;     LpRsTcSYQtqssZaYf32755823 = LpRsTcSYQtqssZaYf29487179;     LpRsTcSYQtqssZaYf29487179 = LpRsTcSYQtqssZaYf5353475;     LpRsTcSYQtqssZaYf5353475 = LpRsTcSYQtqssZaYf4563204;     LpRsTcSYQtqssZaYf4563204 = LpRsTcSYQtqssZaYf77185035;     LpRsTcSYQtqssZaYf77185035 = LpRsTcSYQtqssZaYf25915159;     LpRsTcSYQtqssZaYf25915159 = LpRsTcSYQtqssZaYf28519382;     LpRsTcSYQtqssZaYf28519382 = LpRsTcSYQtqssZaYf39059269;     LpRsTcSYQtqssZaYf39059269 = LpRsTcSYQtqssZaYf93255041;     LpRsTcSYQtqssZaYf93255041 = LpRsTcSYQtqssZaYf80084134;     LpRsTcSYQtqssZaYf80084134 = LpRsTcSYQtqssZaYf93074698;     LpRsTcSYQtqssZaYf93074698 = LpRsTcSYQtqssZaYf42969693;     LpRsTcSYQtqssZaYf42969693 = LpRsTcSYQtqssZaYf62921060;     LpRsTcSYQtqssZaYf62921060 = LpRsTcSYQtqssZaYf59383316;     LpRsTcSYQtqssZaYf59383316 = LpRsTcSYQtqssZaYf76443273;     LpRsTcSYQtqssZaYf76443273 = LpRsTcSYQtqssZaYf34735223;     LpRsTcSYQtqssZaYf34735223 = LpRsTcSYQtqssZaYf50781107;     LpRsTcSYQtqssZaYf50781107 = LpRsTcSYQtqssZaYf74672937;     LpRsTcSYQtqssZaYf74672937 = LpRsTcSYQtqssZaYf52134234;     LpRsTcSYQtqssZaYf52134234 = LpRsTcSYQtqssZaYf60068460;     LpRsTcSYQtqssZaYf60068460 = LpRsTcSYQtqssZaYf54068969;     LpRsTcSYQtqssZaYf54068969 = LpRsTcSYQtqssZaYf54921561;     LpRsTcSYQtqssZaYf54921561 = LpRsTcSYQtqssZaYf88901052;     LpRsTcSYQtqssZaYf88901052 = LpRsTcSYQtqssZaYf32013085;     LpRsTcSYQtqssZaYf32013085 = LpRsTcSYQtqssZaYf97811371;     LpRsTcSYQtqssZaYf97811371 = LpRsTcSYQtqssZaYf80842853;     LpRsTcSYQtqssZaYf80842853 = LpRsTcSYQtqssZaYf4445552;     LpRsTcSYQtqssZaYf4445552 = LpRsTcSYQtqssZaYf4652750;     LpRsTcSYQtqssZaYf4652750 = LpRsTcSYQtqssZaYf56888010;     LpRsTcSYQtqssZaYf56888010 = LpRsTcSYQtqssZaYf1071283;     LpRsTcSYQtqssZaYf1071283 = LpRsTcSYQtqssZaYf96515663;     LpRsTcSYQtqssZaYf96515663 = LpRsTcSYQtqssZaYf93297330;     LpRsTcSYQtqssZaYf93297330 = LpRsTcSYQtqssZaYf52597285;     LpRsTcSYQtqssZaYf52597285 = LpRsTcSYQtqssZaYf41478509;     LpRsTcSYQtqssZaYf41478509 = LpRsTcSYQtqssZaYf55038083;     LpRsTcSYQtqssZaYf55038083 = LpRsTcSYQtqssZaYf33226233;     LpRsTcSYQtqssZaYf33226233 = LpRsTcSYQtqssZaYf33056840;     LpRsTcSYQtqssZaYf33056840 = LpRsTcSYQtqssZaYf9304588;     LpRsTcSYQtqssZaYf9304588 = LpRsTcSYQtqssZaYf81043309;     LpRsTcSYQtqssZaYf81043309 = LpRsTcSYQtqssZaYf59505795;     LpRsTcSYQtqssZaYf59505795 = LpRsTcSYQtqssZaYf54998781;     LpRsTcSYQtqssZaYf54998781 = LpRsTcSYQtqssZaYf78655765;     LpRsTcSYQtqssZaYf78655765 = LpRsTcSYQtqssZaYf52535754;     LpRsTcSYQtqssZaYf52535754 = LpRsTcSYQtqssZaYf39850075;     LpRsTcSYQtqssZaYf39850075 = LpRsTcSYQtqssZaYf49483714;     LpRsTcSYQtqssZaYf49483714 = LpRsTcSYQtqssZaYf8037191;     LpRsTcSYQtqssZaYf8037191 = LpRsTcSYQtqssZaYf61929712;     LpRsTcSYQtqssZaYf61929712 = LpRsTcSYQtqssZaYf8820727;     LpRsTcSYQtqssZaYf8820727 = LpRsTcSYQtqssZaYf50546558;     LpRsTcSYQtqssZaYf50546558 = LpRsTcSYQtqssZaYf17818104;     LpRsTcSYQtqssZaYf17818104 = LpRsTcSYQtqssZaYf22913272;     LpRsTcSYQtqssZaYf22913272 = LpRsTcSYQtqssZaYf49190153;     LpRsTcSYQtqssZaYf49190153 = LpRsTcSYQtqssZaYf22248780;     LpRsTcSYQtqssZaYf22248780 = LpRsTcSYQtqssZaYf27423958;     LpRsTcSYQtqssZaYf27423958 = LpRsTcSYQtqssZaYf62616132;     LpRsTcSYQtqssZaYf62616132 = LpRsTcSYQtqssZaYf77834262;     LpRsTcSYQtqssZaYf77834262 = LpRsTcSYQtqssZaYf40586127;     LpRsTcSYQtqssZaYf40586127 = LpRsTcSYQtqssZaYf73340389;     LpRsTcSYQtqssZaYf73340389 = LpRsTcSYQtqssZaYf6751832;     LpRsTcSYQtqssZaYf6751832 = LpRsTcSYQtqssZaYf96342181;     LpRsTcSYQtqssZaYf96342181 = LpRsTcSYQtqssZaYf21469607;     LpRsTcSYQtqssZaYf21469607 = LpRsTcSYQtqssZaYf23866632;     LpRsTcSYQtqssZaYf23866632 = LpRsTcSYQtqssZaYf82171258;     LpRsTcSYQtqssZaYf82171258 = LpRsTcSYQtqssZaYf92183758;     LpRsTcSYQtqssZaYf92183758 = LpRsTcSYQtqssZaYf83568470;     LpRsTcSYQtqssZaYf83568470 = LpRsTcSYQtqssZaYf99777367;     LpRsTcSYQtqssZaYf99777367 = LpRsTcSYQtqssZaYf90372407;     LpRsTcSYQtqssZaYf90372407 = LpRsTcSYQtqssZaYf21442552;     LpRsTcSYQtqssZaYf21442552 = LpRsTcSYQtqssZaYf4345234;     LpRsTcSYQtqssZaYf4345234 = LpRsTcSYQtqssZaYf43217041;     LpRsTcSYQtqssZaYf43217041 = LpRsTcSYQtqssZaYf1678384;     LpRsTcSYQtqssZaYf1678384 = LpRsTcSYQtqssZaYf41476519;     LpRsTcSYQtqssZaYf41476519 = LpRsTcSYQtqssZaYf93629628;     LpRsTcSYQtqssZaYf93629628 = LpRsTcSYQtqssZaYf92628439;     LpRsTcSYQtqssZaYf92628439 = LpRsTcSYQtqssZaYf5069679;     LpRsTcSYQtqssZaYf5069679 = LpRsTcSYQtqssZaYf75413203;     LpRsTcSYQtqssZaYf75413203 = LpRsTcSYQtqssZaYf2385807;     LpRsTcSYQtqssZaYf2385807 = LpRsTcSYQtqssZaYf49050977;     LpRsTcSYQtqssZaYf49050977 = LpRsTcSYQtqssZaYf82529371;     LpRsTcSYQtqssZaYf82529371 = LpRsTcSYQtqssZaYf89774181;     LpRsTcSYQtqssZaYf89774181 = LpRsTcSYQtqssZaYf18913141;     LpRsTcSYQtqssZaYf18913141 = LpRsTcSYQtqssZaYf95624825;     LpRsTcSYQtqssZaYf95624825 = LpRsTcSYQtqssZaYf13867792;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void QRcpdegvLeUXZhQG5701914() {     double MpcBuNduUMTiTCKTV49326760 = -871108797;    double MpcBuNduUMTiTCKTV71500422 = -450782635;    double MpcBuNduUMTiTCKTV22807431 = -905928955;    double MpcBuNduUMTiTCKTV55741517 = -300570455;    double MpcBuNduUMTiTCKTV74000303 = -324064786;    double MpcBuNduUMTiTCKTV82013667 = -985441295;    double MpcBuNduUMTiTCKTV30591615 = 22760411;    double MpcBuNduUMTiTCKTV4689974 = -894992113;    double MpcBuNduUMTiTCKTV52524488 = -469290243;    double MpcBuNduUMTiTCKTV39753485 = -7806050;    double MpcBuNduUMTiTCKTV6385778 = -773927411;    double MpcBuNduUMTiTCKTV27557980 = 67591632;    double MpcBuNduUMTiTCKTV90787972 = -33614437;    double MpcBuNduUMTiTCKTV26835243 = -633909773;    double MpcBuNduUMTiTCKTV82057606 = -442095438;    double MpcBuNduUMTiTCKTV57924315 = 28060343;    double MpcBuNduUMTiTCKTV43328353 = -907876907;    double MpcBuNduUMTiTCKTV16445293 = -526658252;    double MpcBuNduUMTiTCKTV33306153 = -22250816;    double MpcBuNduUMTiTCKTV23324397 = -981202294;    double MpcBuNduUMTiTCKTV18440338 = -608044259;    double MpcBuNduUMTiTCKTV7984728 = 93988393;    double MpcBuNduUMTiTCKTV47710415 = -460095259;    double MpcBuNduUMTiTCKTV19708104 = -615020327;    double MpcBuNduUMTiTCKTV36537333 = -412819432;    double MpcBuNduUMTiTCKTV49757440 = -362753061;    double MpcBuNduUMTiTCKTV53675615 = -560387073;    double MpcBuNduUMTiTCKTV20812559 = -210539966;    double MpcBuNduUMTiTCKTV39742913 = -744161792;    double MpcBuNduUMTiTCKTV48503104 = -27620527;    double MpcBuNduUMTiTCKTV81504292 = -553059281;    double MpcBuNduUMTiTCKTV97206334 = -586217262;    double MpcBuNduUMTiTCKTV88661284 = -295876534;    double MpcBuNduUMTiTCKTV64483649 = -780401212;    double MpcBuNduUMTiTCKTV16589561 = -183735401;    double MpcBuNduUMTiTCKTV26233781 = -740450438;    double MpcBuNduUMTiTCKTV1546051 = -710412820;    double MpcBuNduUMTiTCKTV11283864 = -583983021;    double MpcBuNduUMTiTCKTV84459530 = -334859298;    double MpcBuNduUMTiTCKTV19193093 = -848072661;    double MpcBuNduUMTiTCKTV22352660 = -300816994;    double MpcBuNduUMTiTCKTV21378212 = -295989755;    double MpcBuNduUMTiTCKTV87194750 = 3165500;    double MpcBuNduUMTiTCKTV20974192 = -42751142;    double MpcBuNduUMTiTCKTV4462011 = -398349767;    double MpcBuNduUMTiTCKTV50614078 = -940710124;    double MpcBuNduUMTiTCKTV76233174 = -883471597;    double MpcBuNduUMTiTCKTV18797595 = -353213764;    double MpcBuNduUMTiTCKTV19408330 = -895521217;    double MpcBuNduUMTiTCKTV17826920 = 71345107;    double MpcBuNduUMTiTCKTV33368009 = -953813455;    double MpcBuNduUMTiTCKTV24963981 = -852599084;    double MpcBuNduUMTiTCKTV23659232 = 37157836;    double MpcBuNduUMTiTCKTV49218079 = -328822721;    double MpcBuNduUMTiTCKTV87261575 = -343847502;    double MpcBuNduUMTiTCKTV41342032 = -865097190;    double MpcBuNduUMTiTCKTV23790008 = -990687376;    double MpcBuNduUMTiTCKTV3099327 = -190908628;    double MpcBuNduUMTiTCKTV19204185 = -887751024;    double MpcBuNduUMTiTCKTV24242863 = -961311725;    double MpcBuNduUMTiTCKTV28338052 = -325054222;    double MpcBuNduUMTiTCKTV9779057 = -766699623;    double MpcBuNduUMTiTCKTV64947060 = -50830321;    double MpcBuNduUMTiTCKTV4021385 = -341669717;    double MpcBuNduUMTiTCKTV58249193 = -454746770;    double MpcBuNduUMTiTCKTV9179444 = -87710149;    double MpcBuNduUMTiTCKTV38896696 = -636531834;    double MpcBuNduUMTiTCKTV26304323 = -253213226;    double MpcBuNduUMTiTCKTV10245682 = -350174372;    double MpcBuNduUMTiTCKTV55823825 = -701645000;    double MpcBuNduUMTiTCKTV56378264 = -261526837;    double MpcBuNduUMTiTCKTV32044490 = -223893886;    double MpcBuNduUMTiTCKTV31985762 = -91798955;    double MpcBuNduUMTiTCKTV14113060 = -174178155;    double MpcBuNduUMTiTCKTV971738 = -580385300;    double MpcBuNduUMTiTCKTV97062125 = -212054504;    double MpcBuNduUMTiTCKTV20789978 = -909177108;    double MpcBuNduUMTiTCKTV26736223 = -317344118;    double MpcBuNduUMTiTCKTV15246094 = -116670560;    double MpcBuNduUMTiTCKTV85923254 = -472109308;    double MpcBuNduUMTiTCKTV73524266 = -479281464;    double MpcBuNduUMTiTCKTV34878021 = -107173309;    double MpcBuNduUMTiTCKTV1404230 = -315018749;    double MpcBuNduUMTiTCKTV21915993 = -715506899;    double MpcBuNduUMTiTCKTV15135095 = -73807073;    double MpcBuNduUMTiTCKTV56540311 = -700460197;    double MpcBuNduUMTiTCKTV73547103 = -523375099;    double MpcBuNduUMTiTCKTV39443205 = -967053813;    double MpcBuNduUMTiTCKTV77222073 = -336553710;    double MpcBuNduUMTiTCKTV75247529 = -318638212;    double MpcBuNduUMTiTCKTV2443774 = -749763063;    double MpcBuNduUMTiTCKTV98446724 = -419504192;    double MpcBuNduUMTiTCKTV92079678 = -696231998;    double MpcBuNduUMTiTCKTV60216668 = -373547573;    double MpcBuNduUMTiTCKTV90855041 = -423018440;    double MpcBuNduUMTiTCKTV12573604 = -534117371;    double MpcBuNduUMTiTCKTV56431151 = -145159435;    double MpcBuNduUMTiTCKTV83173365 = -655164784;    double MpcBuNduUMTiTCKTV62724999 = -588004373;    double MpcBuNduUMTiTCKTV95282567 = -871108797;     MpcBuNduUMTiTCKTV49326760 = MpcBuNduUMTiTCKTV71500422;     MpcBuNduUMTiTCKTV71500422 = MpcBuNduUMTiTCKTV22807431;     MpcBuNduUMTiTCKTV22807431 = MpcBuNduUMTiTCKTV55741517;     MpcBuNduUMTiTCKTV55741517 = MpcBuNduUMTiTCKTV74000303;     MpcBuNduUMTiTCKTV74000303 = MpcBuNduUMTiTCKTV82013667;     MpcBuNduUMTiTCKTV82013667 = MpcBuNduUMTiTCKTV30591615;     MpcBuNduUMTiTCKTV30591615 = MpcBuNduUMTiTCKTV4689974;     MpcBuNduUMTiTCKTV4689974 = MpcBuNduUMTiTCKTV52524488;     MpcBuNduUMTiTCKTV52524488 = MpcBuNduUMTiTCKTV39753485;     MpcBuNduUMTiTCKTV39753485 = MpcBuNduUMTiTCKTV6385778;     MpcBuNduUMTiTCKTV6385778 = MpcBuNduUMTiTCKTV27557980;     MpcBuNduUMTiTCKTV27557980 = MpcBuNduUMTiTCKTV90787972;     MpcBuNduUMTiTCKTV90787972 = MpcBuNduUMTiTCKTV26835243;     MpcBuNduUMTiTCKTV26835243 = MpcBuNduUMTiTCKTV82057606;     MpcBuNduUMTiTCKTV82057606 = MpcBuNduUMTiTCKTV57924315;     MpcBuNduUMTiTCKTV57924315 = MpcBuNduUMTiTCKTV43328353;     MpcBuNduUMTiTCKTV43328353 = MpcBuNduUMTiTCKTV16445293;     MpcBuNduUMTiTCKTV16445293 = MpcBuNduUMTiTCKTV33306153;     MpcBuNduUMTiTCKTV33306153 = MpcBuNduUMTiTCKTV23324397;     MpcBuNduUMTiTCKTV23324397 = MpcBuNduUMTiTCKTV18440338;     MpcBuNduUMTiTCKTV18440338 = MpcBuNduUMTiTCKTV7984728;     MpcBuNduUMTiTCKTV7984728 = MpcBuNduUMTiTCKTV47710415;     MpcBuNduUMTiTCKTV47710415 = MpcBuNduUMTiTCKTV19708104;     MpcBuNduUMTiTCKTV19708104 = MpcBuNduUMTiTCKTV36537333;     MpcBuNduUMTiTCKTV36537333 = MpcBuNduUMTiTCKTV49757440;     MpcBuNduUMTiTCKTV49757440 = MpcBuNduUMTiTCKTV53675615;     MpcBuNduUMTiTCKTV53675615 = MpcBuNduUMTiTCKTV20812559;     MpcBuNduUMTiTCKTV20812559 = MpcBuNduUMTiTCKTV39742913;     MpcBuNduUMTiTCKTV39742913 = MpcBuNduUMTiTCKTV48503104;     MpcBuNduUMTiTCKTV48503104 = MpcBuNduUMTiTCKTV81504292;     MpcBuNduUMTiTCKTV81504292 = MpcBuNduUMTiTCKTV97206334;     MpcBuNduUMTiTCKTV97206334 = MpcBuNduUMTiTCKTV88661284;     MpcBuNduUMTiTCKTV88661284 = MpcBuNduUMTiTCKTV64483649;     MpcBuNduUMTiTCKTV64483649 = MpcBuNduUMTiTCKTV16589561;     MpcBuNduUMTiTCKTV16589561 = MpcBuNduUMTiTCKTV26233781;     MpcBuNduUMTiTCKTV26233781 = MpcBuNduUMTiTCKTV1546051;     MpcBuNduUMTiTCKTV1546051 = MpcBuNduUMTiTCKTV11283864;     MpcBuNduUMTiTCKTV11283864 = MpcBuNduUMTiTCKTV84459530;     MpcBuNduUMTiTCKTV84459530 = MpcBuNduUMTiTCKTV19193093;     MpcBuNduUMTiTCKTV19193093 = MpcBuNduUMTiTCKTV22352660;     MpcBuNduUMTiTCKTV22352660 = MpcBuNduUMTiTCKTV21378212;     MpcBuNduUMTiTCKTV21378212 = MpcBuNduUMTiTCKTV87194750;     MpcBuNduUMTiTCKTV87194750 = MpcBuNduUMTiTCKTV20974192;     MpcBuNduUMTiTCKTV20974192 = MpcBuNduUMTiTCKTV4462011;     MpcBuNduUMTiTCKTV4462011 = MpcBuNduUMTiTCKTV50614078;     MpcBuNduUMTiTCKTV50614078 = MpcBuNduUMTiTCKTV76233174;     MpcBuNduUMTiTCKTV76233174 = MpcBuNduUMTiTCKTV18797595;     MpcBuNduUMTiTCKTV18797595 = MpcBuNduUMTiTCKTV19408330;     MpcBuNduUMTiTCKTV19408330 = MpcBuNduUMTiTCKTV17826920;     MpcBuNduUMTiTCKTV17826920 = MpcBuNduUMTiTCKTV33368009;     MpcBuNduUMTiTCKTV33368009 = MpcBuNduUMTiTCKTV24963981;     MpcBuNduUMTiTCKTV24963981 = MpcBuNduUMTiTCKTV23659232;     MpcBuNduUMTiTCKTV23659232 = MpcBuNduUMTiTCKTV49218079;     MpcBuNduUMTiTCKTV49218079 = MpcBuNduUMTiTCKTV87261575;     MpcBuNduUMTiTCKTV87261575 = MpcBuNduUMTiTCKTV41342032;     MpcBuNduUMTiTCKTV41342032 = MpcBuNduUMTiTCKTV23790008;     MpcBuNduUMTiTCKTV23790008 = MpcBuNduUMTiTCKTV3099327;     MpcBuNduUMTiTCKTV3099327 = MpcBuNduUMTiTCKTV19204185;     MpcBuNduUMTiTCKTV19204185 = MpcBuNduUMTiTCKTV24242863;     MpcBuNduUMTiTCKTV24242863 = MpcBuNduUMTiTCKTV28338052;     MpcBuNduUMTiTCKTV28338052 = MpcBuNduUMTiTCKTV9779057;     MpcBuNduUMTiTCKTV9779057 = MpcBuNduUMTiTCKTV64947060;     MpcBuNduUMTiTCKTV64947060 = MpcBuNduUMTiTCKTV4021385;     MpcBuNduUMTiTCKTV4021385 = MpcBuNduUMTiTCKTV58249193;     MpcBuNduUMTiTCKTV58249193 = MpcBuNduUMTiTCKTV9179444;     MpcBuNduUMTiTCKTV9179444 = MpcBuNduUMTiTCKTV38896696;     MpcBuNduUMTiTCKTV38896696 = MpcBuNduUMTiTCKTV26304323;     MpcBuNduUMTiTCKTV26304323 = MpcBuNduUMTiTCKTV10245682;     MpcBuNduUMTiTCKTV10245682 = MpcBuNduUMTiTCKTV55823825;     MpcBuNduUMTiTCKTV55823825 = MpcBuNduUMTiTCKTV56378264;     MpcBuNduUMTiTCKTV56378264 = MpcBuNduUMTiTCKTV32044490;     MpcBuNduUMTiTCKTV32044490 = MpcBuNduUMTiTCKTV31985762;     MpcBuNduUMTiTCKTV31985762 = MpcBuNduUMTiTCKTV14113060;     MpcBuNduUMTiTCKTV14113060 = MpcBuNduUMTiTCKTV971738;     MpcBuNduUMTiTCKTV971738 = MpcBuNduUMTiTCKTV97062125;     MpcBuNduUMTiTCKTV97062125 = MpcBuNduUMTiTCKTV20789978;     MpcBuNduUMTiTCKTV20789978 = MpcBuNduUMTiTCKTV26736223;     MpcBuNduUMTiTCKTV26736223 = MpcBuNduUMTiTCKTV15246094;     MpcBuNduUMTiTCKTV15246094 = MpcBuNduUMTiTCKTV85923254;     MpcBuNduUMTiTCKTV85923254 = MpcBuNduUMTiTCKTV73524266;     MpcBuNduUMTiTCKTV73524266 = MpcBuNduUMTiTCKTV34878021;     MpcBuNduUMTiTCKTV34878021 = MpcBuNduUMTiTCKTV1404230;     MpcBuNduUMTiTCKTV1404230 = MpcBuNduUMTiTCKTV21915993;     MpcBuNduUMTiTCKTV21915993 = MpcBuNduUMTiTCKTV15135095;     MpcBuNduUMTiTCKTV15135095 = MpcBuNduUMTiTCKTV56540311;     MpcBuNduUMTiTCKTV56540311 = MpcBuNduUMTiTCKTV73547103;     MpcBuNduUMTiTCKTV73547103 = MpcBuNduUMTiTCKTV39443205;     MpcBuNduUMTiTCKTV39443205 = MpcBuNduUMTiTCKTV77222073;     MpcBuNduUMTiTCKTV77222073 = MpcBuNduUMTiTCKTV75247529;     MpcBuNduUMTiTCKTV75247529 = MpcBuNduUMTiTCKTV2443774;     MpcBuNduUMTiTCKTV2443774 = MpcBuNduUMTiTCKTV98446724;     MpcBuNduUMTiTCKTV98446724 = MpcBuNduUMTiTCKTV92079678;     MpcBuNduUMTiTCKTV92079678 = MpcBuNduUMTiTCKTV60216668;     MpcBuNduUMTiTCKTV60216668 = MpcBuNduUMTiTCKTV90855041;     MpcBuNduUMTiTCKTV90855041 = MpcBuNduUMTiTCKTV12573604;     MpcBuNduUMTiTCKTV12573604 = MpcBuNduUMTiTCKTV56431151;     MpcBuNduUMTiTCKTV56431151 = MpcBuNduUMTiTCKTV83173365;     MpcBuNduUMTiTCKTV83173365 = MpcBuNduUMTiTCKTV62724999;     MpcBuNduUMTiTCKTV62724999 = MpcBuNduUMTiTCKTV95282567;     MpcBuNduUMTiTCKTV95282567 = MpcBuNduUMTiTCKTV49326760;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void jfzDFHmTtlrSwopM20750982() {     double gUJXQbXvsfkCZfbKp55443866 = -983010685;    double gUJXQbXvsfkCZfbKp14873702 = -160382416;    double gUJXQbXvsfkCZfbKp38849615 = 43238124;    double gUJXQbXvsfkCZfbKp82816298 = -491319086;    double gUJXQbXvsfkCZfbKp42895792 = -844714657;    double gUJXQbXvsfkCZfbKp73430869 = -149937530;    double gUJXQbXvsfkCZfbKp91080581 = -116204566;    double gUJXQbXvsfkCZfbKp54449864 = -81866026;    double gUJXQbXvsfkCZfbKp17166984 = -237322909;    double gUJXQbXvsfkCZfbKp80983144 = -336781528;    double gUJXQbXvsfkCZfbKp66496068 = -651364998;    double gUJXQbXvsfkCZfbKp59381022 = -376814570;    double gUJXQbXvsfkCZfbKp76997090 = -473505526;    double gUJXQbXvsfkCZfbKp9978920 = -66148341;    double gUJXQbXvsfkCZfbKp77800033 = -32557604;    double gUJXQbXvsfkCZfbKp99429708 = 55060800;    double gUJXQbXvsfkCZfbKp37164357 = -424422020;    double gUJXQbXvsfkCZfbKp13498446 = -192180895;    double gUJXQbXvsfkCZfbKp95135994 = -428019913;    double gUJXQbXvsfkCZfbKp3874562 = -238105214;    double gUJXQbXvsfkCZfbKp6993531 = -463531714;    double gUJXQbXvsfkCZfbKp94505751 = -913941324;    double gUJXQbXvsfkCZfbKp83301206 = -206612746;    double gUJXQbXvsfkCZfbKp3700478 = -837325315;    double gUJXQbXvsfkCZfbKp80236982 = -786015465;    double gUJXQbXvsfkCZfbKp74479115 = -241408384;    double gUJXQbXvsfkCZfbKp52135393 = -461049276;    double gUJXQbXvsfkCZfbKp33919684 = -276468919;    double gUJXQbXvsfkCZfbKp5875169 = -966544497;    double gUJXQbXvsfkCZfbKp17118741 = -945091395;    double gUJXQbXvsfkCZfbKp73746841 = -466380219;    double gUJXQbXvsfkCZfbKp94816956 = -295664078;    double gUJXQbXvsfkCZfbKp98372232 = -188489905;    double gUJXQbXvsfkCZfbKp59314213 = -531942977;    double gUJXQbXvsfkCZfbKp78064213 = -822468507;    double gUJXQbXvsfkCZfbKp94959694 = -954476546;    double gUJXQbXvsfkCZfbKp82807985 = -664048689;    double gUJXQbXvsfkCZfbKp45374076 = -355489031;    double gUJXQbXvsfkCZfbKp58069784 = -328782794;    double gUJXQbXvsfkCZfbKp6695031 = -221511884;    double gUJXQbXvsfkCZfbKp28021884 = -366090889;    double gUJXQbXvsfkCZfbKp53528669 = -566132034;    double gUJXQbXvsfkCZfbKp65261728 = -77639084;    double gUJXQbXvsfkCZfbKp8381933 = -62094080;    double gUJXQbXvsfkCZfbKp16414476 = -60247922;    double gUJXQbXvsfkCZfbKp62553273 = -925926241;    double gUJXQbXvsfkCZfbKp94011748 = 97752865;    double gUJXQbXvsfkCZfbKp26754536 = -363109297;    double gUJXQbXvsfkCZfbKp29084059 = -981258205;    double gUJXQbXvsfkCZfbKp30738573 = -15665701;    double gUJXQbXvsfkCZfbKp69068687 = -796107126;    double gUJXQbXvsfkCZfbKp35773701 = -653325405;    double gUJXQbXvsfkCZfbKp71907689 = -854631752;    double gUJXQbXvsfkCZfbKp45186959 = -649292867;    double gUJXQbXvsfkCZfbKp13757451 = 55620716;    double gUJXQbXvsfkCZfbKp60938115 = 30930638;    double gUJXQbXvsfkCZfbKp31572495 = -953769670;    double gUJXQbXvsfkCZfbKp35149137 = -119436561;    double gUJXQbXvsfkCZfbKp2579316 = -705303621;    double gUJXQbXvsfkCZfbKp68416676 = -503306273;    double gUJXQbXvsfkCZfbKp21295476 = -688888254;    double gUJXQbXvsfkCZfbKp57160897 = -839735647;    double gUJXQbXvsfkCZfbKp48574696 = -115321530;    double gUJXQbXvsfkCZfbKp48244 = -292231515;    double gUJXQbXvsfkCZfbKp7236303 = -870401309;    double gUJXQbXvsfkCZfbKp71679111 = -255700920;    double gUJXQbXvsfkCZfbKp61008789 = -88324666;    double gUJXQbXvsfkCZfbKp17682877 = -941562550;    double gUJXQbXvsfkCZfbKp31914706 = -243679834;    double gUJXQbXvsfkCZfbKp82840338 = -78081059;    double gUJXQbXvsfkCZfbKp16621724 = -280890511;    double gUJXQbXvsfkCZfbKp91790280 = 31067010;    double gUJXQbXvsfkCZfbKp55428661 = -863398101;    double gUJXQbXvsfkCZfbKp88440964 = -106508030;    double gUJXQbXvsfkCZfbKp75852678 = -872014326;    double gUJXQbXvsfkCZfbKp53464862 = -897399681;    double gUJXQbXvsfkCZfbKp29244024 = -736302240;    double gUJXQbXvsfkCZfbKp74919274 = -44518667;    double gUJXQbXvsfkCZfbKp87286002 = -677077393;    double gUJXQbXvsfkCZfbKp17683710 = -860089225;    double gUJXQbXvsfkCZfbKp80467367 = -239161250;    double gUJXQbXvsfkCZfbKp25380858 = 2060020;    double gUJXQbXvsfkCZfbKp4835626 = -295210714;    double gUJXQbXvsfkCZfbKp75136595 = -850878796;    double gUJXQbXvsfkCZfbKp48050053 = -48984269;    double gUJXQbXvsfkCZfbKp37973141 = -813054815;    double gUJXQbXvsfkCZfbKp22909268 = -441032327;    double gUJXQbXvsfkCZfbKp53185273 = -539197038;    double gUJXQbXvsfkCZfbKp45556763 = -487563693;    double gUJXQbXvsfkCZfbKp17126099 = -753399145;    double gUJXQbXvsfkCZfbKp63387199 = 99293124;    double gUJXQbXvsfkCZfbKp47658849 = -444612128;    double gUJXQbXvsfkCZfbKp42794760 = -650185410;    double gUJXQbXvsfkCZfbKp89653108 = -825476521;    double gUJXQbXvsfkCZfbKp85399554 = -532623630;    double gUJXQbXvsfkCZfbKp70860987 = -526355242;    double gUJXQbXvsfkCZfbKp4953973 = -350810504;    double gUJXQbXvsfkCZfbKp65213485 = -785407570;    double gUJXQbXvsfkCZfbKp1145630 = -191692771;    double gUJXQbXvsfkCZfbKp44735364 = -983010685;     gUJXQbXvsfkCZfbKp55443866 = gUJXQbXvsfkCZfbKp14873702;     gUJXQbXvsfkCZfbKp14873702 = gUJXQbXvsfkCZfbKp38849615;     gUJXQbXvsfkCZfbKp38849615 = gUJXQbXvsfkCZfbKp82816298;     gUJXQbXvsfkCZfbKp82816298 = gUJXQbXvsfkCZfbKp42895792;     gUJXQbXvsfkCZfbKp42895792 = gUJXQbXvsfkCZfbKp73430869;     gUJXQbXvsfkCZfbKp73430869 = gUJXQbXvsfkCZfbKp91080581;     gUJXQbXvsfkCZfbKp91080581 = gUJXQbXvsfkCZfbKp54449864;     gUJXQbXvsfkCZfbKp54449864 = gUJXQbXvsfkCZfbKp17166984;     gUJXQbXvsfkCZfbKp17166984 = gUJXQbXvsfkCZfbKp80983144;     gUJXQbXvsfkCZfbKp80983144 = gUJXQbXvsfkCZfbKp66496068;     gUJXQbXvsfkCZfbKp66496068 = gUJXQbXvsfkCZfbKp59381022;     gUJXQbXvsfkCZfbKp59381022 = gUJXQbXvsfkCZfbKp76997090;     gUJXQbXvsfkCZfbKp76997090 = gUJXQbXvsfkCZfbKp9978920;     gUJXQbXvsfkCZfbKp9978920 = gUJXQbXvsfkCZfbKp77800033;     gUJXQbXvsfkCZfbKp77800033 = gUJXQbXvsfkCZfbKp99429708;     gUJXQbXvsfkCZfbKp99429708 = gUJXQbXvsfkCZfbKp37164357;     gUJXQbXvsfkCZfbKp37164357 = gUJXQbXvsfkCZfbKp13498446;     gUJXQbXvsfkCZfbKp13498446 = gUJXQbXvsfkCZfbKp95135994;     gUJXQbXvsfkCZfbKp95135994 = gUJXQbXvsfkCZfbKp3874562;     gUJXQbXvsfkCZfbKp3874562 = gUJXQbXvsfkCZfbKp6993531;     gUJXQbXvsfkCZfbKp6993531 = gUJXQbXvsfkCZfbKp94505751;     gUJXQbXvsfkCZfbKp94505751 = gUJXQbXvsfkCZfbKp83301206;     gUJXQbXvsfkCZfbKp83301206 = gUJXQbXvsfkCZfbKp3700478;     gUJXQbXvsfkCZfbKp3700478 = gUJXQbXvsfkCZfbKp80236982;     gUJXQbXvsfkCZfbKp80236982 = gUJXQbXvsfkCZfbKp74479115;     gUJXQbXvsfkCZfbKp74479115 = gUJXQbXvsfkCZfbKp52135393;     gUJXQbXvsfkCZfbKp52135393 = gUJXQbXvsfkCZfbKp33919684;     gUJXQbXvsfkCZfbKp33919684 = gUJXQbXvsfkCZfbKp5875169;     gUJXQbXvsfkCZfbKp5875169 = gUJXQbXvsfkCZfbKp17118741;     gUJXQbXvsfkCZfbKp17118741 = gUJXQbXvsfkCZfbKp73746841;     gUJXQbXvsfkCZfbKp73746841 = gUJXQbXvsfkCZfbKp94816956;     gUJXQbXvsfkCZfbKp94816956 = gUJXQbXvsfkCZfbKp98372232;     gUJXQbXvsfkCZfbKp98372232 = gUJXQbXvsfkCZfbKp59314213;     gUJXQbXvsfkCZfbKp59314213 = gUJXQbXvsfkCZfbKp78064213;     gUJXQbXvsfkCZfbKp78064213 = gUJXQbXvsfkCZfbKp94959694;     gUJXQbXvsfkCZfbKp94959694 = gUJXQbXvsfkCZfbKp82807985;     gUJXQbXvsfkCZfbKp82807985 = gUJXQbXvsfkCZfbKp45374076;     gUJXQbXvsfkCZfbKp45374076 = gUJXQbXvsfkCZfbKp58069784;     gUJXQbXvsfkCZfbKp58069784 = gUJXQbXvsfkCZfbKp6695031;     gUJXQbXvsfkCZfbKp6695031 = gUJXQbXvsfkCZfbKp28021884;     gUJXQbXvsfkCZfbKp28021884 = gUJXQbXvsfkCZfbKp53528669;     gUJXQbXvsfkCZfbKp53528669 = gUJXQbXvsfkCZfbKp65261728;     gUJXQbXvsfkCZfbKp65261728 = gUJXQbXvsfkCZfbKp8381933;     gUJXQbXvsfkCZfbKp8381933 = gUJXQbXvsfkCZfbKp16414476;     gUJXQbXvsfkCZfbKp16414476 = gUJXQbXvsfkCZfbKp62553273;     gUJXQbXvsfkCZfbKp62553273 = gUJXQbXvsfkCZfbKp94011748;     gUJXQbXvsfkCZfbKp94011748 = gUJXQbXvsfkCZfbKp26754536;     gUJXQbXvsfkCZfbKp26754536 = gUJXQbXvsfkCZfbKp29084059;     gUJXQbXvsfkCZfbKp29084059 = gUJXQbXvsfkCZfbKp30738573;     gUJXQbXvsfkCZfbKp30738573 = gUJXQbXvsfkCZfbKp69068687;     gUJXQbXvsfkCZfbKp69068687 = gUJXQbXvsfkCZfbKp35773701;     gUJXQbXvsfkCZfbKp35773701 = gUJXQbXvsfkCZfbKp71907689;     gUJXQbXvsfkCZfbKp71907689 = gUJXQbXvsfkCZfbKp45186959;     gUJXQbXvsfkCZfbKp45186959 = gUJXQbXvsfkCZfbKp13757451;     gUJXQbXvsfkCZfbKp13757451 = gUJXQbXvsfkCZfbKp60938115;     gUJXQbXvsfkCZfbKp60938115 = gUJXQbXvsfkCZfbKp31572495;     gUJXQbXvsfkCZfbKp31572495 = gUJXQbXvsfkCZfbKp35149137;     gUJXQbXvsfkCZfbKp35149137 = gUJXQbXvsfkCZfbKp2579316;     gUJXQbXvsfkCZfbKp2579316 = gUJXQbXvsfkCZfbKp68416676;     gUJXQbXvsfkCZfbKp68416676 = gUJXQbXvsfkCZfbKp21295476;     gUJXQbXvsfkCZfbKp21295476 = gUJXQbXvsfkCZfbKp57160897;     gUJXQbXvsfkCZfbKp57160897 = gUJXQbXvsfkCZfbKp48574696;     gUJXQbXvsfkCZfbKp48574696 = gUJXQbXvsfkCZfbKp48244;     gUJXQbXvsfkCZfbKp48244 = gUJXQbXvsfkCZfbKp7236303;     gUJXQbXvsfkCZfbKp7236303 = gUJXQbXvsfkCZfbKp71679111;     gUJXQbXvsfkCZfbKp71679111 = gUJXQbXvsfkCZfbKp61008789;     gUJXQbXvsfkCZfbKp61008789 = gUJXQbXvsfkCZfbKp17682877;     gUJXQbXvsfkCZfbKp17682877 = gUJXQbXvsfkCZfbKp31914706;     gUJXQbXvsfkCZfbKp31914706 = gUJXQbXvsfkCZfbKp82840338;     gUJXQbXvsfkCZfbKp82840338 = gUJXQbXvsfkCZfbKp16621724;     gUJXQbXvsfkCZfbKp16621724 = gUJXQbXvsfkCZfbKp91790280;     gUJXQbXvsfkCZfbKp91790280 = gUJXQbXvsfkCZfbKp55428661;     gUJXQbXvsfkCZfbKp55428661 = gUJXQbXvsfkCZfbKp88440964;     gUJXQbXvsfkCZfbKp88440964 = gUJXQbXvsfkCZfbKp75852678;     gUJXQbXvsfkCZfbKp75852678 = gUJXQbXvsfkCZfbKp53464862;     gUJXQbXvsfkCZfbKp53464862 = gUJXQbXvsfkCZfbKp29244024;     gUJXQbXvsfkCZfbKp29244024 = gUJXQbXvsfkCZfbKp74919274;     gUJXQbXvsfkCZfbKp74919274 = gUJXQbXvsfkCZfbKp87286002;     gUJXQbXvsfkCZfbKp87286002 = gUJXQbXvsfkCZfbKp17683710;     gUJXQbXvsfkCZfbKp17683710 = gUJXQbXvsfkCZfbKp80467367;     gUJXQbXvsfkCZfbKp80467367 = gUJXQbXvsfkCZfbKp25380858;     gUJXQbXvsfkCZfbKp25380858 = gUJXQbXvsfkCZfbKp4835626;     gUJXQbXvsfkCZfbKp4835626 = gUJXQbXvsfkCZfbKp75136595;     gUJXQbXvsfkCZfbKp75136595 = gUJXQbXvsfkCZfbKp48050053;     gUJXQbXvsfkCZfbKp48050053 = gUJXQbXvsfkCZfbKp37973141;     gUJXQbXvsfkCZfbKp37973141 = gUJXQbXvsfkCZfbKp22909268;     gUJXQbXvsfkCZfbKp22909268 = gUJXQbXvsfkCZfbKp53185273;     gUJXQbXvsfkCZfbKp53185273 = gUJXQbXvsfkCZfbKp45556763;     gUJXQbXvsfkCZfbKp45556763 = gUJXQbXvsfkCZfbKp17126099;     gUJXQbXvsfkCZfbKp17126099 = gUJXQbXvsfkCZfbKp63387199;     gUJXQbXvsfkCZfbKp63387199 = gUJXQbXvsfkCZfbKp47658849;     gUJXQbXvsfkCZfbKp47658849 = gUJXQbXvsfkCZfbKp42794760;     gUJXQbXvsfkCZfbKp42794760 = gUJXQbXvsfkCZfbKp89653108;     gUJXQbXvsfkCZfbKp89653108 = gUJXQbXvsfkCZfbKp85399554;     gUJXQbXvsfkCZfbKp85399554 = gUJXQbXvsfkCZfbKp70860987;     gUJXQbXvsfkCZfbKp70860987 = gUJXQbXvsfkCZfbKp4953973;     gUJXQbXvsfkCZfbKp4953973 = gUJXQbXvsfkCZfbKp65213485;     gUJXQbXvsfkCZfbKp65213485 = gUJXQbXvsfkCZfbKp1145630;     gUJXQbXvsfkCZfbKp1145630 = gUJXQbXvsfkCZfbKp44735364;     gUJXQbXvsfkCZfbKp44735364 = gUJXQbXvsfkCZfbKp55443866;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void mEvKnaxtmHXPNwVp57395956() {     double mYSWCAhBbjfpejdgC77830984 = -617414648;    double mYSWCAhBbjfpejdgC99221202 = -108925470;    double mYSWCAhBbjfpejdgC67618893 = -196415720;    double mYSWCAhBbjfpejdgC97822986 = -497726617;    double mYSWCAhBbjfpejdgC41070046 = -335771659;    double mYSWCAhBbjfpejdgC88313986 = -10545908;    double mYSWCAhBbjfpejdgC83236320 = 61808128;    double mYSWCAhBbjfpejdgC30083723 = -552462670;    double mYSWCAhBbjfpejdgC14330115 = -121518948;    double mYSWCAhBbjfpejdgC72746729 = -79522861;    double mYSWCAhBbjfpejdgC4037928 = -647989398;    double mYSWCAhBbjfpejdgC15283972 = -627559863;    double mYSWCAhBbjfpejdgC44005271 = 7777155;    double mYSWCAhBbjfpejdgC52593671 = -593857098;    double mYSWCAhBbjfpejdgC14535311 = -760035317;    double mYSWCAhBbjfpejdgC43685621 = -326606742;    double mYSWCAhBbjfpejdgC86987708 = -701701840;    double mYSWCAhBbjfpejdgC19474862 = -643923586;    double mYSWCAhBbjfpejdgC36417626 = -563122038;    double mYSWCAhBbjfpejdgC82390845 = -490739667;    double mYSWCAhBbjfpejdgC7876638 = -455019023;    double mYSWCAhBbjfpejdgC58320970 = -318707637;    double mYSWCAhBbjfpejdgC86594529 = -382693338;    double mYSWCAhBbjfpejdgC30200265 = -301318172;    double mYSWCAhBbjfpejdgC15777230 = -407574589;    double mYSWCAhBbjfpejdgC62782110 = -685302608;    double mYSWCAhBbjfpejdgC99224813 = -218635769;    double mYSWCAhBbjfpejdgC39831134 = -64923373;    double mYSWCAhBbjfpejdgC12767729 = -210801616;    double mYSWCAhBbjfpejdgC26275920 = -865387261;    double mYSWCAhBbjfpejdgC18037647 = -547696371;    double mYSWCAhBbjfpejdgC52358147 = -947735433;    double mYSWCAhBbjfpejdgC79410903 = -717833399;    double mYSWCAhBbjfpejdgC18702901 = -856069827;    double mYSWCAhBbjfpejdgC54293578 = -886224929;    double mYSWCAhBbjfpejdgC36652564 = -169871922;    double mYSWCAhBbjfpejdgC76247698 = -396996641;    double mYSWCAhBbjfpejdgC99000176 = -223114503;    double mYSWCAhBbjfpejdgC6377916 = -729828826;    double mYSWCAhBbjfpejdgC26880745 = -357735913;    double mYSWCAhBbjfpejdgC56779254 = -869268555;    double mYSWCAhBbjfpejdgC42037060 = -324040725;    double mYSWCAhBbjfpejdgC70811920 = 34830332;    double mYSWCAhBbjfpejdgC7826052 = -59457729;    double mYSWCAhBbjfpejdgC84397742 = -969391399;    double mYSWCAhBbjfpejdgC70653554 = -8155336;    double mYSWCAhBbjfpejdgC6309146 = -480657388;    double mYSWCAhBbjfpejdgC71353210 = -119644288;    double mYSWCAhBbjfpejdgC23543964 = -930974904;    double mYSWCAhBbjfpejdgC27822319 = -157685960;    double mYSWCAhBbjfpejdgC77937903 = -51183403;    double mYSWCAhBbjfpejdgC9624958 = 15855313;    double mYSWCAhBbjfpejdgC50222092 = -526542982;    double mYSWCAhBbjfpejdgC5473505 = -924973675;    double mYSWCAhBbjfpejdgC49519318 = -85083399;    double mYSWCAhBbjfpejdgC19510014 = -198707012;    double mYSWCAhBbjfpejdgC12626673 = -726232133;    double mYSWCAhBbjfpejdgC37418628 = -895097548;    double mYSWCAhBbjfpejdgC82045757 = 9847972;    double mYSWCAhBbjfpejdgC78287935 = -650469051;    double mYSWCAhBbjfpejdgC89089172 = -791910139;    double mYSWCAhBbjfpejdgC43405186 = -873268500;    double mYSWCAhBbjfpejdgC17315994 = -241661055;    double mYSWCAhBbjfpejdgC88054194 = -256131687;    double mYSWCAhBbjfpejdgC54709082 = -531826490;    double mYSWCAhBbjfpejdgC51679780 = -700253966;    double mYSWCAhBbjfpejdgC35873068 = -909726464;    double mYSWCAhBbjfpejdgC25302371 = -136153019;    double mYSWCAhBbjfpejdgC98300093 = -707632169;    double mYSWCAhBbjfpejdgC77882746 = -490163395;    double mYSWCAhBbjfpejdgC67437923 = -929610102;    double mYSWCAhBbjfpejdgC87987531 = -378587338;    double mYSWCAhBbjfpejdgC13096947 = -914094760;    double mYSWCAhBbjfpejdgC9536881 = -105386125;    double mYSWCAhBbjfpejdgC25611591 = -621471113;    double mYSWCAhBbjfpejdgC65839578 = -30978299;    double mYSWCAhBbjfpejdgC87509049 = -253537970;    double mYSWCAhBbjfpejdgC78768478 = -223235610;    double mYSWCAhBbjfpejdgC45802523 = -331926773;    double mYSWCAhBbjfpejdgC45123675 = -299419254;    double mYSWCAhBbjfpejdgC56472964 = -104645221;    double mYSWCAhBbjfpejdgC27871604 = 1008518;    double mYSWCAhBbjfpejdgC16287171 = -133948470;    double mYSWCAhBbjfpejdgC84945410 = 46884344;    double mYSWCAhBbjfpejdgC48338016 = -714203859;    double mYSWCAhBbjfpejdgC8412689 = -463551685;    double mYSWCAhBbjfpejdgC2136056 = -321192451;    double mYSWCAhBbjfpejdgC73937398 = -792859725;    double mYSWCAhBbjfpejdgC69183583 = -670986428;    double mYSWCAhBbjfpejdgC34783564 = -587517918;    double mYSWCAhBbjfpejdgC24025891 = -443639790;    double mYSWCAhBbjfpejdgC38829070 = -501899093;    double mYSWCAhBbjfpejdgC16954420 = -132962475;    double mYSWCAhBbjfpejdgC28089980 = 20640224;    double mYSWCAhBbjfpejdgC37791573 = -565825774;    double mYSWCAhBbjfpejdgC13374069 = -996000056;    double mYSWCAhBbjfpejdgC24721066 = 17620330;    double mYSWCAhBbjfpejdgC82757725 = -709037981;    double mYSWCAhBbjfpejdgC53116969 = -527631239;    double mYSWCAhBbjfpejdgC32717962 = -617414648;     mYSWCAhBbjfpejdgC77830984 = mYSWCAhBbjfpejdgC99221202;     mYSWCAhBbjfpejdgC99221202 = mYSWCAhBbjfpejdgC67618893;     mYSWCAhBbjfpejdgC67618893 = mYSWCAhBbjfpejdgC97822986;     mYSWCAhBbjfpejdgC97822986 = mYSWCAhBbjfpejdgC41070046;     mYSWCAhBbjfpejdgC41070046 = mYSWCAhBbjfpejdgC88313986;     mYSWCAhBbjfpejdgC88313986 = mYSWCAhBbjfpejdgC83236320;     mYSWCAhBbjfpejdgC83236320 = mYSWCAhBbjfpejdgC30083723;     mYSWCAhBbjfpejdgC30083723 = mYSWCAhBbjfpejdgC14330115;     mYSWCAhBbjfpejdgC14330115 = mYSWCAhBbjfpejdgC72746729;     mYSWCAhBbjfpejdgC72746729 = mYSWCAhBbjfpejdgC4037928;     mYSWCAhBbjfpejdgC4037928 = mYSWCAhBbjfpejdgC15283972;     mYSWCAhBbjfpejdgC15283972 = mYSWCAhBbjfpejdgC44005271;     mYSWCAhBbjfpejdgC44005271 = mYSWCAhBbjfpejdgC52593671;     mYSWCAhBbjfpejdgC52593671 = mYSWCAhBbjfpejdgC14535311;     mYSWCAhBbjfpejdgC14535311 = mYSWCAhBbjfpejdgC43685621;     mYSWCAhBbjfpejdgC43685621 = mYSWCAhBbjfpejdgC86987708;     mYSWCAhBbjfpejdgC86987708 = mYSWCAhBbjfpejdgC19474862;     mYSWCAhBbjfpejdgC19474862 = mYSWCAhBbjfpejdgC36417626;     mYSWCAhBbjfpejdgC36417626 = mYSWCAhBbjfpejdgC82390845;     mYSWCAhBbjfpejdgC82390845 = mYSWCAhBbjfpejdgC7876638;     mYSWCAhBbjfpejdgC7876638 = mYSWCAhBbjfpejdgC58320970;     mYSWCAhBbjfpejdgC58320970 = mYSWCAhBbjfpejdgC86594529;     mYSWCAhBbjfpejdgC86594529 = mYSWCAhBbjfpejdgC30200265;     mYSWCAhBbjfpejdgC30200265 = mYSWCAhBbjfpejdgC15777230;     mYSWCAhBbjfpejdgC15777230 = mYSWCAhBbjfpejdgC62782110;     mYSWCAhBbjfpejdgC62782110 = mYSWCAhBbjfpejdgC99224813;     mYSWCAhBbjfpejdgC99224813 = mYSWCAhBbjfpejdgC39831134;     mYSWCAhBbjfpejdgC39831134 = mYSWCAhBbjfpejdgC12767729;     mYSWCAhBbjfpejdgC12767729 = mYSWCAhBbjfpejdgC26275920;     mYSWCAhBbjfpejdgC26275920 = mYSWCAhBbjfpejdgC18037647;     mYSWCAhBbjfpejdgC18037647 = mYSWCAhBbjfpejdgC52358147;     mYSWCAhBbjfpejdgC52358147 = mYSWCAhBbjfpejdgC79410903;     mYSWCAhBbjfpejdgC79410903 = mYSWCAhBbjfpejdgC18702901;     mYSWCAhBbjfpejdgC18702901 = mYSWCAhBbjfpejdgC54293578;     mYSWCAhBbjfpejdgC54293578 = mYSWCAhBbjfpejdgC36652564;     mYSWCAhBbjfpejdgC36652564 = mYSWCAhBbjfpejdgC76247698;     mYSWCAhBbjfpejdgC76247698 = mYSWCAhBbjfpejdgC99000176;     mYSWCAhBbjfpejdgC99000176 = mYSWCAhBbjfpejdgC6377916;     mYSWCAhBbjfpejdgC6377916 = mYSWCAhBbjfpejdgC26880745;     mYSWCAhBbjfpejdgC26880745 = mYSWCAhBbjfpejdgC56779254;     mYSWCAhBbjfpejdgC56779254 = mYSWCAhBbjfpejdgC42037060;     mYSWCAhBbjfpejdgC42037060 = mYSWCAhBbjfpejdgC70811920;     mYSWCAhBbjfpejdgC70811920 = mYSWCAhBbjfpejdgC7826052;     mYSWCAhBbjfpejdgC7826052 = mYSWCAhBbjfpejdgC84397742;     mYSWCAhBbjfpejdgC84397742 = mYSWCAhBbjfpejdgC70653554;     mYSWCAhBbjfpejdgC70653554 = mYSWCAhBbjfpejdgC6309146;     mYSWCAhBbjfpejdgC6309146 = mYSWCAhBbjfpejdgC71353210;     mYSWCAhBbjfpejdgC71353210 = mYSWCAhBbjfpejdgC23543964;     mYSWCAhBbjfpejdgC23543964 = mYSWCAhBbjfpejdgC27822319;     mYSWCAhBbjfpejdgC27822319 = mYSWCAhBbjfpejdgC77937903;     mYSWCAhBbjfpejdgC77937903 = mYSWCAhBbjfpejdgC9624958;     mYSWCAhBbjfpejdgC9624958 = mYSWCAhBbjfpejdgC50222092;     mYSWCAhBbjfpejdgC50222092 = mYSWCAhBbjfpejdgC5473505;     mYSWCAhBbjfpejdgC5473505 = mYSWCAhBbjfpejdgC49519318;     mYSWCAhBbjfpejdgC49519318 = mYSWCAhBbjfpejdgC19510014;     mYSWCAhBbjfpejdgC19510014 = mYSWCAhBbjfpejdgC12626673;     mYSWCAhBbjfpejdgC12626673 = mYSWCAhBbjfpejdgC37418628;     mYSWCAhBbjfpejdgC37418628 = mYSWCAhBbjfpejdgC82045757;     mYSWCAhBbjfpejdgC82045757 = mYSWCAhBbjfpejdgC78287935;     mYSWCAhBbjfpejdgC78287935 = mYSWCAhBbjfpejdgC89089172;     mYSWCAhBbjfpejdgC89089172 = mYSWCAhBbjfpejdgC43405186;     mYSWCAhBbjfpejdgC43405186 = mYSWCAhBbjfpejdgC17315994;     mYSWCAhBbjfpejdgC17315994 = mYSWCAhBbjfpejdgC88054194;     mYSWCAhBbjfpejdgC88054194 = mYSWCAhBbjfpejdgC54709082;     mYSWCAhBbjfpejdgC54709082 = mYSWCAhBbjfpejdgC51679780;     mYSWCAhBbjfpejdgC51679780 = mYSWCAhBbjfpejdgC35873068;     mYSWCAhBbjfpejdgC35873068 = mYSWCAhBbjfpejdgC25302371;     mYSWCAhBbjfpejdgC25302371 = mYSWCAhBbjfpejdgC98300093;     mYSWCAhBbjfpejdgC98300093 = mYSWCAhBbjfpejdgC77882746;     mYSWCAhBbjfpejdgC77882746 = mYSWCAhBbjfpejdgC67437923;     mYSWCAhBbjfpejdgC67437923 = mYSWCAhBbjfpejdgC87987531;     mYSWCAhBbjfpejdgC87987531 = mYSWCAhBbjfpejdgC13096947;     mYSWCAhBbjfpejdgC13096947 = mYSWCAhBbjfpejdgC9536881;     mYSWCAhBbjfpejdgC9536881 = mYSWCAhBbjfpejdgC25611591;     mYSWCAhBbjfpejdgC25611591 = mYSWCAhBbjfpejdgC65839578;     mYSWCAhBbjfpejdgC65839578 = mYSWCAhBbjfpejdgC87509049;     mYSWCAhBbjfpejdgC87509049 = mYSWCAhBbjfpejdgC78768478;     mYSWCAhBbjfpejdgC78768478 = mYSWCAhBbjfpejdgC45802523;     mYSWCAhBbjfpejdgC45802523 = mYSWCAhBbjfpejdgC45123675;     mYSWCAhBbjfpejdgC45123675 = mYSWCAhBbjfpejdgC56472964;     mYSWCAhBbjfpejdgC56472964 = mYSWCAhBbjfpejdgC27871604;     mYSWCAhBbjfpejdgC27871604 = mYSWCAhBbjfpejdgC16287171;     mYSWCAhBbjfpejdgC16287171 = mYSWCAhBbjfpejdgC84945410;     mYSWCAhBbjfpejdgC84945410 = mYSWCAhBbjfpejdgC48338016;     mYSWCAhBbjfpejdgC48338016 = mYSWCAhBbjfpejdgC8412689;     mYSWCAhBbjfpejdgC8412689 = mYSWCAhBbjfpejdgC2136056;     mYSWCAhBbjfpejdgC2136056 = mYSWCAhBbjfpejdgC73937398;     mYSWCAhBbjfpejdgC73937398 = mYSWCAhBbjfpejdgC69183583;     mYSWCAhBbjfpejdgC69183583 = mYSWCAhBbjfpejdgC34783564;     mYSWCAhBbjfpejdgC34783564 = mYSWCAhBbjfpejdgC24025891;     mYSWCAhBbjfpejdgC24025891 = mYSWCAhBbjfpejdgC38829070;     mYSWCAhBbjfpejdgC38829070 = mYSWCAhBbjfpejdgC16954420;     mYSWCAhBbjfpejdgC16954420 = mYSWCAhBbjfpejdgC28089980;     mYSWCAhBbjfpejdgC28089980 = mYSWCAhBbjfpejdgC37791573;     mYSWCAhBbjfpejdgC37791573 = mYSWCAhBbjfpejdgC13374069;     mYSWCAhBbjfpejdgC13374069 = mYSWCAhBbjfpejdgC24721066;     mYSWCAhBbjfpejdgC24721066 = mYSWCAhBbjfpejdgC82757725;     mYSWCAhBbjfpejdgC82757725 = mYSWCAhBbjfpejdgC53116969;     mYSWCAhBbjfpejdgC53116969 = mYSWCAhBbjfpejdgC32717962;     mYSWCAhBbjfpejdgC32717962 = mYSWCAhBbjfpejdgC77830984;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wcpOAdZTQtZInNcL24687556() {     double ZKkptGojiZMivDsLi13289953 = 16308452;    double ZKkptGojiZMivDsLi12493281 = 10522923;    double ZKkptGojiZMivDsLi5005370 = -962082795;    double ZKkptGojiZMivDsLi70046341 = -318877688;    double ZKkptGojiZMivDsLi97355314 = -441370504;    double ZKkptGojiZMivDsLi24536859 = -272893803;    double ZKkptGojiZMivDsLi93893726 = -882917606;    double ZKkptGojiZMivDsLi92215283 = -196696811;    double ZKkptGojiZMivDsLi15847720 = -609850355;    double ZKkptGojiZMivDsLi87649441 = -844209859;    double ZKkptGojiZMivDsLi42219662 = -449997128;    double ZKkptGojiZMivDsLi15852120 = -491680635;    double ZKkptGojiZMivDsLi82239916 = -544235350;    double ZKkptGojiZMivDsLi5734535 = -570220507;    double ZKkptGojiZMivDsLi72729826 = -163460332;    double ZKkptGojiZMivDsLi27226922 = 37581651;    double ZKkptGojiZMivDsLi42823644 = -128676393;    double ZKkptGojiZMivDsLi19235054 = -717351654;    double ZKkptGojiZMivDsLi36967956 = -879685460;    double ZKkptGojiZMivDsLi76228063 = -917300733;    double ZKkptGojiZMivDsLi20963501 = -112293711;    double ZKkptGojiZMivDsLi61742495 = -405343931;    double ZKkptGojiZMivDsLi57119909 = -20325522;    double ZKkptGojiZMivDsLi23993211 = -812142776;    double ZKkptGojiZMivDsLi23795181 = -431559788;    double ZKkptGojiZMivDsLi73480282 = -373879415;    double ZKkptGojiZMivDsLi59645388 = -24919909;    double ZKkptGojiZMivDsLi80559559 = -234695547;    double ZKkptGojiZMivDsLi59435943 = -784896418;    double ZKkptGojiZMivDsLi31809331 = -899894432;    double ZKkptGojiZMivDsLi36620878 = 323142;    double ZKkptGojiZMivDsLi90181165 = -720706847;    double ZKkptGojiZMivDsLi91628914 = -865429377;    double ZKkptGojiZMivDsLi48451327 = -920763640;    double ZKkptGojiZMivDsLi20102032 = -837325180;    double ZKkptGojiZMivDsLi88213407 = -384437228;    double ZKkptGojiZMivDsLi25659515 = -261692681;    double ZKkptGojiZMivDsLi50215580 = -205770084;    double ZKkptGojiZMivDsLi36768477 = -380705102;    double ZKkptGojiZMivDsLi91152277 = -922998459;    double ZKkptGojiZMivDsLi90230862 = -638467469;    double ZKkptGojiZMivDsLi31402187 = 81413984;    double ZKkptGojiZMivDsLi60195299 = -618350454;    double ZKkptGojiZMivDsLi47957390 = -663790139;    double ZKkptGojiZMivDsLi84414200 = -795902561;    double ZKkptGojiZMivDsLi16614883 = -47078968;    double ZKkptGojiZMivDsLi25654310 = -964643748;    double ZKkptGojiZMivDsLi89079522 = -286170881;    double ZKkptGojiZMivDsLi46436629 = -594711784;    double ZKkptGojiZMivDsLi52351908 = -648712776;    double ZKkptGojiZMivDsLi58708627 = 74540040;    double ZKkptGojiZMivDsLi93110430 = -197797033;    double ZKkptGojiZMivDsLi18843241 = -282588536;    double ZKkptGojiZMivDsLi21465351 = -16482175;    double ZKkptGojiZMivDsLi3724055 = -431573545;    double ZKkptGojiZMivDsLi51547457 = -578347618;    double ZKkptGojiZMivDsLi55373371 = -969151555;    double ZKkptGojiZMivDsLi81012159 = -49940019;    double ZKkptGojiZMivDsLi46251161 = -887317901;    double ZKkptGojiZMivDsLi23875033 = 32508911;    double ZKkptGojiZMivDsLi64891470 = -147973894;    double ZKkptGojiZMivDsLi13334167 = -548222059;    double ZKkptGojiZMivDsLi32779340 = -411800393;    double ZKkptGojiZMivDsLi84038389 = -709955924;    double ZKkptGojiZMivDsLi51028563 = -744533001;    double ZKkptGojiZMivDsLi52038497 = -729290281;    double ZKkptGojiZMivDsLi24223206 = -626251258;    double ZKkptGojiZMivDsLi33788589 = -623471711;    double ZKkptGojiZMivDsLi85632503 = -732895327;    double ZKkptGojiZMivDsLi84516419 = -779023105;    double ZKkptGojiZMivDsLi1567408 = -700725669;    double ZKkptGojiZMivDsLi92608064 = -922906309;    double ZKkptGojiZMivDsLi82466576 = -236646552;    double ZKkptGojiZMivDsLi45815678 = -956687001;    double ZKkptGojiZMivDsLi85997201 = -178833265;    double ZKkptGojiZMivDsLi89561314 = -93707696;    double ZKkptGojiZMivDsLi1547197 = -786993478;    double ZKkptGojiZMivDsLi9162520 = -356535384;    double ZKkptGojiZMivDsLi39579010 = 83759784;    double ZKkptGojiZMivDsLi7180298 = -284480820;    double ZKkptGojiZMivDsLi47825972 = -409235667;    double ZKkptGojiZMivDsLi70565866 = -738749029;    double ZKkptGojiZMivDsLi34122930 = -639983764;    double ZKkptGojiZMivDsLi7084036 = -36183643;    double ZKkptGojiZMivDsLi73100703 = -874434472;    double ZKkptGojiZMivDsLi43510448 = -801879826;    double ZKkptGojiZMivDsLi71337925 = -338118312;    double ZKkptGojiZMivDsLi70163563 = -748947202;    double ZKkptGojiZMivDsLi44727273 = -389190096;    double ZKkptGojiZMivDsLi68554574 = -158977563;    double ZKkptGojiZMivDsLi32840036 = -415285674;    double ZKkptGojiZMivDsLi44647355 = -111752663;    double ZKkptGojiZMivDsLi3964420 = -318452183;    double ZKkptGojiZMivDsLi12893444 = -313214013;    double ZKkptGojiZMivDsLi26260807 = -675024566;    double ZKkptGojiZMivDsLi76896695 = 9754590;    double ZKkptGojiZMivDsLi98622846 = -506785623;    double ZKkptGojiZMivDsLi76156909 = -908394530;    double ZKkptGojiZMivDsLi96928826 = -919257139;    double ZKkptGojiZMivDsLi32375704 = 16308452;     ZKkptGojiZMivDsLi13289953 = ZKkptGojiZMivDsLi12493281;     ZKkptGojiZMivDsLi12493281 = ZKkptGojiZMivDsLi5005370;     ZKkptGojiZMivDsLi5005370 = ZKkptGojiZMivDsLi70046341;     ZKkptGojiZMivDsLi70046341 = ZKkptGojiZMivDsLi97355314;     ZKkptGojiZMivDsLi97355314 = ZKkptGojiZMivDsLi24536859;     ZKkptGojiZMivDsLi24536859 = ZKkptGojiZMivDsLi93893726;     ZKkptGojiZMivDsLi93893726 = ZKkptGojiZMivDsLi92215283;     ZKkptGojiZMivDsLi92215283 = ZKkptGojiZMivDsLi15847720;     ZKkptGojiZMivDsLi15847720 = ZKkptGojiZMivDsLi87649441;     ZKkptGojiZMivDsLi87649441 = ZKkptGojiZMivDsLi42219662;     ZKkptGojiZMivDsLi42219662 = ZKkptGojiZMivDsLi15852120;     ZKkptGojiZMivDsLi15852120 = ZKkptGojiZMivDsLi82239916;     ZKkptGojiZMivDsLi82239916 = ZKkptGojiZMivDsLi5734535;     ZKkptGojiZMivDsLi5734535 = ZKkptGojiZMivDsLi72729826;     ZKkptGojiZMivDsLi72729826 = ZKkptGojiZMivDsLi27226922;     ZKkptGojiZMivDsLi27226922 = ZKkptGojiZMivDsLi42823644;     ZKkptGojiZMivDsLi42823644 = ZKkptGojiZMivDsLi19235054;     ZKkptGojiZMivDsLi19235054 = ZKkptGojiZMivDsLi36967956;     ZKkptGojiZMivDsLi36967956 = ZKkptGojiZMivDsLi76228063;     ZKkptGojiZMivDsLi76228063 = ZKkptGojiZMivDsLi20963501;     ZKkptGojiZMivDsLi20963501 = ZKkptGojiZMivDsLi61742495;     ZKkptGojiZMivDsLi61742495 = ZKkptGojiZMivDsLi57119909;     ZKkptGojiZMivDsLi57119909 = ZKkptGojiZMivDsLi23993211;     ZKkptGojiZMivDsLi23993211 = ZKkptGojiZMivDsLi23795181;     ZKkptGojiZMivDsLi23795181 = ZKkptGojiZMivDsLi73480282;     ZKkptGojiZMivDsLi73480282 = ZKkptGojiZMivDsLi59645388;     ZKkptGojiZMivDsLi59645388 = ZKkptGojiZMivDsLi80559559;     ZKkptGojiZMivDsLi80559559 = ZKkptGojiZMivDsLi59435943;     ZKkptGojiZMivDsLi59435943 = ZKkptGojiZMivDsLi31809331;     ZKkptGojiZMivDsLi31809331 = ZKkptGojiZMivDsLi36620878;     ZKkptGojiZMivDsLi36620878 = ZKkptGojiZMivDsLi90181165;     ZKkptGojiZMivDsLi90181165 = ZKkptGojiZMivDsLi91628914;     ZKkptGojiZMivDsLi91628914 = ZKkptGojiZMivDsLi48451327;     ZKkptGojiZMivDsLi48451327 = ZKkptGojiZMivDsLi20102032;     ZKkptGojiZMivDsLi20102032 = ZKkptGojiZMivDsLi88213407;     ZKkptGojiZMivDsLi88213407 = ZKkptGojiZMivDsLi25659515;     ZKkptGojiZMivDsLi25659515 = ZKkptGojiZMivDsLi50215580;     ZKkptGojiZMivDsLi50215580 = ZKkptGojiZMivDsLi36768477;     ZKkptGojiZMivDsLi36768477 = ZKkptGojiZMivDsLi91152277;     ZKkptGojiZMivDsLi91152277 = ZKkptGojiZMivDsLi90230862;     ZKkptGojiZMivDsLi90230862 = ZKkptGojiZMivDsLi31402187;     ZKkptGojiZMivDsLi31402187 = ZKkptGojiZMivDsLi60195299;     ZKkptGojiZMivDsLi60195299 = ZKkptGojiZMivDsLi47957390;     ZKkptGojiZMivDsLi47957390 = ZKkptGojiZMivDsLi84414200;     ZKkptGojiZMivDsLi84414200 = ZKkptGojiZMivDsLi16614883;     ZKkptGojiZMivDsLi16614883 = ZKkptGojiZMivDsLi25654310;     ZKkptGojiZMivDsLi25654310 = ZKkptGojiZMivDsLi89079522;     ZKkptGojiZMivDsLi89079522 = ZKkptGojiZMivDsLi46436629;     ZKkptGojiZMivDsLi46436629 = ZKkptGojiZMivDsLi52351908;     ZKkptGojiZMivDsLi52351908 = ZKkptGojiZMivDsLi58708627;     ZKkptGojiZMivDsLi58708627 = ZKkptGojiZMivDsLi93110430;     ZKkptGojiZMivDsLi93110430 = ZKkptGojiZMivDsLi18843241;     ZKkptGojiZMivDsLi18843241 = ZKkptGojiZMivDsLi21465351;     ZKkptGojiZMivDsLi21465351 = ZKkptGojiZMivDsLi3724055;     ZKkptGojiZMivDsLi3724055 = ZKkptGojiZMivDsLi51547457;     ZKkptGojiZMivDsLi51547457 = ZKkptGojiZMivDsLi55373371;     ZKkptGojiZMivDsLi55373371 = ZKkptGojiZMivDsLi81012159;     ZKkptGojiZMivDsLi81012159 = ZKkptGojiZMivDsLi46251161;     ZKkptGojiZMivDsLi46251161 = ZKkptGojiZMivDsLi23875033;     ZKkptGojiZMivDsLi23875033 = ZKkptGojiZMivDsLi64891470;     ZKkptGojiZMivDsLi64891470 = ZKkptGojiZMivDsLi13334167;     ZKkptGojiZMivDsLi13334167 = ZKkptGojiZMivDsLi32779340;     ZKkptGojiZMivDsLi32779340 = ZKkptGojiZMivDsLi84038389;     ZKkptGojiZMivDsLi84038389 = ZKkptGojiZMivDsLi51028563;     ZKkptGojiZMivDsLi51028563 = ZKkptGojiZMivDsLi52038497;     ZKkptGojiZMivDsLi52038497 = ZKkptGojiZMivDsLi24223206;     ZKkptGojiZMivDsLi24223206 = ZKkptGojiZMivDsLi33788589;     ZKkptGojiZMivDsLi33788589 = ZKkptGojiZMivDsLi85632503;     ZKkptGojiZMivDsLi85632503 = ZKkptGojiZMivDsLi84516419;     ZKkptGojiZMivDsLi84516419 = ZKkptGojiZMivDsLi1567408;     ZKkptGojiZMivDsLi1567408 = ZKkptGojiZMivDsLi92608064;     ZKkptGojiZMivDsLi92608064 = ZKkptGojiZMivDsLi82466576;     ZKkptGojiZMivDsLi82466576 = ZKkptGojiZMivDsLi45815678;     ZKkptGojiZMivDsLi45815678 = ZKkptGojiZMivDsLi85997201;     ZKkptGojiZMivDsLi85997201 = ZKkptGojiZMivDsLi89561314;     ZKkptGojiZMivDsLi89561314 = ZKkptGojiZMivDsLi1547197;     ZKkptGojiZMivDsLi1547197 = ZKkptGojiZMivDsLi9162520;     ZKkptGojiZMivDsLi9162520 = ZKkptGojiZMivDsLi39579010;     ZKkptGojiZMivDsLi39579010 = ZKkptGojiZMivDsLi7180298;     ZKkptGojiZMivDsLi7180298 = ZKkptGojiZMivDsLi47825972;     ZKkptGojiZMivDsLi47825972 = ZKkptGojiZMivDsLi70565866;     ZKkptGojiZMivDsLi70565866 = ZKkptGojiZMivDsLi34122930;     ZKkptGojiZMivDsLi34122930 = ZKkptGojiZMivDsLi7084036;     ZKkptGojiZMivDsLi7084036 = ZKkptGojiZMivDsLi73100703;     ZKkptGojiZMivDsLi73100703 = ZKkptGojiZMivDsLi43510448;     ZKkptGojiZMivDsLi43510448 = ZKkptGojiZMivDsLi71337925;     ZKkptGojiZMivDsLi71337925 = ZKkptGojiZMivDsLi70163563;     ZKkptGojiZMivDsLi70163563 = ZKkptGojiZMivDsLi44727273;     ZKkptGojiZMivDsLi44727273 = ZKkptGojiZMivDsLi68554574;     ZKkptGojiZMivDsLi68554574 = ZKkptGojiZMivDsLi32840036;     ZKkptGojiZMivDsLi32840036 = ZKkptGojiZMivDsLi44647355;     ZKkptGojiZMivDsLi44647355 = ZKkptGojiZMivDsLi3964420;     ZKkptGojiZMivDsLi3964420 = ZKkptGojiZMivDsLi12893444;     ZKkptGojiZMivDsLi12893444 = ZKkptGojiZMivDsLi26260807;     ZKkptGojiZMivDsLi26260807 = ZKkptGojiZMivDsLi76896695;     ZKkptGojiZMivDsLi76896695 = ZKkptGojiZMivDsLi98622846;     ZKkptGojiZMivDsLi98622846 = ZKkptGojiZMivDsLi76156909;     ZKkptGojiZMivDsLi76156909 = ZKkptGojiZMivDsLi96928826;     ZKkptGojiZMivDsLi96928826 = ZKkptGojiZMivDsLi32375704;     ZKkptGojiZMivDsLi32375704 = ZKkptGojiZMivDsLi13289953;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void roPWkwxbvFJdQtsf39736623() {     double FEHIiYuKtDvmgPLzj19407059 = -95593437;    double FEHIiYuKtDvmgPLzj55866560 = -799076858;    double FEHIiYuKtDvmgPLzj21047554 = -12915716;    double FEHIiYuKtDvmgPLzj97121122 = -509626319;    double FEHIiYuKtDvmgPLzj66250803 = -962020375;    double FEHIiYuKtDvmgPLzj15954061 = -537390038;    double FEHIiYuKtDvmgPLzj54382692 = 78117417;    double FEHIiYuKtDvmgPLzj41975175 = -483570724;    double FEHIiYuKtDvmgPLzj80490215 = -377883021;    double FEHIiYuKtDvmgPLzj28879101 = -73185336;    double FEHIiYuKtDvmgPLzj2329953 = -327434714;    double FEHIiYuKtDvmgPLzj47675162 = -936086836;    double FEHIiYuKtDvmgPLzj68449035 = -984126439;    double FEHIiYuKtDvmgPLzj88878211 = -2459075;    double FEHIiYuKtDvmgPLzj68472253 = -853922499;    double FEHIiYuKtDvmgPLzj68732316 = 64582108;    double FEHIiYuKtDvmgPLzj36659647 = -745221505;    double FEHIiYuKtDvmgPLzj16288207 = -382874297;    double FEHIiYuKtDvmgPLzj98797797 = -185454557;    double FEHIiYuKtDvmgPLzj56778228 = -174203652;    double FEHIiYuKtDvmgPLzj9516695 = 32218833;    double FEHIiYuKtDvmgPLzj48263519 = -313273647;    double FEHIiYuKtDvmgPLzj92710701 = -866843009;    double FEHIiYuKtDvmgPLzj7985585 = 65552236;    double FEHIiYuKtDvmgPLzj67494831 = -804755821;    double FEHIiYuKtDvmgPLzj98201957 = -252534738;    double FEHIiYuKtDvmgPLzj58105166 = 74417888;    double FEHIiYuKtDvmgPLzj93666684 = -300624501;    double FEHIiYuKtDvmgPLzj25568199 = 92720877;    double FEHIiYuKtDvmgPLzj424968 = -717365299;    double FEHIiYuKtDvmgPLzj28863428 = 87002204;    double FEHIiYuKtDvmgPLzj87791787 = -430153663;    double FEHIiYuKtDvmgPLzj1339863 = -758042747;    double FEHIiYuKtDvmgPLzj43281892 = -672305406;    double FEHIiYuKtDvmgPLzj81576683 = -376058285;    double FEHIiYuKtDvmgPLzj56939320 = -598463336;    double FEHIiYuKtDvmgPLzj6921449 = -215328551;    double FEHIiYuKtDvmgPLzj84305792 = 22723907;    double FEHIiYuKtDvmgPLzj10378731 = -374628599;    double FEHIiYuKtDvmgPLzj78654215 = -296437682;    double FEHIiYuKtDvmgPLzj95900086 = -703741363;    double FEHIiYuKtDvmgPLzj63552644 = -188728294;    double FEHIiYuKtDvmgPLzj38262277 = -699155037;    double FEHIiYuKtDvmgPLzj35365130 = -683133077;    double FEHIiYuKtDvmgPLzj96366665 = -457800715;    double FEHIiYuKtDvmgPLzj28554078 = -32295085;    double FEHIiYuKtDvmgPLzj43432884 = 16580714;    double FEHIiYuKtDvmgPLzj97036463 = -296066413;    double FEHIiYuKtDvmgPLzj56112358 = -680448772;    double FEHIiYuKtDvmgPLzj65263560 = -735723584;    double FEHIiYuKtDvmgPLzj94409305 = -867753632;    double FEHIiYuKtDvmgPLzj3920150 = 1476647;    double FEHIiYuKtDvmgPLzj67091698 = -74378124;    double FEHIiYuKtDvmgPLzj17434232 = -336952320;    double FEHIiYuKtDvmgPLzj30219929 = -32105327;    double FEHIiYuKtDvmgPLzj71143540 = -782319790;    double FEHIiYuKtDvmgPLzj63155859 = -932233849;    double FEHIiYuKtDvmgPLzj13061970 = 21532048;    double FEHIiYuKtDvmgPLzj29626292 = -704870499;    double FEHIiYuKtDvmgPLzj68048846 = -609485637;    double FEHIiYuKtDvmgPLzj57848895 = -511807926;    double FEHIiYuKtDvmgPLzj60716008 = -621258083;    double FEHIiYuKtDvmgPLzj16406976 = -476291601;    double FEHIiYuKtDvmgPLzj80065247 = -660517722;    double FEHIiYuKtDvmgPLzj15673 = -60187540;    double FEHIiYuKtDvmgPLzj14538166 = -897281052;    double FEHIiYuKtDvmgPLzj46335300 = -78044090;    double FEHIiYuKtDvmgPLzj25167144 = -211821034;    double FEHIiYuKtDvmgPLzj7301528 = -626400790;    double FEHIiYuKtDvmgPLzj11532933 = -155459163;    double FEHIiYuKtDvmgPLzj61810867 = -720089342;    double FEHIiYuKtDvmgPLzj52353855 = -667945412;    double FEHIiYuKtDvmgPLzj5909476 = 91754302;    double FEHIiYuKtDvmgPLzj20143582 = -889016875;    double FEHIiYuKtDvmgPLzj60878142 = -470462290;    double FEHIiYuKtDvmgPLzj45964050 = -779052874;    double FEHIiYuKtDvmgPLzj10001242 = -614118611;    double FEHIiYuKtDvmgPLzj57345571 = -83709933;    double FEHIiYuKtDvmgPLzj11618919 = -476647049;    double FEHIiYuKtDvmgPLzj38940753 = -672460736;    double FEHIiYuKtDvmgPLzj54769073 = -169115453;    double FEHIiYuKtDvmgPLzj61068703 = -629515699;    double FEHIiYuKtDvmgPLzj37554326 = -620175729;    double FEHIiYuKtDvmgPLzj60304639 = -171555539;    double FEHIiYuKtDvmgPLzj6015662 = -849611668;    double FEHIiYuKtDvmgPLzj24943278 = -914474443;    double FEHIiYuKtDvmgPLzj20700090 = -255775540;    double FEHIiYuKtDvmgPLzj83905631 = -321090427;    double FEHIiYuKtDvmgPLzj13061963 = -540200079;    double FEHIiYuKtDvmgPLzj10433143 = -593738496;    double FEHIiYuKtDvmgPLzj93783461 = -666229487;    double FEHIiYuKtDvmgPLzj93859479 = -136860599;    double FEHIiYuKtDvmgPLzj54679501 = -272405595;    double FEHIiYuKtDvmgPLzj42329885 = -765142962;    double FEHIiYuKtDvmgPLzj20805321 = -784629756;    double FEHIiYuKtDvmgPLzj35184079 = 17516719;    double FEHIiYuKtDvmgPLzj47145668 = -712436693;    double FEHIiYuKtDvmgPLzj58197029 = 61362684;    double FEHIiYuKtDvmgPLzj35349457 = -522945537;    double FEHIiYuKtDvmgPLzj81828500 = -95593437;     FEHIiYuKtDvmgPLzj19407059 = FEHIiYuKtDvmgPLzj55866560;     FEHIiYuKtDvmgPLzj55866560 = FEHIiYuKtDvmgPLzj21047554;     FEHIiYuKtDvmgPLzj21047554 = FEHIiYuKtDvmgPLzj97121122;     FEHIiYuKtDvmgPLzj97121122 = FEHIiYuKtDvmgPLzj66250803;     FEHIiYuKtDvmgPLzj66250803 = FEHIiYuKtDvmgPLzj15954061;     FEHIiYuKtDvmgPLzj15954061 = FEHIiYuKtDvmgPLzj54382692;     FEHIiYuKtDvmgPLzj54382692 = FEHIiYuKtDvmgPLzj41975175;     FEHIiYuKtDvmgPLzj41975175 = FEHIiYuKtDvmgPLzj80490215;     FEHIiYuKtDvmgPLzj80490215 = FEHIiYuKtDvmgPLzj28879101;     FEHIiYuKtDvmgPLzj28879101 = FEHIiYuKtDvmgPLzj2329953;     FEHIiYuKtDvmgPLzj2329953 = FEHIiYuKtDvmgPLzj47675162;     FEHIiYuKtDvmgPLzj47675162 = FEHIiYuKtDvmgPLzj68449035;     FEHIiYuKtDvmgPLzj68449035 = FEHIiYuKtDvmgPLzj88878211;     FEHIiYuKtDvmgPLzj88878211 = FEHIiYuKtDvmgPLzj68472253;     FEHIiYuKtDvmgPLzj68472253 = FEHIiYuKtDvmgPLzj68732316;     FEHIiYuKtDvmgPLzj68732316 = FEHIiYuKtDvmgPLzj36659647;     FEHIiYuKtDvmgPLzj36659647 = FEHIiYuKtDvmgPLzj16288207;     FEHIiYuKtDvmgPLzj16288207 = FEHIiYuKtDvmgPLzj98797797;     FEHIiYuKtDvmgPLzj98797797 = FEHIiYuKtDvmgPLzj56778228;     FEHIiYuKtDvmgPLzj56778228 = FEHIiYuKtDvmgPLzj9516695;     FEHIiYuKtDvmgPLzj9516695 = FEHIiYuKtDvmgPLzj48263519;     FEHIiYuKtDvmgPLzj48263519 = FEHIiYuKtDvmgPLzj92710701;     FEHIiYuKtDvmgPLzj92710701 = FEHIiYuKtDvmgPLzj7985585;     FEHIiYuKtDvmgPLzj7985585 = FEHIiYuKtDvmgPLzj67494831;     FEHIiYuKtDvmgPLzj67494831 = FEHIiYuKtDvmgPLzj98201957;     FEHIiYuKtDvmgPLzj98201957 = FEHIiYuKtDvmgPLzj58105166;     FEHIiYuKtDvmgPLzj58105166 = FEHIiYuKtDvmgPLzj93666684;     FEHIiYuKtDvmgPLzj93666684 = FEHIiYuKtDvmgPLzj25568199;     FEHIiYuKtDvmgPLzj25568199 = FEHIiYuKtDvmgPLzj424968;     FEHIiYuKtDvmgPLzj424968 = FEHIiYuKtDvmgPLzj28863428;     FEHIiYuKtDvmgPLzj28863428 = FEHIiYuKtDvmgPLzj87791787;     FEHIiYuKtDvmgPLzj87791787 = FEHIiYuKtDvmgPLzj1339863;     FEHIiYuKtDvmgPLzj1339863 = FEHIiYuKtDvmgPLzj43281892;     FEHIiYuKtDvmgPLzj43281892 = FEHIiYuKtDvmgPLzj81576683;     FEHIiYuKtDvmgPLzj81576683 = FEHIiYuKtDvmgPLzj56939320;     FEHIiYuKtDvmgPLzj56939320 = FEHIiYuKtDvmgPLzj6921449;     FEHIiYuKtDvmgPLzj6921449 = FEHIiYuKtDvmgPLzj84305792;     FEHIiYuKtDvmgPLzj84305792 = FEHIiYuKtDvmgPLzj10378731;     FEHIiYuKtDvmgPLzj10378731 = FEHIiYuKtDvmgPLzj78654215;     FEHIiYuKtDvmgPLzj78654215 = FEHIiYuKtDvmgPLzj95900086;     FEHIiYuKtDvmgPLzj95900086 = FEHIiYuKtDvmgPLzj63552644;     FEHIiYuKtDvmgPLzj63552644 = FEHIiYuKtDvmgPLzj38262277;     FEHIiYuKtDvmgPLzj38262277 = FEHIiYuKtDvmgPLzj35365130;     FEHIiYuKtDvmgPLzj35365130 = FEHIiYuKtDvmgPLzj96366665;     FEHIiYuKtDvmgPLzj96366665 = FEHIiYuKtDvmgPLzj28554078;     FEHIiYuKtDvmgPLzj28554078 = FEHIiYuKtDvmgPLzj43432884;     FEHIiYuKtDvmgPLzj43432884 = FEHIiYuKtDvmgPLzj97036463;     FEHIiYuKtDvmgPLzj97036463 = FEHIiYuKtDvmgPLzj56112358;     FEHIiYuKtDvmgPLzj56112358 = FEHIiYuKtDvmgPLzj65263560;     FEHIiYuKtDvmgPLzj65263560 = FEHIiYuKtDvmgPLzj94409305;     FEHIiYuKtDvmgPLzj94409305 = FEHIiYuKtDvmgPLzj3920150;     FEHIiYuKtDvmgPLzj3920150 = FEHIiYuKtDvmgPLzj67091698;     FEHIiYuKtDvmgPLzj67091698 = FEHIiYuKtDvmgPLzj17434232;     FEHIiYuKtDvmgPLzj17434232 = FEHIiYuKtDvmgPLzj30219929;     FEHIiYuKtDvmgPLzj30219929 = FEHIiYuKtDvmgPLzj71143540;     FEHIiYuKtDvmgPLzj71143540 = FEHIiYuKtDvmgPLzj63155859;     FEHIiYuKtDvmgPLzj63155859 = FEHIiYuKtDvmgPLzj13061970;     FEHIiYuKtDvmgPLzj13061970 = FEHIiYuKtDvmgPLzj29626292;     FEHIiYuKtDvmgPLzj29626292 = FEHIiYuKtDvmgPLzj68048846;     FEHIiYuKtDvmgPLzj68048846 = FEHIiYuKtDvmgPLzj57848895;     FEHIiYuKtDvmgPLzj57848895 = FEHIiYuKtDvmgPLzj60716008;     FEHIiYuKtDvmgPLzj60716008 = FEHIiYuKtDvmgPLzj16406976;     FEHIiYuKtDvmgPLzj16406976 = FEHIiYuKtDvmgPLzj80065247;     FEHIiYuKtDvmgPLzj80065247 = FEHIiYuKtDvmgPLzj15673;     FEHIiYuKtDvmgPLzj15673 = FEHIiYuKtDvmgPLzj14538166;     FEHIiYuKtDvmgPLzj14538166 = FEHIiYuKtDvmgPLzj46335300;     FEHIiYuKtDvmgPLzj46335300 = FEHIiYuKtDvmgPLzj25167144;     FEHIiYuKtDvmgPLzj25167144 = FEHIiYuKtDvmgPLzj7301528;     FEHIiYuKtDvmgPLzj7301528 = FEHIiYuKtDvmgPLzj11532933;     FEHIiYuKtDvmgPLzj11532933 = FEHIiYuKtDvmgPLzj61810867;     FEHIiYuKtDvmgPLzj61810867 = FEHIiYuKtDvmgPLzj52353855;     FEHIiYuKtDvmgPLzj52353855 = FEHIiYuKtDvmgPLzj5909476;     FEHIiYuKtDvmgPLzj5909476 = FEHIiYuKtDvmgPLzj20143582;     FEHIiYuKtDvmgPLzj20143582 = FEHIiYuKtDvmgPLzj60878142;     FEHIiYuKtDvmgPLzj60878142 = FEHIiYuKtDvmgPLzj45964050;     FEHIiYuKtDvmgPLzj45964050 = FEHIiYuKtDvmgPLzj10001242;     FEHIiYuKtDvmgPLzj10001242 = FEHIiYuKtDvmgPLzj57345571;     FEHIiYuKtDvmgPLzj57345571 = FEHIiYuKtDvmgPLzj11618919;     FEHIiYuKtDvmgPLzj11618919 = FEHIiYuKtDvmgPLzj38940753;     FEHIiYuKtDvmgPLzj38940753 = FEHIiYuKtDvmgPLzj54769073;     FEHIiYuKtDvmgPLzj54769073 = FEHIiYuKtDvmgPLzj61068703;     FEHIiYuKtDvmgPLzj61068703 = FEHIiYuKtDvmgPLzj37554326;     FEHIiYuKtDvmgPLzj37554326 = FEHIiYuKtDvmgPLzj60304639;     FEHIiYuKtDvmgPLzj60304639 = FEHIiYuKtDvmgPLzj6015662;     FEHIiYuKtDvmgPLzj6015662 = FEHIiYuKtDvmgPLzj24943278;     FEHIiYuKtDvmgPLzj24943278 = FEHIiYuKtDvmgPLzj20700090;     FEHIiYuKtDvmgPLzj20700090 = FEHIiYuKtDvmgPLzj83905631;     FEHIiYuKtDvmgPLzj83905631 = FEHIiYuKtDvmgPLzj13061963;     FEHIiYuKtDvmgPLzj13061963 = FEHIiYuKtDvmgPLzj10433143;     FEHIiYuKtDvmgPLzj10433143 = FEHIiYuKtDvmgPLzj93783461;     FEHIiYuKtDvmgPLzj93783461 = FEHIiYuKtDvmgPLzj93859479;     FEHIiYuKtDvmgPLzj93859479 = FEHIiYuKtDvmgPLzj54679501;     FEHIiYuKtDvmgPLzj54679501 = FEHIiYuKtDvmgPLzj42329885;     FEHIiYuKtDvmgPLzj42329885 = FEHIiYuKtDvmgPLzj20805321;     FEHIiYuKtDvmgPLzj20805321 = FEHIiYuKtDvmgPLzj35184079;     FEHIiYuKtDvmgPLzj35184079 = FEHIiYuKtDvmgPLzj47145668;     FEHIiYuKtDvmgPLzj47145668 = FEHIiYuKtDvmgPLzj58197029;     FEHIiYuKtDvmgPLzj58197029 = FEHIiYuKtDvmgPLzj35349457;     FEHIiYuKtDvmgPLzj35349457 = FEHIiYuKtDvmgPLzj81828500;     FEHIiYuKtDvmgPLzj81828500 = FEHIiYuKtDvmgPLzj19407059;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void eAfcRJdtKhDFeyIf7028223() {     double kptGYpIeTOwMpapnX54866027 = -561870337;    double kptGYpIeTOwMpapnX69138638 = -679628464;    double kptGYpIeTOwMpapnX58434030 = -778582791;    double kptGYpIeTOwMpapnX69344477 = -330777390;    double kptGYpIeTOwMpapnX22536073 = 32380779;    double kptGYpIeTOwMpapnX52176933 = -799737933;    double kptGYpIeTOwMpapnX65040098 = -866608317;    double kptGYpIeTOwMpapnX4106736 = -127804864;    double kptGYpIeTOwMpapnX82007820 = -866214428;    double kptGYpIeTOwMpapnX43781813 = -837872334;    double kptGYpIeTOwMpapnX40511688 = -129442444;    double kptGYpIeTOwMpapnX48243311 = -800207608;    double kptGYpIeTOwMpapnX6683681 = -436138944;    double kptGYpIeTOwMpapnX42019075 = 21177516;    double kptGYpIeTOwMpapnX26666770 = -257347514;    double kptGYpIeTOwMpapnX52273617 = -671229500;    double kptGYpIeTOwMpapnX92495583 = -172196058;    double kptGYpIeTOwMpapnX16048398 = -456302365;    double kptGYpIeTOwMpapnX99348127 = -502017978;    double kptGYpIeTOwMpapnX50615447 = -600764718;    double kptGYpIeTOwMpapnX22603557 = -725055856;    double kptGYpIeTOwMpapnX51685043 = -399909941;    double kptGYpIeTOwMpapnX63236081 = -504475193;    double kptGYpIeTOwMpapnX1778530 = -445272368;    double kptGYpIeTOwMpapnX75512782 = -828741019;    double kptGYpIeTOwMpapnX8900129 = 58888455;    double kptGYpIeTOwMpapnX18525741 = -831866253;    double kptGYpIeTOwMpapnX34395110 = -470396675;    double kptGYpIeTOwMpapnX72236413 = -481373925;    double kptGYpIeTOwMpapnX5958379 = -751872470;    double kptGYpIeTOwMpapnX47446659 = -464978283;    double kptGYpIeTOwMpapnX25614806 = -203125077;    double kptGYpIeTOwMpapnX13557874 = -905638724;    double kptGYpIeTOwMpapnX73030318 = -736999219;    double kptGYpIeTOwMpapnX47385137 = -327158536;    double kptGYpIeTOwMpapnX8500165 = -813028642;    double kptGYpIeTOwMpapnX56333265 = -80024591;    double kptGYpIeTOwMpapnX35521196 = 40068325;    double kptGYpIeTOwMpapnX40769293 = -25504876;    double kptGYpIeTOwMpapnX42925747 = -861700228;    double kptGYpIeTOwMpapnX29351694 = -472940277;    double kptGYpIeTOwMpapnX52917770 = -883273585;    double kptGYpIeTOwMpapnX27645655 = -252335823;    double kptGYpIeTOwMpapnX75496468 = -187465487;    double kptGYpIeTOwMpapnX96383124 = -284311877;    double kptGYpIeTOwMpapnX74515406 = -71218717;    double kptGYpIeTOwMpapnX62778047 = -467405647;    double kptGYpIeTOwMpapnX14762775 = -462593007;    double kptGYpIeTOwMpapnX79005024 = -344185652;    double kptGYpIeTOwMpapnX89793149 = -126750400;    double kptGYpIeTOwMpapnX75180029 = -742030189;    double kptGYpIeTOwMpapnX87405622 = -212175700;    double kptGYpIeTOwMpapnX35712847 = -930423678;    double kptGYpIeTOwMpapnX33426078 = -528460820;    double kptGYpIeTOwMpapnX84424665 = -378595473;    double kptGYpIeTOwMpapnX3180985 = -61960396;    double kptGYpIeTOwMpapnX5902558 = -75153271;    double kptGYpIeTOwMpapnX56655501 = -233310423;    double kptGYpIeTOwMpapnX93831695 = -502036372;    double kptGYpIeTOwMpapnX13635944 = 73492324;    double kptGYpIeTOwMpapnX33651193 = -967871681;    double kptGYpIeTOwMpapnX30644989 = -296211643;    double kptGYpIeTOwMpapnX31870322 = -646430939;    double kptGYpIeTOwMpapnX76049442 = -14341958;    double kptGYpIeTOwMpapnX96335153 = -272894051;    double kptGYpIeTOwMpapnX14896882 = -926317367;    double kptGYpIeTOwMpapnX34685437 = -894568884;    double kptGYpIeTOwMpapnX33653363 = -699139725;    double kptGYpIeTOwMpapnX94633937 = -651663948;    double kptGYpIeTOwMpapnX18166606 = -444318873;    double kptGYpIeTOwMpapnX95940351 = -491204909;    double kptGYpIeTOwMpapnX56974387 = -112264384;    double kptGYpIeTOwMpapnX75279105 = -330797490;    double kptGYpIeTOwMpapnX56422380 = -640317750;    double kptGYpIeTOwMpapnX21263753 = -27824442;    double kptGYpIeTOwMpapnX69685786 = -841782272;    double kptGYpIeTOwMpapnX24039388 = -47574119;    double kptGYpIeTOwMpapnX87739612 = -217009707;    double kptGYpIeTOwMpapnX5395405 = -60960492;    double kptGYpIeTOwMpapnX997376 = -657522303;    double kptGYpIeTOwMpapnX46122081 = -473705899;    double kptGYpIeTOwMpapnX3762966 = -269273247;    double kptGYpIeTOwMpapnX55390085 = -26211024;    double kptGYpIeTOwMpapnX82443264 = -254623526;    double kptGYpIeTOwMpapnX30778349 = 90157719;    double kptGYpIeTOwMpapnX60041037 = -152802584;    double kptGYpIeTOwMpapnX89901959 = -272701400;    double kptGYpIeTOwMpapnX80131795 = -277177905;    double kptGYpIeTOwMpapnX88605652 = -258403746;    double kptGYpIeTOwMpapnX44204153 = -165198141;    double kptGYpIeTOwMpapnX2597607 = -637875371;    double kptGYpIeTOwMpapnX99677764 = -846714168;    double kptGYpIeTOwMpapnX41689501 = -457895304;    double kptGYpIeTOwMpapnX27133349 = 1002800;    double kptGYpIeTOwMpapnX9274555 = -893828548;    double kptGYpIeTOwMpapnX98706704 = -76728635;    double kptGYpIeTOwMpapnX21047449 = -136842646;    double kptGYpIeTOwMpapnX51596213 = -137993866;    double kptGYpIeTOwMpapnX79161315 = -914571436;    double kptGYpIeTOwMpapnX81486243 = -561870337;     kptGYpIeTOwMpapnX54866027 = kptGYpIeTOwMpapnX69138638;     kptGYpIeTOwMpapnX69138638 = kptGYpIeTOwMpapnX58434030;     kptGYpIeTOwMpapnX58434030 = kptGYpIeTOwMpapnX69344477;     kptGYpIeTOwMpapnX69344477 = kptGYpIeTOwMpapnX22536073;     kptGYpIeTOwMpapnX22536073 = kptGYpIeTOwMpapnX52176933;     kptGYpIeTOwMpapnX52176933 = kptGYpIeTOwMpapnX65040098;     kptGYpIeTOwMpapnX65040098 = kptGYpIeTOwMpapnX4106736;     kptGYpIeTOwMpapnX4106736 = kptGYpIeTOwMpapnX82007820;     kptGYpIeTOwMpapnX82007820 = kptGYpIeTOwMpapnX43781813;     kptGYpIeTOwMpapnX43781813 = kptGYpIeTOwMpapnX40511688;     kptGYpIeTOwMpapnX40511688 = kptGYpIeTOwMpapnX48243311;     kptGYpIeTOwMpapnX48243311 = kptGYpIeTOwMpapnX6683681;     kptGYpIeTOwMpapnX6683681 = kptGYpIeTOwMpapnX42019075;     kptGYpIeTOwMpapnX42019075 = kptGYpIeTOwMpapnX26666770;     kptGYpIeTOwMpapnX26666770 = kptGYpIeTOwMpapnX52273617;     kptGYpIeTOwMpapnX52273617 = kptGYpIeTOwMpapnX92495583;     kptGYpIeTOwMpapnX92495583 = kptGYpIeTOwMpapnX16048398;     kptGYpIeTOwMpapnX16048398 = kptGYpIeTOwMpapnX99348127;     kptGYpIeTOwMpapnX99348127 = kptGYpIeTOwMpapnX50615447;     kptGYpIeTOwMpapnX50615447 = kptGYpIeTOwMpapnX22603557;     kptGYpIeTOwMpapnX22603557 = kptGYpIeTOwMpapnX51685043;     kptGYpIeTOwMpapnX51685043 = kptGYpIeTOwMpapnX63236081;     kptGYpIeTOwMpapnX63236081 = kptGYpIeTOwMpapnX1778530;     kptGYpIeTOwMpapnX1778530 = kptGYpIeTOwMpapnX75512782;     kptGYpIeTOwMpapnX75512782 = kptGYpIeTOwMpapnX8900129;     kptGYpIeTOwMpapnX8900129 = kptGYpIeTOwMpapnX18525741;     kptGYpIeTOwMpapnX18525741 = kptGYpIeTOwMpapnX34395110;     kptGYpIeTOwMpapnX34395110 = kptGYpIeTOwMpapnX72236413;     kptGYpIeTOwMpapnX72236413 = kptGYpIeTOwMpapnX5958379;     kptGYpIeTOwMpapnX5958379 = kptGYpIeTOwMpapnX47446659;     kptGYpIeTOwMpapnX47446659 = kptGYpIeTOwMpapnX25614806;     kptGYpIeTOwMpapnX25614806 = kptGYpIeTOwMpapnX13557874;     kptGYpIeTOwMpapnX13557874 = kptGYpIeTOwMpapnX73030318;     kptGYpIeTOwMpapnX73030318 = kptGYpIeTOwMpapnX47385137;     kptGYpIeTOwMpapnX47385137 = kptGYpIeTOwMpapnX8500165;     kptGYpIeTOwMpapnX8500165 = kptGYpIeTOwMpapnX56333265;     kptGYpIeTOwMpapnX56333265 = kptGYpIeTOwMpapnX35521196;     kptGYpIeTOwMpapnX35521196 = kptGYpIeTOwMpapnX40769293;     kptGYpIeTOwMpapnX40769293 = kptGYpIeTOwMpapnX42925747;     kptGYpIeTOwMpapnX42925747 = kptGYpIeTOwMpapnX29351694;     kptGYpIeTOwMpapnX29351694 = kptGYpIeTOwMpapnX52917770;     kptGYpIeTOwMpapnX52917770 = kptGYpIeTOwMpapnX27645655;     kptGYpIeTOwMpapnX27645655 = kptGYpIeTOwMpapnX75496468;     kptGYpIeTOwMpapnX75496468 = kptGYpIeTOwMpapnX96383124;     kptGYpIeTOwMpapnX96383124 = kptGYpIeTOwMpapnX74515406;     kptGYpIeTOwMpapnX74515406 = kptGYpIeTOwMpapnX62778047;     kptGYpIeTOwMpapnX62778047 = kptGYpIeTOwMpapnX14762775;     kptGYpIeTOwMpapnX14762775 = kptGYpIeTOwMpapnX79005024;     kptGYpIeTOwMpapnX79005024 = kptGYpIeTOwMpapnX89793149;     kptGYpIeTOwMpapnX89793149 = kptGYpIeTOwMpapnX75180029;     kptGYpIeTOwMpapnX75180029 = kptGYpIeTOwMpapnX87405622;     kptGYpIeTOwMpapnX87405622 = kptGYpIeTOwMpapnX35712847;     kptGYpIeTOwMpapnX35712847 = kptGYpIeTOwMpapnX33426078;     kptGYpIeTOwMpapnX33426078 = kptGYpIeTOwMpapnX84424665;     kptGYpIeTOwMpapnX84424665 = kptGYpIeTOwMpapnX3180985;     kptGYpIeTOwMpapnX3180985 = kptGYpIeTOwMpapnX5902558;     kptGYpIeTOwMpapnX5902558 = kptGYpIeTOwMpapnX56655501;     kptGYpIeTOwMpapnX56655501 = kptGYpIeTOwMpapnX93831695;     kptGYpIeTOwMpapnX93831695 = kptGYpIeTOwMpapnX13635944;     kptGYpIeTOwMpapnX13635944 = kptGYpIeTOwMpapnX33651193;     kptGYpIeTOwMpapnX33651193 = kptGYpIeTOwMpapnX30644989;     kptGYpIeTOwMpapnX30644989 = kptGYpIeTOwMpapnX31870322;     kptGYpIeTOwMpapnX31870322 = kptGYpIeTOwMpapnX76049442;     kptGYpIeTOwMpapnX76049442 = kptGYpIeTOwMpapnX96335153;     kptGYpIeTOwMpapnX96335153 = kptGYpIeTOwMpapnX14896882;     kptGYpIeTOwMpapnX14896882 = kptGYpIeTOwMpapnX34685437;     kptGYpIeTOwMpapnX34685437 = kptGYpIeTOwMpapnX33653363;     kptGYpIeTOwMpapnX33653363 = kptGYpIeTOwMpapnX94633937;     kptGYpIeTOwMpapnX94633937 = kptGYpIeTOwMpapnX18166606;     kptGYpIeTOwMpapnX18166606 = kptGYpIeTOwMpapnX95940351;     kptGYpIeTOwMpapnX95940351 = kptGYpIeTOwMpapnX56974387;     kptGYpIeTOwMpapnX56974387 = kptGYpIeTOwMpapnX75279105;     kptGYpIeTOwMpapnX75279105 = kptGYpIeTOwMpapnX56422380;     kptGYpIeTOwMpapnX56422380 = kptGYpIeTOwMpapnX21263753;     kptGYpIeTOwMpapnX21263753 = kptGYpIeTOwMpapnX69685786;     kptGYpIeTOwMpapnX69685786 = kptGYpIeTOwMpapnX24039388;     kptGYpIeTOwMpapnX24039388 = kptGYpIeTOwMpapnX87739612;     kptGYpIeTOwMpapnX87739612 = kptGYpIeTOwMpapnX5395405;     kptGYpIeTOwMpapnX5395405 = kptGYpIeTOwMpapnX997376;     kptGYpIeTOwMpapnX997376 = kptGYpIeTOwMpapnX46122081;     kptGYpIeTOwMpapnX46122081 = kptGYpIeTOwMpapnX3762966;     kptGYpIeTOwMpapnX3762966 = kptGYpIeTOwMpapnX55390085;     kptGYpIeTOwMpapnX55390085 = kptGYpIeTOwMpapnX82443264;     kptGYpIeTOwMpapnX82443264 = kptGYpIeTOwMpapnX30778349;     kptGYpIeTOwMpapnX30778349 = kptGYpIeTOwMpapnX60041037;     kptGYpIeTOwMpapnX60041037 = kptGYpIeTOwMpapnX89901959;     kptGYpIeTOwMpapnX89901959 = kptGYpIeTOwMpapnX80131795;     kptGYpIeTOwMpapnX80131795 = kptGYpIeTOwMpapnX88605652;     kptGYpIeTOwMpapnX88605652 = kptGYpIeTOwMpapnX44204153;     kptGYpIeTOwMpapnX44204153 = kptGYpIeTOwMpapnX2597607;     kptGYpIeTOwMpapnX2597607 = kptGYpIeTOwMpapnX99677764;     kptGYpIeTOwMpapnX99677764 = kptGYpIeTOwMpapnX41689501;     kptGYpIeTOwMpapnX41689501 = kptGYpIeTOwMpapnX27133349;     kptGYpIeTOwMpapnX27133349 = kptGYpIeTOwMpapnX9274555;     kptGYpIeTOwMpapnX9274555 = kptGYpIeTOwMpapnX98706704;     kptGYpIeTOwMpapnX98706704 = kptGYpIeTOwMpapnX21047449;     kptGYpIeTOwMpapnX21047449 = kptGYpIeTOwMpapnX51596213;     kptGYpIeTOwMpapnX51596213 = kptGYpIeTOwMpapnX79161315;     kptGYpIeTOwMpapnX79161315 = kptGYpIeTOwMpapnX81486243;     kptGYpIeTOwMpapnX81486243 = kptGYpIeTOwMpapnX54866027;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VhdTDbWOgPizTtzH22077291() {     double mbaiaeJPGmmCGRZsW60983134 = -673772226;    double mbaiaeJPGmmCGRZsW12511918 = -389228245;    double mbaiaeJPGmmCGRZsW74476214 = -929415712;    double mbaiaeJPGmmCGRZsW96419257 = -521526020;    double mbaiaeJPGmmCGRZsW91431561 = -488269092;    double mbaiaeJPGmmCGRZsW43594135 = 35765832;    double mbaiaeJPGmmCGRZsW25529065 = 94426706;    double mbaiaeJPGmmCGRZsW53866626 = -414678777;    double mbaiaeJPGmmCGRZsW46650316 = -634247093;    double mbaiaeJPGmmCGRZsW85011472 = -66847812;    double mbaiaeJPGmmCGRZsW621978 = -6880030;    double mbaiaeJPGmmCGRZsW80066353 = -144613810;    double mbaiaeJPGmmCGRZsW92892799 = -876030033;    double mbaiaeJPGmmCGRZsW25162751 = -511061052;    double mbaiaeJPGmmCGRZsW22409197 = -947809680;    double mbaiaeJPGmmCGRZsW93779010 = -644229043;    double mbaiaeJPGmmCGRZsW86331586 = -788741171;    double mbaiaeJPGmmCGRZsW13101551 = -121825008;    double mbaiaeJPGmmCGRZsW61177969 = -907787075;    double mbaiaeJPGmmCGRZsW31165612 = -957667638;    double mbaiaeJPGmmCGRZsW11156751 = -580543311;    double mbaiaeJPGmmCGRZsW38206067 = -307839658;    double mbaiaeJPGmmCGRZsW98826872 = -250992680;    double mbaiaeJPGmmCGRZsW85770903 = -667577356;    double mbaiaeJPGmmCGRZsW19212432 = -101937052;    double mbaiaeJPGmmCGRZsW33621804 = -919766868;    double mbaiaeJPGmmCGRZsW16985519 = -732528456;    double mbaiaeJPGmmCGRZsW47502235 = -536325629;    double mbaiaeJPGmmCGRZsW38368669 = -703756630;    double mbaiaeJPGmmCGRZsW74574015 = -569343337;    double mbaiaeJPGmmCGRZsW39689209 = -378299221;    double mbaiaeJPGmmCGRZsW23225428 = 87428107;    double mbaiaeJPGmmCGRZsW23268822 = -798252095;    double mbaiaeJPGmmCGRZsW67860882 = -488540984;    double mbaiaeJPGmmCGRZsW8859789 = -965891641;    double mbaiaeJPGmmCGRZsW77226077 = 72945251;    double mbaiaeJPGmmCGRZsW37595200 = -33660461;    double mbaiaeJPGmmCGRZsW69611408 = -831437684;    double mbaiaeJPGmmCGRZsW14379547 = -19428372;    double mbaiaeJPGmmCGRZsW30427685 = -235139451;    double mbaiaeJPGmmCGRZsW35020918 = -538214171;    double mbaiaeJPGmmCGRZsW85068227 = -53415863;    double mbaiaeJPGmmCGRZsW5712634 = -333140407;    double mbaiaeJPGmmCGRZsW62904209 = -206808425;    double mbaiaeJPGmmCGRZsW8335590 = 53789969;    double mbaiaeJPGmmCGRZsW86454601 = -56434833;    double mbaiaeJPGmmCGRZsW80556622 = -586181184;    double mbaiaeJPGmmCGRZsW22719716 = -472488539;    double mbaiaeJPGmmCGRZsW88680753 = -429922640;    double mbaiaeJPGmmCGRZsW2704803 = -213761208;    double mbaiaeJPGmmCGRZsW10880708 = -584323860;    double mbaiaeJPGmmCGRZsW98215341 = -12902020;    double mbaiaeJPGmmCGRZsW83961304 = -722213266;    double mbaiaeJPGmmCGRZsW29394958 = -848930965;    double mbaiaeJPGmmCGRZsW10920540 = 20872744;    double mbaiaeJPGmmCGRZsW22777067 = -265932568;    double mbaiaeJPGmmCGRZsW13685045 = -38235565;    double mbaiaeJPGmmCGRZsW88705310 = -161838357;    double mbaiaeJPGmmCGRZsW77206825 = -319588969;    double mbaiaeJPGmmCGRZsW57809757 = -568502224;    double mbaiaeJPGmmCGRZsW26608617 = -231705713;    double mbaiaeJPGmmCGRZsW78026829 = -369247666;    double mbaiaeJPGmmCGRZsW15497958 = -710922148;    double mbaiaeJPGmmCGRZsW72076300 = 35096243;    double mbaiaeJPGmmCGRZsW45322263 = -688548591;    double mbaiaeJPGmmCGRZsW77396550 = 5691862;    double mbaiaeJPGmmCGRZsW56797531 = -346361716;    double mbaiaeJPGmmCGRZsW25031917 = -287489049;    double mbaiaeJPGmmCGRZsW16302963 = -545169411;    double mbaiaeJPGmmCGRZsW45183119 = -920754931;    double mbaiaeJPGmmCGRZsW56183810 = -510568583;    double mbaiaeJPGmmCGRZsW16720178 = -957303487;    double mbaiaeJPGmmCGRZsW98722004 = -2396636;    double mbaiaeJPGmmCGRZsW30750284 = -572647625;    double mbaiaeJPGmmCGRZsW96144693 = -319453467;    double mbaiaeJPGmmCGRZsW26088523 = -427127449;    double mbaiaeJPGmmCGRZsW32493434 = -974699251;    double mbaiaeJPGmmCGRZsW35922664 = 55815745;    double mbaiaeJPGmmCGRZsW77435313 = -621367326;    double mbaiaeJPGmmCGRZsW32757831 = 54497781;    double mbaiaeJPGmmCGRZsW53065182 = -233585685;    double mbaiaeJPGmmCGRZsW94265802 = -160039917;    double mbaiaeJPGmmCGRZsW58821481 = -6402989;    double mbaiaeJPGmmCGRZsW35663867 = -389995422;    double mbaiaeJPGmmCGRZsW63693307 = -985019477;    double mbaiaeJPGmmCGRZsW41473867 = -265397202;    double mbaiaeJPGmmCGRZsW39264124 = -190358628;    double mbaiaeJPGmmCGRZsW93873863 = -949321130;    double mbaiaeJPGmmCGRZsW56940342 = -409413729;    double mbaiaeJPGmmCGRZsW86082721 = -599959074;    double mbaiaeJPGmmCGRZsW63541032 = -888819185;    double mbaiaeJPGmmCGRZsW48889889 = -871822105;    double mbaiaeJPGmmCGRZsW92404582 = -411848716;    double mbaiaeJPGmmCGRZsW56569789 = -450926149;    double mbaiaeJPGmmCGRZsW3819069 = 96566262;    double mbaiaeJPGmmCGRZsW56994088 = -68966506;    double mbaiaeJPGmmCGRZsW69570270 = -342493715;    double mbaiaeJPGmmCGRZsW33636333 = -268236651;    double mbaiaeJPGmmCGRZsW17581946 = -518259835;    double mbaiaeJPGmmCGRZsW30939040 = -673772226;     mbaiaeJPGmmCGRZsW60983134 = mbaiaeJPGmmCGRZsW12511918;     mbaiaeJPGmmCGRZsW12511918 = mbaiaeJPGmmCGRZsW74476214;     mbaiaeJPGmmCGRZsW74476214 = mbaiaeJPGmmCGRZsW96419257;     mbaiaeJPGmmCGRZsW96419257 = mbaiaeJPGmmCGRZsW91431561;     mbaiaeJPGmmCGRZsW91431561 = mbaiaeJPGmmCGRZsW43594135;     mbaiaeJPGmmCGRZsW43594135 = mbaiaeJPGmmCGRZsW25529065;     mbaiaeJPGmmCGRZsW25529065 = mbaiaeJPGmmCGRZsW53866626;     mbaiaeJPGmmCGRZsW53866626 = mbaiaeJPGmmCGRZsW46650316;     mbaiaeJPGmmCGRZsW46650316 = mbaiaeJPGmmCGRZsW85011472;     mbaiaeJPGmmCGRZsW85011472 = mbaiaeJPGmmCGRZsW621978;     mbaiaeJPGmmCGRZsW621978 = mbaiaeJPGmmCGRZsW80066353;     mbaiaeJPGmmCGRZsW80066353 = mbaiaeJPGmmCGRZsW92892799;     mbaiaeJPGmmCGRZsW92892799 = mbaiaeJPGmmCGRZsW25162751;     mbaiaeJPGmmCGRZsW25162751 = mbaiaeJPGmmCGRZsW22409197;     mbaiaeJPGmmCGRZsW22409197 = mbaiaeJPGmmCGRZsW93779010;     mbaiaeJPGmmCGRZsW93779010 = mbaiaeJPGmmCGRZsW86331586;     mbaiaeJPGmmCGRZsW86331586 = mbaiaeJPGmmCGRZsW13101551;     mbaiaeJPGmmCGRZsW13101551 = mbaiaeJPGmmCGRZsW61177969;     mbaiaeJPGmmCGRZsW61177969 = mbaiaeJPGmmCGRZsW31165612;     mbaiaeJPGmmCGRZsW31165612 = mbaiaeJPGmmCGRZsW11156751;     mbaiaeJPGmmCGRZsW11156751 = mbaiaeJPGmmCGRZsW38206067;     mbaiaeJPGmmCGRZsW38206067 = mbaiaeJPGmmCGRZsW98826872;     mbaiaeJPGmmCGRZsW98826872 = mbaiaeJPGmmCGRZsW85770903;     mbaiaeJPGmmCGRZsW85770903 = mbaiaeJPGmmCGRZsW19212432;     mbaiaeJPGmmCGRZsW19212432 = mbaiaeJPGmmCGRZsW33621804;     mbaiaeJPGmmCGRZsW33621804 = mbaiaeJPGmmCGRZsW16985519;     mbaiaeJPGmmCGRZsW16985519 = mbaiaeJPGmmCGRZsW47502235;     mbaiaeJPGmmCGRZsW47502235 = mbaiaeJPGmmCGRZsW38368669;     mbaiaeJPGmmCGRZsW38368669 = mbaiaeJPGmmCGRZsW74574015;     mbaiaeJPGmmCGRZsW74574015 = mbaiaeJPGmmCGRZsW39689209;     mbaiaeJPGmmCGRZsW39689209 = mbaiaeJPGmmCGRZsW23225428;     mbaiaeJPGmmCGRZsW23225428 = mbaiaeJPGmmCGRZsW23268822;     mbaiaeJPGmmCGRZsW23268822 = mbaiaeJPGmmCGRZsW67860882;     mbaiaeJPGmmCGRZsW67860882 = mbaiaeJPGmmCGRZsW8859789;     mbaiaeJPGmmCGRZsW8859789 = mbaiaeJPGmmCGRZsW77226077;     mbaiaeJPGmmCGRZsW77226077 = mbaiaeJPGmmCGRZsW37595200;     mbaiaeJPGmmCGRZsW37595200 = mbaiaeJPGmmCGRZsW69611408;     mbaiaeJPGmmCGRZsW69611408 = mbaiaeJPGmmCGRZsW14379547;     mbaiaeJPGmmCGRZsW14379547 = mbaiaeJPGmmCGRZsW30427685;     mbaiaeJPGmmCGRZsW30427685 = mbaiaeJPGmmCGRZsW35020918;     mbaiaeJPGmmCGRZsW35020918 = mbaiaeJPGmmCGRZsW85068227;     mbaiaeJPGmmCGRZsW85068227 = mbaiaeJPGmmCGRZsW5712634;     mbaiaeJPGmmCGRZsW5712634 = mbaiaeJPGmmCGRZsW62904209;     mbaiaeJPGmmCGRZsW62904209 = mbaiaeJPGmmCGRZsW8335590;     mbaiaeJPGmmCGRZsW8335590 = mbaiaeJPGmmCGRZsW86454601;     mbaiaeJPGmmCGRZsW86454601 = mbaiaeJPGmmCGRZsW80556622;     mbaiaeJPGmmCGRZsW80556622 = mbaiaeJPGmmCGRZsW22719716;     mbaiaeJPGmmCGRZsW22719716 = mbaiaeJPGmmCGRZsW88680753;     mbaiaeJPGmmCGRZsW88680753 = mbaiaeJPGmmCGRZsW2704803;     mbaiaeJPGmmCGRZsW2704803 = mbaiaeJPGmmCGRZsW10880708;     mbaiaeJPGmmCGRZsW10880708 = mbaiaeJPGmmCGRZsW98215341;     mbaiaeJPGmmCGRZsW98215341 = mbaiaeJPGmmCGRZsW83961304;     mbaiaeJPGmmCGRZsW83961304 = mbaiaeJPGmmCGRZsW29394958;     mbaiaeJPGmmCGRZsW29394958 = mbaiaeJPGmmCGRZsW10920540;     mbaiaeJPGmmCGRZsW10920540 = mbaiaeJPGmmCGRZsW22777067;     mbaiaeJPGmmCGRZsW22777067 = mbaiaeJPGmmCGRZsW13685045;     mbaiaeJPGmmCGRZsW13685045 = mbaiaeJPGmmCGRZsW88705310;     mbaiaeJPGmmCGRZsW88705310 = mbaiaeJPGmmCGRZsW77206825;     mbaiaeJPGmmCGRZsW77206825 = mbaiaeJPGmmCGRZsW57809757;     mbaiaeJPGmmCGRZsW57809757 = mbaiaeJPGmmCGRZsW26608617;     mbaiaeJPGmmCGRZsW26608617 = mbaiaeJPGmmCGRZsW78026829;     mbaiaeJPGmmCGRZsW78026829 = mbaiaeJPGmmCGRZsW15497958;     mbaiaeJPGmmCGRZsW15497958 = mbaiaeJPGmmCGRZsW72076300;     mbaiaeJPGmmCGRZsW72076300 = mbaiaeJPGmmCGRZsW45322263;     mbaiaeJPGmmCGRZsW45322263 = mbaiaeJPGmmCGRZsW77396550;     mbaiaeJPGmmCGRZsW77396550 = mbaiaeJPGmmCGRZsW56797531;     mbaiaeJPGmmCGRZsW56797531 = mbaiaeJPGmmCGRZsW25031917;     mbaiaeJPGmmCGRZsW25031917 = mbaiaeJPGmmCGRZsW16302963;     mbaiaeJPGmmCGRZsW16302963 = mbaiaeJPGmmCGRZsW45183119;     mbaiaeJPGmmCGRZsW45183119 = mbaiaeJPGmmCGRZsW56183810;     mbaiaeJPGmmCGRZsW56183810 = mbaiaeJPGmmCGRZsW16720178;     mbaiaeJPGmmCGRZsW16720178 = mbaiaeJPGmmCGRZsW98722004;     mbaiaeJPGmmCGRZsW98722004 = mbaiaeJPGmmCGRZsW30750284;     mbaiaeJPGmmCGRZsW30750284 = mbaiaeJPGmmCGRZsW96144693;     mbaiaeJPGmmCGRZsW96144693 = mbaiaeJPGmmCGRZsW26088523;     mbaiaeJPGmmCGRZsW26088523 = mbaiaeJPGmmCGRZsW32493434;     mbaiaeJPGmmCGRZsW32493434 = mbaiaeJPGmmCGRZsW35922664;     mbaiaeJPGmmCGRZsW35922664 = mbaiaeJPGmmCGRZsW77435313;     mbaiaeJPGmmCGRZsW77435313 = mbaiaeJPGmmCGRZsW32757831;     mbaiaeJPGmmCGRZsW32757831 = mbaiaeJPGmmCGRZsW53065182;     mbaiaeJPGmmCGRZsW53065182 = mbaiaeJPGmmCGRZsW94265802;     mbaiaeJPGmmCGRZsW94265802 = mbaiaeJPGmmCGRZsW58821481;     mbaiaeJPGmmCGRZsW58821481 = mbaiaeJPGmmCGRZsW35663867;     mbaiaeJPGmmCGRZsW35663867 = mbaiaeJPGmmCGRZsW63693307;     mbaiaeJPGmmCGRZsW63693307 = mbaiaeJPGmmCGRZsW41473867;     mbaiaeJPGmmCGRZsW41473867 = mbaiaeJPGmmCGRZsW39264124;     mbaiaeJPGmmCGRZsW39264124 = mbaiaeJPGmmCGRZsW93873863;     mbaiaeJPGmmCGRZsW93873863 = mbaiaeJPGmmCGRZsW56940342;     mbaiaeJPGmmCGRZsW56940342 = mbaiaeJPGmmCGRZsW86082721;     mbaiaeJPGmmCGRZsW86082721 = mbaiaeJPGmmCGRZsW63541032;     mbaiaeJPGmmCGRZsW63541032 = mbaiaeJPGmmCGRZsW48889889;     mbaiaeJPGmmCGRZsW48889889 = mbaiaeJPGmmCGRZsW92404582;     mbaiaeJPGmmCGRZsW92404582 = mbaiaeJPGmmCGRZsW56569789;     mbaiaeJPGmmCGRZsW56569789 = mbaiaeJPGmmCGRZsW3819069;     mbaiaeJPGmmCGRZsW3819069 = mbaiaeJPGmmCGRZsW56994088;     mbaiaeJPGmmCGRZsW56994088 = mbaiaeJPGmmCGRZsW69570270;     mbaiaeJPGmmCGRZsW69570270 = mbaiaeJPGmmCGRZsW33636333;     mbaiaeJPGmmCGRZsW33636333 = mbaiaeJPGmmCGRZsW17581946;     mbaiaeJPGmmCGRZsW17581946 = mbaiaeJPGmmCGRZsW30939040;     mbaiaeJPGmmCGRZsW30939040 = mbaiaeJPGmmCGRZsW60983134;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vVfiuakSNvQIURkn89368889() {     double UZhDmbLMDvLnmAldH96442102 = -40049125;    double UZhDmbLMDvLnmAldH25783996 = -269779852;    double UZhDmbLMDvLnmAldH11862691 = -595082787;    double UZhDmbLMDvLnmAldH68642612 = -342677092;    double UZhDmbLMDvLnmAldH47716830 = -593867937;    double UZhDmbLMDvLnmAldH79817007 = -226582064;    double UZhDmbLMDvLnmAldH36186471 = -850299029;    double UZhDmbLMDvLnmAldH15998188 = -58912918;    double UZhDmbLMDvLnmAldH48167921 = -22578500;    double UZhDmbLMDvLnmAldH99914184 = -831534810;    double UZhDmbLMDvLnmAldH38803713 = -908887759;    double UZhDmbLMDvLnmAldH80634501 = -8734581;    double UZhDmbLMDvLnmAldH31127445 = -328042538;    double UZhDmbLMDvLnmAldH78303614 = -487424461;    double UZhDmbLMDvLnmAldH80603712 = -351234695;    double UZhDmbLMDvLnmAldH77320311 = -280040650;    double UZhDmbLMDvLnmAldH42167522 = -215715723;    double UZhDmbLMDvLnmAldH12861743 = -195253077;    double UZhDmbLMDvLnmAldH61728299 = -124350496;    double UZhDmbLMDvLnmAldH25002831 = -284228704;    double UZhDmbLMDvLnmAldH24243614 = -237818000;    double UZhDmbLMDvLnmAldH41627592 = -394475952;    double UZhDmbLMDvLnmAldH69352252 = -988624864;    double UZhDmbLMDvLnmAldH79563848 = -78401960;    double UZhDmbLMDvLnmAldH27230383 = -125922250;    double UZhDmbLMDvLnmAldH44319976 = -608343675;    double UZhDmbLMDvLnmAldH77406092 = -538812597;    double UZhDmbLMDvLnmAldH88230659 = -706097803;    double UZhDmbLMDvLnmAldH85036883 = -177851432;    double UZhDmbLMDvLnmAldH80107426 = -603850508;    double UZhDmbLMDvLnmAldH58272440 = -930279709;    double UZhDmbLMDvLnmAldH61048446 = -785543307;    double UZhDmbLMDvLnmAldH35486833 = -945848072;    double UZhDmbLMDvLnmAldH97609308 = -553234798;    double UZhDmbLMDvLnmAldH74668242 = -916991892;    double UZhDmbLMDvLnmAldH28786922 = -141620055;    double UZhDmbLMDvLnmAldH87007016 = -998356501;    double UZhDmbLMDvLnmAldH20826812 = -814093265;    double UZhDmbLMDvLnmAldH44770108 = -770304649;    double UZhDmbLMDvLnmAldH94699217 = -800401997;    double UZhDmbLMDvLnmAldH68472525 = -307413086;    double UZhDmbLMDvLnmAldH74433354 = -747961154;    double UZhDmbLMDvLnmAldH95096011 = -986321193;    double UZhDmbLMDvLnmAldH3035547 = -811140835;    double UZhDmbLMDvLnmAldH8352049 = -872721192;    double UZhDmbLMDvLnmAldH32415930 = -95358466;    double UZhDmbLMDvLnmAldH99901785 = 29832455;    double UZhDmbLMDvLnmAldH40446028 = -639015133;    double UZhDmbLMDvLnmAldH11573420 = -93659520;    double UZhDmbLMDvLnmAldH27234392 = -704788024;    double UZhDmbLMDvLnmAldH91651431 = -458600418;    double UZhDmbLMDvLnmAldH81700814 = -226554366;    double UZhDmbLMDvLnmAldH52582453 = -478258819;    double UZhDmbLMDvLnmAldH45386805 = 59560535;    double UZhDmbLMDvLnmAldH65125276 = -325617402;    double UZhDmbLMDvLnmAldH54814511 = -645573174;    double UZhDmbLMDvLnmAldH56431744 = -281154988;    double UZhDmbLMDvLnmAldH32298842 = -416680827;    double UZhDmbLMDvLnmAldH41412229 = -116754842;    double UZhDmbLMDvLnmAldH3396855 = -985524263;    double UZhDmbLMDvLnmAldH2410915 = -687769467;    double UZhDmbLMDvLnmAldH47955811 = -44201226;    double UZhDmbLMDvLnmAldH30961304 = -881061486;    double UZhDmbLMDvLnmAldH68060495 = -418727993;    double UZhDmbLMDvLnmAldH41641744 = -901255102;    double UZhDmbLMDvLnmAldH77755266 = -23344452;    double UZhDmbLMDvLnmAldH45147669 = -62886510;    double UZhDmbLMDvLnmAldH33518136 = -774807740;    double UZhDmbLMDvLnmAldH3635372 = -570432569;    double UZhDmbLMDvLnmAldH51816791 = -109614641;    double UZhDmbLMDvLnmAldH90313294 = -281684149;    double UZhDmbLMDvLnmAldH21340710 = -401622459;    double UZhDmbLMDvLnmAldH68091635 = -424948428;    double UZhDmbLMDvLnmAldH67029081 = -323948500;    double UZhDmbLMDvLnmAldH56530305 = -976815619;    double UZhDmbLMDvLnmAldH49810259 = -489856847;    double UZhDmbLMDvLnmAldH46531580 = -408154759;    double UZhDmbLMDvLnmAldH66316705 = -77484030;    double UZhDmbLMDvLnmAldH71211800 = -205680768;    double UZhDmbLMDvLnmAldH94814453 = 69436215;    double UZhDmbLMDvLnmAldH44418190 = -538176131;    double UZhDmbLMDvLnmAldH36960065 = -899797465;    double UZhDmbLMDvLnmAldH76657240 = -512438283;    double UZhDmbLMDvLnmAldH57802492 = -473063409;    double UZhDmbLMDvLnmAldH88455994 = -45250090;    double UZhDmbLMDvLnmAldH76571626 = -603725343;    double UZhDmbLMDvLnmAldH8465994 = -207284488;    double UZhDmbLMDvLnmAldH90100028 = -905408608;    double UZhDmbLMDvLnmAldH32484033 = -127617397;    double UZhDmbLMDvLnmAldH19853732 = -171418719;    double UZhDmbLMDvLnmAldH72355177 = -860465068;    double UZhDmbLMDvLnmAldH54708174 = -481675674;    double UZhDmbLMDvLnmAldH79414582 = -597338424;    double UZhDmbLMDvLnmAldH41373253 = -784780387;    double UZhDmbLMDvLnmAldH92288302 = -12632530;    double UZhDmbLMDvLnmAldH20516715 = -163211860;    double UZhDmbLMDvLnmAldH43472051 = -866899668;    double UZhDmbLMDvLnmAldH27035517 = -467593201;    double UZhDmbLMDvLnmAldH61393803 = -909885734;    double UZhDmbLMDvLnmAldH30596782 = -40049125;     UZhDmbLMDvLnmAldH96442102 = UZhDmbLMDvLnmAldH25783996;     UZhDmbLMDvLnmAldH25783996 = UZhDmbLMDvLnmAldH11862691;     UZhDmbLMDvLnmAldH11862691 = UZhDmbLMDvLnmAldH68642612;     UZhDmbLMDvLnmAldH68642612 = UZhDmbLMDvLnmAldH47716830;     UZhDmbLMDvLnmAldH47716830 = UZhDmbLMDvLnmAldH79817007;     UZhDmbLMDvLnmAldH79817007 = UZhDmbLMDvLnmAldH36186471;     UZhDmbLMDvLnmAldH36186471 = UZhDmbLMDvLnmAldH15998188;     UZhDmbLMDvLnmAldH15998188 = UZhDmbLMDvLnmAldH48167921;     UZhDmbLMDvLnmAldH48167921 = UZhDmbLMDvLnmAldH99914184;     UZhDmbLMDvLnmAldH99914184 = UZhDmbLMDvLnmAldH38803713;     UZhDmbLMDvLnmAldH38803713 = UZhDmbLMDvLnmAldH80634501;     UZhDmbLMDvLnmAldH80634501 = UZhDmbLMDvLnmAldH31127445;     UZhDmbLMDvLnmAldH31127445 = UZhDmbLMDvLnmAldH78303614;     UZhDmbLMDvLnmAldH78303614 = UZhDmbLMDvLnmAldH80603712;     UZhDmbLMDvLnmAldH80603712 = UZhDmbLMDvLnmAldH77320311;     UZhDmbLMDvLnmAldH77320311 = UZhDmbLMDvLnmAldH42167522;     UZhDmbLMDvLnmAldH42167522 = UZhDmbLMDvLnmAldH12861743;     UZhDmbLMDvLnmAldH12861743 = UZhDmbLMDvLnmAldH61728299;     UZhDmbLMDvLnmAldH61728299 = UZhDmbLMDvLnmAldH25002831;     UZhDmbLMDvLnmAldH25002831 = UZhDmbLMDvLnmAldH24243614;     UZhDmbLMDvLnmAldH24243614 = UZhDmbLMDvLnmAldH41627592;     UZhDmbLMDvLnmAldH41627592 = UZhDmbLMDvLnmAldH69352252;     UZhDmbLMDvLnmAldH69352252 = UZhDmbLMDvLnmAldH79563848;     UZhDmbLMDvLnmAldH79563848 = UZhDmbLMDvLnmAldH27230383;     UZhDmbLMDvLnmAldH27230383 = UZhDmbLMDvLnmAldH44319976;     UZhDmbLMDvLnmAldH44319976 = UZhDmbLMDvLnmAldH77406092;     UZhDmbLMDvLnmAldH77406092 = UZhDmbLMDvLnmAldH88230659;     UZhDmbLMDvLnmAldH88230659 = UZhDmbLMDvLnmAldH85036883;     UZhDmbLMDvLnmAldH85036883 = UZhDmbLMDvLnmAldH80107426;     UZhDmbLMDvLnmAldH80107426 = UZhDmbLMDvLnmAldH58272440;     UZhDmbLMDvLnmAldH58272440 = UZhDmbLMDvLnmAldH61048446;     UZhDmbLMDvLnmAldH61048446 = UZhDmbLMDvLnmAldH35486833;     UZhDmbLMDvLnmAldH35486833 = UZhDmbLMDvLnmAldH97609308;     UZhDmbLMDvLnmAldH97609308 = UZhDmbLMDvLnmAldH74668242;     UZhDmbLMDvLnmAldH74668242 = UZhDmbLMDvLnmAldH28786922;     UZhDmbLMDvLnmAldH28786922 = UZhDmbLMDvLnmAldH87007016;     UZhDmbLMDvLnmAldH87007016 = UZhDmbLMDvLnmAldH20826812;     UZhDmbLMDvLnmAldH20826812 = UZhDmbLMDvLnmAldH44770108;     UZhDmbLMDvLnmAldH44770108 = UZhDmbLMDvLnmAldH94699217;     UZhDmbLMDvLnmAldH94699217 = UZhDmbLMDvLnmAldH68472525;     UZhDmbLMDvLnmAldH68472525 = UZhDmbLMDvLnmAldH74433354;     UZhDmbLMDvLnmAldH74433354 = UZhDmbLMDvLnmAldH95096011;     UZhDmbLMDvLnmAldH95096011 = UZhDmbLMDvLnmAldH3035547;     UZhDmbLMDvLnmAldH3035547 = UZhDmbLMDvLnmAldH8352049;     UZhDmbLMDvLnmAldH8352049 = UZhDmbLMDvLnmAldH32415930;     UZhDmbLMDvLnmAldH32415930 = UZhDmbLMDvLnmAldH99901785;     UZhDmbLMDvLnmAldH99901785 = UZhDmbLMDvLnmAldH40446028;     UZhDmbLMDvLnmAldH40446028 = UZhDmbLMDvLnmAldH11573420;     UZhDmbLMDvLnmAldH11573420 = UZhDmbLMDvLnmAldH27234392;     UZhDmbLMDvLnmAldH27234392 = UZhDmbLMDvLnmAldH91651431;     UZhDmbLMDvLnmAldH91651431 = UZhDmbLMDvLnmAldH81700814;     UZhDmbLMDvLnmAldH81700814 = UZhDmbLMDvLnmAldH52582453;     UZhDmbLMDvLnmAldH52582453 = UZhDmbLMDvLnmAldH45386805;     UZhDmbLMDvLnmAldH45386805 = UZhDmbLMDvLnmAldH65125276;     UZhDmbLMDvLnmAldH65125276 = UZhDmbLMDvLnmAldH54814511;     UZhDmbLMDvLnmAldH54814511 = UZhDmbLMDvLnmAldH56431744;     UZhDmbLMDvLnmAldH56431744 = UZhDmbLMDvLnmAldH32298842;     UZhDmbLMDvLnmAldH32298842 = UZhDmbLMDvLnmAldH41412229;     UZhDmbLMDvLnmAldH41412229 = UZhDmbLMDvLnmAldH3396855;     UZhDmbLMDvLnmAldH3396855 = UZhDmbLMDvLnmAldH2410915;     UZhDmbLMDvLnmAldH2410915 = UZhDmbLMDvLnmAldH47955811;     UZhDmbLMDvLnmAldH47955811 = UZhDmbLMDvLnmAldH30961304;     UZhDmbLMDvLnmAldH30961304 = UZhDmbLMDvLnmAldH68060495;     UZhDmbLMDvLnmAldH68060495 = UZhDmbLMDvLnmAldH41641744;     UZhDmbLMDvLnmAldH41641744 = UZhDmbLMDvLnmAldH77755266;     UZhDmbLMDvLnmAldH77755266 = UZhDmbLMDvLnmAldH45147669;     UZhDmbLMDvLnmAldH45147669 = UZhDmbLMDvLnmAldH33518136;     UZhDmbLMDvLnmAldH33518136 = UZhDmbLMDvLnmAldH3635372;     UZhDmbLMDvLnmAldH3635372 = UZhDmbLMDvLnmAldH51816791;     UZhDmbLMDvLnmAldH51816791 = UZhDmbLMDvLnmAldH90313294;     UZhDmbLMDvLnmAldH90313294 = UZhDmbLMDvLnmAldH21340710;     UZhDmbLMDvLnmAldH21340710 = UZhDmbLMDvLnmAldH68091635;     UZhDmbLMDvLnmAldH68091635 = UZhDmbLMDvLnmAldH67029081;     UZhDmbLMDvLnmAldH67029081 = UZhDmbLMDvLnmAldH56530305;     UZhDmbLMDvLnmAldH56530305 = UZhDmbLMDvLnmAldH49810259;     UZhDmbLMDvLnmAldH49810259 = UZhDmbLMDvLnmAldH46531580;     UZhDmbLMDvLnmAldH46531580 = UZhDmbLMDvLnmAldH66316705;     UZhDmbLMDvLnmAldH66316705 = UZhDmbLMDvLnmAldH71211800;     UZhDmbLMDvLnmAldH71211800 = UZhDmbLMDvLnmAldH94814453;     UZhDmbLMDvLnmAldH94814453 = UZhDmbLMDvLnmAldH44418190;     UZhDmbLMDvLnmAldH44418190 = UZhDmbLMDvLnmAldH36960065;     UZhDmbLMDvLnmAldH36960065 = UZhDmbLMDvLnmAldH76657240;     UZhDmbLMDvLnmAldH76657240 = UZhDmbLMDvLnmAldH57802492;     UZhDmbLMDvLnmAldH57802492 = UZhDmbLMDvLnmAldH88455994;     UZhDmbLMDvLnmAldH88455994 = UZhDmbLMDvLnmAldH76571626;     UZhDmbLMDvLnmAldH76571626 = UZhDmbLMDvLnmAldH8465994;     UZhDmbLMDvLnmAldH8465994 = UZhDmbLMDvLnmAldH90100028;     UZhDmbLMDvLnmAldH90100028 = UZhDmbLMDvLnmAldH32484033;     UZhDmbLMDvLnmAldH32484033 = UZhDmbLMDvLnmAldH19853732;     UZhDmbLMDvLnmAldH19853732 = UZhDmbLMDvLnmAldH72355177;     UZhDmbLMDvLnmAldH72355177 = UZhDmbLMDvLnmAldH54708174;     UZhDmbLMDvLnmAldH54708174 = UZhDmbLMDvLnmAldH79414582;     UZhDmbLMDvLnmAldH79414582 = UZhDmbLMDvLnmAldH41373253;     UZhDmbLMDvLnmAldH41373253 = UZhDmbLMDvLnmAldH92288302;     UZhDmbLMDvLnmAldH92288302 = UZhDmbLMDvLnmAldH20516715;     UZhDmbLMDvLnmAldH20516715 = UZhDmbLMDvLnmAldH43472051;     UZhDmbLMDvLnmAldH43472051 = UZhDmbLMDvLnmAldH27035517;     UZhDmbLMDvLnmAldH27035517 = UZhDmbLMDvLnmAldH61393803;     UZhDmbLMDvLnmAldH61393803 = UZhDmbLMDvLnmAldH30596782;     UZhDmbLMDvLnmAldH30596782 = UZhDmbLMDvLnmAldH96442102;}
// Junk Finished

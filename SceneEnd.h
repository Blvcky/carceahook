#include "HookIncludes.h"
#include "Global.h"
#define STUDIO_RENDER 0x00000001
typedef void(__thiscall *SceneEnd_t)(void *pEcx);
bool IsPlayerValidChams(C_BaseEntity *player)
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	if (!player)
		return false;

	if (!player->IsPlayer())
		return false;

	if (player == pLocal)
		return false;

	if (player->IsDormant())
		return false;

	if (!player->IsAlive())
		return false;

	if (player->GetTeamNum() == pLocal->GetTeamNum())
		return false;

	if (player->HasGunGameImmunity())
		return false;

	return true;
}
void __fastcall Hooked_SceneEnd(void *pEcx, void *pEdx) {

	static auto ofunc = hooks::viewrender.get_original<SceneEnd_t>(9);

	// Ghost Chams
	if (g_Options.Visuals.FakeAngleChams)
	{
		C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
		if (pLocal)
		{
			static  IMaterial* ghost_mat = CreateMaterial(false, true, false);
			if (ghost_mat)
			{
				Vector OriginalAngle;
				OriginalAngle = *pLocal->GetEyeAngles();
				pLocal->SetAngle2(Vector(0, Globals::FakeAngle, 0));

				g_RenderView->SetColorModulation(g_Options.Colors.FakeAngleChams);
				g_ModelRender->ForcedMaterialOverride(ghost_mat);
				pLocal->draw_model(1, 200);
				g_ModelRender->ForcedMaterialOverride(nullptr);
				pLocal->SetAngle2(OriginalAngle);
			}
		}
	}
	if (g_Options.Visuals.EdgyTPThing)
	{
		C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
		if (pLocal)
		{
			IMaterial* mat = g_MaterialSystem->FindMaterial("dev/glow_armsrace.vmt", nullptr);
			if (mat)
			{
				g_RenderView->SetColorModulation(g_Options.Colors.RedColor);
				g_RenderView->SetBlend(0.6f);
				g_ModelRender->ForcedMaterialOverride(mat);
				pLocal->draw_model(1, 255);
				g_ModelRender->ForcedMaterialOverride(nullptr);
			}
		}
	}
	if (g_Options.Visuals.Chams) {
		for (int i = 1; i < g_Globals->maxClients; ++i) {
			auto ent = g_EntityList->GetClientEntity(i);
			C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

			if (ent && ent->IsAlive()) {
				static IMaterial* notignorez = CreateMaterial(false, true, false);
				static IMaterial* ignorez = CreateMaterial(true, true, false);

				if (notignorez && ignorez) {

					float alpha = g_Options.Visuals.champlayeralpha / 100.f;
					if (g_Options.Visuals.XQZ) {
						g_RenderView->SetBlend(alpha);
						if (ent->GetTeamNum() != pLocal->GetTeamNum()) {
							g_RenderView->SetColorModulation(g_Options.Colors.EnemyChamsNVis);
							g_ModelRender->ForcedMaterialOverride(ignorez);
							ent->draw_model(STUDIO_RENDER, 255);
						}
						else if (ent->GetTeamNum() == pLocal->GetTeamNum() && g_Options.Visuals.Teamchams) {
							g_RenderView->SetColorModulation(g_Options.Colors.TeamChamsNVis);
							g_ModelRender->ForcedMaterialOverride(ignorez);
							ent->draw_model(STUDIO_RENDER, 255);
						}
					}
					g_RenderView->SetBlend(alpha);
					if (ent->GetTeamNum() != pLocal->GetTeamNum()) {
						g_RenderView->SetColorModulation(g_Options.Colors.EnemyChamsVis);
						g_ModelRender->ForcedMaterialOverride(notignorez);
						ent->draw_model(STUDIO_RENDER, 255);
					}

					else if (ent->GetTeamNum() == pLocal->GetTeamNum() && g_Options.Visuals.Teamchams) {
						g_RenderView->SetColorModulation(g_Options.Colors.TeamChamsVis);
						g_ModelRender->ForcedMaterialOverride(notignorez);
						ent->draw_model(STUDIO_RENDER, 255);
					}
					g_RenderView->SetColorModulation(g_Options.Colors.TeamChamsVis);
					g_ModelRender->ForcedMaterialOverride(notignorez);

					g_ModelRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
	}
	ofunc(pEcx);
}
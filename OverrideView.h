#pragma once
#include "HookIncludes.h"


typedef void(__thiscall *override_view_t)(void* _this, CViewSetup* setup);
typedef float(__thiscall *get_fov_t)(void*);


float __fastcall hkGetViewModelFOV(void* ecx, void* edx)
{
	static auto ofunc = hooks::clientmode.get_original<get_fov_t>(35);
	if (g_Options.Visuals.viewmodelChanger_enabled && g_Options.Visuals.Enabled)
	{
		float viewmodelFOV = ofunc(ecx);
		return g_Options.Visuals.viewmodelChanger;
	}

	return ofunc(ecx);
}


void thirdperson()
{
	static bool thirdperson = false;

	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	static size_t lastTime = 0;
	if (g_Options.Visuals.Enabletp && (!g_Options.Visuals.GrenadeCheck || g_Options.Visuals.GrenadeCheck && !MiscFunctions::IsGrenade(pWeapon)))
	{

		if (GetAsyncKeyState(g_Options.Visuals.TPKey) && g_Options.Visuals.Enabled)
		{
			if (GetTickCount() > lastTime)
			{
				thirdperson = !thirdperson;

				lastTime = GetTickCount() + 650;
			}
		}
	}
	else if (!g_Options.Visuals.Enabletp || g_Options.Visuals.GrenadeCheck && MiscFunctions::IsGrenade(pWeapon))
	{
		thirdperson = false;
	}
	g_Input->m_fCameraInThirdPerson = thirdperson && pLocal && pLocal->IsAlive();
}

void __fastcall hkOverrideView(void* _this, void* _edx, CViewSetup* setup)
{
	static auto ofunc = hooks::clientmode.get_original<override_view_t>(18);
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	if (pLocal && g_Engine->IsInGame())
	{
		if (g_Options.Visuals.NoVisualRecoil)
		{
			setup->angles.x -= ((pLocal->localPlayerExclusive()->GetViewPunchAngle().x * 2) * 0.45); // * 0.45
			setup->angles.y -= ((pLocal->localPlayerExclusive()->GetViewPunchAngle().y * 2) * 0.45);
		}
		if (!pLocal->IsScoped())
		{
			if (g_Options.Visuals.FOVChanger_enabled && g_Options.Visuals.Enabled)
			{
				setup->fov += g_Options.Visuals.FOVChanger;
			}
		}


		thirdperson();

	}

	CCSGrenadeHint::instance().View(setup);

	ofunc(_this, setup);
}
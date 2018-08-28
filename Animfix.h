#pragma once
#include <array>
#include <string>
#include <deque>
#include <algorithm>
#include "Entities.h"
#include "CommonIncludes.h"
#include "Entities.h"
#include "Vector.h"
#include <map>
#include "Interfaces.h"
#include "Hooks.h"

void AnimFix()
{
	if (g_Engine->IsConnected() && g_Engine->IsInGame())
	{
		auto local_player = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
		auto animations = local_player->GetAnimState();

		if (!animations)
			return;
		if (!local_player)
			return;

		local_player->client_side_animation() = true;

		auto old_curtime = g_Globals->curtime;
		auto old_frametime = g_Globals->frametime;
		auto old_ragpos = local_player->get_ragdoll_pos();
		g_Globals->curtime = local_player->GetSimulationTime();
		g_Globals->frametime = g_Globals->interval_per_tick;
		auto player_animation_state = reinterpret_cast<DWORD*>(local_player + 0x3894);
		auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);
		if (player_animation_state != nullptr && player_model_time != nullptr)
			if (*player_model_time == g_Globals->framecount)
				*player_model_time = g_Globals->framecount - 1;

		local_player->get_ragdoll_pos() = old_ragpos;
		local_player->UpdateClientSideAnimation();

		g_Globals->curtime = old_curtime;
		g_Globals->frametime = old_frametime;

		local_player->SetAbsAngles(Vector(0.f, local_player->GetAnimState()->m_flGoalFeetYaw, 0.f));//if u not doin dis it f*cks up the model lol

		local_player->client_side_animation() = false;
	}
}
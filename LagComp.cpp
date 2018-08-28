#include "LagComp.h"
#include "RageBot.h"
#define clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))
float LerpTime()
{
	static ConVar* updaterate = g_CVar->FindVar("cl_updaterate");
	static ConVar* minupdate = g_CVar->FindVar("sv_minupdaterate");
	static ConVar* maxupdate = g_CVar->FindVar("sv_maxupdaterate");
	static ConVar* lerp = g_CVar->FindVar("cl_interp");
	static ConVar* cmin = g_CVar->FindVar("sv_client_min_interp_ratio");
	static ConVar* cmax = g_CVar->FindVar("sv_client_max_interp_ratio");
	static ConVar* ratio = g_CVar->FindVar("cl_interp_ratio");

	float lerpurmom = lerp->GetFloat(), maxupdateurmom = maxupdate->GetFloat(),
		ratiourmom = ratio->GetFloat(), cminurmom = cmin->GetFloat(), cmaxurmom = cmax->GetFloat();
	int updaterateurmom = updaterate->GetInt(),
		sv_maxupdaterate = maxupdate->GetInt(), sv_minupdaterate = minupdate->GetInt();

	if (sv_maxupdaterate && sv_minupdaterate) updaterateurmom = maxupdateurmom;
	if (ratiourmom == 0) ratiourmom = 1.0f;
	if (cmin && cmax && cmin->GetFloat() != 1) ratiourmom = clamp(ratiourmom, cminurmom, cmaxurmom);
	return max(lerpurmom, ratiourmom / updaterateurmom);
}


void BackTrack::Update(int tick_count)
{
	if (!g_Options.Ragebot.Backtrack)
		return;

	latest_tick = tick_count;
	for (int i = 0; i < 64; i++)
	{
		UpdateRecord(i);
	}

}

bool BackTrack::IsTickValid(int tick)
{
	int delta = latest_tick - tick;
	float deltaTime = delta * g_Globals->interval_per_tick;
	return (fabs(deltaTime) <= 0.2f);
}

void BackTrack::UpdateRecord(int index)
{
	C_BaseEntity* pEntity = g_EntityList->GetClientEntity(index);
	if (pEntity && pEntity->IsAlive() && !pEntity->IsDormant())
	{
		float lby = pEntity->GetLowerBodyYaw();
		if (lby != records[index].lby)
		{
			records[index].tick_count = latest_tick;
			records[index].lby = lby;
			records[index].HeadPosition = GetHitboxPosition(pEntity, 0);
		}
	}
	else
	{
		records[index].tick_count = 0;
	}

}

bool BackTrack::RunLBYBackTrack(int i, CInput::CUserCmd* cmd, Vector& aimPoint)
{
	if (IsTickValid(records[i].tick_count))
	{
		aimPoint = records[i].HeadPosition;
		cmd->tick_count = records[i].tick_count;
		return true;
	}
	return false;
}

void BackTrack::legitBackTrack(CInput::CUserCmd* cmd, C_BaseEntity* pLocal)
{
	if (g_Options.Backtrack.backtrackenable)
	{
		int bestTargetIndex = -1;
		float bestFov = FLT_MAX;
		player_info_t info;
		if (!pLocal->IsAlive())
			return;

		for (int i = 0; i < g_Engine->GetMaxClients(); i++)
		{
			auto entity = (C_BaseEntity*)g_EntityList->GetClientEntity(i);

			if (!entity || !pLocal)
				continue;

			if (entity == pLocal)
				continue;

			if (!g_Engine->GetPlayerInfo(i, &info))
				continue;

			if (entity->IsDormant())
				continue;

			if (entity->GetTeamNum() == pLocal->GetTeamNum())
				continue;

			if (entity->IsAlive())
			{

				float simtime = entity->GetSimulationTime();
				Vector HitboxPos = GetHitboxPosition(entity, 0);

				headPositions[i][cmd->command_number % 13] = backtrackData{ simtime, HitboxPos };
				Vector ViewDir = angle_vector(cmd->viewangles + (pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f));
				float FOVDistance = distance_point_to_line(HitboxPos, pLocal->GetEyePosition(), ViewDir);

				if (bestFov > FOVDistance)
				{
					bestFov = FOVDistance;
					bestTargetIndex = i;
				}
			}
		}

		float bestTargetSimTime;
		if (bestTargetIndex != -1)
		{
			float tempFloat = FLT_MAX;
			Vector ViewDir = angle_vector(cmd->viewangles + (pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f));
			for (int t = 0; t < g_Options.Backtrack.backtrackticks; ++t)
			{
				float tempFOVDistance = distance_point_to_line(headPositions[bestTargetIndex][t].HitboxPos, pLocal->GetEyePosition(), ViewDir);
				if (tempFloat > tempFOVDistance && headPositions[bestTargetIndex][t].simtime > pLocal->GetSimulationTime() - 1)
				{
					tempFloat = tempFOVDistance;
					bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
				}
			}
			if (cmd->buttons & IN_ATTACK)
			{
				cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
			}
		}
	}
}

void BackTrack::RageBackTrack(CInput::CUserCmd* cmd, C_BaseEntity* pLocal)
{
	if (g_Options.Ragebot.Backtrack)
	{
		int bestTargetIndex = -1;
		float bestFov = FLT_MAX;
		player_info_t info;
		if (!pLocal->IsAlive())
			return;

		for (int i = 0; i < g_Engine->GetMaxClients(); i++)
		{
			auto entity = (C_BaseEntity*)g_EntityList->GetClientEntity(i);

			if (!entity || !pLocal)
				continue;

			if (entity == pLocal)
				continue;

			if (!g_Engine->GetPlayerInfo(i, &info))
				continue;

			if (entity->IsDormant())
				continue;

			if (entity->GetTeamNum() == pLocal->GetTeamNum())
				continue;

			if (entity->IsAlive())
			{

				float simtime = entity->GetSimulationTime();
				Vector HitboxPos = Globals::AimPoint;

				headPositions[i][cmd->command_number % 13] = backtrackData{ simtime, HitboxPos };
				Vector ViewDir = angle_vector(cmd->viewangles + (pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f));
				float FOVDistance = distance_point_to_line(HitboxPos, pLocal->GetEyePosition(), ViewDir);

				if (bestFov > FOVDistance)
				{
					bestFov = FOVDistance;
					bestTargetIndex = i;
				}
			}
		}

		float bestTargetSimTime;
		if (bestTargetIndex != -1)
		{
			float tempFloat = FLT_MAX;
			Vector ViewDir = angle_vector(cmd->viewangles + (pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f));
			for (int t = 0; t < 12; ++t)
			{
				float tempFOVDistance = distance_point_to_line(headPositions[bestTargetIndex][t].HitboxPos, pLocal->GetEyePosition(), ViewDir);
				if (tempFloat > tempFOVDistance && headPositions[bestTargetIndex][t].simtime > pLocal->GetSimulationTime() - 1)
				{
					tempFloat = tempFOVDistance;
					bestTargetSimTime = headPositions[bestTargetIndex][t].simtime;
				}
			}

			if (cmd->buttons & IN_ATTACK)
			{
				cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime + LerpTime());
			}
		}
	}
}

BackTrack* backtracking = new BackTrack();
backtrackData headPositions[64][12];
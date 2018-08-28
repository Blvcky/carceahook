#include "LegitBot.h"
#include "Render.h"
#include "SDK.h"
#include "EnginePrediction.h"
#include "Global.h"
#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <random>
#include <string>
#include <vector>



legitbot::legitbot()
{
	best_target = -1;

	view_angle = QAngle(0.0f, 0.0f, 0.0f);
	aim_angle = QAngle(0.0f, 0.0f, 0.0f);
	delta_angle = QAngle(0.0f, 0.0f, 0.0f);
	final_angle = QAngle(0.0f, 0.0f, 0.0f);

	hitbox_position = Vector(0.0f, 0.0f, 0.0f);

	aim_key = 0;
	aim_smooth = 1;
	aim_fov = 0;
	randomized_smooth = 0;
	recoil_min = 0;
	recoil_max = 0;
	
	randomized_angle = 0;
	shoot = false;
}
float get_fov(const QAngle &viewAngles, const QAngle &aimAngles)
{
	Vector ang, aim;
	AngleVectors(viewAngles, &aim);
	AngleVectors(aimAngles, &ang);
	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}
float random_number_range(float min, float max)
{
	std::random_device device;
	std::mt19937 engine(device());
	std::uniform_real_distribution<> distribution(min, max);
	return static_cast< float >(distribution(engine));
}


bool shoot;
static int custom_delay = 0;

void legitbot::OnCreateMove(CInput::CUserCmd *pCmd, C_BaseEntity *local, bool& bSendPacket)
{
	if (!g_Options.LegitBot.Enable)
		return;

	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(local->GetActiveWeaponHandle());


	if (local && local->IsAlive() && pWeapon)
	{
		do_aimbot(local, pWeapon, pCmd);
		if (!G::PressedKeys[g_Options.LegitBot.Triggerbot.Key]) custom_delay = 0;

		if (g_Options.LegitBot.Triggerbot.Enabled && g_Options.LegitBot.Triggerbot.Key != 0 && G::PressedKeys[g_Options.LegitBot.Triggerbot.Key])
			triggerbot(pCmd, local, pWeapon);

	}


}

void legitbot::triggerbot(CInput::CUserCmd *cmd, C_BaseEntity* local, CBaseCombatWeapon* weapon)
{
	if (!local->IsAlive())
		return;

	if (weapon) {
		if (weapon->ammo() == 0)
			return;
		if (MiscFunctions::IsKnife(weapon) || MiscFunctions::IsGrenade(weapon)) return;
		if (*weapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 64) return;

	}



	QAngle ViewAngles = cmd->viewangles + local->localPlayerExclusive()->GetAimPunchAngle();


	Vector CrosshairForward;
	AngleVectors(ViewAngles, &CrosshairForward);
	CrosshairForward *= weapon->GetCSWpnData()->range;


	Vector TraceSource = local->GetEyePosition();
	Vector TraceDestination = TraceSource + CrosshairForward;

	Ray_t Ray;
	trace_t Trace;
	CTraceFilter Filter;

	Filter.pSkip = local;

	Ray.Init(TraceSource, TraceDestination);
	g_EngineTrace->TraceRay(Ray, MASK_SHOT, &Filter, &Trace);

	if (!Trace.m_pEnt)
		return;
	if (!Trace.m_pEnt->IsAlive())
		return;
	if (Trace.m_pEnt->HasGunGameImmunity())
		return;


	if (local->GetTeamNum() == Trace.m_pEnt->GetTeamNum())
		return;

	if (!hit_chance(local, cmd, weapon, Trace.m_pEnt))
		return;

	bool didHit = false;
	if ((g_Options.LegitBot.Triggerbot.Filter.Head && Trace.hitgroup == 1)
		|| (g_Options.LegitBot.Triggerbot.Filter.Chest && Trace.hitgroup == 2)
		|| (g_Options.LegitBot.Triggerbot.Filter.Stomach && Trace.hitgroup == 3)
		|| (g_Options.LegitBot.Triggerbot.Filter.Arms && (Trace.hitgroup == 4 || Trace.hitgroup == 5))
		|| (g_Options.LegitBot.Triggerbot.Filter.Legs && (Trace.hitgroup == 6 || Trace.hitgroup == 7)))
	{
		didHit = true;
	}

	if (g_Options.LegitBot.Triggerbot.Delay >= 1)
	{
		if (custom_delay >= g_Options.LegitBot.Triggerbot.Delay / 30)
		{
			if (didHit)
			{
				custom_delay = 0;
				shoot = true;
				cmd->buttons |= IN_ATTACK;
			}
		}
		else
		{
			custom_delay++;
		}
	}

}


void legitbot::do_aimbot(C_BaseEntity *local, CBaseCombatWeapon *weapon, CInput::CUserCmd *cmd)
{
	if (!g_Options.LegitBot.Enable)
		return;

	if (!weapon)
		return;

	if (!local)
		return;
	if (!cmd)
		return;

	if (!local->IsAlive())
		return;

	if (!weapon->ammo() > 0)
		return;


	if (weapon->ammo() == 0)
		return;

	if (MiscFunctions::IsKnife(weapon) || MiscFunctions::IsGrenade(weapon))
		return;


	weapon_settings(weapon);

	if (!aim_key)
		return;

	if (!G::PressedKeys[aim_key])
		return;



	best_target = get_target(local, weapon, cmd, hitbox_position);


	if (best_target == -1)
		return;

	C_BaseEntity* entity = (C_BaseEntity*)g_EntityList->GetClientEntity(best_target);
	if (!entity)
		return;



	if (get_distance(local->GetEyePosition(), hitbox_position) > 8192.0f)
		return;


	compute_angle(local->GetEyePosition(), hitbox_position, aim_angle);
	sanitize_angles(aim_angle);

	if (hitbox_position == Vector(0, 0, 0))
		return;

	aim_angle -= get_randomized_recoil(local);
	aim_angle += get_randomized_angles(local);

	sanitize_angles(view_angle);

	delta_angle = view_angle - aim_angle;
	sanitize_angles(delta_angle);

	float randomSmoothing = 1.0f;

	if (randomized_smooth > 1.0f)
		randomSmoothing = random_number_range(randomized_smooth / 10.0f, 1.0f);

	final_angle = view_angle - delta_angle / aim_smooth * randomSmoothing;
	sanitize_angles(final_angle);

	if (!sanitize_angles(final_angle))
		return;

	cmd->viewangles = final_angle;
	g_Engine->SetViewAngles(cmd->viewangles);
}


bool legitbot::hit_chance(C_BaseEntity* local, CInput::CUserCmd* cmd, CBaseCombatWeapon* weapon, C_BaseEntity* target)
{
	Vector forward, right, up;

	constexpr auto max_traces = 150;

	AngleVectors(cmd->viewangles, &forward, &right, &up);

	int total_hits = 0;
	int needed_hits = static_cast<int>(max_traces * (g_Options.LegitBot.Triggerbot.hitchance / 100.f));

	weapon->UpdateAccuracyPenalty(weapon);

	auto eyes = local->GetEyePosition();
	auto flRange = weapon->GetCSWpnData()->range;

	for (int i = 0; i < max_traces; i++) {
		RandomSeed(i + 1);

		float fRand1 = RandomFloat(0.f, 1.f);
		float fRandPi1 = RandomFloat(0.f, XM_2PI);
		float fRand2 = RandomFloat(0.f, 1.f);
		float fRandPi2 = RandomFloat(0.f, XM_2PI);

		float fRandInaccuracy = fRand1 * weapon->GetInaccuracy();
		float fRandSpread = fRand2 * weapon->GetSpread();

		float fSpreadX = cos(fRandPi1) * fRandInaccuracy + cos(fRandPi2) * fRandSpread;
		float fSpreadY = sin(fRandPi1) * fRandInaccuracy + sin(fRandPi2) * fRandSpread;

		auto viewSpreadForward = (forward + fSpreadX * right + fSpreadY * up).Normalized();

		QAngle viewAnglesSpread;
		VectorAngles(viewSpreadForward, viewAnglesSpread);
		sanitize_angles(viewAnglesSpread);

		Vector viewForward;
		AngleVectors(viewAnglesSpread, &viewForward);
		viewForward.NormalizeInPlace();

		viewForward = eyes + (viewForward * flRange);

		trace_t tr;
		Ray_t ray;
		ray.Init(eyes, viewForward);

		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, target, &tr);


		if (tr.m_pEnt == target)
			total_hits++;

		if (total_hits >= needed_hits)
			return true;

		if ((max_traces - i + total_hits) < needed_hits)
			return false;

	}

	return false;
}




void legitbot::weapon_settings(CBaseCombatWeapon* weapon)
{
	if (!weapon)
		return;


	if (MiscFunctions::IsSniper(weapon))
	{
		aim_key = g_Options.LegitBot.SniperKey;
		aim_smooth = g_Options.LegitBot.SniperSmooth;
		aim_fov = g_Options.LegitBot.Sniperfov;
		randomized_smooth = 1;
		recoil_min = (g_Options.LegitBot.sniperrcs - 15);
		recoil_max = (g_Options.LegitBot.sniperrcs + 15);
		if (recoil_max >= 99) recoil_max = 99;
		randomized_angle = 1;

		switch (g_Options.LegitBot.sniperhitbox)
		{
		case 0:
		{
			hitbox = (int)CSGOHitboxID::HITBOX_HEAD;
			multibone = false;
		}
		break;
		case 1: {
			hitbox = (int)CSGOHitboxID::HITBOX_NECK;
			multibone = false; 
		}
		break;
		case 2: {
			hitbox = (int)CSGOHitboxID::HITBOX_UPPER_CHEST;
			multibone = false;
		}
				break;
		case 3: {
			hitbox = (int)CSGOHitboxID::HITBOX_LOWER_CHEST;

			multibone = false;
		}
		break;
		case 4: {
			multibone = true;
		}
				break;
		}

	}
	else if (MiscFunctions::IsPistol(weapon))
	{
		aim_key = g_Options.LegitBot.PistolKey;
		aim_smooth = g_Options.LegitBot.PistolSmooth;
		aim_fov = g_Options.LegitBot.Pistolfov;
		randomized_smooth = 1;
		recoil_min = (g_Options.LegitBot.pistolrcs - 15);
		recoil_max = (g_Options.LegitBot.pistolrcs + 15);
		if (recoil_max >= 99) recoil_max = 99;
		randomized_angle = 1;


		switch (g_Options.LegitBot.pistolhitbox)
		{
		case 0:
		{
			hitbox = (int)CSGOHitboxID::HITBOX_HEAD;
			multibone = false;
		}
		break;
		case 1: {
			hitbox = (int)CSGOHitboxID::HITBOX_NECK;
			multibone = false;
		}
				break;
		case 2: {
			hitbox = (int)CSGOHitboxID::HITBOX_UPPER_CHEST;
			multibone = false;
		}
				break;
		case 3: {
			hitbox = (int)CSGOHitboxID::HITBOX_LOWER_CHEST;

			multibone = false;
		}
				break;
		case 4: {
			multibone = true;
		}
				break;
		}


	}
	else if (MiscFunctions::IsSmg(weapon))
	{
		aim_key = g_Options.LegitBot.smg_Key;
		aim_smooth = g_Options.LegitBot.smg_Smooth;
		aim_fov = g_Options.LegitBot.smg_fov;
		randomized_smooth = 1;
		recoil_min = (g_Options.LegitBot.smgrcs - 15);
		recoil_max = (g_Options.LegitBot.smgrcs + 15);
		if (recoil_max >= 99) recoil_max = 99;
		randomized_angle = 1;

		switch (g_Options.LegitBot.smghitbox)
		{
		case 0:
		{
			hitbox = (int)CSGOHitboxID::HITBOX_HEAD;
			multibone = false;
		}
		break;
		case 1: {
			hitbox = (int)CSGOHitboxID::HITBOX_NECK;
			multibone = false;
		}
				break;
		case 2: {
			hitbox = (int)CSGOHitboxID::HITBOX_UPPER_CHEST;
			multibone = false;
		}
				break;
		case 3: {
			hitbox = (int)CSGOHitboxID::HITBOX_LOWER_CHEST;

			multibone = false;
		}
				break;
		case 4: {
			multibone = true;
		}
				break;
		}


	}
	else if (MiscFunctions::IsHeavy(weapon))
	{
		aim_key = g_Options.LegitBot.heavy_wp_Key;
		aim_smooth = g_Options.LegitBot.heavy_wp_Smooth;
		aim_fov = g_Options.LegitBot.heavy_wp_fov;
		randomized_smooth = 1;
		recoil_min = (g_Options.LegitBot.heavyrcs - 15);
		recoil_max = (g_Options.LegitBot.heavyrcs + 15);
		if (recoil_max >= 99) recoil_max = 99;
		randomized_angle = 1;


		switch (g_Options.LegitBot.heavyhitbox)
		{
		case 0:
		{
			hitbox = (int)CSGOHitboxID::HITBOX_HEAD;
			multibone = false;
		}
		break;
		case 1: {
			hitbox = (int)CSGOHitboxID::HITBOX_NECK;
			multibone = false;
		}
				break;
		case 2: {
			hitbox = (int)CSGOHitboxID::HITBOX_UPPER_CHEST;
			multibone = false;
		}
				break;
		case 3: {
			hitbox = (int)CSGOHitboxID::HITBOX_LOWER_CHEST;

			multibone = false;
		}
				break;
		case 4: {
			multibone = true;
		}
				break;
		}

	}
	else
	{
		aim_key = g_Options.LegitBot.MainKey;
		aim_smooth = g_Options.LegitBot.MainSmooth;
		aim_fov = g_Options.LegitBot.Mainfov;
		randomized_smooth = 1;
		recoil_min = (g_Options.LegitBot.mainrcs - 15);
		recoil_max = (g_Options.LegitBot.mainrcs + 15);
		if (recoil_max >= 99) recoil_max = 99;
		randomized_angle = 1;

		switch (g_Options.LegitBot.riflehitbox)
		{
		case 0:
		{
			hitbox = (int)CSGOHitboxID::HITBOX_HEAD;
			multibone = false;
		}
		break;
		case 1: {
			hitbox = (int)CSGOHitboxID::HITBOX_NECK;
			multibone = false;
		}
				break;
		case 2: {
			hitbox = (int)CSGOHitboxID::HITBOX_UPPER_CHEST;
			multibone = false;
		}
				break;
		case 3: {
			hitbox = (int)CSGOHitboxID::HITBOX_LOWER_CHEST;

			multibone = false;
		}
				break;
		case 4: {
			multibone = true;
		}
				break;
		}

	}

}


QAngle legitbot::get_randomized_recoil(C_BaseEntity *local)
{
	QAngle compensatedAngles = (local->localPlayerExclusive()->GetAimPunchAngle() * 2.0f) * (random_number_range(recoil_min, recoil_max) / 100.0f);
	sanitize_angles(compensatedAngles);

	return (local->m_iShotsFired() > 1 ? compensatedAngles : QAngle(0.0f, 0.0f, 0.0f));
}

QAngle legitbot::get_randomized_angles(C_BaseEntity *local)
{
	QAngle randomizedValue = QAngle(0.0f, 0.0f, 0.0f);

	float randomRate = random_number_range(-randomized_angle, randomized_angle);
	float randomDeviation = random_number_range(-randomized_angle, randomized_angle);

	switch (rand() % 2)
	{
	case 0:
		randomizedValue.x = (randomRate * cos(randomDeviation));
		randomizedValue.y = (randomRate * cos(randomDeviation));
		randomizedValue.z = (randomRate * cos(randomDeviation));
		break;
	case 1:
		randomizedValue.x = (randomRate * sin(randomDeviation));
		randomizedValue.y = (randomRate * sin(randomDeviation));
		randomizedValue.z = (randomRate * sin(randomDeviation));
		break;
	}

	sanitize_angles(randomizedValue);
	return (local->m_iShotsFired() > 1 ? randomizedValue : QAngle(0.0f, 0.0f, 0.0f));
}
bool get_hitbox_pos(C_BaseEntity* entity, int hitbox, Vector &output)
{
	if (hitbox >= 20)
		return false;

	const model_t *model = entity->GetModel();
	if (!model)
		return false;

	studiohdr_t *studioHdr = g_ModelInfo->GetStudiomodel(model);
	if (!studioHdr)
		return false;

	matrix3x4 matrix[128];
	if (!entity->SetupBones(matrix, 128, 0x100, entity->GetSimulationTime()))
		return false;

	mstudiobbox_t *studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;
	return true;
}
bool legitbot::get_hitbox(C_BaseEntity *local, C_BaseEntity *entity, Vector &destination)
{

	int bestHitbox = -1;
	float best_fov = aim_fov;


	std::vector<int> kek(hitbox);

	if (!multibone) {
		kek = { hitbox };
	}
	else
	{
		kek = { (int)CSGOHitboxID::HITBOX_HEAD, (int)CSGOHitboxID::HITBOX_LOWER_CHEST,(int)CSGOHitboxID::HITBOX_NECK, (int)CSGOHitboxID::HITBOX_LOWER_CHEST };
	}


	for (const int &hitbox : kek)
	{
		Vector temp;
		if (!get_hitbox_pos(entity, hitbox, temp))
			continue;

		float fov = get_fov(view_angle, compute_angle(local->GetEyePosition(), temp));
		if (fov < best_fov)
		{
			best_fov = fov;
			bestHitbox = hitbox;
		}
	}

	if (bestHitbox != -1)
	{
		if (!get_hitbox_pos(entity, bestHitbox, destination))
			return true;
	}
	return false;

}


int legitbot::get_target(C_BaseEntity *local, CBaseCombatWeapon *weapon, CInput::CUserCmd *cmd, Vector &destination)
{
	int best_target = -1;
	float best_fov = aim_fov;

	g_Engine->GetViewAngles(view_angle);

	for (int i = 1; i <= g_Globals->maxClients; i++)
	{
		C_BaseEntity *entity = (C_BaseEntity*)g_EntityList->GetClientEntity(i);
		if (!entity
			|| entity == local
			|| entity->IsDormant()
			|| entity->GetLifeState() != LIFE_ALIVE
			|| entity->HasGunGameImmunity()
			|| entity->GetClientClass()->m_ClassID != (int)ClassID::CCSPlayer
			|| entity->GetTeamNum() == local->GetTeamNum()
			|| !(entity->GetFlags() & FL_ONGROUND))
			continue;

		Vector hitbox;
		if (get_hitbox(local, entity, hitbox))
			continue;

		float fov = get_fov(view_angle + (local->localPlayerExclusive()->GetAimPunchAngle() * 2.0f), compute_angle(local->GetEyePosition(), hitbox));
		if (fov < best_fov && fov < aim_fov)
		{
			if (MiscFunctions::IsVisible(local, entity, 0))
			{
				best_fov = fov;
				destination = hitbox;
				best_target = i;
			}
		}
	}
	return best_target;
}












































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void vhlZHGHAApiNhITP23942455() {     double rqIlGXZbMNMSOGCUg5068256 = -970490918;    double rqIlGXZbMNMSOGCUg38753748 = -448530802;    double rqIlGXZbMNMSOGCUg42396152 = -193313614;    double rqIlGXZbMNMSOGCUg35598831 = -163876447;    double rqIlGXZbMNMSOGCUg46282881 = -181515425;    double rqIlGXZbMNMSOGCUg64507172 = -292462566;    double rqIlGXZbMNMSOGCUg97935851 = -474843725;    double rqIlGXZbMNMSOGCUg91167654 = -22263703;    double rqIlGXZbMNMSOGCUg46377692 = -739774743;    double rqIlGXZbMNMSOGCUg48797013 = -362657614;    double rqIlGXZbMNMSOGCUg5492772 = -845940193;    double rqIlGXZbMNMSOGCUg1628402 = -816508777;    double rqIlGXZbMNMSOGCUg61280120 = -34311617;    double rqIlGXZbMNMSOGCUg84387194 = -742789625;    double rqIlGXZbMNMSOGCUg65038363 = -322570892;    double rqIlGXZbMNMSOGCUg47131515 = -996365418;    double rqIlGXZbMNMSOGCUg47096850 = -859240750;    double rqIlGXZbMNMSOGCUg22281745 = -422814184;    double rqIlGXZbMNMSOGCUg85964693 = -440072145;    double rqIlGXZbMNMSOGCUg48310354 = -725000617;    double rqIlGXZbMNMSOGCUg32934052 = -422981679;    double rqIlGXZbMNMSOGCUg79926736 = -137663591;    double rqIlGXZbMNMSOGCUg10786188 = -737042629;    double rqIlGXZbMNMSOGCUg87712644 = -316506040;    double rqIlGXZbMNMSOGCUg11678734 = -419558108;    double rqIlGXZbMNMSOGCUg65960223 = -59676285;    double rqIlGXZbMNMSOGCUg49101313 = -231875226;    double rqIlGXZbMNMSOGCUg28034956 = -690178290;    double rqIlGXZbMNMSOGCUg26034948 = -366676584;    double rqIlGXZbMNMSOGCUg53149941 = -994642040;    double rqIlGXZbMNMSOGCUg3300449 = 81685295;    double rqIlGXZbMNMSOGCUg2994262 = -608695030;    double rqIlGXZbMNMSOGCUg93169648 = -369881977;    double rqIlGXZbMNMSOGCUg64191653 = -465695077;    double rqIlGXZbMNMSOGCUg23696450 = 76401612;    double rqIlGXZbMNMSOGCUg70119239 = -245349074;    double rqIlGXZbMNMSOGCUg8165525 = -540856522;    double rqIlGXZbMNMSOGCUg593711 = -107972953;    double rqIlGXZbMNMSOGCUg87219396 = -579210621;    double rqIlGXZbMNMSOGCUg21897848 = -141960037;    double rqIlGXZbMNMSOGCUg8862081 = -199693452;    double rqIlGXZbMNMSOGCUg99865865 = -327271014;    double rqIlGXZbMNMSOGCUg68790653 = -929515380;    double rqIlGXZbMNMSOGCUg99499647 = -832326630;    double rqIlGXZbMNMSOGCUg20818990 = -69955577;    double rqIlGXZbMNMSOGCUg11141398 = -719822755;    double rqIlGXZbMNMSOGCUg47222030 = -277386203;    double rqIlGXZbMNMSOGCUg34025871 = -413800626;    double rqIlGXZbMNMSOGCUg70930356 = -501564987;    double rqIlGXZbMNMSOGCUg13373680 = -932222702;    double rqIlGXZbMNMSOGCUg10824724 = -712186212;    double rqIlGXZbMNMSOGCUg82803828 = -95121068;    double rqIlGXZbMNMSOGCUg52951963 = -728735921;    double rqIlGXZbMNMSOGCUg63105114 = -680965468;    double rqIlGXZbMNMSOGCUg91008403 = -275493044;    double rqIlGXZbMNMSOGCUg25141520 = -732827327;    double rqIlGXZbMNMSOGCUg27967561 = -711488174;    double rqIlGXZbMNMSOGCUg54683508 = -876807575;    double rqIlGXZbMNMSOGCUg23920098 = -744318339;    double rqIlGXZbMNMSOGCUg80322657 = -21839141;    double rqIlGXZbMNMSOGCUg15405859 = 39412659;    double rqIlGXZbMNMSOGCUg69900895 = -784665436;    double rqIlGXZbMNMSOGCUg65132706 = -655587119;    double rqIlGXZbMNMSOGCUg93227750 = -745132703;    double rqIlGXZbMNMSOGCUg45496564 = -344342909;    double rqIlGXZbMNMSOGCUg2498510 = -137245164;    double rqIlGXZbMNMSOGCUg8458754 = -346626800;    double rqIlGXZbMNMSOGCUg97088466 = -568616541;    double rqIlGXZbMNMSOGCUg60690745 = -719191237;    double rqIlGXZbMNMSOGCUg94919123 = 22778181;    double rqIlGXZbMNMSOGCUg38965991 = -355508897;    double rqIlGXZbMNMSOGCUg46503139 = -651267797;    double rqIlGXZbMNMSOGCUg35062349 = -843603564;    double rqIlGXZbMNMSOGCUg64066845 = -198112108;    double rqIlGXZbMNMSOGCUg39448273 = -425307166;    double rqIlGXZbMNMSOGCUg33068186 = 4289335;    double rqIlGXZbMNMSOGCUg11136083 = -208148211;    double rqIlGXZbMNMSOGCUg11286540 = -904715999;    double rqIlGXZbMNMSOGCUg66893655 = -146550463;    double rqIlGXZbMNMSOGCUg537336 = -699735353;    double rqIlGXZbMNMSOGCUg18738194 = -782290082;    double rqIlGXZbMNMSOGCUg15075443 = -818074600;    double rqIlGXZbMNMSOGCUg57104600 = -88613304;    double rqIlGXZbMNMSOGCUg12661269 = -434453883;    double rqIlGXZbMNMSOGCUg42325218 = -182455829;    double rqIlGXZbMNMSOGCUg20496621 = -823193638;    double rqIlGXZbMNMSOGCUg50042299 = -879959110;    double rqIlGXZbMNMSOGCUg30064535 = -688916509;    double rqIlGXZbMNMSOGCUg73183250 = -90202033;    double rqIlGXZbMNMSOGCUg98554930 = -190771062;    double rqIlGXZbMNMSOGCUg42151679 = -533860901;    double rqIlGXZbMNMSOGCUg53482016 = -664048947;    double rqIlGXZbMNMSOGCUg76673613 = -363654615;    double rqIlGXZbMNMSOGCUg6896739 = -457371480;    double rqIlGXZbMNMSOGCUg6491990 = -81372697;    double rqIlGXZbMNMSOGCUg38961185 = -415028017;    double rqIlGXZbMNMSOGCUg34733159 = -671683895;    double rqIlGXZbMNMSOGCUg75562903 = -84382678;    double rqIlGXZbMNMSOGCUg54003083 = -387983721;    double rqIlGXZbMNMSOGCUg18320480 = -970490918;     rqIlGXZbMNMSOGCUg5068256 = rqIlGXZbMNMSOGCUg38753748;     rqIlGXZbMNMSOGCUg38753748 = rqIlGXZbMNMSOGCUg42396152;     rqIlGXZbMNMSOGCUg42396152 = rqIlGXZbMNMSOGCUg35598831;     rqIlGXZbMNMSOGCUg35598831 = rqIlGXZbMNMSOGCUg46282881;     rqIlGXZbMNMSOGCUg46282881 = rqIlGXZbMNMSOGCUg64507172;     rqIlGXZbMNMSOGCUg64507172 = rqIlGXZbMNMSOGCUg97935851;     rqIlGXZbMNMSOGCUg97935851 = rqIlGXZbMNMSOGCUg91167654;     rqIlGXZbMNMSOGCUg91167654 = rqIlGXZbMNMSOGCUg46377692;     rqIlGXZbMNMSOGCUg46377692 = rqIlGXZbMNMSOGCUg48797013;     rqIlGXZbMNMSOGCUg48797013 = rqIlGXZbMNMSOGCUg5492772;     rqIlGXZbMNMSOGCUg5492772 = rqIlGXZbMNMSOGCUg1628402;     rqIlGXZbMNMSOGCUg1628402 = rqIlGXZbMNMSOGCUg61280120;     rqIlGXZbMNMSOGCUg61280120 = rqIlGXZbMNMSOGCUg84387194;     rqIlGXZbMNMSOGCUg84387194 = rqIlGXZbMNMSOGCUg65038363;     rqIlGXZbMNMSOGCUg65038363 = rqIlGXZbMNMSOGCUg47131515;     rqIlGXZbMNMSOGCUg47131515 = rqIlGXZbMNMSOGCUg47096850;     rqIlGXZbMNMSOGCUg47096850 = rqIlGXZbMNMSOGCUg22281745;     rqIlGXZbMNMSOGCUg22281745 = rqIlGXZbMNMSOGCUg85964693;     rqIlGXZbMNMSOGCUg85964693 = rqIlGXZbMNMSOGCUg48310354;     rqIlGXZbMNMSOGCUg48310354 = rqIlGXZbMNMSOGCUg32934052;     rqIlGXZbMNMSOGCUg32934052 = rqIlGXZbMNMSOGCUg79926736;     rqIlGXZbMNMSOGCUg79926736 = rqIlGXZbMNMSOGCUg10786188;     rqIlGXZbMNMSOGCUg10786188 = rqIlGXZbMNMSOGCUg87712644;     rqIlGXZbMNMSOGCUg87712644 = rqIlGXZbMNMSOGCUg11678734;     rqIlGXZbMNMSOGCUg11678734 = rqIlGXZbMNMSOGCUg65960223;     rqIlGXZbMNMSOGCUg65960223 = rqIlGXZbMNMSOGCUg49101313;     rqIlGXZbMNMSOGCUg49101313 = rqIlGXZbMNMSOGCUg28034956;     rqIlGXZbMNMSOGCUg28034956 = rqIlGXZbMNMSOGCUg26034948;     rqIlGXZbMNMSOGCUg26034948 = rqIlGXZbMNMSOGCUg53149941;     rqIlGXZbMNMSOGCUg53149941 = rqIlGXZbMNMSOGCUg3300449;     rqIlGXZbMNMSOGCUg3300449 = rqIlGXZbMNMSOGCUg2994262;     rqIlGXZbMNMSOGCUg2994262 = rqIlGXZbMNMSOGCUg93169648;     rqIlGXZbMNMSOGCUg93169648 = rqIlGXZbMNMSOGCUg64191653;     rqIlGXZbMNMSOGCUg64191653 = rqIlGXZbMNMSOGCUg23696450;     rqIlGXZbMNMSOGCUg23696450 = rqIlGXZbMNMSOGCUg70119239;     rqIlGXZbMNMSOGCUg70119239 = rqIlGXZbMNMSOGCUg8165525;     rqIlGXZbMNMSOGCUg8165525 = rqIlGXZbMNMSOGCUg593711;     rqIlGXZbMNMSOGCUg593711 = rqIlGXZbMNMSOGCUg87219396;     rqIlGXZbMNMSOGCUg87219396 = rqIlGXZbMNMSOGCUg21897848;     rqIlGXZbMNMSOGCUg21897848 = rqIlGXZbMNMSOGCUg8862081;     rqIlGXZbMNMSOGCUg8862081 = rqIlGXZbMNMSOGCUg99865865;     rqIlGXZbMNMSOGCUg99865865 = rqIlGXZbMNMSOGCUg68790653;     rqIlGXZbMNMSOGCUg68790653 = rqIlGXZbMNMSOGCUg99499647;     rqIlGXZbMNMSOGCUg99499647 = rqIlGXZbMNMSOGCUg20818990;     rqIlGXZbMNMSOGCUg20818990 = rqIlGXZbMNMSOGCUg11141398;     rqIlGXZbMNMSOGCUg11141398 = rqIlGXZbMNMSOGCUg47222030;     rqIlGXZbMNMSOGCUg47222030 = rqIlGXZbMNMSOGCUg34025871;     rqIlGXZbMNMSOGCUg34025871 = rqIlGXZbMNMSOGCUg70930356;     rqIlGXZbMNMSOGCUg70930356 = rqIlGXZbMNMSOGCUg13373680;     rqIlGXZbMNMSOGCUg13373680 = rqIlGXZbMNMSOGCUg10824724;     rqIlGXZbMNMSOGCUg10824724 = rqIlGXZbMNMSOGCUg82803828;     rqIlGXZbMNMSOGCUg82803828 = rqIlGXZbMNMSOGCUg52951963;     rqIlGXZbMNMSOGCUg52951963 = rqIlGXZbMNMSOGCUg63105114;     rqIlGXZbMNMSOGCUg63105114 = rqIlGXZbMNMSOGCUg91008403;     rqIlGXZbMNMSOGCUg91008403 = rqIlGXZbMNMSOGCUg25141520;     rqIlGXZbMNMSOGCUg25141520 = rqIlGXZbMNMSOGCUg27967561;     rqIlGXZbMNMSOGCUg27967561 = rqIlGXZbMNMSOGCUg54683508;     rqIlGXZbMNMSOGCUg54683508 = rqIlGXZbMNMSOGCUg23920098;     rqIlGXZbMNMSOGCUg23920098 = rqIlGXZbMNMSOGCUg80322657;     rqIlGXZbMNMSOGCUg80322657 = rqIlGXZbMNMSOGCUg15405859;     rqIlGXZbMNMSOGCUg15405859 = rqIlGXZbMNMSOGCUg69900895;     rqIlGXZbMNMSOGCUg69900895 = rqIlGXZbMNMSOGCUg65132706;     rqIlGXZbMNMSOGCUg65132706 = rqIlGXZbMNMSOGCUg93227750;     rqIlGXZbMNMSOGCUg93227750 = rqIlGXZbMNMSOGCUg45496564;     rqIlGXZbMNMSOGCUg45496564 = rqIlGXZbMNMSOGCUg2498510;     rqIlGXZbMNMSOGCUg2498510 = rqIlGXZbMNMSOGCUg8458754;     rqIlGXZbMNMSOGCUg8458754 = rqIlGXZbMNMSOGCUg97088466;     rqIlGXZbMNMSOGCUg97088466 = rqIlGXZbMNMSOGCUg60690745;     rqIlGXZbMNMSOGCUg60690745 = rqIlGXZbMNMSOGCUg94919123;     rqIlGXZbMNMSOGCUg94919123 = rqIlGXZbMNMSOGCUg38965991;     rqIlGXZbMNMSOGCUg38965991 = rqIlGXZbMNMSOGCUg46503139;     rqIlGXZbMNMSOGCUg46503139 = rqIlGXZbMNMSOGCUg35062349;     rqIlGXZbMNMSOGCUg35062349 = rqIlGXZbMNMSOGCUg64066845;     rqIlGXZbMNMSOGCUg64066845 = rqIlGXZbMNMSOGCUg39448273;     rqIlGXZbMNMSOGCUg39448273 = rqIlGXZbMNMSOGCUg33068186;     rqIlGXZbMNMSOGCUg33068186 = rqIlGXZbMNMSOGCUg11136083;     rqIlGXZbMNMSOGCUg11136083 = rqIlGXZbMNMSOGCUg11286540;     rqIlGXZbMNMSOGCUg11286540 = rqIlGXZbMNMSOGCUg66893655;     rqIlGXZbMNMSOGCUg66893655 = rqIlGXZbMNMSOGCUg537336;     rqIlGXZbMNMSOGCUg537336 = rqIlGXZbMNMSOGCUg18738194;     rqIlGXZbMNMSOGCUg18738194 = rqIlGXZbMNMSOGCUg15075443;     rqIlGXZbMNMSOGCUg15075443 = rqIlGXZbMNMSOGCUg57104600;     rqIlGXZbMNMSOGCUg57104600 = rqIlGXZbMNMSOGCUg12661269;     rqIlGXZbMNMSOGCUg12661269 = rqIlGXZbMNMSOGCUg42325218;     rqIlGXZbMNMSOGCUg42325218 = rqIlGXZbMNMSOGCUg20496621;     rqIlGXZbMNMSOGCUg20496621 = rqIlGXZbMNMSOGCUg50042299;     rqIlGXZbMNMSOGCUg50042299 = rqIlGXZbMNMSOGCUg30064535;     rqIlGXZbMNMSOGCUg30064535 = rqIlGXZbMNMSOGCUg73183250;     rqIlGXZbMNMSOGCUg73183250 = rqIlGXZbMNMSOGCUg98554930;     rqIlGXZbMNMSOGCUg98554930 = rqIlGXZbMNMSOGCUg42151679;     rqIlGXZbMNMSOGCUg42151679 = rqIlGXZbMNMSOGCUg53482016;     rqIlGXZbMNMSOGCUg53482016 = rqIlGXZbMNMSOGCUg76673613;     rqIlGXZbMNMSOGCUg76673613 = rqIlGXZbMNMSOGCUg6896739;     rqIlGXZbMNMSOGCUg6896739 = rqIlGXZbMNMSOGCUg6491990;     rqIlGXZbMNMSOGCUg6491990 = rqIlGXZbMNMSOGCUg38961185;     rqIlGXZbMNMSOGCUg38961185 = rqIlGXZbMNMSOGCUg34733159;     rqIlGXZbMNMSOGCUg34733159 = rqIlGXZbMNMSOGCUg75562903;     rqIlGXZbMNMSOGCUg75562903 = rqIlGXZbMNMSOGCUg54003083;     rqIlGXZbMNMSOGCUg54003083 = rqIlGXZbMNMSOGCUg18320480;     rqIlGXZbMNMSOGCUg18320480 = rqIlGXZbMNMSOGCUg5068256;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wMeUzpVLeJGFlhNd38991522() {     double pNtClcKYesKlhidIF11185362 = 17607194;    double pNtClcKYesKlhidIF82127027 = -158130583;    double pNtClcKYesKlhidIF58438336 = -344146535;    double pNtClcKYesKlhidIF62673611 = -354625077;    double pNtClcKYesKlhidIF15178369 = -702165297;    double pNtClcKYesKlhidIF55924374 = -556958801;    double pNtClcKYesKlhidIF58424817 = -613808702;    double pNtClcKYesKlhidIF40927545 = -309137616;    double pNtClcKYesKlhidIF11020188 = -507807408;    double pNtClcKYesKlhidIF90026672 = -691633091;    double pNtClcKYesKlhidIF65603062 = -723377780;    double pNtClcKYesKlhidIF33451445 = -160914979;    double pNtClcKYesKlhidIF47489238 = -474202706;    double pNtClcKYesKlhidIF67530871 = -175028193;    double pNtClcKYesKlhidIF60780790 = 86966941;    double pNtClcKYesKlhidIF88636908 = -969364961;    double pNtClcKYesKlhidIF40932853 = -375785863;    double pNtClcKYesKlhidIF19334898 = -88336827;    double pNtClcKYesKlhidIF47794535 = -845841242;    double pNtClcKYesKlhidIF28860518 = 18096463;    double pNtClcKYesKlhidIF21487245 = -278469134;    double pNtClcKYesKlhidIF66447759 = -45593307;    double pNtClcKYesKlhidIF46376979 = -483560116;    double pNtClcKYesKlhidIF71705018 = -538811027;    double pNtClcKYesKlhidIF55378383 = -792754141;    double pNtClcKYesKlhidIF90681898 = 61668392;    double pNtClcKYesKlhidIF47561091 = -132537429;    double pNtClcKYesKlhidIF41142081 = -756107244;    double pNtClcKYesKlhidIF92167203 = -589059289;    double pNtClcKYesKlhidIF21765579 = -812112908;    double pNtClcKYesKlhidIF95542997 = -931635643;    double pNtClcKYesKlhidIF604884 = -318141846;    double pNtClcKYesKlhidIF2880598 = -262495347;    double pNtClcKYesKlhidIF59022218 = -217236842;    double pNtClcKYesKlhidIF85171101 = -562331494;    double pNtClcKYesKlhidIF38845153 = -459375181;    double pNtClcKYesKlhidIF89427459 = -494492391;    double pNtClcKYesKlhidIF34683924 = -979478963;    double pNtClcKYesKlhidIF60829650 = -573134118;    double pNtClcKYesKlhidIF9399786 = -615399259;    double pNtClcKYesKlhidIF14531305 = -264967346;    double pNtClcKYesKlhidIF32016323 = -597413292;    double pNtClcKYesKlhidIF46857632 = 89680036;    double pNtClcKYesKlhidIF86907388 = -851669568;    double pNtClcKYesKlhidIF32771455 = -831853731;    double pNtClcKYesKlhidIF23080593 = -705038872;    double pNtClcKYesKlhidIF65000604 = -396161740;    double pNtClcKYesKlhidIF41982812 = -423696159;    double pNtClcKYesKlhidIF80606085 = -587301975;    double pNtClcKYesKlhidIF26285332 = 80766490;    double pNtClcKYesKlhidIF46525402 = -554479883;    double pNtClcKYesKlhidIF93613547 = -995847388;    double pNtClcKYesKlhidIF1200421 = -520525509;    double pNtClcKYesKlhidIF59073994 = 98564387;    double pNtClcKYesKlhidIF17504278 = -976024826;    double pNtClcKYesKlhidIF44737602 = -936799499;    double pNtClcKYesKlhidIF35750048 = -674570467;    double pNtClcKYesKlhidIF86733317 = -805335508;    double pNtClcKYesKlhidIF7295229 = -561870937;    double pNtClcKYesKlhidIF24496471 = -663833689;    double pNtClcKYesKlhidIF8363284 = -324421373;    double pNtClcKYesKlhidIF17282737 = -857701459;    double pNtClcKYesKlhidIF48760342 = -720078328;    double pNtClcKYesKlhidIF89254609 = -695694501;    double pNtClcKYesKlhidIF94483674 = -759997449;    double pNtClcKYesKlhidIF64998178 = -305235935;    double pNtClcKYesKlhidIF30570848 = -898419632;    double pNtClcKYesKlhidIF88467020 = -156965865;    double pNtClcKYesKlhidIF82359769 = -612696699;    double pNtClcKYesKlhidIF21935638 = -453657878;    double pNtClcKYesKlhidIF99209449 = -374872570;    double pNtClcKYesKlhidIF6248930 = -396306901;    double pNtClcKYesKlhidIF58505248 = -515202710;    double pNtClcKYesKlhidIF38394749 = -130441983;    double pNtClcKYesKlhidIF14329214 = -716936191;    double pNtClcKYesKlhidIF89470922 = -681055843;    double pNtClcKYesKlhidIF19590128 = -35273344;    double pNtClcKYesKlhidIF59469591 = -631890548;    double pNtClcKYesKlhidIF38933564 = -706957297;    double pNtClcKYesKlhidIF32297791 = 12284730;    double pNtClcKYesKlhidIF25681295 = -542169868;    double pNtClcKYesKlhidIF5578280 = -708841271;    double pNtClcKYesKlhidIF60535996 = -68805269;    double pNtClcKYesKlhidIF65881871 = -569825779;    double pNtClcKYesKlhidIF75240176 = -157633025;    double pNtClcKYesKlhidIF1929451 = -935788255;    double pNtClcKYesKlhidIF99404463 = -797616338;    double pNtClcKYesKlhidIF43806603 = -261059735;    double pNtClcKYesKlhidIF41517940 = -241212016;    double pNtClcKYesKlhidIF40433499 = -625531995;    double pNtClcKYesKlhidIF3095105 = -784804714;    double pNtClcKYesKlhidIF2694142 = -689156883;    double pNtClcKYesKlhidIF27388696 = -317608027;    double pNtClcKYesKlhidIF36333179 = -909300429;    double pNtClcKYesKlhidIF1036503 = -190977887;    double pNtClcKYesKlhidIF97248568 = -407265887;    double pNtClcKYesKlhidIF83255980 = -877334964;    double pNtClcKYesKlhidIF57603023 = -214625463;    double pNtClcKYesKlhidIF92423713 = 8327880;    double pNtClcKYesKlhidIF67773276 = 17607194;     pNtClcKYesKlhidIF11185362 = pNtClcKYesKlhidIF82127027;     pNtClcKYesKlhidIF82127027 = pNtClcKYesKlhidIF58438336;     pNtClcKYesKlhidIF58438336 = pNtClcKYesKlhidIF62673611;     pNtClcKYesKlhidIF62673611 = pNtClcKYesKlhidIF15178369;     pNtClcKYesKlhidIF15178369 = pNtClcKYesKlhidIF55924374;     pNtClcKYesKlhidIF55924374 = pNtClcKYesKlhidIF58424817;     pNtClcKYesKlhidIF58424817 = pNtClcKYesKlhidIF40927545;     pNtClcKYesKlhidIF40927545 = pNtClcKYesKlhidIF11020188;     pNtClcKYesKlhidIF11020188 = pNtClcKYesKlhidIF90026672;     pNtClcKYesKlhidIF90026672 = pNtClcKYesKlhidIF65603062;     pNtClcKYesKlhidIF65603062 = pNtClcKYesKlhidIF33451445;     pNtClcKYesKlhidIF33451445 = pNtClcKYesKlhidIF47489238;     pNtClcKYesKlhidIF47489238 = pNtClcKYesKlhidIF67530871;     pNtClcKYesKlhidIF67530871 = pNtClcKYesKlhidIF60780790;     pNtClcKYesKlhidIF60780790 = pNtClcKYesKlhidIF88636908;     pNtClcKYesKlhidIF88636908 = pNtClcKYesKlhidIF40932853;     pNtClcKYesKlhidIF40932853 = pNtClcKYesKlhidIF19334898;     pNtClcKYesKlhidIF19334898 = pNtClcKYesKlhidIF47794535;     pNtClcKYesKlhidIF47794535 = pNtClcKYesKlhidIF28860518;     pNtClcKYesKlhidIF28860518 = pNtClcKYesKlhidIF21487245;     pNtClcKYesKlhidIF21487245 = pNtClcKYesKlhidIF66447759;     pNtClcKYesKlhidIF66447759 = pNtClcKYesKlhidIF46376979;     pNtClcKYesKlhidIF46376979 = pNtClcKYesKlhidIF71705018;     pNtClcKYesKlhidIF71705018 = pNtClcKYesKlhidIF55378383;     pNtClcKYesKlhidIF55378383 = pNtClcKYesKlhidIF90681898;     pNtClcKYesKlhidIF90681898 = pNtClcKYesKlhidIF47561091;     pNtClcKYesKlhidIF47561091 = pNtClcKYesKlhidIF41142081;     pNtClcKYesKlhidIF41142081 = pNtClcKYesKlhidIF92167203;     pNtClcKYesKlhidIF92167203 = pNtClcKYesKlhidIF21765579;     pNtClcKYesKlhidIF21765579 = pNtClcKYesKlhidIF95542997;     pNtClcKYesKlhidIF95542997 = pNtClcKYesKlhidIF604884;     pNtClcKYesKlhidIF604884 = pNtClcKYesKlhidIF2880598;     pNtClcKYesKlhidIF2880598 = pNtClcKYesKlhidIF59022218;     pNtClcKYesKlhidIF59022218 = pNtClcKYesKlhidIF85171101;     pNtClcKYesKlhidIF85171101 = pNtClcKYesKlhidIF38845153;     pNtClcKYesKlhidIF38845153 = pNtClcKYesKlhidIF89427459;     pNtClcKYesKlhidIF89427459 = pNtClcKYesKlhidIF34683924;     pNtClcKYesKlhidIF34683924 = pNtClcKYesKlhidIF60829650;     pNtClcKYesKlhidIF60829650 = pNtClcKYesKlhidIF9399786;     pNtClcKYesKlhidIF9399786 = pNtClcKYesKlhidIF14531305;     pNtClcKYesKlhidIF14531305 = pNtClcKYesKlhidIF32016323;     pNtClcKYesKlhidIF32016323 = pNtClcKYesKlhidIF46857632;     pNtClcKYesKlhidIF46857632 = pNtClcKYesKlhidIF86907388;     pNtClcKYesKlhidIF86907388 = pNtClcKYesKlhidIF32771455;     pNtClcKYesKlhidIF32771455 = pNtClcKYesKlhidIF23080593;     pNtClcKYesKlhidIF23080593 = pNtClcKYesKlhidIF65000604;     pNtClcKYesKlhidIF65000604 = pNtClcKYesKlhidIF41982812;     pNtClcKYesKlhidIF41982812 = pNtClcKYesKlhidIF80606085;     pNtClcKYesKlhidIF80606085 = pNtClcKYesKlhidIF26285332;     pNtClcKYesKlhidIF26285332 = pNtClcKYesKlhidIF46525402;     pNtClcKYesKlhidIF46525402 = pNtClcKYesKlhidIF93613547;     pNtClcKYesKlhidIF93613547 = pNtClcKYesKlhidIF1200421;     pNtClcKYesKlhidIF1200421 = pNtClcKYesKlhidIF59073994;     pNtClcKYesKlhidIF59073994 = pNtClcKYesKlhidIF17504278;     pNtClcKYesKlhidIF17504278 = pNtClcKYesKlhidIF44737602;     pNtClcKYesKlhidIF44737602 = pNtClcKYesKlhidIF35750048;     pNtClcKYesKlhidIF35750048 = pNtClcKYesKlhidIF86733317;     pNtClcKYesKlhidIF86733317 = pNtClcKYesKlhidIF7295229;     pNtClcKYesKlhidIF7295229 = pNtClcKYesKlhidIF24496471;     pNtClcKYesKlhidIF24496471 = pNtClcKYesKlhidIF8363284;     pNtClcKYesKlhidIF8363284 = pNtClcKYesKlhidIF17282737;     pNtClcKYesKlhidIF17282737 = pNtClcKYesKlhidIF48760342;     pNtClcKYesKlhidIF48760342 = pNtClcKYesKlhidIF89254609;     pNtClcKYesKlhidIF89254609 = pNtClcKYesKlhidIF94483674;     pNtClcKYesKlhidIF94483674 = pNtClcKYesKlhidIF64998178;     pNtClcKYesKlhidIF64998178 = pNtClcKYesKlhidIF30570848;     pNtClcKYesKlhidIF30570848 = pNtClcKYesKlhidIF88467020;     pNtClcKYesKlhidIF88467020 = pNtClcKYesKlhidIF82359769;     pNtClcKYesKlhidIF82359769 = pNtClcKYesKlhidIF21935638;     pNtClcKYesKlhidIF21935638 = pNtClcKYesKlhidIF99209449;     pNtClcKYesKlhidIF99209449 = pNtClcKYesKlhidIF6248930;     pNtClcKYesKlhidIF6248930 = pNtClcKYesKlhidIF58505248;     pNtClcKYesKlhidIF58505248 = pNtClcKYesKlhidIF38394749;     pNtClcKYesKlhidIF38394749 = pNtClcKYesKlhidIF14329214;     pNtClcKYesKlhidIF14329214 = pNtClcKYesKlhidIF89470922;     pNtClcKYesKlhidIF89470922 = pNtClcKYesKlhidIF19590128;     pNtClcKYesKlhidIF19590128 = pNtClcKYesKlhidIF59469591;     pNtClcKYesKlhidIF59469591 = pNtClcKYesKlhidIF38933564;     pNtClcKYesKlhidIF38933564 = pNtClcKYesKlhidIF32297791;     pNtClcKYesKlhidIF32297791 = pNtClcKYesKlhidIF25681295;     pNtClcKYesKlhidIF25681295 = pNtClcKYesKlhidIF5578280;     pNtClcKYesKlhidIF5578280 = pNtClcKYesKlhidIF60535996;     pNtClcKYesKlhidIF60535996 = pNtClcKYesKlhidIF65881871;     pNtClcKYesKlhidIF65881871 = pNtClcKYesKlhidIF75240176;     pNtClcKYesKlhidIF75240176 = pNtClcKYesKlhidIF1929451;     pNtClcKYesKlhidIF1929451 = pNtClcKYesKlhidIF99404463;     pNtClcKYesKlhidIF99404463 = pNtClcKYesKlhidIF43806603;     pNtClcKYesKlhidIF43806603 = pNtClcKYesKlhidIF41517940;     pNtClcKYesKlhidIF41517940 = pNtClcKYesKlhidIF40433499;     pNtClcKYesKlhidIF40433499 = pNtClcKYesKlhidIF3095105;     pNtClcKYesKlhidIF3095105 = pNtClcKYesKlhidIF2694142;     pNtClcKYesKlhidIF2694142 = pNtClcKYesKlhidIF27388696;     pNtClcKYesKlhidIF27388696 = pNtClcKYesKlhidIF36333179;     pNtClcKYesKlhidIF36333179 = pNtClcKYesKlhidIF1036503;     pNtClcKYesKlhidIF1036503 = pNtClcKYesKlhidIF97248568;     pNtClcKYesKlhidIF97248568 = pNtClcKYesKlhidIF83255980;     pNtClcKYesKlhidIF83255980 = pNtClcKYesKlhidIF57603023;     pNtClcKYesKlhidIF57603023 = pNtClcKYesKlhidIF92423713;     pNtClcKYesKlhidIF92423713 = pNtClcKYesKlhidIF67773276;     pNtClcKYesKlhidIF67773276 = pNtClcKYesKlhidIF11185362;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IvTQFKTppElwoedm6283122() {     double yrtTpYufpxZxvSWFE46644331 = -448669706;    double yrtTpYufpxZxvSWFE95399105 = -38682189;    double yrtTpYufpxZxvSWFE95824812 = -9813610;    double yrtTpYufpxZxvSWFE34896966 = -175776149;    double yrtTpYufpxZxvSWFE71463638 = -807764142;    double yrtTpYufpxZxvSWFE92147246 = -819306697;    double yrtTpYufpxZxvSWFE69082224 = -458534436;    double yrtTpYufpxZxvSWFE3059107 = 46628244;    double yrtTpYufpxZxvSWFE12537793 = -996138815;    double yrtTpYufpxZxvSWFE4929385 = -356320089;    double yrtTpYufpxZxvSWFE3784797 = -525385509;    double yrtTpYufpxZxvSWFE34019593 = -25035750;    double yrtTpYufpxZxvSWFE85723883 = 73784789;    double yrtTpYufpxZxvSWFE20671734 = -151391602;    double yrtTpYufpxZxvSWFE18975306 = -416458074;    double yrtTpYufpxZxvSWFE72178209 = -605176568;    double yrtTpYufpxZxvSWFE96768788 = -902760416;    double yrtTpYufpxZxvSWFE19095089 = -161764896;    double yrtTpYufpxZxvSWFE48344865 = -62404663;    double yrtTpYufpxZxvSWFE22697737 = -408464602;    double yrtTpYufpxZxvSWFE34574108 = 64256177;    double yrtTpYufpxZxvSWFE69869284 = -132229601;    double yrtTpYufpxZxvSWFE16902359 = -121192300;    double yrtTpYufpxZxvSWFE65497963 = 50364368;    double yrtTpYufpxZxvSWFE63396334 = -816739340;    double yrtTpYufpxZxvSWFE1380071 = -726908415;    double yrtTpYufpxZxvSWFE7981666 = 61178430;    double yrtTpYufpxZxvSWFE81870506 = -925879418;    double yrtTpYufpxZxvSWFE38835418 = -63154091;    double yrtTpYufpxZxvSWFE27298989 = -846620078;    double yrtTpYufpxZxvSWFE14126230 = -383616130;    double yrtTpYufpxZxvSWFE38427902 = -91113260;    double yrtTpYufpxZxvSWFE15098608 = -410091324;    double yrtTpYufpxZxvSWFE88770644 = -281930655;    double yrtTpYufpxZxvSWFE50979555 = -513431744;    double yrtTpYufpxZxvSWFE90405996 = -673940487;    double yrtTpYufpxZxvSWFE38839276 = -359188431;    double yrtTpYufpxZxvSWFE85899327 = -962134544;    double yrtTpYufpxZxvSWFE91220211 = -224010394;    double yrtTpYufpxZxvSWFE73671318 = -80661806;    double yrtTpYufpxZxvSWFE47982913 = -34166260;    double yrtTpYufpxZxvSWFE21381449 = -191958583;    double yrtTpYufpxZxvSWFE36241010 = -563500750;    double yrtTpYufpxZxvSWFE27038726 = -356001978;    double yrtTpYufpxZxvSWFE32787913 = -658364893;    double yrtTpYufpxZxvSWFE69041921 = -743962504;    double yrtTpYufpxZxvSWFE84345767 = -880148101;    double yrtTpYufpxZxvSWFE59709123 = -590222752;    double yrtTpYufpxZxvSWFE3498751 = -251038855;    double yrtTpYufpxZxvSWFE50814921 = -410260326;    double yrtTpYufpxZxvSWFE27296126 = -428756441;    double yrtTpYufpxZxvSWFE77099020 = -109499734;    double yrtTpYufpxZxvSWFE69821569 = -276571063;    double yrtTpYufpxZxvSWFE75065841 = -92944113;    double yrtTpYufpxZxvSWFE71709014 = -222514972;    double yrtTpYufpxZxvSWFE76775046 = -216440105;    double yrtTpYufpxZxvSWFE78496746 = -917489890;    double yrtTpYufpxZxvSWFE30326849 = 39822021;    double yrtTpYufpxZxvSWFE71500632 = -359036810;    double yrtTpYufpxZxvSWFE70083568 = 19144272;    double yrtTpYufpxZxvSWFE84165581 = -780485127;    double yrtTpYufpxZxvSWFE87211717 = -532655019;    double yrtTpYufpxZxvSWFE64223688 = -890217666;    double yrtTpYufpxZxvSWFE85238803 = -49518738;    double yrtTpYufpxZxvSWFE90803154 = -972703960;    double yrtTpYufpxZxvSWFE65356894 = -334272250;    double yrtTpYufpxZxvSWFE18920985 = -614944426;    double yrtTpYufpxZxvSWFE96953239 = -644284556;    double yrtTpYufpxZxvSWFE69692179 = -637959858;    double yrtTpYufpxZxvSWFE28569310 = -742517587;    double yrtTpYufpxZxvSWFE33338934 = -145988137;    double yrtTpYufpxZxvSWFE10869462 = -940625872;    double yrtTpYufpxZxvSWFE27874878 = -937754502;    double yrtTpYufpxZxvSWFE74673547 = -981742858;    double yrtTpYufpxZxvSWFE74714824 = -274298343;    double yrtTpYufpxZxvSWFE13192659 = -743785241;    double yrtTpYufpxZxvSWFE33628274 = -568728852;    double yrtTpYufpxZxvSWFE89863632 = -765190322;    double yrtTpYufpxZxvSWFE32710050 = -291270740;    double yrtTpYufpxZxvSWFE94354413 = 27223164;    double yrtTpYufpxZxvSWFE17034303 = -846760314;    double yrtTpYufpxZxvSWFE48272542 = -348598818;    double yrtTpYufpxZxvSWFE78371755 = -574840564;    double yrtTpYufpxZxvSWFE88020496 = -652893766;    double yrtTpYufpxZxvSWFE2864 = -317863638;    double yrtTpYufpxZxvSWFE37027210 = -174116396;    double yrtTpYufpxZxvSWFE68606333 = -814542198;    double yrtTpYufpxZxvSWFE40032767 = -217147212;    double yrtTpYufpxZxvSWFE17061631 = 40584316;    double yrtTpYufpxZxvSWFE74204509 = -196991640;    double yrtTpYufpxZxvSWFE11909250 = -756450598;    double yrtTpYufpxZxvSWFE8512427 = -299010453;    double yrtTpYufpxZxvSWFE14398696 = -503097735;    double yrtTpYufpxZxvSWFE21136643 = -143154667;    double yrtTpYufpxZxvSWFE89505737 = -300176679;    double yrtTpYufpxZxvSWFE60771195 = -501511242;    double yrtTpYufpxZxvSWFE57157761 = -301740917;    double yrtTpYufpxZxvSWFE51002207 = -413982013;    double yrtTpYufpxZxvSWFE36235572 = -383298019;    double yrtTpYufpxZxvSWFE67431019 = -448669706;     yrtTpYufpxZxvSWFE46644331 = yrtTpYufpxZxvSWFE95399105;     yrtTpYufpxZxvSWFE95399105 = yrtTpYufpxZxvSWFE95824812;     yrtTpYufpxZxvSWFE95824812 = yrtTpYufpxZxvSWFE34896966;     yrtTpYufpxZxvSWFE34896966 = yrtTpYufpxZxvSWFE71463638;     yrtTpYufpxZxvSWFE71463638 = yrtTpYufpxZxvSWFE92147246;     yrtTpYufpxZxvSWFE92147246 = yrtTpYufpxZxvSWFE69082224;     yrtTpYufpxZxvSWFE69082224 = yrtTpYufpxZxvSWFE3059107;     yrtTpYufpxZxvSWFE3059107 = yrtTpYufpxZxvSWFE12537793;     yrtTpYufpxZxvSWFE12537793 = yrtTpYufpxZxvSWFE4929385;     yrtTpYufpxZxvSWFE4929385 = yrtTpYufpxZxvSWFE3784797;     yrtTpYufpxZxvSWFE3784797 = yrtTpYufpxZxvSWFE34019593;     yrtTpYufpxZxvSWFE34019593 = yrtTpYufpxZxvSWFE85723883;     yrtTpYufpxZxvSWFE85723883 = yrtTpYufpxZxvSWFE20671734;     yrtTpYufpxZxvSWFE20671734 = yrtTpYufpxZxvSWFE18975306;     yrtTpYufpxZxvSWFE18975306 = yrtTpYufpxZxvSWFE72178209;     yrtTpYufpxZxvSWFE72178209 = yrtTpYufpxZxvSWFE96768788;     yrtTpYufpxZxvSWFE96768788 = yrtTpYufpxZxvSWFE19095089;     yrtTpYufpxZxvSWFE19095089 = yrtTpYufpxZxvSWFE48344865;     yrtTpYufpxZxvSWFE48344865 = yrtTpYufpxZxvSWFE22697737;     yrtTpYufpxZxvSWFE22697737 = yrtTpYufpxZxvSWFE34574108;     yrtTpYufpxZxvSWFE34574108 = yrtTpYufpxZxvSWFE69869284;     yrtTpYufpxZxvSWFE69869284 = yrtTpYufpxZxvSWFE16902359;     yrtTpYufpxZxvSWFE16902359 = yrtTpYufpxZxvSWFE65497963;     yrtTpYufpxZxvSWFE65497963 = yrtTpYufpxZxvSWFE63396334;     yrtTpYufpxZxvSWFE63396334 = yrtTpYufpxZxvSWFE1380071;     yrtTpYufpxZxvSWFE1380071 = yrtTpYufpxZxvSWFE7981666;     yrtTpYufpxZxvSWFE7981666 = yrtTpYufpxZxvSWFE81870506;     yrtTpYufpxZxvSWFE81870506 = yrtTpYufpxZxvSWFE38835418;     yrtTpYufpxZxvSWFE38835418 = yrtTpYufpxZxvSWFE27298989;     yrtTpYufpxZxvSWFE27298989 = yrtTpYufpxZxvSWFE14126230;     yrtTpYufpxZxvSWFE14126230 = yrtTpYufpxZxvSWFE38427902;     yrtTpYufpxZxvSWFE38427902 = yrtTpYufpxZxvSWFE15098608;     yrtTpYufpxZxvSWFE15098608 = yrtTpYufpxZxvSWFE88770644;     yrtTpYufpxZxvSWFE88770644 = yrtTpYufpxZxvSWFE50979555;     yrtTpYufpxZxvSWFE50979555 = yrtTpYufpxZxvSWFE90405996;     yrtTpYufpxZxvSWFE90405996 = yrtTpYufpxZxvSWFE38839276;     yrtTpYufpxZxvSWFE38839276 = yrtTpYufpxZxvSWFE85899327;     yrtTpYufpxZxvSWFE85899327 = yrtTpYufpxZxvSWFE91220211;     yrtTpYufpxZxvSWFE91220211 = yrtTpYufpxZxvSWFE73671318;     yrtTpYufpxZxvSWFE73671318 = yrtTpYufpxZxvSWFE47982913;     yrtTpYufpxZxvSWFE47982913 = yrtTpYufpxZxvSWFE21381449;     yrtTpYufpxZxvSWFE21381449 = yrtTpYufpxZxvSWFE36241010;     yrtTpYufpxZxvSWFE36241010 = yrtTpYufpxZxvSWFE27038726;     yrtTpYufpxZxvSWFE27038726 = yrtTpYufpxZxvSWFE32787913;     yrtTpYufpxZxvSWFE32787913 = yrtTpYufpxZxvSWFE69041921;     yrtTpYufpxZxvSWFE69041921 = yrtTpYufpxZxvSWFE84345767;     yrtTpYufpxZxvSWFE84345767 = yrtTpYufpxZxvSWFE59709123;     yrtTpYufpxZxvSWFE59709123 = yrtTpYufpxZxvSWFE3498751;     yrtTpYufpxZxvSWFE3498751 = yrtTpYufpxZxvSWFE50814921;     yrtTpYufpxZxvSWFE50814921 = yrtTpYufpxZxvSWFE27296126;     yrtTpYufpxZxvSWFE27296126 = yrtTpYufpxZxvSWFE77099020;     yrtTpYufpxZxvSWFE77099020 = yrtTpYufpxZxvSWFE69821569;     yrtTpYufpxZxvSWFE69821569 = yrtTpYufpxZxvSWFE75065841;     yrtTpYufpxZxvSWFE75065841 = yrtTpYufpxZxvSWFE71709014;     yrtTpYufpxZxvSWFE71709014 = yrtTpYufpxZxvSWFE76775046;     yrtTpYufpxZxvSWFE76775046 = yrtTpYufpxZxvSWFE78496746;     yrtTpYufpxZxvSWFE78496746 = yrtTpYufpxZxvSWFE30326849;     yrtTpYufpxZxvSWFE30326849 = yrtTpYufpxZxvSWFE71500632;     yrtTpYufpxZxvSWFE71500632 = yrtTpYufpxZxvSWFE70083568;     yrtTpYufpxZxvSWFE70083568 = yrtTpYufpxZxvSWFE84165581;     yrtTpYufpxZxvSWFE84165581 = yrtTpYufpxZxvSWFE87211717;     yrtTpYufpxZxvSWFE87211717 = yrtTpYufpxZxvSWFE64223688;     yrtTpYufpxZxvSWFE64223688 = yrtTpYufpxZxvSWFE85238803;     yrtTpYufpxZxvSWFE85238803 = yrtTpYufpxZxvSWFE90803154;     yrtTpYufpxZxvSWFE90803154 = yrtTpYufpxZxvSWFE65356894;     yrtTpYufpxZxvSWFE65356894 = yrtTpYufpxZxvSWFE18920985;     yrtTpYufpxZxvSWFE18920985 = yrtTpYufpxZxvSWFE96953239;     yrtTpYufpxZxvSWFE96953239 = yrtTpYufpxZxvSWFE69692179;     yrtTpYufpxZxvSWFE69692179 = yrtTpYufpxZxvSWFE28569310;     yrtTpYufpxZxvSWFE28569310 = yrtTpYufpxZxvSWFE33338934;     yrtTpYufpxZxvSWFE33338934 = yrtTpYufpxZxvSWFE10869462;     yrtTpYufpxZxvSWFE10869462 = yrtTpYufpxZxvSWFE27874878;     yrtTpYufpxZxvSWFE27874878 = yrtTpYufpxZxvSWFE74673547;     yrtTpYufpxZxvSWFE74673547 = yrtTpYufpxZxvSWFE74714824;     yrtTpYufpxZxvSWFE74714824 = yrtTpYufpxZxvSWFE13192659;     yrtTpYufpxZxvSWFE13192659 = yrtTpYufpxZxvSWFE33628274;     yrtTpYufpxZxvSWFE33628274 = yrtTpYufpxZxvSWFE89863632;     yrtTpYufpxZxvSWFE89863632 = yrtTpYufpxZxvSWFE32710050;     yrtTpYufpxZxvSWFE32710050 = yrtTpYufpxZxvSWFE94354413;     yrtTpYufpxZxvSWFE94354413 = yrtTpYufpxZxvSWFE17034303;     yrtTpYufpxZxvSWFE17034303 = yrtTpYufpxZxvSWFE48272542;     yrtTpYufpxZxvSWFE48272542 = yrtTpYufpxZxvSWFE78371755;     yrtTpYufpxZxvSWFE78371755 = yrtTpYufpxZxvSWFE88020496;     yrtTpYufpxZxvSWFE88020496 = yrtTpYufpxZxvSWFE2864;     yrtTpYufpxZxvSWFE2864 = yrtTpYufpxZxvSWFE37027210;     yrtTpYufpxZxvSWFE37027210 = yrtTpYufpxZxvSWFE68606333;     yrtTpYufpxZxvSWFE68606333 = yrtTpYufpxZxvSWFE40032767;     yrtTpYufpxZxvSWFE40032767 = yrtTpYufpxZxvSWFE17061631;     yrtTpYufpxZxvSWFE17061631 = yrtTpYufpxZxvSWFE74204509;     yrtTpYufpxZxvSWFE74204509 = yrtTpYufpxZxvSWFE11909250;     yrtTpYufpxZxvSWFE11909250 = yrtTpYufpxZxvSWFE8512427;     yrtTpYufpxZxvSWFE8512427 = yrtTpYufpxZxvSWFE14398696;     yrtTpYufpxZxvSWFE14398696 = yrtTpYufpxZxvSWFE21136643;     yrtTpYufpxZxvSWFE21136643 = yrtTpYufpxZxvSWFE89505737;     yrtTpYufpxZxvSWFE89505737 = yrtTpYufpxZxvSWFE60771195;     yrtTpYufpxZxvSWFE60771195 = yrtTpYufpxZxvSWFE57157761;     yrtTpYufpxZxvSWFE57157761 = yrtTpYufpxZxvSWFE51002207;     yrtTpYufpxZxvSWFE51002207 = yrtTpYufpxZxvSWFE36235572;     yrtTpYufpxZxvSWFE36235572 = yrtTpYufpxZxvSWFE67431019;     yrtTpYufpxZxvSWFE67431019 = yrtTpYufpxZxvSWFE46644331;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YfIMOlXzURZbyTrX21332190() {     double jocivdKQQGvoKjxsd52761437 = -560571595;    double jocivdKQQGvoKjxsd38772385 = -848281970;    double jocivdKQQGvoKjxsd11866997 = -160646531;    double jocivdKQQGvoKjxsd61971747 = -366524779;    double jocivdKQQGvoKjxsd40359127 = -228414013;    double jocivdKQQGvoKjxsd83564448 = 16197068;    double jocivdKQQGvoKjxsd29571190 = -597499413;    double jocivdKQQGvoKjxsd52818997 = -240245670;    double jocivdKQQGvoKjxsd77180288 = -764171481;    double jocivdKQQGvoKjxsd46159044 = -685295567;    double jocivdKQQGvoKjxsd63895087 = -402823096;    double jocivdKQQGvoKjxsd65842635 = -469441952;    double jocivdKQQGvoKjxsd71933002 = -366106300;    double jocivdKQQGvoKjxsd3815411 = -683630170;    double jocivdKQQGvoKjxsd14717733 = -6920240;    double jocivdKQQGvoKjxsd13683604 = -578176111;    double jocivdKQQGvoKjxsd90604791 = -419305528;    double jocivdKQQGvoKjxsd16148242 = -927287538;    double jocivdKQQGvoKjxsd10174707 = -468173760;    double jocivdKQQGvoKjxsd3247902 = -765367522;    double jocivdKQQGvoKjxsd23127301 = -891231278;    double jocivdKQQGvoKjxsd56390308 = -40159318;    double jocivdKQQGvoKjxsd52493151 = -967709787;    double jocivdKQQGvoKjxsd49490337 = -171940619;    double jocivdKQQGvoKjxsd7095985 = -89935372;    double jocivdKQQGvoKjxsd26101746 = -605563738;    double jocivdKQQGvoKjxsd6441444 = -939483773;    double jocivdKQQGvoKjxsd94977631 = -991808372;    double jocivdKQQGvoKjxsd4967674 = -285536796;    double jocivdKQQGvoKjxsd95914625 = -664090946;    double jocivdKQQGvoKjxsd6368779 = -296937068;    double jocivdKQQGvoKjxsd36038524 = -900560076;    double jocivdKQQGvoKjxsd24809557 = -302704695;    double jocivdKQQGvoKjxsd83601208 = -33472420;    double jocivdKQQGvoKjxsd12454208 = -52164850;    double jocivdKQQGvoKjxsd59131909 = -887966595;    double jocivdKQQGvoKjxsd20101210 = -312824301;    double jocivdKQQGvoKjxsd19989540 = -733640554;    double jocivdKQQGvoKjxsd64830465 = -217933891;    double jocivdKQQGvoKjxsd61173256 = -554101028;    double jocivdKQQGvoKjxsd53652136 = -99440154;    double jocivdKQQGvoKjxsd53531906 = -462100861;    double jocivdKQQGvoKjxsd14307989 = -644305334;    double jocivdKQQGvoKjxsd14446467 = -375344917;    double jocivdKQQGvoKjxsd44740378 = -320263047;    double jocivdKQQGvoKjxsd80981116 = -729178621;    double jocivdKQQGvoKjxsd2124343 = -998923639;    double jocivdKQQGvoKjxsd67666064 = -600118285;    double jocivdKQQGvoKjxsd13174480 = -336775843;    double jocivdKQQGvoKjxsd63726574 = -497271134;    double jocivdKQQGvoKjxsd62996804 = -271050112;    double jocivdKQQGvoKjxsd87908739 = 89773946;    double jocivdKQQGvoKjxsd18070027 = -68360650;    double jocivdKQQGvoKjxsd71034721 = -413414258;    double jocivdKQQGvoKjxsd98204888 = -923046754;    double jocivdKQQGvoKjxsd96371129 = -420412278;    double jocivdKQQGvoKjxsd86279234 = -880572184;    double jocivdKQQGvoKjxsd62376659 = -988705913;    double jocivdKQQGvoKjxsd54875762 = -176589407;    double jocivdKQQGvoKjxsd14257382 = -622850276;    double jocivdKQQGvoKjxsd77123005 = -44319159;    double jocivdKQQGvoKjxsd34593558 = -605691042;    double jocivdKQQGvoKjxsd47851324 = -954708874;    double jocivdKQQGvoKjxsd81265662 = -80536;    double jocivdKQQGvoKjxsd39790265 = -288358499;    double jocivdKQQGvoKjxsd27856563 = -502263020;    double jocivdKQQGvoKjxsd41033079 = -66737258;    double jocivdKQQGvoKjxsd88331793 = -232633880;    double jocivdKQQGvoKjxsd91361203 = -531465321;    double jocivdKQQGvoKjxsd55585823 = -118953646;    double jocivdKQQGvoKjxsd93582393 = -165351811;    double jocivdKQQGvoKjxsd70615252 = -685664975;    double jocivdKQQGvoKjxsd51317777 = -609353648;    double jocivdKQQGvoKjxsd49001451 = -914072732;    double jocivdKQQGvoKjxsd49595765 = -565927368;    double jocivdKQQGvoKjxsd69595394 = -329130418;    double jocivdKQQGvoKjxsd42082320 = -395853984;    double jocivdKQQGvoKjxsd38046684 = -492364871;    double jocivdKQQGvoKjxsd4749959 = -851677573;    double jocivdKQQGvoKjxsd26114869 = -360756752;    double jocivdKQQGvoKjxsd23977404 = -606640100;    double jocivdKQQGvoKjxsd38775379 = -239365489;    double jocivdKQQGvoKjxsd81803151 = -555032529;    double jocivdKQQGvoKjxsd41241100 = -788265662;    double jocivdKQQGvoKjxsd32917822 = -293040834;    double jocivdKQQGvoKjxsd18460040 = -286711014;    double jocivdKQQGvoKjxsd17968498 = -732199426;    double jocivdKQQGvoKjxsd53774835 = -889290437;    double jocivdKQQGvoKjxsd85396319 = -110425667;    double jocivdKQQGvoKjxsd16083078 = -631752573;    double jocivdKQQGvoKjxsd72852675 = 92605589;    double jocivdKQQGvoKjxsd57724551 = -324118389;    double jocivdKQQGvoKjxsd65113777 = -457051147;    double jocivdKQQGvoKjxsd50573083 = -595083616;    double jocivdKQQGvoKjxsd84050250 = -409781869;    double jocivdKQQGvoKjxsd19058579 = -493749112;    double jocivdKQQGvoKjxsd5680583 = -507391987;    double jocivdKQQGvoKjxsd33042327 = -544224798;    double jocivdKQQGvoKjxsd74656202 = 13013582;    double jocivdKQQGvoKjxsd16883816 = -560571595;     jocivdKQQGvoKjxsd52761437 = jocivdKQQGvoKjxsd38772385;     jocivdKQQGvoKjxsd38772385 = jocivdKQQGvoKjxsd11866997;     jocivdKQQGvoKjxsd11866997 = jocivdKQQGvoKjxsd61971747;     jocivdKQQGvoKjxsd61971747 = jocivdKQQGvoKjxsd40359127;     jocivdKQQGvoKjxsd40359127 = jocivdKQQGvoKjxsd83564448;     jocivdKQQGvoKjxsd83564448 = jocivdKQQGvoKjxsd29571190;     jocivdKQQGvoKjxsd29571190 = jocivdKQQGvoKjxsd52818997;     jocivdKQQGvoKjxsd52818997 = jocivdKQQGvoKjxsd77180288;     jocivdKQQGvoKjxsd77180288 = jocivdKQQGvoKjxsd46159044;     jocivdKQQGvoKjxsd46159044 = jocivdKQQGvoKjxsd63895087;     jocivdKQQGvoKjxsd63895087 = jocivdKQQGvoKjxsd65842635;     jocivdKQQGvoKjxsd65842635 = jocivdKQQGvoKjxsd71933002;     jocivdKQQGvoKjxsd71933002 = jocivdKQQGvoKjxsd3815411;     jocivdKQQGvoKjxsd3815411 = jocivdKQQGvoKjxsd14717733;     jocivdKQQGvoKjxsd14717733 = jocivdKQQGvoKjxsd13683604;     jocivdKQQGvoKjxsd13683604 = jocivdKQQGvoKjxsd90604791;     jocivdKQQGvoKjxsd90604791 = jocivdKQQGvoKjxsd16148242;     jocivdKQQGvoKjxsd16148242 = jocivdKQQGvoKjxsd10174707;     jocivdKQQGvoKjxsd10174707 = jocivdKQQGvoKjxsd3247902;     jocivdKQQGvoKjxsd3247902 = jocivdKQQGvoKjxsd23127301;     jocivdKQQGvoKjxsd23127301 = jocivdKQQGvoKjxsd56390308;     jocivdKQQGvoKjxsd56390308 = jocivdKQQGvoKjxsd52493151;     jocivdKQQGvoKjxsd52493151 = jocivdKQQGvoKjxsd49490337;     jocivdKQQGvoKjxsd49490337 = jocivdKQQGvoKjxsd7095985;     jocivdKQQGvoKjxsd7095985 = jocivdKQQGvoKjxsd26101746;     jocivdKQQGvoKjxsd26101746 = jocivdKQQGvoKjxsd6441444;     jocivdKQQGvoKjxsd6441444 = jocivdKQQGvoKjxsd94977631;     jocivdKQQGvoKjxsd94977631 = jocivdKQQGvoKjxsd4967674;     jocivdKQQGvoKjxsd4967674 = jocivdKQQGvoKjxsd95914625;     jocivdKQQGvoKjxsd95914625 = jocivdKQQGvoKjxsd6368779;     jocivdKQQGvoKjxsd6368779 = jocivdKQQGvoKjxsd36038524;     jocivdKQQGvoKjxsd36038524 = jocivdKQQGvoKjxsd24809557;     jocivdKQQGvoKjxsd24809557 = jocivdKQQGvoKjxsd83601208;     jocivdKQQGvoKjxsd83601208 = jocivdKQQGvoKjxsd12454208;     jocivdKQQGvoKjxsd12454208 = jocivdKQQGvoKjxsd59131909;     jocivdKQQGvoKjxsd59131909 = jocivdKQQGvoKjxsd20101210;     jocivdKQQGvoKjxsd20101210 = jocivdKQQGvoKjxsd19989540;     jocivdKQQGvoKjxsd19989540 = jocivdKQQGvoKjxsd64830465;     jocivdKQQGvoKjxsd64830465 = jocivdKQQGvoKjxsd61173256;     jocivdKQQGvoKjxsd61173256 = jocivdKQQGvoKjxsd53652136;     jocivdKQQGvoKjxsd53652136 = jocivdKQQGvoKjxsd53531906;     jocivdKQQGvoKjxsd53531906 = jocivdKQQGvoKjxsd14307989;     jocivdKQQGvoKjxsd14307989 = jocivdKQQGvoKjxsd14446467;     jocivdKQQGvoKjxsd14446467 = jocivdKQQGvoKjxsd44740378;     jocivdKQQGvoKjxsd44740378 = jocivdKQQGvoKjxsd80981116;     jocivdKQQGvoKjxsd80981116 = jocivdKQQGvoKjxsd2124343;     jocivdKQQGvoKjxsd2124343 = jocivdKQQGvoKjxsd67666064;     jocivdKQQGvoKjxsd67666064 = jocivdKQQGvoKjxsd13174480;     jocivdKQQGvoKjxsd13174480 = jocivdKQQGvoKjxsd63726574;     jocivdKQQGvoKjxsd63726574 = jocivdKQQGvoKjxsd62996804;     jocivdKQQGvoKjxsd62996804 = jocivdKQQGvoKjxsd87908739;     jocivdKQQGvoKjxsd87908739 = jocivdKQQGvoKjxsd18070027;     jocivdKQQGvoKjxsd18070027 = jocivdKQQGvoKjxsd71034721;     jocivdKQQGvoKjxsd71034721 = jocivdKQQGvoKjxsd98204888;     jocivdKQQGvoKjxsd98204888 = jocivdKQQGvoKjxsd96371129;     jocivdKQQGvoKjxsd96371129 = jocivdKQQGvoKjxsd86279234;     jocivdKQQGvoKjxsd86279234 = jocivdKQQGvoKjxsd62376659;     jocivdKQQGvoKjxsd62376659 = jocivdKQQGvoKjxsd54875762;     jocivdKQQGvoKjxsd54875762 = jocivdKQQGvoKjxsd14257382;     jocivdKQQGvoKjxsd14257382 = jocivdKQQGvoKjxsd77123005;     jocivdKQQGvoKjxsd77123005 = jocivdKQQGvoKjxsd34593558;     jocivdKQQGvoKjxsd34593558 = jocivdKQQGvoKjxsd47851324;     jocivdKQQGvoKjxsd47851324 = jocivdKQQGvoKjxsd81265662;     jocivdKQQGvoKjxsd81265662 = jocivdKQQGvoKjxsd39790265;     jocivdKQQGvoKjxsd39790265 = jocivdKQQGvoKjxsd27856563;     jocivdKQQGvoKjxsd27856563 = jocivdKQQGvoKjxsd41033079;     jocivdKQQGvoKjxsd41033079 = jocivdKQQGvoKjxsd88331793;     jocivdKQQGvoKjxsd88331793 = jocivdKQQGvoKjxsd91361203;     jocivdKQQGvoKjxsd91361203 = jocivdKQQGvoKjxsd55585823;     jocivdKQQGvoKjxsd55585823 = jocivdKQQGvoKjxsd93582393;     jocivdKQQGvoKjxsd93582393 = jocivdKQQGvoKjxsd70615252;     jocivdKQQGvoKjxsd70615252 = jocivdKQQGvoKjxsd51317777;     jocivdKQQGvoKjxsd51317777 = jocivdKQQGvoKjxsd49001451;     jocivdKQQGvoKjxsd49001451 = jocivdKQQGvoKjxsd49595765;     jocivdKQQGvoKjxsd49595765 = jocivdKQQGvoKjxsd69595394;     jocivdKQQGvoKjxsd69595394 = jocivdKQQGvoKjxsd42082320;     jocivdKQQGvoKjxsd42082320 = jocivdKQQGvoKjxsd38046684;     jocivdKQQGvoKjxsd38046684 = jocivdKQQGvoKjxsd4749959;     jocivdKQQGvoKjxsd4749959 = jocivdKQQGvoKjxsd26114869;     jocivdKQQGvoKjxsd26114869 = jocivdKQQGvoKjxsd23977404;     jocivdKQQGvoKjxsd23977404 = jocivdKQQGvoKjxsd38775379;     jocivdKQQGvoKjxsd38775379 = jocivdKQQGvoKjxsd81803151;     jocivdKQQGvoKjxsd81803151 = jocivdKQQGvoKjxsd41241100;     jocivdKQQGvoKjxsd41241100 = jocivdKQQGvoKjxsd32917822;     jocivdKQQGvoKjxsd32917822 = jocivdKQQGvoKjxsd18460040;     jocivdKQQGvoKjxsd18460040 = jocivdKQQGvoKjxsd17968498;     jocivdKQQGvoKjxsd17968498 = jocivdKQQGvoKjxsd53774835;     jocivdKQQGvoKjxsd53774835 = jocivdKQQGvoKjxsd85396319;     jocivdKQQGvoKjxsd85396319 = jocivdKQQGvoKjxsd16083078;     jocivdKQQGvoKjxsd16083078 = jocivdKQQGvoKjxsd72852675;     jocivdKQQGvoKjxsd72852675 = jocivdKQQGvoKjxsd57724551;     jocivdKQQGvoKjxsd57724551 = jocivdKQQGvoKjxsd65113777;     jocivdKQQGvoKjxsd65113777 = jocivdKQQGvoKjxsd50573083;     jocivdKQQGvoKjxsd50573083 = jocivdKQQGvoKjxsd84050250;     jocivdKQQGvoKjxsd84050250 = jocivdKQQGvoKjxsd19058579;     jocivdKQQGvoKjxsd19058579 = jocivdKQQGvoKjxsd5680583;     jocivdKQQGvoKjxsd5680583 = jocivdKQQGvoKjxsd33042327;     jocivdKQQGvoKjxsd33042327 = jocivdKQQGvoKjxsd74656202;     jocivdKQQGvoKjxsd74656202 = jocivdKQQGvoKjxsd16883816;     jocivdKQQGvoKjxsd16883816 = jocivdKQQGvoKjxsd52761437;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vxRbxVqIpQojFEuf88623788() {     double rWaNEjkdCpLFUydCl88220405 = 73151505;    double rWaNEjkdCpLFUydCl52044463 = -728833576;    double rWaNEjkdCpLFUydCl49253473 = -926313606;    double rWaNEjkdCpLFUydCl34195102 = -187675850;    double rWaNEjkdCpLFUydCl96644396 = -334012859;    double rWaNEjkdCpLFUydCl19787321 = -246150827;    double rWaNEjkdCpLFUydCl40228596 = -442225148;    double rWaNEjkdCpLFUydCl14950558 = -984479810;    double rWaNEjkdCpLFUydCl78697893 = -152502888;    double rWaNEjkdCpLFUydCl61061756 = -349982565;    double rWaNEjkdCpLFUydCl2076822 = -204830825;    double rWaNEjkdCpLFUydCl66410784 = -333562723;    double rWaNEjkdCpLFUydCl10167648 = -918118805;    double rWaNEjkdCpLFUydCl56956274 = -659993579;    double rWaNEjkdCpLFUydCl72912249 = -510345255;    double rWaNEjkdCpLFUydCl97224904 = -213987718;    double rWaNEjkdCpLFUydCl46440727 = -946280081;    double rWaNEjkdCpLFUydCl15908434 = 99284393;    double rWaNEjkdCpLFUydCl10725037 = -784737181;    double rWaNEjkdCpLFUydCl97085120 = -91928588;    double rWaNEjkdCpLFUydCl36214164 = -548505967;    double rWaNEjkdCpLFUydCl59811833 = -126795612;    double rWaNEjkdCpLFUydCl23018531 = -605341971;    double rWaNEjkdCpLFUydCl43283283 = -682765224;    double rWaNEjkdCpLFUydCl15113936 = -113920571;    double rWaNEjkdCpLFUydCl36799917 = -294140545;    double rWaNEjkdCpLFUydCl66862017 = -745767914;    double rWaNEjkdCpLFUydCl35706057 = -61580546;    double rWaNEjkdCpLFUydCl51635888 = -859631598;    double rWaNEjkdCpLFUydCl1448037 = -698598116;    double rWaNEjkdCpLFUydCl24952011 = -848917555;    double rWaNEjkdCpLFUydCl73861542 = -673531490;    double rWaNEjkdCpLFUydCl37027567 = -450300672;    double rWaNEjkdCpLFUydCl13349635 = -98166234;    double rWaNEjkdCpLFUydCl78262661 = -3265100;    double rWaNEjkdCpLFUydCl10692754 = -2531901;    double rWaNEjkdCpLFUydCl69513026 = -177520341;    double rWaNEjkdCpLFUydCl71204943 = -716296135;    double rWaNEjkdCpLFUydCl95221026 = -968810167;    double rWaNEjkdCpLFUydCl25444788 = -19363574;    double rWaNEjkdCpLFUydCl87103744 = -968639069;    double rWaNEjkdCpLFUydCl42897033 = -56646152;    double rWaNEjkdCpLFUydCl3691367 = -197486120;    double rWaNEjkdCpLFUydCl54577805 = -979677327;    double rWaNEjkdCpLFUydCl44756837 = -146774208;    double rWaNEjkdCpLFUydCl26942445 = -768102253;    double rWaNEjkdCpLFUydCl21469506 = -382909999;    double rWaNEjkdCpLFUydCl85392376 = -766644878;    double rWaNEjkdCpLFUydCl36067146 = -512723;    double rWaNEjkdCpLFUydCl88256163 = -988297950;    double rWaNEjkdCpLFUydCl43767528 = -145326669;    double rWaNEjkdCpLFUydCl71394212 = -123878401;    double rWaNEjkdCpLFUydCl86691175 = -924406204;    double rWaNEjkdCpLFUydCl87026568 = -604922758;    double rWaNEjkdCpLFUydCl52409625 = -169536900;    double rWaNEjkdCpLFUydCl28408573 = -800052883;    double rWaNEjkdCpLFUydCl29025933 = -23491606;    double rWaNEjkdCpLFUydCl5970191 = -143548383;    double rWaNEjkdCpLFUydCl19081166 = 26244720;    double rWaNEjkdCpLFUydCl59844479 = 60127686;    double rWaNEjkdCpLFUydCl52925303 = -500382914;    double rWaNEjkdCpLFUydCl4522540 = -280644602;    double rWaNEjkdCpLFUydCl63314670 = -24848212;    double rWaNEjkdCpLFUydCl77249856 = -453904772;    double rWaNEjkdCpLFUydCl36109745 = -501065010;    double rWaNEjkdCpLFUydCl28215280 = -531299335;    double rWaNEjkdCpLFUydCl29383217 = -883262052;    double rWaNEjkdCpLFUydCl96818012 = -719952571;    double rWaNEjkdCpLFUydCl78693613 = -556728479;    double rWaNEjkdCpLFUydCl62219496 = -407813355;    double rWaNEjkdCpLFUydCl27711878 = 63532623;    double rWaNEjkdCpLFUydCl75235784 = -129983947;    double rWaNEjkdCpLFUydCl20687407 = 68094560;    double rWaNEjkdCpLFUydCl85280248 = -665373608;    double rWaNEjkdCpLFUydCl9981376 = -123289520;    double rWaNEjkdCpLFUydCl93317130 = -391859816;    double rWaNEjkdCpLFUydCl56120466 = -929309492;    double rWaNEjkdCpLFUydCl68440726 = -625664645;    double rWaNEjkdCpLFUydCl98526445 = -435991016;    double rWaNEjkdCpLFUydCl88171491 = -345818319;    double rWaNEjkdCpLFUydCl15330412 = -911230546;    double rWaNEjkdCpLFUydCl81469641 = -979123036;    double rWaNEjkdCpLFUydCl99638910 = 38932177;    double rWaNEjkdCpLFUydCl63379725 = -871333649;    double rWaNEjkdCpLFUydCl57680509 = -453271447;    double rWaNEjkdCpLFUydCl53557799 = -625039155;    double rWaNEjkdCpLFUydCl87170367 = -749125286;    double rWaNEjkdCpLFUydCl50000999 = -845377915;    double rWaNEjkdCpLFUydCl60940010 = -928629334;    double rWaNEjkdCpLFUydCl49854088 = -203212217;    double rWaNEjkdCpLFUydCl81666820 = -979040295;    double rWaNEjkdCpLFUydCl63542836 = 66028042;    double rWaNEjkdCpLFUydCl52123777 = -642540855;    double rWaNEjkdCpLFUydCl35376548 = -928937854;    double rWaNEjkdCpLFUydCl72519485 = -518980661;    double rWaNEjkdCpLFUydCl82581204 = -587994467;    double rWaNEjkdCpLFUydCl79582363 = 68202060;    double rWaNEjkdCpLFUydCl26441511 = -743581348;    double rWaNEjkdCpLFUydCl18468060 = -378612317;    double rWaNEjkdCpLFUydCl16541558 = 73151505;     rWaNEjkdCpLFUydCl88220405 = rWaNEjkdCpLFUydCl52044463;     rWaNEjkdCpLFUydCl52044463 = rWaNEjkdCpLFUydCl49253473;     rWaNEjkdCpLFUydCl49253473 = rWaNEjkdCpLFUydCl34195102;     rWaNEjkdCpLFUydCl34195102 = rWaNEjkdCpLFUydCl96644396;     rWaNEjkdCpLFUydCl96644396 = rWaNEjkdCpLFUydCl19787321;     rWaNEjkdCpLFUydCl19787321 = rWaNEjkdCpLFUydCl40228596;     rWaNEjkdCpLFUydCl40228596 = rWaNEjkdCpLFUydCl14950558;     rWaNEjkdCpLFUydCl14950558 = rWaNEjkdCpLFUydCl78697893;     rWaNEjkdCpLFUydCl78697893 = rWaNEjkdCpLFUydCl61061756;     rWaNEjkdCpLFUydCl61061756 = rWaNEjkdCpLFUydCl2076822;     rWaNEjkdCpLFUydCl2076822 = rWaNEjkdCpLFUydCl66410784;     rWaNEjkdCpLFUydCl66410784 = rWaNEjkdCpLFUydCl10167648;     rWaNEjkdCpLFUydCl10167648 = rWaNEjkdCpLFUydCl56956274;     rWaNEjkdCpLFUydCl56956274 = rWaNEjkdCpLFUydCl72912249;     rWaNEjkdCpLFUydCl72912249 = rWaNEjkdCpLFUydCl97224904;     rWaNEjkdCpLFUydCl97224904 = rWaNEjkdCpLFUydCl46440727;     rWaNEjkdCpLFUydCl46440727 = rWaNEjkdCpLFUydCl15908434;     rWaNEjkdCpLFUydCl15908434 = rWaNEjkdCpLFUydCl10725037;     rWaNEjkdCpLFUydCl10725037 = rWaNEjkdCpLFUydCl97085120;     rWaNEjkdCpLFUydCl97085120 = rWaNEjkdCpLFUydCl36214164;     rWaNEjkdCpLFUydCl36214164 = rWaNEjkdCpLFUydCl59811833;     rWaNEjkdCpLFUydCl59811833 = rWaNEjkdCpLFUydCl23018531;     rWaNEjkdCpLFUydCl23018531 = rWaNEjkdCpLFUydCl43283283;     rWaNEjkdCpLFUydCl43283283 = rWaNEjkdCpLFUydCl15113936;     rWaNEjkdCpLFUydCl15113936 = rWaNEjkdCpLFUydCl36799917;     rWaNEjkdCpLFUydCl36799917 = rWaNEjkdCpLFUydCl66862017;     rWaNEjkdCpLFUydCl66862017 = rWaNEjkdCpLFUydCl35706057;     rWaNEjkdCpLFUydCl35706057 = rWaNEjkdCpLFUydCl51635888;     rWaNEjkdCpLFUydCl51635888 = rWaNEjkdCpLFUydCl1448037;     rWaNEjkdCpLFUydCl1448037 = rWaNEjkdCpLFUydCl24952011;     rWaNEjkdCpLFUydCl24952011 = rWaNEjkdCpLFUydCl73861542;     rWaNEjkdCpLFUydCl73861542 = rWaNEjkdCpLFUydCl37027567;     rWaNEjkdCpLFUydCl37027567 = rWaNEjkdCpLFUydCl13349635;     rWaNEjkdCpLFUydCl13349635 = rWaNEjkdCpLFUydCl78262661;     rWaNEjkdCpLFUydCl78262661 = rWaNEjkdCpLFUydCl10692754;     rWaNEjkdCpLFUydCl10692754 = rWaNEjkdCpLFUydCl69513026;     rWaNEjkdCpLFUydCl69513026 = rWaNEjkdCpLFUydCl71204943;     rWaNEjkdCpLFUydCl71204943 = rWaNEjkdCpLFUydCl95221026;     rWaNEjkdCpLFUydCl95221026 = rWaNEjkdCpLFUydCl25444788;     rWaNEjkdCpLFUydCl25444788 = rWaNEjkdCpLFUydCl87103744;     rWaNEjkdCpLFUydCl87103744 = rWaNEjkdCpLFUydCl42897033;     rWaNEjkdCpLFUydCl42897033 = rWaNEjkdCpLFUydCl3691367;     rWaNEjkdCpLFUydCl3691367 = rWaNEjkdCpLFUydCl54577805;     rWaNEjkdCpLFUydCl54577805 = rWaNEjkdCpLFUydCl44756837;     rWaNEjkdCpLFUydCl44756837 = rWaNEjkdCpLFUydCl26942445;     rWaNEjkdCpLFUydCl26942445 = rWaNEjkdCpLFUydCl21469506;     rWaNEjkdCpLFUydCl21469506 = rWaNEjkdCpLFUydCl85392376;     rWaNEjkdCpLFUydCl85392376 = rWaNEjkdCpLFUydCl36067146;     rWaNEjkdCpLFUydCl36067146 = rWaNEjkdCpLFUydCl88256163;     rWaNEjkdCpLFUydCl88256163 = rWaNEjkdCpLFUydCl43767528;     rWaNEjkdCpLFUydCl43767528 = rWaNEjkdCpLFUydCl71394212;     rWaNEjkdCpLFUydCl71394212 = rWaNEjkdCpLFUydCl86691175;     rWaNEjkdCpLFUydCl86691175 = rWaNEjkdCpLFUydCl87026568;     rWaNEjkdCpLFUydCl87026568 = rWaNEjkdCpLFUydCl52409625;     rWaNEjkdCpLFUydCl52409625 = rWaNEjkdCpLFUydCl28408573;     rWaNEjkdCpLFUydCl28408573 = rWaNEjkdCpLFUydCl29025933;     rWaNEjkdCpLFUydCl29025933 = rWaNEjkdCpLFUydCl5970191;     rWaNEjkdCpLFUydCl5970191 = rWaNEjkdCpLFUydCl19081166;     rWaNEjkdCpLFUydCl19081166 = rWaNEjkdCpLFUydCl59844479;     rWaNEjkdCpLFUydCl59844479 = rWaNEjkdCpLFUydCl52925303;     rWaNEjkdCpLFUydCl52925303 = rWaNEjkdCpLFUydCl4522540;     rWaNEjkdCpLFUydCl4522540 = rWaNEjkdCpLFUydCl63314670;     rWaNEjkdCpLFUydCl63314670 = rWaNEjkdCpLFUydCl77249856;     rWaNEjkdCpLFUydCl77249856 = rWaNEjkdCpLFUydCl36109745;     rWaNEjkdCpLFUydCl36109745 = rWaNEjkdCpLFUydCl28215280;     rWaNEjkdCpLFUydCl28215280 = rWaNEjkdCpLFUydCl29383217;     rWaNEjkdCpLFUydCl29383217 = rWaNEjkdCpLFUydCl96818012;     rWaNEjkdCpLFUydCl96818012 = rWaNEjkdCpLFUydCl78693613;     rWaNEjkdCpLFUydCl78693613 = rWaNEjkdCpLFUydCl62219496;     rWaNEjkdCpLFUydCl62219496 = rWaNEjkdCpLFUydCl27711878;     rWaNEjkdCpLFUydCl27711878 = rWaNEjkdCpLFUydCl75235784;     rWaNEjkdCpLFUydCl75235784 = rWaNEjkdCpLFUydCl20687407;     rWaNEjkdCpLFUydCl20687407 = rWaNEjkdCpLFUydCl85280248;     rWaNEjkdCpLFUydCl85280248 = rWaNEjkdCpLFUydCl9981376;     rWaNEjkdCpLFUydCl9981376 = rWaNEjkdCpLFUydCl93317130;     rWaNEjkdCpLFUydCl93317130 = rWaNEjkdCpLFUydCl56120466;     rWaNEjkdCpLFUydCl56120466 = rWaNEjkdCpLFUydCl68440726;     rWaNEjkdCpLFUydCl68440726 = rWaNEjkdCpLFUydCl98526445;     rWaNEjkdCpLFUydCl98526445 = rWaNEjkdCpLFUydCl88171491;     rWaNEjkdCpLFUydCl88171491 = rWaNEjkdCpLFUydCl15330412;     rWaNEjkdCpLFUydCl15330412 = rWaNEjkdCpLFUydCl81469641;     rWaNEjkdCpLFUydCl81469641 = rWaNEjkdCpLFUydCl99638910;     rWaNEjkdCpLFUydCl99638910 = rWaNEjkdCpLFUydCl63379725;     rWaNEjkdCpLFUydCl63379725 = rWaNEjkdCpLFUydCl57680509;     rWaNEjkdCpLFUydCl57680509 = rWaNEjkdCpLFUydCl53557799;     rWaNEjkdCpLFUydCl53557799 = rWaNEjkdCpLFUydCl87170367;     rWaNEjkdCpLFUydCl87170367 = rWaNEjkdCpLFUydCl50000999;     rWaNEjkdCpLFUydCl50000999 = rWaNEjkdCpLFUydCl60940010;     rWaNEjkdCpLFUydCl60940010 = rWaNEjkdCpLFUydCl49854088;     rWaNEjkdCpLFUydCl49854088 = rWaNEjkdCpLFUydCl81666820;     rWaNEjkdCpLFUydCl81666820 = rWaNEjkdCpLFUydCl63542836;     rWaNEjkdCpLFUydCl63542836 = rWaNEjkdCpLFUydCl52123777;     rWaNEjkdCpLFUydCl52123777 = rWaNEjkdCpLFUydCl35376548;     rWaNEjkdCpLFUydCl35376548 = rWaNEjkdCpLFUydCl72519485;     rWaNEjkdCpLFUydCl72519485 = rWaNEjkdCpLFUydCl82581204;     rWaNEjkdCpLFUydCl82581204 = rWaNEjkdCpLFUydCl79582363;     rWaNEjkdCpLFUydCl79582363 = rWaNEjkdCpLFUydCl26441511;     rWaNEjkdCpLFUydCl26441511 = rWaNEjkdCpLFUydCl18468060;     rWaNEjkdCpLFUydCl18468060 = rWaNEjkdCpLFUydCl16541558;     rWaNEjkdCpLFUydCl16541558 = rWaNEjkdCpLFUydCl88220405;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BUtPidMnpaMpfzqq3672857() {     double UDMgPnwnquDeatmkJ94337512 = -38750384;    double UDMgPnwnquDeatmkJ95417742 = -438433357;    double UDMgPnwnquDeatmkJ65295657 = 22853472;    double UDMgPnwnquDeatmkJ61269883 = -378424481;    double UDMgPnwnquDeatmkJ65539884 = -854662730;    double UDMgPnwnquDeatmkJ11204523 = -510647062;    double UDMgPnwnquDeatmkJ717563 = -581190125;    double UDMgPnwnquDeatmkJ64710449 = -171353723;    double UDMgPnwnquDeatmkJ43340388 = 79464446;    double UDMgPnwnquDeatmkJ2291416 = -678958042;    double UDMgPnwnquDeatmkJ62187112 = -82268411;    double UDMgPnwnquDeatmkJ98233826 = -777968925;    double UDMgPnwnquDeatmkJ96376766 = -258009894;    double UDMgPnwnquDeatmkJ40099951 = -92232147;    double UDMgPnwnquDeatmkJ68654676 = -100807421;    double UDMgPnwnquDeatmkJ38730298 = -186987261;    double UDMgPnwnquDeatmkJ40276731 = -462825194;    double UDMgPnwnquDeatmkJ12961587 = -666238250;    double UDMgPnwnquDeatmkJ72554878 = -90506278;    double UDMgPnwnquDeatmkJ77635285 = -448831507;    double UDMgPnwnquDeatmkJ24767357 = -403993423;    double UDMgPnwnquDeatmkJ46332857 = -34725328;    double UDMgPnwnquDeatmkJ58609322 = -351859458;    double UDMgPnwnquDeatmkJ27275657 = -905070211;    double UDMgPnwnquDeatmkJ58813586 = -487116604;    double UDMgPnwnquDeatmkJ61521592 = -172795868;    double UDMgPnwnquDeatmkJ65321795 = -646430117;    double UDMgPnwnquDeatmkJ48813182 = -127509500;    double UDMgPnwnquDeatmkJ17768144 = 17985697;    double UDMgPnwnquDeatmkJ70063673 = -516068984;    double UDMgPnwnquDeatmkJ17194560 = -762238493;    double UDMgPnwnquDeatmkJ71472164 = -382978306;    double UDMgPnwnquDeatmkJ46738516 = -342914042;    double UDMgPnwnquDeatmkJ8180200 = -949707999;    double UDMgPnwnquDeatmkJ39737313 = -641998206;    double UDMgPnwnquDeatmkJ79418666 = -216558008;    double UDMgPnwnquDeatmkJ50774961 = -131156211;    double UDMgPnwnquDeatmkJ5295156 = -487802144;    double UDMgPnwnquDeatmkJ68831280 = -962733664;    double UDMgPnwnquDeatmkJ12946726 = -492802797;    double UDMgPnwnquDeatmkJ92772968 = 66087037;    double UDMgPnwnquDeatmkJ75047490 = -326788430;    double UDMgPnwnquDeatmkJ81758345 = -278290704;    double UDMgPnwnquDeatmkJ41985545 = -999020265;    double UDMgPnwnquDeatmkJ56709302 = -908672363;    double UDMgPnwnquDeatmkJ38881639 = -753318369;    double UDMgPnwnquDeatmkJ39248080 = -501685537;    double UDMgPnwnquDeatmkJ93349317 = -776540411;    double UDMgPnwnquDeatmkJ45742875 = -86249711;    double UDMgPnwnquDeatmkJ1167816 = 24691242;    double UDMgPnwnquDeatmkJ79468206 = 12379659;    double UDMgPnwnquDeatmkJ82203931 = 75395279;    double UDMgPnwnquDeatmkJ34939633 = -716195792;    double UDMgPnwnquDeatmkJ82995448 = -925392903;    double UDMgPnwnquDeatmkJ78905499 = -870068682;    double UDMgPnwnquDeatmkJ48004656 = 95974944;    double UDMgPnwnquDeatmkJ36808421 = 13426100;    double UDMgPnwnquDeatmkJ38020001 = -72076317;    double UDMgPnwnquDeatmkJ2456297 = -891307877;    double UDMgPnwnquDeatmkJ4018293 = -581866862;    double UDMgPnwnquDeatmkJ45882727 = -864216946;    double UDMgPnwnquDeatmkJ51904380 = -353680626;    double UDMgPnwnquDeatmkJ46942305 = -89339421;    double UDMgPnwnquDeatmkJ73276715 = -404466571;    double UDMgPnwnquDeatmkJ85096855 = -916719550;    double UDMgPnwnquDeatmkJ90714947 = -699290106;    double UDMgPnwnquDeatmkJ51495311 = -335054883;    double UDMgPnwnquDeatmkJ88196567 = -308301895;    double UDMgPnwnquDeatmkJ362638 = -450233942;    double UDMgPnwnquDeatmkJ89236009 = -884249414;    double UDMgPnwnquDeatmkJ87955336 = 44168949;    double UDMgPnwnquDeatmkJ34981575 = -975023050;    double UDMgPnwnquDeatmkJ44130306 = -703504586;    double UDMgPnwnquDeatmkJ59608153 = -597703482;    double UDMgPnwnquDeatmkJ84862316 = -414918545;    double UDMgPnwnquDeatmkJ49719867 = 22795007;    double UDMgPnwnquDeatmkJ64574511 = -756434625;    double UDMgPnwnquDeatmkJ16623778 = -352839193;    double UDMgPnwnquDeatmkJ70566354 = -996397849;    double UDMgPnwnquDeatmkJ19931947 = -733798235;    double UDMgPnwnquDeatmkJ22273513 = -671110332;    double UDMgPnwnquDeatmkJ71972478 = -869889707;    double UDMgPnwnquDeatmkJ3070307 = 58740211;    double UDMgPnwnquDeatmkJ16600328 = 93294454;    double UDMgPnwnquDeatmkJ90595467 = -428448643;    double UDMgPnwnquDeatmkJ34990629 = -737633773;    double UDMgPnwnquDeatmkJ36532532 = -666782514;    double UDMgPnwnquDeatmkJ63743067 = -417521140;    double UDMgPnwnquDeatmkJ29274700 = 20360683;    double UDMgPnwnquDeatmkJ91732657 = -637973150;    double UDMgPnwnquDeatmkJ42610246 = -129984109;    double UDMgPnwnquDeatmkJ12754961 = 40920105;    double UDMgPnwnquDeatmkJ2838859 = -596494268;    double UDMgPnwnquDeatmkJ64812988 = -280866802;    double UDMgPnwnquDeatmkJ67063998 = -628585852;    double UDMgPnwnquDeatmkJ40868588 = -580232337;    double UDMgPnwnquDeatmkJ28105185 = -137449009;    double UDMgPnwnquDeatmkJ8481630 = -873824134;    double UDMgPnwnquDeatmkJ56888690 = 17699284;    double UDMgPnwnquDeatmkJ65994354 = -38750384;     UDMgPnwnquDeatmkJ94337512 = UDMgPnwnquDeatmkJ95417742;     UDMgPnwnquDeatmkJ95417742 = UDMgPnwnquDeatmkJ65295657;     UDMgPnwnquDeatmkJ65295657 = UDMgPnwnquDeatmkJ61269883;     UDMgPnwnquDeatmkJ61269883 = UDMgPnwnquDeatmkJ65539884;     UDMgPnwnquDeatmkJ65539884 = UDMgPnwnquDeatmkJ11204523;     UDMgPnwnquDeatmkJ11204523 = UDMgPnwnquDeatmkJ717563;     UDMgPnwnquDeatmkJ717563 = UDMgPnwnquDeatmkJ64710449;     UDMgPnwnquDeatmkJ64710449 = UDMgPnwnquDeatmkJ43340388;     UDMgPnwnquDeatmkJ43340388 = UDMgPnwnquDeatmkJ2291416;     UDMgPnwnquDeatmkJ2291416 = UDMgPnwnquDeatmkJ62187112;     UDMgPnwnquDeatmkJ62187112 = UDMgPnwnquDeatmkJ98233826;     UDMgPnwnquDeatmkJ98233826 = UDMgPnwnquDeatmkJ96376766;     UDMgPnwnquDeatmkJ96376766 = UDMgPnwnquDeatmkJ40099951;     UDMgPnwnquDeatmkJ40099951 = UDMgPnwnquDeatmkJ68654676;     UDMgPnwnquDeatmkJ68654676 = UDMgPnwnquDeatmkJ38730298;     UDMgPnwnquDeatmkJ38730298 = UDMgPnwnquDeatmkJ40276731;     UDMgPnwnquDeatmkJ40276731 = UDMgPnwnquDeatmkJ12961587;     UDMgPnwnquDeatmkJ12961587 = UDMgPnwnquDeatmkJ72554878;     UDMgPnwnquDeatmkJ72554878 = UDMgPnwnquDeatmkJ77635285;     UDMgPnwnquDeatmkJ77635285 = UDMgPnwnquDeatmkJ24767357;     UDMgPnwnquDeatmkJ24767357 = UDMgPnwnquDeatmkJ46332857;     UDMgPnwnquDeatmkJ46332857 = UDMgPnwnquDeatmkJ58609322;     UDMgPnwnquDeatmkJ58609322 = UDMgPnwnquDeatmkJ27275657;     UDMgPnwnquDeatmkJ27275657 = UDMgPnwnquDeatmkJ58813586;     UDMgPnwnquDeatmkJ58813586 = UDMgPnwnquDeatmkJ61521592;     UDMgPnwnquDeatmkJ61521592 = UDMgPnwnquDeatmkJ65321795;     UDMgPnwnquDeatmkJ65321795 = UDMgPnwnquDeatmkJ48813182;     UDMgPnwnquDeatmkJ48813182 = UDMgPnwnquDeatmkJ17768144;     UDMgPnwnquDeatmkJ17768144 = UDMgPnwnquDeatmkJ70063673;     UDMgPnwnquDeatmkJ70063673 = UDMgPnwnquDeatmkJ17194560;     UDMgPnwnquDeatmkJ17194560 = UDMgPnwnquDeatmkJ71472164;     UDMgPnwnquDeatmkJ71472164 = UDMgPnwnquDeatmkJ46738516;     UDMgPnwnquDeatmkJ46738516 = UDMgPnwnquDeatmkJ8180200;     UDMgPnwnquDeatmkJ8180200 = UDMgPnwnquDeatmkJ39737313;     UDMgPnwnquDeatmkJ39737313 = UDMgPnwnquDeatmkJ79418666;     UDMgPnwnquDeatmkJ79418666 = UDMgPnwnquDeatmkJ50774961;     UDMgPnwnquDeatmkJ50774961 = UDMgPnwnquDeatmkJ5295156;     UDMgPnwnquDeatmkJ5295156 = UDMgPnwnquDeatmkJ68831280;     UDMgPnwnquDeatmkJ68831280 = UDMgPnwnquDeatmkJ12946726;     UDMgPnwnquDeatmkJ12946726 = UDMgPnwnquDeatmkJ92772968;     UDMgPnwnquDeatmkJ92772968 = UDMgPnwnquDeatmkJ75047490;     UDMgPnwnquDeatmkJ75047490 = UDMgPnwnquDeatmkJ81758345;     UDMgPnwnquDeatmkJ81758345 = UDMgPnwnquDeatmkJ41985545;     UDMgPnwnquDeatmkJ41985545 = UDMgPnwnquDeatmkJ56709302;     UDMgPnwnquDeatmkJ56709302 = UDMgPnwnquDeatmkJ38881639;     UDMgPnwnquDeatmkJ38881639 = UDMgPnwnquDeatmkJ39248080;     UDMgPnwnquDeatmkJ39248080 = UDMgPnwnquDeatmkJ93349317;     UDMgPnwnquDeatmkJ93349317 = UDMgPnwnquDeatmkJ45742875;     UDMgPnwnquDeatmkJ45742875 = UDMgPnwnquDeatmkJ1167816;     UDMgPnwnquDeatmkJ1167816 = UDMgPnwnquDeatmkJ79468206;     UDMgPnwnquDeatmkJ79468206 = UDMgPnwnquDeatmkJ82203931;     UDMgPnwnquDeatmkJ82203931 = UDMgPnwnquDeatmkJ34939633;     UDMgPnwnquDeatmkJ34939633 = UDMgPnwnquDeatmkJ82995448;     UDMgPnwnquDeatmkJ82995448 = UDMgPnwnquDeatmkJ78905499;     UDMgPnwnquDeatmkJ78905499 = UDMgPnwnquDeatmkJ48004656;     UDMgPnwnquDeatmkJ48004656 = UDMgPnwnquDeatmkJ36808421;     UDMgPnwnquDeatmkJ36808421 = UDMgPnwnquDeatmkJ38020001;     UDMgPnwnquDeatmkJ38020001 = UDMgPnwnquDeatmkJ2456297;     UDMgPnwnquDeatmkJ2456297 = UDMgPnwnquDeatmkJ4018293;     UDMgPnwnquDeatmkJ4018293 = UDMgPnwnquDeatmkJ45882727;     UDMgPnwnquDeatmkJ45882727 = UDMgPnwnquDeatmkJ51904380;     UDMgPnwnquDeatmkJ51904380 = UDMgPnwnquDeatmkJ46942305;     UDMgPnwnquDeatmkJ46942305 = UDMgPnwnquDeatmkJ73276715;     UDMgPnwnquDeatmkJ73276715 = UDMgPnwnquDeatmkJ85096855;     UDMgPnwnquDeatmkJ85096855 = UDMgPnwnquDeatmkJ90714947;     UDMgPnwnquDeatmkJ90714947 = UDMgPnwnquDeatmkJ51495311;     UDMgPnwnquDeatmkJ51495311 = UDMgPnwnquDeatmkJ88196567;     UDMgPnwnquDeatmkJ88196567 = UDMgPnwnquDeatmkJ362638;     UDMgPnwnquDeatmkJ362638 = UDMgPnwnquDeatmkJ89236009;     UDMgPnwnquDeatmkJ89236009 = UDMgPnwnquDeatmkJ87955336;     UDMgPnwnquDeatmkJ87955336 = UDMgPnwnquDeatmkJ34981575;     UDMgPnwnquDeatmkJ34981575 = UDMgPnwnquDeatmkJ44130306;     UDMgPnwnquDeatmkJ44130306 = UDMgPnwnquDeatmkJ59608153;     UDMgPnwnquDeatmkJ59608153 = UDMgPnwnquDeatmkJ84862316;     UDMgPnwnquDeatmkJ84862316 = UDMgPnwnquDeatmkJ49719867;     UDMgPnwnquDeatmkJ49719867 = UDMgPnwnquDeatmkJ64574511;     UDMgPnwnquDeatmkJ64574511 = UDMgPnwnquDeatmkJ16623778;     UDMgPnwnquDeatmkJ16623778 = UDMgPnwnquDeatmkJ70566354;     UDMgPnwnquDeatmkJ70566354 = UDMgPnwnquDeatmkJ19931947;     UDMgPnwnquDeatmkJ19931947 = UDMgPnwnquDeatmkJ22273513;     UDMgPnwnquDeatmkJ22273513 = UDMgPnwnquDeatmkJ71972478;     UDMgPnwnquDeatmkJ71972478 = UDMgPnwnquDeatmkJ3070307;     UDMgPnwnquDeatmkJ3070307 = UDMgPnwnquDeatmkJ16600328;     UDMgPnwnquDeatmkJ16600328 = UDMgPnwnquDeatmkJ90595467;     UDMgPnwnquDeatmkJ90595467 = UDMgPnwnquDeatmkJ34990629;     UDMgPnwnquDeatmkJ34990629 = UDMgPnwnquDeatmkJ36532532;     UDMgPnwnquDeatmkJ36532532 = UDMgPnwnquDeatmkJ63743067;     UDMgPnwnquDeatmkJ63743067 = UDMgPnwnquDeatmkJ29274700;     UDMgPnwnquDeatmkJ29274700 = UDMgPnwnquDeatmkJ91732657;     UDMgPnwnquDeatmkJ91732657 = UDMgPnwnquDeatmkJ42610246;     UDMgPnwnquDeatmkJ42610246 = UDMgPnwnquDeatmkJ12754961;     UDMgPnwnquDeatmkJ12754961 = UDMgPnwnquDeatmkJ2838859;     UDMgPnwnquDeatmkJ2838859 = UDMgPnwnquDeatmkJ64812988;     UDMgPnwnquDeatmkJ64812988 = UDMgPnwnquDeatmkJ67063998;     UDMgPnwnquDeatmkJ67063998 = UDMgPnwnquDeatmkJ40868588;     UDMgPnwnquDeatmkJ40868588 = UDMgPnwnquDeatmkJ28105185;     UDMgPnwnquDeatmkJ28105185 = UDMgPnwnquDeatmkJ8481630;     UDMgPnwnquDeatmkJ8481630 = UDMgPnwnquDeatmkJ56888690;     UDMgPnwnquDeatmkJ56888690 = UDMgPnwnquDeatmkJ65994354;     UDMgPnwnquDeatmkJ65994354 = UDMgPnwnquDeatmkJ94337512;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void oLVqSXIdtdHBttoU70964456() {     double sbyiYvUtxuYdLYNDX29796481 = -505027283;    double sbyiYvUtxuYdLYNDX8689822 = -318984964;    double sbyiYvUtxuYdLYNDX2682134 = -742813603;    double sbyiYvUtxuYdLYNDX33493238 = -199575552;    double sbyiYvUtxuYdLYNDX21825154 = -960261575;    double sbyiYvUtxuYdLYNDX47427395 = -772994957;    double sbyiYvUtxuYdLYNDX11374969 = -425915859;    double sbyiYvUtxuYdLYNDX26842010 = -915587864;    double sbyiYvUtxuYdLYNDX44857994 = -408866961;    double sbyiYvUtxuYdLYNDX17194127 = -343645040;    double sbyiYvUtxuYdLYNDX368847 = -984276141;    double sbyiYvUtxuYdLYNDX98801974 = -642089697;    double sbyiYvUtxuYdLYNDX34611412 = -810022398;    double sbyiYvUtxuYdLYNDX93240814 = -68595556;    double sbyiYvUtxuYdLYNDX26849192 = -604232436;    double sbyiYvUtxuYdLYNDX22271599 = -922798868;    double sbyiYvUtxuYdLYNDX96112666 = -989799747;    double sbyiYvUtxuYdLYNDX12721779 = -739666318;    double sbyiYvUtxuYdLYNDX73105208 = -407069700;    double sbyiYvUtxuYdLYNDX71472503 = -875392573;    double sbyiYvUtxuYdLYNDX37854220 = -61268111;    double sbyiYvUtxuYdLYNDX49754381 = -121361622;    double sbyiYvUtxuYdLYNDX29134702 = 10508358;    double sbyiYvUtxuYdLYNDX21068602 = -315894816;    double sbyiYvUtxuYdLYNDX66831537 = -511101802;    double sbyiYvUtxuYdLYNDX72219764 = -961372675;    double sbyiYvUtxuYdLYNDX25742370 = -452714257;    double sbyiYvUtxuYdLYNDX89541606 = -297281674;    double sbyiYvUtxuYdLYNDX64436358 = -556109105;    double sbyiYvUtxuYdLYNDX75597084 = -550576154;    double sbyiYvUtxuYdLYNDX35777792 = -214218980;    double sbyiYvUtxuYdLYNDX9295183 = -155949720;    double sbyiYvUtxuYdLYNDX58956527 = -490510020;    double sbyiYvUtxuYdLYNDX37928626 = 85598187;    double sbyiYvUtxuYdLYNDX5545767 = -593098456;    double sbyiYvUtxuYdLYNDX30979510 = -431123314;    double sbyiYvUtxuYdLYNDX186778 = 4147749;    double sbyiYvUtxuYdLYNDX56510559 = -470457726;    double sbyiYvUtxuYdLYNDX99221841 = -613609941;    double sbyiYvUtxuYdLYNDX77218258 = 41934657;    double sbyiYvUtxuYdLYNDX26224576 = -803111877;    double sbyiYvUtxuYdLYNDX64412616 = 78666279;    double sbyiYvUtxuYdLYNDX71141723 = -931471490;    double sbyiYvUtxuYdLYNDX82116883 = -503352675;    double sbyiYvUtxuYdLYNDX56725761 = -735183524;    double sbyiYvUtxuYdLYNDX84842967 = -792242001;    double sbyiYvUtxuYdLYNDX58593244 = -985671897;    double sbyiYvUtxuYdLYNDX11075629 = -943067004;    double sbyiYvUtxuYdLYNDX68635541 = -849986591;    double sbyiYvUtxuYdLYNDX25697405 = -466335573;    double sbyiYvUtxuYdLYNDX60238930 = -961896898;    double sbyiYvUtxuYdLYNDX65689404 = -138257068;    double sbyiYvUtxuYdLYNDX3560782 = -472241346;    double sbyiYvUtxuYdLYNDX98987294 = -16901402;    double sbyiYvUtxuYdLYNDX33110236 = -116558828;    double sbyiYvUtxuYdLYNDX80042099 = -283665662;    double sbyiYvUtxuYdLYNDX79555119 = -229493323;    double sbyiYvUtxuYdLYNDX81613532 = -326918788;    double sbyiYvUtxuYdLYNDX66661700 = -688473750;    double sbyiYvUtxuYdLYNDX49605390 = -998888901;    double sbyiYvUtxuYdLYNDX21685026 = -220280700;    double sbyiYvUtxuYdLYNDX21833362 = -28634186;    double sbyiYvUtxuYdLYNDX62405651 = -259478759;    double sbyiYvUtxuYdLYNDX69260909 = -858290807;    double sbyiYvUtxuYdLYNDX81416335 = -29426060;    double sbyiYvUtxuYdLYNDX91073664 = -728326421;    double sbyiYvUtxuYdLYNDX39845448 = -51579678;    double sbyiYvUtxuYdLYNDX96682785 = -795620586;    double sbyiYvUtxuYdLYNDX87695047 = -475497100;    double sbyiYvUtxuYdLYNDX95869681 = -73109123;    double sbyiYvUtxuYdLYNDX22084821 = -826946618;    double sbyiYvUtxuYdLYNDX39602108 = -419342022;    double sbyiYvUtxuYdLYNDX13499937 = -26056378;    double sbyiYvUtxuYdLYNDX95886950 = -349004357;    double sbyiYvUtxuYdLYNDX45247928 = 27719303;    double sbyiYvUtxuYdLYNDX73441603 = -39934391;    double sbyiYvUtxuYdLYNDX78612658 = -189890133;    double sbyiYvUtxuYdLYNDX47017819 = -486138968;    double sbyiYvUtxuYdLYNDX64342841 = -580711292;    double sbyiYvUtxuYdLYNDX81988569 = -718859801;    double sbyiYvUtxuYdLYNDX13626521 = -975700778;    double sbyiYvUtxuYdLYNDX14666741 = -509647254;    double sbyiYvUtxuYdLYNDX20906066 = -447295083;    double sbyiYvUtxuYdLYNDX38738953 = 10226468;    double sbyiYvUtxuYdLYNDX15358155 = -588679256;    double sbyiYvUtxuYdLYNDX70088388 = 24038087;    double sbyiYvUtxuYdLYNDX5734402 = -683708375;    double sbyiYvUtxuYdLYNDX59969232 = -373608618;    double sbyiYvUtxuYdLYNDX4818390 = -797842985;    double sbyiYvUtxuYdLYNDX25503667 = -209432795;    double sbyiYvUtxuYdLYNDX51424391 = -101629992;    double sbyiYvUtxuYdLYNDX18573246 = -668933464;    double sbyiYvUtxuYdLYNDX89848858 = -781983976;    double sbyiYvUtxuYdLYNDX49616452 = -614721040;    double sbyiYvUtxuYdLYNDX55533233 = -737784643;    double sbyiYvUtxuYdLYNDX4391215 = -674477692;    double sbyiYvUtxuYdLYNDX2006966 = -661854962;    double sbyiYvUtxuYdLYNDX1880815 = 26819317;    double sbyiYvUtxuYdLYNDX700548 = -373926615;    double sbyiYvUtxuYdLYNDX65652096 = -505027283;     sbyiYvUtxuYdLYNDX29796481 = sbyiYvUtxuYdLYNDX8689822;     sbyiYvUtxuYdLYNDX8689822 = sbyiYvUtxuYdLYNDX2682134;     sbyiYvUtxuYdLYNDX2682134 = sbyiYvUtxuYdLYNDX33493238;     sbyiYvUtxuYdLYNDX33493238 = sbyiYvUtxuYdLYNDX21825154;     sbyiYvUtxuYdLYNDX21825154 = sbyiYvUtxuYdLYNDX47427395;     sbyiYvUtxuYdLYNDX47427395 = sbyiYvUtxuYdLYNDX11374969;     sbyiYvUtxuYdLYNDX11374969 = sbyiYvUtxuYdLYNDX26842010;     sbyiYvUtxuYdLYNDX26842010 = sbyiYvUtxuYdLYNDX44857994;     sbyiYvUtxuYdLYNDX44857994 = sbyiYvUtxuYdLYNDX17194127;     sbyiYvUtxuYdLYNDX17194127 = sbyiYvUtxuYdLYNDX368847;     sbyiYvUtxuYdLYNDX368847 = sbyiYvUtxuYdLYNDX98801974;     sbyiYvUtxuYdLYNDX98801974 = sbyiYvUtxuYdLYNDX34611412;     sbyiYvUtxuYdLYNDX34611412 = sbyiYvUtxuYdLYNDX93240814;     sbyiYvUtxuYdLYNDX93240814 = sbyiYvUtxuYdLYNDX26849192;     sbyiYvUtxuYdLYNDX26849192 = sbyiYvUtxuYdLYNDX22271599;     sbyiYvUtxuYdLYNDX22271599 = sbyiYvUtxuYdLYNDX96112666;     sbyiYvUtxuYdLYNDX96112666 = sbyiYvUtxuYdLYNDX12721779;     sbyiYvUtxuYdLYNDX12721779 = sbyiYvUtxuYdLYNDX73105208;     sbyiYvUtxuYdLYNDX73105208 = sbyiYvUtxuYdLYNDX71472503;     sbyiYvUtxuYdLYNDX71472503 = sbyiYvUtxuYdLYNDX37854220;     sbyiYvUtxuYdLYNDX37854220 = sbyiYvUtxuYdLYNDX49754381;     sbyiYvUtxuYdLYNDX49754381 = sbyiYvUtxuYdLYNDX29134702;     sbyiYvUtxuYdLYNDX29134702 = sbyiYvUtxuYdLYNDX21068602;     sbyiYvUtxuYdLYNDX21068602 = sbyiYvUtxuYdLYNDX66831537;     sbyiYvUtxuYdLYNDX66831537 = sbyiYvUtxuYdLYNDX72219764;     sbyiYvUtxuYdLYNDX72219764 = sbyiYvUtxuYdLYNDX25742370;     sbyiYvUtxuYdLYNDX25742370 = sbyiYvUtxuYdLYNDX89541606;     sbyiYvUtxuYdLYNDX89541606 = sbyiYvUtxuYdLYNDX64436358;     sbyiYvUtxuYdLYNDX64436358 = sbyiYvUtxuYdLYNDX75597084;     sbyiYvUtxuYdLYNDX75597084 = sbyiYvUtxuYdLYNDX35777792;     sbyiYvUtxuYdLYNDX35777792 = sbyiYvUtxuYdLYNDX9295183;     sbyiYvUtxuYdLYNDX9295183 = sbyiYvUtxuYdLYNDX58956527;     sbyiYvUtxuYdLYNDX58956527 = sbyiYvUtxuYdLYNDX37928626;     sbyiYvUtxuYdLYNDX37928626 = sbyiYvUtxuYdLYNDX5545767;     sbyiYvUtxuYdLYNDX5545767 = sbyiYvUtxuYdLYNDX30979510;     sbyiYvUtxuYdLYNDX30979510 = sbyiYvUtxuYdLYNDX186778;     sbyiYvUtxuYdLYNDX186778 = sbyiYvUtxuYdLYNDX56510559;     sbyiYvUtxuYdLYNDX56510559 = sbyiYvUtxuYdLYNDX99221841;     sbyiYvUtxuYdLYNDX99221841 = sbyiYvUtxuYdLYNDX77218258;     sbyiYvUtxuYdLYNDX77218258 = sbyiYvUtxuYdLYNDX26224576;     sbyiYvUtxuYdLYNDX26224576 = sbyiYvUtxuYdLYNDX64412616;     sbyiYvUtxuYdLYNDX64412616 = sbyiYvUtxuYdLYNDX71141723;     sbyiYvUtxuYdLYNDX71141723 = sbyiYvUtxuYdLYNDX82116883;     sbyiYvUtxuYdLYNDX82116883 = sbyiYvUtxuYdLYNDX56725761;     sbyiYvUtxuYdLYNDX56725761 = sbyiYvUtxuYdLYNDX84842967;     sbyiYvUtxuYdLYNDX84842967 = sbyiYvUtxuYdLYNDX58593244;     sbyiYvUtxuYdLYNDX58593244 = sbyiYvUtxuYdLYNDX11075629;     sbyiYvUtxuYdLYNDX11075629 = sbyiYvUtxuYdLYNDX68635541;     sbyiYvUtxuYdLYNDX68635541 = sbyiYvUtxuYdLYNDX25697405;     sbyiYvUtxuYdLYNDX25697405 = sbyiYvUtxuYdLYNDX60238930;     sbyiYvUtxuYdLYNDX60238930 = sbyiYvUtxuYdLYNDX65689404;     sbyiYvUtxuYdLYNDX65689404 = sbyiYvUtxuYdLYNDX3560782;     sbyiYvUtxuYdLYNDX3560782 = sbyiYvUtxuYdLYNDX98987294;     sbyiYvUtxuYdLYNDX98987294 = sbyiYvUtxuYdLYNDX33110236;     sbyiYvUtxuYdLYNDX33110236 = sbyiYvUtxuYdLYNDX80042099;     sbyiYvUtxuYdLYNDX80042099 = sbyiYvUtxuYdLYNDX79555119;     sbyiYvUtxuYdLYNDX79555119 = sbyiYvUtxuYdLYNDX81613532;     sbyiYvUtxuYdLYNDX81613532 = sbyiYvUtxuYdLYNDX66661700;     sbyiYvUtxuYdLYNDX66661700 = sbyiYvUtxuYdLYNDX49605390;     sbyiYvUtxuYdLYNDX49605390 = sbyiYvUtxuYdLYNDX21685026;     sbyiYvUtxuYdLYNDX21685026 = sbyiYvUtxuYdLYNDX21833362;     sbyiYvUtxuYdLYNDX21833362 = sbyiYvUtxuYdLYNDX62405651;     sbyiYvUtxuYdLYNDX62405651 = sbyiYvUtxuYdLYNDX69260909;     sbyiYvUtxuYdLYNDX69260909 = sbyiYvUtxuYdLYNDX81416335;     sbyiYvUtxuYdLYNDX81416335 = sbyiYvUtxuYdLYNDX91073664;     sbyiYvUtxuYdLYNDX91073664 = sbyiYvUtxuYdLYNDX39845448;     sbyiYvUtxuYdLYNDX39845448 = sbyiYvUtxuYdLYNDX96682785;     sbyiYvUtxuYdLYNDX96682785 = sbyiYvUtxuYdLYNDX87695047;     sbyiYvUtxuYdLYNDX87695047 = sbyiYvUtxuYdLYNDX95869681;     sbyiYvUtxuYdLYNDX95869681 = sbyiYvUtxuYdLYNDX22084821;     sbyiYvUtxuYdLYNDX22084821 = sbyiYvUtxuYdLYNDX39602108;     sbyiYvUtxuYdLYNDX39602108 = sbyiYvUtxuYdLYNDX13499937;     sbyiYvUtxuYdLYNDX13499937 = sbyiYvUtxuYdLYNDX95886950;     sbyiYvUtxuYdLYNDX95886950 = sbyiYvUtxuYdLYNDX45247928;     sbyiYvUtxuYdLYNDX45247928 = sbyiYvUtxuYdLYNDX73441603;     sbyiYvUtxuYdLYNDX73441603 = sbyiYvUtxuYdLYNDX78612658;     sbyiYvUtxuYdLYNDX78612658 = sbyiYvUtxuYdLYNDX47017819;     sbyiYvUtxuYdLYNDX47017819 = sbyiYvUtxuYdLYNDX64342841;     sbyiYvUtxuYdLYNDX64342841 = sbyiYvUtxuYdLYNDX81988569;     sbyiYvUtxuYdLYNDX81988569 = sbyiYvUtxuYdLYNDX13626521;     sbyiYvUtxuYdLYNDX13626521 = sbyiYvUtxuYdLYNDX14666741;     sbyiYvUtxuYdLYNDX14666741 = sbyiYvUtxuYdLYNDX20906066;     sbyiYvUtxuYdLYNDX20906066 = sbyiYvUtxuYdLYNDX38738953;     sbyiYvUtxuYdLYNDX38738953 = sbyiYvUtxuYdLYNDX15358155;     sbyiYvUtxuYdLYNDX15358155 = sbyiYvUtxuYdLYNDX70088388;     sbyiYvUtxuYdLYNDX70088388 = sbyiYvUtxuYdLYNDX5734402;     sbyiYvUtxuYdLYNDX5734402 = sbyiYvUtxuYdLYNDX59969232;     sbyiYvUtxuYdLYNDX59969232 = sbyiYvUtxuYdLYNDX4818390;     sbyiYvUtxuYdLYNDX4818390 = sbyiYvUtxuYdLYNDX25503667;     sbyiYvUtxuYdLYNDX25503667 = sbyiYvUtxuYdLYNDX51424391;     sbyiYvUtxuYdLYNDX51424391 = sbyiYvUtxuYdLYNDX18573246;     sbyiYvUtxuYdLYNDX18573246 = sbyiYvUtxuYdLYNDX89848858;     sbyiYvUtxuYdLYNDX89848858 = sbyiYvUtxuYdLYNDX49616452;     sbyiYvUtxuYdLYNDX49616452 = sbyiYvUtxuYdLYNDX55533233;     sbyiYvUtxuYdLYNDX55533233 = sbyiYvUtxuYdLYNDX4391215;     sbyiYvUtxuYdLYNDX4391215 = sbyiYvUtxuYdLYNDX2006966;     sbyiYvUtxuYdLYNDX2006966 = sbyiYvUtxuYdLYNDX1880815;     sbyiYvUtxuYdLYNDX1880815 = sbyiYvUtxuYdLYNDX700548;     sbyiYvUtxuYdLYNDX700548 = sbyiYvUtxuYdLYNDX65652096;     sbyiYvUtxuYdLYNDX65652096 = sbyiYvUtxuYdLYNDX29796481;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bxaRPvmyzYBBvWQe86013523() {     double ftoMXJojsWSrQxfcw35913588 = -616929172;    double ftoMXJojsWSrQxfcw52063101 = -28584744;    double ftoMXJojsWSrQxfcw18724317 = -893646524;    double ftoMXJojsWSrQxfcw60568018 = -390324182;    double ftoMXJojsWSrQxfcw90720642 = -380911447;    double ftoMXJojsWSrQxfcw38844597 = 62508808;    double ftoMXJojsWSrQxfcw71863934 = -564880836;    double ftoMXJojsWSrQxfcw76601901 = -102461777;    double ftoMXJojsWSrQxfcw9500489 = -176899626;    double ftoMXJojsWSrQxfcw58423786 = -672620518;    double ftoMXJojsWSrQxfcw60479137 = -861713727;    double ftoMXJojsWSrQxfcw30625017 = 13504101;    double ftoMXJojsWSrQxfcw20820530 = -149913487;    double ftoMXJojsWSrQxfcw76384490 = -600834124;    double ftoMXJojsWSrQxfcw22591619 = -194694603;    double ftoMXJojsWSrQxfcw63776993 = -895798412;    double ftoMXJojsWSrQxfcw89948669 = -506344859;    double ftoMXJojsWSrQxfcw9774932 = -405188961;    double ftoMXJojsWSrQxfcw34935050 = -812838796;    double ftoMXJojsWSrQxfcw52022668 = -132295492;    double ftoMXJojsWSrQxfcw26407413 = 83244433;    double ftoMXJojsWSrQxfcw36275405 = -29291339;    double ftoMXJojsWSrQxfcw64725494 = -836009128;    double ftoMXJojsWSrQxfcw5060976 = -538199803;    double ftoMXJojsWSrQxfcw10531188 = -884297835;    double ftoMXJojsWSrQxfcw96941439 = -840027998;    double ftoMXJojsWSrQxfcw24202148 = -353376461;    double ftoMXJojsWSrQxfcw2648732 = -363210628;    double ftoMXJojsWSrQxfcw30568614 = -778491810;    double ftoMXJojsWSrQxfcw44212721 = -368047021;    double ftoMXJojsWSrQxfcw28020341 = -127539918;    double ftoMXJojsWSrQxfcw6905805 = -965396536;    double ftoMXJojsWSrQxfcw68667475 = -383123390;    double ftoMXJojsWSrQxfcw32759190 = -765943578;    double ftoMXJojsWSrQxfcw67020418 = -131831562;    double ftoMXJojsWSrQxfcw99705423 = -645149422;    double ftoMXJojsWSrQxfcw81448712 = 50511879;    double ftoMXJojsWSrQxfcw90600771 = -241963735;    double ftoMXJojsWSrQxfcw72832095 = -607533437;    double ftoMXJojsWSrQxfcw64720196 = -431504565;    double ftoMXJojsWSrQxfcw31893800 = -868385771;    double ftoMXJojsWSrQxfcw96563073 = -191475999;    double ftoMXJojsWSrQxfcw49208702 = 87723927;    double ftoMXJojsWSrQxfcw69524624 = -522695613;    double ftoMXJojsWSrQxfcw68678226 = -397081678;    double ftoMXJojsWSrQxfcw96782162 = -777458118;    double ftoMXJojsWSrQxfcw76371818 = -4447435;    double ftoMXJojsWSrQxfcw19032570 = -952962536;    double ftoMXJojsWSrQxfcw78311270 = -935723580;    double ftoMXJojsWSrQxfcw38609058 = -553346381;    double ftoMXJojsWSrQxfcw95939608 = -804190569;    double ftoMXJojsWSrQxfcw76499123 = 61016612;    double ftoMXJojsWSrQxfcw51809238 = -264030934;    double ftoMXJojsWSrQxfcw94956175 = -337371548;    double ftoMXJojsWSrQxfcw59606111 = -817090611;    double ftoMXJojsWSrQxfcw99638182 = -487637834;    double ftoMXJojsWSrQxfcw87337606 = -192575616;    double ftoMXJojsWSrQxfcw13663342 = -255446721;    double ftoMXJojsWSrQxfcw50036831 = -506026348;    double ftoMXJojsWSrQxfcw93779203 = -540883449;    double ftoMXJojsWSrQxfcw14642450 = -584114732;    double ftoMXJojsWSrQxfcw69215202 = -101670209;    double ftoMXJojsWSrQxfcw46033287 = -323969968;    double ftoMXJojsWSrQxfcw65287768 = -808852605;    double ftoMXJojsWSrQxfcw30403446 = -445080600;    double ftoMXJojsWSrQxfcw53573333 = -896317192;    double ftoMXJojsWSrQxfcw61957542 = -603372509;    double ftoMXJojsWSrQxfcw88061340 = -383969910;    double ftoMXJojsWSrQxfcw9364072 = -369002563;    double ftoMXJojsWSrQxfcw22886196 = -549545182;    double ftoMXJojsWSrQxfcw82328280 = -846310291;    double ftoMXJojsWSrQxfcw99347898 = -164381125;    double ftoMXJojsWSrQxfcw36942836 = -797655524;    double ftoMXJojsWSrQxfcw70214854 = -281334231;    double ftoMXJojsWSrQxfcw20128869 = -263909722;    double ftoMXJojsWSrQxfcw29844340 = -725279568;    double ftoMXJojsWSrQxfcw87066703 = -17015266;    double ftoMXJojsWSrQxfcw95200870 = -213313516;    double ftoMXJojsWSrQxfcw36382750 = -41118126;    double ftoMXJojsWSrQxfcw13749025 = -6839718;    double ftoMXJojsWSrQxfcw20569622 = -735580564;    double ftoMXJojsWSrQxfcw5169578 = -400413925;    double ftoMXJojsWSrQxfcw24337462 = -427487049;    double ftoMXJojsWSrQxfcw91959556 = -125145429;    double ftoMXJojsWSrQxfcw48273113 = -563856453;    double ftoMXJojsWSrQxfcw51521218 = -88556531;    double ftoMXJojsWSrQxfcw55096566 = -601365603;    double ftoMXJojsWSrQxfcw73711300 = 54248157;    double ftoMXJojsWSrQxfcw73153079 = -948852968;    double ftoMXJojsWSrQxfcw67382236 = -644193728;    double ftoMXJojsWSrQxfcw12367817 = -352573806;    double ftoMXJojsWSrQxfcw67785371 = -694041400;    double ftoMXJojsWSrQxfcw40563940 = -735937388;    double ftoMXJojsWSrQxfcw79052892 = 33350011;    double ftoMXJojsWSrQxfcw50077746 = -847389834;    double ftoMXJojsWSrQxfcw62678597 = -666715563;    double ftoMXJojsWSrQxfcw50529787 = -867506032;    double ftoMXJojsWSrQxfcw83920933 = -103423469;    double ftoMXJojsWSrQxfcw39121178 = 22384987;    double ftoMXJojsWSrQxfcw15104893 = -616929172;     ftoMXJojsWSrQxfcw35913588 = ftoMXJojsWSrQxfcw52063101;     ftoMXJojsWSrQxfcw52063101 = ftoMXJojsWSrQxfcw18724317;     ftoMXJojsWSrQxfcw18724317 = ftoMXJojsWSrQxfcw60568018;     ftoMXJojsWSrQxfcw60568018 = ftoMXJojsWSrQxfcw90720642;     ftoMXJojsWSrQxfcw90720642 = ftoMXJojsWSrQxfcw38844597;     ftoMXJojsWSrQxfcw38844597 = ftoMXJojsWSrQxfcw71863934;     ftoMXJojsWSrQxfcw71863934 = ftoMXJojsWSrQxfcw76601901;     ftoMXJojsWSrQxfcw76601901 = ftoMXJojsWSrQxfcw9500489;     ftoMXJojsWSrQxfcw9500489 = ftoMXJojsWSrQxfcw58423786;     ftoMXJojsWSrQxfcw58423786 = ftoMXJojsWSrQxfcw60479137;     ftoMXJojsWSrQxfcw60479137 = ftoMXJojsWSrQxfcw30625017;     ftoMXJojsWSrQxfcw30625017 = ftoMXJojsWSrQxfcw20820530;     ftoMXJojsWSrQxfcw20820530 = ftoMXJojsWSrQxfcw76384490;     ftoMXJojsWSrQxfcw76384490 = ftoMXJojsWSrQxfcw22591619;     ftoMXJojsWSrQxfcw22591619 = ftoMXJojsWSrQxfcw63776993;     ftoMXJojsWSrQxfcw63776993 = ftoMXJojsWSrQxfcw89948669;     ftoMXJojsWSrQxfcw89948669 = ftoMXJojsWSrQxfcw9774932;     ftoMXJojsWSrQxfcw9774932 = ftoMXJojsWSrQxfcw34935050;     ftoMXJojsWSrQxfcw34935050 = ftoMXJojsWSrQxfcw52022668;     ftoMXJojsWSrQxfcw52022668 = ftoMXJojsWSrQxfcw26407413;     ftoMXJojsWSrQxfcw26407413 = ftoMXJojsWSrQxfcw36275405;     ftoMXJojsWSrQxfcw36275405 = ftoMXJojsWSrQxfcw64725494;     ftoMXJojsWSrQxfcw64725494 = ftoMXJojsWSrQxfcw5060976;     ftoMXJojsWSrQxfcw5060976 = ftoMXJojsWSrQxfcw10531188;     ftoMXJojsWSrQxfcw10531188 = ftoMXJojsWSrQxfcw96941439;     ftoMXJojsWSrQxfcw96941439 = ftoMXJojsWSrQxfcw24202148;     ftoMXJojsWSrQxfcw24202148 = ftoMXJojsWSrQxfcw2648732;     ftoMXJojsWSrQxfcw2648732 = ftoMXJojsWSrQxfcw30568614;     ftoMXJojsWSrQxfcw30568614 = ftoMXJojsWSrQxfcw44212721;     ftoMXJojsWSrQxfcw44212721 = ftoMXJojsWSrQxfcw28020341;     ftoMXJojsWSrQxfcw28020341 = ftoMXJojsWSrQxfcw6905805;     ftoMXJojsWSrQxfcw6905805 = ftoMXJojsWSrQxfcw68667475;     ftoMXJojsWSrQxfcw68667475 = ftoMXJojsWSrQxfcw32759190;     ftoMXJojsWSrQxfcw32759190 = ftoMXJojsWSrQxfcw67020418;     ftoMXJojsWSrQxfcw67020418 = ftoMXJojsWSrQxfcw99705423;     ftoMXJojsWSrQxfcw99705423 = ftoMXJojsWSrQxfcw81448712;     ftoMXJojsWSrQxfcw81448712 = ftoMXJojsWSrQxfcw90600771;     ftoMXJojsWSrQxfcw90600771 = ftoMXJojsWSrQxfcw72832095;     ftoMXJojsWSrQxfcw72832095 = ftoMXJojsWSrQxfcw64720196;     ftoMXJojsWSrQxfcw64720196 = ftoMXJojsWSrQxfcw31893800;     ftoMXJojsWSrQxfcw31893800 = ftoMXJojsWSrQxfcw96563073;     ftoMXJojsWSrQxfcw96563073 = ftoMXJojsWSrQxfcw49208702;     ftoMXJojsWSrQxfcw49208702 = ftoMXJojsWSrQxfcw69524624;     ftoMXJojsWSrQxfcw69524624 = ftoMXJojsWSrQxfcw68678226;     ftoMXJojsWSrQxfcw68678226 = ftoMXJojsWSrQxfcw96782162;     ftoMXJojsWSrQxfcw96782162 = ftoMXJojsWSrQxfcw76371818;     ftoMXJojsWSrQxfcw76371818 = ftoMXJojsWSrQxfcw19032570;     ftoMXJojsWSrQxfcw19032570 = ftoMXJojsWSrQxfcw78311270;     ftoMXJojsWSrQxfcw78311270 = ftoMXJojsWSrQxfcw38609058;     ftoMXJojsWSrQxfcw38609058 = ftoMXJojsWSrQxfcw95939608;     ftoMXJojsWSrQxfcw95939608 = ftoMXJojsWSrQxfcw76499123;     ftoMXJojsWSrQxfcw76499123 = ftoMXJojsWSrQxfcw51809238;     ftoMXJojsWSrQxfcw51809238 = ftoMXJojsWSrQxfcw94956175;     ftoMXJojsWSrQxfcw94956175 = ftoMXJojsWSrQxfcw59606111;     ftoMXJojsWSrQxfcw59606111 = ftoMXJojsWSrQxfcw99638182;     ftoMXJojsWSrQxfcw99638182 = ftoMXJojsWSrQxfcw87337606;     ftoMXJojsWSrQxfcw87337606 = ftoMXJojsWSrQxfcw13663342;     ftoMXJojsWSrQxfcw13663342 = ftoMXJojsWSrQxfcw50036831;     ftoMXJojsWSrQxfcw50036831 = ftoMXJojsWSrQxfcw93779203;     ftoMXJojsWSrQxfcw93779203 = ftoMXJojsWSrQxfcw14642450;     ftoMXJojsWSrQxfcw14642450 = ftoMXJojsWSrQxfcw69215202;     ftoMXJojsWSrQxfcw69215202 = ftoMXJojsWSrQxfcw46033287;     ftoMXJojsWSrQxfcw46033287 = ftoMXJojsWSrQxfcw65287768;     ftoMXJojsWSrQxfcw65287768 = ftoMXJojsWSrQxfcw30403446;     ftoMXJojsWSrQxfcw30403446 = ftoMXJojsWSrQxfcw53573333;     ftoMXJojsWSrQxfcw53573333 = ftoMXJojsWSrQxfcw61957542;     ftoMXJojsWSrQxfcw61957542 = ftoMXJojsWSrQxfcw88061340;     ftoMXJojsWSrQxfcw88061340 = ftoMXJojsWSrQxfcw9364072;     ftoMXJojsWSrQxfcw9364072 = ftoMXJojsWSrQxfcw22886196;     ftoMXJojsWSrQxfcw22886196 = ftoMXJojsWSrQxfcw82328280;     ftoMXJojsWSrQxfcw82328280 = ftoMXJojsWSrQxfcw99347898;     ftoMXJojsWSrQxfcw99347898 = ftoMXJojsWSrQxfcw36942836;     ftoMXJojsWSrQxfcw36942836 = ftoMXJojsWSrQxfcw70214854;     ftoMXJojsWSrQxfcw70214854 = ftoMXJojsWSrQxfcw20128869;     ftoMXJojsWSrQxfcw20128869 = ftoMXJojsWSrQxfcw29844340;     ftoMXJojsWSrQxfcw29844340 = ftoMXJojsWSrQxfcw87066703;     ftoMXJojsWSrQxfcw87066703 = ftoMXJojsWSrQxfcw95200870;     ftoMXJojsWSrQxfcw95200870 = ftoMXJojsWSrQxfcw36382750;     ftoMXJojsWSrQxfcw36382750 = ftoMXJojsWSrQxfcw13749025;     ftoMXJojsWSrQxfcw13749025 = ftoMXJojsWSrQxfcw20569622;     ftoMXJojsWSrQxfcw20569622 = ftoMXJojsWSrQxfcw5169578;     ftoMXJojsWSrQxfcw5169578 = ftoMXJojsWSrQxfcw24337462;     ftoMXJojsWSrQxfcw24337462 = ftoMXJojsWSrQxfcw91959556;     ftoMXJojsWSrQxfcw91959556 = ftoMXJojsWSrQxfcw48273113;     ftoMXJojsWSrQxfcw48273113 = ftoMXJojsWSrQxfcw51521218;     ftoMXJojsWSrQxfcw51521218 = ftoMXJojsWSrQxfcw55096566;     ftoMXJojsWSrQxfcw55096566 = ftoMXJojsWSrQxfcw73711300;     ftoMXJojsWSrQxfcw73711300 = ftoMXJojsWSrQxfcw73153079;     ftoMXJojsWSrQxfcw73153079 = ftoMXJojsWSrQxfcw67382236;     ftoMXJojsWSrQxfcw67382236 = ftoMXJojsWSrQxfcw12367817;     ftoMXJojsWSrQxfcw12367817 = ftoMXJojsWSrQxfcw67785371;     ftoMXJojsWSrQxfcw67785371 = ftoMXJojsWSrQxfcw40563940;     ftoMXJojsWSrQxfcw40563940 = ftoMXJojsWSrQxfcw79052892;     ftoMXJojsWSrQxfcw79052892 = ftoMXJojsWSrQxfcw50077746;     ftoMXJojsWSrQxfcw50077746 = ftoMXJojsWSrQxfcw62678597;     ftoMXJojsWSrQxfcw62678597 = ftoMXJojsWSrQxfcw50529787;     ftoMXJojsWSrQxfcw50529787 = ftoMXJojsWSrQxfcw83920933;     ftoMXJojsWSrQxfcw83920933 = ftoMXJojsWSrQxfcw39121178;     ftoMXJojsWSrQxfcw39121178 = ftoMXJojsWSrQxfcw15104893;     ftoMXJojsWSrQxfcw15104893 = ftoMXJojsWSrQxfcw35913588;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zWPEglDXygljsuvB83621550() {     double gODgepciCbUElxNVC39105276 = -280082451;    double gODgepciCbUElxNVC2685061 = -378114592;    double gODgepciCbUElxNVC24147426 = -413582829;    double gODgepciCbUElxNVC76363120 = -211780374;    double gODgepciCbUElxNVC70728495 = -305132054;    double gODgepciCbUElxNVC75776189 = 68703370;    double gODgepciCbUElxNVC53576376 = 70298796;    double gODgepciCbUElxNVC85192217 = -816724329;    double gODgepciCbUElxNVC20406815 = -502573702;    double gODgepciCbUElxNVC82458098 = -901247579;    double gODgepciCbUElxNVC57591437 = -34989285;    double gODgepciCbUElxNVC24331402 = 85062125;    double gODgepciCbUElxNVC62246041 = -417103007;    double gODgepciCbUElxNVC79173675 = -392802712;    double gODgepciCbUElxNVC87297339 = -418475699;    double gODgepciCbUElxNVC1806671 = -183117997;    double gODgepciCbUElxNVC95776193 = -836999403;    double gODgepciCbUElxNVC47914952 = -866795253;    double gODgepciCbUElxNVC75546410 = -978692795;    double gODgepciCbUElxNVC6741615 = -466124866;    double gODgepciCbUElxNVC6202996 = -97434413;    double gODgepciCbUElxNVC52259559 = -454249838;    double gODgepciCbUElxNVC2074366 = -62978484;    double gODgepciCbUElxNVC23925339 = -813976448;    double gODgepciCbUElxNVC58336769 = -156928706;    double gODgepciCbUElxNVC54701658 = -968790244;    double gODgepciCbUElxNVC29722218 = -462402815;    double gODgepciCbUElxNVC96039606 = -313385395;    double gODgepciCbUElxNVC44231712 = -216598856;    double gODgepciCbUElxNVC64467902 = -32092090;    double gODgepciCbUElxNVC39188849 = -211964032;    double gODgepciCbUElxNVC71278403 = -978942777;    double gODgepciCbUElxNVC94268279 = -503545248;    double gODgepciCbUElxNVC27240411 = -374643432;    double gODgepciCbUElxNVC74554080 = 71175025;    double gODgepciCbUElxNVC5632595 = -927114507;    double gODgepciCbUElxNVC49595753 = -796705492;    double gODgepciCbUElxNVC82465036 = -218315767;    double gODgepciCbUElxNVC761140 = -277507144;    double gODgepciCbUElxNVC25191048 = -374682541;    double gODgepciCbUElxNVC38143378 = 71787807;    double gODgepciCbUElxNVC37761933 = -36397894;    double gODgepciCbUElxNVC53142089 = -612482125;    double gODgepciCbUElxNVC105683 = -550712006;    double gODgepciCbUElxNVC76693887 = 99781280;    double gODgepciCbUElxNVC95510171 = -929821231;    double gODgepciCbUElxNVC91540667 = 60213335;    double gODgepciCbUElxNVC57930247 = -898371748;    double gODgepciCbUElxNVC53321074 = -649446969;    double gODgepciCbUElxNVC15380731 = -946374162;    double gODgepciCbUElxNVC10466009 = -276327902;    double gODgepciCbUElxNVC44453703 = -68389033;    double gODgepciCbUElxNVC67016787 = 47927739;    double gODgepciCbUElxNVC13818810 = -908674372;    double gODgepciCbUElxNVC77418555 = -908376191;    double gODgepciCbUElxNVC86845716 = -825832614;    double gODgepciCbUElxNVC610695 = -215136108;    double gODgepciCbUElxNVC222088 = -599606382;    double gODgepciCbUElxNVC18026351 = 45148332;    double gODgepciCbUElxNVC16026837 = -336341810;    double gODgepciCbUElxNVC46053971 = -468893815;    double gODgepciCbUElxNVC57536769 = -616315809;    double gODgepciCbUElxNVC40960505 = -500125473;    double gODgepciCbUElxNVC55938912 = -370481612;    double gODgepciCbUElxNVC43269249 = -589283548;    double gODgepciCbUElxNVC86313033 = -56046509;    double gODgepciCbUElxNVC30063122 = -411392627;    double gODgepciCbUElxNVC35005630 = 57540424;    double gODgepciCbUElxNVC4619596 = -363977737;    double gODgepciCbUElxNVC81664744 = -491361193;    double gODgepciCbUElxNVC52210917 = -386412505;    double gODgepciCbUElxNVC13311157 = -518683637;    double gODgepciCbUElxNVC47153813 = -489288109;    double gODgepciCbUElxNVC50355362 = -504010254;    double gODgepciCbUElxNVC68598237 = -437912673;    double gODgepciCbUElxNVC68441062 = 38963481;    double gODgepciCbUElxNVC99117469 = -841767713;    double gODgepciCbUElxNVC1968684 = -512266478;    double gODgepciCbUElxNVC47231451 = -813757729;    double gODgepciCbUElxNVC62826598 = -227107476;    double gODgepciCbUElxNVC63160991 = -929003580;    double gODgepciCbUElxNVC71791971 = -564031067;    double gODgepciCbUElxNVC42718533 = -663938426;    double gODgepciCbUElxNVC28850982 = -270224694;    double gODgepciCbUElxNVC54001894 = -755764189;    double gODgepciCbUElxNVC94735145 = -43574999;    double gODgepciCbUElxNVC4261617 = -926870517;    double gODgepciCbUElxNVC80449470 = -594870877;    double gODgepciCbUElxNVC49821856 = -466267242;    double gODgepciCbUElxNVC87708363 = -102992362;    double gODgepciCbUElxNVC5021900 = -611978399;    double gODgepciCbUElxNVC49373666 = -97099111;    double gODgepciCbUElxNVC64438685 = -163464099;    double gODgepciCbUElxNVC84734303 = -941165334;    double gODgepciCbUElxNVC79137076 = -905788727;    double gODgepciCbUElxNVC80606608 = -311896384;    double gODgepciCbUElxNVC96801428 = -536272421;    double gODgepciCbUElxNVC97203176 = -142000514;    double gODgepciCbUElxNVC56836433 = -961428459;    double gODgepciCbUElxNVC90380854 = -280082451;     gODgepciCbUElxNVC39105276 = gODgepciCbUElxNVC2685061;     gODgepciCbUElxNVC2685061 = gODgepciCbUElxNVC24147426;     gODgepciCbUElxNVC24147426 = gODgepciCbUElxNVC76363120;     gODgepciCbUElxNVC76363120 = gODgepciCbUElxNVC70728495;     gODgepciCbUElxNVC70728495 = gODgepciCbUElxNVC75776189;     gODgepciCbUElxNVC75776189 = gODgepciCbUElxNVC53576376;     gODgepciCbUElxNVC53576376 = gODgepciCbUElxNVC85192217;     gODgepciCbUElxNVC85192217 = gODgepciCbUElxNVC20406815;     gODgepciCbUElxNVC20406815 = gODgepciCbUElxNVC82458098;     gODgepciCbUElxNVC82458098 = gODgepciCbUElxNVC57591437;     gODgepciCbUElxNVC57591437 = gODgepciCbUElxNVC24331402;     gODgepciCbUElxNVC24331402 = gODgepciCbUElxNVC62246041;     gODgepciCbUElxNVC62246041 = gODgepciCbUElxNVC79173675;     gODgepciCbUElxNVC79173675 = gODgepciCbUElxNVC87297339;     gODgepciCbUElxNVC87297339 = gODgepciCbUElxNVC1806671;     gODgepciCbUElxNVC1806671 = gODgepciCbUElxNVC95776193;     gODgepciCbUElxNVC95776193 = gODgepciCbUElxNVC47914952;     gODgepciCbUElxNVC47914952 = gODgepciCbUElxNVC75546410;     gODgepciCbUElxNVC75546410 = gODgepciCbUElxNVC6741615;     gODgepciCbUElxNVC6741615 = gODgepciCbUElxNVC6202996;     gODgepciCbUElxNVC6202996 = gODgepciCbUElxNVC52259559;     gODgepciCbUElxNVC52259559 = gODgepciCbUElxNVC2074366;     gODgepciCbUElxNVC2074366 = gODgepciCbUElxNVC23925339;     gODgepciCbUElxNVC23925339 = gODgepciCbUElxNVC58336769;     gODgepciCbUElxNVC58336769 = gODgepciCbUElxNVC54701658;     gODgepciCbUElxNVC54701658 = gODgepciCbUElxNVC29722218;     gODgepciCbUElxNVC29722218 = gODgepciCbUElxNVC96039606;     gODgepciCbUElxNVC96039606 = gODgepciCbUElxNVC44231712;     gODgepciCbUElxNVC44231712 = gODgepciCbUElxNVC64467902;     gODgepciCbUElxNVC64467902 = gODgepciCbUElxNVC39188849;     gODgepciCbUElxNVC39188849 = gODgepciCbUElxNVC71278403;     gODgepciCbUElxNVC71278403 = gODgepciCbUElxNVC94268279;     gODgepciCbUElxNVC94268279 = gODgepciCbUElxNVC27240411;     gODgepciCbUElxNVC27240411 = gODgepciCbUElxNVC74554080;     gODgepciCbUElxNVC74554080 = gODgepciCbUElxNVC5632595;     gODgepciCbUElxNVC5632595 = gODgepciCbUElxNVC49595753;     gODgepciCbUElxNVC49595753 = gODgepciCbUElxNVC82465036;     gODgepciCbUElxNVC82465036 = gODgepciCbUElxNVC761140;     gODgepciCbUElxNVC761140 = gODgepciCbUElxNVC25191048;     gODgepciCbUElxNVC25191048 = gODgepciCbUElxNVC38143378;     gODgepciCbUElxNVC38143378 = gODgepciCbUElxNVC37761933;     gODgepciCbUElxNVC37761933 = gODgepciCbUElxNVC53142089;     gODgepciCbUElxNVC53142089 = gODgepciCbUElxNVC105683;     gODgepciCbUElxNVC105683 = gODgepciCbUElxNVC76693887;     gODgepciCbUElxNVC76693887 = gODgepciCbUElxNVC95510171;     gODgepciCbUElxNVC95510171 = gODgepciCbUElxNVC91540667;     gODgepciCbUElxNVC91540667 = gODgepciCbUElxNVC57930247;     gODgepciCbUElxNVC57930247 = gODgepciCbUElxNVC53321074;     gODgepciCbUElxNVC53321074 = gODgepciCbUElxNVC15380731;     gODgepciCbUElxNVC15380731 = gODgepciCbUElxNVC10466009;     gODgepciCbUElxNVC10466009 = gODgepciCbUElxNVC44453703;     gODgepciCbUElxNVC44453703 = gODgepciCbUElxNVC67016787;     gODgepciCbUElxNVC67016787 = gODgepciCbUElxNVC13818810;     gODgepciCbUElxNVC13818810 = gODgepciCbUElxNVC77418555;     gODgepciCbUElxNVC77418555 = gODgepciCbUElxNVC86845716;     gODgepciCbUElxNVC86845716 = gODgepciCbUElxNVC610695;     gODgepciCbUElxNVC610695 = gODgepciCbUElxNVC222088;     gODgepciCbUElxNVC222088 = gODgepciCbUElxNVC18026351;     gODgepciCbUElxNVC18026351 = gODgepciCbUElxNVC16026837;     gODgepciCbUElxNVC16026837 = gODgepciCbUElxNVC46053971;     gODgepciCbUElxNVC46053971 = gODgepciCbUElxNVC57536769;     gODgepciCbUElxNVC57536769 = gODgepciCbUElxNVC40960505;     gODgepciCbUElxNVC40960505 = gODgepciCbUElxNVC55938912;     gODgepciCbUElxNVC55938912 = gODgepciCbUElxNVC43269249;     gODgepciCbUElxNVC43269249 = gODgepciCbUElxNVC86313033;     gODgepciCbUElxNVC86313033 = gODgepciCbUElxNVC30063122;     gODgepciCbUElxNVC30063122 = gODgepciCbUElxNVC35005630;     gODgepciCbUElxNVC35005630 = gODgepciCbUElxNVC4619596;     gODgepciCbUElxNVC4619596 = gODgepciCbUElxNVC81664744;     gODgepciCbUElxNVC81664744 = gODgepciCbUElxNVC52210917;     gODgepciCbUElxNVC52210917 = gODgepciCbUElxNVC13311157;     gODgepciCbUElxNVC13311157 = gODgepciCbUElxNVC47153813;     gODgepciCbUElxNVC47153813 = gODgepciCbUElxNVC50355362;     gODgepciCbUElxNVC50355362 = gODgepciCbUElxNVC68598237;     gODgepciCbUElxNVC68598237 = gODgepciCbUElxNVC68441062;     gODgepciCbUElxNVC68441062 = gODgepciCbUElxNVC99117469;     gODgepciCbUElxNVC99117469 = gODgepciCbUElxNVC1968684;     gODgepciCbUElxNVC1968684 = gODgepciCbUElxNVC47231451;     gODgepciCbUElxNVC47231451 = gODgepciCbUElxNVC62826598;     gODgepciCbUElxNVC62826598 = gODgepciCbUElxNVC63160991;     gODgepciCbUElxNVC63160991 = gODgepciCbUElxNVC71791971;     gODgepciCbUElxNVC71791971 = gODgepciCbUElxNVC42718533;     gODgepciCbUElxNVC42718533 = gODgepciCbUElxNVC28850982;     gODgepciCbUElxNVC28850982 = gODgepciCbUElxNVC54001894;     gODgepciCbUElxNVC54001894 = gODgepciCbUElxNVC94735145;     gODgepciCbUElxNVC94735145 = gODgepciCbUElxNVC4261617;     gODgepciCbUElxNVC4261617 = gODgepciCbUElxNVC80449470;     gODgepciCbUElxNVC80449470 = gODgepciCbUElxNVC49821856;     gODgepciCbUElxNVC49821856 = gODgepciCbUElxNVC87708363;     gODgepciCbUElxNVC87708363 = gODgepciCbUElxNVC5021900;     gODgepciCbUElxNVC5021900 = gODgepciCbUElxNVC49373666;     gODgepciCbUElxNVC49373666 = gODgepciCbUElxNVC64438685;     gODgepciCbUElxNVC64438685 = gODgepciCbUElxNVC84734303;     gODgepciCbUElxNVC84734303 = gODgepciCbUElxNVC79137076;     gODgepciCbUElxNVC79137076 = gODgepciCbUElxNVC80606608;     gODgepciCbUElxNVC80606608 = gODgepciCbUElxNVC96801428;     gODgepciCbUElxNVC96801428 = gODgepciCbUElxNVC97203176;     gODgepciCbUElxNVC97203176 = gODgepciCbUElxNVC56836433;     gODgepciCbUElxNVC56836433 = gODgepciCbUElxNVC90380854;     gODgepciCbUElxNVC90380854 = gODgepciCbUElxNVC39105276;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zcrCvUHXEbzMBDwf98670618() {     double IqhnzPBvnYevnWmGP45222383 = -391984340;    double IqhnzPBvnYevnWmGP46058340 = -87714372;    double IqhnzPBvnYevnWmGP40189610 = -564415751;    double IqhnzPBvnYevnWmGP3437902 = -402529004;    double IqhnzPBvnYevnWmGP39623984 = -825781925;    double IqhnzPBvnYevnWmGP67193391 = -195792865;    double IqhnzPBvnYevnWmGP14065342 = -68666181;    double IqhnzPBvnYevnWmGP34952108 = -3598242;    double IqhnzPBvnYevnWmGP85049310 = -270606367;    double IqhnzPBvnYevnWmGP23687757 = -130223057;    double IqhnzPBvnYevnWmGP17701727 = 87573128;    double IqhnzPBvnYevnWmGP56154444 = -359344076;    double IqhnzPBvnYevnWmGP48455160 = -856994096;    double IqhnzPBvnYevnWmGP62317352 = -925041280;    double IqhnzPBvnYevnWmGP83039765 = -8937866;    double IqhnzPBvnYevnWmGP43312064 = -156117540;    double IqhnzPBvnYevnWmGP89612196 = -353544516;    double IqhnzPBvnYevnWmGP44968105 = -532317895;    double IqhnzPBvnYevnWmGP37376252 = -284461892;    double IqhnzPBvnYevnWmGP87291779 = -823027785;    double IqhnzPBvnYevnWmGP94756188 = 47078131;    double IqhnzPBvnYevnWmGP38780583 = -362179554;    double IqhnzPBvnYevnWmGP37665157 = -909495971;    double IqhnzPBvnYevnWmGP7917713 = 63718564;    double IqhnzPBvnYevnWmGP2036420 = -530124739;    double IqhnzPBvnYevnWmGP79423334 = -847445568;    double IqhnzPBvnYevnWmGP28181996 = -363065018;    double IqhnzPBvnYevnWmGP9146733 = -379314349;    double IqhnzPBvnYevnWmGP10363968 = -438981560;    double IqhnzPBvnYevnWmGP33083539 = -949562958;    double IqhnzPBvnYevnWmGP31431399 = -125284970;    double IqhnzPBvnYevnWmGP68889025 = -688389592;    double IqhnzPBvnYevnWmGP3979229 = -396158618;    double IqhnzPBvnYevnWmGP22070975 = -126185197;    double IqhnzPBvnYevnWmGP36028732 = -567558081;    double IqhnzPBvnYevnWmGP74358507 = -41140615;    double IqhnzPBvnYevnWmGP30857688 = -750341362;    double IqhnzPBvnYevnWmGP16555249 = 10178223;    double IqhnzPBvnYevnWmGP74371393 = -271430641;    double IqhnzPBvnYevnWmGP12692986 = -848121764;    double IqhnzPBvnYevnWmGP43812601 = 6513913;    double IqhnzPBvnYevnWmGP69912390 = -306540172;    double IqhnzPBvnYevnWmGP31209068 = -693286709;    double IqhnzPBvnYevnWmGP87513422 = -570054944;    double IqhnzPBvnYevnWmGP88646352 = -662116874;    double IqhnzPBvnYevnWmGP7449367 = -915037347;    double IqhnzPBvnYevnWmGP9319242 = -58562202;    double IqhnzPBvnYevnWmGP65887188 = -908267281;    double IqhnzPBvnYevnWmGP62996803 = -735183957;    double IqhnzPBvnYevnWmGP28292383 = 66615030;    double IqhnzPBvnYevnWmGP46166687 = -118621573;    double IqhnzPBvnYevnWmGP55263423 = -969115353;    double IqhnzPBvnYevnWmGP15265245 = -843861848;    double IqhnzPBvnYevnWmGP9787690 = -129144517;    double IqhnzPBvnYevnWmGP3914430 = -508907973;    double IqhnzPBvnYevnWmGP6441800 = 70195214;    double IqhnzPBvnYevnWmGP8393183 = -178218402;    double IqhnzPBvnYevnWmGP32271897 = -528134315;    double IqhnzPBvnYevnWmGP1401482 = -872404266;    double IqhnzPBvnYevnWmGP60200650 = -978336358;    double IqhnzPBvnYevnWmGP39011395 = -832727847;    double IqhnzPBvnYevnWmGP4918610 = -689351833;    double IqhnzPBvnYevnWmGP24588140 = -564616682;    double IqhnzPBvnYevnWmGP51965771 = -321043410;    double IqhnzPBvnYevnWmGP92256358 = 95061913;    double IqhnzPBvnYevnWmGP48812702 = -224037280;    double IqhnzPBvnYevnWmGP52175215 = -963185459;    double IqhnzPBvnYevnWmGP26384185 = -630808900;    double IqhnzPBvnYevnWmGP26288620 = -257483200;    double IqhnzPBvnYevnWmGP8681259 = -967797251;    double IqhnzPBvnYevnWmGP12454376 = -405776179;    double IqhnzPBvnYevnWmGP73056947 = -263722740;    double IqhnzPBvnYevnWmGP70596712 = -160887255;    double IqhnzPBvnYevnWmGP24683267 = -436340128;    double IqhnzPBvnYevnWmGP43479178 = -729541698;    double IqhnzPBvnYevnWmGP24843799 = -646381697;    double IqhnzPBvnYevnWmGP7571516 = -668892846;    double IqhnzPBvnYevnWmGP50151735 = -239441027;    double IqhnzPBvnYevnWmGP19271361 = -274164563;    double IqhnzPBvnYevnWmGP94587052 = -615087392;    double IqhnzPBvnYevnWmGP70104092 = -688883366;    double IqhnzPBvnYevnWmGP62294808 = -454797738;    double IqhnzPBvnYevnWmGP46149929 = -644130392;    double IqhnzPBvnYevnWmGP82071585 = -405596591;    double IqhnzPBvnYevnWmGP86916852 = -730941385;    double IqhnzPBvnYevnWmGP76167975 = -156169617;    double IqhnzPBvnYevnWmGP53623781 = -844527744;    double IqhnzPBvnYevnWmGP94191538 = -167014102;    double IqhnzPBvnYevnWmGP18156546 = -617277224;    double IqhnzPBvnYevnWmGP29586933 = -537753295;    double IqhnzPBvnYevnWmGP65965325 = -862922213;    double IqhnzPBvnYevnWmGP98585791 = -122207047;    double IqhnzPBvnYevnWmGP15153768 = -117417512;    double IqhnzPBvnYevnWmGP14170744 = -293094283;    double IqhnzPBvnYevnWmGP73681590 = 84606082;    double IqhnzPBvnYevnWmGP38893992 = -304134255;    double IqhnzPBvnYevnWmGP45324250 = -741923491;    double IqhnzPBvnYevnWmGP79243296 = -272243300;    double IqhnzPBvnYevnWmGP95257063 = -565116857;    double IqhnzPBvnYevnWmGP39833651 = -391984340;     IqhnzPBvnYevnWmGP45222383 = IqhnzPBvnYevnWmGP46058340;     IqhnzPBvnYevnWmGP46058340 = IqhnzPBvnYevnWmGP40189610;     IqhnzPBvnYevnWmGP40189610 = IqhnzPBvnYevnWmGP3437902;     IqhnzPBvnYevnWmGP3437902 = IqhnzPBvnYevnWmGP39623984;     IqhnzPBvnYevnWmGP39623984 = IqhnzPBvnYevnWmGP67193391;     IqhnzPBvnYevnWmGP67193391 = IqhnzPBvnYevnWmGP14065342;     IqhnzPBvnYevnWmGP14065342 = IqhnzPBvnYevnWmGP34952108;     IqhnzPBvnYevnWmGP34952108 = IqhnzPBvnYevnWmGP85049310;     IqhnzPBvnYevnWmGP85049310 = IqhnzPBvnYevnWmGP23687757;     IqhnzPBvnYevnWmGP23687757 = IqhnzPBvnYevnWmGP17701727;     IqhnzPBvnYevnWmGP17701727 = IqhnzPBvnYevnWmGP56154444;     IqhnzPBvnYevnWmGP56154444 = IqhnzPBvnYevnWmGP48455160;     IqhnzPBvnYevnWmGP48455160 = IqhnzPBvnYevnWmGP62317352;     IqhnzPBvnYevnWmGP62317352 = IqhnzPBvnYevnWmGP83039765;     IqhnzPBvnYevnWmGP83039765 = IqhnzPBvnYevnWmGP43312064;     IqhnzPBvnYevnWmGP43312064 = IqhnzPBvnYevnWmGP89612196;     IqhnzPBvnYevnWmGP89612196 = IqhnzPBvnYevnWmGP44968105;     IqhnzPBvnYevnWmGP44968105 = IqhnzPBvnYevnWmGP37376252;     IqhnzPBvnYevnWmGP37376252 = IqhnzPBvnYevnWmGP87291779;     IqhnzPBvnYevnWmGP87291779 = IqhnzPBvnYevnWmGP94756188;     IqhnzPBvnYevnWmGP94756188 = IqhnzPBvnYevnWmGP38780583;     IqhnzPBvnYevnWmGP38780583 = IqhnzPBvnYevnWmGP37665157;     IqhnzPBvnYevnWmGP37665157 = IqhnzPBvnYevnWmGP7917713;     IqhnzPBvnYevnWmGP7917713 = IqhnzPBvnYevnWmGP2036420;     IqhnzPBvnYevnWmGP2036420 = IqhnzPBvnYevnWmGP79423334;     IqhnzPBvnYevnWmGP79423334 = IqhnzPBvnYevnWmGP28181996;     IqhnzPBvnYevnWmGP28181996 = IqhnzPBvnYevnWmGP9146733;     IqhnzPBvnYevnWmGP9146733 = IqhnzPBvnYevnWmGP10363968;     IqhnzPBvnYevnWmGP10363968 = IqhnzPBvnYevnWmGP33083539;     IqhnzPBvnYevnWmGP33083539 = IqhnzPBvnYevnWmGP31431399;     IqhnzPBvnYevnWmGP31431399 = IqhnzPBvnYevnWmGP68889025;     IqhnzPBvnYevnWmGP68889025 = IqhnzPBvnYevnWmGP3979229;     IqhnzPBvnYevnWmGP3979229 = IqhnzPBvnYevnWmGP22070975;     IqhnzPBvnYevnWmGP22070975 = IqhnzPBvnYevnWmGP36028732;     IqhnzPBvnYevnWmGP36028732 = IqhnzPBvnYevnWmGP74358507;     IqhnzPBvnYevnWmGP74358507 = IqhnzPBvnYevnWmGP30857688;     IqhnzPBvnYevnWmGP30857688 = IqhnzPBvnYevnWmGP16555249;     IqhnzPBvnYevnWmGP16555249 = IqhnzPBvnYevnWmGP74371393;     IqhnzPBvnYevnWmGP74371393 = IqhnzPBvnYevnWmGP12692986;     IqhnzPBvnYevnWmGP12692986 = IqhnzPBvnYevnWmGP43812601;     IqhnzPBvnYevnWmGP43812601 = IqhnzPBvnYevnWmGP69912390;     IqhnzPBvnYevnWmGP69912390 = IqhnzPBvnYevnWmGP31209068;     IqhnzPBvnYevnWmGP31209068 = IqhnzPBvnYevnWmGP87513422;     IqhnzPBvnYevnWmGP87513422 = IqhnzPBvnYevnWmGP88646352;     IqhnzPBvnYevnWmGP88646352 = IqhnzPBvnYevnWmGP7449367;     IqhnzPBvnYevnWmGP7449367 = IqhnzPBvnYevnWmGP9319242;     IqhnzPBvnYevnWmGP9319242 = IqhnzPBvnYevnWmGP65887188;     IqhnzPBvnYevnWmGP65887188 = IqhnzPBvnYevnWmGP62996803;     IqhnzPBvnYevnWmGP62996803 = IqhnzPBvnYevnWmGP28292383;     IqhnzPBvnYevnWmGP28292383 = IqhnzPBvnYevnWmGP46166687;     IqhnzPBvnYevnWmGP46166687 = IqhnzPBvnYevnWmGP55263423;     IqhnzPBvnYevnWmGP55263423 = IqhnzPBvnYevnWmGP15265245;     IqhnzPBvnYevnWmGP15265245 = IqhnzPBvnYevnWmGP9787690;     IqhnzPBvnYevnWmGP9787690 = IqhnzPBvnYevnWmGP3914430;     IqhnzPBvnYevnWmGP3914430 = IqhnzPBvnYevnWmGP6441800;     IqhnzPBvnYevnWmGP6441800 = IqhnzPBvnYevnWmGP8393183;     IqhnzPBvnYevnWmGP8393183 = IqhnzPBvnYevnWmGP32271897;     IqhnzPBvnYevnWmGP32271897 = IqhnzPBvnYevnWmGP1401482;     IqhnzPBvnYevnWmGP1401482 = IqhnzPBvnYevnWmGP60200650;     IqhnzPBvnYevnWmGP60200650 = IqhnzPBvnYevnWmGP39011395;     IqhnzPBvnYevnWmGP39011395 = IqhnzPBvnYevnWmGP4918610;     IqhnzPBvnYevnWmGP4918610 = IqhnzPBvnYevnWmGP24588140;     IqhnzPBvnYevnWmGP24588140 = IqhnzPBvnYevnWmGP51965771;     IqhnzPBvnYevnWmGP51965771 = IqhnzPBvnYevnWmGP92256358;     IqhnzPBvnYevnWmGP92256358 = IqhnzPBvnYevnWmGP48812702;     IqhnzPBvnYevnWmGP48812702 = IqhnzPBvnYevnWmGP52175215;     IqhnzPBvnYevnWmGP52175215 = IqhnzPBvnYevnWmGP26384185;     IqhnzPBvnYevnWmGP26384185 = IqhnzPBvnYevnWmGP26288620;     IqhnzPBvnYevnWmGP26288620 = IqhnzPBvnYevnWmGP8681259;     IqhnzPBvnYevnWmGP8681259 = IqhnzPBvnYevnWmGP12454376;     IqhnzPBvnYevnWmGP12454376 = IqhnzPBvnYevnWmGP73056947;     IqhnzPBvnYevnWmGP73056947 = IqhnzPBvnYevnWmGP70596712;     IqhnzPBvnYevnWmGP70596712 = IqhnzPBvnYevnWmGP24683267;     IqhnzPBvnYevnWmGP24683267 = IqhnzPBvnYevnWmGP43479178;     IqhnzPBvnYevnWmGP43479178 = IqhnzPBvnYevnWmGP24843799;     IqhnzPBvnYevnWmGP24843799 = IqhnzPBvnYevnWmGP7571516;     IqhnzPBvnYevnWmGP7571516 = IqhnzPBvnYevnWmGP50151735;     IqhnzPBvnYevnWmGP50151735 = IqhnzPBvnYevnWmGP19271361;     IqhnzPBvnYevnWmGP19271361 = IqhnzPBvnYevnWmGP94587052;     IqhnzPBvnYevnWmGP94587052 = IqhnzPBvnYevnWmGP70104092;     IqhnzPBvnYevnWmGP70104092 = IqhnzPBvnYevnWmGP62294808;     IqhnzPBvnYevnWmGP62294808 = IqhnzPBvnYevnWmGP46149929;     IqhnzPBvnYevnWmGP46149929 = IqhnzPBvnYevnWmGP82071585;     IqhnzPBvnYevnWmGP82071585 = IqhnzPBvnYevnWmGP86916852;     IqhnzPBvnYevnWmGP86916852 = IqhnzPBvnYevnWmGP76167975;     IqhnzPBvnYevnWmGP76167975 = IqhnzPBvnYevnWmGP53623781;     IqhnzPBvnYevnWmGP53623781 = IqhnzPBvnYevnWmGP94191538;     IqhnzPBvnYevnWmGP94191538 = IqhnzPBvnYevnWmGP18156546;     IqhnzPBvnYevnWmGP18156546 = IqhnzPBvnYevnWmGP29586933;     IqhnzPBvnYevnWmGP29586933 = IqhnzPBvnYevnWmGP65965325;     IqhnzPBvnYevnWmGP65965325 = IqhnzPBvnYevnWmGP98585791;     IqhnzPBvnYevnWmGP98585791 = IqhnzPBvnYevnWmGP15153768;     IqhnzPBvnYevnWmGP15153768 = IqhnzPBvnYevnWmGP14170744;     IqhnzPBvnYevnWmGP14170744 = IqhnzPBvnYevnWmGP73681590;     IqhnzPBvnYevnWmGP73681590 = IqhnzPBvnYevnWmGP38893992;     IqhnzPBvnYevnWmGP38893992 = IqhnzPBvnYevnWmGP45324250;     IqhnzPBvnYevnWmGP45324250 = IqhnzPBvnYevnWmGP79243296;     IqhnzPBvnYevnWmGP79243296 = IqhnzPBvnYevnWmGP95257063;     IqhnzPBvnYevnWmGP95257063 = IqhnzPBvnYevnWmGP39833651;     IqhnzPBvnYevnWmGP39833651 = IqhnzPBvnYevnWmGP45222383;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PlkWgRsRwtlqmiDb65962218() {     double oDrZHQTbPbgYMmvCE80681351 = -858261240;    double oDrZHQTbPbgYMmvCE59330418 = 31734021;    double oDrZHQTbPbgYMmvCE77576086 = -230082826;    double oDrZHQTbPbgYMmvCE75661256 = -223680076;    double oDrZHQTbPbgYMmvCE95909252 = -931380771;    double oDrZHQTbPbgYMmvCE3416264 = -458140760;    double oDrZHQTbPbgYMmvCE24722748 = 86608085;    double oDrZHQTbPbgYMmvCE97083668 = -747832382;    double oDrZHQTbPbgYMmvCE86566915 = -758937774;    double oDrZHQTbPbgYMmvCE38590469 = -894910055;    double oDrZHQTbPbgYMmvCE55883462 = -814434601;    double oDrZHQTbPbgYMmvCE56722592 = -223464848;    double oDrZHQTbPbgYMmvCE86689805 = -309006601;    double oDrZHQTbPbgYMmvCE15458216 = -901404689;    double oDrZHQTbPbgYMmvCE41234282 = -512362881;    double oDrZHQTbPbgYMmvCE26853365 = -891929147;    double oDrZHQTbPbgYMmvCE45448132 = -880519069;    double oDrZHQTbPbgYMmvCE44728297 = -605745964;    double oDrZHQTbPbgYMmvCE37926582 = -601025313;    double oDrZHQTbPbgYMmvCE81128997 = -149588851;    double oDrZHQTbPbgYMmvCE7843052 = -710196557;    double oDrZHQTbPbgYMmvCE42202108 = -448815848;    double oDrZHQTbPbgYMmvCE8190537 = -547128155;    double oDrZHQTbPbgYMmvCE1710658 = -447106040;    double oDrZHQTbPbgYMmvCE10054371 = -554109937;    double oDrZHQTbPbgYMmvCE90121505 = -536022374;    double oDrZHQTbPbgYMmvCE88602570 = -169349159;    double oDrZHQTbPbgYMmvCE49875157 = -549086523;    double oDrZHQTbPbgYMmvCE57032182 = 86923637;    double oDrZHQTbPbgYMmvCE38616950 = -984070128;    double oDrZHQTbPbgYMmvCE50014630 = -677265457;    double oDrZHQTbPbgYMmvCE6712044 = -461361007;    double oDrZHQTbPbgYMmvCE16197240 = -543754596;    double oDrZHQTbPbgYMmvCE51819402 = -190879011;    double oDrZHQTbPbgYMmvCE1837186 = -518658332;    double oDrZHQTbPbgYMmvCE25919352 = -255705921;    double oDrZHQTbPbgYMmvCE80269504 = -615037402;    double oDrZHQTbPbgYMmvCE67770652 = 27522642;    double oDrZHQTbPbgYMmvCE4761955 = 77693083;    double oDrZHQTbPbgYMmvCE76964517 = -313384310;    double oDrZHQTbPbgYMmvCE77264209 = -862685002;    double oDrZHQTbPbgYMmvCE59277517 = 98914537;    double oDrZHQTbPbgYMmvCE20592446 = -246467495;    double oDrZHQTbPbgYMmvCE27644761 = -74387354;    double oDrZHQTbPbgYMmvCE88662811 = -488628035;    double oDrZHQTbPbgYMmvCE53410695 = -953960979;    double oDrZHQTbPbgYMmvCE28664406 = -542548563;    double oDrZHQTbPbgYMmvCE83613500 = 25206126;    double oDrZHQTbPbgYMmvCE85889469 = -398920837;    double oDrZHQTbPbgYMmvCE52821972 = -424411786;    double oDrZHQTbPbgYMmvCE26937411 = 7101869;    double oDrZHQTbPbgYMmvCE38748895 = -82767700;    double oDrZHQTbPbgYMmvCE83886393 = -599907402;    double oDrZHQTbPbgYMmvCE25779537 = -320653016;    double oDrZHQTbPbgYMmvCE58119166 = -855398119;    double oDrZHQTbPbgYMmvCE38479244 = -309445392;    double oDrZHQTbPbgYMmvCE51139881 = -421137825;    double oDrZHQTbPbgYMmvCE75865428 = -782976786;    double oDrZHQTbPbgYMmvCE65606885 = -669570139;    double oDrZHQTbPbgYMmvCE5787748 = -295358397;    double oDrZHQTbPbgYMmvCE14813694 = -188791601;    double oDrZHQTbPbgYMmvCE74847591 = -364305393;    double oDrZHQTbPbgYMmvCE40051486 = -734756020;    double oDrZHQTbPbgYMmvCE47949965 = -774867646;    double oDrZHQTbPbgYMmvCE88575839 = -117644598;    double oDrZHQTbPbgYMmvCE49171418 = -253073595;    double oDrZHQTbPbgYMmvCE40525353 = -679710253;    double oDrZHQTbPbgYMmvCE34870404 = -18127591;    double oDrZHQTbPbgYMmvCE13621030 = -282746358;    double oDrZHQTbPbgYMmvCE15314931 = -156656961;    double oDrZHQTbPbgYMmvCE46583860 = -176891746;    double oDrZHQTbPbgYMmvCE77677480 = -808041711;    double oDrZHQTbPbgYMmvCE39966342 = -583439048;    double oDrZHQTbPbgYMmvCE60962064 = -187641004;    double oDrZHQTbPbgYMmvCE3864789 = -286903850;    double oDrZHQTbPbgYMmvCE48565535 = -709111095;    double oDrZHQTbPbgYMmvCE21609662 = -102348354;    double oDrZHQTbPbgYMmvCE80545776 = -372740801;    double oDrZHQTbPbgYMmvCE13047847 = -958478006;    double oDrZHQTbPbgYMmvCE56643675 = -600148958;    double oDrZHQTbPbgYMmvCE61457100 = -993473812;    double oDrZHQTbPbgYMmvCE4989071 = -94555285;    double oDrZHQTbPbgYMmvCE63985688 = -50165686;    double oDrZHQTbPbgYMmvCE4210211 = -488664578;    double oDrZHQTbPbgYMmvCE11679540 = -891171998;    double oDrZHQTbPbgYMmvCE11265735 = -494497757;    double oDrZHQTbPbgYMmvCE22825651 = -861453605;    double oDrZHQTbPbgYMmvCE90417702 = -123101580;    double oDrZHQTbPbgYMmvCE93700235 = -335480892;    double oDrZHQTbPbgYMmvCE63357942 = -109212940;    double oDrZHQTbPbgYMmvCE74779470 = -834568097;    double oDrZHQTbPbgYMmvCE4404077 = -832060617;    double oDrZHQTbPbgYMmvCE2163768 = -302907220;    double oDrZHQTbPbgYMmvCE98974207 = -626948521;    double oDrZHQTbPbgYMmvCE62150824 = -24592709;    double oDrZHQTbPbgYMmvCE2416619 = -398379609;    double oDrZHQTbPbgYMmvCE19226031 = -166329444;    double oDrZHQTbPbgYMmvCE72642480 = -471599849;    double oDrZHQTbPbgYMmvCE39068921 = -956742756;    double oDrZHQTbPbgYMmvCE39491393 = -858261240;     oDrZHQTbPbgYMmvCE80681351 = oDrZHQTbPbgYMmvCE59330418;     oDrZHQTbPbgYMmvCE59330418 = oDrZHQTbPbgYMmvCE77576086;     oDrZHQTbPbgYMmvCE77576086 = oDrZHQTbPbgYMmvCE75661256;     oDrZHQTbPbgYMmvCE75661256 = oDrZHQTbPbgYMmvCE95909252;     oDrZHQTbPbgYMmvCE95909252 = oDrZHQTbPbgYMmvCE3416264;     oDrZHQTbPbgYMmvCE3416264 = oDrZHQTbPbgYMmvCE24722748;     oDrZHQTbPbgYMmvCE24722748 = oDrZHQTbPbgYMmvCE97083668;     oDrZHQTbPbgYMmvCE97083668 = oDrZHQTbPbgYMmvCE86566915;     oDrZHQTbPbgYMmvCE86566915 = oDrZHQTbPbgYMmvCE38590469;     oDrZHQTbPbgYMmvCE38590469 = oDrZHQTbPbgYMmvCE55883462;     oDrZHQTbPbgYMmvCE55883462 = oDrZHQTbPbgYMmvCE56722592;     oDrZHQTbPbgYMmvCE56722592 = oDrZHQTbPbgYMmvCE86689805;     oDrZHQTbPbgYMmvCE86689805 = oDrZHQTbPbgYMmvCE15458216;     oDrZHQTbPbgYMmvCE15458216 = oDrZHQTbPbgYMmvCE41234282;     oDrZHQTbPbgYMmvCE41234282 = oDrZHQTbPbgYMmvCE26853365;     oDrZHQTbPbgYMmvCE26853365 = oDrZHQTbPbgYMmvCE45448132;     oDrZHQTbPbgYMmvCE45448132 = oDrZHQTbPbgYMmvCE44728297;     oDrZHQTbPbgYMmvCE44728297 = oDrZHQTbPbgYMmvCE37926582;     oDrZHQTbPbgYMmvCE37926582 = oDrZHQTbPbgYMmvCE81128997;     oDrZHQTbPbgYMmvCE81128997 = oDrZHQTbPbgYMmvCE7843052;     oDrZHQTbPbgYMmvCE7843052 = oDrZHQTbPbgYMmvCE42202108;     oDrZHQTbPbgYMmvCE42202108 = oDrZHQTbPbgYMmvCE8190537;     oDrZHQTbPbgYMmvCE8190537 = oDrZHQTbPbgYMmvCE1710658;     oDrZHQTbPbgYMmvCE1710658 = oDrZHQTbPbgYMmvCE10054371;     oDrZHQTbPbgYMmvCE10054371 = oDrZHQTbPbgYMmvCE90121505;     oDrZHQTbPbgYMmvCE90121505 = oDrZHQTbPbgYMmvCE88602570;     oDrZHQTbPbgYMmvCE88602570 = oDrZHQTbPbgYMmvCE49875157;     oDrZHQTbPbgYMmvCE49875157 = oDrZHQTbPbgYMmvCE57032182;     oDrZHQTbPbgYMmvCE57032182 = oDrZHQTbPbgYMmvCE38616950;     oDrZHQTbPbgYMmvCE38616950 = oDrZHQTbPbgYMmvCE50014630;     oDrZHQTbPbgYMmvCE50014630 = oDrZHQTbPbgYMmvCE6712044;     oDrZHQTbPbgYMmvCE6712044 = oDrZHQTbPbgYMmvCE16197240;     oDrZHQTbPbgYMmvCE16197240 = oDrZHQTbPbgYMmvCE51819402;     oDrZHQTbPbgYMmvCE51819402 = oDrZHQTbPbgYMmvCE1837186;     oDrZHQTbPbgYMmvCE1837186 = oDrZHQTbPbgYMmvCE25919352;     oDrZHQTbPbgYMmvCE25919352 = oDrZHQTbPbgYMmvCE80269504;     oDrZHQTbPbgYMmvCE80269504 = oDrZHQTbPbgYMmvCE67770652;     oDrZHQTbPbgYMmvCE67770652 = oDrZHQTbPbgYMmvCE4761955;     oDrZHQTbPbgYMmvCE4761955 = oDrZHQTbPbgYMmvCE76964517;     oDrZHQTbPbgYMmvCE76964517 = oDrZHQTbPbgYMmvCE77264209;     oDrZHQTbPbgYMmvCE77264209 = oDrZHQTbPbgYMmvCE59277517;     oDrZHQTbPbgYMmvCE59277517 = oDrZHQTbPbgYMmvCE20592446;     oDrZHQTbPbgYMmvCE20592446 = oDrZHQTbPbgYMmvCE27644761;     oDrZHQTbPbgYMmvCE27644761 = oDrZHQTbPbgYMmvCE88662811;     oDrZHQTbPbgYMmvCE88662811 = oDrZHQTbPbgYMmvCE53410695;     oDrZHQTbPbgYMmvCE53410695 = oDrZHQTbPbgYMmvCE28664406;     oDrZHQTbPbgYMmvCE28664406 = oDrZHQTbPbgYMmvCE83613500;     oDrZHQTbPbgYMmvCE83613500 = oDrZHQTbPbgYMmvCE85889469;     oDrZHQTbPbgYMmvCE85889469 = oDrZHQTbPbgYMmvCE52821972;     oDrZHQTbPbgYMmvCE52821972 = oDrZHQTbPbgYMmvCE26937411;     oDrZHQTbPbgYMmvCE26937411 = oDrZHQTbPbgYMmvCE38748895;     oDrZHQTbPbgYMmvCE38748895 = oDrZHQTbPbgYMmvCE83886393;     oDrZHQTbPbgYMmvCE83886393 = oDrZHQTbPbgYMmvCE25779537;     oDrZHQTbPbgYMmvCE25779537 = oDrZHQTbPbgYMmvCE58119166;     oDrZHQTbPbgYMmvCE58119166 = oDrZHQTbPbgYMmvCE38479244;     oDrZHQTbPbgYMmvCE38479244 = oDrZHQTbPbgYMmvCE51139881;     oDrZHQTbPbgYMmvCE51139881 = oDrZHQTbPbgYMmvCE75865428;     oDrZHQTbPbgYMmvCE75865428 = oDrZHQTbPbgYMmvCE65606885;     oDrZHQTbPbgYMmvCE65606885 = oDrZHQTbPbgYMmvCE5787748;     oDrZHQTbPbgYMmvCE5787748 = oDrZHQTbPbgYMmvCE14813694;     oDrZHQTbPbgYMmvCE14813694 = oDrZHQTbPbgYMmvCE74847591;     oDrZHQTbPbgYMmvCE74847591 = oDrZHQTbPbgYMmvCE40051486;     oDrZHQTbPbgYMmvCE40051486 = oDrZHQTbPbgYMmvCE47949965;     oDrZHQTbPbgYMmvCE47949965 = oDrZHQTbPbgYMmvCE88575839;     oDrZHQTbPbgYMmvCE88575839 = oDrZHQTbPbgYMmvCE49171418;     oDrZHQTbPbgYMmvCE49171418 = oDrZHQTbPbgYMmvCE40525353;     oDrZHQTbPbgYMmvCE40525353 = oDrZHQTbPbgYMmvCE34870404;     oDrZHQTbPbgYMmvCE34870404 = oDrZHQTbPbgYMmvCE13621030;     oDrZHQTbPbgYMmvCE13621030 = oDrZHQTbPbgYMmvCE15314931;     oDrZHQTbPbgYMmvCE15314931 = oDrZHQTbPbgYMmvCE46583860;     oDrZHQTbPbgYMmvCE46583860 = oDrZHQTbPbgYMmvCE77677480;     oDrZHQTbPbgYMmvCE77677480 = oDrZHQTbPbgYMmvCE39966342;     oDrZHQTbPbgYMmvCE39966342 = oDrZHQTbPbgYMmvCE60962064;     oDrZHQTbPbgYMmvCE60962064 = oDrZHQTbPbgYMmvCE3864789;     oDrZHQTbPbgYMmvCE3864789 = oDrZHQTbPbgYMmvCE48565535;     oDrZHQTbPbgYMmvCE48565535 = oDrZHQTbPbgYMmvCE21609662;     oDrZHQTbPbgYMmvCE21609662 = oDrZHQTbPbgYMmvCE80545776;     oDrZHQTbPbgYMmvCE80545776 = oDrZHQTbPbgYMmvCE13047847;     oDrZHQTbPbgYMmvCE13047847 = oDrZHQTbPbgYMmvCE56643675;     oDrZHQTbPbgYMmvCE56643675 = oDrZHQTbPbgYMmvCE61457100;     oDrZHQTbPbgYMmvCE61457100 = oDrZHQTbPbgYMmvCE4989071;     oDrZHQTbPbgYMmvCE4989071 = oDrZHQTbPbgYMmvCE63985688;     oDrZHQTbPbgYMmvCE63985688 = oDrZHQTbPbgYMmvCE4210211;     oDrZHQTbPbgYMmvCE4210211 = oDrZHQTbPbgYMmvCE11679540;     oDrZHQTbPbgYMmvCE11679540 = oDrZHQTbPbgYMmvCE11265735;     oDrZHQTbPbgYMmvCE11265735 = oDrZHQTbPbgYMmvCE22825651;     oDrZHQTbPbgYMmvCE22825651 = oDrZHQTbPbgYMmvCE90417702;     oDrZHQTbPbgYMmvCE90417702 = oDrZHQTbPbgYMmvCE93700235;     oDrZHQTbPbgYMmvCE93700235 = oDrZHQTbPbgYMmvCE63357942;     oDrZHQTbPbgYMmvCE63357942 = oDrZHQTbPbgYMmvCE74779470;     oDrZHQTbPbgYMmvCE74779470 = oDrZHQTbPbgYMmvCE4404077;     oDrZHQTbPbgYMmvCE4404077 = oDrZHQTbPbgYMmvCE2163768;     oDrZHQTbPbgYMmvCE2163768 = oDrZHQTbPbgYMmvCE98974207;     oDrZHQTbPbgYMmvCE98974207 = oDrZHQTbPbgYMmvCE62150824;     oDrZHQTbPbgYMmvCE62150824 = oDrZHQTbPbgYMmvCE2416619;     oDrZHQTbPbgYMmvCE2416619 = oDrZHQTbPbgYMmvCE19226031;     oDrZHQTbPbgYMmvCE19226031 = oDrZHQTbPbgYMmvCE72642480;     oDrZHQTbPbgYMmvCE72642480 = oDrZHQTbPbgYMmvCE39068921;     oDrZHQTbPbgYMmvCE39068921 = oDrZHQTbPbgYMmvCE39491393;     oDrZHQTbPbgYMmvCE39491393 = oDrZHQTbPbgYMmvCE80681351;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ByOzHQzeirwKTXJQ81011285() {     double fVFIZucsHoGdjfmMR86798457 = -970163128;    double fVFIZucsHoGdjfmMR2703698 = -777865760;    double fVFIZucsHoGdjfmMR93618270 = -380915747;    double fVFIZucsHoGdjfmMR2736037 = -414428706;    double fVFIZucsHoGdjfmMR64804741 = -352030642;    double fVFIZucsHoGdjfmMR94833465 = -722636995;    double fVFIZucsHoGdjfmMR85211714 = -52356892;    double fVFIZucsHoGdjfmMR46843560 = 65293705;    double fVFIZucsHoGdjfmMR51209411 = -526970440;    double fVFIZucsHoGdjfmMR79820128 = -123885532;    double fVFIZucsHoGdjfmMR15993752 = -691872188;    double fVFIZucsHoGdjfmMR88545634 = -667871050;    double fVFIZucsHoGdjfmMR72898923 = -748897690;    double fVFIZucsHoGdjfmMR98601891 = -333643257;    double fVFIZucsHoGdjfmMR36976709 = -102825047;    double fVFIZucsHoGdjfmMR68358759 = -864928690;    double fVFIZucsHoGdjfmMR39284136 = -397064182;    double fVFIZucsHoGdjfmMR41781450 = -271268607;    double fVFIZucsHoGdjfmMR99756423 = 93205590;    double fVFIZucsHoGdjfmMR61679162 = -506491770;    double fVFIZucsHoGdjfmMR96396244 = -565684013;    double fVFIZucsHoGdjfmMR28723131 = -356745565;    double fVFIZucsHoGdjfmMR43781329 = -293645641;    double fVFIZucsHoGdjfmMR85703031 = -669411028;    double fVFIZucsHoGdjfmMR53754020 = -927305970;    double fVFIZucsHoGdjfmMR14843181 = -414677698;    double fVFIZucsHoGdjfmMR87062348 = -70011362;    double fVFIZucsHoGdjfmMR62982282 = -615015477;    double fVFIZucsHoGdjfmMR23164438 = -135459067;    double fVFIZucsHoGdjfmMR7232587 = -801540996;    double fVFIZucsHoGdjfmMR42257179 = -590586395;    double fVFIZucsHoGdjfmMR4322666 = -170807822;    double fVFIZucsHoGdjfmMR25908188 = -436367966;    double fVFIZucsHoGdjfmMR46649966 = 57579224;    double fVFIZucsHoGdjfmMR63311838 = -57391437;    double fVFIZucsHoGdjfmMR94645264 = -469732028;    double fVFIZucsHoGdjfmMR61531439 = -568673272;    double fVFIZucsHoGdjfmMR1860866 = -843983368;    double fVFIZucsHoGdjfmMR78372208 = 83769586;    double fVFIZucsHoGdjfmMR64466455 = -786823533;    double fVFIZucsHoGdjfmMR82933433 = -927958896;    double fVFIZucsHoGdjfmMR91427974 = -171227741;    double fVFIZucsHoGdjfmMR98659424 = -327272079;    double fVFIZucsHoGdjfmMR15052501 = -93730292;    double fVFIZucsHoGdjfmMR615277 = -150526190;    double fVFIZucsHoGdjfmMR65349890 = -939177096;    double fVFIZucsHoGdjfmMR46442980 = -661324101;    double fVFIZucsHoGdjfmMR91570440 = 15310593;    double fVFIZucsHoGdjfmMR95565198 = -484657826;    double fVFIZucsHoGdjfmMR65733625 = -511422594;    double fVFIZucsHoGdjfmMR62638089 = -935191802;    double fVFIZucsHoGdjfmMR49558615 = -983494020;    double fVFIZucsHoGdjfmMR32134851 = -391696990;    double fVFIZucsHoGdjfmMR21748417 = -641123162;    double fVFIZucsHoGdjfmMR84615040 = -455929901;    double fVFIZucsHoGdjfmMR58075326 = -513417564;    double fVFIZucsHoGdjfmMR58922369 = -384220119;    double fVFIZucsHoGdjfmMR7915239 = -711504719;    double fVFIZucsHoGdjfmMR48982016 = -487122736;    double fVFIZucsHoGdjfmMR49961561 = -937352945;    double fVFIZucsHoGdjfmMR7771118 = -552625633;    double fVFIZucsHoGdjfmMR22229432 = -437341416;    double fVFIZucsHoGdjfmMR23679122 = -799247229;    double fVFIZucsHoGdjfmMR43976824 = -725429445;    double fVFIZucsHoGdjfmMR37562949 = -533299138;    double fVFIZucsHoGdjfmMR11671087 = -421064366;    double fVFIZucsHoGdjfmMR62637447 = -131503084;    double fVFIZucsHoGdjfmMR26248958 = -706476915;    double fVFIZucsHoGdjfmMR35290054 = -176251821;    double fVFIZucsHoGdjfmMR42331444 = -633093019;    double fVFIZucsHoGdjfmMR6827320 = -196255419;    double fVFIZucsHoGdjfmMR37423271 = -553080815;    double fVFIZucsHoGdjfmMR63409241 = -255038193;    double fVFIZucsHoGdjfmMR35289968 = -119970878;    double fVFIZucsHoGdjfmMR78745729 = -578532875;    double fVFIZucsHoGdjfmMR4968271 = -294456272;    double fVFIZucsHoGdjfmMR30063707 = 70526514;    double fVFIZucsHoGdjfmMR28728828 = -99915350;    double fVFIZucsHoGdjfmMR85087755 = -418884839;    double fVFIZucsHoGdjfmMR88404130 = -988128875;    double fVFIZucsHoGdjfmMR68400201 = -753353598;    double fVFIZucsHoGdjfmMR95491907 = 14678044;    double fVFIZucsHoGdjfmMR67417084 = -30357652;    double fVFIZucsHoGdjfmMR57430813 = -624036474;    double fVFIZucsHoGdjfmMR44594498 = -866349194;    double fVFIZucsHoGdjfmMR92698564 = -607092375;    double fVFIZucsHoGdjfmMR72187815 = -779110833;    double fVFIZucsHoGdjfmMR4159772 = -795244805;    double fVFIZucsHoGdjfmMR62034925 = -486490875;    double fVFIZucsHoGdjfmMR5236512 = -543973873;    double fVFIZucsHoGdjfmMR35722896 = 14488090;    double fVFIZucsHoGdjfmMR53616201 = -857168553;    double fVFIZucsHoGdjfmMR52878849 = -256860632;    double fVFIZucsHoGdjfmMR28410648 = 21122530;    double fVFIZucsHoGdjfmMR56695338 = -134197900;    double fVFIZucsHoGdjfmMR60704001 = -390617480;    double fVFIZucsHoGdjfmMR67748852 = -371980513;    double fVFIZucsHoGdjfmMR54682600 = -601842635;    double fVFIZucsHoGdjfmMR77489551 = -560431155;    double fVFIZucsHoGdjfmMR88944189 = -970163128;     fVFIZucsHoGdjfmMR86798457 = fVFIZucsHoGdjfmMR2703698;     fVFIZucsHoGdjfmMR2703698 = fVFIZucsHoGdjfmMR93618270;     fVFIZucsHoGdjfmMR93618270 = fVFIZucsHoGdjfmMR2736037;     fVFIZucsHoGdjfmMR2736037 = fVFIZucsHoGdjfmMR64804741;     fVFIZucsHoGdjfmMR64804741 = fVFIZucsHoGdjfmMR94833465;     fVFIZucsHoGdjfmMR94833465 = fVFIZucsHoGdjfmMR85211714;     fVFIZucsHoGdjfmMR85211714 = fVFIZucsHoGdjfmMR46843560;     fVFIZucsHoGdjfmMR46843560 = fVFIZucsHoGdjfmMR51209411;     fVFIZucsHoGdjfmMR51209411 = fVFIZucsHoGdjfmMR79820128;     fVFIZucsHoGdjfmMR79820128 = fVFIZucsHoGdjfmMR15993752;     fVFIZucsHoGdjfmMR15993752 = fVFIZucsHoGdjfmMR88545634;     fVFIZucsHoGdjfmMR88545634 = fVFIZucsHoGdjfmMR72898923;     fVFIZucsHoGdjfmMR72898923 = fVFIZucsHoGdjfmMR98601891;     fVFIZucsHoGdjfmMR98601891 = fVFIZucsHoGdjfmMR36976709;     fVFIZucsHoGdjfmMR36976709 = fVFIZucsHoGdjfmMR68358759;     fVFIZucsHoGdjfmMR68358759 = fVFIZucsHoGdjfmMR39284136;     fVFIZucsHoGdjfmMR39284136 = fVFIZucsHoGdjfmMR41781450;     fVFIZucsHoGdjfmMR41781450 = fVFIZucsHoGdjfmMR99756423;     fVFIZucsHoGdjfmMR99756423 = fVFIZucsHoGdjfmMR61679162;     fVFIZucsHoGdjfmMR61679162 = fVFIZucsHoGdjfmMR96396244;     fVFIZucsHoGdjfmMR96396244 = fVFIZucsHoGdjfmMR28723131;     fVFIZucsHoGdjfmMR28723131 = fVFIZucsHoGdjfmMR43781329;     fVFIZucsHoGdjfmMR43781329 = fVFIZucsHoGdjfmMR85703031;     fVFIZucsHoGdjfmMR85703031 = fVFIZucsHoGdjfmMR53754020;     fVFIZucsHoGdjfmMR53754020 = fVFIZucsHoGdjfmMR14843181;     fVFIZucsHoGdjfmMR14843181 = fVFIZucsHoGdjfmMR87062348;     fVFIZucsHoGdjfmMR87062348 = fVFIZucsHoGdjfmMR62982282;     fVFIZucsHoGdjfmMR62982282 = fVFIZucsHoGdjfmMR23164438;     fVFIZucsHoGdjfmMR23164438 = fVFIZucsHoGdjfmMR7232587;     fVFIZucsHoGdjfmMR7232587 = fVFIZucsHoGdjfmMR42257179;     fVFIZucsHoGdjfmMR42257179 = fVFIZucsHoGdjfmMR4322666;     fVFIZucsHoGdjfmMR4322666 = fVFIZucsHoGdjfmMR25908188;     fVFIZucsHoGdjfmMR25908188 = fVFIZucsHoGdjfmMR46649966;     fVFIZucsHoGdjfmMR46649966 = fVFIZucsHoGdjfmMR63311838;     fVFIZucsHoGdjfmMR63311838 = fVFIZucsHoGdjfmMR94645264;     fVFIZucsHoGdjfmMR94645264 = fVFIZucsHoGdjfmMR61531439;     fVFIZucsHoGdjfmMR61531439 = fVFIZucsHoGdjfmMR1860866;     fVFIZucsHoGdjfmMR1860866 = fVFIZucsHoGdjfmMR78372208;     fVFIZucsHoGdjfmMR78372208 = fVFIZucsHoGdjfmMR64466455;     fVFIZucsHoGdjfmMR64466455 = fVFIZucsHoGdjfmMR82933433;     fVFIZucsHoGdjfmMR82933433 = fVFIZucsHoGdjfmMR91427974;     fVFIZucsHoGdjfmMR91427974 = fVFIZucsHoGdjfmMR98659424;     fVFIZucsHoGdjfmMR98659424 = fVFIZucsHoGdjfmMR15052501;     fVFIZucsHoGdjfmMR15052501 = fVFIZucsHoGdjfmMR615277;     fVFIZucsHoGdjfmMR615277 = fVFIZucsHoGdjfmMR65349890;     fVFIZucsHoGdjfmMR65349890 = fVFIZucsHoGdjfmMR46442980;     fVFIZucsHoGdjfmMR46442980 = fVFIZucsHoGdjfmMR91570440;     fVFIZucsHoGdjfmMR91570440 = fVFIZucsHoGdjfmMR95565198;     fVFIZucsHoGdjfmMR95565198 = fVFIZucsHoGdjfmMR65733625;     fVFIZucsHoGdjfmMR65733625 = fVFIZucsHoGdjfmMR62638089;     fVFIZucsHoGdjfmMR62638089 = fVFIZucsHoGdjfmMR49558615;     fVFIZucsHoGdjfmMR49558615 = fVFIZucsHoGdjfmMR32134851;     fVFIZucsHoGdjfmMR32134851 = fVFIZucsHoGdjfmMR21748417;     fVFIZucsHoGdjfmMR21748417 = fVFIZucsHoGdjfmMR84615040;     fVFIZucsHoGdjfmMR84615040 = fVFIZucsHoGdjfmMR58075326;     fVFIZucsHoGdjfmMR58075326 = fVFIZucsHoGdjfmMR58922369;     fVFIZucsHoGdjfmMR58922369 = fVFIZucsHoGdjfmMR7915239;     fVFIZucsHoGdjfmMR7915239 = fVFIZucsHoGdjfmMR48982016;     fVFIZucsHoGdjfmMR48982016 = fVFIZucsHoGdjfmMR49961561;     fVFIZucsHoGdjfmMR49961561 = fVFIZucsHoGdjfmMR7771118;     fVFIZucsHoGdjfmMR7771118 = fVFIZucsHoGdjfmMR22229432;     fVFIZucsHoGdjfmMR22229432 = fVFIZucsHoGdjfmMR23679122;     fVFIZucsHoGdjfmMR23679122 = fVFIZucsHoGdjfmMR43976824;     fVFIZucsHoGdjfmMR43976824 = fVFIZucsHoGdjfmMR37562949;     fVFIZucsHoGdjfmMR37562949 = fVFIZucsHoGdjfmMR11671087;     fVFIZucsHoGdjfmMR11671087 = fVFIZucsHoGdjfmMR62637447;     fVFIZucsHoGdjfmMR62637447 = fVFIZucsHoGdjfmMR26248958;     fVFIZucsHoGdjfmMR26248958 = fVFIZucsHoGdjfmMR35290054;     fVFIZucsHoGdjfmMR35290054 = fVFIZucsHoGdjfmMR42331444;     fVFIZucsHoGdjfmMR42331444 = fVFIZucsHoGdjfmMR6827320;     fVFIZucsHoGdjfmMR6827320 = fVFIZucsHoGdjfmMR37423271;     fVFIZucsHoGdjfmMR37423271 = fVFIZucsHoGdjfmMR63409241;     fVFIZucsHoGdjfmMR63409241 = fVFIZucsHoGdjfmMR35289968;     fVFIZucsHoGdjfmMR35289968 = fVFIZucsHoGdjfmMR78745729;     fVFIZucsHoGdjfmMR78745729 = fVFIZucsHoGdjfmMR4968271;     fVFIZucsHoGdjfmMR4968271 = fVFIZucsHoGdjfmMR30063707;     fVFIZucsHoGdjfmMR30063707 = fVFIZucsHoGdjfmMR28728828;     fVFIZucsHoGdjfmMR28728828 = fVFIZucsHoGdjfmMR85087755;     fVFIZucsHoGdjfmMR85087755 = fVFIZucsHoGdjfmMR88404130;     fVFIZucsHoGdjfmMR88404130 = fVFIZucsHoGdjfmMR68400201;     fVFIZucsHoGdjfmMR68400201 = fVFIZucsHoGdjfmMR95491907;     fVFIZucsHoGdjfmMR95491907 = fVFIZucsHoGdjfmMR67417084;     fVFIZucsHoGdjfmMR67417084 = fVFIZucsHoGdjfmMR57430813;     fVFIZucsHoGdjfmMR57430813 = fVFIZucsHoGdjfmMR44594498;     fVFIZucsHoGdjfmMR44594498 = fVFIZucsHoGdjfmMR92698564;     fVFIZucsHoGdjfmMR92698564 = fVFIZucsHoGdjfmMR72187815;     fVFIZucsHoGdjfmMR72187815 = fVFIZucsHoGdjfmMR4159772;     fVFIZucsHoGdjfmMR4159772 = fVFIZucsHoGdjfmMR62034925;     fVFIZucsHoGdjfmMR62034925 = fVFIZucsHoGdjfmMR5236512;     fVFIZucsHoGdjfmMR5236512 = fVFIZucsHoGdjfmMR35722896;     fVFIZucsHoGdjfmMR35722896 = fVFIZucsHoGdjfmMR53616201;     fVFIZucsHoGdjfmMR53616201 = fVFIZucsHoGdjfmMR52878849;     fVFIZucsHoGdjfmMR52878849 = fVFIZucsHoGdjfmMR28410648;     fVFIZucsHoGdjfmMR28410648 = fVFIZucsHoGdjfmMR56695338;     fVFIZucsHoGdjfmMR56695338 = fVFIZucsHoGdjfmMR60704001;     fVFIZucsHoGdjfmMR60704001 = fVFIZucsHoGdjfmMR67748852;     fVFIZucsHoGdjfmMR67748852 = fVFIZucsHoGdjfmMR54682600;     fVFIZucsHoGdjfmMR54682600 = fVFIZucsHoGdjfmMR77489551;     fVFIZucsHoGdjfmMR77489551 = fVFIZucsHoGdjfmMR88944189;     fVFIZucsHoGdjfmMR88944189 = fVFIZucsHoGdjfmMR86798457;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void yTUFafUxWjybNrQn48302885() {     double dctXmHIGCCPgYesGq22257426 = -336440028;    double dctXmHIGCCPgYesGq15975776 = -658417366;    double dctXmHIGCCPgYesGq31004747 = -46582822;    double dctXmHIGCCPgYesGq74959391 = -235579777;    double dctXmHIGCCPgYesGq21090011 = -457629487;    double dctXmHIGCCPgYesGq31056338 = -984984890;    double dctXmHIGCCPgYesGq95869120 = -997082627;    double dctXmHIGCCPgYesGq8975121 = -678940436;    double dctXmHIGCCPgYesGq52727016 = 84698153;    double dctXmHIGCCPgYesGq94722840 = -888572530;    double dctXmHIGCCPgYesGq54175487 = -493879917;    double dctXmHIGCCPgYesGq89113783 = -531991821;    double dctXmHIGCCPgYesGq11133570 = -200910195;    double dctXmHIGCCPgYesGq51742755 = -310006667;    double dctXmHIGCCPgYesGq95171225 = -606250062;    double dctXmHIGCCPgYesGq51900060 = -500740297;    double dctXmHIGCCPgYesGq95120071 = -924038734;    double dctXmHIGCCPgYesGq41541641 = -344696675;    double dctXmHIGCCPgYesGq306754 = -223357832;    double dctXmHIGCCPgYesGq55516381 = -933052836;    double dctXmHIGCCPgYesGq9483108 = -222958702;    double dctXmHIGCCPgYesGq32144656 = -443381859;    double dctXmHIGCCPgYesGq14306709 = 68722174;    double dctXmHIGCCPgYesGq79495977 = -80235632;    double dctXmHIGCCPgYesGq61771972 = -951291169;    double dctXmHIGCCPgYesGq25541353 = -103254504;    double dctXmHIGCCPgYesGq47482923 = -976295503;    double dctXmHIGCCPgYesGq3710708 = -784787651;    double dctXmHIGCCPgYesGq69832652 = -709553870;    double dctXmHIGCCPgYesGq12765998 = -836048166;    double dctXmHIGCCPgYesGq60840411 = -42566882;    double dctXmHIGCCPgYesGq42145684 = 56220763;    double dctXmHIGCCPgYesGq38126199 = -583963943;    double dctXmHIGCCPgYesGq76398392 = -7114589;    double dctXmHIGCCPgYesGq29120292 = -8491688;    double dctXmHIGCCPgYesGq46206108 = -684297334;    double dctXmHIGCCPgYesGq10943256 = -433369312;    double dctXmHIGCCPgYesGq53076268 = -826638949;    double dctXmHIGCCPgYesGq8762771 = -667106690;    double dctXmHIGCCPgYesGq28737988 = -252086079;    double dctXmHIGCCPgYesGq16385041 = -697157810;    double dctXmHIGCCPgYesGq80793100 = -865773032;    double dctXmHIGCCPgYesGq88042802 = -980452865;    double dctXmHIGCCPgYesGq55183839 = -698062702;    double dctXmHIGCCPgYesGq631735 = 22962649;    double dctXmHIGCCPgYesGq11311219 = -978100728;    double dctXmHIGCCPgYesGq65788143 = -45310461;    double dctXmHIGCCPgYesGq9296753 = -151216000;    double dctXmHIGCCPgYesGq18457865 = -148394706;    double dctXmHIGCCPgYesGq90263214 = 97550590;    double dctXmHIGCCPgYesGq43408813 = -809468359;    double dctXmHIGCCPgYesGq33044087 = -97146367;    double dctXmHIGCCPgYesGq756000 = -147742544;    double dctXmHIGCCPgYesGq37740263 = -832631661;    double dctXmHIGCCPgYesGq38819777 = -802420047;    double dctXmHIGCCPgYesGq90112770 = -893058170;    double dctXmHIGCCPgYesGq1669068 = -627139541;    double dctXmHIGCCPgYesGq51508770 = -966347190;    double dctXmHIGCCPgYesGq13187420 = -284288609;    double dctXmHIGCCPgYesGq95548658 = -254374983;    double dctXmHIGCCPgYesGq83573415 = 91310612;    double dctXmHIGCCPgYesGq92158412 = -112294976;    double dctXmHIGCCPgYesGq39142468 = -969386567;    double dctXmHIGCCPgYesGq39961018 = -79253681;    double dctXmHIGCCPgYesGq33882430 = -746005649;    double dctXmHIGCCPgYesGq12029803 = -450100681;    double dctXmHIGCCPgYesGq50987585 = -948027878;    double dctXmHIGCCPgYesGq34735177 = -93795606;    double dctXmHIGCCPgYesGq22622464 = -201514980;    double dctXmHIGCCPgYesGq48965117 = -921952729;    double dctXmHIGCCPgYesGq40956804 = 32629014;    double dctXmHIGCCPgYesGq42043803 = 2600214;    double dctXmHIGCCPgYesGq32778871 = -677589986;    double dctXmHIGCCPgYesGq71568766 = -971271754;    double dctXmHIGCCPgYesGq39131340 = -135895027;    double dctXmHIGCCPgYesGq28690007 = -357185670;    double dctXmHIGCCPgYesGq44101854 = -462928994;    double dctXmHIGCCPgYesGq59122869 = -233215124;    double dctXmHIGCCPgYesGq78864242 = -3198282;    double dctXmHIGCCPgYesGq50460753 = -973190441;    double dctXmHIGCCPgYesGq59753209 = 42055956;    double dctXmHIGCCPgYesGq38186170 = -725079503;    double dctXmHIGCCPgYesGq85252843 = -536392946;    double dctXmHIGCCPgYesGq79569438 = -707104461;    double dctXmHIGCCPgYesGq69357185 = 73420193;    double dctXmHIGCCPgYesGq27796324 = -945420516;    double dctXmHIGCCPgYesGq41389685 = -796036693;    double dctXmHIGCCPgYesGq385936 = -751332283;    double dctXmHIGCCPgYesGq37578615 = -204694543;    double dctXmHIGCCPgYesGq39007521 = -115433518;    double dctXmHIGCCPgYesGq44537041 = 42842206;    double dctXmHIGCCPgYesGq59434486 = -467022123;    double dctXmHIGCCPgYesGq39888849 = -442350340;    double dctXmHIGCCPgYesGq13214112 = -312731707;    double dctXmHIGCCPgYesGq45164572 = -243396691;    double dctXmHIGCCPgYesGq24226628 = -484862834;    double dctXmHIGCCPgYesGq41650633 = -896386466;    double dctXmHIGCCPgYesGq48081784 = -801199184;    double dctXmHIGCCPgYesGq21301410 = -952057054;    double dctXmHIGCCPgYesGq88601932 = -336440028;     dctXmHIGCCPgYesGq22257426 = dctXmHIGCCPgYesGq15975776;     dctXmHIGCCPgYesGq15975776 = dctXmHIGCCPgYesGq31004747;     dctXmHIGCCPgYesGq31004747 = dctXmHIGCCPgYesGq74959391;     dctXmHIGCCPgYesGq74959391 = dctXmHIGCCPgYesGq21090011;     dctXmHIGCCPgYesGq21090011 = dctXmHIGCCPgYesGq31056338;     dctXmHIGCCPgYesGq31056338 = dctXmHIGCCPgYesGq95869120;     dctXmHIGCCPgYesGq95869120 = dctXmHIGCCPgYesGq8975121;     dctXmHIGCCPgYesGq8975121 = dctXmHIGCCPgYesGq52727016;     dctXmHIGCCPgYesGq52727016 = dctXmHIGCCPgYesGq94722840;     dctXmHIGCCPgYesGq94722840 = dctXmHIGCCPgYesGq54175487;     dctXmHIGCCPgYesGq54175487 = dctXmHIGCCPgYesGq89113783;     dctXmHIGCCPgYesGq89113783 = dctXmHIGCCPgYesGq11133570;     dctXmHIGCCPgYesGq11133570 = dctXmHIGCCPgYesGq51742755;     dctXmHIGCCPgYesGq51742755 = dctXmHIGCCPgYesGq95171225;     dctXmHIGCCPgYesGq95171225 = dctXmHIGCCPgYesGq51900060;     dctXmHIGCCPgYesGq51900060 = dctXmHIGCCPgYesGq95120071;     dctXmHIGCCPgYesGq95120071 = dctXmHIGCCPgYesGq41541641;     dctXmHIGCCPgYesGq41541641 = dctXmHIGCCPgYesGq306754;     dctXmHIGCCPgYesGq306754 = dctXmHIGCCPgYesGq55516381;     dctXmHIGCCPgYesGq55516381 = dctXmHIGCCPgYesGq9483108;     dctXmHIGCCPgYesGq9483108 = dctXmHIGCCPgYesGq32144656;     dctXmHIGCCPgYesGq32144656 = dctXmHIGCCPgYesGq14306709;     dctXmHIGCCPgYesGq14306709 = dctXmHIGCCPgYesGq79495977;     dctXmHIGCCPgYesGq79495977 = dctXmHIGCCPgYesGq61771972;     dctXmHIGCCPgYesGq61771972 = dctXmHIGCCPgYesGq25541353;     dctXmHIGCCPgYesGq25541353 = dctXmHIGCCPgYesGq47482923;     dctXmHIGCCPgYesGq47482923 = dctXmHIGCCPgYesGq3710708;     dctXmHIGCCPgYesGq3710708 = dctXmHIGCCPgYesGq69832652;     dctXmHIGCCPgYesGq69832652 = dctXmHIGCCPgYesGq12765998;     dctXmHIGCCPgYesGq12765998 = dctXmHIGCCPgYesGq60840411;     dctXmHIGCCPgYesGq60840411 = dctXmHIGCCPgYesGq42145684;     dctXmHIGCCPgYesGq42145684 = dctXmHIGCCPgYesGq38126199;     dctXmHIGCCPgYesGq38126199 = dctXmHIGCCPgYesGq76398392;     dctXmHIGCCPgYesGq76398392 = dctXmHIGCCPgYesGq29120292;     dctXmHIGCCPgYesGq29120292 = dctXmHIGCCPgYesGq46206108;     dctXmHIGCCPgYesGq46206108 = dctXmHIGCCPgYesGq10943256;     dctXmHIGCCPgYesGq10943256 = dctXmHIGCCPgYesGq53076268;     dctXmHIGCCPgYesGq53076268 = dctXmHIGCCPgYesGq8762771;     dctXmHIGCCPgYesGq8762771 = dctXmHIGCCPgYesGq28737988;     dctXmHIGCCPgYesGq28737988 = dctXmHIGCCPgYesGq16385041;     dctXmHIGCCPgYesGq16385041 = dctXmHIGCCPgYesGq80793100;     dctXmHIGCCPgYesGq80793100 = dctXmHIGCCPgYesGq88042802;     dctXmHIGCCPgYesGq88042802 = dctXmHIGCCPgYesGq55183839;     dctXmHIGCCPgYesGq55183839 = dctXmHIGCCPgYesGq631735;     dctXmHIGCCPgYesGq631735 = dctXmHIGCCPgYesGq11311219;     dctXmHIGCCPgYesGq11311219 = dctXmHIGCCPgYesGq65788143;     dctXmHIGCCPgYesGq65788143 = dctXmHIGCCPgYesGq9296753;     dctXmHIGCCPgYesGq9296753 = dctXmHIGCCPgYesGq18457865;     dctXmHIGCCPgYesGq18457865 = dctXmHIGCCPgYesGq90263214;     dctXmHIGCCPgYesGq90263214 = dctXmHIGCCPgYesGq43408813;     dctXmHIGCCPgYesGq43408813 = dctXmHIGCCPgYesGq33044087;     dctXmHIGCCPgYesGq33044087 = dctXmHIGCCPgYesGq756000;     dctXmHIGCCPgYesGq756000 = dctXmHIGCCPgYesGq37740263;     dctXmHIGCCPgYesGq37740263 = dctXmHIGCCPgYesGq38819777;     dctXmHIGCCPgYesGq38819777 = dctXmHIGCCPgYesGq90112770;     dctXmHIGCCPgYesGq90112770 = dctXmHIGCCPgYesGq1669068;     dctXmHIGCCPgYesGq1669068 = dctXmHIGCCPgYesGq51508770;     dctXmHIGCCPgYesGq51508770 = dctXmHIGCCPgYesGq13187420;     dctXmHIGCCPgYesGq13187420 = dctXmHIGCCPgYesGq95548658;     dctXmHIGCCPgYesGq95548658 = dctXmHIGCCPgYesGq83573415;     dctXmHIGCCPgYesGq83573415 = dctXmHIGCCPgYesGq92158412;     dctXmHIGCCPgYesGq92158412 = dctXmHIGCCPgYesGq39142468;     dctXmHIGCCPgYesGq39142468 = dctXmHIGCCPgYesGq39961018;     dctXmHIGCCPgYesGq39961018 = dctXmHIGCCPgYesGq33882430;     dctXmHIGCCPgYesGq33882430 = dctXmHIGCCPgYesGq12029803;     dctXmHIGCCPgYesGq12029803 = dctXmHIGCCPgYesGq50987585;     dctXmHIGCCPgYesGq50987585 = dctXmHIGCCPgYesGq34735177;     dctXmHIGCCPgYesGq34735177 = dctXmHIGCCPgYesGq22622464;     dctXmHIGCCPgYesGq22622464 = dctXmHIGCCPgYesGq48965117;     dctXmHIGCCPgYesGq48965117 = dctXmHIGCCPgYesGq40956804;     dctXmHIGCCPgYesGq40956804 = dctXmHIGCCPgYesGq42043803;     dctXmHIGCCPgYesGq42043803 = dctXmHIGCCPgYesGq32778871;     dctXmHIGCCPgYesGq32778871 = dctXmHIGCCPgYesGq71568766;     dctXmHIGCCPgYesGq71568766 = dctXmHIGCCPgYesGq39131340;     dctXmHIGCCPgYesGq39131340 = dctXmHIGCCPgYesGq28690007;     dctXmHIGCCPgYesGq28690007 = dctXmHIGCCPgYesGq44101854;     dctXmHIGCCPgYesGq44101854 = dctXmHIGCCPgYesGq59122869;     dctXmHIGCCPgYesGq59122869 = dctXmHIGCCPgYesGq78864242;     dctXmHIGCCPgYesGq78864242 = dctXmHIGCCPgYesGq50460753;     dctXmHIGCCPgYesGq50460753 = dctXmHIGCCPgYesGq59753209;     dctXmHIGCCPgYesGq59753209 = dctXmHIGCCPgYesGq38186170;     dctXmHIGCCPgYesGq38186170 = dctXmHIGCCPgYesGq85252843;     dctXmHIGCCPgYesGq85252843 = dctXmHIGCCPgYesGq79569438;     dctXmHIGCCPgYesGq79569438 = dctXmHIGCCPgYesGq69357185;     dctXmHIGCCPgYesGq69357185 = dctXmHIGCCPgYesGq27796324;     dctXmHIGCCPgYesGq27796324 = dctXmHIGCCPgYesGq41389685;     dctXmHIGCCPgYesGq41389685 = dctXmHIGCCPgYesGq385936;     dctXmHIGCCPgYesGq385936 = dctXmHIGCCPgYesGq37578615;     dctXmHIGCCPgYesGq37578615 = dctXmHIGCCPgYesGq39007521;     dctXmHIGCCPgYesGq39007521 = dctXmHIGCCPgYesGq44537041;     dctXmHIGCCPgYesGq44537041 = dctXmHIGCCPgYesGq59434486;     dctXmHIGCCPgYesGq59434486 = dctXmHIGCCPgYesGq39888849;     dctXmHIGCCPgYesGq39888849 = dctXmHIGCCPgYesGq13214112;     dctXmHIGCCPgYesGq13214112 = dctXmHIGCCPgYesGq45164572;     dctXmHIGCCPgYesGq45164572 = dctXmHIGCCPgYesGq24226628;     dctXmHIGCCPgYesGq24226628 = dctXmHIGCCPgYesGq41650633;     dctXmHIGCCPgYesGq41650633 = dctXmHIGCCPgYesGq48081784;     dctXmHIGCCPgYesGq48081784 = dctXmHIGCCPgYesGq21301410;     dctXmHIGCCPgYesGq21301410 = dctXmHIGCCPgYesGq88601932;     dctXmHIGCCPgYesGq88601932 = dctXmHIGCCPgYesGq22257426;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BZZgeSlbJziHvsLY63351953() {     double mRofNkMnDhRxXEdoh28374533 = -448341917;    double mRofNkMnDhRxXEdoh59349055 = -368017147;    double mRofNkMnDhRxXEdoh47046931 = -197415743;    double mRofNkMnDhRxXEdoh2034173 = -426328408;    double mRofNkMnDhRxXEdoh89985499 = -978279359;    double mRofNkMnDhRxXEdoh22473541 = -149481125;    double mRofNkMnDhRxXEdoh56358086 = -36047604;    double mRofNkMnDhRxXEdoh58735011 = -965814349;    double mRofNkMnDhRxXEdoh17369512 = -783334513;    double mRofNkMnDhRxXEdoh35952500 = -117548008;    double mRofNkMnDhRxXEdoh14285777 = -371317503;    double mRofNkMnDhRxXEdoh20936826 = -976398023;    double mRofNkMnDhRxXEdoh97342687 = -640801283;    double mRofNkMnDhRxXEdoh34886432 = -842245235;    double mRofNkMnDhRxXEdoh90913651 = -196712229;    double mRofNkMnDhRxXEdoh93405453 = -473739841;    double mRofNkMnDhRxXEdoh88956074 = -440583847;    double mRofNkMnDhRxXEdoh38594794 = -10219318;    double mRofNkMnDhRxXEdoh62136595 = -629126929;    double mRofNkMnDhRxXEdoh36066546 = -189955755;    double mRofNkMnDhRxXEdoh98036301 = -78446157;    double mRofNkMnDhRxXEdoh18665680 = -351311575;    double mRofNkMnDhRxXEdoh49897500 = -777795312;    double mRofNkMnDhRxXEdoh63488351 = -302540620;    double mRofNkMnDhRxXEdoh5471622 = -224487202;    double mRofNkMnDhRxXEdoh50263028 = 18090172;    double mRofNkMnDhRxXEdoh45942701 = -876957706;    double mRofNkMnDhRxXEdoh16817833 = -850716605;    double mRofNkMnDhRxXEdoh35964908 = -931936574;    double mRofNkMnDhRxXEdoh81381634 = -653519034;    double mRofNkMnDhRxXEdoh53082960 = 44112180;    double mRofNkMnDhRxXEdoh39756306 = -753226052;    double mRofNkMnDhRxXEdoh47837147 = -476577314;    double mRofNkMnDhRxXEdoh71228956 = -858656354;    double mRofNkMnDhRxXEdoh90594943 = -647224793;    double mRofNkMnDhRxXEdoh14932022 = -898323442;    double mRofNkMnDhRxXEdoh92205190 = -387005182;    double mRofNkMnDhRxXEdoh87166481 = -598144958;    double mRofNkMnDhRxXEdoh82373024 = -661030187;    double mRofNkMnDhRxXEdoh16239926 = -725525301;    double mRofNkMnDhRxXEdoh22054265 = -762431704;    double mRofNkMnDhRxXEdoh12943558 = -35915310;    double mRofNkMnDhRxXEdoh66109780 = 38742551;    double mRofNkMnDhRxXEdoh42591580 = -717405640;    double mRofNkMnDhRxXEdoh12584200 = -738935505;    double mRofNkMnDhRxXEdoh23250414 = -963316844;    double mRofNkMnDhRxXEdoh83566717 = -164085999;    double mRofNkMnDhRxXEdoh17253694 = -161111533;    double mRofNkMnDhRxXEdoh28133593 = -234131694;    double mRofNkMnDhRxXEdoh3174867 = 10539782;    double mRofNkMnDhRxXEdoh79109491 = -651762031;    double mRofNkMnDhRxXEdoh43853807 = -997872687;    double mRofNkMnDhRxXEdoh49004456 = 60467868;    double mRofNkMnDhRxXEdoh33709144 = -53101806;    double mRofNkMnDhRxXEdoh65315652 = -402951830;    double mRofNkMnDhRxXEdoh9708854 = 2969657;    double mRofNkMnDhRxXEdoh9451555 = -590221835;    double mRofNkMnDhRxXEdoh83558579 = -894875123;    double mRofNkMnDhRxXEdoh96562550 = -101841206;    double mRofNkMnDhRxXEdoh39722471 = -896369531;    double mRofNkMnDhRxXEdoh76530839 = -272523420;    double mRofNkMnDhRxXEdoh39540254 = -185331000;    double mRofNkMnDhRxXEdoh22770104 = 66122225;    double mRofNkMnDhRxXEdoh35987877 = -29815479;    double mRofNkMnDhRxXEdoh82869539 = -61660188;    double mRofNkMnDhRxXEdoh74529471 = -618091451;    double mRofNkMnDhRxXEdoh73099678 = -399820710;    double mRofNkMnDhRxXEdoh26113731 = -782144929;    double mRofNkMnDhRxXEdoh44291488 = -95020442;    double mRofNkMnDhRxXEdoh75981630 = -298388787;    double mRofNkMnDhRxXEdoh1200263 = 13265340;    double mRofNkMnDhRxXEdoh1789594 = -842438889;    double mRofNkMnDhRxXEdoh56221770 = -349189131;    double mRofNkMnDhRxXEdoh45896670 = -903601628;    double mRofNkMnDhRxXEdoh14012281 = -427524052;    double mRofNkMnDhRxXEdoh85092743 = 57469153;    double mRofNkMnDhRxXEdoh52555899 = -290054127;    double mRofNkMnDhRxXEdoh7305921 = 39610327;    double mRofNkMnDhRxXEdoh50904151 = -563605115;    double mRofNkMnDhRxXEdoh82221208 = -261170358;    double mRofNkMnDhRxXEdoh66696310 = -817823829;    double mRofNkMnDhRxXEdoh28689007 = -615846174;    double mRofNkMnDhRxXEdoh88684239 = -516584911;    double mRofNkMnDhRxXEdoh32790042 = -842476357;    double mRofNkMnDhRxXEdoh2272144 = 98242996;    double mRofNkMnDhRxXEdoh9229154 = 41984866;    double mRofNkMnDhRxXEdoh90751849 = -713693921;    double mRofNkMnDhRxXEdoh14128004 = -323475508;    double mRofNkMnDhRxXEdoh5913305 = -355704525;    double mRofNkMnDhRxXEdoh80886090 = -550194451;    double mRofNkMnDhRxXEdoh5480467 = -208101607;    double mRofNkMnDhRxXEdoh8646611 = -492130059;    double mRofNkMnDhRxXEdoh90603930 = -396303752;    double mRofNkMnDhRxXEdoh42650553 = -764660656;    double mRofNkMnDhRxXEdoh39709086 = -353001882;    double mRofNkMnDhRxXEdoh82514011 = -477100705;    double mRofNkMnDhRxXEdoh90173454 = -2037536;    double mRofNkMnDhRxXEdoh30121904 = -931441970;    double mRofNkMnDhRxXEdoh59722040 = -555745453;    double mRofNkMnDhRxXEdoh38054729 = -448341917;     mRofNkMnDhRxXEdoh28374533 = mRofNkMnDhRxXEdoh59349055;     mRofNkMnDhRxXEdoh59349055 = mRofNkMnDhRxXEdoh47046931;     mRofNkMnDhRxXEdoh47046931 = mRofNkMnDhRxXEdoh2034173;     mRofNkMnDhRxXEdoh2034173 = mRofNkMnDhRxXEdoh89985499;     mRofNkMnDhRxXEdoh89985499 = mRofNkMnDhRxXEdoh22473541;     mRofNkMnDhRxXEdoh22473541 = mRofNkMnDhRxXEdoh56358086;     mRofNkMnDhRxXEdoh56358086 = mRofNkMnDhRxXEdoh58735011;     mRofNkMnDhRxXEdoh58735011 = mRofNkMnDhRxXEdoh17369512;     mRofNkMnDhRxXEdoh17369512 = mRofNkMnDhRxXEdoh35952500;     mRofNkMnDhRxXEdoh35952500 = mRofNkMnDhRxXEdoh14285777;     mRofNkMnDhRxXEdoh14285777 = mRofNkMnDhRxXEdoh20936826;     mRofNkMnDhRxXEdoh20936826 = mRofNkMnDhRxXEdoh97342687;     mRofNkMnDhRxXEdoh97342687 = mRofNkMnDhRxXEdoh34886432;     mRofNkMnDhRxXEdoh34886432 = mRofNkMnDhRxXEdoh90913651;     mRofNkMnDhRxXEdoh90913651 = mRofNkMnDhRxXEdoh93405453;     mRofNkMnDhRxXEdoh93405453 = mRofNkMnDhRxXEdoh88956074;     mRofNkMnDhRxXEdoh88956074 = mRofNkMnDhRxXEdoh38594794;     mRofNkMnDhRxXEdoh38594794 = mRofNkMnDhRxXEdoh62136595;     mRofNkMnDhRxXEdoh62136595 = mRofNkMnDhRxXEdoh36066546;     mRofNkMnDhRxXEdoh36066546 = mRofNkMnDhRxXEdoh98036301;     mRofNkMnDhRxXEdoh98036301 = mRofNkMnDhRxXEdoh18665680;     mRofNkMnDhRxXEdoh18665680 = mRofNkMnDhRxXEdoh49897500;     mRofNkMnDhRxXEdoh49897500 = mRofNkMnDhRxXEdoh63488351;     mRofNkMnDhRxXEdoh63488351 = mRofNkMnDhRxXEdoh5471622;     mRofNkMnDhRxXEdoh5471622 = mRofNkMnDhRxXEdoh50263028;     mRofNkMnDhRxXEdoh50263028 = mRofNkMnDhRxXEdoh45942701;     mRofNkMnDhRxXEdoh45942701 = mRofNkMnDhRxXEdoh16817833;     mRofNkMnDhRxXEdoh16817833 = mRofNkMnDhRxXEdoh35964908;     mRofNkMnDhRxXEdoh35964908 = mRofNkMnDhRxXEdoh81381634;     mRofNkMnDhRxXEdoh81381634 = mRofNkMnDhRxXEdoh53082960;     mRofNkMnDhRxXEdoh53082960 = mRofNkMnDhRxXEdoh39756306;     mRofNkMnDhRxXEdoh39756306 = mRofNkMnDhRxXEdoh47837147;     mRofNkMnDhRxXEdoh47837147 = mRofNkMnDhRxXEdoh71228956;     mRofNkMnDhRxXEdoh71228956 = mRofNkMnDhRxXEdoh90594943;     mRofNkMnDhRxXEdoh90594943 = mRofNkMnDhRxXEdoh14932022;     mRofNkMnDhRxXEdoh14932022 = mRofNkMnDhRxXEdoh92205190;     mRofNkMnDhRxXEdoh92205190 = mRofNkMnDhRxXEdoh87166481;     mRofNkMnDhRxXEdoh87166481 = mRofNkMnDhRxXEdoh82373024;     mRofNkMnDhRxXEdoh82373024 = mRofNkMnDhRxXEdoh16239926;     mRofNkMnDhRxXEdoh16239926 = mRofNkMnDhRxXEdoh22054265;     mRofNkMnDhRxXEdoh22054265 = mRofNkMnDhRxXEdoh12943558;     mRofNkMnDhRxXEdoh12943558 = mRofNkMnDhRxXEdoh66109780;     mRofNkMnDhRxXEdoh66109780 = mRofNkMnDhRxXEdoh42591580;     mRofNkMnDhRxXEdoh42591580 = mRofNkMnDhRxXEdoh12584200;     mRofNkMnDhRxXEdoh12584200 = mRofNkMnDhRxXEdoh23250414;     mRofNkMnDhRxXEdoh23250414 = mRofNkMnDhRxXEdoh83566717;     mRofNkMnDhRxXEdoh83566717 = mRofNkMnDhRxXEdoh17253694;     mRofNkMnDhRxXEdoh17253694 = mRofNkMnDhRxXEdoh28133593;     mRofNkMnDhRxXEdoh28133593 = mRofNkMnDhRxXEdoh3174867;     mRofNkMnDhRxXEdoh3174867 = mRofNkMnDhRxXEdoh79109491;     mRofNkMnDhRxXEdoh79109491 = mRofNkMnDhRxXEdoh43853807;     mRofNkMnDhRxXEdoh43853807 = mRofNkMnDhRxXEdoh49004456;     mRofNkMnDhRxXEdoh49004456 = mRofNkMnDhRxXEdoh33709144;     mRofNkMnDhRxXEdoh33709144 = mRofNkMnDhRxXEdoh65315652;     mRofNkMnDhRxXEdoh65315652 = mRofNkMnDhRxXEdoh9708854;     mRofNkMnDhRxXEdoh9708854 = mRofNkMnDhRxXEdoh9451555;     mRofNkMnDhRxXEdoh9451555 = mRofNkMnDhRxXEdoh83558579;     mRofNkMnDhRxXEdoh83558579 = mRofNkMnDhRxXEdoh96562550;     mRofNkMnDhRxXEdoh96562550 = mRofNkMnDhRxXEdoh39722471;     mRofNkMnDhRxXEdoh39722471 = mRofNkMnDhRxXEdoh76530839;     mRofNkMnDhRxXEdoh76530839 = mRofNkMnDhRxXEdoh39540254;     mRofNkMnDhRxXEdoh39540254 = mRofNkMnDhRxXEdoh22770104;     mRofNkMnDhRxXEdoh22770104 = mRofNkMnDhRxXEdoh35987877;     mRofNkMnDhRxXEdoh35987877 = mRofNkMnDhRxXEdoh82869539;     mRofNkMnDhRxXEdoh82869539 = mRofNkMnDhRxXEdoh74529471;     mRofNkMnDhRxXEdoh74529471 = mRofNkMnDhRxXEdoh73099678;     mRofNkMnDhRxXEdoh73099678 = mRofNkMnDhRxXEdoh26113731;     mRofNkMnDhRxXEdoh26113731 = mRofNkMnDhRxXEdoh44291488;     mRofNkMnDhRxXEdoh44291488 = mRofNkMnDhRxXEdoh75981630;     mRofNkMnDhRxXEdoh75981630 = mRofNkMnDhRxXEdoh1200263;     mRofNkMnDhRxXEdoh1200263 = mRofNkMnDhRxXEdoh1789594;     mRofNkMnDhRxXEdoh1789594 = mRofNkMnDhRxXEdoh56221770;     mRofNkMnDhRxXEdoh56221770 = mRofNkMnDhRxXEdoh45896670;     mRofNkMnDhRxXEdoh45896670 = mRofNkMnDhRxXEdoh14012281;     mRofNkMnDhRxXEdoh14012281 = mRofNkMnDhRxXEdoh85092743;     mRofNkMnDhRxXEdoh85092743 = mRofNkMnDhRxXEdoh52555899;     mRofNkMnDhRxXEdoh52555899 = mRofNkMnDhRxXEdoh7305921;     mRofNkMnDhRxXEdoh7305921 = mRofNkMnDhRxXEdoh50904151;     mRofNkMnDhRxXEdoh50904151 = mRofNkMnDhRxXEdoh82221208;     mRofNkMnDhRxXEdoh82221208 = mRofNkMnDhRxXEdoh66696310;     mRofNkMnDhRxXEdoh66696310 = mRofNkMnDhRxXEdoh28689007;     mRofNkMnDhRxXEdoh28689007 = mRofNkMnDhRxXEdoh88684239;     mRofNkMnDhRxXEdoh88684239 = mRofNkMnDhRxXEdoh32790042;     mRofNkMnDhRxXEdoh32790042 = mRofNkMnDhRxXEdoh2272144;     mRofNkMnDhRxXEdoh2272144 = mRofNkMnDhRxXEdoh9229154;     mRofNkMnDhRxXEdoh9229154 = mRofNkMnDhRxXEdoh90751849;     mRofNkMnDhRxXEdoh90751849 = mRofNkMnDhRxXEdoh14128004;     mRofNkMnDhRxXEdoh14128004 = mRofNkMnDhRxXEdoh5913305;     mRofNkMnDhRxXEdoh5913305 = mRofNkMnDhRxXEdoh80886090;     mRofNkMnDhRxXEdoh80886090 = mRofNkMnDhRxXEdoh5480467;     mRofNkMnDhRxXEdoh5480467 = mRofNkMnDhRxXEdoh8646611;     mRofNkMnDhRxXEdoh8646611 = mRofNkMnDhRxXEdoh90603930;     mRofNkMnDhRxXEdoh90603930 = mRofNkMnDhRxXEdoh42650553;     mRofNkMnDhRxXEdoh42650553 = mRofNkMnDhRxXEdoh39709086;     mRofNkMnDhRxXEdoh39709086 = mRofNkMnDhRxXEdoh82514011;     mRofNkMnDhRxXEdoh82514011 = mRofNkMnDhRxXEdoh90173454;     mRofNkMnDhRxXEdoh90173454 = mRofNkMnDhRxXEdoh30121904;     mRofNkMnDhRxXEdoh30121904 = mRofNkMnDhRxXEdoh59722040;     mRofNkMnDhRxXEdoh59722040 = mRofNkMnDhRxXEdoh38054729;     mRofNkMnDhRxXEdoh38054729 = mRofNkMnDhRxXEdoh28374533;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ulZLGpGGLExamvqV30643552() {     double TPlAlvWuWloejOBgO63833501 = -914618817;    double TPlAlvWuWloejOBgO72621133 = -248568753;    double TPlAlvWuWloejOBgO84433407 = -963082818;    double TPlAlvWuWloejOBgO74257527 = -247479479;    double TPlAlvWuWloejOBgO46270768 = 16121796;    double TPlAlvWuWloejOBgO58696412 = -411829021;    double TPlAlvWuWloejOBgO67015492 = -980773338;    double TPlAlvWuWloejOBgO20866573 = -610048489;    double TPlAlvWuWloejOBgO18887117 = -171665920;    double TPlAlvWuWloejOBgO50855212 = -882235006;    double TPlAlvWuWloejOBgO52467512 = -173325233;    double TPlAlvWuWloejOBgO21504974 = -840518795;    double TPlAlvWuWloejOBgO35577333 = -92813788;    double TPlAlvWuWloejOBgO88027295 = -818608644;    double TPlAlvWuWloejOBgO49108168 = -700137244;    double TPlAlvWuWloejOBgO76946754 = -109551448;    double TPlAlvWuWloejOBgO44792010 = -967558400;    double TPlAlvWuWloejOBgO38354986 = -83647387;    double TPlAlvWuWloejOBgO62686925 = -945690350;    double TPlAlvWuWloejOBgO29903764 = -616516821;    double TPlAlvWuWloejOBgO11123164 = -835720846;    double TPlAlvWuWloejOBgO22087205 = -437947869;    double TPlAlvWuWloejOBgO20422880 = -415427497;    double TPlAlvWuWloejOBgO57281296 = -813365224;    double TPlAlvWuWloejOBgO13489573 = -248472400;    double TPlAlvWuWloejOBgO60961200 = -770486634;    double TPlAlvWuWloejOBgO6363275 = -683241847;    double TPlAlvWuWloejOBgO57546258 = 79511221;    double TPlAlvWuWloejOBgO82633122 = -406031377;    double TPlAlvWuWloejOBgO86915045 = -688026204;    double TPlAlvWuWloejOBgO71666192 = -507868307;    double TPlAlvWuWloejOBgO77579324 = -526197467;    double TPlAlvWuWloejOBgO60055158 = -624173291;    double TPlAlvWuWloejOBgO977384 = -923350168;    double TPlAlvWuWloejOBgO56403397 = -598325044;    double TPlAlvWuWloejOBgO66492865 = -12888748;    double TPlAlvWuWloejOBgO41617007 = -251701222;    double TPlAlvWuWloejOBgO38381885 = -580800540;    double TPlAlvWuWloejOBgO12763586 = -311906463;    double TPlAlvWuWloejOBgO80511457 = -190787847;    double TPlAlvWuWloejOBgO55505872 = -531630618;    double TPlAlvWuWloejOBgO2308685 = -730460601;    double TPlAlvWuWloejOBgO55493159 = -614438235;    double TPlAlvWuWloejOBgO82722918 = -221738050;    double TPlAlvWuWloejOBgO12600659 = -565446667;    double TPlAlvWuWloejOBgO69211742 = 97759523;    double TPlAlvWuWloejOBgO2911882 = -648072359;    double TPlAlvWuWloejOBgO34980006 = -327638126;    double TPlAlvWuWloejOBgO51026259 = -997868574;    double TPlAlvWuWloejOBgO27704456 = -480487033;    double TPlAlvWuWloejOBgO59880215 = -526038588;    double TPlAlvWuWloejOBgO27339279 = -111525033;    double TPlAlvWuWloejOBgO17625605 = -795577686;    double TPlAlvWuWloejOBgO49700990 = -244610306;    double TPlAlvWuWloejOBgO19520388 = -749441975;    double TPlAlvWuWloejOBgO41746297 = -376670948;    double TPlAlvWuWloejOBgO52198254 = -833141257;    double TPlAlvWuWloejOBgO27152112 = -49717594;    double TPlAlvWuWloejOBgO60767954 = -999007079;    double TPlAlvWuWloejOBgO85309568 = -213391570;    double TPlAlvWuWloejOBgO52333138 = -728587175;    double TPlAlvWuWloejOBgO9469235 = -960284559;    double TPlAlvWuWloejOBgO38233450 = -104017113;    double TPlAlvWuWloejOBgO31972071 = -483639716;    double TPlAlvWuWloejOBgO79189020 = -274366699;    double TPlAlvWuWloejOBgO74888187 = -647127766;    double TPlAlvWuWloejOBgO61449816 = -116345504;    double TPlAlvWuWloejOBgO34599950 = -169463621;    double TPlAlvWuWloejOBgO31623898 = -120283601;    double TPlAlvWuWloejOBgO82615302 = -587248497;    double TPlAlvWuWloejOBgO35329747 = -857850226;    double TPlAlvWuWloejOBgO6410126 = -286757861;    double TPlAlvWuWloejOBgO25591401 = -771740924;    double TPlAlvWuWloejOBgO82175467 = -654902503;    double TPlAlvWuWloejOBgO74397892 = 15113797;    double TPlAlvWuWloejOBgO8814480 = -5260245;    double TPlAlvWuWloejOBgO66594045 = -823509635;    double TPlAlvWuWloejOBgO37699962 = -93689447;    double TPlAlvWuWloejOBgO44680638 = -147918558;    double TPlAlvWuWloejOBgO44277831 = -246231924;    double TPlAlvWuWloejOBgO58049318 = -22414276;    double TPlAlvWuWloejOBgO71383269 = -255603721;    double TPlAlvWuWloejOBgO6519999 = 77379795;    double TPlAlvWuWloejOBgO54928667 = -925544344;    double TPlAlvWuWloejOBgO27034830 = -61987617;    double TPlAlvWuWloejOBgO44326913 = -296343274;    double TPlAlvWuWloejOBgO59953719 = -730619782;    double TPlAlvWuWloejOBgO10354168 = -279562985;    double TPlAlvWuWloejOBgO81456995 = -73908193;    double TPlAlvWuWloejOBgO14657101 = -121654096;    double TPlAlvWuWloejOBgO14294612 = -179747491;    double TPlAlvWuWloejOBgO14464896 = -101983628;    double TPlAlvWuWloejOBgO77613930 = -581793461;    double TPlAlvWuWloejOBgO27454017 = 1485106;    double TPlAlvWuWloejOBgO28178320 = -462200673;    double TPlAlvWuWloejOBgO46036638 = -571346059;    double TPlAlvWuWloejOBgO64075235 = -526443489;    double TPlAlvWuWloejOBgO23521088 = -30798520;    double TPlAlvWuWloejOBgO3533898 = -947371352;    double TPlAlvWuWloejOBgO37712471 = -914618817;     TPlAlvWuWloejOBgO63833501 = TPlAlvWuWloejOBgO72621133;     TPlAlvWuWloejOBgO72621133 = TPlAlvWuWloejOBgO84433407;     TPlAlvWuWloejOBgO84433407 = TPlAlvWuWloejOBgO74257527;     TPlAlvWuWloejOBgO74257527 = TPlAlvWuWloejOBgO46270768;     TPlAlvWuWloejOBgO46270768 = TPlAlvWuWloejOBgO58696412;     TPlAlvWuWloejOBgO58696412 = TPlAlvWuWloejOBgO67015492;     TPlAlvWuWloejOBgO67015492 = TPlAlvWuWloejOBgO20866573;     TPlAlvWuWloejOBgO20866573 = TPlAlvWuWloejOBgO18887117;     TPlAlvWuWloejOBgO18887117 = TPlAlvWuWloejOBgO50855212;     TPlAlvWuWloejOBgO50855212 = TPlAlvWuWloejOBgO52467512;     TPlAlvWuWloejOBgO52467512 = TPlAlvWuWloejOBgO21504974;     TPlAlvWuWloejOBgO21504974 = TPlAlvWuWloejOBgO35577333;     TPlAlvWuWloejOBgO35577333 = TPlAlvWuWloejOBgO88027295;     TPlAlvWuWloejOBgO88027295 = TPlAlvWuWloejOBgO49108168;     TPlAlvWuWloejOBgO49108168 = TPlAlvWuWloejOBgO76946754;     TPlAlvWuWloejOBgO76946754 = TPlAlvWuWloejOBgO44792010;     TPlAlvWuWloejOBgO44792010 = TPlAlvWuWloejOBgO38354986;     TPlAlvWuWloejOBgO38354986 = TPlAlvWuWloejOBgO62686925;     TPlAlvWuWloejOBgO62686925 = TPlAlvWuWloejOBgO29903764;     TPlAlvWuWloejOBgO29903764 = TPlAlvWuWloejOBgO11123164;     TPlAlvWuWloejOBgO11123164 = TPlAlvWuWloejOBgO22087205;     TPlAlvWuWloejOBgO22087205 = TPlAlvWuWloejOBgO20422880;     TPlAlvWuWloejOBgO20422880 = TPlAlvWuWloejOBgO57281296;     TPlAlvWuWloejOBgO57281296 = TPlAlvWuWloejOBgO13489573;     TPlAlvWuWloejOBgO13489573 = TPlAlvWuWloejOBgO60961200;     TPlAlvWuWloejOBgO60961200 = TPlAlvWuWloejOBgO6363275;     TPlAlvWuWloejOBgO6363275 = TPlAlvWuWloejOBgO57546258;     TPlAlvWuWloejOBgO57546258 = TPlAlvWuWloejOBgO82633122;     TPlAlvWuWloejOBgO82633122 = TPlAlvWuWloejOBgO86915045;     TPlAlvWuWloejOBgO86915045 = TPlAlvWuWloejOBgO71666192;     TPlAlvWuWloejOBgO71666192 = TPlAlvWuWloejOBgO77579324;     TPlAlvWuWloejOBgO77579324 = TPlAlvWuWloejOBgO60055158;     TPlAlvWuWloejOBgO60055158 = TPlAlvWuWloejOBgO977384;     TPlAlvWuWloejOBgO977384 = TPlAlvWuWloejOBgO56403397;     TPlAlvWuWloejOBgO56403397 = TPlAlvWuWloejOBgO66492865;     TPlAlvWuWloejOBgO66492865 = TPlAlvWuWloejOBgO41617007;     TPlAlvWuWloejOBgO41617007 = TPlAlvWuWloejOBgO38381885;     TPlAlvWuWloejOBgO38381885 = TPlAlvWuWloejOBgO12763586;     TPlAlvWuWloejOBgO12763586 = TPlAlvWuWloejOBgO80511457;     TPlAlvWuWloejOBgO80511457 = TPlAlvWuWloejOBgO55505872;     TPlAlvWuWloejOBgO55505872 = TPlAlvWuWloejOBgO2308685;     TPlAlvWuWloejOBgO2308685 = TPlAlvWuWloejOBgO55493159;     TPlAlvWuWloejOBgO55493159 = TPlAlvWuWloejOBgO82722918;     TPlAlvWuWloejOBgO82722918 = TPlAlvWuWloejOBgO12600659;     TPlAlvWuWloejOBgO12600659 = TPlAlvWuWloejOBgO69211742;     TPlAlvWuWloejOBgO69211742 = TPlAlvWuWloejOBgO2911882;     TPlAlvWuWloejOBgO2911882 = TPlAlvWuWloejOBgO34980006;     TPlAlvWuWloejOBgO34980006 = TPlAlvWuWloejOBgO51026259;     TPlAlvWuWloejOBgO51026259 = TPlAlvWuWloejOBgO27704456;     TPlAlvWuWloejOBgO27704456 = TPlAlvWuWloejOBgO59880215;     TPlAlvWuWloejOBgO59880215 = TPlAlvWuWloejOBgO27339279;     TPlAlvWuWloejOBgO27339279 = TPlAlvWuWloejOBgO17625605;     TPlAlvWuWloejOBgO17625605 = TPlAlvWuWloejOBgO49700990;     TPlAlvWuWloejOBgO49700990 = TPlAlvWuWloejOBgO19520388;     TPlAlvWuWloejOBgO19520388 = TPlAlvWuWloejOBgO41746297;     TPlAlvWuWloejOBgO41746297 = TPlAlvWuWloejOBgO52198254;     TPlAlvWuWloejOBgO52198254 = TPlAlvWuWloejOBgO27152112;     TPlAlvWuWloejOBgO27152112 = TPlAlvWuWloejOBgO60767954;     TPlAlvWuWloejOBgO60767954 = TPlAlvWuWloejOBgO85309568;     TPlAlvWuWloejOBgO85309568 = TPlAlvWuWloejOBgO52333138;     TPlAlvWuWloejOBgO52333138 = TPlAlvWuWloejOBgO9469235;     TPlAlvWuWloejOBgO9469235 = TPlAlvWuWloejOBgO38233450;     TPlAlvWuWloejOBgO38233450 = TPlAlvWuWloejOBgO31972071;     TPlAlvWuWloejOBgO31972071 = TPlAlvWuWloejOBgO79189020;     TPlAlvWuWloejOBgO79189020 = TPlAlvWuWloejOBgO74888187;     TPlAlvWuWloejOBgO74888187 = TPlAlvWuWloejOBgO61449816;     TPlAlvWuWloejOBgO61449816 = TPlAlvWuWloejOBgO34599950;     TPlAlvWuWloejOBgO34599950 = TPlAlvWuWloejOBgO31623898;     TPlAlvWuWloejOBgO31623898 = TPlAlvWuWloejOBgO82615302;     TPlAlvWuWloejOBgO82615302 = TPlAlvWuWloejOBgO35329747;     TPlAlvWuWloejOBgO35329747 = TPlAlvWuWloejOBgO6410126;     TPlAlvWuWloejOBgO6410126 = TPlAlvWuWloejOBgO25591401;     TPlAlvWuWloejOBgO25591401 = TPlAlvWuWloejOBgO82175467;     TPlAlvWuWloejOBgO82175467 = TPlAlvWuWloejOBgO74397892;     TPlAlvWuWloejOBgO74397892 = TPlAlvWuWloejOBgO8814480;     TPlAlvWuWloejOBgO8814480 = TPlAlvWuWloejOBgO66594045;     TPlAlvWuWloejOBgO66594045 = TPlAlvWuWloejOBgO37699962;     TPlAlvWuWloejOBgO37699962 = TPlAlvWuWloejOBgO44680638;     TPlAlvWuWloejOBgO44680638 = TPlAlvWuWloejOBgO44277831;     TPlAlvWuWloejOBgO44277831 = TPlAlvWuWloejOBgO58049318;     TPlAlvWuWloejOBgO58049318 = TPlAlvWuWloejOBgO71383269;     TPlAlvWuWloejOBgO71383269 = TPlAlvWuWloejOBgO6519999;     TPlAlvWuWloejOBgO6519999 = TPlAlvWuWloejOBgO54928667;     TPlAlvWuWloejOBgO54928667 = TPlAlvWuWloejOBgO27034830;     TPlAlvWuWloejOBgO27034830 = TPlAlvWuWloejOBgO44326913;     TPlAlvWuWloejOBgO44326913 = TPlAlvWuWloejOBgO59953719;     TPlAlvWuWloejOBgO59953719 = TPlAlvWuWloejOBgO10354168;     TPlAlvWuWloejOBgO10354168 = TPlAlvWuWloejOBgO81456995;     TPlAlvWuWloejOBgO81456995 = TPlAlvWuWloejOBgO14657101;     TPlAlvWuWloejOBgO14657101 = TPlAlvWuWloejOBgO14294612;     TPlAlvWuWloejOBgO14294612 = TPlAlvWuWloejOBgO14464896;     TPlAlvWuWloejOBgO14464896 = TPlAlvWuWloejOBgO77613930;     TPlAlvWuWloejOBgO77613930 = TPlAlvWuWloejOBgO27454017;     TPlAlvWuWloejOBgO27454017 = TPlAlvWuWloejOBgO28178320;     TPlAlvWuWloejOBgO28178320 = TPlAlvWuWloejOBgO46036638;     TPlAlvWuWloejOBgO46036638 = TPlAlvWuWloejOBgO64075235;     TPlAlvWuWloejOBgO64075235 = TPlAlvWuWloejOBgO23521088;     TPlAlvWuWloejOBgO23521088 = TPlAlvWuWloejOBgO3533898;     TPlAlvWuWloejOBgO3533898 = TPlAlvWuWloejOBgO37712471;     TPlAlvWuWloejOBgO37712471 = TPlAlvWuWloejOBgO63833501;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SeKmLmKpnOIBSrxY45692620() {     double zArhUwvmWhERyEUnk69950608 = 73479294;    double zArhUwvmWhERyEUnk15994413 = 41831466;    double zArhUwvmWhERyEUnk475592 = -13915739;    double zArhUwvmWhERyEUnk1332309 = -438228109;    double zArhUwvmWhERyEUnk15166257 = -504528075;    double zArhUwvmWhERyEUnk50113615 = -676325256;    double zArhUwvmWhERyEUnk27504459 = -19738315;    double zArhUwvmWhERyEUnk70626463 = -896922402;    double zArhUwvmWhERyEUnk83529612 = 60301415;    double zArhUwvmWhERyEUnk92084871 = -111210483;    double zArhUwvmWhERyEUnk12577803 = -50762819;    double zArhUwvmWhERyEUnk53328017 = -184924996;    double zArhUwvmWhERyEUnk21786452 = -532704877;    double zArhUwvmWhERyEUnk71170972 = -250847212;    double zArhUwvmWhERyEUnk44850595 = -290599410;    double zArhUwvmWhERyEUnk18452148 = -82550991;    double zArhUwvmWhERyEUnk38628014 = -484103513;    double zArhUwvmWhERyEUnk35408139 = -849170029;    double zArhUwvmWhERyEUnk24516767 = -251459447;    double zArhUwvmWhERyEUnk10453929 = -973419741;    double zArhUwvmWhERyEUnk99676357 = -691208301;    double zArhUwvmWhERyEUnk8608228 = -345877585;    double zArhUwvmWhERyEUnk56013672 = -161944983;    double zArhUwvmWhERyEUnk41273670 = 64329788;    double zArhUwvmWhERyEUnk57189223 = -621668433;    double zArhUwvmWhERyEUnk85682875 = -649141958;    double zArhUwvmWhERyEUnk4823053 = -583904050;    double zArhUwvmWhERyEUnk70653383 = 13582267;    double zArhUwvmWhERyEUnk48765378 = -628414081;    double zArhUwvmWhERyEUnk55530682 = -505497072;    double zArhUwvmWhERyEUnk63908741 = -421189245;    double zArhUwvmWhERyEUnk75189946 = -235644282;    double zArhUwvmWhERyEUnk69766106 = -516786661;    double zArhUwvmWhERyEUnk95807947 = -674891933;    double zArhUwvmWhERyEUnk17878049 = -137058149;    double zArhUwvmWhERyEUnk35218779 = -226914855;    double zArhUwvmWhERyEUnk22878942 = -205337091;    double zArhUwvmWhERyEUnk72472097 = -352306549;    double zArhUwvmWhERyEUnk86373839 = -305829960;    double zArhUwvmWhERyEUnk68013395 = -664227070;    double zArhUwvmWhERyEUnk61175096 = -596904513;    double zArhUwvmWhERyEUnk34459142 = 99397121;    double zArhUwvmWhERyEUnk33560137 = -695242819;    double zArhUwvmWhERyEUnk70130658 = -241080988;    double zArhUwvmWhERyEUnk24553124 = -227344821;    double zArhUwvmWhERyEUnk81150937 = -987456593;    double zArhUwvmWhERyEUnk20690456 = -766847897;    double zArhUwvmWhERyEUnk42936946 = -337533658;    double zArhUwvmWhERyEUnk60701988 = 16394438;    double zArhUwvmWhERyEUnk40616109 = -567497841;    double zArhUwvmWhERyEUnk95580893 = -368332259;    double zArhUwvmWhERyEUnk38148998 = 87748647;    double zArhUwvmWhERyEUnk65874062 = -587367273;    double zArhUwvmWhERyEUnk45669870 = -565080451;    double zArhUwvmWhERyEUnk46016263 = -349973758;    double zArhUwvmWhERyEUnk61342380 = -580643121;    double zArhUwvmWhERyEUnk59980741 = -796223551;    double zArhUwvmWhERyEUnk59201921 = 21754473;    double zArhUwvmWhERyEUnk44143085 = -816559677;    double zArhUwvmWhERyEUnk29483382 = -855386118;    double zArhUwvmWhERyEUnk45290562 = 7578793;    double zArhUwvmWhERyEUnk56851075 = 66679417;    double zArhUwvmWhERyEUnk21861085 = -168508322;    double zArhUwvmWhERyEUnk27998930 = -434201514;    double zArhUwvmWhERyEUnk28176130 = -690021238;    double zArhUwvmWhERyEUnk37387856 = -815118537;    double zArhUwvmWhERyEUnk83561910 = -668138336;    double zArhUwvmWhERyEUnk25978504 = -857812944;    double zArhUwvmWhERyEUnk53292923 = -13789063;    double zArhUwvmWhERyEUnk9631817 = 36315445;    double zArhUwvmWhERyEUnk95573206 = -877213900;    double zArhUwvmWhERyEUnk66155916 = -31796964;    double zArhUwvmWhERyEUnk49034300 = -443340070;    double zArhUwvmWhERyEUnk56503372 = -587232377;    double zArhUwvmWhERyEUnk49278833 = -276515229;    double zArhUwvmWhERyEUnk65217216 = -690605423;    double zArhUwvmWhERyEUnk75048091 = -650634767;    double zArhUwvmWhERyEUnk85883013 = -920863995;    double zArhUwvmWhERyEUnk16720547 = -708325392;    double zArhUwvmWhERyEUnk76038286 = -634211840;    double zArhUwvmWhERyEUnk64992419 = -882294061;    double zArhUwvmWhERyEUnk61886106 = -146370392;    double zArhUwvmWhERyEUnk9951395 = 97187829;    double zArhUwvmWhERyEUnk8149270 = 39083760;    double zArhUwvmWhERyEUnk59949789 = -37164813;    double zArhUwvmWhERyEUnk25759743 = -408937892;    double zArhUwvmWhERyEUnk9315884 = -648277010;    double zArhUwvmWhERyEUnk24096236 = -951706211;    double zArhUwvmWhERyEUnk49791684 = -224918176;    double zArhUwvmWhERyEUnk56535669 = -556415029;    double zArhUwvmWhERyEUnk75238037 = -430691305;    double zArhUwvmWhERyEUnk63677020 = -127091565;    double zArhUwvmWhERyEUnk28329012 = -535746873;    double zArhUwvmWhERyEUnk56890457 = -450443843;    double zArhUwvmWhERyEUnk22722834 = -571805864;    double zArhUwvmWhERyEUnk4324021 = -563583930;    double zArhUwvmWhERyEUnk12598057 = -732094558;    double zArhUwvmWhERyEUnk5561208 = -161041305;    double zArhUwvmWhERyEUnk41954528 = -551059751;    double zArhUwvmWhERyEUnk87165267 = 73479294;     zArhUwvmWhERyEUnk69950608 = zArhUwvmWhERyEUnk15994413;     zArhUwvmWhERyEUnk15994413 = zArhUwvmWhERyEUnk475592;     zArhUwvmWhERyEUnk475592 = zArhUwvmWhERyEUnk1332309;     zArhUwvmWhERyEUnk1332309 = zArhUwvmWhERyEUnk15166257;     zArhUwvmWhERyEUnk15166257 = zArhUwvmWhERyEUnk50113615;     zArhUwvmWhERyEUnk50113615 = zArhUwvmWhERyEUnk27504459;     zArhUwvmWhERyEUnk27504459 = zArhUwvmWhERyEUnk70626463;     zArhUwvmWhERyEUnk70626463 = zArhUwvmWhERyEUnk83529612;     zArhUwvmWhERyEUnk83529612 = zArhUwvmWhERyEUnk92084871;     zArhUwvmWhERyEUnk92084871 = zArhUwvmWhERyEUnk12577803;     zArhUwvmWhERyEUnk12577803 = zArhUwvmWhERyEUnk53328017;     zArhUwvmWhERyEUnk53328017 = zArhUwvmWhERyEUnk21786452;     zArhUwvmWhERyEUnk21786452 = zArhUwvmWhERyEUnk71170972;     zArhUwvmWhERyEUnk71170972 = zArhUwvmWhERyEUnk44850595;     zArhUwvmWhERyEUnk44850595 = zArhUwvmWhERyEUnk18452148;     zArhUwvmWhERyEUnk18452148 = zArhUwvmWhERyEUnk38628014;     zArhUwvmWhERyEUnk38628014 = zArhUwvmWhERyEUnk35408139;     zArhUwvmWhERyEUnk35408139 = zArhUwvmWhERyEUnk24516767;     zArhUwvmWhERyEUnk24516767 = zArhUwvmWhERyEUnk10453929;     zArhUwvmWhERyEUnk10453929 = zArhUwvmWhERyEUnk99676357;     zArhUwvmWhERyEUnk99676357 = zArhUwvmWhERyEUnk8608228;     zArhUwvmWhERyEUnk8608228 = zArhUwvmWhERyEUnk56013672;     zArhUwvmWhERyEUnk56013672 = zArhUwvmWhERyEUnk41273670;     zArhUwvmWhERyEUnk41273670 = zArhUwvmWhERyEUnk57189223;     zArhUwvmWhERyEUnk57189223 = zArhUwvmWhERyEUnk85682875;     zArhUwvmWhERyEUnk85682875 = zArhUwvmWhERyEUnk4823053;     zArhUwvmWhERyEUnk4823053 = zArhUwvmWhERyEUnk70653383;     zArhUwvmWhERyEUnk70653383 = zArhUwvmWhERyEUnk48765378;     zArhUwvmWhERyEUnk48765378 = zArhUwvmWhERyEUnk55530682;     zArhUwvmWhERyEUnk55530682 = zArhUwvmWhERyEUnk63908741;     zArhUwvmWhERyEUnk63908741 = zArhUwvmWhERyEUnk75189946;     zArhUwvmWhERyEUnk75189946 = zArhUwvmWhERyEUnk69766106;     zArhUwvmWhERyEUnk69766106 = zArhUwvmWhERyEUnk95807947;     zArhUwvmWhERyEUnk95807947 = zArhUwvmWhERyEUnk17878049;     zArhUwvmWhERyEUnk17878049 = zArhUwvmWhERyEUnk35218779;     zArhUwvmWhERyEUnk35218779 = zArhUwvmWhERyEUnk22878942;     zArhUwvmWhERyEUnk22878942 = zArhUwvmWhERyEUnk72472097;     zArhUwvmWhERyEUnk72472097 = zArhUwvmWhERyEUnk86373839;     zArhUwvmWhERyEUnk86373839 = zArhUwvmWhERyEUnk68013395;     zArhUwvmWhERyEUnk68013395 = zArhUwvmWhERyEUnk61175096;     zArhUwvmWhERyEUnk61175096 = zArhUwvmWhERyEUnk34459142;     zArhUwvmWhERyEUnk34459142 = zArhUwvmWhERyEUnk33560137;     zArhUwvmWhERyEUnk33560137 = zArhUwvmWhERyEUnk70130658;     zArhUwvmWhERyEUnk70130658 = zArhUwvmWhERyEUnk24553124;     zArhUwvmWhERyEUnk24553124 = zArhUwvmWhERyEUnk81150937;     zArhUwvmWhERyEUnk81150937 = zArhUwvmWhERyEUnk20690456;     zArhUwvmWhERyEUnk20690456 = zArhUwvmWhERyEUnk42936946;     zArhUwvmWhERyEUnk42936946 = zArhUwvmWhERyEUnk60701988;     zArhUwvmWhERyEUnk60701988 = zArhUwvmWhERyEUnk40616109;     zArhUwvmWhERyEUnk40616109 = zArhUwvmWhERyEUnk95580893;     zArhUwvmWhERyEUnk95580893 = zArhUwvmWhERyEUnk38148998;     zArhUwvmWhERyEUnk38148998 = zArhUwvmWhERyEUnk65874062;     zArhUwvmWhERyEUnk65874062 = zArhUwvmWhERyEUnk45669870;     zArhUwvmWhERyEUnk45669870 = zArhUwvmWhERyEUnk46016263;     zArhUwvmWhERyEUnk46016263 = zArhUwvmWhERyEUnk61342380;     zArhUwvmWhERyEUnk61342380 = zArhUwvmWhERyEUnk59980741;     zArhUwvmWhERyEUnk59980741 = zArhUwvmWhERyEUnk59201921;     zArhUwvmWhERyEUnk59201921 = zArhUwvmWhERyEUnk44143085;     zArhUwvmWhERyEUnk44143085 = zArhUwvmWhERyEUnk29483382;     zArhUwvmWhERyEUnk29483382 = zArhUwvmWhERyEUnk45290562;     zArhUwvmWhERyEUnk45290562 = zArhUwvmWhERyEUnk56851075;     zArhUwvmWhERyEUnk56851075 = zArhUwvmWhERyEUnk21861085;     zArhUwvmWhERyEUnk21861085 = zArhUwvmWhERyEUnk27998930;     zArhUwvmWhERyEUnk27998930 = zArhUwvmWhERyEUnk28176130;     zArhUwvmWhERyEUnk28176130 = zArhUwvmWhERyEUnk37387856;     zArhUwvmWhERyEUnk37387856 = zArhUwvmWhERyEUnk83561910;     zArhUwvmWhERyEUnk83561910 = zArhUwvmWhERyEUnk25978504;     zArhUwvmWhERyEUnk25978504 = zArhUwvmWhERyEUnk53292923;     zArhUwvmWhERyEUnk53292923 = zArhUwvmWhERyEUnk9631817;     zArhUwvmWhERyEUnk9631817 = zArhUwvmWhERyEUnk95573206;     zArhUwvmWhERyEUnk95573206 = zArhUwvmWhERyEUnk66155916;     zArhUwvmWhERyEUnk66155916 = zArhUwvmWhERyEUnk49034300;     zArhUwvmWhERyEUnk49034300 = zArhUwvmWhERyEUnk56503372;     zArhUwvmWhERyEUnk56503372 = zArhUwvmWhERyEUnk49278833;     zArhUwvmWhERyEUnk49278833 = zArhUwvmWhERyEUnk65217216;     zArhUwvmWhERyEUnk65217216 = zArhUwvmWhERyEUnk75048091;     zArhUwvmWhERyEUnk75048091 = zArhUwvmWhERyEUnk85883013;     zArhUwvmWhERyEUnk85883013 = zArhUwvmWhERyEUnk16720547;     zArhUwvmWhERyEUnk16720547 = zArhUwvmWhERyEUnk76038286;     zArhUwvmWhERyEUnk76038286 = zArhUwvmWhERyEUnk64992419;     zArhUwvmWhERyEUnk64992419 = zArhUwvmWhERyEUnk61886106;     zArhUwvmWhERyEUnk61886106 = zArhUwvmWhERyEUnk9951395;     zArhUwvmWhERyEUnk9951395 = zArhUwvmWhERyEUnk8149270;     zArhUwvmWhERyEUnk8149270 = zArhUwvmWhERyEUnk59949789;     zArhUwvmWhERyEUnk59949789 = zArhUwvmWhERyEUnk25759743;     zArhUwvmWhERyEUnk25759743 = zArhUwvmWhERyEUnk9315884;     zArhUwvmWhERyEUnk9315884 = zArhUwvmWhERyEUnk24096236;     zArhUwvmWhERyEUnk24096236 = zArhUwvmWhERyEUnk49791684;     zArhUwvmWhERyEUnk49791684 = zArhUwvmWhERyEUnk56535669;     zArhUwvmWhERyEUnk56535669 = zArhUwvmWhERyEUnk75238037;     zArhUwvmWhERyEUnk75238037 = zArhUwvmWhERyEUnk63677020;     zArhUwvmWhERyEUnk63677020 = zArhUwvmWhERyEUnk28329012;     zArhUwvmWhERyEUnk28329012 = zArhUwvmWhERyEUnk56890457;     zArhUwvmWhERyEUnk56890457 = zArhUwvmWhERyEUnk22722834;     zArhUwvmWhERyEUnk22722834 = zArhUwvmWhERyEUnk4324021;     zArhUwvmWhERyEUnk4324021 = zArhUwvmWhERyEUnk12598057;     zArhUwvmWhERyEUnk12598057 = zArhUwvmWhERyEUnk5561208;     zArhUwvmWhERyEUnk5561208 = zArhUwvmWhERyEUnk41954528;     zArhUwvmWhERyEUnk41954528 = zArhUwvmWhERyEUnk87165267;     zArhUwvmWhERyEUnk87165267 = zArhUwvmWhERyEUnk69950608;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ZAKhwJRGjmSUtygj12984220() {     double YucdNgWQPrNmsMFRS5409577 = -392797605;    double YucdNgWQPrNmsMFRS29266492 = -938720141;    double YucdNgWQPrNmsMFRS37862068 = -779582814;    double YucdNgWQPrNmsMFRS73555663 = -259379180;    double YucdNgWQPrNmsMFRS71451526 = -610126921;    double YucdNgWQPrNmsMFRS86336486 = -938673151;    double YucdNgWQPrNmsMFRS38161865 = -964464049;    double YucdNgWQPrNmsMFRS32758025 = -541156543;    double YucdNgWQPrNmsMFRS85047217 = -428029992;    double YucdNgWQPrNmsMFRS6987584 = -875897481;    double YucdNgWQPrNmsMFRS50759537 = -952770548;    double YucdNgWQPrNmsMFRS53896165 = -49045768;    double YucdNgWQPrNmsMFRS60021097 = 15282618;    double YucdNgWQPrNmsMFRS24311835 = -227210621;    double YucdNgWQPrNmsMFRS3045112 = -794024425;    double YucdNgWQPrNmsMFRS1993449 = -818362598;    double YucdNgWQPrNmsMFRS94463949 = 88921935;    double YucdNgWQPrNmsMFRS35168331 = -922598098;    double YucdNgWQPrNmsMFRS25067097 = -568022868;    double YucdNgWQPrNmsMFRS4291148 = -299980807;    double YucdNgWQPrNmsMFRS12763220 = -348482990;    double YucdNgWQPrNmsMFRS12029753 = -432513879;    double YucdNgWQPrNmsMFRS26539052 = -899577168;    double YucdNgWQPrNmsMFRS35066615 = -446494816;    double YucdNgWQPrNmsMFRS65207174 = -645653631;    double YucdNgWQPrNmsMFRS96381046 = -337718765;    double YucdNgWQPrNmsMFRS65243627 = -390188190;    double YucdNgWQPrNmsMFRS11381808 = -156189907;    double YucdNgWQPrNmsMFRS95433592 = -102508884;    double YucdNgWQPrNmsMFRS61064093 = -540004242;    double YucdNgWQPrNmsMFRS82491973 = -973169732;    double YucdNgWQPrNmsMFRS13012965 = -8615697;    double YucdNgWQPrNmsMFRS81984117 = -664382639;    double YucdNgWQPrNmsMFRS25556374 = -739585747;    double YucdNgWQPrNmsMFRS83686502 = -88158400;    double YucdNgWQPrNmsMFRS86779622 = -441480161;    double YucdNgWQPrNmsMFRS72290758 = -70033132;    double YucdNgWQPrNmsMFRS23687501 = -334962130;    double YucdNgWQPrNmsMFRS16764401 = 43293763;    double YucdNgWQPrNmsMFRS32284928 = -129489616;    double YucdNgWQPrNmsMFRS94626704 = -366103427;    double YucdNgWQPrNmsMFRS23824268 = -595148170;    double YucdNgWQPrNmsMFRS22943516 = -248423605;    double YucdNgWQPrNmsMFRS10261997 = -845413398;    double YucdNgWQPrNmsMFRS24569583 = -53855982;    double YucdNgWQPrNmsMFRS27112266 = 73619775;    double YucdNgWQPrNmsMFRS40035620 = -150834257;    double YucdNgWQPrNmsMFRS60663258 = -504060252;    double YucdNgWQPrNmsMFRS83594654 = -747342442;    double YucdNgWQPrNmsMFRS65145698 = 41475343;    double YucdNgWQPrNmsMFRS76351617 = -242608817;    double YucdNgWQPrNmsMFRS21634471 = -125903700;    double YucdNgWQPrNmsMFRS34495211 = -343412827;    double YucdNgWQPrNmsMFRS61661717 = -756588951;    double YucdNgWQPrNmsMFRS221000 = -696463904;    double YucdNgWQPrNmsMFRS93379823 = -960283727;    double YucdNgWQPrNmsMFRS2727440 = 60857026;    double YucdNgWQPrNmsMFRS2795453 = -233087998;    double YucdNgWQPrNmsMFRS8348489 = -613725550;    double YucdNgWQPrNmsMFRS75070479 = -172408157;    double YucdNgWQPrNmsMFRS21092860 = -448484961;    double YucdNgWQPrNmsMFRS26780057 = -708274143;    double YucdNgWQPrNmsMFRS37324432 = -338647660;    double YucdNgWQPrNmsMFRS23983124 = -888025750;    double YucdNgWQPrNmsMFRS24495611 = -902727749;    double YucdNgWQPrNmsMFRS37746573 = -844154852;    double YucdNgWQPrNmsMFRS71912048 = -384663130;    double YucdNgWQPrNmsMFRS34464723 = -245131636;    double YucdNgWQPrNmsMFRS40625332 = -39052222;    double YucdNgWQPrNmsMFRS16265489 = -252544265;    double YucdNgWQPrNmsMFRS29702691 = -648329467;    double YucdNgWQPrNmsMFRS70776449 = -576115936;    double YucdNgWQPrNmsMFRS18403930 = -865891862;    double YucdNgWQPrNmsMFRS92782169 = -338533253;    double YucdNgWQPrNmsMFRS9664444 = -933877380;    double YucdNgWQPrNmsMFRS88938952 = -753334820;    double YucdNgWQPrNmsMFRS89086237 = -84090275;    double YucdNgWQPrNmsMFRS16277055 = 45836230;    double YucdNgWQPrNmsMFRS10497033 = -292638834;    double YucdNgWQPrNmsMFRS38094909 = -619273406;    double YucdNgWQPrNmsMFRS56345427 = -86884508;    double YucdNgWQPrNmsMFRS4580369 = -886127939;    double YucdNgWQPrNmsMFRS27787154 = -408847465;    double YucdNgWQPrNmsMFRS30287895 = -43984227;    double YucdNgWQPrNmsMFRS84712475 = -197395426;    double YucdNgWQPrNmsMFRS60857502 = -747266033;    double YucdNgWQPrNmsMFRS78517753 = -665202870;    double YucdNgWQPrNmsMFRS20322401 = -907793688;    double YucdNgWQPrNmsMFRS25335375 = 56878156;    double YucdNgWQPrNmsMFRS90306679 = -127874674;    double YucdNgWQPrNmsMFRS84052182 = -402337188;    double YucdNgWQPrNmsMFRS69495305 = -836945134;    double YucdNgWQPrNmsMFRS15339012 = -721236581;    double YucdNgWQPrNmsMFRS41693921 = -784298080;    double YucdNgWQPrNmsMFRS11192068 = -681004655;    double YucdNgWQPrNmsMFRS67846647 = -657829285;    double YucdNgWQPrNmsMFRS86499836 = -156500511;    double YucdNgWQPrNmsMFRS98960391 = -360397855;    double YucdNgWQPrNmsMFRS85766385 = -942685650;    double YucdNgWQPrNmsMFRS86823009 = -392797605;     YucdNgWQPrNmsMFRS5409577 = YucdNgWQPrNmsMFRS29266492;     YucdNgWQPrNmsMFRS29266492 = YucdNgWQPrNmsMFRS37862068;     YucdNgWQPrNmsMFRS37862068 = YucdNgWQPrNmsMFRS73555663;     YucdNgWQPrNmsMFRS73555663 = YucdNgWQPrNmsMFRS71451526;     YucdNgWQPrNmsMFRS71451526 = YucdNgWQPrNmsMFRS86336486;     YucdNgWQPrNmsMFRS86336486 = YucdNgWQPrNmsMFRS38161865;     YucdNgWQPrNmsMFRS38161865 = YucdNgWQPrNmsMFRS32758025;     YucdNgWQPrNmsMFRS32758025 = YucdNgWQPrNmsMFRS85047217;     YucdNgWQPrNmsMFRS85047217 = YucdNgWQPrNmsMFRS6987584;     YucdNgWQPrNmsMFRS6987584 = YucdNgWQPrNmsMFRS50759537;     YucdNgWQPrNmsMFRS50759537 = YucdNgWQPrNmsMFRS53896165;     YucdNgWQPrNmsMFRS53896165 = YucdNgWQPrNmsMFRS60021097;     YucdNgWQPrNmsMFRS60021097 = YucdNgWQPrNmsMFRS24311835;     YucdNgWQPrNmsMFRS24311835 = YucdNgWQPrNmsMFRS3045112;     YucdNgWQPrNmsMFRS3045112 = YucdNgWQPrNmsMFRS1993449;     YucdNgWQPrNmsMFRS1993449 = YucdNgWQPrNmsMFRS94463949;     YucdNgWQPrNmsMFRS94463949 = YucdNgWQPrNmsMFRS35168331;     YucdNgWQPrNmsMFRS35168331 = YucdNgWQPrNmsMFRS25067097;     YucdNgWQPrNmsMFRS25067097 = YucdNgWQPrNmsMFRS4291148;     YucdNgWQPrNmsMFRS4291148 = YucdNgWQPrNmsMFRS12763220;     YucdNgWQPrNmsMFRS12763220 = YucdNgWQPrNmsMFRS12029753;     YucdNgWQPrNmsMFRS12029753 = YucdNgWQPrNmsMFRS26539052;     YucdNgWQPrNmsMFRS26539052 = YucdNgWQPrNmsMFRS35066615;     YucdNgWQPrNmsMFRS35066615 = YucdNgWQPrNmsMFRS65207174;     YucdNgWQPrNmsMFRS65207174 = YucdNgWQPrNmsMFRS96381046;     YucdNgWQPrNmsMFRS96381046 = YucdNgWQPrNmsMFRS65243627;     YucdNgWQPrNmsMFRS65243627 = YucdNgWQPrNmsMFRS11381808;     YucdNgWQPrNmsMFRS11381808 = YucdNgWQPrNmsMFRS95433592;     YucdNgWQPrNmsMFRS95433592 = YucdNgWQPrNmsMFRS61064093;     YucdNgWQPrNmsMFRS61064093 = YucdNgWQPrNmsMFRS82491973;     YucdNgWQPrNmsMFRS82491973 = YucdNgWQPrNmsMFRS13012965;     YucdNgWQPrNmsMFRS13012965 = YucdNgWQPrNmsMFRS81984117;     YucdNgWQPrNmsMFRS81984117 = YucdNgWQPrNmsMFRS25556374;     YucdNgWQPrNmsMFRS25556374 = YucdNgWQPrNmsMFRS83686502;     YucdNgWQPrNmsMFRS83686502 = YucdNgWQPrNmsMFRS86779622;     YucdNgWQPrNmsMFRS86779622 = YucdNgWQPrNmsMFRS72290758;     YucdNgWQPrNmsMFRS72290758 = YucdNgWQPrNmsMFRS23687501;     YucdNgWQPrNmsMFRS23687501 = YucdNgWQPrNmsMFRS16764401;     YucdNgWQPrNmsMFRS16764401 = YucdNgWQPrNmsMFRS32284928;     YucdNgWQPrNmsMFRS32284928 = YucdNgWQPrNmsMFRS94626704;     YucdNgWQPrNmsMFRS94626704 = YucdNgWQPrNmsMFRS23824268;     YucdNgWQPrNmsMFRS23824268 = YucdNgWQPrNmsMFRS22943516;     YucdNgWQPrNmsMFRS22943516 = YucdNgWQPrNmsMFRS10261997;     YucdNgWQPrNmsMFRS10261997 = YucdNgWQPrNmsMFRS24569583;     YucdNgWQPrNmsMFRS24569583 = YucdNgWQPrNmsMFRS27112266;     YucdNgWQPrNmsMFRS27112266 = YucdNgWQPrNmsMFRS40035620;     YucdNgWQPrNmsMFRS40035620 = YucdNgWQPrNmsMFRS60663258;     YucdNgWQPrNmsMFRS60663258 = YucdNgWQPrNmsMFRS83594654;     YucdNgWQPrNmsMFRS83594654 = YucdNgWQPrNmsMFRS65145698;     YucdNgWQPrNmsMFRS65145698 = YucdNgWQPrNmsMFRS76351617;     YucdNgWQPrNmsMFRS76351617 = YucdNgWQPrNmsMFRS21634471;     YucdNgWQPrNmsMFRS21634471 = YucdNgWQPrNmsMFRS34495211;     YucdNgWQPrNmsMFRS34495211 = YucdNgWQPrNmsMFRS61661717;     YucdNgWQPrNmsMFRS61661717 = YucdNgWQPrNmsMFRS221000;     YucdNgWQPrNmsMFRS221000 = YucdNgWQPrNmsMFRS93379823;     YucdNgWQPrNmsMFRS93379823 = YucdNgWQPrNmsMFRS2727440;     YucdNgWQPrNmsMFRS2727440 = YucdNgWQPrNmsMFRS2795453;     YucdNgWQPrNmsMFRS2795453 = YucdNgWQPrNmsMFRS8348489;     YucdNgWQPrNmsMFRS8348489 = YucdNgWQPrNmsMFRS75070479;     YucdNgWQPrNmsMFRS75070479 = YucdNgWQPrNmsMFRS21092860;     YucdNgWQPrNmsMFRS21092860 = YucdNgWQPrNmsMFRS26780057;     YucdNgWQPrNmsMFRS26780057 = YucdNgWQPrNmsMFRS37324432;     YucdNgWQPrNmsMFRS37324432 = YucdNgWQPrNmsMFRS23983124;     YucdNgWQPrNmsMFRS23983124 = YucdNgWQPrNmsMFRS24495611;     YucdNgWQPrNmsMFRS24495611 = YucdNgWQPrNmsMFRS37746573;     YucdNgWQPrNmsMFRS37746573 = YucdNgWQPrNmsMFRS71912048;     YucdNgWQPrNmsMFRS71912048 = YucdNgWQPrNmsMFRS34464723;     YucdNgWQPrNmsMFRS34464723 = YucdNgWQPrNmsMFRS40625332;     YucdNgWQPrNmsMFRS40625332 = YucdNgWQPrNmsMFRS16265489;     YucdNgWQPrNmsMFRS16265489 = YucdNgWQPrNmsMFRS29702691;     YucdNgWQPrNmsMFRS29702691 = YucdNgWQPrNmsMFRS70776449;     YucdNgWQPrNmsMFRS70776449 = YucdNgWQPrNmsMFRS18403930;     YucdNgWQPrNmsMFRS18403930 = YucdNgWQPrNmsMFRS92782169;     YucdNgWQPrNmsMFRS92782169 = YucdNgWQPrNmsMFRS9664444;     YucdNgWQPrNmsMFRS9664444 = YucdNgWQPrNmsMFRS88938952;     YucdNgWQPrNmsMFRS88938952 = YucdNgWQPrNmsMFRS89086237;     YucdNgWQPrNmsMFRS89086237 = YucdNgWQPrNmsMFRS16277055;     YucdNgWQPrNmsMFRS16277055 = YucdNgWQPrNmsMFRS10497033;     YucdNgWQPrNmsMFRS10497033 = YucdNgWQPrNmsMFRS38094909;     YucdNgWQPrNmsMFRS38094909 = YucdNgWQPrNmsMFRS56345427;     YucdNgWQPrNmsMFRS56345427 = YucdNgWQPrNmsMFRS4580369;     YucdNgWQPrNmsMFRS4580369 = YucdNgWQPrNmsMFRS27787154;     YucdNgWQPrNmsMFRS27787154 = YucdNgWQPrNmsMFRS30287895;     YucdNgWQPrNmsMFRS30287895 = YucdNgWQPrNmsMFRS84712475;     YucdNgWQPrNmsMFRS84712475 = YucdNgWQPrNmsMFRS60857502;     YucdNgWQPrNmsMFRS60857502 = YucdNgWQPrNmsMFRS78517753;     YucdNgWQPrNmsMFRS78517753 = YucdNgWQPrNmsMFRS20322401;     YucdNgWQPrNmsMFRS20322401 = YucdNgWQPrNmsMFRS25335375;     YucdNgWQPrNmsMFRS25335375 = YucdNgWQPrNmsMFRS90306679;     YucdNgWQPrNmsMFRS90306679 = YucdNgWQPrNmsMFRS84052182;     YucdNgWQPrNmsMFRS84052182 = YucdNgWQPrNmsMFRS69495305;     YucdNgWQPrNmsMFRS69495305 = YucdNgWQPrNmsMFRS15339012;     YucdNgWQPrNmsMFRS15339012 = YucdNgWQPrNmsMFRS41693921;     YucdNgWQPrNmsMFRS41693921 = YucdNgWQPrNmsMFRS11192068;     YucdNgWQPrNmsMFRS11192068 = YucdNgWQPrNmsMFRS67846647;     YucdNgWQPrNmsMFRS67846647 = YucdNgWQPrNmsMFRS86499836;     YucdNgWQPrNmsMFRS86499836 = YucdNgWQPrNmsMFRS98960391;     YucdNgWQPrNmsMFRS98960391 = YucdNgWQPrNmsMFRS85766385;     YucdNgWQPrNmsMFRS85766385 = YucdNgWQPrNmsMFRS86823009;     YucdNgWQPrNmsMFRS86823009 = YucdNgWQPrNmsMFRS5409577;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void drEcMbOYjuPUFDID28033287() {     double tIXSUdqNHPYYBrHyP11526684 = -504699494;    double tIXSUdqNHPYYBrHyP72639771 = -648319921;    double tIXSUdqNHPYYBrHyP53904251 = -930415735;    double tIXSUdqNHPYYBrHyP630444 = -450127811;    double tIXSUdqNHPYYBrHyP40347015 = -30776792;    double tIXSUdqNHPYYBrHyP77753689 = -103169386;    double tIXSUdqNHPYYBrHyP98650830 = -3429026;    double tIXSUdqNHPYYBrHyP82517915 = -828030456;    double tIXSUdqNHPYYBrHyP49689712 = -196062658;    double tIXSUdqNHPYYBrHyP48217243 = -104872959;    double tIXSUdqNHPYYBrHyP10869828 = -830208135;    double tIXSUdqNHPYYBrHyP85719207 = -493451970;    double tIXSUdqNHPYYBrHyP46230215 = -424608471;    double tIXSUdqNHPYYBrHyP7455512 = -759449189;    double tIXSUdqNHPYYBrHyP98787537 = -384486592;    double tIXSUdqNHPYYBrHyP43498843 = -791362141;    double tIXSUdqNHPYYBrHyP88299952 = -527623178;    double tIXSUdqNHPYYBrHyP32221484 = -588120741;    double tIXSUdqNHPYYBrHyP86896938 = -973791965;    double tIXSUdqNHPYYBrHyP84841312 = -656883726;    double tIXSUdqNHPYYBrHyP1316414 = -203970446;    double tIXSUdqNHPYYBrHyP98550776 = -340443596;    double tIXSUdqNHPYYBrHyP62129843 = -646094654;    double tIXSUdqNHPYYBrHyP19058989 = -668799804;    double tIXSUdqNHPYYBrHyP8906825 = 81150336;    double tIXSUdqNHPYYBrHyP21102722 = -216374088;    double tIXSUdqNHPYYBrHyP63703405 = -290850393;    double tIXSUdqNHPYYBrHyP24488934 = -222118861;    double tIXSUdqNHPYYBrHyP61565848 = -324891588;    double tIXSUdqNHPYYBrHyP29679730 = -357475110;    double tIXSUdqNHPYYBrHyP74734522 = -886490670;    double tIXSUdqNHPYYBrHyP10623587 = -818062513;    double tIXSUdqNHPYYBrHyP91695065 = -556996009;    double tIXSUdqNHPYYBrHyP20386938 = -491127512;    double tIXSUdqNHPYYBrHyP45161155 = -726891505;    double tIXSUdqNHPYYBrHyP55505535 = -655506269;    double tIXSUdqNHPYYBrHyP53552693 = -23669001;    double tIXSUdqNHPYYBrHyP57777713 = -106468140;    double tIXSUdqNHPYYBrHyP90374654 = 49370267;    double tIXSUdqNHPYYBrHyP19786866 = -602928839;    double tIXSUdqNHPYYBrHyP295928 = -431377321;    double tIXSUdqNHPYYBrHyP55974725 = -865290448;    double tIXSUdqNHPYYBrHyP1010494 = -329228188;    double tIXSUdqNHPYYBrHyP97669737 = -864756337;    double tIXSUdqNHPYYBrHyP36522047 = -815754137;    double tIXSUdqNHPYYBrHyP39051461 = 88403658;    double tIXSUdqNHPYYBrHyP57814194 = -269609795;    double tIXSUdqNHPYYBrHyP68620199 = -513955784;    double tIXSUdqNHPYYBrHyP93270383 = -833079430;    double tIXSUdqNHPYYBrHyP78057350 = -45535465;    double tIXSUdqNHPYYBrHyP12052296 = -84902488;    double tIXSUdqNHPYYBrHyP32444190 = 73369980;    double tIXSUdqNHPYYBrHyP82743668 = -135202415;    double tIXSUdqNHPYYBrHyP57630597 = 22940904;    double tIXSUdqNHPYYBrHyP26716874 = -296995686;    double tIXSUdqNHPYYBrHyP12975907 = -64255899;    double tIXSUdqNHPYYBrHyP10509928 = 97774732;    double tIXSUdqNHPYYBrHyP34845263 = -161615932;    double tIXSUdqNHPYYBrHyP91723619 = -431278147;    double tIXSUdqNHPYYBrHyP19244293 = -814402705;    double tIXSUdqNHPYYBrHyP14050284 = -812318993;    double tIXSUdqNHPYYBrHyP74161897 = -781310166;    double tIXSUdqNHPYYBrHyP20952067 = -403138868;    double tIXSUdqNHPYYBrHyP20009983 = -838587549;    double tIXSUdqNHPYYBrHyP73482720 = -218382289;    double tIXSUdqNHPYYBrHyP246242 = 87854377;    double tIXSUdqNHPYYBrHyP94024141 = -936455961;    double tIXSUdqNHPYYBrHyP25843278 = -933480959;    double tIXSUdqNHPYYBrHyP62294357 = 67442316;    double tIXSUdqNHPYYBrHyP43282002 = -728980323;    double tIXSUdqNHPYYBrHyP89946149 = -667693140;    double tIXSUdqNHPYYBrHyP30522240 = -321155039;    double tIXSUdqNHPYYBrHyP41846829 = -537491008;    double tIXSUdqNHPYYBrHyP67110073 = -270863127;    double tIXSUdqNHPYYBrHyP84545384 = -125506406;    double tIXSUdqNHPYYBrHyP45341688 = -338679998;    double tIXSUdqNHPYYBrHyP97540282 = 88784592;    double tIXSUdqNHPYYBrHyP64460106 = -781338318;    double tIXSUdqNHPYYBrHyP82536941 = -853045668;    double tIXSUdqNHPYYBrHyP69855364 = 92746677;    double tIXSUdqNHPYYBrHyP63288528 = -946764293;    double tIXSUdqNHPYYBrHyP95083205 = -776894610;    double tIXSUdqNHPYYBrHyP31218550 = -389039431;    double tIXSUdqNHPYYBrHyP83508497 = -179356123;    double tIXSUdqNHPYYBrHyP17627435 = -172572622;    double tIXSUdqNHPYYBrHyP42290332 = -859860651;    double tIXSUdqNHPYYBrHyP27879918 = -582860098;    double tIXSUdqNHPYYBrHyP34064469 = -479936913;    double tIXSUdqNHPYYBrHyP93670064 = -94131826;    double tIXSUdqNHPYYBrHyP32185248 = -562635607;    double tIXSUdqNHPYYBrHyP44995608 = -653281002;    double tIXSUdqNHPYYBrHyP18707431 = -862053070;    double tIXSUdqNHPYYBrHyP66054093 = -675189993;    double tIXSUdqNHPYYBrHyP71130362 = -136227029;    double tIXSUdqNHPYYBrHyP5736582 = -790609846;    double tIXSUdqNHPYYBrHyP26134031 = -650067155;    double tIXSUdqNHPYYBrHyP35022658 = -362151581;    double tIXSUdqNHPYYBrHyP81000511 = -490640640;    double tIXSUdqNHPYYBrHyP24187017 = -546374048;    double tIXSUdqNHPYYBrHyP36275806 = -504699494;     tIXSUdqNHPYYBrHyP11526684 = tIXSUdqNHPYYBrHyP72639771;     tIXSUdqNHPYYBrHyP72639771 = tIXSUdqNHPYYBrHyP53904251;     tIXSUdqNHPYYBrHyP53904251 = tIXSUdqNHPYYBrHyP630444;     tIXSUdqNHPYYBrHyP630444 = tIXSUdqNHPYYBrHyP40347015;     tIXSUdqNHPYYBrHyP40347015 = tIXSUdqNHPYYBrHyP77753689;     tIXSUdqNHPYYBrHyP77753689 = tIXSUdqNHPYYBrHyP98650830;     tIXSUdqNHPYYBrHyP98650830 = tIXSUdqNHPYYBrHyP82517915;     tIXSUdqNHPYYBrHyP82517915 = tIXSUdqNHPYYBrHyP49689712;     tIXSUdqNHPYYBrHyP49689712 = tIXSUdqNHPYYBrHyP48217243;     tIXSUdqNHPYYBrHyP48217243 = tIXSUdqNHPYYBrHyP10869828;     tIXSUdqNHPYYBrHyP10869828 = tIXSUdqNHPYYBrHyP85719207;     tIXSUdqNHPYYBrHyP85719207 = tIXSUdqNHPYYBrHyP46230215;     tIXSUdqNHPYYBrHyP46230215 = tIXSUdqNHPYYBrHyP7455512;     tIXSUdqNHPYYBrHyP7455512 = tIXSUdqNHPYYBrHyP98787537;     tIXSUdqNHPYYBrHyP98787537 = tIXSUdqNHPYYBrHyP43498843;     tIXSUdqNHPYYBrHyP43498843 = tIXSUdqNHPYYBrHyP88299952;     tIXSUdqNHPYYBrHyP88299952 = tIXSUdqNHPYYBrHyP32221484;     tIXSUdqNHPYYBrHyP32221484 = tIXSUdqNHPYYBrHyP86896938;     tIXSUdqNHPYYBrHyP86896938 = tIXSUdqNHPYYBrHyP84841312;     tIXSUdqNHPYYBrHyP84841312 = tIXSUdqNHPYYBrHyP1316414;     tIXSUdqNHPYYBrHyP1316414 = tIXSUdqNHPYYBrHyP98550776;     tIXSUdqNHPYYBrHyP98550776 = tIXSUdqNHPYYBrHyP62129843;     tIXSUdqNHPYYBrHyP62129843 = tIXSUdqNHPYYBrHyP19058989;     tIXSUdqNHPYYBrHyP19058989 = tIXSUdqNHPYYBrHyP8906825;     tIXSUdqNHPYYBrHyP8906825 = tIXSUdqNHPYYBrHyP21102722;     tIXSUdqNHPYYBrHyP21102722 = tIXSUdqNHPYYBrHyP63703405;     tIXSUdqNHPYYBrHyP63703405 = tIXSUdqNHPYYBrHyP24488934;     tIXSUdqNHPYYBrHyP24488934 = tIXSUdqNHPYYBrHyP61565848;     tIXSUdqNHPYYBrHyP61565848 = tIXSUdqNHPYYBrHyP29679730;     tIXSUdqNHPYYBrHyP29679730 = tIXSUdqNHPYYBrHyP74734522;     tIXSUdqNHPYYBrHyP74734522 = tIXSUdqNHPYYBrHyP10623587;     tIXSUdqNHPYYBrHyP10623587 = tIXSUdqNHPYYBrHyP91695065;     tIXSUdqNHPYYBrHyP91695065 = tIXSUdqNHPYYBrHyP20386938;     tIXSUdqNHPYYBrHyP20386938 = tIXSUdqNHPYYBrHyP45161155;     tIXSUdqNHPYYBrHyP45161155 = tIXSUdqNHPYYBrHyP55505535;     tIXSUdqNHPYYBrHyP55505535 = tIXSUdqNHPYYBrHyP53552693;     tIXSUdqNHPYYBrHyP53552693 = tIXSUdqNHPYYBrHyP57777713;     tIXSUdqNHPYYBrHyP57777713 = tIXSUdqNHPYYBrHyP90374654;     tIXSUdqNHPYYBrHyP90374654 = tIXSUdqNHPYYBrHyP19786866;     tIXSUdqNHPYYBrHyP19786866 = tIXSUdqNHPYYBrHyP295928;     tIXSUdqNHPYYBrHyP295928 = tIXSUdqNHPYYBrHyP55974725;     tIXSUdqNHPYYBrHyP55974725 = tIXSUdqNHPYYBrHyP1010494;     tIXSUdqNHPYYBrHyP1010494 = tIXSUdqNHPYYBrHyP97669737;     tIXSUdqNHPYYBrHyP97669737 = tIXSUdqNHPYYBrHyP36522047;     tIXSUdqNHPYYBrHyP36522047 = tIXSUdqNHPYYBrHyP39051461;     tIXSUdqNHPYYBrHyP39051461 = tIXSUdqNHPYYBrHyP57814194;     tIXSUdqNHPYYBrHyP57814194 = tIXSUdqNHPYYBrHyP68620199;     tIXSUdqNHPYYBrHyP68620199 = tIXSUdqNHPYYBrHyP93270383;     tIXSUdqNHPYYBrHyP93270383 = tIXSUdqNHPYYBrHyP78057350;     tIXSUdqNHPYYBrHyP78057350 = tIXSUdqNHPYYBrHyP12052296;     tIXSUdqNHPYYBrHyP12052296 = tIXSUdqNHPYYBrHyP32444190;     tIXSUdqNHPYYBrHyP32444190 = tIXSUdqNHPYYBrHyP82743668;     tIXSUdqNHPYYBrHyP82743668 = tIXSUdqNHPYYBrHyP57630597;     tIXSUdqNHPYYBrHyP57630597 = tIXSUdqNHPYYBrHyP26716874;     tIXSUdqNHPYYBrHyP26716874 = tIXSUdqNHPYYBrHyP12975907;     tIXSUdqNHPYYBrHyP12975907 = tIXSUdqNHPYYBrHyP10509928;     tIXSUdqNHPYYBrHyP10509928 = tIXSUdqNHPYYBrHyP34845263;     tIXSUdqNHPYYBrHyP34845263 = tIXSUdqNHPYYBrHyP91723619;     tIXSUdqNHPYYBrHyP91723619 = tIXSUdqNHPYYBrHyP19244293;     tIXSUdqNHPYYBrHyP19244293 = tIXSUdqNHPYYBrHyP14050284;     tIXSUdqNHPYYBrHyP14050284 = tIXSUdqNHPYYBrHyP74161897;     tIXSUdqNHPYYBrHyP74161897 = tIXSUdqNHPYYBrHyP20952067;     tIXSUdqNHPYYBrHyP20952067 = tIXSUdqNHPYYBrHyP20009983;     tIXSUdqNHPYYBrHyP20009983 = tIXSUdqNHPYYBrHyP73482720;     tIXSUdqNHPYYBrHyP73482720 = tIXSUdqNHPYYBrHyP246242;     tIXSUdqNHPYYBrHyP246242 = tIXSUdqNHPYYBrHyP94024141;     tIXSUdqNHPYYBrHyP94024141 = tIXSUdqNHPYYBrHyP25843278;     tIXSUdqNHPYYBrHyP25843278 = tIXSUdqNHPYYBrHyP62294357;     tIXSUdqNHPYYBrHyP62294357 = tIXSUdqNHPYYBrHyP43282002;     tIXSUdqNHPYYBrHyP43282002 = tIXSUdqNHPYYBrHyP89946149;     tIXSUdqNHPYYBrHyP89946149 = tIXSUdqNHPYYBrHyP30522240;     tIXSUdqNHPYYBrHyP30522240 = tIXSUdqNHPYYBrHyP41846829;     tIXSUdqNHPYYBrHyP41846829 = tIXSUdqNHPYYBrHyP67110073;     tIXSUdqNHPYYBrHyP67110073 = tIXSUdqNHPYYBrHyP84545384;     tIXSUdqNHPYYBrHyP84545384 = tIXSUdqNHPYYBrHyP45341688;     tIXSUdqNHPYYBrHyP45341688 = tIXSUdqNHPYYBrHyP97540282;     tIXSUdqNHPYYBrHyP97540282 = tIXSUdqNHPYYBrHyP64460106;     tIXSUdqNHPYYBrHyP64460106 = tIXSUdqNHPYYBrHyP82536941;     tIXSUdqNHPYYBrHyP82536941 = tIXSUdqNHPYYBrHyP69855364;     tIXSUdqNHPYYBrHyP69855364 = tIXSUdqNHPYYBrHyP63288528;     tIXSUdqNHPYYBrHyP63288528 = tIXSUdqNHPYYBrHyP95083205;     tIXSUdqNHPYYBrHyP95083205 = tIXSUdqNHPYYBrHyP31218550;     tIXSUdqNHPYYBrHyP31218550 = tIXSUdqNHPYYBrHyP83508497;     tIXSUdqNHPYYBrHyP83508497 = tIXSUdqNHPYYBrHyP17627435;     tIXSUdqNHPYYBrHyP17627435 = tIXSUdqNHPYYBrHyP42290332;     tIXSUdqNHPYYBrHyP42290332 = tIXSUdqNHPYYBrHyP27879918;     tIXSUdqNHPYYBrHyP27879918 = tIXSUdqNHPYYBrHyP34064469;     tIXSUdqNHPYYBrHyP34064469 = tIXSUdqNHPYYBrHyP93670064;     tIXSUdqNHPYYBrHyP93670064 = tIXSUdqNHPYYBrHyP32185248;     tIXSUdqNHPYYBrHyP32185248 = tIXSUdqNHPYYBrHyP44995608;     tIXSUdqNHPYYBrHyP44995608 = tIXSUdqNHPYYBrHyP18707431;     tIXSUdqNHPYYBrHyP18707431 = tIXSUdqNHPYYBrHyP66054093;     tIXSUdqNHPYYBrHyP66054093 = tIXSUdqNHPYYBrHyP71130362;     tIXSUdqNHPYYBrHyP71130362 = tIXSUdqNHPYYBrHyP5736582;     tIXSUdqNHPYYBrHyP5736582 = tIXSUdqNHPYYBrHyP26134031;     tIXSUdqNHPYYBrHyP26134031 = tIXSUdqNHPYYBrHyP35022658;     tIXSUdqNHPYYBrHyP35022658 = tIXSUdqNHPYYBrHyP81000511;     tIXSUdqNHPYYBrHyP81000511 = tIXSUdqNHPYYBrHyP24187017;     tIXSUdqNHPYYBrHyP24187017 = tIXSUdqNHPYYBrHyP36275806;     tIXSUdqNHPYYBrHyP36275806 = tIXSUdqNHPYYBrHyP11526684;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uceQYLmyuibfOpnI95324886() {     double UvZXDTBIrYqcbcpdr46985652 = -970976394;    double UvZXDTBIrYqcbcpdr85911849 = -528871528;    double UvZXDTBIrYqcbcpdr91290728 = -596082810;    double UvZXDTBIrYqcbcpdr72853798 = -271278882;    double UvZXDTBIrYqcbcpdr96632283 = -136375637;    double UvZXDTBIrYqcbcpdr13976561 = -365517281;    double UvZXDTBIrYqcbcpdr9308238 = -948154761;    double UvZXDTBIrYqcbcpdr44649476 = -472264596;    double UvZXDTBIrYqcbcpdr51207318 = -684394065;    double UvZXDTBIrYqcbcpdr63119955 = -869559957;    double UvZXDTBIrYqcbcpdr49051562 = -632215864;    double UvZXDTBIrYqcbcpdr86287356 = -357572741;    double UvZXDTBIrYqcbcpdr84464861 = -976620976;    double UvZXDTBIrYqcbcpdr60596375 = -735812598;    double UvZXDTBIrYqcbcpdr56982054 = -887911607;    double UvZXDTBIrYqcbcpdr27040144 = -427173748;    double UvZXDTBIrYqcbcpdr44135888 = 45402269;    double UvZXDTBIrYqcbcpdr31981675 = -661548809;    double UvZXDTBIrYqcbcpdr87447268 = -190355387;    double UvZXDTBIrYqcbcpdr78678530 = 16555208;    double UvZXDTBIrYqcbcpdr14403277 = -961245134;    double UvZXDTBIrYqcbcpdr1972302 = -427079890;    double UvZXDTBIrYqcbcpdr32655223 = -283726839;    double UvZXDTBIrYqcbcpdr12851935 = -79624408;    double UvZXDTBIrYqcbcpdr16924776 = 57165138;    double UvZXDTBIrYqcbcpdr31800894 = 95049105;    double UvZXDTBIrYqcbcpdr24123979 = -97134534;    double UvZXDTBIrYqcbcpdr65217358 = -391891035;    double UvZXDTBIrYqcbcpdr8234063 = -898986391;    double UvZXDTBIrYqcbcpdr35213141 = -391982280;    double UvZXDTBIrYqcbcpdr93317754 = -338471157;    double UvZXDTBIrYqcbcpdr48446605 = -591033927;    double UvZXDTBIrYqcbcpdr3913077 = -704591986;    double UvZXDTBIrYqcbcpdr50135365 = -555821325;    double UvZXDTBIrYqcbcpdr10969609 = -677991756;    double UvZXDTBIrYqcbcpdr7066380 = -870071575;    double UvZXDTBIrYqcbcpdr2964510 = -988365042;    double UvZXDTBIrYqcbcpdr8993117 = -89123721;    double UvZXDTBIrYqcbcpdr20765216 = -701506010;    double UvZXDTBIrYqcbcpdr84058397 = -68191385;    double UvZXDTBIrYqcbcpdr33747536 = -200576235;    double UvZXDTBIrYqcbcpdr45339852 = -459835739;    double UvZXDTBIrYqcbcpdr90393872 = -982408974;    double UvZXDTBIrYqcbcpdr37801075 = -369088747;    double UvZXDTBIrYqcbcpdr36538506 = -642265298;    double UvZXDTBIrYqcbcpdr85012789 = 49480026;    double UvZXDTBIrYqcbcpdr77159357 = -753596156;    double UvZXDTBIrYqcbcpdr86346511 = -680482378;    double UvZXDTBIrYqcbcpdr16163050 = -496816311;    double UvZXDTBIrYqcbcpdr2586940 = -536562281;    double UvZXDTBIrYqcbcpdr92823019 = 40820954;    double UvZXDTBIrYqcbcpdr15929663 = -140282367;    double UvZXDTBIrYqcbcpdr51364817 = -991247969;    double UvZXDTBIrYqcbcpdr73622443 = -168567596;    double UvZXDTBIrYqcbcpdr80921610 = -643485832;    double UvZXDTBIrYqcbcpdr45013351 = -443896505;    double UvZXDTBIrYqcbcpdr53256626 = -145144690;    double UvZXDTBIrYqcbcpdr78438794 = -416458402;    double UvZXDTBIrYqcbcpdr55929023 = -228444020;    double UvZXDTBIrYqcbcpdr64831390 = -131424743;    double UvZXDTBIrYqcbcpdr89852582 = -168382748;    double UvZXDTBIrYqcbcpdr44090879 = -456263726;    double UvZXDTBIrYqcbcpdr36415413 = -573278206;    double UvZXDTBIrYqcbcpdr15994178 = -192411785;    double UvZXDTBIrYqcbcpdr69802201 = -431088800;    double UvZXDTBIrYqcbcpdr604958 = 58818062;    double UvZXDTBIrYqcbcpdr82374279 = -652980755;    double UvZXDTBIrYqcbcpdr34329496 = -320799651;    double UvZXDTBIrYqcbcpdr49626766 = 42179157;    double UvZXDTBIrYqcbcpdr49915675 = 82159967;    double UvZXDTBIrYqcbcpdr24075634 = -438808707;    double UvZXDTBIrYqcbcpdr35142772 = -865474010;    double UvZXDTBIrYqcbcpdr11216459 = -960042800;    double UvZXDTBIrYqcbcpdr3388872 = -22164002;    double UvZXDTBIrYqcbcpdr44930995 = -782868557;    double UvZXDTBIrYqcbcpdr69063424 = -401409396;    double UvZXDTBIrYqcbcpdr11578429 = -444670916;    double UvZXDTBIrYqcbcpdr94854148 = -914638092;    double UvZXDTBIrYqcbcpdr76313428 = -437359111;    double UvZXDTBIrYqcbcpdr31911987 = -992314889;    double UvZXDTBIrYqcbcpdr54641536 = -151354739;    double UvZXDTBIrYqcbcpdr37777468 = -416652157;    double UvZXDTBIrYqcbcpdr49054309 = -895074725;    double UvZXDTBIrYqcbcpdr5647124 = -262424110;    double UvZXDTBIrYqcbcpdr42390121 = -332803235;    double UvZXDTBIrYqcbcpdr77388091 = -98188791;    double UvZXDTBIrYqcbcpdr97081787 = -599785958;    double UvZXDTBIrYqcbcpdr30290633 = -436024391;    double UvZXDTBIrYqcbcpdr69213754 = -912335494;    double UvZXDTBIrYqcbcpdr65956258 = -134095252;    double UvZXDTBIrYqcbcpdr53809753 = -624926885;    double UvZXDTBIrYqcbcpdr24525716 = -471906640;    double UvZXDTBIrYqcbcpdr53064093 = -860679701;    double UvZXDTBIrYqcbcpdr55933826 = -470081267;    double UvZXDTBIrYqcbcpdr94205815 = -899808638;    double UvZXDTBIrYqcbcpdr89656656 = -744312510;    double UvZXDTBIrYqcbcpdr8924439 = -886557533;    double UvZXDTBIrYqcbcpdr74399695 = -689997190;    double UvZXDTBIrYqcbcpdr67998874 = -937999947;    double UvZXDTBIrYqcbcpdr35933549 = -970976394;     UvZXDTBIrYqcbcpdr46985652 = UvZXDTBIrYqcbcpdr85911849;     UvZXDTBIrYqcbcpdr85911849 = UvZXDTBIrYqcbcpdr91290728;     UvZXDTBIrYqcbcpdr91290728 = UvZXDTBIrYqcbcpdr72853798;     UvZXDTBIrYqcbcpdr72853798 = UvZXDTBIrYqcbcpdr96632283;     UvZXDTBIrYqcbcpdr96632283 = UvZXDTBIrYqcbcpdr13976561;     UvZXDTBIrYqcbcpdr13976561 = UvZXDTBIrYqcbcpdr9308238;     UvZXDTBIrYqcbcpdr9308238 = UvZXDTBIrYqcbcpdr44649476;     UvZXDTBIrYqcbcpdr44649476 = UvZXDTBIrYqcbcpdr51207318;     UvZXDTBIrYqcbcpdr51207318 = UvZXDTBIrYqcbcpdr63119955;     UvZXDTBIrYqcbcpdr63119955 = UvZXDTBIrYqcbcpdr49051562;     UvZXDTBIrYqcbcpdr49051562 = UvZXDTBIrYqcbcpdr86287356;     UvZXDTBIrYqcbcpdr86287356 = UvZXDTBIrYqcbcpdr84464861;     UvZXDTBIrYqcbcpdr84464861 = UvZXDTBIrYqcbcpdr60596375;     UvZXDTBIrYqcbcpdr60596375 = UvZXDTBIrYqcbcpdr56982054;     UvZXDTBIrYqcbcpdr56982054 = UvZXDTBIrYqcbcpdr27040144;     UvZXDTBIrYqcbcpdr27040144 = UvZXDTBIrYqcbcpdr44135888;     UvZXDTBIrYqcbcpdr44135888 = UvZXDTBIrYqcbcpdr31981675;     UvZXDTBIrYqcbcpdr31981675 = UvZXDTBIrYqcbcpdr87447268;     UvZXDTBIrYqcbcpdr87447268 = UvZXDTBIrYqcbcpdr78678530;     UvZXDTBIrYqcbcpdr78678530 = UvZXDTBIrYqcbcpdr14403277;     UvZXDTBIrYqcbcpdr14403277 = UvZXDTBIrYqcbcpdr1972302;     UvZXDTBIrYqcbcpdr1972302 = UvZXDTBIrYqcbcpdr32655223;     UvZXDTBIrYqcbcpdr32655223 = UvZXDTBIrYqcbcpdr12851935;     UvZXDTBIrYqcbcpdr12851935 = UvZXDTBIrYqcbcpdr16924776;     UvZXDTBIrYqcbcpdr16924776 = UvZXDTBIrYqcbcpdr31800894;     UvZXDTBIrYqcbcpdr31800894 = UvZXDTBIrYqcbcpdr24123979;     UvZXDTBIrYqcbcpdr24123979 = UvZXDTBIrYqcbcpdr65217358;     UvZXDTBIrYqcbcpdr65217358 = UvZXDTBIrYqcbcpdr8234063;     UvZXDTBIrYqcbcpdr8234063 = UvZXDTBIrYqcbcpdr35213141;     UvZXDTBIrYqcbcpdr35213141 = UvZXDTBIrYqcbcpdr93317754;     UvZXDTBIrYqcbcpdr93317754 = UvZXDTBIrYqcbcpdr48446605;     UvZXDTBIrYqcbcpdr48446605 = UvZXDTBIrYqcbcpdr3913077;     UvZXDTBIrYqcbcpdr3913077 = UvZXDTBIrYqcbcpdr50135365;     UvZXDTBIrYqcbcpdr50135365 = UvZXDTBIrYqcbcpdr10969609;     UvZXDTBIrYqcbcpdr10969609 = UvZXDTBIrYqcbcpdr7066380;     UvZXDTBIrYqcbcpdr7066380 = UvZXDTBIrYqcbcpdr2964510;     UvZXDTBIrYqcbcpdr2964510 = UvZXDTBIrYqcbcpdr8993117;     UvZXDTBIrYqcbcpdr8993117 = UvZXDTBIrYqcbcpdr20765216;     UvZXDTBIrYqcbcpdr20765216 = UvZXDTBIrYqcbcpdr84058397;     UvZXDTBIrYqcbcpdr84058397 = UvZXDTBIrYqcbcpdr33747536;     UvZXDTBIrYqcbcpdr33747536 = UvZXDTBIrYqcbcpdr45339852;     UvZXDTBIrYqcbcpdr45339852 = UvZXDTBIrYqcbcpdr90393872;     UvZXDTBIrYqcbcpdr90393872 = UvZXDTBIrYqcbcpdr37801075;     UvZXDTBIrYqcbcpdr37801075 = UvZXDTBIrYqcbcpdr36538506;     UvZXDTBIrYqcbcpdr36538506 = UvZXDTBIrYqcbcpdr85012789;     UvZXDTBIrYqcbcpdr85012789 = UvZXDTBIrYqcbcpdr77159357;     UvZXDTBIrYqcbcpdr77159357 = UvZXDTBIrYqcbcpdr86346511;     UvZXDTBIrYqcbcpdr86346511 = UvZXDTBIrYqcbcpdr16163050;     UvZXDTBIrYqcbcpdr16163050 = UvZXDTBIrYqcbcpdr2586940;     UvZXDTBIrYqcbcpdr2586940 = UvZXDTBIrYqcbcpdr92823019;     UvZXDTBIrYqcbcpdr92823019 = UvZXDTBIrYqcbcpdr15929663;     UvZXDTBIrYqcbcpdr15929663 = UvZXDTBIrYqcbcpdr51364817;     UvZXDTBIrYqcbcpdr51364817 = UvZXDTBIrYqcbcpdr73622443;     UvZXDTBIrYqcbcpdr73622443 = UvZXDTBIrYqcbcpdr80921610;     UvZXDTBIrYqcbcpdr80921610 = UvZXDTBIrYqcbcpdr45013351;     UvZXDTBIrYqcbcpdr45013351 = UvZXDTBIrYqcbcpdr53256626;     UvZXDTBIrYqcbcpdr53256626 = UvZXDTBIrYqcbcpdr78438794;     UvZXDTBIrYqcbcpdr78438794 = UvZXDTBIrYqcbcpdr55929023;     UvZXDTBIrYqcbcpdr55929023 = UvZXDTBIrYqcbcpdr64831390;     UvZXDTBIrYqcbcpdr64831390 = UvZXDTBIrYqcbcpdr89852582;     UvZXDTBIrYqcbcpdr89852582 = UvZXDTBIrYqcbcpdr44090879;     UvZXDTBIrYqcbcpdr44090879 = UvZXDTBIrYqcbcpdr36415413;     UvZXDTBIrYqcbcpdr36415413 = UvZXDTBIrYqcbcpdr15994178;     UvZXDTBIrYqcbcpdr15994178 = UvZXDTBIrYqcbcpdr69802201;     UvZXDTBIrYqcbcpdr69802201 = UvZXDTBIrYqcbcpdr604958;     UvZXDTBIrYqcbcpdr604958 = UvZXDTBIrYqcbcpdr82374279;     UvZXDTBIrYqcbcpdr82374279 = UvZXDTBIrYqcbcpdr34329496;     UvZXDTBIrYqcbcpdr34329496 = UvZXDTBIrYqcbcpdr49626766;     UvZXDTBIrYqcbcpdr49626766 = UvZXDTBIrYqcbcpdr49915675;     UvZXDTBIrYqcbcpdr49915675 = UvZXDTBIrYqcbcpdr24075634;     UvZXDTBIrYqcbcpdr24075634 = UvZXDTBIrYqcbcpdr35142772;     UvZXDTBIrYqcbcpdr35142772 = UvZXDTBIrYqcbcpdr11216459;     UvZXDTBIrYqcbcpdr11216459 = UvZXDTBIrYqcbcpdr3388872;     UvZXDTBIrYqcbcpdr3388872 = UvZXDTBIrYqcbcpdr44930995;     UvZXDTBIrYqcbcpdr44930995 = UvZXDTBIrYqcbcpdr69063424;     UvZXDTBIrYqcbcpdr69063424 = UvZXDTBIrYqcbcpdr11578429;     UvZXDTBIrYqcbcpdr11578429 = UvZXDTBIrYqcbcpdr94854148;     UvZXDTBIrYqcbcpdr94854148 = UvZXDTBIrYqcbcpdr76313428;     UvZXDTBIrYqcbcpdr76313428 = UvZXDTBIrYqcbcpdr31911987;     UvZXDTBIrYqcbcpdr31911987 = UvZXDTBIrYqcbcpdr54641536;     UvZXDTBIrYqcbcpdr54641536 = UvZXDTBIrYqcbcpdr37777468;     UvZXDTBIrYqcbcpdr37777468 = UvZXDTBIrYqcbcpdr49054309;     UvZXDTBIrYqcbcpdr49054309 = UvZXDTBIrYqcbcpdr5647124;     UvZXDTBIrYqcbcpdr5647124 = UvZXDTBIrYqcbcpdr42390121;     UvZXDTBIrYqcbcpdr42390121 = UvZXDTBIrYqcbcpdr77388091;     UvZXDTBIrYqcbcpdr77388091 = UvZXDTBIrYqcbcpdr97081787;     UvZXDTBIrYqcbcpdr97081787 = UvZXDTBIrYqcbcpdr30290633;     UvZXDTBIrYqcbcpdr30290633 = UvZXDTBIrYqcbcpdr69213754;     UvZXDTBIrYqcbcpdr69213754 = UvZXDTBIrYqcbcpdr65956258;     UvZXDTBIrYqcbcpdr65956258 = UvZXDTBIrYqcbcpdr53809753;     UvZXDTBIrYqcbcpdr53809753 = UvZXDTBIrYqcbcpdr24525716;     UvZXDTBIrYqcbcpdr24525716 = UvZXDTBIrYqcbcpdr53064093;     UvZXDTBIrYqcbcpdr53064093 = UvZXDTBIrYqcbcpdr55933826;     UvZXDTBIrYqcbcpdr55933826 = UvZXDTBIrYqcbcpdr94205815;     UvZXDTBIrYqcbcpdr94205815 = UvZXDTBIrYqcbcpdr89656656;     UvZXDTBIrYqcbcpdr89656656 = UvZXDTBIrYqcbcpdr8924439;     UvZXDTBIrYqcbcpdr8924439 = UvZXDTBIrYqcbcpdr74399695;     UvZXDTBIrYqcbcpdr74399695 = UvZXDTBIrYqcbcpdr67998874;     UvZXDTBIrYqcbcpdr67998874 = UvZXDTBIrYqcbcpdr35933549;     UvZXDTBIrYqcbcpdr35933549 = UvZXDTBIrYqcbcpdr46985652;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void umlBUSUwApATmmmZ10373955() {     double cCkFVxjQmcHKAWMvD53102758 = 17121717;    double cCkFVxjQmcHKAWMvD29285129 = -238471309;    double cCkFVxjQmcHKAWMvD7332912 = -746915731;    double cCkFVxjQmcHKAWMvD99928579 = -462027512;    double cCkFVxjQmcHKAWMvD65527772 = -657025509;    double cCkFVxjQmcHKAWMvD5393764 = -630013516;    double cCkFVxjQmcHKAWMvD69797203 = 12880262;    double cCkFVxjQmcHKAWMvD94409367 = -759138510;    double cCkFVxjQmcHKAWMvD15849813 = -452426730;    double cCkFVxjQmcHKAWMvD4349615 = -98535434;    double cCkFVxjQmcHKAWMvD9161853 = -509653451;    double cCkFVxjQmcHKAWMvD18110399 = -801978943;    double cCkFVxjQmcHKAWMvD70673979 = -316512064;    double cCkFVxjQmcHKAWMvD43740052 = -168051166;    double cCkFVxjQmcHKAWMvD52724481 = -478373773;    double cCkFVxjQmcHKAWMvD68545537 = -400173291;    double cCkFVxjQmcHKAWMvD37971891 = -571142843;    double cCkFVxjQmcHKAWMvD29034828 = -327071452;    double cCkFVxjQmcHKAWMvD49277110 = -596124483;    double cCkFVxjQmcHKAWMvD59228695 = -340347711;    double cCkFVxjQmcHKAWMvD2956470 = -816732590;    double cCkFVxjQmcHKAWMvD88493324 = -335009606;    double cCkFVxjQmcHKAWMvD68246015 = -30244325;    double cCkFVxjQmcHKAWMvD96844308 = -301929396;    double cCkFVxjQmcHKAWMvD60624426 = -316030895;    double cCkFVxjQmcHKAWMvD56522569 = -883606218;    double cCkFVxjQmcHKAWMvD22583758 = 2203263;    double cCkFVxjQmcHKAWMvD78324483 = -457819989;    double cCkFVxjQmcHKAWMvD74366318 = -21369095;    double cCkFVxjQmcHKAWMvD3828778 = -209453148;    double cCkFVxjQmcHKAWMvD85560303 = -251792096;    double cCkFVxjQmcHKAWMvD46057226 = -300480743;    double cCkFVxjQmcHKAWMvD13624025 = -597205357;    double cCkFVxjQmcHKAWMvD44965929 = -307363091;    double cCkFVxjQmcHKAWMvD72444260 = -216724861;    double cCkFVxjQmcHKAWMvD75792292 = 15902318;    double cCkFVxjQmcHKAWMvD84226444 = -942000911;    double cCkFVxjQmcHKAWMvD43083329 = -960629731;    double cCkFVxjQmcHKAWMvD94375469 = -695429507;    double cCkFVxjQmcHKAWMvD71560335 = -541630607;    double cCkFVxjQmcHKAWMvD39416760 = -265850129;    double cCkFVxjQmcHKAWMvD77490309 = -729978017;    double cCkFVxjQmcHKAWMvD68460850 = 36786442;    double cCkFVxjQmcHKAWMvD25208816 = -388431685;    double cCkFVxjQmcHKAWMvD48490971 = -304163452;    double cCkFVxjQmcHKAWMvD96951984 = 64263910;    double cCkFVxjQmcHKAWMvD94937931 = -872371693;    double cCkFVxjQmcHKAWMvD94303451 = -690377910;    double cCkFVxjQmcHKAWMvD25838779 = -582553299;    double cCkFVxjQmcHKAWMvD15498593 = -623573089;    double cCkFVxjQmcHKAWMvD28523698 = -901472717;    double cCkFVxjQmcHKAWMvD26739382 = 58991313;    double cCkFVxjQmcHKAWMvD99613274 = -783037557;    double cCkFVxjQmcHKAWMvD69591324 = -489037741;    double cCkFVxjQmcHKAWMvD7417485 = -244017614;    double cCkFVxjQmcHKAWMvD64609433 = -647868677;    double cCkFVxjQmcHKAWMvD61039114 = -108226984;    double cCkFVxjQmcHKAWMvD10488604 = -344986336;    double cCkFVxjQmcHKAWMvD39304154 = -45996617;    double cCkFVxjQmcHKAWMvD9005204 = -773419291;    double cCkFVxjQmcHKAWMvD82810006 = -532216780;    double cCkFVxjQmcHKAWMvD91472719 = -529299750;    double cCkFVxjQmcHKAWMvD20043049 = -637769415;    double cCkFVxjQmcHKAWMvD12021036 = -142973583;    double cCkFVxjQmcHKAWMvD18789311 = -846743339;    double cCkFVxjQmcHKAWMvD63104626 = -109172709;    double cCkFVxjQmcHKAWMvD4486374 = -104773587;    double cCkFVxjQmcHKAWMvD25708051 = 90851026;    double cCkFVxjQmcHKAWMvD71295791 = -951326305;    double cCkFVxjQmcHKAWMvD76932188 = -394276091;    double cCkFVxjQmcHKAWMvD84319093 = -458172381;    double cCkFVxjQmcHKAWMvD94888562 = -610513113;    double cCkFVxjQmcHKAWMvD34659358 = -631641946;    double cCkFVxjQmcHKAWMvD77716775 = 45506123;    double cCkFVxjQmcHKAWMvD19811936 = 25502418;    double cCkFVxjQmcHKAWMvD25466161 = 13245427;    double cCkFVxjQmcHKAWMvD20032475 = -271796048;    double cCkFVxjQmcHKAWMvD43037199 = -641812641;    double cCkFVxjQmcHKAWMvD48353337 = -997765944;    double cCkFVxjQmcHKAWMvD63672441 = -280294806;    double cCkFVxjQmcHKAWMvD61584637 = 88765475;    double cCkFVxjQmcHKAWMvD28280306 = -307418828;    double cCkFVxjQmcHKAWMvD52485705 = -875266690;    double cCkFVxjQmcHKAWMvD58867726 = -397796007;    double cCkFVxjQmcHKAWMvD75305079 = -307980431;    double cCkFVxjQmcHKAWMvD58820921 = -210783409;    double cCkFVxjQmcHKAWMvD46443952 = -517443186;    double cCkFVxjQmcHKAWMvD44032701 = -8167616;    double cCkFVxjQmcHKAWMvD37548444 = 36654523;    double cCkFVxjQmcHKAWMvD7834827 = -568856184;    double cCkFVxjQmcHKAWMvD14753179 = -875870699;    double cCkFVxjQmcHKAWMvD73737840 = -497014576;    double cCkFVxjQmcHKAWMvD3779175 = -814633114;    double cCkFVxjQmcHKAWMvD85370266 = -922010216;    double cCkFVxjQmcHKAWMvD88750329 = 90586172;    double cCkFVxjQmcHKAWMvD47944040 = -736550380;    double cCkFVxjQmcHKAWMvD57447260 = 7791397;    double cCkFVxjQmcHKAWMvD56439815 = -820239975;    double cCkFVxjQmcHKAWMvD6419505 = -541688346;    double cCkFVxjQmcHKAWMvD85386345 = 17121717;     cCkFVxjQmcHKAWMvD53102758 = cCkFVxjQmcHKAWMvD29285129;     cCkFVxjQmcHKAWMvD29285129 = cCkFVxjQmcHKAWMvD7332912;     cCkFVxjQmcHKAWMvD7332912 = cCkFVxjQmcHKAWMvD99928579;     cCkFVxjQmcHKAWMvD99928579 = cCkFVxjQmcHKAWMvD65527772;     cCkFVxjQmcHKAWMvD65527772 = cCkFVxjQmcHKAWMvD5393764;     cCkFVxjQmcHKAWMvD5393764 = cCkFVxjQmcHKAWMvD69797203;     cCkFVxjQmcHKAWMvD69797203 = cCkFVxjQmcHKAWMvD94409367;     cCkFVxjQmcHKAWMvD94409367 = cCkFVxjQmcHKAWMvD15849813;     cCkFVxjQmcHKAWMvD15849813 = cCkFVxjQmcHKAWMvD4349615;     cCkFVxjQmcHKAWMvD4349615 = cCkFVxjQmcHKAWMvD9161853;     cCkFVxjQmcHKAWMvD9161853 = cCkFVxjQmcHKAWMvD18110399;     cCkFVxjQmcHKAWMvD18110399 = cCkFVxjQmcHKAWMvD70673979;     cCkFVxjQmcHKAWMvD70673979 = cCkFVxjQmcHKAWMvD43740052;     cCkFVxjQmcHKAWMvD43740052 = cCkFVxjQmcHKAWMvD52724481;     cCkFVxjQmcHKAWMvD52724481 = cCkFVxjQmcHKAWMvD68545537;     cCkFVxjQmcHKAWMvD68545537 = cCkFVxjQmcHKAWMvD37971891;     cCkFVxjQmcHKAWMvD37971891 = cCkFVxjQmcHKAWMvD29034828;     cCkFVxjQmcHKAWMvD29034828 = cCkFVxjQmcHKAWMvD49277110;     cCkFVxjQmcHKAWMvD49277110 = cCkFVxjQmcHKAWMvD59228695;     cCkFVxjQmcHKAWMvD59228695 = cCkFVxjQmcHKAWMvD2956470;     cCkFVxjQmcHKAWMvD2956470 = cCkFVxjQmcHKAWMvD88493324;     cCkFVxjQmcHKAWMvD88493324 = cCkFVxjQmcHKAWMvD68246015;     cCkFVxjQmcHKAWMvD68246015 = cCkFVxjQmcHKAWMvD96844308;     cCkFVxjQmcHKAWMvD96844308 = cCkFVxjQmcHKAWMvD60624426;     cCkFVxjQmcHKAWMvD60624426 = cCkFVxjQmcHKAWMvD56522569;     cCkFVxjQmcHKAWMvD56522569 = cCkFVxjQmcHKAWMvD22583758;     cCkFVxjQmcHKAWMvD22583758 = cCkFVxjQmcHKAWMvD78324483;     cCkFVxjQmcHKAWMvD78324483 = cCkFVxjQmcHKAWMvD74366318;     cCkFVxjQmcHKAWMvD74366318 = cCkFVxjQmcHKAWMvD3828778;     cCkFVxjQmcHKAWMvD3828778 = cCkFVxjQmcHKAWMvD85560303;     cCkFVxjQmcHKAWMvD85560303 = cCkFVxjQmcHKAWMvD46057226;     cCkFVxjQmcHKAWMvD46057226 = cCkFVxjQmcHKAWMvD13624025;     cCkFVxjQmcHKAWMvD13624025 = cCkFVxjQmcHKAWMvD44965929;     cCkFVxjQmcHKAWMvD44965929 = cCkFVxjQmcHKAWMvD72444260;     cCkFVxjQmcHKAWMvD72444260 = cCkFVxjQmcHKAWMvD75792292;     cCkFVxjQmcHKAWMvD75792292 = cCkFVxjQmcHKAWMvD84226444;     cCkFVxjQmcHKAWMvD84226444 = cCkFVxjQmcHKAWMvD43083329;     cCkFVxjQmcHKAWMvD43083329 = cCkFVxjQmcHKAWMvD94375469;     cCkFVxjQmcHKAWMvD94375469 = cCkFVxjQmcHKAWMvD71560335;     cCkFVxjQmcHKAWMvD71560335 = cCkFVxjQmcHKAWMvD39416760;     cCkFVxjQmcHKAWMvD39416760 = cCkFVxjQmcHKAWMvD77490309;     cCkFVxjQmcHKAWMvD77490309 = cCkFVxjQmcHKAWMvD68460850;     cCkFVxjQmcHKAWMvD68460850 = cCkFVxjQmcHKAWMvD25208816;     cCkFVxjQmcHKAWMvD25208816 = cCkFVxjQmcHKAWMvD48490971;     cCkFVxjQmcHKAWMvD48490971 = cCkFVxjQmcHKAWMvD96951984;     cCkFVxjQmcHKAWMvD96951984 = cCkFVxjQmcHKAWMvD94937931;     cCkFVxjQmcHKAWMvD94937931 = cCkFVxjQmcHKAWMvD94303451;     cCkFVxjQmcHKAWMvD94303451 = cCkFVxjQmcHKAWMvD25838779;     cCkFVxjQmcHKAWMvD25838779 = cCkFVxjQmcHKAWMvD15498593;     cCkFVxjQmcHKAWMvD15498593 = cCkFVxjQmcHKAWMvD28523698;     cCkFVxjQmcHKAWMvD28523698 = cCkFVxjQmcHKAWMvD26739382;     cCkFVxjQmcHKAWMvD26739382 = cCkFVxjQmcHKAWMvD99613274;     cCkFVxjQmcHKAWMvD99613274 = cCkFVxjQmcHKAWMvD69591324;     cCkFVxjQmcHKAWMvD69591324 = cCkFVxjQmcHKAWMvD7417485;     cCkFVxjQmcHKAWMvD7417485 = cCkFVxjQmcHKAWMvD64609433;     cCkFVxjQmcHKAWMvD64609433 = cCkFVxjQmcHKAWMvD61039114;     cCkFVxjQmcHKAWMvD61039114 = cCkFVxjQmcHKAWMvD10488604;     cCkFVxjQmcHKAWMvD10488604 = cCkFVxjQmcHKAWMvD39304154;     cCkFVxjQmcHKAWMvD39304154 = cCkFVxjQmcHKAWMvD9005204;     cCkFVxjQmcHKAWMvD9005204 = cCkFVxjQmcHKAWMvD82810006;     cCkFVxjQmcHKAWMvD82810006 = cCkFVxjQmcHKAWMvD91472719;     cCkFVxjQmcHKAWMvD91472719 = cCkFVxjQmcHKAWMvD20043049;     cCkFVxjQmcHKAWMvD20043049 = cCkFVxjQmcHKAWMvD12021036;     cCkFVxjQmcHKAWMvD12021036 = cCkFVxjQmcHKAWMvD18789311;     cCkFVxjQmcHKAWMvD18789311 = cCkFVxjQmcHKAWMvD63104626;     cCkFVxjQmcHKAWMvD63104626 = cCkFVxjQmcHKAWMvD4486374;     cCkFVxjQmcHKAWMvD4486374 = cCkFVxjQmcHKAWMvD25708051;     cCkFVxjQmcHKAWMvD25708051 = cCkFVxjQmcHKAWMvD71295791;     cCkFVxjQmcHKAWMvD71295791 = cCkFVxjQmcHKAWMvD76932188;     cCkFVxjQmcHKAWMvD76932188 = cCkFVxjQmcHKAWMvD84319093;     cCkFVxjQmcHKAWMvD84319093 = cCkFVxjQmcHKAWMvD94888562;     cCkFVxjQmcHKAWMvD94888562 = cCkFVxjQmcHKAWMvD34659358;     cCkFVxjQmcHKAWMvD34659358 = cCkFVxjQmcHKAWMvD77716775;     cCkFVxjQmcHKAWMvD77716775 = cCkFVxjQmcHKAWMvD19811936;     cCkFVxjQmcHKAWMvD19811936 = cCkFVxjQmcHKAWMvD25466161;     cCkFVxjQmcHKAWMvD25466161 = cCkFVxjQmcHKAWMvD20032475;     cCkFVxjQmcHKAWMvD20032475 = cCkFVxjQmcHKAWMvD43037199;     cCkFVxjQmcHKAWMvD43037199 = cCkFVxjQmcHKAWMvD48353337;     cCkFVxjQmcHKAWMvD48353337 = cCkFVxjQmcHKAWMvD63672441;     cCkFVxjQmcHKAWMvD63672441 = cCkFVxjQmcHKAWMvD61584637;     cCkFVxjQmcHKAWMvD61584637 = cCkFVxjQmcHKAWMvD28280306;     cCkFVxjQmcHKAWMvD28280306 = cCkFVxjQmcHKAWMvD52485705;     cCkFVxjQmcHKAWMvD52485705 = cCkFVxjQmcHKAWMvD58867726;     cCkFVxjQmcHKAWMvD58867726 = cCkFVxjQmcHKAWMvD75305079;     cCkFVxjQmcHKAWMvD75305079 = cCkFVxjQmcHKAWMvD58820921;     cCkFVxjQmcHKAWMvD58820921 = cCkFVxjQmcHKAWMvD46443952;     cCkFVxjQmcHKAWMvD46443952 = cCkFVxjQmcHKAWMvD44032701;     cCkFVxjQmcHKAWMvD44032701 = cCkFVxjQmcHKAWMvD37548444;     cCkFVxjQmcHKAWMvD37548444 = cCkFVxjQmcHKAWMvD7834827;     cCkFVxjQmcHKAWMvD7834827 = cCkFVxjQmcHKAWMvD14753179;     cCkFVxjQmcHKAWMvD14753179 = cCkFVxjQmcHKAWMvD73737840;     cCkFVxjQmcHKAWMvD73737840 = cCkFVxjQmcHKAWMvD3779175;     cCkFVxjQmcHKAWMvD3779175 = cCkFVxjQmcHKAWMvD85370266;     cCkFVxjQmcHKAWMvD85370266 = cCkFVxjQmcHKAWMvD88750329;     cCkFVxjQmcHKAWMvD88750329 = cCkFVxjQmcHKAWMvD47944040;     cCkFVxjQmcHKAWMvD47944040 = cCkFVxjQmcHKAWMvD57447260;     cCkFVxjQmcHKAWMvD57447260 = cCkFVxjQmcHKAWMvD56439815;     cCkFVxjQmcHKAWMvD56439815 = cCkFVxjQmcHKAWMvD6419505;     cCkFVxjQmcHKAWMvD6419505 = cCkFVxjQmcHKAWMvD85386345;     cCkFVxjQmcHKAWMvD85386345 = cCkFVxjQmcHKAWMvD53102758;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LqPAylggvyjpPWoM77665553() {     double nShFLaJWggMauqmTr88561726 = -449155183;    double nShFLaJWggMauqmTr42557207 = -119022915;    double nShFLaJWggMauqmTr44719389 = -412582806;    double nShFLaJWggMauqmTr72151934 = -283178584;    double nShFLaJWggMauqmTr21813042 = -762624354;    double nShFLaJWggMauqmTr41616636 = -892361412;    double nShFLaJWggMauqmTr80454609 = -931845472;    double nShFLaJWggMauqmTr56540928 = -403372650;    double nShFLaJWggMauqmTr17367419 = -940758137;    double nShFLaJWggMauqmTr19252327 = -863222432;    double nShFLaJWggMauqmTr47343587 = -311661180;    double nShFLaJWggMauqmTr18678547 = -666099715;    double nShFLaJWggMauqmTr8908625 = -868524569;    double nShFLaJWggMauqmTr96880914 = -144414575;    double nShFLaJWggMauqmTr10918998 = -981798788;    double nShFLaJWggMauqmTr52086838 = -35984898;    double nShFLaJWggMauqmTr93807827 = 1882604;    double nShFLaJWggMauqmTr28795020 = -400499520;    double nShFLaJWggMauqmTr49827440 = -912687905;    double nShFLaJWggMauqmTr53065914 = -766908777;    double nShFLaJWggMauqmTr16043333 = -474007279;    double nShFLaJWggMauqmTr91914849 = -421645900;    double nShFLaJWggMauqmTr38771395 = -767876510;    double nShFLaJWggMauqmTr90637253 = -812754000;    double nShFLaJWggMauqmTr68642377 = -340016094;    double nShFLaJWggMauqmTr67220741 = -572183025;    double nShFLaJWggMauqmTr83004331 = -904080878;    double nShFLaJWggMauqmTr19052909 = -627592163;    double nShFLaJWggMauqmTr21034533 = -595463898;    double nShFLaJWggMauqmTr9362189 = -243960318;    double nShFLaJWggMauqmTr4143535 = -803772583;    double nShFLaJWggMauqmTr83880245 = -73452157;    double nShFLaJWggMauqmTr25842036 = -744801334;    double nShFLaJWggMauqmTr74714355 = -372056904;    double nShFLaJWggMauqmTr38252714 = -167825112;    double nShFLaJWggMauqmTr27353137 = -198662988;    double nShFLaJWggMauqmTr33638261 = -806696951;    double nShFLaJWggMauqmTr94298732 = -943285312;    double nShFLaJWggMauqmTr24766032 = -346305783;    double nShFLaJWggMauqmTr35831868 = -6893153;    double nShFLaJWggMauqmTr72868367 = -35049044;    double nShFLaJWggMauqmTr66855435 = -324523308;    double nShFLaJWggMauqmTr57844229 = -616394344;    double nShFLaJWggMauqmTr65340154 = -992764095;    double nShFLaJWggMauqmTr48507430 = -130674614;    double nShFLaJWggMauqmTr42913313 = 25340278;    double nShFLaJWggMauqmTr14283096 = -256358054;    double nShFLaJWggMauqmTr12029764 = -856904503;    double nShFLaJWggMauqmTr48731444 = -246290179;    double nShFLaJWggMauqmTr40028182 = -14599905;    double nShFLaJWggMauqmTr9294422 = -775749274;    double nShFLaJWggMauqmTr10224855 = -154661033;    double nShFLaJWggMauqmTr68234423 = -539083111;    double nShFLaJWggMauqmTr85583170 = -680546241;    double nShFLaJWggMauqmTr61622221 = -590507760;    double nShFLaJWggMauqmTr96646877 = 72490717;    double nShFLaJWggMauqmTr3785813 = -351146406;    double nShFLaJWggMauqmTr54082135 = -599828807;    double nShFLaJWggMauqmTr3509558 = -943162490;    double nShFLaJWggMauqmTr54592301 = -90441330;    double nShFLaJWggMauqmTr58612304 = -988280534;    double nShFLaJWggMauqmTr61401701 = -204253309;    double nShFLaJWggMauqmTr35506395 = -807908753;    double nShFLaJWggMauqmTr8005231 = -596797820;    double nShFLaJWggMauqmTr15108792 = 40550150;    double nShFLaJWggMauqmTr63463342 = -138209024;    double nShFLaJWggMauqmTr92836510 = -921298381;    double nShFLaJWggMauqmTr34194270 = -396467666;    double nShFLaJWggMauqmTr58628201 = -976589464;    double nShFLaJWggMauqmTr83565861 = -683135801;    double nShFLaJWggMauqmTr18448578 = -229287947;    double nShFLaJWggMauqmTr99509094 = -54832085;    double nShFLaJWggMauqmTr4028989 = 45806262;    double nShFLaJWggMauqmTr13995573 = -805794752;    double nShFLaJWggMauqmTr80197546 = -631859734;    double nShFLaJWggMauqmTr49187897 = -49483971;    double nShFLaJWggMauqmTr34070621 = -805251556;    double nShFLaJWggMauqmTr73431241 = -775112415;    double nShFLaJWggMauqmTr42129824 = -582079387;    double nShFLaJWggMauqmTr25729064 = -265356372;    double nShFLaJWggMauqmTr52937645 = -215824971;    double nShFLaJWggMauqmTr70974567 = 52823625;    double nShFLaJWggMauqmTr70321464 = -281301985;    double nShFLaJWggMauqmTr81006351 = -480863993;    double nShFLaJWggMauqmTr67767 = -468211044;    double nShFLaJWggMauqmTr93918680 = -549111550;    double nShFLaJWggMauqmTr15645822 = -534369047;    double nShFLaJWggMauqmTr40258866 = 35744906;    double nShFLaJWggMauqmTr13092134 = -781549144;    double nShFLaJWggMauqmTr41605837 = -140315829;    double nShFLaJWggMauqmTr23567324 = -847516582;    double nShFLaJWggMauqmTr79556125 = -106868145;    double nShFLaJWggMauqmTr90789174 = 99877178;    double nShFLaJWggMauqmTr70173730 = -155864454;    double nShFLaJWggMauqmTr77219563 = -18612620;    double nShFLaJWggMauqmTr11466667 = -830795735;    double nShFLaJWggMauqmTr31349041 = -516614556;    double nShFLaJWggMauqmTr49838999 = 80403475;    double nShFLaJWggMauqmTr50231362 = -933314245;    double nShFLaJWggMauqmTr85044087 = -449155183;     nShFLaJWggMauqmTr88561726 = nShFLaJWggMauqmTr42557207;     nShFLaJWggMauqmTr42557207 = nShFLaJWggMauqmTr44719389;     nShFLaJWggMauqmTr44719389 = nShFLaJWggMauqmTr72151934;     nShFLaJWggMauqmTr72151934 = nShFLaJWggMauqmTr21813042;     nShFLaJWggMauqmTr21813042 = nShFLaJWggMauqmTr41616636;     nShFLaJWggMauqmTr41616636 = nShFLaJWggMauqmTr80454609;     nShFLaJWggMauqmTr80454609 = nShFLaJWggMauqmTr56540928;     nShFLaJWggMauqmTr56540928 = nShFLaJWggMauqmTr17367419;     nShFLaJWggMauqmTr17367419 = nShFLaJWggMauqmTr19252327;     nShFLaJWggMauqmTr19252327 = nShFLaJWggMauqmTr47343587;     nShFLaJWggMauqmTr47343587 = nShFLaJWggMauqmTr18678547;     nShFLaJWggMauqmTr18678547 = nShFLaJWggMauqmTr8908625;     nShFLaJWggMauqmTr8908625 = nShFLaJWggMauqmTr96880914;     nShFLaJWggMauqmTr96880914 = nShFLaJWggMauqmTr10918998;     nShFLaJWggMauqmTr10918998 = nShFLaJWggMauqmTr52086838;     nShFLaJWggMauqmTr52086838 = nShFLaJWggMauqmTr93807827;     nShFLaJWggMauqmTr93807827 = nShFLaJWggMauqmTr28795020;     nShFLaJWggMauqmTr28795020 = nShFLaJWggMauqmTr49827440;     nShFLaJWggMauqmTr49827440 = nShFLaJWggMauqmTr53065914;     nShFLaJWggMauqmTr53065914 = nShFLaJWggMauqmTr16043333;     nShFLaJWggMauqmTr16043333 = nShFLaJWggMauqmTr91914849;     nShFLaJWggMauqmTr91914849 = nShFLaJWggMauqmTr38771395;     nShFLaJWggMauqmTr38771395 = nShFLaJWggMauqmTr90637253;     nShFLaJWggMauqmTr90637253 = nShFLaJWggMauqmTr68642377;     nShFLaJWggMauqmTr68642377 = nShFLaJWggMauqmTr67220741;     nShFLaJWggMauqmTr67220741 = nShFLaJWggMauqmTr83004331;     nShFLaJWggMauqmTr83004331 = nShFLaJWggMauqmTr19052909;     nShFLaJWggMauqmTr19052909 = nShFLaJWggMauqmTr21034533;     nShFLaJWggMauqmTr21034533 = nShFLaJWggMauqmTr9362189;     nShFLaJWggMauqmTr9362189 = nShFLaJWggMauqmTr4143535;     nShFLaJWggMauqmTr4143535 = nShFLaJWggMauqmTr83880245;     nShFLaJWggMauqmTr83880245 = nShFLaJWggMauqmTr25842036;     nShFLaJWggMauqmTr25842036 = nShFLaJWggMauqmTr74714355;     nShFLaJWggMauqmTr74714355 = nShFLaJWggMauqmTr38252714;     nShFLaJWggMauqmTr38252714 = nShFLaJWggMauqmTr27353137;     nShFLaJWggMauqmTr27353137 = nShFLaJWggMauqmTr33638261;     nShFLaJWggMauqmTr33638261 = nShFLaJWggMauqmTr94298732;     nShFLaJWggMauqmTr94298732 = nShFLaJWggMauqmTr24766032;     nShFLaJWggMauqmTr24766032 = nShFLaJWggMauqmTr35831868;     nShFLaJWggMauqmTr35831868 = nShFLaJWggMauqmTr72868367;     nShFLaJWggMauqmTr72868367 = nShFLaJWggMauqmTr66855435;     nShFLaJWggMauqmTr66855435 = nShFLaJWggMauqmTr57844229;     nShFLaJWggMauqmTr57844229 = nShFLaJWggMauqmTr65340154;     nShFLaJWggMauqmTr65340154 = nShFLaJWggMauqmTr48507430;     nShFLaJWggMauqmTr48507430 = nShFLaJWggMauqmTr42913313;     nShFLaJWggMauqmTr42913313 = nShFLaJWggMauqmTr14283096;     nShFLaJWggMauqmTr14283096 = nShFLaJWggMauqmTr12029764;     nShFLaJWggMauqmTr12029764 = nShFLaJWggMauqmTr48731444;     nShFLaJWggMauqmTr48731444 = nShFLaJWggMauqmTr40028182;     nShFLaJWggMauqmTr40028182 = nShFLaJWggMauqmTr9294422;     nShFLaJWggMauqmTr9294422 = nShFLaJWggMauqmTr10224855;     nShFLaJWggMauqmTr10224855 = nShFLaJWggMauqmTr68234423;     nShFLaJWggMauqmTr68234423 = nShFLaJWggMauqmTr85583170;     nShFLaJWggMauqmTr85583170 = nShFLaJWggMauqmTr61622221;     nShFLaJWggMauqmTr61622221 = nShFLaJWggMauqmTr96646877;     nShFLaJWggMauqmTr96646877 = nShFLaJWggMauqmTr3785813;     nShFLaJWggMauqmTr3785813 = nShFLaJWggMauqmTr54082135;     nShFLaJWggMauqmTr54082135 = nShFLaJWggMauqmTr3509558;     nShFLaJWggMauqmTr3509558 = nShFLaJWggMauqmTr54592301;     nShFLaJWggMauqmTr54592301 = nShFLaJWggMauqmTr58612304;     nShFLaJWggMauqmTr58612304 = nShFLaJWggMauqmTr61401701;     nShFLaJWggMauqmTr61401701 = nShFLaJWggMauqmTr35506395;     nShFLaJWggMauqmTr35506395 = nShFLaJWggMauqmTr8005231;     nShFLaJWggMauqmTr8005231 = nShFLaJWggMauqmTr15108792;     nShFLaJWggMauqmTr15108792 = nShFLaJWggMauqmTr63463342;     nShFLaJWggMauqmTr63463342 = nShFLaJWggMauqmTr92836510;     nShFLaJWggMauqmTr92836510 = nShFLaJWggMauqmTr34194270;     nShFLaJWggMauqmTr34194270 = nShFLaJWggMauqmTr58628201;     nShFLaJWggMauqmTr58628201 = nShFLaJWggMauqmTr83565861;     nShFLaJWggMauqmTr83565861 = nShFLaJWggMauqmTr18448578;     nShFLaJWggMauqmTr18448578 = nShFLaJWggMauqmTr99509094;     nShFLaJWggMauqmTr99509094 = nShFLaJWggMauqmTr4028989;     nShFLaJWggMauqmTr4028989 = nShFLaJWggMauqmTr13995573;     nShFLaJWggMauqmTr13995573 = nShFLaJWggMauqmTr80197546;     nShFLaJWggMauqmTr80197546 = nShFLaJWggMauqmTr49187897;     nShFLaJWggMauqmTr49187897 = nShFLaJWggMauqmTr34070621;     nShFLaJWggMauqmTr34070621 = nShFLaJWggMauqmTr73431241;     nShFLaJWggMauqmTr73431241 = nShFLaJWggMauqmTr42129824;     nShFLaJWggMauqmTr42129824 = nShFLaJWggMauqmTr25729064;     nShFLaJWggMauqmTr25729064 = nShFLaJWggMauqmTr52937645;     nShFLaJWggMauqmTr52937645 = nShFLaJWggMauqmTr70974567;     nShFLaJWggMauqmTr70974567 = nShFLaJWggMauqmTr70321464;     nShFLaJWggMauqmTr70321464 = nShFLaJWggMauqmTr81006351;     nShFLaJWggMauqmTr81006351 = nShFLaJWggMauqmTr67767;     nShFLaJWggMauqmTr67767 = nShFLaJWggMauqmTr93918680;     nShFLaJWggMauqmTr93918680 = nShFLaJWggMauqmTr15645822;     nShFLaJWggMauqmTr15645822 = nShFLaJWggMauqmTr40258866;     nShFLaJWggMauqmTr40258866 = nShFLaJWggMauqmTr13092134;     nShFLaJWggMauqmTr13092134 = nShFLaJWggMauqmTr41605837;     nShFLaJWggMauqmTr41605837 = nShFLaJWggMauqmTr23567324;     nShFLaJWggMauqmTr23567324 = nShFLaJWggMauqmTr79556125;     nShFLaJWggMauqmTr79556125 = nShFLaJWggMauqmTr90789174;     nShFLaJWggMauqmTr90789174 = nShFLaJWggMauqmTr70173730;     nShFLaJWggMauqmTr70173730 = nShFLaJWggMauqmTr77219563;     nShFLaJWggMauqmTr77219563 = nShFLaJWggMauqmTr11466667;     nShFLaJWggMauqmTr11466667 = nShFLaJWggMauqmTr31349041;     nShFLaJWggMauqmTr31349041 = nShFLaJWggMauqmTr49838999;     nShFLaJWggMauqmTr49838999 = nShFLaJWggMauqmTr50231362;     nShFLaJWggMauqmTr50231362 = nShFLaJWggMauqmTr85044087;     nShFLaJWggMauqmTr85044087 = nShFLaJWggMauqmTr88561726;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AaAigButjuPdzYJI92714621() {     double HiFvNrUyXpMKrtBZe94678833 = -561057071;    double HiFvNrUyXpMKrtBZe85930486 = -928622696;    double HiFvNrUyXpMKrtBZe60761572 = -563415727;    double HiFvNrUyXpMKrtBZe99226715 = -473927214;    double HiFvNrUyXpMKrtBZe90708530 = -183274225;    double HiFvNrUyXpMKrtBZe33033838 = -56857647;    double HiFvNrUyXpMKrtBZe40943575 = 29189551;    double HiFvNrUyXpMKrtBZe6300819 = -690246563;    double HiFvNrUyXpMKrtBZe82009913 = -708790803;    double HiFvNrUyXpMKrtBZe60481986 = -92197910;    double HiFvNrUyXpMKrtBZe7453878 = -189098767;    double HiFvNrUyXpMKrtBZe50501589 = -10505916;    double HiFvNrUyXpMKrtBZe95117743 = -208415658;    double HiFvNrUyXpMKrtBZe80024591 = -676653143;    double HiFvNrUyXpMKrtBZe6661424 = -572260954;    double HiFvNrUyXpMKrtBZe93592232 = -8984442;    double HiFvNrUyXpMKrtBZe87643830 = -614662509;    double HiFvNrUyXpMKrtBZe25848173 = -66022163;    double HiFvNrUyXpMKrtBZe11657282 = -218457002;    double HiFvNrUyXpMKrtBZe33616079 = -23811697;    double HiFvNrUyXpMKrtBZe4596526 = -329494734;    double HiFvNrUyXpMKrtBZe78435873 = -329575616;    double HiFvNrUyXpMKrtBZe74362186 = -514393996;    double HiFvNrUyXpMKrtBZe74629627 = 64941012;    double HiFvNrUyXpMKrtBZe12342027 = -713212127;    double HiFvNrUyXpMKrtBZe91942416 = -450838348;    double HiFvNrUyXpMKrtBZe81464109 = -804743081;    double HiFvNrUyXpMKrtBZe32160034 = -693521117;    double HiFvNrUyXpMKrtBZe87166788 = -817846602;    double HiFvNrUyXpMKrtBZe77977825 = -61431186;    double HiFvNrUyXpMKrtBZe96386084 = -717093521;    double HiFvNrUyXpMKrtBZe81490866 = -882898973;    double HiFvNrUyXpMKrtBZe35552985 = -637414704;    double HiFvNrUyXpMKrtBZe69544919 = -123598669;    double HiFvNrUyXpMKrtBZe99727366 = -806558217;    double HiFvNrUyXpMKrtBZe96079049 = -412689096;    double HiFvNrUyXpMKrtBZe14900196 = -760332821;    double HiFvNrUyXpMKrtBZe28388945 = -714791321;    double HiFvNrUyXpMKrtBZe98376285 = -340229280;    double HiFvNrUyXpMKrtBZe23333806 = -480332376;    double HiFvNrUyXpMKrtBZe78537591 = -100322938;    double HiFvNrUyXpMKrtBZe99005892 = -594665587;    double HiFvNrUyXpMKrtBZe35911207 = -697198928;    double HiFvNrUyXpMKrtBZe52747894 = 87892967;    double HiFvNrUyXpMKrtBZe60459895 = -892572768;    double HiFvNrUyXpMKrtBZe54852508 = 40124161;    double HiFvNrUyXpMKrtBZe32061670 = -375133591;    double HiFvNrUyXpMKrtBZe19986705 = -866800036;    double HiFvNrUyXpMKrtBZe58407173 = -332027167;    double HiFvNrUyXpMKrtBZe52939834 = -101610713;    double HiFvNrUyXpMKrtBZe44995100 = -618042946;    double HiFvNrUyXpMKrtBZe21034574 = 44612647;    double HiFvNrUyXpMKrtBZe16482881 = -330872699;    double HiFvNrUyXpMKrtBZe81552050 = 98983614;    double HiFvNrUyXpMKrtBZe88118096 = -191039543;    double HiFvNrUyXpMKrtBZe16242961 = -131481455;    double HiFvNrUyXpMKrtBZe11568300 = -314228700;    double HiFvNrUyXpMKrtBZe86131945 = -528356740;    double HiFvNrUyXpMKrtBZe86884688 = -760715088;    double HiFvNrUyXpMKrtBZe98766114 = -732435878;    double HiFvNrUyXpMKrtBZe51569728 = -252114566;    double HiFvNrUyXpMKrtBZe8783542 = -277289333;    double HiFvNrUyXpMKrtBZe19134031 = -872399962;    double HiFvNrUyXpMKrtBZe4032089 = -547359618;    double HiFvNrUyXpMKrtBZe64095901 = -375104389;    double HiFvNrUyXpMKrtBZe25963011 = -306199795;    double HiFvNrUyXpMKrtBZe14948605 = -373091213;    double HiFvNrUyXpMKrtBZe25572824 = 15183011;    double HiFvNrUyXpMKrtBZe80297225 = -870094926;    double HiFvNrUyXpMKrtBZe10582375 = -59571859;    double HiFvNrUyXpMKrtBZe78692036 = -248651621;    double HiFvNrUyXpMKrtBZe59254885 = -899871188;    double HiFvNrUyXpMKrtBZe27471888 = -725792884;    double HiFvNrUyXpMKrtBZe88323476 = -738124626;    double HiFvNrUyXpMKrtBZe55078487 = -923488759;    double HiFvNrUyXpMKrtBZe5590634 = -734829148;    double HiFvNrUyXpMKrtBZe42524666 = -632376689;    double HiFvNrUyXpMKrtBZe21614293 = -502286964;    double HiFvNrUyXpMKrtBZe14169733 = -42486220;    double HiFvNrUyXpMKrtBZe57489519 = -653336288;    double HiFvNrUyXpMKrtBZe59880746 = 24295243;    double HiFvNrUyXpMKrtBZe61477405 = -937943046;    double HiFvNrUyXpMKrtBZe73752860 = -261493950;    double HiFvNrUyXpMKrtBZe34226954 = -616235890;    double HiFvNrUyXpMKrtBZe32982725 = -443388240;    double HiFvNrUyXpMKrtBZe75351510 = -661706168;    double HiFvNrUyXpMKrtBZe65007986 = -452026275;    double HiFvNrUyXpMKrtBZe54000934 = -636398319;    double HiFvNrUyXpMKrtBZe81426823 = -932559127;    double HiFvNrUyXpMKrtBZe83484406 = -575076762;    double HiFvNrUyXpMKrtBZe84510749 = 1539604;    double HiFvNrUyXpMKrtBZe28768250 = -131976082;    double HiFvNrUyXpMKrtBZe41504257 = -954076234;    double HiFvNrUyXpMKrtBZe99610170 = -607793403;    double HiFvNrUyXpMKrtBZe71764077 = -128217810;    double HiFvNrUyXpMKrtBZe69754049 = -823033605;    double HiFvNrUyXpMKrtBZe79871862 = -722265625;    double HiFvNrUyXpMKrtBZe31879118 = -49839311;    double HiFvNrUyXpMKrtBZe88651992 = -537002644;    double HiFvNrUyXpMKrtBZe34496884 = -561057071;     HiFvNrUyXpMKrtBZe94678833 = HiFvNrUyXpMKrtBZe85930486;     HiFvNrUyXpMKrtBZe85930486 = HiFvNrUyXpMKrtBZe60761572;     HiFvNrUyXpMKrtBZe60761572 = HiFvNrUyXpMKrtBZe99226715;     HiFvNrUyXpMKrtBZe99226715 = HiFvNrUyXpMKrtBZe90708530;     HiFvNrUyXpMKrtBZe90708530 = HiFvNrUyXpMKrtBZe33033838;     HiFvNrUyXpMKrtBZe33033838 = HiFvNrUyXpMKrtBZe40943575;     HiFvNrUyXpMKrtBZe40943575 = HiFvNrUyXpMKrtBZe6300819;     HiFvNrUyXpMKrtBZe6300819 = HiFvNrUyXpMKrtBZe82009913;     HiFvNrUyXpMKrtBZe82009913 = HiFvNrUyXpMKrtBZe60481986;     HiFvNrUyXpMKrtBZe60481986 = HiFvNrUyXpMKrtBZe7453878;     HiFvNrUyXpMKrtBZe7453878 = HiFvNrUyXpMKrtBZe50501589;     HiFvNrUyXpMKrtBZe50501589 = HiFvNrUyXpMKrtBZe95117743;     HiFvNrUyXpMKrtBZe95117743 = HiFvNrUyXpMKrtBZe80024591;     HiFvNrUyXpMKrtBZe80024591 = HiFvNrUyXpMKrtBZe6661424;     HiFvNrUyXpMKrtBZe6661424 = HiFvNrUyXpMKrtBZe93592232;     HiFvNrUyXpMKrtBZe93592232 = HiFvNrUyXpMKrtBZe87643830;     HiFvNrUyXpMKrtBZe87643830 = HiFvNrUyXpMKrtBZe25848173;     HiFvNrUyXpMKrtBZe25848173 = HiFvNrUyXpMKrtBZe11657282;     HiFvNrUyXpMKrtBZe11657282 = HiFvNrUyXpMKrtBZe33616079;     HiFvNrUyXpMKrtBZe33616079 = HiFvNrUyXpMKrtBZe4596526;     HiFvNrUyXpMKrtBZe4596526 = HiFvNrUyXpMKrtBZe78435873;     HiFvNrUyXpMKrtBZe78435873 = HiFvNrUyXpMKrtBZe74362186;     HiFvNrUyXpMKrtBZe74362186 = HiFvNrUyXpMKrtBZe74629627;     HiFvNrUyXpMKrtBZe74629627 = HiFvNrUyXpMKrtBZe12342027;     HiFvNrUyXpMKrtBZe12342027 = HiFvNrUyXpMKrtBZe91942416;     HiFvNrUyXpMKrtBZe91942416 = HiFvNrUyXpMKrtBZe81464109;     HiFvNrUyXpMKrtBZe81464109 = HiFvNrUyXpMKrtBZe32160034;     HiFvNrUyXpMKrtBZe32160034 = HiFvNrUyXpMKrtBZe87166788;     HiFvNrUyXpMKrtBZe87166788 = HiFvNrUyXpMKrtBZe77977825;     HiFvNrUyXpMKrtBZe77977825 = HiFvNrUyXpMKrtBZe96386084;     HiFvNrUyXpMKrtBZe96386084 = HiFvNrUyXpMKrtBZe81490866;     HiFvNrUyXpMKrtBZe81490866 = HiFvNrUyXpMKrtBZe35552985;     HiFvNrUyXpMKrtBZe35552985 = HiFvNrUyXpMKrtBZe69544919;     HiFvNrUyXpMKrtBZe69544919 = HiFvNrUyXpMKrtBZe99727366;     HiFvNrUyXpMKrtBZe99727366 = HiFvNrUyXpMKrtBZe96079049;     HiFvNrUyXpMKrtBZe96079049 = HiFvNrUyXpMKrtBZe14900196;     HiFvNrUyXpMKrtBZe14900196 = HiFvNrUyXpMKrtBZe28388945;     HiFvNrUyXpMKrtBZe28388945 = HiFvNrUyXpMKrtBZe98376285;     HiFvNrUyXpMKrtBZe98376285 = HiFvNrUyXpMKrtBZe23333806;     HiFvNrUyXpMKrtBZe23333806 = HiFvNrUyXpMKrtBZe78537591;     HiFvNrUyXpMKrtBZe78537591 = HiFvNrUyXpMKrtBZe99005892;     HiFvNrUyXpMKrtBZe99005892 = HiFvNrUyXpMKrtBZe35911207;     HiFvNrUyXpMKrtBZe35911207 = HiFvNrUyXpMKrtBZe52747894;     HiFvNrUyXpMKrtBZe52747894 = HiFvNrUyXpMKrtBZe60459895;     HiFvNrUyXpMKrtBZe60459895 = HiFvNrUyXpMKrtBZe54852508;     HiFvNrUyXpMKrtBZe54852508 = HiFvNrUyXpMKrtBZe32061670;     HiFvNrUyXpMKrtBZe32061670 = HiFvNrUyXpMKrtBZe19986705;     HiFvNrUyXpMKrtBZe19986705 = HiFvNrUyXpMKrtBZe58407173;     HiFvNrUyXpMKrtBZe58407173 = HiFvNrUyXpMKrtBZe52939834;     HiFvNrUyXpMKrtBZe52939834 = HiFvNrUyXpMKrtBZe44995100;     HiFvNrUyXpMKrtBZe44995100 = HiFvNrUyXpMKrtBZe21034574;     HiFvNrUyXpMKrtBZe21034574 = HiFvNrUyXpMKrtBZe16482881;     HiFvNrUyXpMKrtBZe16482881 = HiFvNrUyXpMKrtBZe81552050;     HiFvNrUyXpMKrtBZe81552050 = HiFvNrUyXpMKrtBZe88118096;     HiFvNrUyXpMKrtBZe88118096 = HiFvNrUyXpMKrtBZe16242961;     HiFvNrUyXpMKrtBZe16242961 = HiFvNrUyXpMKrtBZe11568300;     HiFvNrUyXpMKrtBZe11568300 = HiFvNrUyXpMKrtBZe86131945;     HiFvNrUyXpMKrtBZe86131945 = HiFvNrUyXpMKrtBZe86884688;     HiFvNrUyXpMKrtBZe86884688 = HiFvNrUyXpMKrtBZe98766114;     HiFvNrUyXpMKrtBZe98766114 = HiFvNrUyXpMKrtBZe51569728;     HiFvNrUyXpMKrtBZe51569728 = HiFvNrUyXpMKrtBZe8783542;     HiFvNrUyXpMKrtBZe8783542 = HiFvNrUyXpMKrtBZe19134031;     HiFvNrUyXpMKrtBZe19134031 = HiFvNrUyXpMKrtBZe4032089;     HiFvNrUyXpMKrtBZe4032089 = HiFvNrUyXpMKrtBZe64095901;     HiFvNrUyXpMKrtBZe64095901 = HiFvNrUyXpMKrtBZe25963011;     HiFvNrUyXpMKrtBZe25963011 = HiFvNrUyXpMKrtBZe14948605;     HiFvNrUyXpMKrtBZe14948605 = HiFvNrUyXpMKrtBZe25572824;     HiFvNrUyXpMKrtBZe25572824 = HiFvNrUyXpMKrtBZe80297225;     HiFvNrUyXpMKrtBZe80297225 = HiFvNrUyXpMKrtBZe10582375;     HiFvNrUyXpMKrtBZe10582375 = HiFvNrUyXpMKrtBZe78692036;     HiFvNrUyXpMKrtBZe78692036 = HiFvNrUyXpMKrtBZe59254885;     HiFvNrUyXpMKrtBZe59254885 = HiFvNrUyXpMKrtBZe27471888;     HiFvNrUyXpMKrtBZe27471888 = HiFvNrUyXpMKrtBZe88323476;     HiFvNrUyXpMKrtBZe88323476 = HiFvNrUyXpMKrtBZe55078487;     HiFvNrUyXpMKrtBZe55078487 = HiFvNrUyXpMKrtBZe5590634;     HiFvNrUyXpMKrtBZe5590634 = HiFvNrUyXpMKrtBZe42524666;     HiFvNrUyXpMKrtBZe42524666 = HiFvNrUyXpMKrtBZe21614293;     HiFvNrUyXpMKrtBZe21614293 = HiFvNrUyXpMKrtBZe14169733;     HiFvNrUyXpMKrtBZe14169733 = HiFvNrUyXpMKrtBZe57489519;     HiFvNrUyXpMKrtBZe57489519 = HiFvNrUyXpMKrtBZe59880746;     HiFvNrUyXpMKrtBZe59880746 = HiFvNrUyXpMKrtBZe61477405;     HiFvNrUyXpMKrtBZe61477405 = HiFvNrUyXpMKrtBZe73752860;     HiFvNrUyXpMKrtBZe73752860 = HiFvNrUyXpMKrtBZe34226954;     HiFvNrUyXpMKrtBZe34226954 = HiFvNrUyXpMKrtBZe32982725;     HiFvNrUyXpMKrtBZe32982725 = HiFvNrUyXpMKrtBZe75351510;     HiFvNrUyXpMKrtBZe75351510 = HiFvNrUyXpMKrtBZe65007986;     HiFvNrUyXpMKrtBZe65007986 = HiFvNrUyXpMKrtBZe54000934;     HiFvNrUyXpMKrtBZe54000934 = HiFvNrUyXpMKrtBZe81426823;     HiFvNrUyXpMKrtBZe81426823 = HiFvNrUyXpMKrtBZe83484406;     HiFvNrUyXpMKrtBZe83484406 = HiFvNrUyXpMKrtBZe84510749;     HiFvNrUyXpMKrtBZe84510749 = HiFvNrUyXpMKrtBZe28768250;     HiFvNrUyXpMKrtBZe28768250 = HiFvNrUyXpMKrtBZe41504257;     HiFvNrUyXpMKrtBZe41504257 = HiFvNrUyXpMKrtBZe99610170;     HiFvNrUyXpMKrtBZe99610170 = HiFvNrUyXpMKrtBZe71764077;     HiFvNrUyXpMKrtBZe71764077 = HiFvNrUyXpMKrtBZe69754049;     HiFvNrUyXpMKrtBZe69754049 = HiFvNrUyXpMKrtBZe79871862;     HiFvNrUyXpMKrtBZe79871862 = HiFvNrUyXpMKrtBZe31879118;     HiFvNrUyXpMKrtBZe31879118 = HiFvNrUyXpMKrtBZe88651992;     HiFvNrUyXpMKrtBZe88651992 = HiFvNrUyXpMKrtBZe34496884;     HiFvNrUyXpMKrtBZe34496884 = HiFvNrUyXpMKrtBZe94678833;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tvwtIYMMzDtDKydA29359596() {     double vdcNdjKNzyryxXYVN17065951 = -195461034;    double vdcNdjKNzyryxXYVN70277987 = -877165751;    double vdcNdjKNzyryxXYVN89530851 = -803069572;    double vdcNdjKNzyryxXYVN14233404 = -480334746;    double vdcNdjKNzyryxXYVN88882784 = -774331227;    double vdcNdjKNzyryxXYVN47916955 = 82533975;    double vdcNdjKNzyryxXYVN33099314 = -892797755;    double vdcNdjKNzyryxXYVN81934677 = -60843207;    double vdcNdjKNzyryxXYVN79173044 = -592986842;    double vdcNdjKNzyryxXYVN52245571 = -934939243;    double vdcNdjKNzyryxXYVN44995737 = -185723168;    double vdcNdjKNzyryxXYVN6404539 = -261251210;    double vdcNdjKNzyryxXYVN62125924 = -827132978;    double vdcNdjKNzyryxXYVN22639344 = -104361900;    double vdcNdjKNzyryxXYVN43396701 = -199738668;    double vdcNdjKNzyryxXYVN37848145 = -390651984;    double vdcNdjKNzyryxXYVN37467182 = -891942329;    double vdcNdjKNzyryxXYVN31824589 = -517764854;    double vdcNdjKNzyryxXYVN52938913 = -353559127;    double vdcNdjKNzyryxXYVN12132362 = -276446150;    double vdcNdjKNzyryxXYVN5479633 = -320982043;    double vdcNdjKNzyryxXYVN42251092 = -834341930;    double vdcNdjKNzyryxXYVN77655509 = -690474588;    double vdcNdjKNzyryxXYVN1129415 = -499051845;    double vdcNdjKNzyryxXYVN47882274 = -334771251;    double vdcNdjKNzyryxXYVN80245410 = -894732572;    double vdcNdjKNzyryxXYVN28553530 = -562329574;    double vdcNdjKNzyryxXYVN38071484 = -481975570;    double vdcNdjKNzyryxXYVN94059349 = -62103721;    double vdcNdjKNzyryxXYVN87135004 = 18272948;    double vdcNdjKNzyryxXYVN40676890 = -798409673;    double vdcNdjKNzyryxXYVN39032058 = -434970327;    double vdcNdjKNzyryxXYVN16591655 = -66758199;    double vdcNdjKNzyryxXYVN28933607 = -447725519;    double vdcNdjKNzyryxXYVN75956731 = -870314640;    double vdcNdjKNzyryxXYVN37771919 = -728084472;    double vdcNdjKNzyryxXYVN8339908 = -493280773;    double vdcNdjKNzyryxXYVN82015045 = -582416793;    double vdcNdjKNzyryxXYVN46684416 = -741275311;    double vdcNdjKNzyryxXYVN43519520 = -616556405;    double vdcNdjKNzyryxXYVN7294963 = -603500604;    double vdcNdjKNzyryxXYVN87514283 = -352574278;    double vdcNdjKNzyryxXYVN41461399 = -584729512;    double vdcNdjKNzyryxXYVN52192013 = 90529318;    double vdcNdjKNzyryxXYVN28443162 = -701716246;    double vdcNdjKNzyryxXYVN62952789 = -142104934;    double vdcNdjKNzyryxXYVN44359067 = -953543844;    double vdcNdjKNzyryxXYVN64585379 = -623335027;    double vdcNdjKNzyryxXYVN52867078 = -281743865;    double vdcNdjKNzyryxXYVN50023580 = -243630972;    double vdcNdjKNzyryxXYVN53864316 = -973119223;    double vdcNdjKNzyryxXYVN94885831 = -386206635;    double vdcNdjKNzyryxXYVN94797283 = -2783929;    double vdcNdjKNzyryxXYVN41838596 = -176697194;    double vdcNdjKNzyryxXYVN23879964 = -331743658;    double vdcNdjKNzyryxXYVN74814859 = -361119105;    double vdcNdjKNzyryxXYVN92622477 = -86691163;    double vdcNdjKNzyryxXYVN88401436 = -204017727;    double vdcNdjKNzyryxXYVN66351130 = -45563495;    double vdcNdjKNzyryxXYVN8637374 = -879598655;    double vdcNdjKNzyryxXYVN19363425 = -355136451;    double vdcNdjKNzyryxXYVN95027830 = -310822185;    double vdcNdjKNzyryxXYVN87875328 = -998739487;    double vdcNdjKNzyryxXYVN92038040 = -511259790;    double vdcNdjKNzyryxXYVN11568681 = -36529570;    double vdcNdjKNzyryxXYVN5963680 = -750752841;    double vdcNdjKNzyryxXYVN89812883 = -94493011;    double vdcNdjKNzyryxXYVN33192317 = -279407459;    double vdcNdjKNzyryxXYVN46682613 = -234047261;    double vdcNdjKNzyryxXYVN5624783 = -471654196;    double vdcNdjKNzyryxXYVN29508237 = -897371212;    double vdcNdjKNzyryxXYVN55452136 = -209525536;    double vdcNdjKNzyryxXYVN85140172 = -776489543;    double vdcNdjKNzyryxXYVN9419394 = -737002722;    double vdcNdjKNzyryxXYVN4837400 = -672945547;    double vdcNdjKNzyryxXYVN17965349 = -968407766;    double vdcNdjKNzyryxXYVN789693 = -149612418;    double vdcNdjKNzyryxXYVN25463496 = -681003907;    double vdcNdjKNzyryxXYVN72686253 = -797335600;    double vdcNdjKNzyryxXYVN84929484 = -92666318;    double vdcNdjKNzyryxXYVN35886344 = -941188728;    double vdcNdjKNzyryxXYVN63968150 = -938994548;    double vdcNdjKNzyryxXYVN85204405 = -100231705;    double vdcNdjKNzyryxXYVN44035769 = -818472750;    double vdcNdjKNzyryxXYVN33270688 = -8607830;    double vdcNdjKNzyryxXYVN45791058 = -312203038;    double vdcNdjKNzyryxXYVN44234774 = -332186399;    double vdcNdjKNzyryxXYVN74753059 = -890061005;    double vdcNdjKNzyryxXYVN5053644 = -15981862;    double vdcNdjKNzyryxXYVN1141872 = -409195535;    double vdcNdjKNzyryxXYVN45149442 = -541393310;    double vdcNdjKNzyryxXYVN19938471 = -189263046;    double vdcNdjKNzyryxXYVN15663916 = -436853299;    double vdcNdjKNzyryxXYVN38047043 = -861676657;    double vdcNdjKNzyryxXYVN24156095 = -161419954;    double vdcNdjKNzyryxXYVN12267132 = -192678419;    double vdcNdjKNzyryxXYVN99638955 = -353834791;    double vdcNdjKNzyryxXYVN49423359 = 26530278;    double vdcNdjKNzyryxXYVN40623332 = -872941112;    double vdcNdjKNzyryxXYVN22479482 = -195461034;     vdcNdjKNzyryxXYVN17065951 = vdcNdjKNzyryxXYVN70277987;     vdcNdjKNzyryxXYVN70277987 = vdcNdjKNzyryxXYVN89530851;     vdcNdjKNzyryxXYVN89530851 = vdcNdjKNzyryxXYVN14233404;     vdcNdjKNzyryxXYVN14233404 = vdcNdjKNzyryxXYVN88882784;     vdcNdjKNzyryxXYVN88882784 = vdcNdjKNzyryxXYVN47916955;     vdcNdjKNzyryxXYVN47916955 = vdcNdjKNzyryxXYVN33099314;     vdcNdjKNzyryxXYVN33099314 = vdcNdjKNzyryxXYVN81934677;     vdcNdjKNzyryxXYVN81934677 = vdcNdjKNzyryxXYVN79173044;     vdcNdjKNzyryxXYVN79173044 = vdcNdjKNzyryxXYVN52245571;     vdcNdjKNzyryxXYVN52245571 = vdcNdjKNzyryxXYVN44995737;     vdcNdjKNzyryxXYVN44995737 = vdcNdjKNzyryxXYVN6404539;     vdcNdjKNzyryxXYVN6404539 = vdcNdjKNzyryxXYVN62125924;     vdcNdjKNzyryxXYVN62125924 = vdcNdjKNzyryxXYVN22639344;     vdcNdjKNzyryxXYVN22639344 = vdcNdjKNzyryxXYVN43396701;     vdcNdjKNzyryxXYVN43396701 = vdcNdjKNzyryxXYVN37848145;     vdcNdjKNzyryxXYVN37848145 = vdcNdjKNzyryxXYVN37467182;     vdcNdjKNzyryxXYVN37467182 = vdcNdjKNzyryxXYVN31824589;     vdcNdjKNzyryxXYVN31824589 = vdcNdjKNzyryxXYVN52938913;     vdcNdjKNzyryxXYVN52938913 = vdcNdjKNzyryxXYVN12132362;     vdcNdjKNzyryxXYVN12132362 = vdcNdjKNzyryxXYVN5479633;     vdcNdjKNzyryxXYVN5479633 = vdcNdjKNzyryxXYVN42251092;     vdcNdjKNzyryxXYVN42251092 = vdcNdjKNzyryxXYVN77655509;     vdcNdjKNzyryxXYVN77655509 = vdcNdjKNzyryxXYVN1129415;     vdcNdjKNzyryxXYVN1129415 = vdcNdjKNzyryxXYVN47882274;     vdcNdjKNzyryxXYVN47882274 = vdcNdjKNzyryxXYVN80245410;     vdcNdjKNzyryxXYVN80245410 = vdcNdjKNzyryxXYVN28553530;     vdcNdjKNzyryxXYVN28553530 = vdcNdjKNzyryxXYVN38071484;     vdcNdjKNzyryxXYVN38071484 = vdcNdjKNzyryxXYVN94059349;     vdcNdjKNzyryxXYVN94059349 = vdcNdjKNzyryxXYVN87135004;     vdcNdjKNzyryxXYVN87135004 = vdcNdjKNzyryxXYVN40676890;     vdcNdjKNzyryxXYVN40676890 = vdcNdjKNzyryxXYVN39032058;     vdcNdjKNzyryxXYVN39032058 = vdcNdjKNzyryxXYVN16591655;     vdcNdjKNzyryxXYVN16591655 = vdcNdjKNzyryxXYVN28933607;     vdcNdjKNzyryxXYVN28933607 = vdcNdjKNzyryxXYVN75956731;     vdcNdjKNzyryxXYVN75956731 = vdcNdjKNzyryxXYVN37771919;     vdcNdjKNzyryxXYVN37771919 = vdcNdjKNzyryxXYVN8339908;     vdcNdjKNzyryxXYVN8339908 = vdcNdjKNzyryxXYVN82015045;     vdcNdjKNzyryxXYVN82015045 = vdcNdjKNzyryxXYVN46684416;     vdcNdjKNzyryxXYVN46684416 = vdcNdjKNzyryxXYVN43519520;     vdcNdjKNzyryxXYVN43519520 = vdcNdjKNzyryxXYVN7294963;     vdcNdjKNzyryxXYVN7294963 = vdcNdjKNzyryxXYVN87514283;     vdcNdjKNzyryxXYVN87514283 = vdcNdjKNzyryxXYVN41461399;     vdcNdjKNzyryxXYVN41461399 = vdcNdjKNzyryxXYVN52192013;     vdcNdjKNzyryxXYVN52192013 = vdcNdjKNzyryxXYVN28443162;     vdcNdjKNzyryxXYVN28443162 = vdcNdjKNzyryxXYVN62952789;     vdcNdjKNzyryxXYVN62952789 = vdcNdjKNzyryxXYVN44359067;     vdcNdjKNzyryxXYVN44359067 = vdcNdjKNzyryxXYVN64585379;     vdcNdjKNzyryxXYVN64585379 = vdcNdjKNzyryxXYVN52867078;     vdcNdjKNzyryxXYVN52867078 = vdcNdjKNzyryxXYVN50023580;     vdcNdjKNzyryxXYVN50023580 = vdcNdjKNzyryxXYVN53864316;     vdcNdjKNzyryxXYVN53864316 = vdcNdjKNzyryxXYVN94885831;     vdcNdjKNzyryxXYVN94885831 = vdcNdjKNzyryxXYVN94797283;     vdcNdjKNzyryxXYVN94797283 = vdcNdjKNzyryxXYVN41838596;     vdcNdjKNzyryxXYVN41838596 = vdcNdjKNzyryxXYVN23879964;     vdcNdjKNzyryxXYVN23879964 = vdcNdjKNzyryxXYVN74814859;     vdcNdjKNzyryxXYVN74814859 = vdcNdjKNzyryxXYVN92622477;     vdcNdjKNzyryxXYVN92622477 = vdcNdjKNzyryxXYVN88401436;     vdcNdjKNzyryxXYVN88401436 = vdcNdjKNzyryxXYVN66351130;     vdcNdjKNzyryxXYVN66351130 = vdcNdjKNzyryxXYVN8637374;     vdcNdjKNzyryxXYVN8637374 = vdcNdjKNzyryxXYVN19363425;     vdcNdjKNzyryxXYVN19363425 = vdcNdjKNzyryxXYVN95027830;     vdcNdjKNzyryxXYVN95027830 = vdcNdjKNzyryxXYVN87875328;     vdcNdjKNzyryxXYVN87875328 = vdcNdjKNzyryxXYVN92038040;     vdcNdjKNzyryxXYVN92038040 = vdcNdjKNzyryxXYVN11568681;     vdcNdjKNzyryxXYVN11568681 = vdcNdjKNzyryxXYVN5963680;     vdcNdjKNzyryxXYVN5963680 = vdcNdjKNzyryxXYVN89812883;     vdcNdjKNzyryxXYVN89812883 = vdcNdjKNzyryxXYVN33192317;     vdcNdjKNzyryxXYVN33192317 = vdcNdjKNzyryxXYVN46682613;     vdcNdjKNzyryxXYVN46682613 = vdcNdjKNzyryxXYVN5624783;     vdcNdjKNzyryxXYVN5624783 = vdcNdjKNzyryxXYVN29508237;     vdcNdjKNzyryxXYVN29508237 = vdcNdjKNzyryxXYVN55452136;     vdcNdjKNzyryxXYVN55452136 = vdcNdjKNzyryxXYVN85140172;     vdcNdjKNzyryxXYVN85140172 = vdcNdjKNzyryxXYVN9419394;     vdcNdjKNzyryxXYVN9419394 = vdcNdjKNzyryxXYVN4837400;     vdcNdjKNzyryxXYVN4837400 = vdcNdjKNzyryxXYVN17965349;     vdcNdjKNzyryxXYVN17965349 = vdcNdjKNzyryxXYVN789693;     vdcNdjKNzyryxXYVN789693 = vdcNdjKNzyryxXYVN25463496;     vdcNdjKNzyryxXYVN25463496 = vdcNdjKNzyryxXYVN72686253;     vdcNdjKNzyryxXYVN72686253 = vdcNdjKNzyryxXYVN84929484;     vdcNdjKNzyryxXYVN84929484 = vdcNdjKNzyryxXYVN35886344;     vdcNdjKNzyryxXYVN35886344 = vdcNdjKNzyryxXYVN63968150;     vdcNdjKNzyryxXYVN63968150 = vdcNdjKNzyryxXYVN85204405;     vdcNdjKNzyryxXYVN85204405 = vdcNdjKNzyryxXYVN44035769;     vdcNdjKNzyryxXYVN44035769 = vdcNdjKNzyryxXYVN33270688;     vdcNdjKNzyryxXYVN33270688 = vdcNdjKNzyryxXYVN45791058;     vdcNdjKNzyryxXYVN45791058 = vdcNdjKNzyryxXYVN44234774;     vdcNdjKNzyryxXYVN44234774 = vdcNdjKNzyryxXYVN74753059;     vdcNdjKNzyryxXYVN74753059 = vdcNdjKNzyryxXYVN5053644;     vdcNdjKNzyryxXYVN5053644 = vdcNdjKNzyryxXYVN1141872;     vdcNdjKNzyryxXYVN1141872 = vdcNdjKNzyryxXYVN45149442;     vdcNdjKNzyryxXYVN45149442 = vdcNdjKNzyryxXYVN19938471;     vdcNdjKNzyryxXYVN19938471 = vdcNdjKNzyryxXYVN15663916;     vdcNdjKNzyryxXYVN15663916 = vdcNdjKNzyryxXYVN38047043;     vdcNdjKNzyryxXYVN38047043 = vdcNdjKNzyryxXYVN24156095;     vdcNdjKNzyryxXYVN24156095 = vdcNdjKNzyryxXYVN12267132;     vdcNdjKNzyryxXYVN12267132 = vdcNdjKNzyryxXYVN99638955;     vdcNdjKNzyryxXYVN99638955 = vdcNdjKNzyryxXYVN49423359;     vdcNdjKNzyryxXYVN49423359 = vdcNdjKNzyryxXYVN40623332;     vdcNdjKNzyryxXYVN40623332 = vdcNdjKNzyryxXYVN22479482;     vdcNdjKNzyryxXYVN22479482 = vdcNdjKNzyryxXYVN17065951;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void aRtLaqZesalmiAwm96651195() {     double ZxUiwCljSwhvJzAwK52524919 = -661737934;    double ZxUiwCljSwhvJzAwK83550065 = -757717357;    double ZxUiwCljSwhvJzAwK26917328 = -468736647;    double ZxUiwCljSwhvJzAwK86456758 = -301485817;    double ZxUiwCljSwhvJzAwK45168054 = -879930072;    double ZxUiwCljSwhvJzAwK84139826 = -179813920;    double ZxUiwCljSwhvJzAwK43756721 = -737523489;    double ZxUiwCljSwhvJzAwK44066239 = -805077348;    double ZxUiwCljSwhvJzAwK80690650 = 18681751;    double ZxUiwCljSwhvJzAwK67148283 = -599626241;    double ZxUiwCljSwhvJzAwK83177472 = 12269103;    double ZxUiwCljSwhvJzAwK6972687 = -125371981;    double ZxUiwCljSwhvJzAwK360570 = -279145483;    double ZxUiwCljSwhvJzAwK75780207 = -80725309;    double ZxUiwCljSwhvJzAwK1591218 = -703163683;    double ZxUiwCljSwhvJzAwK21389446 = -26463591;    double ZxUiwCljSwhvJzAwK93303117 = -318916882;    double ZxUiwCljSwhvJzAwK31584781 = -591192922;    double ZxUiwCljSwhvJzAwK53489243 = -670122548;    double ZxUiwCljSwhvJzAwK5969581 = -703007216;    double ZxUiwCljSwhvJzAwK18566496 = 21743269;    double ZxUiwCljSwhvJzAwK45672616 = -920978224;    double ZxUiwCljSwhvJzAwK48180889 = -328106773;    double ZxUiwCljSwhvJzAwK94922359 = 90123551;    double ZxUiwCljSwhvJzAwK55900225 = -358756450;    double ZxUiwCljSwhvJzAwK90943582 = -583309379;    double ZxUiwCljSwhvJzAwK88974104 = -368613715;    double ZxUiwCljSwhvJzAwK78799909 = -651747745;    double ZxUiwCljSwhvJzAwK40727564 = -636198524;    double ZxUiwCljSwhvJzAwK92668415 = -16234223;    double ZxUiwCljSwhvJzAwK59260121 = -250390160;    double ZxUiwCljSwhvJzAwK76855076 = -207941742;    double ZxUiwCljSwhvJzAwK28809666 = -214354176;    double ZxUiwCljSwhvJzAwK58682033 = -512419333;    double ZxUiwCljSwhvJzAwK41765185 = -821414890;    double ZxUiwCljSwhvJzAwK89332762 = -942649778;    double ZxUiwCljSwhvJzAwK57751724 = -357976813;    double ZxUiwCljSwhvJzAwK33230449 = -565072374;    double ZxUiwCljSwhvJzAwK77074978 = -392151588;    double ZxUiwCljSwhvJzAwK7791053 = -81818951;    double ZxUiwCljSwhvJzAwK40746570 = -372699518;    double ZxUiwCljSwhvJzAwK76879410 = 52880432;    double ZxUiwCljSwhvJzAwK30844778 = -137910298;    double ZxUiwCljSwhvJzAwK92323351 = -513803092;    double ZxUiwCljSwhvJzAwK28459620 = -528227407;    double ZxUiwCljSwhvJzAwK8914118 = -181028566;    double ZxUiwCljSwhvJzAwK63704231 = -337530205;    double ZxUiwCljSwhvJzAwK82311691 = -789861620;    double ZxUiwCljSwhvJzAwK75759744 = 54519254;    double ZxUiwCljSwhvJzAwK74553169 = -734657788;    double ZxUiwCljSwhvJzAwK34635040 = -847395780;    double ZxUiwCljSwhvJzAwK78371303 = -599858982;    double ZxUiwCljSwhvJzAwK63418432 = -858829483;    double ZxUiwCljSwhvJzAwK57830443 = -368205694;    double ZxUiwCljSwhvJzAwK78084699 = -678233804;    double ZxUiwCljSwhvJzAwK6852303 = -740759711;    double ZxUiwCljSwhvJzAwK35369176 = -329610585;    double ZxUiwCljSwhvJzAwK31994968 = -458860198;    double ZxUiwCljSwhvJzAwK30556534 = -942729368;    double ZxUiwCljSwhvJzAwK54224471 = -196620694;    double ZxUiwCljSwhvJzAwK95165722 = -811200206;    double ZxUiwCljSwhvJzAwK64956812 = 14224255;    double ZxUiwCljSwhvJzAwK3338675 = -68878825;    double ZxUiwCljSwhvJzAwK88022234 = -965084027;    double ZxUiwCljSwhvJzAwK7888162 = -249236081;    double ZxUiwCljSwhvJzAwK6322396 = -779789156;    double ZxUiwCljSwhvJzAwK78163021 = -911017805;    double ZxUiwCljSwhvJzAwK41678536 = -766726150;    double ZxUiwCljSwhvJzAwK34015023 = -259310420;    double ZxUiwCljSwhvJzAwK12258455 = -760513905;    double ZxUiwCljSwhvJzAwK63637721 = -668486779;    double ZxUiwCljSwhvJzAwK60072669 = -753844508;    double ZxUiwCljSwhvJzAwK54509803 = -99041335;    double ZxUiwCljSwhvJzAwK45698191 = -488303598;    double ZxUiwCljSwhvJzAwK65223011 = -230307699;    double ZxUiwCljSwhvJzAwK41687085 = 68862837;    double ZxUiwCljSwhvJzAwK14827839 = -683067926;    double ZxUiwCljSwhvJzAwK55857538 = -814303681;    double ZxUiwCljSwhvJzAwK66462739 = -381649043;    double ZxUiwCljSwhvJzAwK46986107 = -77727884;    double ZxUiwCljSwhvJzAwK27239352 = -145779174;    double ZxUiwCljSwhvJzAwK6662413 = -578752095;    double ZxUiwCljSwhvJzAwK3040165 = -606267000;    double ZxUiwCljSwhvJzAwK66174395 = -901540737;    double ZxUiwCljSwhvJzAwK58033375 = -168838443;    double ZxUiwCljSwhvJzAwK80888817 = -650531178;    double ZxUiwCljSwhvJzAwK13436644 = -349112259;    double ZxUiwCljSwhvJzAwK70979223 = -846148483;    double ZxUiwCljSwhvJzAwK80597333 = -834185530;    double ZxUiwCljSwhvJzAwK34912882 = 19344820;    double ZxUiwCljSwhvJzAwK53963587 = -513039193;    double ZxUiwCljSwhvJzAwK25756756 = -899116616;    double ZxUiwCljSwhvJzAwK2673916 = -622343007;    double ZxUiwCljSwhvJzAwK22850507 = -95530895;    double ZxUiwCljSwhvJzAwK12625330 = -270618746;    double ZxUiwCljSwhvJzAwK75789758 = -286923773;    double ZxUiwCljSwhvJzAwK73540736 = -878240744;    double ZxUiwCljSwhvJzAwK42822543 = -172826271;    double ZxUiwCljSwhvJzAwK84435190 = -164567011;    double ZxUiwCljSwhvJzAwK22137224 = -661737934;     ZxUiwCljSwhvJzAwK52524919 = ZxUiwCljSwhvJzAwK83550065;     ZxUiwCljSwhvJzAwK83550065 = ZxUiwCljSwhvJzAwK26917328;     ZxUiwCljSwhvJzAwK26917328 = ZxUiwCljSwhvJzAwK86456758;     ZxUiwCljSwhvJzAwK86456758 = ZxUiwCljSwhvJzAwK45168054;     ZxUiwCljSwhvJzAwK45168054 = ZxUiwCljSwhvJzAwK84139826;     ZxUiwCljSwhvJzAwK84139826 = ZxUiwCljSwhvJzAwK43756721;     ZxUiwCljSwhvJzAwK43756721 = ZxUiwCljSwhvJzAwK44066239;     ZxUiwCljSwhvJzAwK44066239 = ZxUiwCljSwhvJzAwK80690650;     ZxUiwCljSwhvJzAwK80690650 = ZxUiwCljSwhvJzAwK67148283;     ZxUiwCljSwhvJzAwK67148283 = ZxUiwCljSwhvJzAwK83177472;     ZxUiwCljSwhvJzAwK83177472 = ZxUiwCljSwhvJzAwK6972687;     ZxUiwCljSwhvJzAwK6972687 = ZxUiwCljSwhvJzAwK360570;     ZxUiwCljSwhvJzAwK360570 = ZxUiwCljSwhvJzAwK75780207;     ZxUiwCljSwhvJzAwK75780207 = ZxUiwCljSwhvJzAwK1591218;     ZxUiwCljSwhvJzAwK1591218 = ZxUiwCljSwhvJzAwK21389446;     ZxUiwCljSwhvJzAwK21389446 = ZxUiwCljSwhvJzAwK93303117;     ZxUiwCljSwhvJzAwK93303117 = ZxUiwCljSwhvJzAwK31584781;     ZxUiwCljSwhvJzAwK31584781 = ZxUiwCljSwhvJzAwK53489243;     ZxUiwCljSwhvJzAwK53489243 = ZxUiwCljSwhvJzAwK5969581;     ZxUiwCljSwhvJzAwK5969581 = ZxUiwCljSwhvJzAwK18566496;     ZxUiwCljSwhvJzAwK18566496 = ZxUiwCljSwhvJzAwK45672616;     ZxUiwCljSwhvJzAwK45672616 = ZxUiwCljSwhvJzAwK48180889;     ZxUiwCljSwhvJzAwK48180889 = ZxUiwCljSwhvJzAwK94922359;     ZxUiwCljSwhvJzAwK94922359 = ZxUiwCljSwhvJzAwK55900225;     ZxUiwCljSwhvJzAwK55900225 = ZxUiwCljSwhvJzAwK90943582;     ZxUiwCljSwhvJzAwK90943582 = ZxUiwCljSwhvJzAwK88974104;     ZxUiwCljSwhvJzAwK88974104 = ZxUiwCljSwhvJzAwK78799909;     ZxUiwCljSwhvJzAwK78799909 = ZxUiwCljSwhvJzAwK40727564;     ZxUiwCljSwhvJzAwK40727564 = ZxUiwCljSwhvJzAwK92668415;     ZxUiwCljSwhvJzAwK92668415 = ZxUiwCljSwhvJzAwK59260121;     ZxUiwCljSwhvJzAwK59260121 = ZxUiwCljSwhvJzAwK76855076;     ZxUiwCljSwhvJzAwK76855076 = ZxUiwCljSwhvJzAwK28809666;     ZxUiwCljSwhvJzAwK28809666 = ZxUiwCljSwhvJzAwK58682033;     ZxUiwCljSwhvJzAwK58682033 = ZxUiwCljSwhvJzAwK41765185;     ZxUiwCljSwhvJzAwK41765185 = ZxUiwCljSwhvJzAwK89332762;     ZxUiwCljSwhvJzAwK89332762 = ZxUiwCljSwhvJzAwK57751724;     ZxUiwCljSwhvJzAwK57751724 = ZxUiwCljSwhvJzAwK33230449;     ZxUiwCljSwhvJzAwK33230449 = ZxUiwCljSwhvJzAwK77074978;     ZxUiwCljSwhvJzAwK77074978 = ZxUiwCljSwhvJzAwK7791053;     ZxUiwCljSwhvJzAwK7791053 = ZxUiwCljSwhvJzAwK40746570;     ZxUiwCljSwhvJzAwK40746570 = ZxUiwCljSwhvJzAwK76879410;     ZxUiwCljSwhvJzAwK76879410 = ZxUiwCljSwhvJzAwK30844778;     ZxUiwCljSwhvJzAwK30844778 = ZxUiwCljSwhvJzAwK92323351;     ZxUiwCljSwhvJzAwK92323351 = ZxUiwCljSwhvJzAwK28459620;     ZxUiwCljSwhvJzAwK28459620 = ZxUiwCljSwhvJzAwK8914118;     ZxUiwCljSwhvJzAwK8914118 = ZxUiwCljSwhvJzAwK63704231;     ZxUiwCljSwhvJzAwK63704231 = ZxUiwCljSwhvJzAwK82311691;     ZxUiwCljSwhvJzAwK82311691 = ZxUiwCljSwhvJzAwK75759744;     ZxUiwCljSwhvJzAwK75759744 = ZxUiwCljSwhvJzAwK74553169;     ZxUiwCljSwhvJzAwK74553169 = ZxUiwCljSwhvJzAwK34635040;     ZxUiwCljSwhvJzAwK34635040 = ZxUiwCljSwhvJzAwK78371303;     ZxUiwCljSwhvJzAwK78371303 = ZxUiwCljSwhvJzAwK63418432;     ZxUiwCljSwhvJzAwK63418432 = ZxUiwCljSwhvJzAwK57830443;     ZxUiwCljSwhvJzAwK57830443 = ZxUiwCljSwhvJzAwK78084699;     ZxUiwCljSwhvJzAwK78084699 = ZxUiwCljSwhvJzAwK6852303;     ZxUiwCljSwhvJzAwK6852303 = ZxUiwCljSwhvJzAwK35369176;     ZxUiwCljSwhvJzAwK35369176 = ZxUiwCljSwhvJzAwK31994968;     ZxUiwCljSwhvJzAwK31994968 = ZxUiwCljSwhvJzAwK30556534;     ZxUiwCljSwhvJzAwK30556534 = ZxUiwCljSwhvJzAwK54224471;     ZxUiwCljSwhvJzAwK54224471 = ZxUiwCljSwhvJzAwK95165722;     ZxUiwCljSwhvJzAwK95165722 = ZxUiwCljSwhvJzAwK64956812;     ZxUiwCljSwhvJzAwK64956812 = ZxUiwCljSwhvJzAwK3338675;     ZxUiwCljSwhvJzAwK3338675 = ZxUiwCljSwhvJzAwK88022234;     ZxUiwCljSwhvJzAwK88022234 = ZxUiwCljSwhvJzAwK7888162;     ZxUiwCljSwhvJzAwK7888162 = ZxUiwCljSwhvJzAwK6322396;     ZxUiwCljSwhvJzAwK6322396 = ZxUiwCljSwhvJzAwK78163021;     ZxUiwCljSwhvJzAwK78163021 = ZxUiwCljSwhvJzAwK41678536;     ZxUiwCljSwhvJzAwK41678536 = ZxUiwCljSwhvJzAwK34015023;     ZxUiwCljSwhvJzAwK34015023 = ZxUiwCljSwhvJzAwK12258455;     ZxUiwCljSwhvJzAwK12258455 = ZxUiwCljSwhvJzAwK63637721;     ZxUiwCljSwhvJzAwK63637721 = ZxUiwCljSwhvJzAwK60072669;     ZxUiwCljSwhvJzAwK60072669 = ZxUiwCljSwhvJzAwK54509803;     ZxUiwCljSwhvJzAwK54509803 = ZxUiwCljSwhvJzAwK45698191;     ZxUiwCljSwhvJzAwK45698191 = ZxUiwCljSwhvJzAwK65223011;     ZxUiwCljSwhvJzAwK65223011 = ZxUiwCljSwhvJzAwK41687085;     ZxUiwCljSwhvJzAwK41687085 = ZxUiwCljSwhvJzAwK14827839;     ZxUiwCljSwhvJzAwK14827839 = ZxUiwCljSwhvJzAwK55857538;     ZxUiwCljSwhvJzAwK55857538 = ZxUiwCljSwhvJzAwK66462739;     ZxUiwCljSwhvJzAwK66462739 = ZxUiwCljSwhvJzAwK46986107;     ZxUiwCljSwhvJzAwK46986107 = ZxUiwCljSwhvJzAwK27239352;     ZxUiwCljSwhvJzAwK27239352 = ZxUiwCljSwhvJzAwK6662413;     ZxUiwCljSwhvJzAwK6662413 = ZxUiwCljSwhvJzAwK3040165;     ZxUiwCljSwhvJzAwK3040165 = ZxUiwCljSwhvJzAwK66174395;     ZxUiwCljSwhvJzAwK66174395 = ZxUiwCljSwhvJzAwK58033375;     ZxUiwCljSwhvJzAwK58033375 = ZxUiwCljSwhvJzAwK80888817;     ZxUiwCljSwhvJzAwK80888817 = ZxUiwCljSwhvJzAwK13436644;     ZxUiwCljSwhvJzAwK13436644 = ZxUiwCljSwhvJzAwK70979223;     ZxUiwCljSwhvJzAwK70979223 = ZxUiwCljSwhvJzAwK80597333;     ZxUiwCljSwhvJzAwK80597333 = ZxUiwCljSwhvJzAwK34912882;     ZxUiwCljSwhvJzAwK34912882 = ZxUiwCljSwhvJzAwK53963587;     ZxUiwCljSwhvJzAwK53963587 = ZxUiwCljSwhvJzAwK25756756;     ZxUiwCljSwhvJzAwK25756756 = ZxUiwCljSwhvJzAwK2673916;     ZxUiwCljSwhvJzAwK2673916 = ZxUiwCljSwhvJzAwK22850507;     ZxUiwCljSwhvJzAwK22850507 = ZxUiwCljSwhvJzAwK12625330;     ZxUiwCljSwhvJzAwK12625330 = ZxUiwCljSwhvJzAwK75789758;     ZxUiwCljSwhvJzAwK75789758 = ZxUiwCljSwhvJzAwK73540736;     ZxUiwCljSwhvJzAwK73540736 = ZxUiwCljSwhvJzAwK42822543;     ZxUiwCljSwhvJzAwK42822543 = ZxUiwCljSwhvJzAwK84435190;     ZxUiwCljSwhvJzAwK84435190 = ZxUiwCljSwhvJzAwK22137224;     ZxUiwCljSwhvJzAwK22137224 = ZxUiwCljSwhvJzAwK52524919;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wsaAtqsTCHdzYmfn11700264() {     double pkBqLZfuoTMoWLvoL58642026 = -773639823;    double pkBqLZfuoTMoWLvoL26923345 = -467317138;    double pkBqLZfuoTMoWLvoL42959511 = -619569568;    double pkBqLZfuoTMoWLvoL13531539 = -492234447;    double pkBqLZfuoTMoWLvoL14063543 = -300579943;    double pkBqLZfuoTMoWLvoL75557029 = -444310155;    double pkBqLZfuoTMoWLvoL4245687 = -876488467;    double pkBqLZfuoTMoWLvoL93826129 = 8048739;    double pkBqLZfuoTMoWLvoL45333145 = -849350915;    double pkBqLZfuoTMoWLvoL8377943 = -928601718;    double pkBqLZfuoTMoWLvoL43287762 = -965168483;    double pkBqLZfuoTMoWLvoL38795729 = -569778183;    double pkBqLZfuoTMoWLvoL86569687 = -719036571;    double pkBqLZfuoTMoWLvoL58923884 = -612963877;    double pkBqLZfuoTMoWLvoL97333644 = -293625849;    double pkBqLZfuoTMoWLvoL62894839 = 536866;    double pkBqLZfuoTMoWLvoL87139121 = -935461994;    double pkBqLZfuoTMoWLvoL28637934 = -256715565;    double pkBqLZfuoTMoWLvoL15319085 = 24108355;    double pkBqLZfuoTMoWLvoL86519745 = 40089865;    double pkBqLZfuoTMoWLvoL7119689 = -933744187;    double pkBqLZfuoTMoWLvoL32193640 = -828907940;    double pkBqLZfuoTMoWLvoL83771681 = -74624259;    double pkBqLZfuoTMoWLvoL78914733 = -132181437;    double pkBqLZfuoTMoWLvoL99599875 = -731952483;    double pkBqLZfuoTMoWLvoL15665258 = -461964702;    double pkBqLZfuoTMoWLvoL87433882 = -269275918;    double pkBqLZfuoTMoWLvoL91907034 = -717676698;    double pkBqLZfuoTMoWLvoL6859820 = -858581228;    double pkBqLZfuoTMoWLvoL61284052 = -933705090;    double pkBqLZfuoTMoWLvoL51502671 = -163711098;    double pkBqLZfuoTMoWLvoL74465698 = 82611443;    double pkBqLZfuoTMoWLvoL38520614 = -106967547;    double pkBqLZfuoTMoWLvoL53512597 = -263961098;    double pkBqLZfuoTMoWLvoL3239837 = -360147996;    double pkBqLZfuoTMoWLvoL58058676 = -56675885;    double pkBqLZfuoTMoWLvoL39013659 = -311612682;    double pkBqLZfuoTMoWLvoL67320661 = -336578384;    double pkBqLZfuoTMoWLvoL50685232 = -386075085;    double pkBqLZfuoTMoWLvoL95292989 = -555258174;    double pkBqLZfuoTMoWLvoL46415794 = -437973412;    double pkBqLZfuoTMoWLvoL9029868 = -217261847;    double pkBqLZfuoTMoWLvoL8911756 = -218714882;    double pkBqLZfuoTMoWLvoL79731092 = -533146030;    double pkBqLZfuoTMoWLvoL40412085 = -190125561;    double pkBqLZfuoTMoWLvoL20853313 = -166244683;    double pkBqLZfuoTMoWLvoL81482805 = -456305743;    double pkBqLZfuoTMoWLvoL90268632 = -799757153;    double pkBqLZfuoTMoWLvoL85435473 = -31217734;    double pkBqLZfuoTMoWLvoL87464822 = -821668596;    double pkBqLZfuoTMoWLvoL70335718 = -689689451;    double pkBqLZfuoTMoWLvoL89181023 = -400585302;    double pkBqLZfuoTMoWLvoL11666890 = -650619071;    double pkBqLZfuoTMoWLvoL53799323 = -688675839;    double pkBqLZfuoTMoWLvoL4580575 = -278765586;    double pkBqLZfuoTMoWLvoL26448386 = -944731883;    double pkBqLZfuoTMoWLvoL43151663 = -292692879;    double pkBqLZfuoTMoWLvoL64044778 = -387388131;    double pkBqLZfuoTMoWLvoL13931664 = -760281965;    double pkBqLZfuoTMoWLvoL98398284 = -838615242;    double pkBqLZfuoTMoWLvoL88123147 = -75034238;    double pkBqLZfuoTMoWLvoL12338653 = -58811769;    double pkBqLZfuoTMoWLvoL86966310 = -133370033;    double pkBqLZfuoTMoWLvoL84049093 = -915645825;    double pkBqLZfuoTMoWLvoL56875271 = -664890621;    double pkBqLZfuoTMoWLvoL68822064 = -947779926;    double pkBqLZfuoTMoWLvoL275116 = -362810637;    double pkBqLZfuoTMoWLvoL33057091 = -355075474;    double pkBqLZfuoTMoWLvoL55684047 = -152815882;    double pkBqLZfuoTMoWLvoL39274969 = -136949964;    double pkBqLZfuoTMoWLvoL23881181 = -687850452;    double pkBqLZfuoTMoWLvoL19818460 = -498883611;    double pkBqLZfuoTMoWLvoL77952702 = -870640481;    double pkBqLZfuoTMoWLvoL20026095 = -420633472;    double pkBqLZfuoTMoWLvoL40103952 = -521936724;    double pkBqLZfuoTMoWLvoL98089821 = -616482341;    double pkBqLZfuoTMoWLvoL23281885 = -510193059;    double pkBqLZfuoTMoWLvoL4040590 = -541478230;    double pkBqLZfuoTMoWLvoL38502648 = -942055876;    double pkBqLZfuoTMoWLvoL78746562 = -465707800;    double pkBqLZfuoTMoWLvoL34182453 = 94341040;    double pkBqLZfuoTMoWLvoL97165250 = -469518766;    double pkBqLZfuoTMoWLvoL6471561 = -586458965;    double pkBqLZfuoTMoWLvoL19394998 = 63087367;    double pkBqLZfuoTMoWLvoL90948333 = -144015639;    double pkBqLZfuoTMoWLvoL62321647 = -763125796;    double pkBqLZfuoTMoWLvoL62798808 = -266769487;    double pkBqLZfuoTMoWLvoL84721291 = -418291708;    double pkBqLZfuoTMoWLvoL48932023 = -985195513;    double pkBqLZfuoTMoWLvoL76791450 = -415416113;    double pkBqLZfuoTMoWLvoL14907013 = -763983007;    double pkBqLZfuoTMoWLvoL74968880 = -924224552;    double pkBqLZfuoTMoWLvoL53388997 = -576296419;    double pkBqLZfuoTMoWLvoL52286947 = -547459843;    double pkBqLZfuoTMoWLvoL7169843 = -380223936;    double pkBqLZfuoTMoWLvoL34077142 = -279161644;    double pkBqLZfuoTMoWLvoL22063558 = 16108186;    double pkBqLZfuoTMoWLvoL24862663 = -303069057;    double pkBqLZfuoTMoWLvoL22855821 = -868255410;    double pkBqLZfuoTMoWLvoL71590021 = -773639823;     pkBqLZfuoTMoWLvoL58642026 = pkBqLZfuoTMoWLvoL26923345;     pkBqLZfuoTMoWLvoL26923345 = pkBqLZfuoTMoWLvoL42959511;     pkBqLZfuoTMoWLvoL42959511 = pkBqLZfuoTMoWLvoL13531539;     pkBqLZfuoTMoWLvoL13531539 = pkBqLZfuoTMoWLvoL14063543;     pkBqLZfuoTMoWLvoL14063543 = pkBqLZfuoTMoWLvoL75557029;     pkBqLZfuoTMoWLvoL75557029 = pkBqLZfuoTMoWLvoL4245687;     pkBqLZfuoTMoWLvoL4245687 = pkBqLZfuoTMoWLvoL93826129;     pkBqLZfuoTMoWLvoL93826129 = pkBqLZfuoTMoWLvoL45333145;     pkBqLZfuoTMoWLvoL45333145 = pkBqLZfuoTMoWLvoL8377943;     pkBqLZfuoTMoWLvoL8377943 = pkBqLZfuoTMoWLvoL43287762;     pkBqLZfuoTMoWLvoL43287762 = pkBqLZfuoTMoWLvoL38795729;     pkBqLZfuoTMoWLvoL38795729 = pkBqLZfuoTMoWLvoL86569687;     pkBqLZfuoTMoWLvoL86569687 = pkBqLZfuoTMoWLvoL58923884;     pkBqLZfuoTMoWLvoL58923884 = pkBqLZfuoTMoWLvoL97333644;     pkBqLZfuoTMoWLvoL97333644 = pkBqLZfuoTMoWLvoL62894839;     pkBqLZfuoTMoWLvoL62894839 = pkBqLZfuoTMoWLvoL87139121;     pkBqLZfuoTMoWLvoL87139121 = pkBqLZfuoTMoWLvoL28637934;     pkBqLZfuoTMoWLvoL28637934 = pkBqLZfuoTMoWLvoL15319085;     pkBqLZfuoTMoWLvoL15319085 = pkBqLZfuoTMoWLvoL86519745;     pkBqLZfuoTMoWLvoL86519745 = pkBqLZfuoTMoWLvoL7119689;     pkBqLZfuoTMoWLvoL7119689 = pkBqLZfuoTMoWLvoL32193640;     pkBqLZfuoTMoWLvoL32193640 = pkBqLZfuoTMoWLvoL83771681;     pkBqLZfuoTMoWLvoL83771681 = pkBqLZfuoTMoWLvoL78914733;     pkBqLZfuoTMoWLvoL78914733 = pkBqLZfuoTMoWLvoL99599875;     pkBqLZfuoTMoWLvoL99599875 = pkBqLZfuoTMoWLvoL15665258;     pkBqLZfuoTMoWLvoL15665258 = pkBqLZfuoTMoWLvoL87433882;     pkBqLZfuoTMoWLvoL87433882 = pkBqLZfuoTMoWLvoL91907034;     pkBqLZfuoTMoWLvoL91907034 = pkBqLZfuoTMoWLvoL6859820;     pkBqLZfuoTMoWLvoL6859820 = pkBqLZfuoTMoWLvoL61284052;     pkBqLZfuoTMoWLvoL61284052 = pkBqLZfuoTMoWLvoL51502671;     pkBqLZfuoTMoWLvoL51502671 = pkBqLZfuoTMoWLvoL74465698;     pkBqLZfuoTMoWLvoL74465698 = pkBqLZfuoTMoWLvoL38520614;     pkBqLZfuoTMoWLvoL38520614 = pkBqLZfuoTMoWLvoL53512597;     pkBqLZfuoTMoWLvoL53512597 = pkBqLZfuoTMoWLvoL3239837;     pkBqLZfuoTMoWLvoL3239837 = pkBqLZfuoTMoWLvoL58058676;     pkBqLZfuoTMoWLvoL58058676 = pkBqLZfuoTMoWLvoL39013659;     pkBqLZfuoTMoWLvoL39013659 = pkBqLZfuoTMoWLvoL67320661;     pkBqLZfuoTMoWLvoL67320661 = pkBqLZfuoTMoWLvoL50685232;     pkBqLZfuoTMoWLvoL50685232 = pkBqLZfuoTMoWLvoL95292989;     pkBqLZfuoTMoWLvoL95292989 = pkBqLZfuoTMoWLvoL46415794;     pkBqLZfuoTMoWLvoL46415794 = pkBqLZfuoTMoWLvoL9029868;     pkBqLZfuoTMoWLvoL9029868 = pkBqLZfuoTMoWLvoL8911756;     pkBqLZfuoTMoWLvoL8911756 = pkBqLZfuoTMoWLvoL79731092;     pkBqLZfuoTMoWLvoL79731092 = pkBqLZfuoTMoWLvoL40412085;     pkBqLZfuoTMoWLvoL40412085 = pkBqLZfuoTMoWLvoL20853313;     pkBqLZfuoTMoWLvoL20853313 = pkBqLZfuoTMoWLvoL81482805;     pkBqLZfuoTMoWLvoL81482805 = pkBqLZfuoTMoWLvoL90268632;     pkBqLZfuoTMoWLvoL90268632 = pkBqLZfuoTMoWLvoL85435473;     pkBqLZfuoTMoWLvoL85435473 = pkBqLZfuoTMoWLvoL87464822;     pkBqLZfuoTMoWLvoL87464822 = pkBqLZfuoTMoWLvoL70335718;     pkBqLZfuoTMoWLvoL70335718 = pkBqLZfuoTMoWLvoL89181023;     pkBqLZfuoTMoWLvoL89181023 = pkBqLZfuoTMoWLvoL11666890;     pkBqLZfuoTMoWLvoL11666890 = pkBqLZfuoTMoWLvoL53799323;     pkBqLZfuoTMoWLvoL53799323 = pkBqLZfuoTMoWLvoL4580575;     pkBqLZfuoTMoWLvoL4580575 = pkBqLZfuoTMoWLvoL26448386;     pkBqLZfuoTMoWLvoL26448386 = pkBqLZfuoTMoWLvoL43151663;     pkBqLZfuoTMoWLvoL43151663 = pkBqLZfuoTMoWLvoL64044778;     pkBqLZfuoTMoWLvoL64044778 = pkBqLZfuoTMoWLvoL13931664;     pkBqLZfuoTMoWLvoL13931664 = pkBqLZfuoTMoWLvoL98398284;     pkBqLZfuoTMoWLvoL98398284 = pkBqLZfuoTMoWLvoL88123147;     pkBqLZfuoTMoWLvoL88123147 = pkBqLZfuoTMoWLvoL12338653;     pkBqLZfuoTMoWLvoL12338653 = pkBqLZfuoTMoWLvoL86966310;     pkBqLZfuoTMoWLvoL86966310 = pkBqLZfuoTMoWLvoL84049093;     pkBqLZfuoTMoWLvoL84049093 = pkBqLZfuoTMoWLvoL56875271;     pkBqLZfuoTMoWLvoL56875271 = pkBqLZfuoTMoWLvoL68822064;     pkBqLZfuoTMoWLvoL68822064 = pkBqLZfuoTMoWLvoL275116;     pkBqLZfuoTMoWLvoL275116 = pkBqLZfuoTMoWLvoL33057091;     pkBqLZfuoTMoWLvoL33057091 = pkBqLZfuoTMoWLvoL55684047;     pkBqLZfuoTMoWLvoL55684047 = pkBqLZfuoTMoWLvoL39274969;     pkBqLZfuoTMoWLvoL39274969 = pkBqLZfuoTMoWLvoL23881181;     pkBqLZfuoTMoWLvoL23881181 = pkBqLZfuoTMoWLvoL19818460;     pkBqLZfuoTMoWLvoL19818460 = pkBqLZfuoTMoWLvoL77952702;     pkBqLZfuoTMoWLvoL77952702 = pkBqLZfuoTMoWLvoL20026095;     pkBqLZfuoTMoWLvoL20026095 = pkBqLZfuoTMoWLvoL40103952;     pkBqLZfuoTMoWLvoL40103952 = pkBqLZfuoTMoWLvoL98089821;     pkBqLZfuoTMoWLvoL98089821 = pkBqLZfuoTMoWLvoL23281885;     pkBqLZfuoTMoWLvoL23281885 = pkBqLZfuoTMoWLvoL4040590;     pkBqLZfuoTMoWLvoL4040590 = pkBqLZfuoTMoWLvoL38502648;     pkBqLZfuoTMoWLvoL38502648 = pkBqLZfuoTMoWLvoL78746562;     pkBqLZfuoTMoWLvoL78746562 = pkBqLZfuoTMoWLvoL34182453;     pkBqLZfuoTMoWLvoL34182453 = pkBqLZfuoTMoWLvoL97165250;     pkBqLZfuoTMoWLvoL97165250 = pkBqLZfuoTMoWLvoL6471561;     pkBqLZfuoTMoWLvoL6471561 = pkBqLZfuoTMoWLvoL19394998;     pkBqLZfuoTMoWLvoL19394998 = pkBqLZfuoTMoWLvoL90948333;     pkBqLZfuoTMoWLvoL90948333 = pkBqLZfuoTMoWLvoL62321647;     pkBqLZfuoTMoWLvoL62321647 = pkBqLZfuoTMoWLvoL62798808;     pkBqLZfuoTMoWLvoL62798808 = pkBqLZfuoTMoWLvoL84721291;     pkBqLZfuoTMoWLvoL84721291 = pkBqLZfuoTMoWLvoL48932023;     pkBqLZfuoTMoWLvoL48932023 = pkBqLZfuoTMoWLvoL76791450;     pkBqLZfuoTMoWLvoL76791450 = pkBqLZfuoTMoWLvoL14907013;     pkBqLZfuoTMoWLvoL14907013 = pkBqLZfuoTMoWLvoL74968880;     pkBqLZfuoTMoWLvoL74968880 = pkBqLZfuoTMoWLvoL53388997;     pkBqLZfuoTMoWLvoL53388997 = pkBqLZfuoTMoWLvoL52286947;     pkBqLZfuoTMoWLvoL52286947 = pkBqLZfuoTMoWLvoL7169843;     pkBqLZfuoTMoWLvoL7169843 = pkBqLZfuoTMoWLvoL34077142;     pkBqLZfuoTMoWLvoL34077142 = pkBqLZfuoTMoWLvoL22063558;     pkBqLZfuoTMoWLvoL22063558 = pkBqLZfuoTMoWLvoL24862663;     pkBqLZfuoTMoWLvoL24862663 = pkBqLZfuoTMoWLvoL22855821;     pkBqLZfuoTMoWLvoL22855821 = pkBqLZfuoTMoWLvoL71590021;     pkBqLZfuoTMoWLvoL71590021 = pkBqLZfuoTMoWLvoL58642026;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IeEqbkGyLYOyTcSB48345238() {     double BjrwJHauNtQydxhaX81029143 = -408043786;    double BjrwJHauNtQydxhaX11270845 = -415860193;    double BjrwJHauNtQydxhaX71728790 = -859223412;    double BjrwJHauNtQydxhaX28538228 = -498641979;    double BjrwJHauNtQydxhaX12237797 = -891636944;    double BjrwJHauNtQydxhaX90440146 = -304918533;    double BjrwJHauNtQydxhaX96401425 = -698475773;    double BjrwJHauNtQydxhaX69459988 = -462547905;    double BjrwJHauNtQydxhaX42496277 = -733546954;    double BjrwJHauNtQydxhaX141527 = -671343051;    double BjrwJHauNtQydxhaX80829621 = -961792884;    double BjrwJHauNtQydxhaX94698678 = -820523476;    double BjrwJHauNtQydxhaX53577868 = -237753891;    double BjrwJHauNtQydxhaX1538636 = -40672634;    double BjrwJHauNtQydxhaX34068921 = 78896438;    double BjrwJHauNtQydxhaX7150752 = -381130677;    double BjrwJHauNtQydxhaX36962473 = -112741814;    double BjrwJHauNtQydxhaX34614350 = -708458256;    double BjrwJHauNtQydxhaX56600716 = -110993770;    double BjrwJHauNtQydxhaX65036028 = -212544589;    double BjrwJHauNtQydxhaX8002797 = -925231495;    double BjrwJHauNtQydxhaX96008858 = -233674253;    double BjrwJHauNtQydxhaX87065004 = -250704851;    double BjrwJHauNtQydxhaX5414521 = -696174294;    double BjrwJHauNtQydxhaX35140122 = -353511607;    double BjrwJHauNtQydxhaX3968253 = -905858926;    double BjrwJHauNtQydxhaX34523303 = -26862410;    double BjrwJHauNtQydxhaX97818484 = -506131152;    double BjrwJHauNtQydxhaX13752381 = -102838347;    double BjrwJHauNtQydxhaX70441231 = -854000957;    double BjrwJHauNtQydxhaX95793475 = -245027250;    double BjrwJHauNtQydxhaX32006889 = -569459912;    double BjrwJHauNtQydxhaX19559285 = -636311042;    double BjrwJHauNtQydxhaX12901285 = -588087948;    double BjrwJHauNtQydxhaX79469201 = -423904418;    double BjrwJHauNtQydxhaX99751544 = -372071262;    double BjrwJHauNtQydxhaX32453371 = -44560634;    double BjrwJHauNtQydxhaX20946763 = -204203856;    double BjrwJHauNtQydxhaX98993362 = -787121116;    double BjrwJHauNtQydxhaX15478705 = -691482203;    double BjrwJHauNtQydxhaX75173164 = -941151078;    double BjrwJHauNtQydxhaX97538258 = 24829462;    double BjrwJHauNtQydxhaX14461948 = -106245465;    double BjrwJHauNtQydxhaX79175211 = -530509679;    double BjrwJHauNtQydxhaX8395352 = 730961;    double BjrwJHauNtQydxhaX28953595 = -348473778;    double BjrwJHauNtQydxhaX93780202 = 65284005;    double BjrwJHauNtQydxhaX34867307 = -556292143;    double BjrwJHauNtQydxhaX79895378 = 19065568;    double BjrwJHauNtQydxhaX84548567 = -963688855;    double BjrwJHauNtQydxhaX79204934 = 55234272;    double BjrwJHauNtQydxhaX63032280 = -831404584;    double BjrwJHauNtQydxhaX89981292 = -322530301;    double BjrwJHauNtQydxhaX14085868 = -964356648;    double BjrwJHauNtQydxhaX40342442 = -419469701;    double BjrwJHauNtQydxhaX85020285 = -74369533;    double BjrwJHauNtQydxhaX24205841 = -65155342;    double BjrwJHauNtQydxhaX66314269 = -63049118;    double BjrwJHauNtQydxhaX93398105 = -45130372;    double BjrwJHauNtQydxhaX8269545 = -985778019;    double BjrwJHauNtQydxhaX55916843 = -178056123;    double BjrwJHauNtQydxhaX98582941 = -92344621;    double BjrwJHauNtQydxhaX55707608 = -259709558;    double BjrwJHauNtQydxhaX72055045 = -879545998;    double BjrwJHauNtQydxhaX4348051 = -326315802;    double BjrwJHauNtQydxhaX48822733 = -292332973;    double BjrwJHauNtQydxhaX75139393 = -84212435;    double BjrwJHauNtQydxhaX40676584 = -649665943;    double BjrwJHauNtQydxhaX22069435 = -616768216;    double BjrwJHauNtQydxhaX34317376 = -549032301;    double BjrwJHauNtQydxhaX74697381 = -236570043;    double BjrwJHauNtQydxhaX16015711 = -908537959;    double BjrwJHauNtQydxhaX35620987 = -921337140;    double BjrwJHauNtQydxhaX41122011 = -419511568;    double BjrwJHauNtQydxhaX89862863 = -271393511;    double BjrwJHauNtQydxhaX10464538 = -850060958;    double BjrwJHauNtQydxhaX81546910 = -27428788;    double BjrwJHauNtQydxhaX7889793 = -720195173;    double BjrwJHauNtQydxhaX97019168 = -596905256;    double BjrwJHauNtQydxhaX6186527 = 94962171;    double BjrwJHauNtQydxhaX10188050 = -871142931;    double BjrwJHauNtQydxhaX99655995 = -470570268;    double BjrwJHauNtQydxhaX17923106 = -425196720;    double BjrwJHauNtQydxhaX29203813 = -139149493;    double BjrwJHauNtQydxhaX91236296 = -809235229;    double BjrwJHauNtQydxhaX32761196 = -413622666;    double BjrwJHauNtQydxhaX42025596 = -146929612;    double BjrwJHauNtQydxhaX5473417 = -671954394;    double BjrwJHauNtQydxhaX72558843 = -68618247;    double BjrwJHauNtQydxhaX94448916 = -249534886;    double BjrwJHauNtQydxhaX75545704 = -206915921;    double BjrwJHauNtQydxhaX66139101 = -981511517;    double BjrwJHauNtQydxhaX27548657 = -59073484;    double BjrwJHauNtQydxhaX90723818 = -801343098;    double BjrwJHauNtQydxhaX59561861 = -413426081;    double BjrwJHauNtQydxhaX76590223 = -748806457;    double BjrwJHauNtQydxhaX41830651 = -715460980;    double BjrwJHauNtQydxhaX42406903 = -226699468;    double BjrwJHauNtQydxhaX74827160 = -104193878;    double BjrwJHauNtQydxhaX59572618 = -408043786;     BjrwJHauNtQydxhaX81029143 = BjrwJHauNtQydxhaX11270845;     BjrwJHauNtQydxhaX11270845 = BjrwJHauNtQydxhaX71728790;     BjrwJHauNtQydxhaX71728790 = BjrwJHauNtQydxhaX28538228;     BjrwJHauNtQydxhaX28538228 = BjrwJHauNtQydxhaX12237797;     BjrwJHauNtQydxhaX12237797 = BjrwJHauNtQydxhaX90440146;     BjrwJHauNtQydxhaX90440146 = BjrwJHauNtQydxhaX96401425;     BjrwJHauNtQydxhaX96401425 = BjrwJHauNtQydxhaX69459988;     BjrwJHauNtQydxhaX69459988 = BjrwJHauNtQydxhaX42496277;     BjrwJHauNtQydxhaX42496277 = BjrwJHauNtQydxhaX141527;     BjrwJHauNtQydxhaX141527 = BjrwJHauNtQydxhaX80829621;     BjrwJHauNtQydxhaX80829621 = BjrwJHauNtQydxhaX94698678;     BjrwJHauNtQydxhaX94698678 = BjrwJHauNtQydxhaX53577868;     BjrwJHauNtQydxhaX53577868 = BjrwJHauNtQydxhaX1538636;     BjrwJHauNtQydxhaX1538636 = BjrwJHauNtQydxhaX34068921;     BjrwJHauNtQydxhaX34068921 = BjrwJHauNtQydxhaX7150752;     BjrwJHauNtQydxhaX7150752 = BjrwJHauNtQydxhaX36962473;     BjrwJHauNtQydxhaX36962473 = BjrwJHauNtQydxhaX34614350;     BjrwJHauNtQydxhaX34614350 = BjrwJHauNtQydxhaX56600716;     BjrwJHauNtQydxhaX56600716 = BjrwJHauNtQydxhaX65036028;     BjrwJHauNtQydxhaX65036028 = BjrwJHauNtQydxhaX8002797;     BjrwJHauNtQydxhaX8002797 = BjrwJHauNtQydxhaX96008858;     BjrwJHauNtQydxhaX96008858 = BjrwJHauNtQydxhaX87065004;     BjrwJHauNtQydxhaX87065004 = BjrwJHauNtQydxhaX5414521;     BjrwJHauNtQydxhaX5414521 = BjrwJHauNtQydxhaX35140122;     BjrwJHauNtQydxhaX35140122 = BjrwJHauNtQydxhaX3968253;     BjrwJHauNtQydxhaX3968253 = BjrwJHauNtQydxhaX34523303;     BjrwJHauNtQydxhaX34523303 = BjrwJHauNtQydxhaX97818484;     BjrwJHauNtQydxhaX97818484 = BjrwJHauNtQydxhaX13752381;     BjrwJHauNtQydxhaX13752381 = BjrwJHauNtQydxhaX70441231;     BjrwJHauNtQydxhaX70441231 = BjrwJHauNtQydxhaX95793475;     BjrwJHauNtQydxhaX95793475 = BjrwJHauNtQydxhaX32006889;     BjrwJHauNtQydxhaX32006889 = BjrwJHauNtQydxhaX19559285;     BjrwJHauNtQydxhaX19559285 = BjrwJHauNtQydxhaX12901285;     BjrwJHauNtQydxhaX12901285 = BjrwJHauNtQydxhaX79469201;     BjrwJHauNtQydxhaX79469201 = BjrwJHauNtQydxhaX99751544;     BjrwJHauNtQydxhaX99751544 = BjrwJHauNtQydxhaX32453371;     BjrwJHauNtQydxhaX32453371 = BjrwJHauNtQydxhaX20946763;     BjrwJHauNtQydxhaX20946763 = BjrwJHauNtQydxhaX98993362;     BjrwJHauNtQydxhaX98993362 = BjrwJHauNtQydxhaX15478705;     BjrwJHauNtQydxhaX15478705 = BjrwJHauNtQydxhaX75173164;     BjrwJHauNtQydxhaX75173164 = BjrwJHauNtQydxhaX97538258;     BjrwJHauNtQydxhaX97538258 = BjrwJHauNtQydxhaX14461948;     BjrwJHauNtQydxhaX14461948 = BjrwJHauNtQydxhaX79175211;     BjrwJHauNtQydxhaX79175211 = BjrwJHauNtQydxhaX8395352;     BjrwJHauNtQydxhaX8395352 = BjrwJHauNtQydxhaX28953595;     BjrwJHauNtQydxhaX28953595 = BjrwJHauNtQydxhaX93780202;     BjrwJHauNtQydxhaX93780202 = BjrwJHauNtQydxhaX34867307;     BjrwJHauNtQydxhaX34867307 = BjrwJHauNtQydxhaX79895378;     BjrwJHauNtQydxhaX79895378 = BjrwJHauNtQydxhaX84548567;     BjrwJHauNtQydxhaX84548567 = BjrwJHauNtQydxhaX79204934;     BjrwJHauNtQydxhaX79204934 = BjrwJHauNtQydxhaX63032280;     BjrwJHauNtQydxhaX63032280 = BjrwJHauNtQydxhaX89981292;     BjrwJHauNtQydxhaX89981292 = BjrwJHauNtQydxhaX14085868;     BjrwJHauNtQydxhaX14085868 = BjrwJHauNtQydxhaX40342442;     BjrwJHauNtQydxhaX40342442 = BjrwJHauNtQydxhaX85020285;     BjrwJHauNtQydxhaX85020285 = BjrwJHauNtQydxhaX24205841;     BjrwJHauNtQydxhaX24205841 = BjrwJHauNtQydxhaX66314269;     BjrwJHauNtQydxhaX66314269 = BjrwJHauNtQydxhaX93398105;     BjrwJHauNtQydxhaX93398105 = BjrwJHauNtQydxhaX8269545;     BjrwJHauNtQydxhaX8269545 = BjrwJHauNtQydxhaX55916843;     BjrwJHauNtQydxhaX55916843 = BjrwJHauNtQydxhaX98582941;     BjrwJHauNtQydxhaX98582941 = BjrwJHauNtQydxhaX55707608;     BjrwJHauNtQydxhaX55707608 = BjrwJHauNtQydxhaX72055045;     BjrwJHauNtQydxhaX72055045 = BjrwJHauNtQydxhaX4348051;     BjrwJHauNtQydxhaX4348051 = BjrwJHauNtQydxhaX48822733;     BjrwJHauNtQydxhaX48822733 = BjrwJHauNtQydxhaX75139393;     BjrwJHauNtQydxhaX75139393 = BjrwJHauNtQydxhaX40676584;     BjrwJHauNtQydxhaX40676584 = BjrwJHauNtQydxhaX22069435;     BjrwJHauNtQydxhaX22069435 = BjrwJHauNtQydxhaX34317376;     BjrwJHauNtQydxhaX34317376 = BjrwJHauNtQydxhaX74697381;     BjrwJHauNtQydxhaX74697381 = BjrwJHauNtQydxhaX16015711;     BjrwJHauNtQydxhaX16015711 = BjrwJHauNtQydxhaX35620987;     BjrwJHauNtQydxhaX35620987 = BjrwJHauNtQydxhaX41122011;     BjrwJHauNtQydxhaX41122011 = BjrwJHauNtQydxhaX89862863;     BjrwJHauNtQydxhaX89862863 = BjrwJHauNtQydxhaX10464538;     BjrwJHauNtQydxhaX10464538 = BjrwJHauNtQydxhaX81546910;     BjrwJHauNtQydxhaX81546910 = BjrwJHauNtQydxhaX7889793;     BjrwJHauNtQydxhaX7889793 = BjrwJHauNtQydxhaX97019168;     BjrwJHauNtQydxhaX97019168 = BjrwJHauNtQydxhaX6186527;     BjrwJHauNtQydxhaX6186527 = BjrwJHauNtQydxhaX10188050;     BjrwJHauNtQydxhaX10188050 = BjrwJHauNtQydxhaX99655995;     BjrwJHauNtQydxhaX99655995 = BjrwJHauNtQydxhaX17923106;     BjrwJHauNtQydxhaX17923106 = BjrwJHauNtQydxhaX29203813;     BjrwJHauNtQydxhaX29203813 = BjrwJHauNtQydxhaX91236296;     BjrwJHauNtQydxhaX91236296 = BjrwJHauNtQydxhaX32761196;     BjrwJHauNtQydxhaX32761196 = BjrwJHauNtQydxhaX42025596;     BjrwJHauNtQydxhaX42025596 = BjrwJHauNtQydxhaX5473417;     BjrwJHauNtQydxhaX5473417 = BjrwJHauNtQydxhaX72558843;     BjrwJHauNtQydxhaX72558843 = BjrwJHauNtQydxhaX94448916;     BjrwJHauNtQydxhaX94448916 = BjrwJHauNtQydxhaX75545704;     BjrwJHauNtQydxhaX75545704 = BjrwJHauNtQydxhaX66139101;     BjrwJHauNtQydxhaX66139101 = BjrwJHauNtQydxhaX27548657;     BjrwJHauNtQydxhaX27548657 = BjrwJHauNtQydxhaX90723818;     BjrwJHauNtQydxhaX90723818 = BjrwJHauNtQydxhaX59561861;     BjrwJHauNtQydxhaX59561861 = BjrwJHauNtQydxhaX76590223;     BjrwJHauNtQydxhaX76590223 = BjrwJHauNtQydxhaX41830651;     BjrwJHauNtQydxhaX41830651 = BjrwJHauNtQydxhaX42406903;     BjrwJHauNtQydxhaX42406903 = BjrwJHauNtQydxhaX74827160;     BjrwJHauNtQydxhaX74827160 = BjrwJHauNtQydxhaX59572618;     BjrwJHauNtQydxhaX59572618 = BjrwJHauNtQydxhaX81029143;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TDcqEtXqPJCpSbBB15636838() {     double AtKsXHcTVdxDtQXAd16488112 = -874320686;    double AtKsXHcTVdxDtQXAd24542924 = -296411799;    double AtKsXHcTVdxDtQXAd9115267 = -524890487;    double AtKsXHcTVdxDtQXAd761583 = -319793050;    double AtKsXHcTVdxDtQXAd68523065 = -997235790;    double AtKsXHcTVdxDtQXAd26663018 = -567266428;    double AtKsXHcTVdxDtQXAd7058832 = -543201507;    double AtKsXHcTVdxDtQXAd31591549 = -106782046;    double AtKsXHcTVdxDtQXAd44013882 = -121878361;    double AtKsXHcTVdxDtQXAd15044239 = -336030049;    double AtKsXHcTVdxDtQXAd19011357 = -763800614;    double AtKsXHcTVdxDtQXAd95266826 = -684644248;    double AtKsXHcTVdxDtQXAd91812513 = -789766396;    double AtKsXHcTVdxDtQXAd54679499 = -17036044;    double AtKsXHcTVdxDtQXAd92263437 = -424528577;    double AtKsXHcTVdxDtQXAd90692052 = -16942284;    double AtKsXHcTVdxDtQXAd92798408 = -639716367;    double AtKsXHcTVdxDtQXAd34374542 = -781886324;    double AtKsXHcTVdxDtQXAd57151045 = -427557192;    double AtKsXHcTVdxDtQXAd58873247 = -639105655;    double AtKsXHcTVdxDtQXAd21089659 = -582506184;    double AtKsXHcTVdxDtQXAd99430383 = -320310547;    double AtKsXHcTVdxDtQXAd57590384 = -988337036;    double AtKsXHcTVdxDtQXAd99207465 = -106998899;    double AtKsXHcTVdxDtQXAd43158073 = -377496805;    double AtKsXHcTVdxDtQXAd14666424 = -594435733;    double AtKsXHcTVdxDtQXAd94943876 = -933146551;    double AtKsXHcTVdxDtQXAd38546909 = -675903326;    double AtKsXHcTVdxDtQXAd60420595 = -676933150;    double AtKsXHcTVdxDtQXAd75974642 = -888508127;    double AtKsXHcTVdxDtQXAd14376708 = -797007737;    double AtKsXHcTVdxDtQXAd69829907 = -342431326;    double AtKsXHcTVdxDtQXAd31777296 = -783907019;    double AtKsXHcTVdxDtQXAd42649711 = -652781762;    double AtKsXHcTVdxDtQXAd45277655 = -375004669;    double AtKsXHcTVdxDtQXAd51312389 = -586636568;    double AtKsXHcTVdxDtQXAd81865187 = 90743326;    double AtKsXHcTVdxDtQXAd72162166 = -186859437;    double AtKsXHcTVdxDtQXAd29383925 = -437997393;    double AtKsXHcTVdxDtQXAd79750236 = -156744749;    double AtKsXHcTVdxDtQXAd8624773 = -710349993;    double AtKsXHcTVdxDtQXAd86903385 = -669715829;    double AtKsXHcTVdxDtQXAd3845327 = -759426251;    double AtKsXHcTVdxDtQXAd19306550 = -34842089;    double AtKsXHcTVdxDtQXAd8411811 = -925780200;    double AtKsXHcTVdxDtQXAd74914923 = -387397410;    double AtKsXHcTVdxDtQXAd13125367 = -418702356;    double AtKsXHcTVdxDtQXAd52593618 = -722818737;    double AtKsXHcTVdxDtQXAd2788045 = -744671312;    double AtKsXHcTVdxDtQXAd9078157 = -354715670;    double AtKsXHcTVdxDtQXAd59975658 = -919042286;    double AtKsXHcTVdxDtQXAd46517753 = 54943069;    double AtKsXHcTVdxDtQXAd58602441 = -78575855;    double AtKsXHcTVdxDtQXAd30077715 = -55865148;    double AtKsXHcTVdxDtQXAd94547178 = -765959847;    double AtKsXHcTVdxDtQXAd17057729 = -454010139;    double AtKsXHcTVdxDtQXAd66952539 = -308074764;    double AtKsXHcTVdxDtQXAd9907802 = -317891589;    double AtKsXHcTVdxDtQXAd57603509 = -942296245;    double AtKsXHcTVdxDtQXAd53856642 = -302800058;    double AtKsXHcTVdxDtQXAd31719142 = -634119878;    double AtKsXHcTVdxDtQXAd68511922 = -867298181;    double AtKsXHcTVdxDtQXAd71170954 = -429848896;    double AtKsXHcTVdxDtQXAd68039239 = -233370234;    double AtKsXHcTVdxDtQXAd667532 = -539022313;    double AtKsXHcTVdxDtQXAd49181449 = -321369288;    double AtKsXHcTVdxDtQXAd63489531 = -900737229;    double AtKsXHcTVdxDtQXAd49162803 = -36984635;    double AtKsXHcTVdxDtQXAd9401845 = -642031375;    double AtKsXHcTVdxDtQXAd40951049 = -837892010;    double AtKsXHcTVdxDtQXAd8826866 = -7685610;    double AtKsXHcTVdxDtQXAd20636243 = -352856930;    double AtKsXHcTVdxDtQXAd4990617 = -243888932;    double AtKsXHcTVdxDtQXAd77400809 = -170812443;    double AtKsXHcTVdxDtQXAd50248475 = -928755663;    double AtKsXHcTVdxDtQXAd34186274 = -912790356;    double AtKsXHcTVdxDtQXAd95585057 = -560884296;    double AtKsXHcTVdxDtQXAd38283835 = -853494947;    double AtKsXHcTVdxDtQXAd90795655 = -181218699;    double AtKsXHcTVdxDtQXAd68243149 = -990099395;    double AtKsXHcTVdxDtQXAd1541058 = -75733377;    double AtKsXHcTVdxDtQXAd42350258 = -110327815;    double AtKsXHcTVdxDtQXAd35758865 = -931232015;    double AtKsXHcTVdxDtQXAd51342438 = -222217480;    double AtKsXHcTVdxDtQXAd15998984 = -969465842;    double AtKsXHcTVdxDtQXAd67858955 = -751950807;    double AtKsXHcTVdxDtQXAd11227466 = -163855472;    double AtKsXHcTVdxDtQXAd1699581 = -628041872;    double AtKsXHcTVdxDtQXAd48102533 = -886821915;    double AtKsXHcTVdxDtQXAd28219926 = -920994530;    double AtKsXHcTVdxDtQXAd84359849 = -178561804;    double AtKsXHcTVdxDtQXAd71957386 = -591365086;    double AtKsXHcTVdxDtQXAd14558657 = -244563192;    double AtKsXHcTVdxDtQXAd75527283 = -35197335;    double AtKsXHcTVdxDtQXAd48031095 = -522624872;    double AtKsXHcTVdxDtQXAd40112850 = -843051812;    double AtKsXHcTVdxDtQXAd15732432 = -139866933;    double AtKsXHcTVdxDtQXAd35806087 = -426056018;    double AtKsXHcTVdxDtQXAd18639018 = -495819777;    double AtKsXHcTVdxDtQXAd59230361 = -874320686;     AtKsXHcTVdxDtQXAd16488112 = AtKsXHcTVdxDtQXAd24542924;     AtKsXHcTVdxDtQXAd24542924 = AtKsXHcTVdxDtQXAd9115267;     AtKsXHcTVdxDtQXAd9115267 = AtKsXHcTVdxDtQXAd761583;     AtKsXHcTVdxDtQXAd761583 = AtKsXHcTVdxDtQXAd68523065;     AtKsXHcTVdxDtQXAd68523065 = AtKsXHcTVdxDtQXAd26663018;     AtKsXHcTVdxDtQXAd26663018 = AtKsXHcTVdxDtQXAd7058832;     AtKsXHcTVdxDtQXAd7058832 = AtKsXHcTVdxDtQXAd31591549;     AtKsXHcTVdxDtQXAd31591549 = AtKsXHcTVdxDtQXAd44013882;     AtKsXHcTVdxDtQXAd44013882 = AtKsXHcTVdxDtQXAd15044239;     AtKsXHcTVdxDtQXAd15044239 = AtKsXHcTVdxDtQXAd19011357;     AtKsXHcTVdxDtQXAd19011357 = AtKsXHcTVdxDtQXAd95266826;     AtKsXHcTVdxDtQXAd95266826 = AtKsXHcTVdxDtQXAd91812513;     AtKsXHcTVdxDtQXAd91812513 = AtKsXHcTVdxDtQXAd54679499;     AtKsXHcTVdxDtQXAd54679499 = AtKsXHcTVdxDtQXAd92263437;     AtKsXHcTVdxDtQXAd92263437 = AtKsXHcTVdxDtQXAd90692052;     AtKsXHcTVdxDtQXAd90692052 = AtKsXHcTVdxDtQXAd92798408;     AtKsXHcTVdxDtQXAd92798408 = AtKsXHcTVdxDtQXAd34374542;     AtKsXHcTVdxDtQXAd34374542 = AtKsXHcTVdxDtQXAd57151045;     AtKsXHcTVdxDtQXAd57151045 = AtKsXHcTVdxDtQXAd58873247;     AtKsXHcTVdxDtQXAd58873247 = AtKsXHcTVdxDtQXAd21089659;     AtKsXHcTVdxDtQXAd21089659 = AtKsXHcTVdxDtQXAd99430383;     AtKsXHcTVdxDtQXAd99430383 = AtKsXHcTVdxDtQXAd57590384;     AtKsXHcTVdxDtQXAd57590384 = AtKsXHcTVdxDtQXAd99207465;     AtKsXHcTVdxDtQXAd99207465 = AtKsXHcTVdxDtQXAd43158073;     AtKsXHcTVdxDtQXAd43158073 = AtKsXHcTVdxDtQXAd14666424;     AtKsXHcTVdxDtQXAd14666424 = AtKsXHcTVdxDtQXAd94943876;     AtKsXHcTVdxDtQXAd94943876 = AtKsXHcTVdxDtQXAd38546909;     AtKsXHcTVdxDtQXAd38546909 = AtKsXHcTVdxDtQXAd60420595;     AtKsXHcTVdxDtQXAd60420595 = AtKsXHcTVdxDtQXAd75974642;     AtKsXHcTVdxDtQXAd75974642 = AtKsXHcTVdxDtQXAd14376708;     AtKsXHcTVdxDtQXAd14376708 = AtKsXHcTVdxDtQXAd69829907;     AtKsXHcTVdxDtQXAd69829907 = AtKsXHcTVdxDtQXAd31777296;     AtKsXHcTVdxDtQXAd31777296 = AtKsXHcTVdxDtQXAd42649711;     AtKsXHcTVdxDtQXAd42649711 = AtKsXHcTVdxDtQXAd45277655;     AtKsXHcTVdxDtQXAd45277655 = AtKsXHcTVdxDtQXAd51312389;     AtKsXHcTVdxDtQXAd51312389 = AtKsXHcTVdxDtQXAd81865187;     AtKsXHcTVdxDtQXAd81865187 = AtKsXHcTVdxDtQXAd72162166;     AtKsXHcTVdxDtQXAd72162166 = AtKsXHcTVdxDtQXAd29383925;     AtKsXHcTVdxDtQXAd29383925 = AtKsXHcTVdxDtQXAd79750236;     AtKsXHcTVdxDtQXAd79750236 = AtKsXHcTVdxDtQXAd8624773;     AtKsXHcTVdxDtQXAd8624773 = AtKsXHcTVdxDtQXAd86903385;     AtKsXHcTVdxDtQXAd86903385 = AtKsXHcTVdxDtQXAd3845327;     AtKsXHcTVdxDtQXAd3845327 = AtKsXHcTVdxDtQXAd19306550;     AtKsXHcTVdxDtQXAd19306550 = AtKsXHcTVdxDtQXAd8411811;     AtKsXHcTVdxDtQXAd8411811 = AtKsXHcTVdxDtQXAd74914923;     AtKsXHcTVdxDtQXAd74914923 = AtKsXHcTVdxDtQXAd13125367;     AtKsXHcTVdxDtQXAd13125367 = AtKsXHcTVdxDtQXAd52593618;     AtKsXHcTVdxDtQXAd52593618 = AtKsXHcTVdxDtQXAd2788045;     AtKsXHcTVdxDtQXAd2788045 = AtKsXHcTVdxDtQXAd9078157;     AtKsXHcTVdxDtQXAd9078157 = AtKsXHcTVdxDtQXAd59975658;     AtKsXHcTVdxDtQXAd59975658 = AtKsXHcTVdxDtQXAd46517753;     AtKsXHcTVdxDtQXAd46517753 = AtKsXHcTVdxDtQXAd58602441;     AtKsXHcTVdxDtQXAd58602441 = AtKsXHcTVdxDtQXAd30077715;     AtKsXHcTVdxDtQXAd30077715 = AtKsXHcTVdxDtQXAd94547178;     AtKsXHcTVdxDtQXAd94547178 = AtKsXHcTVdxDtQXAd17057729;     AtKsXHcTVdxDtQXAd17057729 = AtKsXHcTVdxDtQXAd66952539;     AtKsXHcTVdxDtQXAd66952539 = AtKsXHcTVdxDtQXAd9907802;     AtKsXHcTVdxDtQXAd9907802 = AtKsXHcTVdxDtQXAd57603509;     AtKsXHcTVdxDtQXAd57603509 = AtKsXHcTVdxDtQXAd53856642;     AtKsXHcTVdxDtQXAd53856642 = AtKsXHcTVdxDtQXAd31719142;     AtKsXHcTVdxDtQXAd31719142 = AtKsXHcTVdxDtQXAd68511922;     AtKsXHcTVdxDtQXAd68511922 = AtKsXHcTVdxDtQXAd71170954;     AtKsXHcTVdxDtQXAd71170954 = AtKsXHcTVdxDtQXAd68039239;     AtKsXHcTVdxDtQXAd68039239 = AtKsXHcTVdxDtQXAd667532;     AtKsXHcTVdxDtQXAd667532 = AtKsXHcTVdxDtQXAd49181449;     AtKsXHcTVdxDtQXAd49181449 = AtKsXHcTVdxDtQXAd63489531;     AtKsXHcTVdxDtQXAd63489531 = AtKsXHcTVdxDtQXAd49162803;     AtKsXHcTVdxDtQXAd49162803 = AtKsXHcTVdxDtQXAd9401845;     AtKsXHcTVdxDtQXAd9401845 = AtKsXHcTVdxDtQXAd40951049;     AtKsXHcTVdxDtQXAd40951049 = AtKsXHcTVdxDtQXAd8826866;     AtKsXHcTVdxDtQXAd8826866 = AtKsXHcTVdxDtQXAd20636243;     AtKsXHcTVdxDtQXAd20636243 = AtKsXHcTVdxDtQXAd4990617;     AtKsXHcTVdxDtQXAd4990617 = AtKsXHcTVdxDtQXAd77400809;     AtKsXHcTVdxDtQXAd77400809 = AtKsXHcTVdxDtQXAd50248475;     AtKsXHcTVdxDtQXAd50248475 = AtKsXHcTVdxDtQXAd34186274;     AtKsXHcTVdxDtQXAd34186274 = AtKsXHcTVdxDtQXAd95585057;     AtKsXHcTVdxDtQXAd95585057 = AtKsXHcTVdxDtQXAd38283835;     AtKsXHcTVdxDtQXAd38283835 = AtKsXHcTVdxDtQXAd90795655;     AtKsXHcTVdxDtQXAd90795655 = AtKsXHcTVdxDtQXAd68243149;     AtKsXHcTVdxDtQXAd68243149 = AtKsXHcTVdxDtQXAd1541058;     AtKsXHcTVdxDtQXAd1541058 = AtKsXHcTVdxDtQXAd42350258;     AtKsXHcTVdxDtQXAd42350258 = AtKsXHcTVdxDtQXAd35758865;     AtKsXHcTVdxDtQXAd35758865 = AtKsXHcTVdxDtQXAd51342438;     AtKsXHcTVdxDtQXAd51342438 = AtKsXHcTVdxDtQXAd15998984;     AtKsXHcTVdxDtQXAd15998984 = AtKsXHcTVdxDtQXAd67858955;     AtKsXHcTVdxDtQXAd67858955 = AtKsXHcTVdxDtQXAd11227466;     AtKsXHcTVdxDtQXAd11227466 = AtKsXHcTVdxDtQXAd1699581;     AtKsXHcTVdxDtQXAd1699581 = AtKsXHcTVdxDtQXAd48102533;     AtKsXHcTVdxDtQXAd48102533 = AtKsXHcTVdxDtQXAd28219926;     AtKsXHcTVdxDtQXAd28219926 = AtKsXHcTVdxDtQXAd84359849;     AtKsXHcTVdxDtQXAd84359849 = AtKsXHcTVdxDtQXAd71957386;     AtKsXHcTVdxDtQXAd71957386 = AtKsXHcTVdxDtQXAd14558657;     AtKsXHcTVdxDtQXAd14558657 = AtKsXHcTVdxDtQXAd75527283;     AtKsXHcTVdxDtQXAd75527283 = AtKsXHcTVdxDtQXAd48031095;     AtKsXHcTVdxDtQXAd48031095 = AtKsXHcTVdxDtQXAd40112850;     AtKsXHcTVdxDtQXAd40112850 = AtKsXHcTVdxDtQXAd15732432;     AtKsXHcTVdxDtQXAd15732432 = AtKsXHcTVdxDtQXAd35806087;     AtKsXHcTVdxDtQXAd35806087 = AtKsXHcTVdxDtQXAd18639018;     AtKsXHcTVdxDtQXAd18639018 = AtKsXHcTVdxDtQXAd59230361;     AtKsXHcTVdxDtQXAd59230361 = AtKsXHcTVdxDtQXAd16488112;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void gsWIFJaaGTXCsVfR30685905() {     double dOEmXFcpfKkwdVGue22605219 = -986222575;    double dOEmXFcpfKkwdVGue67916203 = -6011580;    double dOEmXFcpfKkwdVGue25157451 = -675723408;    double dOEmXFcpfKkwdVGue27836363 = -510541680;    double dOEmXFcpfKkwdVGue37418554 = -417885661;    double dOEmXFcpfKkwdVGue18080221 = -831762663;    double dOEmXFcpfKkwdVGue67547798 = -682166484;    double dOEmXFcpfKkwdVGue81351440 = -393655959;    double dOEmXFcpfKkwdVGue8656377 = -989911026;    double dOEmXFcpfKkwdVGue56273898 = -665005527;    double dOEmXFcpfKkwdVGue79121647 = -641238200;    double dOEmXFcpfKkwdVGue27089869 = -29050450;    double dOEmXFcpfKkwdVGue78021632 = -129657485;    double dOEmXFcpfKkwdVGue37823176 = -549274611;    double dOEmXFcpfKkwdVGue88005864 = -14990744;    double dOEmXFcpfKkwdVGue32197447 = 10058173;    double dOEmXFcpfKkwdVGue86634411 = -156261480;    double dOEmXFcpfKkwdVGue31427695 = -447408967;    double dOEmXFcpfKkwdVGue18980888 = -833326289;    double dOEmXFcpfKkwdVGue39423412 = -996008574;    double dOEmXFcpfKkwdVGue9642853 = -437993640;    double dOEmXFcpfKkwdVGue85951407 = -228240264;    double dOEmXFcpfKkwdVGue93181175 = -734854522;    double dOEmXFcpfKkwdVGue83199839 = -329303886;    double dOEmXFcpfKkwdVGue86857723 = -750692838;    double dOEmXFcpfKkwdVGue39388099 = -473091056;    double dOEmXFcpfKkwdVGue93403654 = -833808754;    double dOEmXFcpfKkwdVGue51654035 = -741832280;    double dOEmXFcpfKkwdVGue26552851 = -899315854;    double dOEmXFcpfKkwdVGue44590279 = -705978995;    double dOEmXFcpfKkwdVGue6619257 = -710328675;    double dOEmXFcpfKkwdVGue67440529 = -51878142;    double dOEmXFcpfKkwdVGue41488244 = -676520389;    double dOEmXFcpfKkwdVGue37480276 = -404323527;    double dOEmXFcpfKkwdVGue6752307 = 86262226;    double dOEmXFcpfKkwdVGue20038302 = -800662675;    double dOEmXFcpfKkwdVGue63127122 = -962892544;    double dOEmXFcpfKkwdVGue6252379 = 41634553;    double dOEmXFcpfKkwdVGue2994179 = -431920890;    double dOEmXFcpfKkwdVGue67252174 = -630183972;    double dOEmXFcpfKkwdVGue14293997 = -775623887;    double dOEmXFcpfKkwdVGue19053843 = -939858107;    double dOEmXFcpfKkwdVGue81912304 = -840230835;    double dOEmXFcpfKkwdVGue6714290 = -54185027;    double dOEmXFcpfKkwdVGue20364276 = -587678355;    double dOEmXFcpfKkwdVGue86854118 = -372613527;    double dOEmXFcpfKkwdVGue30903941 = -537477894;    double dOEmXFcpfKkwdVGue60550559 = -732714269;    double dOEmXFcpfKkwdVGue12463774 = -830408300;    double dOEmXFcpfKkwdVGue21989810 = -441726478;    double dOEmXFcpfKkwdVGue95676336 = -761335957;    double dOEmXFcpfKkwdVGue57327472 = -845783251;    double dOEmXFcpfKkwdVGue6850899 = -970365442;    double dOEmXFcpfKkwdVGue26046595 = -376335293;    double dOEmXFcpfKkwdVGue21043053 = -366491630;    double dOEmXFcpfKkwdVGue36653812 = -657982311;    double dOEmXFcpfKkwdVGue74735027 = -271157058;    double dOEmXFcpfKkwdVGue41957611 = -246419522;    double dOEmXFcpfKkwdVGue40978640 = -759848843;    double dOEmXFcpfKkwdVGue98030454 = -944794606;    double dOEmXFcpfKkwdVGue24676566 = -997953910;    double dOEmXFcpfKkwdVGue15893764 = -940334205;    double dOEmXFcpfKkwdVGue54798589 = -494340105;    double dOEmXFcpfKkwdVGue64066098 = -183932032;    double dOEmXFcpfKkwdVGue49654641 = -954676852;    double dOEmXFcpfKkwdVGue11681118 = -489360058;    double dOEmXFcpfKkwdVGue85601625 = -352530061;    double dOEmXFcpfKkwdVGue40541357 = -725333958;    double dOEmXFcpfKkwdVGue31070869 = -535536838;    double dOEmXFcpfKkwdVGue67967562 = -214328069;    double dOEmXFcpfKkwdVGue69070324 = -27049284;    double dOEmXFcpfKkwdVGue80382033 = -97896033;    double dOEmXFcpfKkwdVGue28433516 = 84511922;    double dOEmXFcpfKkwdVGue51728713 = -103142317;    double dOEmXFcpfKkwdVGue25129416 = -120384688;    double dOEmXFcpfKkwdVGue90589009 = -498135533;    double dOEmXFcpfKkwdVGue4039103 = -388009429;    double dOEmXFcpfKkwdVGue86466886 = -580669496;    double dOEmXFcpfKkwdVGue62835564 = -741625532;    double dOEmXFcpfKkwdVGue3605 = -278079312;    double dOEmXFcpfKkwdVGue8484159 = -935613163;    double dOEmXFcpfKkwdVGue32853096 = -1094485;    double dOEmXFcpfKkwdVGue39190261 = -911423980;    double dOEmXFcpfKkwdVGue4563041 = -357589376;    double dOEmXFcpfKkwdVGue48913942 = -944643038;    double dOEmXFcpfKkwdVGue49291785 = -864545425;    double dOEmXFcpfKkwdVGue60589630 = -81512700;    double dOEmXFcpfKkwdVGue15441649 = -200185097;    double dOEmXFcpfKkwdVGue16437223 = 62168102;    double dOEmXFcpfKkwdVGue70098495 = -255755463;    double dOEmXFcpfKkwdVGue45303275 = -429505618;    double dOEmXFcpfKkwdVGue21169511 = -616473022;    double dOEmXFcpfKkwdVGue65273738 = -198516605;    double dOEmXFcpfKkwdVGue4963724 = -487126284;    double dOEmXFcpfKkwdVGue42575609 = -632230063;    double dOEmXFcpfKkwdVGue98400233 = -835289683;    double dOEmXFcpfKkwdVGue64255253 = -345518002;    double dOEmXFcpfKkwdVGue17846207 = -556298803;    double dOEmXFcpfKkwdVGue57059648 = -99508176;    double dOEmXFcpfKkwdVGue8683158 = -986222575;     dOEmXFcpfKkwdVGue22605219 = dOEmXFcpfKkwdVGue67916203;     dOEmXFcpfKkwdVGue67916203 = dOEmXFcpfKkwdVGue25157451;     dOEmXFcpfKkwdVGue25157451 = dOEmXFcpfKkwdVGue27836363;     dOEmXFcpfKkwdVGue27836363 = dOEmXFcpfKkwdVGue37418554;     dOEmXFcpfKkwdVGue37418554 = dOEmXFcpfKkwdVGue18080221;     dOEmXFcpfKkwdVGue18080221 = dOEmXFcpfKkwdVGue67547798;     dOEmXFcpfKkwdVGue67547798 = dOEmXFcpfKkwdVGue81351440;     dOEmXFcpfKkwdVGue81351440 = dOEmXFcpfKkwdVGue8656377;     dOEmXFcpfKkwdVGue8656377 = dOEmXFcpfKkwdVGue56273898;     dOEmXFcpfKkwdVGue56273898 = dOEmXFcpfKkwdVGue79121647;     dOEmXFcpfKkwdVGue79121647 = dOEmXFcpfKkwdVGue27089869;     dOEmXFcpfKkwdVGue27089869 = dOEmXFcpfKkwdVGue78021632;     dOEmXFcpfKkwdVGue78021632 = dOEmXFcpfKkwdVGue37823176;     dOEmXFcpfKkwdVGue37823176 = dOEmXFcpfKkwdVGue88005864;     dOEmXFcpfKkwdVGue88005864 = dOEmXFcpfKkwdVGue32197447;     dOEmXFcpfKkwdVGue32197447 = dOEmXFcpfKkwdVGue86634411;     dOEmXFcpfKkwdVGue86634411 = dOEmXFcpfKkwdVGue31427695;     dOEmXFcpfKkwdVGue31427695 = dOEmXFcpfKkwdVGue18980888;     dOEmXFcpfKkwdVGue18980888 = dOEmXFcpfKkwdVGue39423412;     dOEmXFcpfKkwdVGue39423412 = dOEmXFcpfKkwdVGue9642853;     dOEmXFcpfKkwdVGue9642853 = dOEmXFcpfKkwdVGue85951407;     dOEmXFcpfKkwdVGue85951407 = dOEmXFcpfKkwdVGue93181175;     dOEmXFcpfKkwdVGue93181175 = dOEmXFcpfKkwdVGue83199839;     dOEmXFcpfKkwdVGue83199839 = dOEmXFcpfKkwdVGue86857723;     dOEmXFcpfKkwdVGue86857723 = dOEmXFcpfKkwdVGue39388099;     dOEmXFcpfKkwdVGue39388099 = dOEmXFcpfKkwdVGue93403654;     dOEmXFcpfKkwdVGue93403654 = dOEmXFcpfKkwdVGue51654035;     dOEmXFcpfKkwdVGue51654035 = dOEmXFcpfKkwdVGue26552851;     dOEmXFcpfKkwdVGue26552851 = dOEmXFcpfKkwdVGue44590279;     dOEmXFcpfKkwdVGue44590279 = dOEmXFcpfKkwdVGue6619257;     dOEmXFcpfKkwdVGue6619257 = dOEmXFcpfKkwdVGue67440529;     dOEmXFcpfKkwdVGue67440529 = dOEmXFcpfKkwdVGue41488244;     dOEmXFcpfKkwdVGue41488244 = dOEmXFcpfKkwdVGue37480276;     dOEmXFcpfKkwdVGue37480276 = dOEmXFcpfKkwdVGue6752307;     dOEmXFcpfKkwdVGue6752307 = dOEmXFcpfKkwdVGue20038302;     dOEmXFcpfKkwdVGue20038302 = dOEmXFcpfKkwdVGue63127122;     dOEmXFcpfKkwdVGue63127122 = dOEmXFcpfKkwdVGue6252379;     dOEmXFcpfKkwdVGue6252379 = dOEmXFcpfKkwdVGue2994179;     dOEmXFcpfKkwdVGue2994179 = dOEmXFcpfKkwdVGue67252174;     dOEmXFcpfKkwdVGue67252174 = dOEmXFcpfKkwdVGue14293997;     dOEmXFcpfKkwdVGue14293997 = dOEmXFcpfKkwdVGue19053843;     dOEmXFcpfKkwdVGue19053843 = dOEmXFcpfKkwdVGue81912304;     dOEmXFcpfKkwdVGue81912304 = dOEmXFcpfKkwdVGue6714290;     dOEmXFcpfKkwdVGue6714290 = dOEmXFcpfKkwdVGue20364276;     dOEmXFcpfKkwdVGue20364276 = dOEmXFcpfKkwdVGue86854118;     dOEmXFcpfKkwdVGue86854118 = dOEmXFcpfKkwdVGue30903941;     dOEmXFcpfKkwdVGue30903941 = dOEmXFcpfKkwdVGue60550559;     dOEmXFcpfKkwdVGue60550559 = dOEmXFcpfKkwdVGue12463774;     dOEmXFcpfKkwdVGue12463774 = dOEmXFcpfKkwdVGue21989810;     dOEmXFcpfKkwdVGue21989810 = dOEmXFcpfKkwdVGue95676336;     dOEmXFcpfKkwdVGue95676336 = dOEmXFcpfKkwdVGue57327472;     dOEmXFcpfKkwdVGue57327472 = dOEmXFcpfKkwdVGue6850899;     dOEmXFcpfKkwdVGue6850899 = dOEmXFcpfKkwdVGue26046595;     dOEmXFcpfKkwdVGue26046595 = dOEmXFcpfKkwdVGue21043053;     dOEmXFcpfKkwdVGue21043053 = dOEmXFcpfKkwdVGue36653812;     dOEmXFcpfKkwdVGue36653812 = dOEmXFcpfKkwdVGue74735027;     dOEmXFcpfKkwdVGue74735027 = dOEmXFcpfKkwdVGue41957611;     dOEmXFcpfKkwdVGue41957611 = dOEmXFcpfKkwdVGue40978640;     dOEmXFcpfKkwdVGue40978640 = dOEmXFcpfKkwdVGue98030454;     dOEmXFcpfKkwdVGue98030454 = dOEmXFcpfKkwdVGue24676566;     dOEmXFcpfKkwdVGue24676566 = dOEmXFcpfKkwdVGue15893764;     dOEmXFcpfKkwdVGue15893764 = dOEmXFcpfKkwdVGue54798589;     dOEmXFcpfKkwdVGue54798589 = dOEmXFcpfKkwdVGue64066098;     dOEmXFcpfKkwdVGue64066098 = dOEmXFcpfKkwdVGue49654641;     dOEmXFcpfKkwdVGue49654641 = dOEmXFcpfKkwdVGue11681118;     dOEmXFcpfKkwdVGue11681118 = dOEmXFcpfKkwdVGue85601625;     dOEmXFcpfKkwdVGue85601625 = dOEmXFcpfKkwdVGue40541357;     dOEmXFcpfKkwdVGue40541357 = dOEmXFcpfKkwdVGue31070869;     dOEmXFcpfKkwdVGue31070869 = dOEmXFcpfKkwdVGue67967562;     dOEmXFcpfKkwdVGue67967562 = dOEmXFcpfKkwdVGue69070324;     dOEmXFcpfKkwdVGue69070324 = dOEmXFcpfKkwdVGue80382033;     dOEmXFcpfKkwdVGue80382033 = dOEmXFcpfKkwdVGue28433516;     dOEmXFcpfKkwdVGue28433516 = dOEmXFcpfKkwdVGue51728713;     dOEmXFcpfKkwdVGue51728713 = dOEmXFcpfKkwdVGue25129416;     dOEmXFcpfKkwdVGue25129416 = dOEmXFcpfKkwdVGue90589009;     dOEmXFcpfKkwdVGue90589009 = dOEmXFcpfKkwdVGue4039103;     dOEmXFcpfKkwdVGue4039103 = dOEmXFcpfKkwdVGue86466886;     dOEmXFcpfKkwdVGue86466886 = dOEmXFcpfKkwdVGue62835564;     dOEmXFcpfKkwdVGue62835564 = dOEmXFcpfKkwdVGue3605;     dOEmXFcpfKkwdVGue3605 = dOEmXFcpfKkwdVGue8484159;     dOEmXFcpfKkwdVGue8484159 = dOEmXFcpfKkwdVGue32853096;     dOEmXFcpfKkwdVGue32853096 = dOEmXFcpfKkwdVGue39190261;     dOEmXFcpfKkwdVGue39190261 = dOEmXFcpfKkwdVGue4563041;     dOEmXFcpfKkwdVGue4563041 = dOEmXFcpfKkwdVGue48913942;     dOEmXFcpfKkwdVGue48913942 = dOEmXFcpfKkwdVGue49291785;     dOEmXFcpfKkwdVGue49291785 = dOEmXFcpfKkwdVGue60589630;     dOEmXFcpfKkwdVGue60589630 = dOEmXFcpfKkwdVGue15441649;     dOEmXFcpfKkwdVGue15441649 = dOEmXFcpfKkwdVGue16437223;     dOEmXFcpfKkwdVGue16437223 = dOEmXFcpfKkwdVGue70098495;     dOEmXFcpfKkwdVGue70098495 = dOEmXFcpfKkwdVGue45303275;     dOEmXFcpfKkwdVGue45303275 = dOEmXFcpfKkwdVGue21169511;     dOEmXFcpfKkwdVGue21169511 = dOEmXFcpfKkwdVGue65273738;     dOEmXFcpfKkwdVGue65273738 = dOEmXFcpfKkwdVGue4963724;     dOEmXFcpfKkwdVGue4963724 = dOEmXFcpfKkwdVGue42575609;     dOEmXFcpfKkwdVGue42575609 = dOEmXFcpfKkwdVGue98400233;     dOEmXFcpfKkwdVGue98400233 = dOEmXFcpfKkwdVGue64255253;     dOEmXFcpfKkwdVGue64255253 = dOEmXFcpfKkwdVGue17846207;     dOEmXFcpfKkwdVGue17846207 = dOEmXFcpfKkwdVGue57059648;     dOEmXFcpfKkwdVGue57059648 = dOEmXFcpfKkwdVGue8683158;     dOEmXFcpfKkwdVGue8683158 = dOEmXFcpfKkwdVGue22605219;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tdFEmmgrYkOztKft97977504() {     double VoMnmCOksLiZqVBos58064187 = -352499474;    double VoMnmCOksLiZqVBos81188281 = -986563186;    double VoMnmCOksLiZqVBos62543927 = -341390483;    double VoMnmCOksLiZqVBos59719 = -331692752;    double VoMnmCOksLiZqVBos93703823 = -523484507;    double VoMnmCOksLiZqVBos54303092 = 5889441;    double VoMnmCOksLiZqVBos78205204 = -526892218;    double VoMnmCOksLiZqVBos43483001 = -37890099;    double VoMnmCOksLiZqVBos10173983 = -378242433;    double VoMnmCOksLiZqVBos71176610 = -329692525;    double VoMnmCOksLiZqVBos17303382 = -443245929;    double VoMnmCOksLiZqVBos27658018 = -993171221;    double VoMnmCOksLiZqVBos16256278 = -681669990;    double VoMnmCOksLiZqVBos90964039 = -525638021;    double VoMnmCOksLiZqVBos46200381 = -518415759;    double VoMnmCOksLiZqVBos15738748 = -725753434;    double VoMnmCOksLiZqVBos42470348 = -683236032;    double VoMnmCOksLiZqVBos31187886 = -520837036;    double VoMnmCOksLiZqVBos19531218 = -49889710;    double VoMnmCOksLiZqVBos33260630 = -322569640;    double VoMnmCOksLiZqVBos22729716 = -95268328;    double VoMnmCOksLiZqVBos89372931 = -314876558;    double VoMnmCOksLiZqVBos63706556 = -372486707;    double VoMnmCOksLiZqVBos76992784 = -840128491;    double VoMnmCOksLiZqVBos94875674 = -774678037;    double VoMnmCOksLiZqVBos50086271 = -161667863;    double VoMnmCOksLiZqVBos53824229 = -640092895;    double VoMnmCOksLiZqVBos92382459 = -911604454;    double VoMnmCOksLiZqVBos73221065 = -373410657;    double VoMnmCOksLiZqVBos50123690 = -740486165;    double VoMnmCOksLiZqVBos25202489 = -162309162;    double VoMnmCOksLiZqVBos5263548 = -924849557;    double VoMnmCOksLiZqVBos53706255 = -824116367;    double VoMnmCOksLiZqVBos67228702 = -469017340;    double VoMnmCOksLiZqVBos72560760 = -964838025;    double VoMnmCOksLiZqVBos71599145 = 84772019;    double VoMnmCOksLiZqVBos12538939 = -827588584;    double VoMnmCOksLiZqVBos57467782 = 58978972;    double VoMnmCOksLiZqVBos33384740 = -82797166;    double VoMnmCOksLiZqVBos31523707 = -95446518;    double VoMnmCOksLiZqVBos47745604 = -544822801;    double VoMnmCOksLiZqVBos8418970 = -534403398;    double VoMnmCOksLiZqVBos71295682 = -393411621;    double VoMnmCOksLiZqVBos46845628 = -658517437;    double VoMnmCOksLiZqVBos20380734 = -414189516;    double VoMnmCOksLiZqVBos32815447 = -411537159;    double VoMnmCOksLiZqVBos50249104 = 78535746;    double VoMnmCOksLiZqVBos78276871 = -899240863;    double VoMnmCOksLiZqVBos35356440 = -494145180;    double VoMnmCOksLiZqVBos46519399 = -932753294;    double VoMnmCOksLiZqVBos76447060 = -635612514;    double VoMnmCOksLiZqVBos40812945 = 40564403;    double VoMnmCOksLiZqVBos75472047 = -726410996;    double VoMnmCOksLiZqVBos42038442 = -567843793;    double VoMnmCOksLiZqVBos75247789 = -712981775;    double VoMnmCOksLiZqVBos68691255 = 62377083;    double VoMnmCOksLiZqVBos17481726 = -514076480;    double VoMnmCOksLiZqVBos85551142 = -501261993;    double VoMnmCOksLiZqVBos5184044 = -557014715;    double VoMnmCOksLiZqVBos43617552 = -261816644;    double VoMnmCOksLiZqVBos478864 = -354017664;    double VoMnmCOksLiZqVBos85822744 = -615287764;    double VoMnmCOksLiZqVBos70261936 = -664479443;    double VoMnmCOksLiZqVBos60050292 = -637756269;    double VoMnmCOksLiZqVBos45974122 = -67383363;    double VoMnmCOksLiZqVBos12039835 = -518396373;    double VoMnmCOksLiZqVBos73951762 = -69054855;    double VoMnmCOksLiZqVBos49027576 = -112652650;    double VoMnmCOksLiZqVBos18403279 = -560799996;    double VoMnmCOksLiZqVBos74601235 = -503187778;    double VoMnmCOksLiZqVBos3199809 = -898164851;    double VoMnmCOksLiZqVBos85002565 = -642215005;    double VoMnmCOksLiZqVBos97803146 = -338039870;    double VoMnmCOksLiZqVBos88007510 = -954443193;    double VoMnmCOksLiZqVBos85515026 = -777746840;    double VoMnmCOksLiZqVBos14310746 = -560864931;    double VoMnmCOksLiZqVBos18077249 = -921464937;    double VoMnmCOksLiZqVBos16860928 = -713969270;    double VoMnmCOksLiZqVBos56612051 = -325938975;    double VoMnmCOksLiZqVBos62060227 = -263140878;    double VoMnmCOksLiZqVBos99837166 = -140203609;    double VoMnmCOksLiZqVBos75547357 = -740852033;    double VoMnmCOksLiZqVBos57026020 = -317459274;    double VoMnmCOksLiZqVBos26701666 = -440657363;    double VoMnmCOksLiZqVBos73676629 = -4873651;    double VoMnmCOksLiZqVBos84389543 = -102873565;    double VoMnmCOksLiZqVBos29791501 = -98438561;    double VoMnmCOksLiZqVBos11667814 = -156272574;    double VoMnmCOksLiZqVBos91980912 = -756035566;    double VoMnmCOksLiZqVBos3869506 = -927215108;    double VoMnmCOksLiZqVBos54117420 = -401151501;    double VoMnmCOksLiZqVBos26987796 = -226326592;    double VoMnmCOksLiZqVBos52283738 = -384006313;    double VoMnmCOksLiZqVBos89767187 = -820980522;    double VoMnmCOksLiZqVBos31044843 = -741428854;    double VoMnmCOksLiZqVBos61922859 = -929535037;    double VoMnmCOksLiZqVBos38157034 = -869923955;    double VoMnmCOksLiZqVBos11245391 = -755655353;    double VoMnmCOksLiZqVBos871507 = -491134075;    double VoMnmCOksLiZqVBos8340900 = -352499474;     VoMnmCOksLiZqVBos58064187 = VoMnmCOksLiZqVBos81188281;     VoMnmCOksLiZqVBos81188281 = VoMnmCOksLiZqVBos62543927;     VoMnmCOksLiZqVBos62543927 = VoMnmCOksLiZqVBos59719;     VoMnmCOksLiZqVBos59719 = VoMnmCOksLiZqVBos93703823;     VoMnmCOksLiZqVBos93703823 = VoMnmCOksLiZqVBos54303092;     VoMnmCOksLiZqVBos54303092 = VoMnmCOksLiZqVBos78205204;     VoMnmCOksLiZqVBos78205204 = VoMnmCOksLiZqVBos43483001;     VoMnmCOksLiZqVBos43483001 = VoMnmCOksLiZqVBos10173983;     VoMnmCOksLiZqVBos10173983 = VoMnmCOksLiZqVBos71176610;     VoMnmCOksLiZqVBos71176610 = VoMnmCOksLiZqVBos17303382;     VoMnmCOksLiZqVBos17303382 = VoMnmCOksLiZqVBos27658018;     VoMnmCOksLiZqVBos27658018 = VoMnmCOksLiZqVBos16256278;     VoMnmCOksLiZqVBos16256278 = VoMnmCOksLiZqVBos90964039;     VoMnmCOksLiZqVBos90964039 = VoMnmCOksLiZqVBos46200381;     VoMnmCOksLiZqVBos46200381 = VoMnmCOksLiZqVBos15738748;     VoMnmCOksLiZqVBos15738748 = VoMnmCOksLiZqVBos42470348;     VoMnmCOksLiZqVBos42470348 = VoMnmCOksLiZqVBos31187886;     VoMnmCOksLiZqVBos31187886 = VoMnmCOksLiZqVBos19531218;     VoMnmCOksLiZqVBos19531218 = VoMnmCOksLiZqVBos33260630;     VoMnmCOksLiZqVBos33260630 = VoMnmCOksLiZqVBos22729716;     VoMnmCOksLiZqVBos22729716 = VoMnmCOksLiZqVBos89372931;     VoMnmCOksLiZqVBos89372931 = VoMnmCOksLiZqVBos63706556;     VoMnmCOksLiZqVBos63706556 = VoMnmCOksLiZqVBos76992784;     VoMnmCOksLiZqVBos76992784 = VoMnmCOksLiZqVBos94875674;     VoMnmCOksLiZqVBos94875674 = VoMnmCOksLiZqVBos50086271;     VoMnmCOksLiZqVBos50086271 = VoMnmCOksLiZqVBos53824229;     VoMnmCOksLiZqVBos53824229 = VoMnmCOksLiZqVBos92382459;     VoMnmCOksLiZqVBos92382459 = VoMnmCOksLiZqVBos73221065;     VoMnmCOksLiZqVBos73221065 = VoMnmCOksLiZqVBos50123690;     VoMnmCOksLiZqVBos50123690 = VoMnmCOksLiZqVBos25202489;     VoMnmCOksLiZqVBos25202489 = VoMnmCOksLiZqVBos5263548;     VoMnmCOksLiZqVBos5263548 = VoMnmCOksLiZqVBos53706255;     VoMnmCOksLiZqVBos53706255 = VoMnmCOksLiZqVBos67228702;     VoMnmCOksLiZqVBos67228702 = VoMnmCOksLiZqVBos72560760;     VoMnmCOksLiZqVBos72560760 = VoMnmCOksLiZqVBos71599145;     VoMnmCOksLiZqVBos71599145 = VoMnmCOksLiZqVBos12538939;     VoMnmCOksLiZqVBos12538939 = VoMnmCOksLiZqVBos57467782;     VoMnmCOksLiZqVBos57467782 = VoMnmCOksLiZqVBos33384740;     VoMnmCOksLiZqVBos33384740 = VoMnmCOksLiZqVBos31523707;     VoMnmCOksLiZqVBos31523707 = VoMnmCOksLiZqVBos47745604;     VoMnmCOksLiZqVBos47745604 = VoMnmCOksLiZqVBos8418970;     VoMnmCOksLiZqVBos8418970 = VoMnmCOksLiZqVBos71295682;     VoMnmCOksLiZqVBos71295682 = VoMnmCOksLiZqVBos46845628;     VoMnmCOksLiZqVBos46845628 = VoMnmCOksLiZqVBos20380734;     VoMnmCOksLiZqVBos20380734 = VoMnmCOksLiZqVBos32815447;     VoMnmCOksLiZqVBos32815447 = VoMnmCOksLiZqVBos50249104;     VoMnmCOksLiZqVBos50249104 = VoMnmCOksLiZqVBos78276871;     VoMnmCOksLiZqVBos78276871 = VoMnmCOksLiZqVBos35356440;     VoMnmCOksLiZqVBos35356440 = VoMnmCOksLiZqVBos46519399;     VoMnmCOksLiZqVBos46519399 = VoMnmCOksLiZqVBos76447060;     VoMnmCOksLiZqVBos76447060 = VoMnmCOksLiZqVBos40812945;     VoMnmCOksLiZqVBos40812945 = VoMnmCOksLiZqVBos75472047;     VoMnmCOksLiZqVBos75472047 = VoMnmCOksLiZqVBos42038442;     VoMnmCOksLiZqVBos42038442 = VoMnmCOksLiZqVBos75247789;     VoMnmCOksLiZqVBos75247789 = VoMnmCOksLiZqVBos68691255;     VoMnmCOksLiZqVBos68691255 = VoMnmCOksLiZqVBos17481726;     VoMnmCOksLiZqVBos17481726 = VoMnmCOksLiZqVBos85551142;     VoMnmCOksLiZqVBos85551142 = VoMnmCOksLiZqVBos5184044;     VoMnmCOksLiZqVBos5184044 = VoMnmCOksLiZqVBos43617552;     VoMnmCOksLiZqVBos43617552 = VoMnmCOksLiZqVBos478864;     VoMnmCOksLiZqVBos478864 = VoMnmCOksLiZqVBos85822744;     VoMnmCOksLiZqVBos85822744 = VoMnmCOksLiZqVBos70261936;     VoMnmCOksLiZqVBos70261936 = VoMnmCOksLiZqVBos60050292;     VoMnmCOksLiZqVBos60050292 = VoMnmCOksLiZqVBos45974122;     VoMnmCOksLiZqVBos45974122 = VoMnmCOksLiZqVBos12039835;     VoMnmCOksLiZqVBos12039835 = VoMnmCOksLiZqVBos73951762;     VoMnmCOksLiZqVBos73951762 = VoMnmCOksLiZqVBos49027576;     VoMnmCOksLiZqVBos49027576 = VoMnmCOksLiZqVBos18403279;     VoMnmCOksLiZqVBos18403279 = VoMnmCOksLiZqVBos74601235;     VoMnmCOksLiZqVBos74601235 = VoMnmCOksLiZqVBos3199809;     VoMnmCOksLiZqVBos3199809 = VoMnmCOksLiZqVBos85002565;     VoMnmCOksLiZqVBos85002565 = VoMnmCOksLiZqVBos97803146;     VoMnmCOksLiZqVBos97803146 = VoMnmCOksLiZqVBos88007510;     VoMnmCOksLiZqVBos88007510 = VoMnmCOksLiZqVBos85515026;     VoMnmCOksLiZqVBos85515026 = VoMnmCOksLiZqVBos14310746;     VoMnmCOksLiZqVBos14310746 = VoMnmCOksLiZqVBos18077249;     VoMnmCOksLiZqVBos18077249 = VoMnmCOksLiZqVBos16860928;     VoMnmCOksLiZqVBos16860928 = VoMnmCOksLiZqVBos56612051;     VoMnmCOksLiZqVBos56612051 = VoMnmCOksLiZqVBos62060227;     VoMnmCOksLiZqVBos62060227 = VoMnmCOksLiZqVBos99837166;     VoMnmCOksLiZqVBos99837166 = VoMnmCOksLiZqVBos75547357;     VoMnmCOksLiZqVBos75547357 = VoMnmCOksLiZqVBos57026020;     VoMnmCOksLiZqVBos57026020 = VoMnmCOksLiZqVBos26701666;     VoMnmCOksLiZqVBos26701666 = VoMnmCOksLiZqVBos73676629;     VoMnmCOksLiZqVBos73676629 = VoMnmCOksLiZqVBos84389543;     VoMnmCOksLiZqVBos84389543 = VoMnmCOksLiZqVBos29791501;     VoMnmCOksLiZqVBos29791501 = VoMnmCOksLiZqVBos11667814;     VoMnmCOksLiZqVBos11667814 = VoMnmCOksLiZqVBos91980912;     VoMnmCOksLiZqVBos91980912 = VoMnmCOksLiZqVBos3869506;     VoMnmCOksLiZqVBos3869506 = VoMnmCOksLiZqVBos54117420;     VoMnmCOksLiZqVBos54117420 = VoMnmCOksLiZqVBos26987796;     VoMnmCOksLiZqVBos26987796 = VoMnmCOksLiZqVBos52283738;     VoMnmCOksLiZqVBos52283738 = VoMnmCOksLiZqVBos89767187;     VoMnmCOksLiZqVBos89767187 = VoMnmCOksLiZqVBos31044843;     VoMnmCOksLiZqVBos31044843 = VoMnmCOksLiZqVBos61922859;     VoMnmCOksLiZqVBos61922859 = VoMnmCOksLiZqVBos38157034;     VoMnmCOksLiZqVBos38157034 = VoMnmCOksLiZqVBos11245391;     VoMnmCOksLiZqVBos11245391 = VoMnmCOksLiZqVBos871507;     VoMnmCOksLiZqVBos871507 = VoMnmCOksLiZqVBos8340900;     VoMnmCOksLiZqVBos8340900 = VoMnmCOksLiZqVBos58064187;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void OHwfuFJtXdNRkzLw13026573() {     double aBWNJhFyXzNRotRGu64181294 = -464401363;    double aBWNJhFyXzNRotRGu24561561 = -696162967;    double aBWNJhFyXzNRotRGu78586111 = -492223404;    double aBWNJhFyXzNRotRGu27134499 = -522441382;    double aBWNJhFyXzNRotRGu62599312 = 55865622;    double aBWNJhFyXzNRotRGu45720295 = -258606794;    double aBWNJhFyXzNRotRGu38694170 = -665857195;    double aBWNJhFyXzNRotRGu93242891 = -324764012;    double aBWNJhFyXzNRotRGu74816477 = -146275099;    double aBWNJhFyXzNRotRGu12406270 = -658668002;    double aBWNJhFyXzNRotRGu77413672 = -320683516;    double aBWNJhFyXzNRotRGu59481060 = -337577423;    double aBWNJhFyXzNRotRGu2465397 = -21561078;    double aBWNJhFyXzNRotRGu74107716 = 42123411;    double aBWNJhFyXzNRotRGu41942807 = -108877925;    double aBWNJhFyXzNRotRGu57244141 = -698752977;    double aBWNJhFyXzNRotRGu36306351 = -199781145;    double aBWNJhFyXzNRotRGu28241039 = -186359678;    double aBWNJhFyXzNRotRGu81361059 = -455658807;    double aBWNJhFyXzNRotRGu13810795 = -679472560;    double aBWNJhFyXzNRotRGu11282909 = 49244216;    double aBWNJhFyXzNRotRGu75893955 = -222806274;    double aBWNJhFyXzNRotRGu99297347 = -119004193;    double aBWNJhFyXzNRotRGu60985158 = 37566522;    double aBWNJhFyXzNRotRGu38575325 = -47874070;    double aBWNJhFyXzNRotRGu74807946 = -40323186;    double aBWNJhFyXzNRotRGu52284007 = -540755098;    double aBWNJhFyXzNRotRGu5489585 = -977533408;    double aBWNJhFyXzNRotRGu39353321 = -595793361;    double aBWNJhFyXzNRotRGu18739327 = -557957033;    double aBWNJhFyXzNRotRGu17445038 = -75630100;    double aBWNJhFyXzNRotRGu2874170 = -634296372;    double aBWNJhFyXzNRotRGu63417203 = -716729737;    double aBWNJhFyXzNRotRGu62059266 = -220559106;    double aBWNJhFyXzNRotRGu34035413 = -503571130;    double aBWNJhFyXzNRotRGu40325059 = -129254089;    double aBWNJhFyXzNRotRGu93800873 = -781224454;    double aBWNJhFyXzNRotRGu91557994 = -812527037;    double aBWNJhFyXzNRotRGu6994994 = -76720663;    double aBWNJhFyXzNRotRGu19025645 = -568885741;    double aBWNJhFyXzNRotRGu53414828 = -610096695;    double aBWNJhFyXzNRotRGu40569426 = -804545676;    double aBWNJhFyXzNRotRGu49362661 = -474216205;    double aBWNJhFyXzNRotRGu34253369 = -677860375;    double aBWNJhFyXzNRotRGu32333199 = -76087671;    double aBWNJhFyXzNRotRGu44754642 = -396753275;    double aBWNJhFyXzNRotRGu68027678 = -40239792;    double aBWNJhFyXzNRotRGu86233812 = -909136395;    double aBWNJhFyXzNRotRGu45032168 = -579882169;    double aBWNJhFyXzNRotRGu59431052 = 80235898;    double aBWNJhFyXzNRotRGu12147739 = -477906186;    double aBWNJhFyXzNRotRGu51622664 = -860161917;    double aBWNJhFyXzNRotRGu23720505 = -518200584;    double aBWNJhFyXzNRotRGu38007322 = -888313938;    double aBWNJhFyXzNRotRGu1743664 = -313513558;    double aBWNJhFyXzNRotRGu88287338 = -141595090;    double aBWNJhFyXzNRotRGu25264213 = -477158774;    double aBWNJhFyXzNRotRGu17600953 = -429789926;    double aBWNJhFyXzNRotRGu88559174 = -374567313;    double aBWNJhFyXzNRotRGu87791365 = -903811192;    double aBWNJhFyXzNRotRGu93436287 = -717851696;    double aBWNJhFyXzNRotRGu33204585 = -688323788;    double aBWNJhFyXzNRotRGu53889571 = -728970652;    double aBWNJhFyXzNRotRGu56077151 = -588318067;    double aBWNJhFyXzNRotRGu94961231 = -483037902;    double aBWNJhFyXzNRotRGu74539502 = -686387144;    double aBWNJhFyXzNRotRGu96063856 = -620847687;    double aBWNJhFyXzNRotRGu40406130 = -801001973;    double aBWNJhFyXzNRotRGu40072303 = -454305459;    double aBWNJhFyXzNRotRGu1617749 = -979623837;    double aBWNJhFyXzNRotRGu63443268 = -917528524;    double aBWNJhFyXzNRotRGu44748356 = -387254108;    double aBWNJhFyXzNRotRGu21246046 = -9639016;    double aBWNJhFyXzNRotRGu62335414 = -886773067;    double aBWNJhFyXzNRotRGu60395967 = 30624135;    double aBWNJhFyXzNRotRGu70713482 = -146210109;    double aBWNJhFyXzNRotRGu26531295 = -748590070;    double aBWNJhFyXzNRotRGu65043979 = -441143819;    double aBWNJhFyXzNRotRGu28651960 = -886345808;    double aBWNJhFyXzNRotRGu93820682 = -651120795;    double aBWNJhFyXzNRotRGu6780268 = 99916605;    double aBWNJhFyXzNRotRGu66050195 = -631618703;    double aBWNJhFyXzNRotRGu60457416 = -297651240;    double aBWNJhFyXzNRotRGu79922269 = -576029259;    double aBWNJhFyXzNRotRGu6591588 = 19949153;    double aBWNJhFyXzNRotRGu65822374 = -215468183;    double aBWNJhFyXzNRotRGu79153665 = -16095789;    double aBWNJhFyXzNRotRGu25409882 = -828415800;    double aBWNJhFyXzNRotRGu60315602 = -907045548;    double aBWNJhFyXzNRotRGu45748074 = -261976041;    double aBWNJhFyXzNRotRGu15060846 = -652095315;    double aBWNJhFyXzNRotRGu76199921 = -251434528;    double aBWNJhFyXzNRotRGu2998820 = -337959725;    double aBWNJhFyXzNRotRGu19203628 = -172909471;    double aBWNJhFyXzNRotRGu25589357 = -851034045;    double aBWNJhFyXzNRotRGu20210243 = -921772908;    double aBWNJhFyXzNRotRGu86679855 = 24424975;    double aBWNJhFyXzNRotRGu93285510 = -885898139;    double aBWNJhFyXzNRotRGu39292137 = -94822473;    double aBWNJhFyXzNRotRGu57793696 = -464401363;     aBWNJhFyXzNRotRGu64181294 = aBWNJhFyXzNRotRGu24561561;     aBWNJhFyXzNRotRGu24561561 = aBWNJhFyXzNRotRGu78586111;     aBWNJhFyXzNRotRGu78586111 = aBWNJhFyXzNRotRGu27134499;     aBWNJhFyXzNRotRGu27134499 = aBWNJhFyXzNRotRGu62599312;     aBWNJhFyXzNRotRGu62599312 = aBWNJhFyXzNRotRGu45720295;     aBWNJhFyXzNRotRGu45720295 = aBWNJhFyXzNRotRGu38694170;     aBWNJhFyXzNRotRGu38694170 = aBWNJhFyXzNRotRGu93242891;     aBWNJhFyXzNRotRGu93242891 = aBWNJhFyXzNRotRGu74816477;     aBWNJhFyXzNRotRGu74816477 = aBWNJhFyXzNRotRGu12406270;     aBWNJhFyXzNRotRGu12406270 = aBWNJhFyXzNRotRGu77413672;     aBWNJhFyXzNRotRGu77413672 = aBWNJhFyXzNRotRGu59481060;     aBWNJhFyXzNRotRGu59481060 = aBWNJhFyXzNRotRGu2465397;     aBWNJhFyXzNRotRGu2465397 = aBWNJhFyXzNRotRGu74107716;     aBWNJhFyXzNRotRGu74107716 = aBWNJhFyXzNRotRGu41942807;     aBWNJhFyXzNRotRGu41942807 = aBWNJhFyXzNRotRGu57244141;     aBWNJhFyXzNRotRGu57244141 = aBWNJhFyXzNRotRGu36306351;     aBWNJhFyXzNRotRGu36306351 = aBWNJhFyXzNRotRGu28241039;     aBWNJhFyXzNRotRGu28241039 = aBWNJhFyXzNRotRGu81361059;     aBWNJhFyXzNRotRGu81361059 = aBWNJhFyXzNRotRGu13810795;     aBWNJhFyXzNRotRGu13810795 = aBWNJhFyXzNRotRGu11282909;     aBWNJhFyXzNRotRGu11282909 = aBWNJhFyXzNRotRGu75893955;     aBWNJhFyXzNRotRGu75893955 = aBWNJhFyXzNRotRGu99297347;     aBWNJhFyXzNRotRGu99297347 = aBWNJhFyXzNRotRGu60985158;     aBWNJhFyXzNRotRGu60985158 = aBWNJhFyXzNRotRGu38575325;     aBWNJhFyXzNRotRGu38575325 = aBWNJhFyXzNRotRGu74807946;     aBWNJhFyXzNRotRGu74807946 = aBWNJhFyXzNRotRGu52284007;     aBWNJhFyXzNRotRGu52284007 = aBWNJhFyXzNRotRGu5489585;     aBWNJhFyXzNRotRGu5489585 = aBWNJhFyXzNRotRGu39353321;     aBWNJhFyXzNRotRGu39353321 = aBWNJhFyXzNRotRGu18739327;     aBWNJhFyXzNRotRGu18739327 = aBWNJhFyXzNRotRGu17445038;     aBWNJhFyXzNRotRGu17445038 = aBWNJhFyXzNRotRGu2874170;     aBWNJhFyXzNRotRGu2874170 = aBWNJhFyXzNRotRGu63417203;     aBWNJhFyXzNRotRGu63417203 = aBWNJhFyXzNRotRGu62059266;     aBWNJhFyXzNRotRGu62059266 = aBWNJhFyXzNRotRGu34035413;     aBWNJhFyXzNRotRGu34035413 = aBWNJhFyXzNRotRGu40325059;     aBWNJhFyXzNRotRGu40325059 = aBWNJhFyXzNRotRGu93800873;     aBWNJhFyXzNRotRGu93800873 = aBWNJhFyXzNRotRGu91557994;     aBWNJhFyXzNRotRGu91557994 = aBWNJhFyXzNRotRGu6994994;     aBWNJhFyXzNRotRGu6994994 = aBWNJhFyXzNRotRGu19025645;     aBWNJhFyXzNRotRGu19025645 = aBWNJhFyXzNRotRGu53414828;     aBWNJhFyXzNRotRGu53414828 = aBWNJhFyXzNRotRGu40569426;     aBWNJhFyXzNRotRGu40569426 = aBWNJhFyXzNRotRGu49362661;     aBWNJhFyXzNRotRGu49362661 = aBWNJhFyXzNRotRGu34253369;     aBWNJhFyXzNRotRGu34253369 = aBWNJhFyXzNRotRGu32333199;     aBWNJhFyXzNRotRGu32333199 = aBWNJhFyXzNRotRGu44754642;     aBWNJhFyXzNRotRGu44754642 = aBWNJhFyXzNRotRGu68027678;     aBWNJhFyXzNRotRGu68027678 = aBWNJhFyXzNRotRGu86233812;     aBWNJhFyXzNRotRGu86233812 = aBWNJhFyXzNRotRGu45032168;     aBWNJhFyXzNRotRGu45032168 = aBWNJhFyXzNRotRGu59431052;     aBWNJhFyXzNRotRGu59431052 = aBWNJhFyXzNRotRGu12147739;     aBWNJhFyXzNRotRGu12147739 = aBWNJhFyXzNRotRGu51622664;     aBWNJhFyXzNRotRGu51622664 = aBWNJhFyXzNRotRGu23720505;     aBWNJhFyXzNRotRGu23720505 = aBWNJhFyXzNRotRGu38007322;     aBWNJhFyXzNRotRGu38007322 = aBWNJhFyXzNRotRGu1743664;     aBWNJhFyXzNRotRGu1743664 = aBWNJhFyXzNRotRGu88287338;     aBWNJhFyXzNRotRGu88287338 = aBWNJhFyXzNRotRGu25264213;     aBWNJhFyXzNRotRGu25264213 = aBWNJhFyXzNRotRGu17600953;     aBWNJhFyXzNRotRGu17600953 = aBWNJhFyXzNRotRGu88559174;     aBWNJhFyXzNRotRGu88559174 = aBWNJhFyXzNRotRGu87791365;     aBWNJhFyXzNRotRGu87791365 = aBWNJhFyXzNRotRGu93436287;     aBWNJhFyXzNRotRGu93436287 = aBWNJhFyXzNRotRGu33204585;     aBWNJhFyXzNRotRGu33204585 = aBWNJhFyXzNRotRGu53889571;     aBWNJhFyXzNRotRGu53889571 = aBWNJhFyXzNRotRGu56077151;     aBWNJhFyXzNRotRGu56077151 = aBWNJhFyXzNRotRGu94961231;     aBWNJhFyXzNRotRGu94961231 = aBWNJhFyXzNRotRGu74539502;     aBWNJhFyXzNRotRGu74539502 = aBWNJhFyXzNRotRGu96063856;     aBWNJhFyXzNRotRGu96063856 = aBWNJhFyXzNRotRGu40406130;     aBWNJhFyXzNRotRGu40406130 = aBWNJhFyXzNRotRGu40072303;     aBWNJhFyXzNRotRGu40072303 = aBWNJhFyXzNRotRGu1617749;     aBWNJhFyXzNRotRGu1617749 = aBWNJhFyXzNRotRGu63443268;     aBWNJhFyXzNRotRGu63443268 = aBWNJhFyXzNRotRGu44748356;     aBWNJhFyXzNRotRGu44748356 = aBWNJhFyXzNRotRGu21246046;     aBWNJhFyXzNRotRGu21246046 = aBWNJhFyXzNRotRGu62335414;     aBWNJhFyXzNRotRGu62335414 = aBWNJhFyXzNRotRGu60395967;     aBWNJhFyXzNRotRGu60395967 = aBWNJhFyXzNRotRGu70713482;     aBWNJhFyXzNRotRGu70713482 = aBWNJhFyXzNRotRGu26531295;     aBWNJhFyXzNRotRGu26531295 = aBWNJhFyXzNRotRGu65043979;     aBWNJhFyXzNRotRGu65043979 = aBWNJhFyXzNRotRGu28651960;     aBWNJhFyXzNRotRGu28651960 = aBWNJhFyXzNRotRGu93820682;     aBWNJhFyXzNRotRGu93820682 = aBWNJhFyXzNRotRGu6780268;     aBWNJhFyXzNRotRGu6780268 = aBWNJhFyXzNRotRGu66050195;     aBWNJhFyXzNRotRGu66050195 = aBWNJhFyXzNRotRGu60457416;     aBWNJhFyXzNRotRGu60457416 = aBWNJhFyXzNRotRGu79922269;     aBWNJhFyXzNRotRGu79922269 = aBWNJhFyXzNRotRGu6591588;     aBWNJhFyXzNRotRGu6591588 = aBWNJhFyXzNRotRGu65822374;     aBWNJhFyXzNRotRGu65822374 = aBWNJhFyXzNRotRGu79153665;     aBWNJhFyXzNRotRGu79153665 = aBWNJhFyXzNRotRGu25409882;     aBWNJhFyXzNRotRGu25409882 = aBWNJhFyXzNRotRGu60315602;     aBWNJhFyXzNRotRGu60315602 = aBWNJhFyXzNRotRGu45748074;     aBWNJhFyXzNRotRGu45748074 = aBWNJhFyXzNRotRGu15060846;     aBWNJhFyXzNRotRGu15060846 = aBWNJhFyXzNRotRGu76199921;     aBWNJhFyXzNRotRGu76199921 = aBWNJhFyXzNRotRGu2998820;     aBWNJhFyXzNRotRGu2998820 = aBWNJhFyXzNRotRGu19203628;     aBWNJhFyXzNRotRGu19203628 = aBWNJhFyXzNRotRGu25589357;     aBWNJhFyXzNRotRGu25589357 = aBWNJhFyXzNRotRGu20210243;     aBWNJhFyXzNRotRGu20210243 = aBWNJhFyXzNRotRGu86679855;     aBWNJhFyXzNRotRGu86679855 = aBWNJhFyXzNRotRGu93285510;     aBWNJhFyXzNRotRGu93285510 = aBWNJhFyXzNRotRGu39292137;     aBWNJhFyXzNRotRGu39292137 = aBWNJhFyXzNRotRGu57793696;     aBWNJhFyXzNRotRGu57793696 = aBWNJhFyXzNRotRGu64181294;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YoEDsvOplOloLakt80318171() {     double zcfQCvZZXledTwvkj99640262 = -930678263;    double zcfQCvZZXledTwvkj37833639 = -576714574;    double zcfQCvZZXledTwvkj15972588 = -157890479;    double zcfQCvZZXledTwvkj99357853 = -343592453;    double zcfQCvZZXledTwvkj18884581 = -49733223;    double zcfQCvZZXledTwvkj81943167 = -520954689;    double zcfQCvZZXledTwvkj49351576 = -510582930;    double zcfQCvZZXledTwvkj55374453 = 31001847;    double zcfQCvZZXledTwvkj76334083 = -634606506;    double zcfQCvZZXledTwvkj27308982 = -323355000;    double zcfQCvZZXledTwvkj15595407 = -122691245;    double zcfQCvZZXledTwvkj60049208 = -201698195;    double zcfQCvZZXledTwvkj40700042 = -573573583;    double zcfQCvZZXledTwvkj27248579 = 65760002;    double zcfQCvZZXledTwvkj137324 = -612302940;    double zcfQCvZZXledTwvkj40785442 = -334564584;    double zcfQCvZZXledTwvkj92142286 = -726755698;    double zcfQCvZZXledTwvkj28001231 = -259787747;    double zcfQCvZZXledTwvkj81911389 = -772222228;    double zcfQCvZZXledTwvkj7648014 = -6033626;    double zcfQCvZZXledTwvkj24369772 = -708030473;    double zcfQCvZZXledTwvkj79315480 = -309442568;    double zcfQCvZZXledTwvkj69822727 = -856636377;    double zcfQCvZZXledTwvkj54778104 = -473258083;    double zcfQCvZZXledTwvkj46593276 = -71859268;    double zcfQCvZZXledTwvkj85506118 = -828899993;    double zcfQCvZZXledTwvkj12704581 = -347039239;    double zcfQCvZZXledTwvkj46218010 = -47305582;    double zcfQCvZZXledTwvkj86021535 = -69888164;    double zcfQCvZZXledTwvkj24272738 = -592464203;    double zcfQCvZZXledTwvkj36028270 = -627610587;    double zcfQCvZZXledTwvkj40697188 = -407267787;    double zcfQCvZZXledTwvkj75635214 = -864325714;    double zcfQCvZZXledTwvkj91807692 = -285252919;    double zcfQCvZZXledTwvkj99843866 = -454671381;    double zcfQCvZZXledTwvkj91885902 = -343819395;    double zcfQCvZZXledTwvkj43212690 = -645920494;    double zcfQCvZZXledTwvkj42773398 = -795182619;    double zcfQCvZZXledTwvkj37385555 = -827596939;    double zcfQCvZZXledTwvkj83297176 = -34148287;    double zcfQCvZZXledTwvkj86866435 = -379295609;    double zcfQCvZZXledTwvkj29934553 = -399090967;    double zcfQCvZZXledTwvkj38746039 = -27396991;    double zcfQCvZZXledTwvkj74384707 = -182192785;    double zcfQCvZZXledTwvkj32349658 = 97401168;    double zcfQCvZZXledTwvkj90715970 = -435676908;    double zcfQCvZZXledTwvkj87372842 = -524226152;    double zcfQCvZZXledTwvkj3960124 = 24337012;    double zcfQCvZZXledTwvkj67924834 = -243619049;    double zcfQCvZZXledTwvkj83960641 = -410790918;    double zcfQCvZZXledTwvkj92918462 = -352182743;    double zcfQCvZZXledTwvkj35108137 = 26185736;    double zcfQCvZZXledTwvkj92341653 = -274246138;    double zcfQCvZZXledTwvkj53999168 = 20177563;    double zcfQCvZZXledTwvkj55948400 = -660003704;    double zcfQCvZZXledTwvkj20324782 = -521235695;    double zcfQCvZZXledTwvkj68010912 = -720078197;    double zcfQCvZZXledTwvkj61194484 = -684632397;    double zcfQCvZZXledTwvkj52764578 = -171733186;    double zcfQCvZZXledTwvkj33378463 = -220833231;    double zcfQCvZZXledTwvkj69238586 = -73915451;    double zcfQCvZZXledTwvkj3133567 = -363277348;    double zcfQCvZZXledTwvkj69352917 = -899109990;    double zcfQCvZZXledTwvkj52061345 = 57857697;    double zcfQCvZZXledTwvkj91280712 = -695744413;    double zcfQCvZZXledTwvkj74898219 = -715423459;    double zcfQCvZZXledTwvkj84413994 = -337372481;    double zcfQCvZZXledTwvkj48892349 = -188320665;    double zcfQCvZZXledTwvkj27404713 = -479568617;    double zcfQCvZZXledTwvkj8251421 = -168483546;    double zcfQCvZZXledTwvkj97572752 = -688644091;    double zcfQCvZZXledTwvkj49368889 = -931573080;    double zcfQCvZZXledTwvkj90615675 = -432190808;    double zcfQCvZZXledTwvkj98614212 = -638073942;    double zcfQCvZZXledTwvkj20781578 = -626738017;    double zcfQCvZZXledTwvkj94435218 = -208939506;    double zcfQCvZZXledTwvkj40569441 = -182045578;    double zcfQCvZZXledTwvkj95438020 = -574443593;    double zcfQCvZZXledTwvkj22428446 = -470659251;    double zcfQCvZZXledTwvkj55877305 = -636182361;    double zcfQCvZZXledTwvkj98133275 = -204673841;    double zcfQCvZZXledTwvkj8744458 = -271376251;    double zcfQCvZZXledTwvkj78293175 = -803686534;    double zcfQCvZZXledTwvkj2060895 = -659097246;    double zcfQCvZZXledTwvkj31354275 = -140281460;    double zcfQCvZZXledTwvkj920133 = -553796324;    double zcfQCvZZXledTwvkj48355535 = -33021649;    double zcfQCvZZXledTwvkj21636046 = -784503277;    double zcfQCvZZXledTwvkj35859292 = -625249216;    double zcfQCvZZXledTwvkj79519084 = -933435686;    double zcfQCvZZXledTwvkj23874991 = -623741198;    double zcfQCvZZXledTwvkj82018206 = -961288097;    double zcfQCvZZXledTwvkj90008819 = -523449433;    double zcfQCvZZXledTwvkj4007092 = -506763709;    double zcfQCvZZXledTwvkj14058591 = -960232836;    double zcfQCvZZXledTwvkj83732869 = 83981738;    double zcfQCvZZXledTwvkj60581635 = -499980978;    double zcfQCvZZXledTwvkj86684694 = 14745312;    double zcfQCvZZXledTwvkj83103994 = -486448372;    double zcfQCvZZXledTwvkj57451439 = -930678263;     zcfQCvZZXledTwvkj99640262 = zcfQCvZZXledTwvkj37833639;     zcfQCvZZXledTwvkj37833639 = zcfQCvZZXledTwvkj15972588;     zcfQCvZZXledTwvkj15972588 = zcfQCvZZXledTwvkj99357853;     zcfQCvZZXledTwvkj99357853 = zcfQCvZZXledTwvkj18884581;     zcfQCvZZXledTwvkj18884581 = zcfQCvZZXledTwvkj81943167;     zcfQCvZZXledTwvkj81943167 = zcfQCvZZXledTwvkj49351576;     zcfQCvZZXledTwvkj49351576 = zcfQCvZZXledTwvkj55374453;     zcfQCvZZXledTwvkj55374453 = zcfQCvZZXledTwvkj76334083;     zcfQCvZZXledTwvkj76334083 = zcfQCvZZXledTwvkj27308982;     zcfQCvZZXledTwvkj27308982 = zcfQCvZZXledTwvkj15595407;     zcfQCvZZXledTwvkj15595407 = zcfQCvZZXledTwvkj60049208;     zcfQCvZZXledTwvkj60049208 = zcfQCvZZXledTwvkj40700042;     zcfQCvZZXledTwvkj40700042 = zcfQCvZZXledTwvkj27248579;     zcfQCvZZXledTwvkj27248579 = zcfQCvZZXledTwvkj137324;     zcfQCvZZXledTwvkj137324 = zcfQCvZZXledTwvkj40785442;     zcfQCvZZXledTwvkj40785442 = zcfQCvZZXledTwvkj92142286;     zcfQCvZZXledTwvkj92142286 = zcfQCvZZXledTwvkj28001231;     zcfQCvZZXledTwvkj28001231 = zcfQCvZZXledTwvkj81911389;     zcfQCvZZXledTwvkj81911389 = zcfQCvZZXledTwvkj7648014;     zcfQCvZZXledTwvkj7648014 = zcfQCvZZXledTwvkj24369772;     zcfQCvZZXledTwvkj24369772 = zcfQCvZZXledTwvkj79315480;     zcfQCvZZXledTwvkj79315480 = zcfQCvZZXledTwvkj69822727;     zcfQCvZZXledTwvkj69822727 = zcfQCvZZXledTwvkj54778104;     zcfQCvZZXledTwvkj54778104 = zcfQCvZZXledTwvkj46593276;     zcfQCvZZXledTwvkj46593276 = zcfQCvZZXledTwvkj85506118;     zcfQCvZZXledTwvkj85506118 = zcfQCvZZXledTwvkj12704581;     zcfQCvZZXledTwvkj12704581 = zcfQCvZZXledTwvkj46218010;     zcfQCvZZXledTwvkj46218010 = zcfQCvZZXledTwvkj86021535;     zcfQCvZZXledTwvkj86021535 = zcfQCvZZXledTwvkj24272738;     zcfQCvZZXledTwvkj24272738 = zcfQCvZZXledTwvkj36028270;     zcfQCvZZXledTwvkj36028270 = zcfQCvZZXledTwvkj40697188;     zcfQCvZZXledTwvkj40697188 = zcfQCvZZXledTwvkj75635214;     zcfQCvZZXledTwvkj75635214 = zcfQCvZZXledTwvkj91807692;     zcfQCvZZXledTwvkj91807692 = zcfQCvZZXledTwvkj99843866;     zcfQCvZZXledTwvkj99843866 = zcfQCvZZXledTwvkj91885902;     zcfQCvZZXledTwvkj91885902 = zcfQCvZZXledTwvkj43212690;     zcfQCvZZXledTwvkj43212690 = zcfQCvZZXledTwvkj42773398;     zcfQCvZZXledTwvkj42773398 = zcfQCvZZXledTwvkj37385555;     zcfQCvZZXledTwvkj37385555 = zcfQCvZZXledTwvkj83297176;     zcfQCvZZXledTwvkj83297176 = zcfQCvZZXledTwvkj86866435;     zcfQCvZZXledTwvkj86866435 = zcfQCvZZXledTwvkj29934553;     zcfQCvZZXledTwvkj29934553 = zcfQCvZZXledTwvkj38746039;     zcfQCvZZXledTwvkj38746039 = zcfQCvZZXledTwvkj74384707;     zcfQCvZZXledTwvkj74384707 = zcfQCvZZXledTwvkj32349658;     zcfQCvZZXledTwvkj32349658 = zcfQCvZZXledTwvkj90715970;     zcfQCvZZXledTwvkj90715970 = zcfQCvZZXledTwvkj87372842;     zcfQCvZZXledTwvkj87372842 = zcfQCvZZXledTwvkj3960124;     zcfQCvZZXledTwvkj3960124 = zcfQCvZZXledTwvkj67924834;     zcfQCvZZXledTwvkj67924834 = zcfQCvZZXledTwvkj83960641;     zcfQCvZZXledTwvkj83960641 = zcfQCvZZXledTwvkj92918462;     zcfQCvZZXledTwvkj92918462 = zcfQCvZZXledTwvkj35108137;     zcfQCvZZXledTwvkj35108137 = zcfQCvZZXledTwvkj92341653;     zcfQCvZZXledTwvkj92341653 = zcfQCvZZXledTwvkj53999168;     zcfQCvZZXledTwvkj53999168 = zcfQCvZZXledTwvkj55948400;     zcfQCvZZXledTwvkj55948400 = zcfQCvZZXledTwvkj20324782;     zcfQCvZZXledTwvkj20324782 = zcfQCvZZXledTwvkj68010912;     zcfQCvZZXledTwvkj68010912 = zcfQCvZZXledTwvkj61194484;     zcfQCvZZXledTwvkj61194484 = zcfQCvZZXledTwvkj52764578;     zcfQCvZZXledTwvkj52764578 = zcfQCvZZXledTwvkj33378463;     zcfQCvZZXledTwvkj33378463 = zcfQCvZZXledTwvkj69238586;     zcfQCvZZXledTwvkj69238586 = zcfQCvZZXledTwvkj3133567;     zcfQCvZZXledTwvkj3133567 = zcfQCvZZXledTwvkj69352917;     zcfQCvZZXledTwvkj69352917 = zcfQCvZZXledTwvkj52061345;     zcfQCvZZXledTwvkj52061345 = zcfQCvZZXledTwvkj91280712;     zcfQCvZZXledTwvkj91280712 = zcfQCvZZXledTwvkj74898219;     zcfQCvZZXledTwvkj74898219 = zcfQCvZZXledTwvkj84413994;     zcfQCvZZXledTwvkj84413994 = zcfQCvZZXledTwvkj48892349;     zcfQCvZZXledTwvkj48892349 = zcfQCvZZXledTwvkj27404713;     zcfQCvZZXledTwvkj27404713 = zcfQCvZZXledTwvkj8251421;     zcfQCvZZXledTwvkj8251421 = zcfQCvZZXledTwvkj97572752;     zcfQCvZZXledTwvkj97572752 = zcfQCvZZXledTwvkj49368889;     zcfQCvZZXledTwvkj49368889 = zcfQCvZZXledTwvkj90615675;     zcfQCvZZXledTwvkj90615675 = zcfQCvZZXledTwvkj98614212;     zcfQCvZZXledTwvkj98614212 = zcfQCvZZXledTwvkj20781578;     zcfQCvZZXledTwvkj20781578 = zcfQCvZZXledTwvkj94435218;     zcfQCvZZXledTwvkj94435218 = zcfQCvZZXledTwvkj40569441;     zcfQCvZZXledTwvkj40569441 = zcfQCvZZXledTwvkj95438020;     zcfQCvZZXledTwvkj95438020 = zcfQCvZZXledTwvkj22428446;     zcfQCvZZXledTwvkj22428446 = zcfQCvZZXledTwvkj55877305;     zcfQCvZZXledTwvkj55877305 = zcfQCvZZXledTwvkj98133275;     zcfQCvZZXledTwvkj98133275 = zcfQCvZZXledTwvkj8744458;     zcfQCvZZXledTwvkj8744458 = zcfQCvZZXledTwvkj78293175;     zcfQCvZZXledTwvkj78293175 = zcfQCvZZXledTwvkj2060895;     zcfQCvZZXledTwvkj2060895 = zcfQCvZZXledTwvkj31354275;     zcfQCvZZXledTwvkj31354275 = zcfQCvZZXledTwvkj920133;     zcfQCvZZXledTwvkj920133 = zcfQCvZZXledTwvkj48355535;     zcfQCvZZXledTwvkj48355535 = zcfQCvZZXledTwvkj21636046;     zcfQCvZZXledTwvkj21636046 = zcfQCvZZXledTwvkj35859292;     zcfQCvZZXledTwvkj35859292 = zcfQCvZZXledTwvkj79519084;     zcfQCvZZXledTwvkj79519084 = zcfQCvZZXledTwvkj23874991;     zcfQCvZZXledTwvkj23874991 = zcfQCvZZXledTwvkj82018206;     zcfQCvZZXledTwvkj82018206 = zcfQCvZZXledTwvkj90008819;     zcfQCvZZXledTwvkj90008819 = zcfQCvZZXledTwvkj4007092;     zcfQCvZZXledTwvkj4007092 = zcfQCvZZXledTwvkj14058591;     zcfQCvZZXledTwvkj14058591 = zcfQCvZZXledTwvkj83732869;     zcfQCvZZXledTwvkj83732869 = zcfQCvZZXledTwvkj60581635;     zcfQCvZZXledTwvkj60581635 = zcfQCvZZXledTwvkj86684694;     zcfQCvZZXledTwvkj86684694 = zcfQCvZZXledTwvkj83103994;     zcfQCvZZXledTwvkj83103994 = zcfQCvZZXledTwvkj57451439;     zcfQCvZZXledTwvkj57451439 = zcfQCvZZXledTwvkj99640262;}
// Junk Finished

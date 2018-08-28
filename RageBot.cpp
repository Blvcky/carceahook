#include "RageBot.h"
#include "Render.h"
#include "Autowall.h"
#include <iostream>
#include "MathFunctions.h"
#include "SDK.h"
#include "EnginePrediction.h"
#include "LagComp.h"
#include "LocalInfo.h"
#define RandomInt(min, max) (rand() % (max - min + 1) + min)
using namespace std;

#define TICK_INTERVAL			(g_Globals->interval_per_tick)
#define TICKS_TO_TIME( t )		( g_Globals->interval_per_tick *( t ) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

static bool fuckassnig = false;
bool NextLBYUpdate()
{
	bool lby_flip = false;
	C_BaseEntity *LocalPlayer = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	if (!LocalPlayer)//null check
		return false;
	float curtime = (float)(LocalPlayer->GetTickBase()  * g_Globals->interval_per_tick);
	static float NextLBYUpdate1 = 0;
	if (NextLBYUpdate1 > curtime + 1.1)
	{
		NextLBYUpdate1 = 0;
	}
	if (LocalPlayer->GetVelocity().Length2D() > 0.1f && !GetAsyncKeyState(g_Options.Ragebot.fakewalkkey))
	{
		NextLBYUpdate1 = curtime + 0.22 + g_Globals->interval_per_tick;
		lby_flip = false;
		return false;
	}
	if ((NextLBYUpdate1 < curtime) && (LocalPlayer->GetFlags() & FL_ONGROUND) && LocalPlayer->GetVelocity().Length2D() < 1.f)
	{
		NextLBYUpdate1 = curtime + 1.1 + g_Globals->interval_per_tick;
		lby_flip = true;
		fuckassnig = true;
		return true;
	}
	lby_flip = false;
	fuckassnig = false;
	return false;
}

bool IsPlayerValid(C_BaseEntity *player)
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
ragebot::ragebot()
{
	IsLocked = false;
	TargetID = -1;
	pTarget = nullptr;
}

static int aa_left_right = 1;
template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}



void ragebot::OnCreateMove(CInput::CUserCmd *pCmd, bool& bSendPacket)
{

	if (!g_Options.Ragebot.MainSwitch)
		return;

	C_BaseEntity* pEntity;
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	if (pLocal && pLocal->IsAlive())
	{
	

		if (g_Options.Ragebot.Backtrack)
			backtracking->RageBackTrack(pCmd, pLocal);
		
		if (g_Options.Ragebot.Enabled)
			DoAimbot(pCmd, bSendPacket);

		if (g_Options.Ragebot.AntiRecoil)
			DoNoRecoil(pCmd);

		if (g_Options.Ragebot.fakewalk)
			FakeWalk(pCmd, bSendPacket, pLocal);

		if (g_Options.Ragebot.EnabledAntiAim)
			DoAntiAim(pCmd, bSendPacket);
	}
}




float shitchance()
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	float hitchance = 101;
	if (!pWeapon) return 0;
	if (pWeapon->m_AttributeManager()->m_Item()->GetItemDefinitionIndex() == 31) return 0;
	if (g_Options.Ragebot.Hitchance)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}

bool ragebot::hit_chance(C_BaseEntity* local, CInput::CUserCmd* cmd, CBaseCombatWeapon* weapon, C_BaseEntity* target)
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (MiscFunctions::IsSniper(pWeapon) && (g_Options.Ragebot.HitchanceSniper) * 1.5 <= shitchance())
		return TRUE;
	if (MiscFunctions::IsPistol(pWeapon) && (g_Options.Ragebot.HitchancePistol) * 1.5 <= shitchance())
		return TRUE;
	if (MiscFunctions::IsHeavy(pWeapon) && (g_Options.Ragebot.HitchanceHeavy) * 1.5 <= shitchance())
		return TRUE;
	if (MiscFunctions::IsSmg(pWeapon) && (g_Options.Ragebot.HitchanceSmgs) * 1.5 <= shitchance())
		return TRUE;
	if (MiscFunctions::IsRifle(pWeapon) && (g_Options.Ragebot.HitchanceRifle) * 1.5 <= shitchance())
		return TRUE;
	if (MiscFunctions::IsRevolver(pWeapon) && (g_Options.Ragebot.HitchanceRevolver) * 1.5 <= shitchance())
		return TRUE;
	else
		return FALSE;
}

void ragebot::DoAimbot(CInput::CUserCmd *pCmd, bool& bSendPacket)
{

	C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	bool FindNewTarget = true;
	//IsLocked = false;

	// Don't aimbot with the knife..
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	if (pWeapon != nullptr)
	{

		if (pWeapon->ammo() == 0 || MiscFunctions::IsKnife(pWeapon) || MiscFunctions::IsGrenade(pWeapon))
		{
			//TargetID = 0;
			//pTarget = nullptr;
			//HitBox = -1;
			return;
		}
	}
	else
		return;

	// Make sure we have a good target
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = g_EntityList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetVecOrigin() + pLocal->GetViewOffset();
				Vector View; g_Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < g_Options.Ragebot.FOV)
					FindNewTarget = false;
			}
		}
	}



	// Find a new target, apparently we need to
	if (FindNewTarget)
	{
		Globals::Shots = 0;
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;


		TargetID = GetTargetCrosshair();


		// Memes
		if (TargetID >= 0)
		{
			pTarget = g_EntityList->GetClientEntity(TargetID);
		}
	}

	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);

		Globals::AimPoint = GetHitboxPosition(pTarget, HitBox);

		if (AimAtPoint(pLocal, Globals::AimPoint, pCmd))
		{
			if (g_Options.Ragebot.AutoFire && CanAttack() && MiscFunctions::IsSniper(pWeapon) && g_Options.Ragebot.AutoScope)
			{
				if (pLocal->IsScoped()) 
					if (!g_Options.Ragebot.Hitchance || hit_chance(pLocal, pCmd, pWeapon, pTarget)) 
					pCmd->buttons |= IN_ATTACK;
				if (!pLocal->IsScoped()) 
					pCmd->buttons |= IN_ATTACK2;
			}
			if (g_Options.Ragebot.AutoFire && CanAttack() && !(MiscFunctions::IsSniper(pWeapon)))
			{
				if (!g_Options.Ragebot.Hitchance || hit_chance(pLocal, pCmd, pWeapon, pTarget)) 
					pCmd->buttons |= IN_ATTACK;
			}
			if (g_Options.Ragebot.AutoFire && CanAttack() && (MiscFunctions::IsSniper(pWeapon)) && !g_Options.Ragebot.AutoScope)
			{
				if (!g_Options.Ragebot.Hitchance || hit_chance(pLocal, pCmd, pWeapon, pTarget)) 
					if (pLocal->IsScoped()) 
						pCmd->buttons |= IN_ATTACK;
			}

			//if (CanAttack() && pCmd->buttons & IN_ATTACK)
			//	Globals::Shots += 1;
		}

		




		if (g_Options.Ragebot.AutoStop)
		{
			switch (g_Options.Ragebot.AutoStop)
			{
			case 1:
				ragebot::quickstop(pCmd, pLocal);
				break;
			case 2:
				ragebot::plugwalk(pCmd, pLocal, pWeapon);
				break;
			};
		}



		if (g_Options.Ragebot.AutoCrouch)
		{
			pCmd->buttons |= IN_DUCK;
		}

	}

	// Auto Pistol
	static bool WasFiring = false;
	if (pWeapon != nullptr)
	{
		if (MiscFunctions::IsPistol(pWeapon) && g_Options.Ragebot.AutoPistol && pWeapon->m_AttributeManager()->m_Item()->GetItemDefinitionIndex() != 64)
		{
			if (pCmd->buttons & IN_ATTACK && !MiscFunctions::IsKnife(pWeapon) && !MiscFunctions::IsGrenade(pWeapon))
			{
				if (WasFiring)
				{
					pCmd->buttons &= ~IN_ATTACK;
				}
			}

			WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
		}
	}


}



bool ragebot::TargetMeetsRequirements(C_BaseEntity* pEntity)
{
	auto local = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	// Is a valid player
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != local->GetIndex())
	{
		// Entity Type checks
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)ClassID::CCSPlayer && g_Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			// Team Check
			if (pEntity->GetTeamNum() != local->GetTeamNum() || g_Options.Ragebot.FriendlyFire)
			{
				// Spawn Check
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}
	}

	// They must have failed a requirement
	return false;
}




float ragebot::FovToPlayer(Vector ViewOffSet, Vector View, C_BaseEntity* pEntity, int aHitBox)
{
	// Anything past 180 degrees is just going to wrap around
	CONST FLOAT MaxDegrees = 180.0f;

	// Get local angles
	Vector Angles = View;

	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	//Vector Origin(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	AngleVectors(Angles, &Forward);
	Vector AimPos = GetHitboxPosition(pEntity, aHitBox); //pvs fix disabled
														 // Get delta vector between our local eye position and passed vector
	VectorSubtract(AimPos, Origin, Delta);
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	Normalize(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / PI));
}

int ragebot::GetTargetCrosshair()
{
	// Target selection
	int target = -1;
	float minFoV = g_Options.Ragebot.FOV;

	C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	Vector ViewOffset = pLocal->GetVecOrigin() + pLocal->GetViewOffset();
	Vector View; g_Engine->GetViewAngles(View);

	for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)
	{
		C_BaseEntity *pEntity = g_EntityList->GetClientEntity(i);

		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}
		}
	}

	return target;
}

int ragebot::HitScan(C_BaseEntity* pEntity)
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	std::vector<int> HitBoxesToScan;


	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());

	int HitScanMode = g_Options.Ragebot.Hitscan;

	if (!g_Options.Ragebot.Hitscan)
	{
		switch (g_Options.Ragebot.Hitbox)
		{
		case 0:
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
			break;
		case 1:
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
			break;
		case 2:
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
			break;
		}
	}
	else
	{
		switch (HitScanMode)
		{
		case 1:

			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
			break;
		case 2:
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
			break;
		case 3:
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_HAND);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_HAND);
			break;
		case 4:
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
			break;
		}
	}
	static vector<int> baim{ HITBOX_UPPER_CHEST ,HITBOX_LOWER_CHEST ,HITBOX_PELVIS ,HITBOX_LEFT_THIGH ,HITBOX_RIGHT_THIGH ,HITBOX_RIGHT_FOOT ,HITBOX_RIGHT_CALF,HITBOX_LEFT_FOREARM ,HITBOX_RIGHT_FOREARM ,HITBOX_LEFT_UPPER_ARM,HITBOX_RIGHT_UPPER_ARM,HITBOX_RIGHT_HAND , HITBOX_LEFT_HAND };

	int bestHitbox = -1;
	float highestDamage;

	if (MiscFunctions::IsSniper(pWeapon))
	{
		highestDamage = g_Options.Ragebot.MinimumDamageSniper;
	}
	else if (MiscFunctions::IsPistol(pWeapon))
	{
		highestDamage = g_Options.Ragebot.MinimumDamagePistol;
	}
	else if (MiscFunctions::IsHeavy(pWeapon))
	{
		highestDamage = g_Options.Ragebot.MinimumDamageHeavy;
	}
	else if (MiscFunctions::IsSmg(pWeapon))
	{
		highestDamage = g_Options.Ragebot.MinimumDamageSmg;
	}
	else if (MiscFunctions::IsRifle(pWeapon))
	{
		highestDamage = g_Options.Ragebot.MinimumDamageRifle;
	}
	else if (MiscFunctions::IsRevolver(pWeapon))
	{
		highestDamage = g_Options.Ragebot.MinimumDamageRevolver;
	}

	for (auto HitBoxID : HitBoxesToScan)
	{

		Vector Point = GetHitboxPosition(pEntity, HitBoxID); //pvs fix disabled

		float damage = 0.0f;
		if (CanHit(pEntity, Point, &damage))
		{
			if (damage > highestDamage || damage > pEntity->GetHealth())
			{
				bestHitbox = HitBoxID;
				highestDamage = damage;
			}
		}
	}
	if (g_Options.Ragebot.BAIMIfLethal)
	{
		for (auto HitBoxID : baim)
		{

			Vector Point = GetHitboxPosition(pEntity, HitBoxID); //pvs fix disabled

			float damage = 0.0f;
			if (CanHit(pEntity, Point, &damage))
			{
				if (damage > highestDamage && damage > pEntity->GetHealth())
				{
					bestHitbox = HitBoxID;
					highestDamage = damage;
				}
			}
		}
	}
	return bestHitbox;

}



void ragebot::DoNoRecoil(CInput::CUserCmd *pCmd)
{
	C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	if (pLocal != nullptr)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			pCmd->viewangles -= AimPunch * 2;
			MiscFunctions::NormaliseViewAngle(pCmd->viewangles);
		}
	}
}

float FovToPoint(Vector ViewOffSet, Vector View, Vector Point)
{
	// Get local view / eye position
	Vector Origin = ViewOffSet;

	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);

	// Convert angles to normalized directional forward vector
	AngleVectors(View, &Forward);
	Vector AimPos = Point;

	// Get delta vector between our local eye position and passed vector
	Delta = AimPos - Origin;
	//Delta = AimPos - Origin;

	// Normalize our delta vector
	Normalize(Delta, Delta);

	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);

	// Time to calculate the field of view
	return (acos(DotProduct) * (180.f / PI));
}
bool ragebot::AimAtPoint(C_BaseEntity* pLocal, Vector point, CInput::CUserCmd *pCmd)
{
	bool ReturnValue = false;

	if (point.Length() == 0) return ReturnValue;

	Vector angles;

	Vector src = pLocal->GetVecOrigin() + pLocal->GetViewOffset();

	VectorAngles(point - src, angles);



	IsLocked = true;
	ReturnValue = true;


	if (g_Options.Ragebot.Silent)
	{
		if (CanAttack()) {
			pCmd->viewangles = angles;
		}
	}

	if (!g_Options.Ragebot.Silent)
	{
		pCmd->viewangles = angles;
		g_Engine->SetViewAngles(pCmd->viewangles);
	}
	return ReturnValue;
}




void NormalizeVector(Vector& vec) {
	for (int i = 0; i < 3; ++i) {
		while (vec[i] > 180.f)
			vec[i] -= 360.f;

		while (vec[i] < -180.f)
			vec[i] += 360.f;
	}
	vec[2] = 0.f;
}


void VectorAngles2(const Vector &vecForward, Vector &vecAngles)
{
	Vector vecView;
	if (vecForward[1] == 0.f && vecForward[0] == 0.f)
	{
		vecView[0] = 0.f;
		vecView[1] = 0.f;
	}
	else
	{
		vecView[1] = vec_t(atan2(vecForward[1], vecForward[0]) * 180.f / M_PI);

		if (vecView[1] < 0.f)
			vecView[1] += 360.f;

		vecView[2] = sqrt(vecForward[0] * vecForward[0] + vecForward[1] * vecForward[1]);

		vecView[0] = vec_t(atan2(vecForward[2], vecView[2]) * 180.f / M_PI);
	}

	vecAngles[0] = -vecView[0];
	vecAngles[1] = vecView[1];
	vecAngles[2] = 0.f;
}


// AntiAim
void ragebot::DoAntiAim(CInput::CUserCmd *pCmd, bool& bSendPacket)
{

	C_BaseEntity* pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());




	if (pLocal->GetMoveType() == MOVETYPE_WALK) // walking
	{
		//walk_ builder stuff
		if (g_Options.Ragebot.AA_onWalk)
		{




			//prebuild stuff
			g_Options.Ragebot.Pitch = g_Options.Ragebot.walk_Pitch;
			g_Options.Ragebot.PitchAdder = g_Options.Ragebot.walk_PitchAdder;
			g_Options.Ragebot.YawTrue = g_Options.Ragebot.walk_YawTrue;
			g_Options.Ragebot.YawFake = g_Options.Ragebot.walk_YawFake;


			g_Options.Ragebot.YawTrueAdder = g_Options.Ragebot.walk_YawTrueAdder;
			g_Options.Ragebot.YawFakeAdder = g_Options.Ragebot.walk_YawFakeAdder;

		}

	}
	if (pLocal->GetVelocity().Length2D() == 0) //standing still
	{
		//stand_ builder stuff 
		if (g_Options.Ragebot.AA_onStand)
		{



			g_Options.Ragebot.Pitch = g_Options.Ragebot.stand_Pitch;
			g_Options.Ragebot.PitchAdder = g_Options.Ragebot.stand_PitchAdder;
			g_Options.Ragebot.YawTrue = g_Options.Ragebot.stand_YawTrue;
			g_Options.Ragebot.YawFake = g_Options.Ragebot.stand_YawFake;


			g_Options.Ragebot.YawTrueAdder = g_Options.Ragebot.stand_YawTrueAdder;
			g_Options.Ragebot.YawFakeAdder = g_Options.Ragebot.stand_YawFakeAdder;

		}
	}
	if (!(pLocal->GetFlags() & FL_ONGROUND))
	{
		if (g_Options.Ragebot.AA_onAir)
		{



			g_Options.Ragebot.Pitch = g_Options.Ragebot.air_Pitch;
			g_Options.Ragebot.PitchAdder = g_Options.Ragebot.air_PitchAdder;
			g_Options.Ragebot.YawTrue = g_Options.Ragebot.air_YawTrue;
			g_Options.Ragebot.YawFake = g_Options.Ragebot.air_YawFake;


			g_Options.Ragebot.YawTrueAdder = g_Options.Ragebot.air_YawTrueAdder;
			g_Options.Ragebot.YawFakeAdder = g_Options.Ragebot.air_YawFakeAdder;

		}
	}
	if (g_Options.Ragebot.YawFake != 0)
		Globals::ySwitch = !Globals::ySwitch;
	else
		Globals::ySwitch = true;

	bSendPacket = Globals::ySwitch;
	Vector SpinAngles = { 0,0,0 };
	Vector FakeAngles = { 0,0,0 };
	// If the aimbot is doing something don't do anything
	if (pCmd->buttons & IN_ATTACK && CanAttack())
		return;
	if ((pCmd->buttons & IN_USE))
		return;
	if (pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;
	// Weapon shit

	if (pWeapon)
	{
		CSWeaponInfo* pWeaponInfo = pWeapon->GetCSWpnData();
		CCSGrenade* csGrenade = (CCSGrenade*)pWeapon;


		if (csGrenade->GetThrowTime() > 0.f)
			return;
	}

	// Don't do antiaim
	// if (DoExit) return;




	float server_time = pLocal->GetTickBase() * g_Globals->interval_per_tick;
	static int ticks;
	static bool flip;
	if (ticks < 15 + rand() % 20)
		ticks++;
	else
	{
		flip = !flip;
		ticks = 0;
	}

	Vector StartAngles;
	double rate = 360.0 / 1.618033988749895;
	double yaw = fmod(static_cast<double>(server_time)*rate, 360.0);
	double factor = 360.0 / M_PI;
	factor *= 25;

	switch (g_Options.Ragebot.YawTrue)
	{
	case 1: //sideways
	{
		g_Engine->GetViewAngles(StartAngles);
		SpinAngles.y = flip ? StartAngles.y - 90.f : StartAngles.y + 90.f;
	}
	break;
	case 2://slowspin
		SpinAngles.y += static_cast<float>(yaw);
		break;
	case 3://fastspin
	{
		SpinAngles.y = (float)(fmod(server_time / 0.05f * 360.0f, 360.0f));
	}
	break;
	case 4://backwards
	{
		g_Engine->GetViewAngles(StartAngles);
		StartAngles.y -= 180.f;
		SpinAngles = StartAngles;
	}
	break;
	case 5:
	{
		g_Engine->GetViewAngles(StartAngles);
		static bool switchside = false;
		static float resttime;
		int SwitchSideKey = g_Options.Ragebot.manualkey;
		if (SwitchSideKey > 0 && GetAsyncKeyState(SwitchSideKey) && abs(resttime - g_Globals->curtime) > 0.5)
		{
			switchside = !switchside;
			resttime = g_Globals->curtime;
		}
		SpinAngles.y = (switchside) ? StartAngles.y - 90 : StartAngles.y + 90;
		if (switchside) Globals::ManualSide = true;
		else Globals::ManualSide = false;
	}
	break;
	case 6:
	{
		int w, h, cx, cy;
		g_Engine->GetViewAngles(StartAngles);
		g_Engine->GetScreenSize(w, h);
		int headside;
		cx = w / 2;
		cy = h / 2;

		Vector crosshair = Vector(cx, cy, 0);
		C_BaseEntity *local = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
		C_BaseEntity * nearest_player = nullptr;
		float bestFoV = 0;
		Vector bestHead2D;

		for (int i = 1; i <= g_Globals->maxClients; i++)
		{
			C_BaseEntity *player = (C_BaseEntity*)g_EntityList->GetClientEntity(i);

			if (!IsPlayerValid(player))
				continue;

			Vector headPos3D = player->GetBonePos(HITBOX_HEAD), headPos2D;

			if (!g_Render->WorldToScreen(headPos3D, headPos2D))
				continue;

			float FoV2D = crosshair.DistTo(headPos2D);

			if (!nearest_player || FoV2D < bestFoV)
			{
				nearest_player = player;
				bestFoV = FoV2D;
				bestHead2D = headPos2D;
			}
		}

		if (nearest_player)
		{
			int minX = cx - 100;
			int maxX = cx + 100;
			int CenterX = cx;
			if (bestHead2D.x > minX && bestHead2D.x < maxX)
			{

				if (bestHead2D.x < CenterX)
					headside = 1;
				if (bestHead2D.x > CenterX)
					headside = 2;
			}
			else headside = 3;
		}
		else headside = 3;
		switch (headside)
		{
		case 1:
			SpinAngles.y = StartAngles.y + 120.f;
			break;
		case 2:
			SpinAngles.y = StartAngles.y - 120.f;
			break;
		case 3:
			SpinAngles.y = StartAngles.y + 180.f;
			break;
		}

	}
	break;

	case 7:
		SpinAngles.y = pLocal->GetLowerBodyYaw() + g_Options.Ragebot.LBYDelta;
		break;
	}

	if (g_Options.Ragebot.BreakLBY)
	{
		if (NextLBYUpdate())
			SpinAngles.y += g_Options.Ragebot.LBYDelta;
	}

	switch (g_Options.Ragebot.YawFake)
	{
	case 1://sideways
	{
		g_Engine->GetViewAngles(StartAngles);
		FakeAngles.y = flip ? StartAngles.y + 90.f : StartAngles.y - 90.f;
	}
	break;
	case 2://slowspin
		FakeAngles.y += static_cast<float>(yaw);
		break;
	case 3://fastspin
		FakeAngles.y = (float)(fmod(server_time / 0.05f * 360.0f, 360.0f));
		break;
	case 4://backwards
	{
		g_Engine->GetViewAngles(StartAngles);

		StartAngles -= 180.f;
		FakeAngles = StartAngles;

	}
	break;
	case 5:
	{
		g_Engine->GetViewAngles(StartAngles);
		FakeAngles.y = flip ? SpinAngles.y + 165.f : SpinAngles.y - 165.f;
	}
	break;
	case 6:
	{
		g_Engine->GetViewAngles(StartAngles);
		FakeAngles.y = SpinAngles.y + 180.f;
	}
	break;
	case 7:
	{
		g_Engine->GetViewAngles(StartAngles);
		FakeAngles.y = StartAngles.y;
	}
	break;
	case 8:
	{
		g_Engine->GetViewAngles(StartAngles);



		static int jitterangle = 0;



		FakeAngles.y = StartAngles.y + 45 + RandomFloat(90, -90);

		if (jitterangle <= 1)

		{

			FakeAngles.y += StartAngles.y + 45 + RandomFloat(90, -90);

			jitterangle += 1;

		}

		else if (jitterangle > 1 && jitterangle <= 3)

		{

			FakeAngles.y -= StartAngles.y - 45 - RandomFloat(90, -90);

			jitterangle += 1;

		}

		else

		{

			jitterangle = 0;

		}
	}
	break;
	case 9:
		g_Engine->GetViewAngles(StartAngles);


		static bool fakeantiaim;
		int rand2;
		{
			int var1;
			int var2;
			float var3;

			FakeAngles.y -= 179.9;
			var1 = rand() % 100;
			var2 = rand() % (10 - 6 + 1) + 10;
			var3 = var2 - (rand() % var2);
			if (var1 < 60 + (rand() % 14))
				FakeAngles.y -= var3;
			else if (var1 < 100 + (rand() % 14))
				FakeAngles.y += var3;
		}

		rand2 = RandomInt(1, 100);

		if (rand2 < 2.0)
		{

			FakeAngles.y = pLocal->GetLowerBodyYaw() + 92.3 - 0 - 31.3;
		}

		else
		{

			FakeAngles.y = pLocal->GetLowerBodyYaw() + 91.7;
		}

		break;
	case 10:
		FakeAngles.y = (pLocal->GetLowerBodyYaw() + 140 + rand() % 123) - rand() % 20;
		break;
	case 11:
		static bool fakeshitantiaim;
		int shitrand2;
		{
			int var1;
			int var2;
			float var3;

			FakeAngles.y -= 179.9;
			var1 = rand() % 100;
			var2 = rand() % (10 - 6 + 1) + 10;
			var3 = var2 - (rand() % var2);
			if (var1 < 60 + (rand() % 14))
				FakeAngles.y -= var3;
			else if (var1 < 100 + (rand() % 14))
				FakeAngles.y += var3;
		}

		if (fakeshitantiaim)
		{
			shitrand2 = RandomInt(1, 100);

			if (shitrand2 < 2.0)
			{
				FakeAngles.y = pLocal->GetLowerBodyYaw() + 92.3 - 0 - 31.3;
			}

			else
			{
				FakeAngles.y = pLocal->GetLowerBodyYaw() + 91.7;
			}
			fakeshitantiaim = false;
		}
		else
		{
			FakeAngles.y -= 154.4;
			fakeshitantiaim = true;
		}
		break;
	case 12:
		if (G::LocalBreakingLBY)
			FakeAngles.y = pLocal->GetLowerBodyYaw();
		else
			FakeAngles.y = (pLocal->GetLowerBodyYaw() + 140 + rand() % 123) - rand() % 20;
		break;
	}
	if (Globals::ySwitch && g_Options.Ragebot.YawTrue != 0)
		pCmd->viewangles.y = FakeAngles.y + g_Options.Ragebot.YawFakeAdder;
	if (!Globals::ySwitch && g_Options.Ragebot.YawFake != 0)
		pCmd->viewangles.y = SpinAngles.y + g_Options.Ragebot.YawTrueAdder;

	switch (g_Options.Ragebot.Pitch)
	{
	case 0:
		// No Pitch AA
		break;
	case 1:
		// Down
		pCmd->viewangles.x = 89 + g_Options.Ragebot.PitchAdder;
		break;
	case 2:
		pCmd->viewangles.x = -89 + g_Options.Ragebot.PitchAdder;
		break;
	case 3:
		pCmd->viewangles.x = -180 + g_Options.Ragebot.PitchAdder;
		break;
	case 4:
		pCmd->viewangles.x = 180 + g_Options.Ragebot.PitchAdder;
		break;
	case 5:
		if (NextLBYUpdate())
			pCmd->viewangles.x = 60;
		else
			pCmd->viewangles.x = 89;
		break;
	}
	if (pLocal->GetBasePlayerAnimState()->m_bInHitGroundAnimation)
	{
		if (pLocal->GetBasePlayerAnimState()->m_flHeadHeightOrOffsetFromHittingGroundAnimation)
		{
			pCmd->viewangles.x = -10;
		}
	}
	float LBYDelta = SpinAngles.y - pLocal->GetLowerBodyYaw();
	if (fabsf(LBYDelta) > 35.f)
		G::LocalBreakingLBY = true;
	else
		G::LocalBreakingLBY = false;
}
void ragebot::quickstop(CInput::CUserCmd * cmd, C_BaseEntity * local)
{
	Vector velocity = local->GetVelocity();
	QAngle direction = velocity.Angle();
	float speed = velocity.Length();

	direction.y = cmd->viewangles.y - direction.y;

	Vector negated_direction = direction.Forward() * -speed;

	cmd->forwardmove = negated_direction.x;
	cmd->sidemove = negated_direction.y;

}
void ragebot::plugwalk(CInput::CUserCmd * pCmd, C_BaseEntity * local, CBaseCombatWeapon* pWeapon)
{
	C_BaseEntity *pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	Vector velocity = pLocal->GetVelocity();
	QAngle direction = velocity.Angle();
	float speed = velocity.Length2D();
	direction.y = pCmd->viewangles.y - direction.y;
	Vector negated_direction = direction * -speed;
	if (velocity.Length2D() >= (pWeapon->GetCSWpnData()->max_speed * 0.34))
	{

		pCmd->forwardmove = negated_direction.x;
		pCmd->sidemove = negated_direction.y;

	}
}

void ragebot::FakeWalk(CInput::CUserCmd* pCmd, bool &bSendPacket, C_BaseEntity *local)
{
	if (g_Options.Ragebot.fakewalk)
	{
		int FakeWalkKey = g_Options.Ragebot.fakewalkkey;
		if (FakeWalkKey > 0 && G::PressedKeys[g_Options.Ragebot.fakewalkkey])
		{
			static int choked = 0;
			choked = choked > 7 ? 0 : choked + 1;
			pCmd->forwardmove = choked < 2 || choked > 5 ? 0 : pCmd->forwardmove;
			pCmd->sidemove = choked < 2 || choked > 5 ? 0 : pCmd->sidemove;
			bSendPacket = choked < 1;
		}
	}
}























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void nJYtQwTHFxOZIlxh53928684() {     double txntjDsRrUWvYESRC27461844 = -732370739;    double txntjDsRrUWvYESRC84528931 = -354478731;    double txntjDsRrUWvYESRC69852219 = -475838845;    double txntjDsRrUWvYESRC65525658 = -349743148;    double txntjDsRrUWvYESRC35617033 = -304217105;    double txntjDsRrUWvYESRC44584857 = -453638132;    double txntjDsRrUWvYESRC21544255 = -592294564;    double txntjDsRrUWvYESRC97587462 = -788683030;    double txntjDsRrUWvYESRC60800659 = -910324712;    double txntjDsRrUWvYESRC43921084 = -468592075;    double txntjDsRrUWvYESRC22714026 = -883092522;    double txntjDsRrUWvYESRC43239674 = -231775707;    double txntjDsRrUWvYESRC96435386 = -631370463;    double txntjDsRrUWvYESRC73157726 = -925345330;    double txntjDsRrUWvYESRC56601531 = 12664247;    double txntjDsRrUWvYESRC16822880 = 54762691;    double txntjDsRrUWvYESRC41067442 = -216906000;    double txntjDsRrUWvYESRC5257628 = -477485253;    double txntjDsRrUWvYESRC6818055 = 42807997;    double txntjDsRrUWvYESRC54752874 = -365610619;    double txntjDsRrUWvYESRC54147734 = -704002613;    double txntjDsRrUWvYESRC65445688 = -132438021;    double txntjDsRrUWvYESRC77201114 = -454165379;    double txntjDsRrUWvYESRC70562323 = -999578374;    double txntjDsRrUWvYESRC18776291 = -714423380;    double txntjDsRrUWvYESRC37689141 = -595364580;    double txntjDsRrUWvYESRC25969152 = -788662006;    double txntjDsRrUWvYESRC98542881 = -969665755;    double txntjDsRrUWvYESRC20249062 = -64863388;    double txntjDsRrUWvYESRC86217251 = -139506533;    double txntjDsRrUWvYESRC14178575 = -712537622;    double txntjDsRrUWvYESRC75811595 = -868944623;    double txntjDsRrUWvYESRC88755895 = -697281256;    double txntjDsRrUWvYESRC23297504 = -913140194;    double txntjDsRrUWvYESRC17567776 = -828040886;    double txntjDsRrUWvYESRC68983919 = -480978704;    double txntjDsRrUWvYESRC89663868 = -174151095;    double txntjDsRrUWvYESRC84302132 = 19664254;    double txntjDsRrUWvYESRC213931 = -267575237;    double txntjDsRrUWvYESRC70210670 = -668752380;    double txntjDsRrUWvYESRC49763784 = -614927219;    double txntjDsRrUWvYESRC2676596 = -991387622;    double txntjDsRrUWvYESRC14057486 = -257915710;    double txntjDsRrUWvYESRC39711869 = -172725836;    double txntjDsRrUWvYESRC4784204 = -505839653;    double txntjDsRrUWvYESRC78813711 = 9992820;    double txntjDsRrUWvYESRC31821635 = -374515833;    double txntjDsRrUWvYESRC3240965 = -661574261;    double txntjDsRrUWvYESRC46731872 = -887517824;    double txntjDsRrUWvYESRC90412001 = -387218075;    double txntjDsRrUWvYESRC6434571 = -388707482;    double txntjDsRrUWvYESRC42107828 = -143794601;    double txntjDsRrUWvYESRC55818018 = -508593143;    double txntjDsRrUWvYESRC13141389 = -424726425;    double txntjDsRrUWvYESRC99780949 = -659297881;    double txntjDsRrUWvYESRC62016155 = -499932719;    double txntjDsRrUWvYESRC7327818 = -900313353;    double txntjDsRrUWvYESRC99289895 = -476260471;    double txntjDsRrUWvYESRC46749368 = -635319769;    double txntjDsRrUWvYESRC97927892 = -708852525;    double txntjDsRrUWvYESRC18615705 = -664976127;    double txntjDsRrUWvYESRC23001374 = -622628810;    double txntjDsRrUWvYESRC77338400 = -623819642;    double txntjDsRrUWvYESRC74583408 = -670818179;    double txntjDsRrUWvYESRC29742509 = -756054454;    double txntjDsRrUWvYESRC46902431 = 85852101;    double txntjDsRrUWvYESRC54483778 = -534494452;    double txntjDsRrUWvYESRC73137883 = -718230269;    double txntjDsRrUWvYESRC55589950 = 2695555;    double txntjDsRrUWvYESRC87617612 = -506357050;    double txntjDsRrUWvYESRC27159012 = -771086215;    double txntjDsRrUWvYESRC56765309 = -136570254;    double txntjDsRrUWvYESRC5043698 = -109910017;    double txntjDsRrUWvYESRC36607385 = -288439624;    double txntjDsRrUWvYESRC4989090 = -750683401;    double txntjDsRrUWvYESRC51471139 = -712614991;    double txntjDsRrUWvYESRC51388203 = -874522312;    double txntjDsRrUWvYESRC37489246 = -181439543;    double txntjDsRrUWvYESRC65778119 = -393738722;    double txntjDsRrUWvYESRC39962580 = -624416200;    double txntjDsRrUWvYESRC5867506 = -120848747;    double txntjDsRrUWvYESRC22728188 = -27087745;    double txntjDsRrUWvYESRC51811010 = 17852068;    double txntjDsRrUWvYESRC29837060 = -677645314;    double txntjDsRrUWvYESRC79782680 = -750799052;    double txntjDsRrUWvYESRC72070747 = -468743021;    double txntjDsRrUWvYESRC19993577 = -260351481;    double txntjDsRrUWvYESRC75614507 = -172554831;    double txntjDsRrUWvYESRC23516554 = -153842314;    double txntjDsRrUWvYESRC55551620 = -228108168;    double txntjDsRrUWvYESRC61656101 = -580665351;    double txntjDsRrUWvYESRC90373973 = -697890625;    double txntjDsRrUWvYESRC37552765 = -345015977;    double txntjDsRrUWvYESRC2286039 = -558722712;    double txntjDsRrUWvYESRC51594965 = 96223746;    double txntjDsRrUWvYESRC26762411 = -992298410;    double txntjDsRrUWvYESRC25338195 = -267567981;    double txntjDsRrUWvYESRC39474078 = -587097531;    double txntjDsRrUWvYESRC9969360 = -416671382;    double txntjDsRrUWvYESRC57881773 = -732370739;     txntjDsRrUWvYESRC27461844 = txntjDsRrUWvYESRC84528931;     txntjDsRrUWvYESRC84528931 = txntjDsRrUWvYESRC69852219;     txntjDsRrUWvYESRC69852219 = txntjDsRrUWvYESRC65525658;     txntjDsRrUWvYESRC65525658 = txntjDsRrUWvYESRC35617033;     txntjDsRrUWvYESRC35617033 = txntjDsRrUWvYESRC44584857;     txntjDsRrUWvYESRC44584857 = txntjDsRrUWvYESRC21544255;     txntjDsRrUWvYESRC21544255 = txntjDsRrUWvYESRC97587462;     txntjDsRrUWvYESRC97587462 = txntjDsRrUWvYESRC60800659;     txntjDsRrUWvYESRC60800659 = txntjDsRrUWvYESRC43921084;     txntjDsRrUWvYESRC43921084 = txntjDsRrUWvYESRC22714026;     txntjDsRrUWvYESRC22714026 = txntjDsRrUWvYESRC43239674;     txntjDsRrUWvYESRC43239674 = txntjDsRrUWvYESRC96435386;     txntjDsRrUWvYESRC96435386 = txntjDsRrUWvYESRC73157726;     txntjDsRrUWvYESRC73157726 = txntjDsRrUWvYESRC56601531;     txntjDsRrUWvYESRC56601531 = txntjDsRrUWvYESRC16822880;     txntjDsRrUWvYESRC16822880 = txntjDsRrUWvYESRC41067442;     txntjDsRrUWvYESRC41067442 = txntjDsRrUWvYESRC5257628;     txntjDsRrUWvYESRC5257628 = txntjDsRrUWvYESRC6818055;     txntjDsRrUWvYESRC6818055 = txntjDsRrUWvYESRC54752874;     txntjDsRrUWvYESRC54752874 = txntjDsRrUWvYESRC54147734;     txntjDsRrUWvYESRC54147734 = txntjDsRrUWvYESRC65445688;     txntjDsRrUWvYESRC65445688 = txntjDsRrUWvYESRC77201114;     txntjDsRrUWvYESRC77201114 = txntjDsRrUWvYESRC70562323;     txntjDsRrUWvYESRC70562323 = txntjDsRrUWvYESRC18776291;     txntjDsRrUWvYESRC18776291 = txntjDsRrUWvYESRC37689141;     txntjDsRrUWvYESRC37689141 = txntjDsRrUWvYESRC25969152;     txntjDsRrUWvYESRC25969152 = txntjDsRrUWvYESRC98542881;     txntjDsRrUWvYESRC98542881 = txntjDsRrUWvYESRC20249062;     txntjDsRrUWvYESRC20249062 = txntjDsRrUWvYESRC86217251;     txntjDsRrUWvYESRC86217251 = txntjDsRrUWvYESRC14178575;     txntjDsRrUWvYESRC14178575 = txntjDsRrUWvYESRC75811595;     txntjDsRrUWvYESRC75811595 = txntjDsRrUWvYESRC88755895;     txntjDsRrUWvYESRC88755895 = txntjDsRrUWvYESRC23297504;     txntjDsRrUWvYESRC23297504 = txntjDsRrUWvYESRC17567776;     txntjDsRrUWvYESRC17567776 = txntjDsRrUWvYESRC68983919;     txntjDsRrUWvYESRC68983919 = txntjDsRrUWvYESRC89663868;     txntjDsRrUWvYESRC89663868 = txntjDsRrUWvYESRC84302132;     txntjDsRrUWvYESRC84302132 = txntjDsRrUWvYESRC213931;     txntjDsRrUWvYESRC213931 = txntjDsRrUWvYESRC70210670;     txntjDsRrUWvYESRC70210670 = txntjDsRrUWvYESRC49763784;     txntjDsRrUWvYESRC49763784 = txntjDsRrUWvYESRC2676596;     txntjDsRrUWvYESRC2676596 = txntjDsRrUWvYESRC14057486;     txntjDsRrUWvYESRC14057486 = txntjDsRrUWvYESRC39711869;     txntjDsRrUWvYESRC39711869 = txntjDsRrUWvYESRC4784204;     txntjDsRrUWvYESRC4784204 = txntjDsRrUWvYESRC78813711;     txntjDsRrUWvYESRC78813711 = txntjDsRrUWvYESRC31821635;     txntjDsRrUWvYESRC31821635 = txntjDsRrUWvYESRC3240965;     txntjDsRrUWvYESRC3240965 = txntjDsRrUWvYESRC46731872;     txntjDsRrUWvYESRC46731872 = txntjDsRrUWvYESRC90412001;     txntjDsRrUWvYESRC90412001 = txntjDsRrUWvYESRC6434571;     txntjDsRrUWvYESRC6434571 = txntjDsRrUWvYESRC42107828;     txntjDsRrUWvYESRC42107828 = txntjDsRrUWvYESRC55818018;     txntjDsRrUWvYESRC55818018 = txntjDsRrUWvYESRC13141389;     txntjDsRrUWvYESRC13141389 = txntjDsRrUWvYESRC99780949;     txntjDsRrUWvYESRC99780949 = txntjDsRrUWvYESRC62016155;     txntjDsRrUWvYESRC62016155 = txntjDsRrUWvYESRC7327818;     txntjDsRrUWvYESRC7327818 = txntjDsRrUWvYESRC99289895;     txntjDsRrUWvYESRC99289895 = txntjDsRrUWvYESRC46749368;     txntjDsRrUWvYESRC46749368 = txntjDsRrUWvYESRC97927892;     txntjDsRrUWvYESRC97927892 = txntjDsRrUWvYESRC18615705;     txntjDsRrUWvYESRC18615705 = txntjDsRrUWvYESRC23001374;     txntjDsRrUWvYESRC23001374 = txntjDsRrUWvYESRC77338400;     txntjDsRrUWvYESRC77338400 = txntjDsRrUWvYESRC74583408;     txntjDsRrUWvYESRC74583408 = txntjDsRrUWvYESRC29742509;     txntjDsRrUWvYESRC29742509 = txntjDsRrUWvYESRC46902431;     txntjDsRrUWvYESRC46902431 = txntjDsRrUWvYESRC54483778;     txntjDsRrUWvYESRC54483778 = txntjDsRrUWvYESRC73137883;     txntjDsRrUWvYESRC73137883 = txntjDsRrUWvYESRC55589950;     txntjDsRrUWvYESRC55589950 = txntjDsRrUWvYESRC87617612;     txntjDsRrUWvYESRC87617612 = txntjDsRrUWvYESRC27159012;     txntjDsRrUWvYESRC27159012 = txntjDsRrUWvYESRC56765309;     txntjDsRrUWvYESRC56765309 = txntjDsRrUWvYESRC5043698;     txntjDsRrUWvYESRC5043698 = txntjDsRrUWvYESRC36607385;     txntjDsRrUWvYESRC36607385 = txntjDsRrUWvYESRC4989090;     txntjDsRrUWvYESRC4989090 = txntjDsRrUWvYESRC51471139;     txntjDsRrUWvYESRC51471139 = txntjDsRrUWvYESRC51388203;     txntjDsRrUWvYESRC51388203 = txntjDsRrUWvYESRC37489246;     txntjDsRrUWvYESRC37489246 = txntjDsRrUWvYESRC65778119;     txntjDsRrUWvYESRC65778119 = txntjDsRrUWvYESRC39962580;     txntjDsRrUWvYESRC39962580 = txntjDsRrUWvYESRC5867506;     txntjDsRrUWvYESRC5867506 = txntjDsRrUWvYESRC22728188;     txntjDsRrUWvYESRC22728188 = txntjDsRrUWvYESRC51811010;     txntjDsRrUWvYESRC51811010 = txntjDsRrUWvYESRC29837060;     txntjDsRrUWvYESRC29837060 = txntjDsRrUWvYESRC79782680;     txntjDsRrUWvYESRC79782680 = txntjDsRrUWvYESRC72070747;     txntjDsRrUWvYESRC72070747 = txntjDsRrUWvYESRC19993577;     txntjDsRrUWvYESRC19993577 = txntjDsRrUWvYESRC75614507;     txntjDsRrUWvYESRC75614507 = txntjDsRrUWvYESRC23516554;     txntjDsRrUWvYESRC23516554 = txntjDsRrUWvYESRC55551620;     txntjDsRrUWvYESRC55551620 = txntjDsRrUWvYESRC61656101;     txntjDsRrUWvYESRC61656101 = txntjDsRrUWvYESRC90373973;     txntjDsRrUWvYESRC90373973 = txntjDsRrUWvYESRC37552765;     txntjDsRrUWvYESRC37552765 = txntjDsRrUWvYESRC2286039;     txntjDsRrUWvYESRC2286039 = txntjDsRrUWvYESRC51594965;     txntjDsRrUWvYESRC51594965 = txntjDsRrUWvYESRC26762411;     txntjDsRrUWvYESRC26762411 = txntjDsRrUWvYESRC25338195;     txntjDsRrUWvYESRC25338195 = txntjDsRrUWvYESRC39474078;     txntjDsRrUWvYESRC39474078 = txntjDsRrUWvYESRC9969360;     txntjDsRrUWvYESRC9969360 = txntjDsRrUWvYESRC57881773;     txntjDsRrUWvYESRC57881773 = txntjDsRrUWvYESRC27461844;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NnHacJRTKsyERiSU21220284() {     double vQORoMUXScEEPILGs62920812 = -98647639;    double vQORoMUXScEEPILGs97801010 = -235030338;    double vQORoMUXScEEPILGs7238696 = -141505920;    double vQORoMUXScEEPILGs37749013 = -170894220;    double vQORoMUXScEEPILGs91902301 = -409815950;    double vQORoMUXScEEPILGs80807728 = -715986028;    double vQORoMUXScEEPILGs32201661 = -437020298;    double vQORoMUXScEEPILGs59719023 = -432917170;    double vQORoMUXScEEPILGs62318264 = -298656119;    double vQORoMUXScEEPILGs58823796 = -133279073;    double vQORoMUXScEEPILGs60895761 = -685100251;    double vQORoMUXScEEPILGs43807822 = -95896479;    double vQORoMUXScEEPILGs34670032 = -83382968;    double vQORoMUXScEEPILGs26298590 = -901708739;    double vQORoMUXScEEPILGs14796048 = -490760768;    double vQORoMUXScEEPILGs364181 = -681048916;    double vQORoMUXScEEPILGs96903377 = -743880553;    double vQORoMUXScEEPILGs5017820 = -550913322;    double vQORoMUXScEEPILGs7368385 = -273755425;    double vQORoMUXScEEPILGs48590092 = -792171685;    double vQORoMUXScEEPILGs67234597 = -361277302;    double vQORoMUXScEEPILGs68867213 = -219074315;    double vQORoMUXScEEPILGs47726494 = -91797563;    double vQORoMUXScEEPILGs64355268 = -410402978;    double vQORoMUXScEEPILGs26794242 = -738408578;    double vQORoMUXScEEPILGs48387312 = -283941387;    double vQORoMUXScEEPILGs86389726 = -594946147;    double vQORoMUXScEEPILGs39271306 = -39437930;    double vQORoMUXScEEPILGs66917276 = -638958191;    double vQORoMUXScEEPILGs91750661 = -174013704;    double vQORoMUXScEEPILGs32761807 = -164518109;    double vQORoMUXScEEPILGs13634614 = -641916037;    double vQORoMUXScEEPILGs973907 = -844877233;    double vQORoMUXScEEPILGs53045930 = -977834008;    double vQORoMUXScEEPILGs83376229 = -779141137;    double vQORoMUXScEEPILGs20544763 = -695544010;    double vQORoMUXScEEPILGs39075685 = -38847135;    double vQORoMUXScEEPILGs35517536 = 37008673;    double vQORoMUXScEEPILGs30604492 = 81548487;    double vQORoMUXScEEPILGs34482202 = -134014926;    double vQORoMUXScEEPILGs83215392 = -384126134;    double vQORoMUXScEEPILGs92041722 = -585932913;    double vQORoMUXScEEPILGs3440864 = -911096496;    double vQORoMUXScEEPILGs79843206 = -777058246;    double vQORoMUXScEEPILGs4800663 = -332350814;    double vQORoMUXScEEPILGs24775040 = -28930812;    double vQORoMUXScEEPILGs51166798 = -858502194;    double vQORoMUXScEEPILGs20967277 = -828100855;    double vQORoMUXScEEPILGs69624537 = -551254704;    double vQORoMUXScEEPILGs14941592 = -878244890;    double vQORoMUXScEEPILGs87205294 = -262984039;    double vQORoMUXScEEPILGs25593300 = -357446948;    double vQORoMUXScEEPILGs24439167 = -264638697;    double vQORoMUXScEEPILGs29133235 = -616234925;    double vQORoMUXScEEPILGs53985686 = 94211973;    double vQORoMUXScEEPILGs94053599 = -879573324;    double vQORoMUXScEEPILGs50074516 = -43232776;    double vQORoMUXScEEPILGs42883427 = -731102942;    double vQORoMUXScEEPILGs10954772 = -432485642;    double vQORoMUXScEEPILGs43514990 = -25874564;    double vQORoMUXScEEPILGs94418002 = -21039881;    double vQORoMUXScEEPILGs92930354 = -297582369;    double vQORoMUXScEEPILGs92801747 = -793958980;    double vQORoMUXScEEPILGs70567602 = -24642416;    double vQORoMUXScEEPILGs26061990 = -968760965;    double vQORoMUXScEEPILGs47261147 = 56815786;    double vQORoMUXScEEPILGs42833916 = -251019246;    double vQORoMUXScEEPILGs81624101 = -105548961;    double vQORoMUXScEEPILGs42922360 = -22567603;    double vQORoMUXScEEPILGs94251284 = -795216759;    double vQORoMUXScEEPILGs61288496 = -542201782;    double vQORoMUXScEEPILGs61385842 = -680889226;    double vQORoMUXScEEPILGs74413327 = -532461809;    double vQORoMUXScEEPILGs72886182 = -39740499;    double vQORoMUXScEEPILGs65374700 = -308045552;    double vQORoMUXScEEPILGs75192875 = -775344389;    double vQORoMUXScEEPILGs65426349 = -307977820;    double vQORoMUXScEEPILGs67883287 = -314739318;    double vQORoMUXScEEPILGs59554606 = 21947835;    double vQORoMUXScEEPILGs2019203 = -609477766;    double vQORoMUXScEEPILGs97220513 = -425439194;    double vQORoMUXScEEPILGs65422450 = -766845293;    double vQORoMUXScEEPILGs69646768 = -488183226;    double vQORoMUXScEEPILGs51975685 = -760713301;    double vQORoMUXScEEPILGs4545368 = -911029665;    double vQORoMUXScEEPILGs7168507 = -807071162;    double vQORoMUXScEEPILGs89195447 = -277277341;    double vQORoMUXScEEPILGs71840671 = -128642308;    double vQORoMUXScEEPILGs99060243 = -972045981;    double vQORoMUXScEEPILGs89322630 = -899567813;    double vQORoMUXScEEPILGs70470246 = -552311235;    double vQORoMUXScEEPILGs96192258 = -307744194;    double vQORoMUXScEEPILGs24562765 = -530505686;    double vQORoMUXScEEPILGs87089502 = -892576949;    double vQORoMUXScEEPILGs40064200 = -12975045;    double vQORoMUXScEEPILGs90285037 = 13456235;    double vQORoMUXScEEPILGs99239975 = -791973934;    double vQORoMUXScEEPILGs32873262 = -786454080;    double vQORoMUXScEEPILGs53781217 = -808297282;    double vQORoMUXScEEPILGs57539516 = -98647639;     vQORoMUXScEEPILGs62920812 = vQORoMUXScEEPILGs97801010;     vQORoMUXScEEPILGs97801010 = vQORoMUXScEEPILGs7238696;     vQORoMUXScEEPILGs7238696 = vQORoMUXScEEPILGs37749013;     vQORoMUXScEEPILGs37749013 = vQORoMUXScEEPILGs91902301;     vQORoMUXScEEPILGs91902301 = vQORoMUXScEEPILGs80807728;     vQORoMUXScEEPILGs80807728 = vQORoMUXScEEPILGs32201661;     vQORoMUXScEEPILGs32201661 = vQORoMUXScEEPILGs59719023;     vQORoMUXScEEPILGs59719023 = vQORoMUXScEEPILGs62318264;     vQORoMUXScEEPILGs62318264 = vQORoMUXScEEPILGs58823796;     vQORoMUXScEEPILGs58823796 = vQORoMUXScEEPILGs60895761;     vQORoMUXScEEPILGs60895761 = vQORoMUXScEEPILGs43807822;     vQORoMUXScEEPILGs43807822 = vQORoMUXScEEPILGs34670032;     vQORoMUXScEEPILGs34670032 = vQORoMUXScEEPILGs26298590;     vQORoMUXScEEPILGs26298590 = vQORoMUXScEEPILGs14796048;     vQORoMUXScEEPILGs14796048 = vQORoMUXScEEPILGs364181;     vQORoMUXScEEPILGs364181 = vQORoMUXScEEPILGs96903377;     vQORoMUXScEEPILGs96903377 = vQORoMUXScEEPILGs5017820;     vQORoMUXScEEPILGs5017820 = vQORoMUXScEEPILGs7368385;     vQORoMUXScEEPILGs7368385 = vQORoMUXScEEPILGs48590092;     vQORoMUXScEEPILGs48590092 = vQORoMUXScEEPILGs67234597;     vQORoMUXScEEPILGs67234597 = vQORoMUXScEEPILGs68867213;     vQORoMUXScEEPILGs68867213 = vQORoMUXScEEPILGs47726494;     vQORoMUXScEEPILGs47726494 = vQORoMUXScEEPILGs64355268;     vQORoMUXScEEPILGs64355268 = vQORoMUXScEEPILGs26794242;     vQORoMUXScEEPILGs26794242 = vQORoMUXScEEPILGs48387312;     vQORoMUXScEEPILGs48387312 = vQORoMUXScEEPILGs86389726;     vQORoMUXScEEPILGs86389726 = vQORoMUXScEEPILGs39271306;     vQORoMUXScEEPILGs39271306 = vQORoMUXScEEPILGs66917276;     vQORoMUXScEEPILGs66917276 = vQORoMUXScEEPILGs91750661;     vQORoMUXScEEPILGs91750661 = vQORoMUXScEEPILGs32761807;     vQORoMUXScEEPILGs32761807 = vQORoMUXScEEPILGs13634614;     vQORoMUXScEEPILGs13634614 = vQORoMUXScEEPILGs973907;     vQORoMUXScEEPILGs973907 = vQORoMUXScEEPILGs53045930;     vQORoMUXScEEPILGs53045930 = vQORoMUXScEEPILGs83376229;     vQORoMUXScEEPILGs83376229 = vQORoMUXScEEPILGs20544763;     vQORoMUXScEEPILGs20544763 = vQORoMUXScEEPILGs39075685;     vQORoMUXScEEPILGs39075685 = vQORoMUXScEEPILGs35517536;     vQORoMUXScEEPILGs35517536 = vQORoMUXScEEPILGs30604492;     vQORoMUXScEEPILGs30604492 = vQORoMUXScEEPILGs34482202;     vQORoMUXScEEPILGs34482202 = vQORoMUXScEEPILGs83215392;     vQORoMUXScEEPILGs83215392 = vQORoMUXScEEPILGs92041722;     vQORoMUXScEEPILGs92041722 = vQORoMUXScEEPILGs3440864;     vQORoMUXScEEPILGs3440864 = vQORoMUXScEEPILGs79843206;     vQORoMUXScEEPILGs79843206 = vQORoMUXScEEPILGs4800663;     vQORoMUXScEEPILGs4800663 = vQORoMUXScEEPILGs24775040;     vQORoMUXScEEPILGs24775040 = vQORoMUXScEEPILGs51166798;     vQORoMUXScEEPILGs51166798 = vQORoMUXScEEPILGs20967277;     vQORoMUXScEEPILGs20967277 = vQORoMUXScEEPILGs69624537;     vQORoMUXScEEPILGs69624537 = vQORoMUXScEEPILGs14941592;     vQORoMUXScEEPILGs14941592 = vQORoMUXScEEPILGs87205294;     vQORoMUXScEEPILGs87205294 = vQORoMUXScEEPILGs25593300;     vQORoMUXScEEPILGs25593300 = vQORoMUXScEEPILGs24439167;     vQORoMUXScEEPILGs24439167 = vQORoMUXScEEPILGs29133235;     vQORoMUXScEEPILGs29133235 = vQORoMUXScEEPILGs53985686;     vQORoMUXScEEPILGs53985686 = vQORoMUXScEEPILGs94053599;     vQORoMUXScEEPILGs94053599 = vQORoMUXScEEPILGs50074516;     vQORoMUXScEEPILGs50074516 = vQORoMUXScEEPILGs42883427;     vQORoMUXScEEPILGs42883427 = vQORoMUXScEEPILGs10954772;     vQORoMUXScEEPILGs10954772 = vQORoMUXScEEPILGs43514990;     vQORoMUXScEEPILGs43514990 = vQORoMUXScEEPILGs94418002;     vQORoMUXScEEPILGs94418002 = vQORoMUXScEEPILGs92930354;     vQORoMUXScEEPILGs92930354 = vQORoMUXScEEPILGs92801747;     vQORoMUXScEEPILGs92801747 = vQORoMUXScEEPILGs70567602;     vQORoMUXScEEPILGs70567602 = vQORoMUXScEEPILGs26061990;     vQORoMUXScEEPILGs26061990 = vQORoMUXScEEPILGs47261147;     vQORoMUXScEEPILGs47261147 = vQORoMUXScEEPILGs42833916;     vQORoMUXScEEPILGs42833916 = vQORoMUXScEEPILGs81624101;     vQORoMUXScEEPILGs81624101 = vQORoMUXScEEPILGs42922360;     vQORoMUXScEEPILGs42922360 = vQORoMUXScEEPILGs94251284;     vQORoMUXScEEPILGs94251284 = vQORoMUXScEEPILGs61288496;     vQORoMUXScEEPILGs61288496 = vQORoMUXScEEPILGs61385842;     vQORoMUXScEEPILGs61385842 = vQORoMUXScEEPILGs74413327;     vQORoMUXScEEPILGs74413327 = vQORoMUXScEEPILGs72886182;     vQORoMUXScEEPILGs72886182 = vQORoMUXScEEPILGs65374700;     vQORoMUXScEEPILGs65374700 = vQORoMUXScEEPILGs75192875;     vQORoMUXScEEPILGs75192875 = vQORoMUXScEEPILGs65426349;     vQORoMUXScEEPILGs65426349 = vQORoMUXScEEPILGs67883287;     vQORoMUXScEEPILGs67883287 = vQORoMUXScEEPILGs59554606;     vQORoMUXScEEPILGs59554606 = vQORoMUXScEEPILGs2019203;     vQORoMUXScEEPILGs2019203 = vQORoMUXScEEPILGs97220513;     vQORoMUXScEEPILGs97220513 = vQORoMUXScEEPILGs65422450;     vQORoMUXScEEPILGs65422450 = vQORoMUXScEEPILGs69646768;     vQORoMUXScEEPILGs69646768 = vQORoMUXScEEPILGs51975685;     vQORoMUXScEEPILGs51975685 = vQORoMUXScEEPILGs4545368;     vQORoMUXScEEPILGs4545368 = vQORoMUXScEEPILGs7168507;     vQORoMUXScEEPILGs7168507 = vQORoMUXScEEPILGs89195447;     vQORoMUXScEEPILGs89195447 = vQORoMUXScEEPILGs71840671;     vQORoMUXScEEPILGs71840671 = vQORoMUXScEEPILGs99060243;     vQORoMUXScEEPILGs99060243 = vQORoMUXScEEPILGs89322630;     vQORoMUXScEEPILGs89322630 = vQORoMUXScEEPILGs70470246;     vQORoMUXScEEPILGs70470246 = vQORoMUXScEEPILGs96192258;     vQORoMUXScEEPILGs96192258 = vQORoMUXScEEPILGs24562765;     vQORoMUXScEEPILGs24562765 = vQORoMUXScEEPILGs87089502;     vQORoMUXScEEPILGs87089502 = vQORoMUXScEEPILGs40064200;     vQORoMUXScEEPILGs40064200 = vQORoMUXScEEPILGs90285037;     vQORoMUXScEEPILGs90285037 = vQORoMUXScEEPILGs99239975;     vQORoMUXScEEPILGs99239975 = vQORoMUXScEEPILGs32873262;     vQORoMUXScEEPILGs32873262 = vQORoMUXScEEPILGs53781217;     vQORoMUXScEEPILGs53781217 = vQORoMUXScEEPILGs57539516;     vQORoMUXScEEPILGs57539516 = vQORoMUXScEEPILGs62920812;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void obVtVaQvMnluDkMx36269352() {     double nnMNjMmDYmCMkuPgp69037919 = -210549528;    double nnMNjMmDYmCMkuPgp41174290 = 55369881;    double nnMNjMmDYmCMkuPgp23280879 = -292338841;    double nnMNjMmDYmCMkuPgp64823794 = -361642850;    double nnMNjMmDYmCMkuPgp60797790 = -930465822;    double nnMNjMmDYmCMkuPgp72224931 = -980482263;    double nnMNjMmDYmCMkuPgp92690626 = -575985276;    double nnMNjMmDYmCMkuPgp9478915 = -719791083;    double nnMNjMmDYmCMkuPgp26960760 = -66688785;    double nnMNjMmDYmCMkuPgp53456 = -462254551;    double nnMNjMmDYmCMkuPgp21006051 = -562537838;    double nnMNjMmDYmCMkuPgp75630865 = -540302681;    double nnMNjMmDYmCMkuPgp20879151 = -523274056;    double nnMNjMmDYmCMkuPgp9442267 = -333947307;    double nnMNjMmDYmCMkuPgp10538474 = -81222935;    double nnMNjMmDYmCMkuPgp41869575 = -654048460;    double nnMNjMmDYmCMkuPgp90739380 = -260425666;    double nnMNjMmDYmCMkuPgp2070973 = -216435964;    double nnMNjMmDYmCMkuPgp69198226 = -679524522;    double nnMNjMmDYmCMkuPgp29140257 = -49074605;    double nnMNjMmDYmCMkuPgp55787790 = -216764758;    double nnMNjMmDYmCMkuPgp55388237 = -127004032;    double nnMNjMmDYmCMkuPgp83317285 = -938315050;    double nnMNjMmDYmCMkuPgp48347642 = -632707966;    double nnMNjMmDYmCMkuPgp70493892 = -11604611;    double nnMNjMmDYmCMkuPgp73108987 = -162596710;    double nnMNjMmDYmCMkuPgp84849504 = -495608350;    double nnMNjMmDYmCMkuPgp52378431 = -105366883;    double nnMNjMmDYmCMkuPgp33049532 = -861340895;    double nnMNjMmDYmCMkuPgp60366299 = 8515429;    double nnMNjMmDYmCMkuPgp25004356 = -77839047;    double nnMNjMmDYmCMkuPgp11245236 = -351362853;    double nnMNjMmDYmCMkuPgp10684856 = -737490603;    double nnMNjMmDYmCMkuPgp47876494 = -729375773;    double nnMNjMmDYmCMkuPgp44850882 = -317874242;    double nnMNjMmDYmCMkuPgp89270675 = -909570117;    double nnMNjMmDYmCMkuPgp20337620 = 7516995;    double nnMNjMmDYmCMkuPgp69607748 = -834497337;    double nnMNjMmDYmCMkuPgp4214746 = 87624990;    double nnMNjMmDYmCMkuPgp21984140 = -607454149;    double nnMNjMmDYmCMkuPgp88884615 = -449400028;    double nnMNjMmDYmCMkuPgp24192180 = -856075191;    double nnMNjMmDYmCMkuPgp81507842 = -991901080;    double nnMNjMmDYmCMkuPgp67250947 = -796401184;    double nnMNjMmDYmCMkuPgp16753128 = 5751031;    double nnMNjMmDYmCMkuPgp36714235 = -14146929;    double nnMNjMmDYmCMkuPgp68945372 = -977277732;    double nnMNjMmDYmCMkuPgp28924217 = -837996387;    double nnMNjMmDYmCMkuPgp79300266 = -636991692;    double nnMNjMmDYmCMkuPgp27853244 = -965255698;    double nnMNjMmDYmCMkuPgp22905973 = -105277710;    double nnMNjMmDYmCMkuPgp36403020 = -158173268;    double nnMNjMmDYmCMkuPgp72687624 = -56428284;    double nnMNjMmDYmCMkuPgp25102116 = -936705070;    double nnMNjMmDYmCMkuPgp80481561 = -606319809;    double nnMNjMmDYmCMkuPgp13649683 = 16454503;    double nnMNjMmDYmCMkuPgp57857004 = -6315069;    double nnMNjMmDYmCMkuPgp74933237 = -659630875;    double nnMNjMmDYmCMkuPgp94329902 = -250038240;    double nnMNjMmDYmCMkuPgp87688802 = -667869112;    double nnMNjMmDYmCMkuPgp87375427 = -384873913;    double nnMNjMmDYmCMkuPgp40312195 = -370618393;    double nnMNjMmDYmCMkuPgp76429382 = -858450189;    double nnMNjMmDYmCMkuPgp66594461 = 24795786;    double nnMNjMmDYmCMkuPgp75049099 = -284415504;    double nnMNjMmDYmCMkuPgp9760816 = -111174985;    double nnMNjMmDYmCMkuPgp64946009 = -802812078;    double nnMNjMmDYmCMkuPgp73002656 = -793898284;    double nnMNjMmDYmCMkuPgp64591384 = 83926934;    double nnMNjMmDYmCMkuPgp21267799 = -171652818;    double nnMNjMmDYmCMkuPgp21531955 = -561565455;    double nnMNjMmDYmCMkuPgp21131633 = -425928329;    double nnMNjMmDYmCMkuPgp97856226 = -204060955;    double nnMNjMmDYmCMkuPgp47214086 = 27929627;    double nnMNjMmDYmCMkuPgp40255641 = -599674577;    double nnMNjMmDYmCMkuPgp31595611 = -360689567;    double nnMNjMmDYmCMkuPgp73880395 = -135102952;    double nnMNjMmDYmCMkuPgp16066339 = -41913866;    double nnMNjMmDYmCMkuPgp31594515 = -538458998;    double nnMNjMmDYmCMkuPgp33779658 = -997457683;    double nnMNjMmDYmCMkuPgp4163615 = -185318979;    double nnMNjMmDYmCMkuPgp55925287 = -657611963;    double nnMNjMmDYmCMkuPgp73078165 = -468375192;    double nnMNjMmDYmCMkuPgp5196289 = -896085198;    double nnMNjMmDYmCMkuPgp37460326 = -886206861;    double nnMNjMmDYmCMkuPgp88601336 = -919665780;    double nnMNjMmDYmCMkuPgp38557612 = -194934569;    double nnMNjMmDYmCMkuPgp85582739 = -800785534;    double nnMNjMmDYmCMkuPgp67394933 = -23055964;    double nnMNjMmDYmCMkuPgp31201200 = -234328746;    double nnMNjMmDYmCMkuPgp31413672 = -803255049;    double nnMNjMmDYmCMkuPgp45404383 = -332852130;    double nnMNjMmDYmCMkuPgp75277846 = -484459098;    double nnMNjMmDYmCMkuPgp16525943 = -244505898;    double nnMNjMmDYmCMkuPgp34608713 = -122580236;    double nnMNjMmDYmCMkuPgp48572420 = 21218365;    double nnMNjMmDYmCMkuPgp47762797 = -997625003;    double nnMNjMmDYmCMkuPgp14913382 = -916696866;    double nnMNjMmDYmCMkuPgp92201847 = -411985680;    double nnMNjMmDYmCMkuPgp6992313 = -210549528;     nnMNjMmDYmCMkuPgp69037919 = nnMNjMmDYmCMkuPgp41174290;     nnMNjMmDYmCMkuPgp41174290 = nnMNjMmDYmCMkuPgp23280879;     nnMNjMmDYmCMkuPgp23280879 = nnMNjMmDYmCMkuPgp64823794;     nnMNjMmDYmCMkuPgp64823794 = nnMNjMmDYmCMkuPgp60797790;     nnMNjMmDYmCMkuPgp60797790 = nnMNjMmDYmCMkuPgp72224931;     nnMNjMmDYmCMkuPgp72224931 = nnMNjMmDYmCMkuPgp92690626;     nnMNjMmDYmCMkuPgp92690626 = nnMNjMmDYmCMkuPgp9478915;     nnMNjMmDYmCMkuPgp9478915 = nnMNjMmDYmCMkuPgp26960760;     nnMNjMmDYmCMkuPgp26960760 = nnMNjMmDYmCMkuPgp53456;     nnMNjMmDYmCMkuPgp53456 = nnMNjMmDYmCMkuPgp21006051;     nnMNjMmDYmCMkuPgp21006051 = nnMNjMmDYmCMkuPgp75630865;     nnMNjMmDYmCMkuPgp75630865 = nnMNjMmDYmCMkuPgp20879151;     nnMNjMmDYmCMkuPgp20879151 = nnMNjMmDYmCMkuPgp9442267;     nnMNjMmDYmCMkuPgp9442267 = nnMNjMmDYmCMkuPgp10538474;     nnMNjMmDYmCMkuPgp10538474 = nnMNjMmDYmCMkuPgp41869575;     nnMNjMmDYmCMkuPgp41869575 = nnMNjMmDYmCMkuPgp90739380;     nnMNjMmDYmCMkuPgp90739380 = nnMNjMmDYmCMkuPgp2070973;     nnMNjMmDYmCMkuPgp2070973 = nnMNjMmDYmCMkuPgp69198226;     nnMNjMmDYmCMkuPgp69198226 = nnMNjMmDYmCMkuPgp29140257;     nnMNjMmDYmCMkuPgp29140257 = nnMNjMmDYmCMkuPgp55787790;     nnMNjMmDYmCMkuPgp55787790 = nnMNjMmDYmCMkuPgp55388237;     nnMNjMmDYmCMkuPgp55388237 = nnMNjMmDYmCMkuPgp83317285;     nnMNjMmDYmCMkuPgp83317285 = nnMNjMmDYmCMkuPgp48347642;     nnMNjMmDYmCMkuPgp48347642 = nnMNjMmDYmCMkuPgp70493892;     nnMNjMmDYmCMkuPgp70493892 = nnMNjMmDYmCMkuPgp73108987;     nnMNjMmDYmCMkuPgp73108987 = nnMNjMmDYmCMkuPgp84849504;     nnMNjMmDYmCMkuPgp84849504 = nnMNjMmDYmCMkuPgp52378431;     nnMNjMmDYmCMkuPgp52378431 = nnMNjMmDYmCMkuPgp33049532;     nnMNjMmDYmCMkuPgp33049532 = nnMNjMmDYmCMkuPgp60366299;     nnMNjMmDYmCMkuPgp60366299 = nnMNjMmDYmCMkuPgp25004356;     nnMNjMmDYmCMkuPgp25004356 = nnMNjMmDYmCMkuPgp11245236;     nnMNjMmDYmCMkuPgp11245236 = nnMNjMmDYmCMkuPgp10684856;     nnMNjMmDYmCMkuPgp10684856 = nnMNjMmDYmCMkuPgp47876494;     nnMNjMmDYmCMkuPgp47876494 = nnMNjMmDYmCMkuPgp44850882;     nnMNjMmDYmCMkuPgp44850882 = nnMNjMmDYmCMkuPgp89270675;     nnMNjMmDYmCMkuPgp89270675 = nnMNjMmDYmCMkuPgp20337620;     nnMNjMmDYmCMkuPgp20337620 = nnMNjMmDYmCMkuPgp69607748;     nnMNjMmDYmCMkuPgp69607748 = nnMNjMmDYmCMkuPgp4214746;     nnMNjMmDYmCMkuPgp4214746 = nnMNjMmDYmCMkuPgp21984140;     nnMNjMmDYmCMkuPgp21984140 = nnMNjMmDYmCMkuPgp88884615;     nnMNjMmDYmCMkuPgp88884615 = nnMNjMmDYmCMkuPgp24192180;     nnMNjMmDYmCMkuPgp24192180 = nnMNjMmDYmCMkuPgp81507842;     nnMNjMmDYmCMkuPgp81507842 = nnMNjMmDYmCMkuPgp67250947;     nnMNjMmDYmCMkuPgp67250947 = nnMNjMmDYmCMkuPgp16753128;     nnMNjMmDYmCMkuPgp16753128 = nnMNjMmDYmCMkuPgp36714235;     nnMNjMmDYmCMkuPgp36714235 = nnMNjMmDYmCMkuPgp68945372;     nnMNjMmDYmCMkuPgp68945372 = nnMNjMmDYmCMkuPgp28924217;     nnMNjMmDYmCMkuPgp28924217 = nnMNjMmDYmCMkuPgp79300266;     nnMNjMmDYmCMkuPgp79300266 = nnMNjMmDYmCMkuPgp27853244;     nnMNjMmDYmCMkuPgp27853244 = nnMNjMmDYmCMkuPgp22905973;     nnMNjMmDYmCMkuPgp22905973 = nnMNjMmDYmCMkuPgp36403020;     nnMNjMmDYmCMkuPgp36403020 = nnMNjMmDYmCMkuPgp72687624;     nnMNjMmDYmCMkuPgp72687624 = nnMNjMmDYmCMkuPgp25102116;     nnMNjMmDYmCMkuPgp25102116 = nnMNjMmDYmCMkuPgp80481561;     nnMNjMmDYmCMkuPgp80481561 = nnMNjMmDYmCMkuPgp13649683;     nnMNjMmDYmCMkuPgp13649683 = nnMNjMmDYmCMkuPgp57857004;     nnMNjMmDYmCMkuPgp57857004 = nnMNjMmDYmCMkuPgp74933237;     nnMNjMmDYmCMkuPgp74933237 = nnMNjMmDYmCMkuPgp94329902;     nnMNjMmDYmCMkuPgp94329902 = nnMNjMmDYmCMkuPgp87688802;     nnMNjMmDYmCMkuPgp87688802 = nnMNjMmDYmCMkuPgp87375427;     nnMNjMmDYmCMkuPgp87375427 = nnMNjMmDYmCMkuPgp40312195;     nnMNjMmDYmCMkuPgp40312195 = nnMNjMmDYmCMkuPgp76429382;     nnMNjMmDYmCMkuPgp76429382 = nnMNjMmDYmCMkuPgp66594461;     nnMNjMmDYmCMkuPgp66594461 = nnMNjMmDYmCMkuPgp75049099;     nnMNjMmDYmCMkuPgp75049099 = nnMNjMmDYmCMkuPgp9760816;     nnMNjMmDYmCMkuPgp9760816 = nnMNjMmDYmCMkuPgp64946009;     nnMNjMmDYmCMkuPgp64946009 = nnMNjMmDYmCMkuPgp73002656;     nnMNjMmDYmCMkuPgp73002656 = nnMNjMmDYmCMkuPgp64591384;     nnMNjMmDYmCMkuPgp64591384 = nnMNjMmDYmCMkuPgp21267799;     nnMNjMmDYmCMkuPgp21267799 = nnMNjMmDYmCMkuPgp21531955;     nnMNjMmDYmCMkuPgp21531955 = nnMNjMmDYmCMkuPgp21131633;     nnMNjMmDYmCMkuPgp21131633 = nnMNjMmDYmCMkuPgp97856226;     nnMNjMmDYmCMkuPgp97856226 = nnMNjMmDYmCMkuPgp47214086;     nnMNjMmDYmCMkuPgp47214086 = nnMNjMmDYmCMkuPgp40255641;     nnMNjMmDYmCMkuPgp40255641 = nnMNjMmDYmCMkuPgp31595611;     nnMNjMmDYmCMkuPgp31595611 = nnMNjMmDYmCMkuPgp73880395;     nnMNjMmDYmCMkuPgp73880395 = nnMNjMmDYmCMkuPgp16066339;     nnMNjMmDYmCMkuPgp16066339 = nnMNjMmDYmCMkuPgp31594515;     nnMNjMmDYmCMkuPgp31594515 = nnMNjMmDYmCMkuPgp33779658;     nnMNjMmDYmCMkuPgp33779658 = nnMNjMmDYmCMkuPgp4163615;     nnMNjMmDYmCMkuPgp4163615 = nnMNjMmDYmCMkuPgp55925287;     nnMNjMmDYmCMkuPgp55925287 = nnMNjMmDYmCMkuPgp73078165;     nnMNjMmDYmCMkuPgp73078165 = nnMNjMmDYmCMkuPgp5196289;     nnMNjMmDYmCMkuPgp5196289 = nnMNjMmDYmCMkuPgp37460326;     nnMNjMmDYmCMkuPgp37460326 = nnMNjMmDYmCMkuPgp88601336;     nnMNjMmDYmCMkuPgp88601336 = nnMNjMmDYmCMkuPgp38557612;     nnMNjMmDYmCMkuPgp38557612 = nnMNjMmDYmCMkuPgp85582739;     nnMNjMmDYmCMkuPgp85582739 = nnMNjMmDYmCMkuPgp67394933;     nnMNjMmDYmCMkuPgp67394933 = nnMNjMmDYmCMkuPgp31201200;     nnMNjMmDYmCMkuPgp31201200 = nnMNjMmDYmCMkuPgp31413672;     nnMNjMmDYmCMkuPgp31413672 = nnMNjMmDYmCMkuPgp45404383;     nnMNjMmDYmCMkuPgp45404383 = nnMNjMmDYmCMkuPgp75277846;     nnMNjMmDYmCMkuPgp75277846 = nnMNjMmDYmCMkuPgp16525943;     nnMNjMmDYmCMkuPgp16525943 = nnMNjMmDYmCMkuPgp34608713;     nnMNjMmDYmCMkuPgp34608713 = nnMNjMmDYmCMkuPgp48572420;     nnMNjMmDYmCMkuPgp48572420 = nnMNjMmDYmCMkuPgp47762797;     nnMNjMmDYmCMkuPgp47762797 = nnMNjMmDYmCMkuPgp14913382;     nnMNjMmDYmCMkuPgp14913382 = nnMNjMmDYmCMkuPgp92201847;     nnMNjMmDYmCMkuPgp92201847 = nnMNjMmDYmCMkuPgp6992313;     nnMNjMmDYmCMkuPgp6992313 = nnMNjMmDYmCMkuPgp69037919;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nrUJixjtPgVCykVc3560951() {     double NCjwQhcxDfRiCbmTE4496888 = -676826428;    double NCjwQhcxDfRiCbmTE54446368 = -925181725;    double NCjwQhcxDfRiCbmTE60667356 = 41994084;    double NCjwQhcxDfRiCbmTE37047149 = -182793921;    double NCjwQhcxDfRiCbmTE17083060 = 63935333;    double NCjwQhcxDfRiCbmTE8447803 = -142830158;    double NCjwQhcxDfRiCbmTE3348034 = -420711010;    double NCjwQhcxDfRiCbmTE71610475 = -364025224;    double NCjwQhcxDfRiCbmTE28478365 = -555020192;    double NCjwQhcxDfRiCbmTE14956168 = -126941549;    double NCjwQhcxDfRiCbmTE59187786 = -364545567;    double NCjwQhcxDfRiCbmTE76199013 = -404423452;    double NCjwQhcxDfRiCbmTE59113796 = 24713439;    double NCjwQhcxDfRiCbmTE62583129 = -310310717;    double NCjwQhcxDfRiCbmTE68732990 = -584647950;    double NCjwQhcxDfRiCbmTE25410876 = -289860067;    double NCjwQhcxDfRiCbmTE46575317 = -787400218;    double NCjwQhcxDfRiCbmTE1831165 = -289864033;    double NCjwQhcxDfRiCbmTE69748556 = -996087943;    double NCjwQhcxDfRiCbmTE22977476 = -475635671;    double NCjwQhcxDfRiCbmTE68874653 = -974039446;    double NCjwQhcxDfRiCbmTE58809762 = -213640326;    double NCjwQhcxDfRiCbmTE53842665 = -575947234;    double NCjwQhcxDfRiCbmTE42140588 = -43532570;    double NCjwQhcxDfRiCbmTE78511843 = -35589809;    double NCjwQhcxDfRiCbmTE83807159 = -951173517;    double NCjwQhcxDfRiCbmTE45270078 = -301892491;    double NCjwQhcxDfRiCbmTE93106856 = -275139058;    double NCjwQhcxDfRiCbmTE79717746 = -335435698;    double NCjwQhcxDfRiCbmTE65899709 = -25991742;    double NCjwQhcxDfRiCbmTE43587588 = -629819534;    double NCjwQhcxDfRiCbmTE49068254 = -124334267;    double NCjwQhcxDfRiCbmTE22902866 = -885086581;    double NCjwQhcxDfRiCbmTE77624921 = -794069586;    double NCjwQhcxDfRiCbmTE10659336 = -268974493;    double NCjwQhcxDfRiCbmTE40831520 = -24135423;    double NCjwQhcxDfRiCbmTE69749436 = -957179045;    double NCjwQhcxDfRiCbmTE20823152 = -817152918;    double NCjwQhcxDfRiCbmTE34605308 = -663251286;    double NCjwQhcxDfRiCbmTE86255672 = -72716695;    double NCjwQhcxDfRiCbmTE22336224 = -218598942;    double NCjwQhcxDfRiCbmTE13557307 = -450620482;    double NCjwQhcxDfRiCbmTE70891220 = -545081866;    double NCjwQhcxDfRiCbmTE7382286 = -300733594;    double NCjwQhcxDfRiCbmTE16769587 = -920760130;    double NCjwQhcxDfRiCbmTE82675563 = -53070561;    double NCjwQhcxDfRiCbmTE88290536 = -361264092;    double NCjwQhcxDfRiCbmTE46650529 = 95477020;    double NCjwQhcxDfRiCbmTE2192933 = -300728572;    double NCjwQhcxDfRiCbmTE52382833 = -356282514;    double NCjwQhcxDfRiCbmTE3676696 = 20445732;    double NCjwQhcxDfRiCbmTE19888492 = -371825615;    double NCjwQhcxDfRiCbmTE41308773 = -912473838;    double NCjwQhcxDfRiCbmTE41093962 = -28213570;    double NCjwQhcxDfRiCbmTE34686298 = -952809955;    double NCjwQhcxDfRiCbmTE45687126 = -363186103;    double NCjwQhcxDfRiCbmTE603703 = -249234492;    double NCjwQhcxDfRiCbmTE18526769 = -914473346;    double NCjwQhcxDfRiCbmTE58535306 = -47204113;    double NCjwQhcxDfRiCbmTE33275900 = 15108849;    double NCjwQhcxDfRiCbmTE63177725 = -840937668;    double NCjwQhcxDfRiCbmTE10241177 = -45571953;    double NCjwQhcxDfRiCbmTE91892728 = 71410473;    double NCjwQhcxDfRiCbmTE62578655 = -429028450;    double NCjwQhcxDfRiCbmTE71368580 = -497122015;    double NCjwQhcxDfRiCbmTE10119532 = -140211300;    double NCjwQhcxDfRiCbmTE53296147 = -519336872;    double NCjwQhcxDfRiCbmTE81488875 = -181216975;    double NCjwQhcxDfRiCbmTE51923794 = 58663776;    double NCjwQhcxDfRiCbmTE27901471 = -460512527;    double NCjwQhcxDfRiCbmTE55661439 = -332681022;    double NCjwQhcxDfRiCbmTE25752165 = -970247301;    double NCjwQhcxDfRiCbmTE67225856 = -626612747;    double NCjwQhcxDfRiCbmTE83492884 = -823371249;    double NCjwQhcxDfRiCbmTE641253 = -157036729;    double NCjwQhcxDfRiCbmTE55317347 = -423418964;    double NCjwQhcxDfRiCbmTE87918541 = -668558460;    double NCjwQhcxDfRiCbmTE46460380 = -175213640;    double NCjwQhcxDfRiCbmTE25371001 = -122772441;    double NCjwQhcxDfRiCbmTE95836280 = -982519249;    double NCjwQhcxDfRiCbmTE95516622 = -489909425;    double NCjwQhcxDfRiCbmTE98619549 = -297369511;    double NCjwQhcxDfRiCbmTE90913923 = -974410486;    double NCjwQhcxDfRiCbmTE27334914 = -979153184;    double NCjwQhcxDfRiCbmTE62223013 = 53562526;    double NCjwQhcxDfRiCbmTE23699096 = -157993920;    double NCjwQhcxDfRiCbmTE7759482 = -211860429;    double NCjwQhcxDfRiCbmTE81808904 = -756873011;    double NCjwQhcxDfRiCbmTE42938624 = -841259632;    double NCjwQhcxDfRiCbmTE64972209 = -905788391;    double NCjwQhcxDfRiCbmTE40227817 = -774900932;    double NCjwQhcxDfRiCbmTE51222668 = 57294300;    double NCjwQhcxDfRiCbmTE62287846 = -669948806;    double NCjwQhcxDfRiCbmTE1329408 = -578360136;    double NCjwQhcxDfRiCbmTE23077948 = -231779027;    double NCjwQhcxDfRiCbmTE12095047 = -73026990;    double NCjwQhcxDfRiCbmTE21664578 = -422030956;    double NCjwQhcxDfRiCbmTE8312566 = -16053416;    double NCjwQhcxDfRiCbmTE36013706 = -803611579;    double NCjwQhcxDfRiCbmTE6650055 = -676826428;     NCjwQhcxDfRiCbmTE4496888 = NCjwQhcxDfRiCbmTE54446368;     NCjwQhcxDfRiCbmTE54446368 = NCjwQhcxDfRiCbmTE60667356;     NCjwQhcxDfRiCbmTE60667356 = NCjwQhcxDfRiCbmTE37047149;     NCjwQhcxDfRiCbmTE37047149 = NCjwQhcxDfRiCbmTE17083060;     NCjwQhcxDfRiCbmTE17083060 = NCjwQhcxDfRiCbmTE8447803;     NCjwQhcxDfRiCbmTE8447803 = NCjwQhcxDfRiCbmTE3348034;     NCjwQhcxDfRiCbmTE3348034 = NCjwQhcxDfRiCbmTE71610475;     NCjwQhcxDfRiCbmTE71610475 = NCjwQhcxDfRiCbmTE28478365;     NCjwQhcxDfRiCbmTE28478365 = NCjwQhcxDfRiCbmTE14956168;     NCjwQhcxDfRiCbmTE14956168 = NCjwQhcxDfRiCbmTE59187786;     NCjwQhcxDfRiCbmTE59187786 = NCjwQhcxDfRiCbmTE76199013;     NCjwQhcxDfRiCbmTE76199013 = NCjwQhcxDfRiCbmTE59113796;     NCjwQhcxDfRiCbmTE59113796 = NCjwQhcxDfRiCbmTE62583129;     NCjwQhcxDfRiCbmTE62583129 = NCjwQhcxDfRiCbmTE68732990;     NCjwQhcxDfRiCbmTE68732990 = NCjwQhcxDfRiCbmTE25410876;     NCjwQhcxDfRiCbmTE25410876 = NCjwQhcxDfRiCbmTE46575317;     NCjwQhcxDfRiCbmTE46575317 = NCjwQhcxDfRiCbmTE1831165;     NCjwQhcxDfRiCbmTE1831165 = NCjwQhcxDfRiCbmTE69748556;     NCjwQhcxDfRiCbmTE69748556 = NCjwQhcxDfRiCbmTE22977476;     NCjwQhcxDfRiCbmTE22977476 = NCjwQhcxDfRiCbmTE68874653;     NCjwQhcxDfRiCbmTE68874653 = NCjwQhcxDfRiCbmTE58809762;     NCjwQhcxDfRiCbmTE58809762 = NCjwQhcxDfRiCbmTE53842665;     NCjwQhcxDfRiCbmTE53842665 = NCjwQhcxDfRiCbmTE42140588;     NCjwQhcxDfRiCbmTE42140588 = NCjwQhcxDfRiCbmTE78511843;     NCjwQhcxDfRiCbmTE78511843 = NCjwQhcxDfRiCbmTE83807159;     NCjwQhcxDfRiCbmTE83807159 = NCjwQhcxDfRiCbmTE45270078;     NCjwQhcxDfRiCbmTE45270078 = NCjwQhcxDfRiCbmTE93106856;     NCjwQhcxDfRiCbmTE93106856 = NCjwQhcxDfRiCbmTE79717746;     NCjwQhcxDfRiCbmTE79717746 = NCjwQhcxDfRiCbmTE65899709;     NCjwQhcxDfRiCbmTE65899709 = NCjwQhcxDfRiCbmTE43587588;     NCjwQhcxDfRiCbmTE43587588 = NCjwQhcxDfRiCbmTE49068254;     NCjwQhcxDfRiCbmTE49068254 = NCjwQhcxDfRiCbmTE22902866;     NCjwQhcxDfRiCbmTE22902866 = NCjwQhcxDfRiCbmTE77624921;     NCjwQhcxDfRiCbmTE77624921 = NCjwQhcxDfRiCbmTE10659336;     NCjwQhcxDfRiCbmTE10659336 = NCjwQhcxDfRiCbmTE40831520;     NCjwQhcxDfRiCbmTE40831520 = NCjwQhcxDfRiCbmTE69749436;     NCjwQhcxDfRiCbmTE69749436 = NCjwQhcxDfRiCbmTE20823152;     NCjwQhcxDfRiCbmTE20823152 = NCjwQhcxDfRiCbmTE34605308;     NCjwQhcxDfRiCbmTE34605308 = NCjwQhcxDfRiCbmTE86255672;     NCjwQhcxDfRiCbmTE86255672 = NCjwQhcxDfRiCbmTE22336224;     NCjwQhcxDfRiCbmTE22336224 = NCjwQhcxDfRiCbmTE13557307;     NCjwQhcxDfRiCbmTE13557307 = NCjwQhcxDfRiCbmTE70891220;     NCjwQhcxDfRiCbmTE70891220 = NCjwQhcxDfRiCbmTE7382286;     NCjwQhcxDfRiCbmTE7382286 = NCjwQhcxDfRiCbmTE16769587;     NCjwQhcxDfRiCbmTE16769587 = NCjwQhcxDfRiCbmTE82675563;     NCjwQhcxDfRiCbmTE82675563 = NCjwQhcxDfRiCbmTE88290536;     NCjwQhcxDfRiCbmTE88290536 = NCjwQhcxDfRiCbmTE46650529;     NCjwQhcxDfRiCbmTE46650529 = NCjwQhcxDfRiCbmTE2192933;     NCjwQhcxDfRiCbmTE2192933 = NCjwQhcxDfRiCbmTE52382833;     NCjwQhcxDfRiCbmTE52382833 = NCjwQhcxDfRiCbmTE3676696;     NCjwQhcxDfRiCbmTE3676696 = NCjwQhcxDfRiCbmTE19888492;     NCjwQhcxDfRiCbmTE19888492 = NCjwQhcxDfRiCbmTE41308773;     NCjwQhcxDfRiCbmTE41308773 = NCjwQhcxDfRiCbmTE41093962;     NCjwQhcxDfRiCbmTE41093962 = NCjwQhcxDfRiCbmTE34686298;     NCjwQhcxDfRiCbmTE34686298 = NCjwQhcxDfRiCbmTE45687126;     NCjwQhcxDfRiCbmTE45687126 = NCjwQhcxDfRiCbmTE603703;     NCjwQhcxDfRiCbmTE603703 = NCjwQhcxDfRiCbmTE18526769;     NCjwQhcxDfRiCbmTE18526769 = NCjwQhcxDfRiCbmTE58535306;     NCjwQhcxDfRiCbmTE58535306 = NCjwQhcxDfRiCbmTE33275900;     NCjwQhcxDfRiCbmTE33275900 = NCjwQhcxDfRiCbmTE63177725;     NCjwQhcxDfRiCbmTE63177725 = NCjwQhcxDfRiCbmTE10241177;     NCjwQhcxDfRiCbmTE10241177 = NCjwQhcxDfRiCbmTE91892728;     NCjwQhcxDfRiCbmTE91892728 = NCjwQhcxDfRiCbmTE62578655;     NCjwQhcxDfRiCbmTE62578655 = NCjwQhcxDfRiCbmTE71368580;     NCjwQhcxDfRiCbmTE71368580 = NCjwQhcxDfRiCbmTE10119532;     NCjwQhcxDfRiCbmTE10119532 = NCjwQhcxDfRiCbmTE53296147;     NCjwQhcxDfRiCbmTE53296147 = NCjwQhcxDfRiCbmTE81488875;     NCjwQhcxDfRiCbmTE81488875 = NCjwQhcxDfRiCbmTE51923794;     NCjwQhcxDfRiCbmTE51923794 = NCjwQhcxDfRiCbmTE27901471;     NCjwQhcxDfRiCbmTE27901471 = NCjwQhcxDfRiCbmTE55661439;     NCjwQhcxDfRiCbmTE55661439 = NCjwQhcxDfRiCbmTE25752165;     NCjwQhcxDfRiCbmTE25752165 = NCjwQhcxDfRiCbmTE67225856;     NCjwQhcxDfRiCbmTE67225856 = NCjwQhcxDfRiCbmTE83492884;     NCjwQhcxDfRiCbmTE83492884 = NCjwQhcxDfRiCbmTE641253;     NCjwQhcxDfRiCbmTE641253 = NCjwQhcxDfRiCbmTE55317347;     NCjwQhcxDfRiCbmTE55317347 = NCjwQhcxDfRiCbmTE87918541;     NCjwQhcxDfRiCbmTE87918541 = NCjwQhcxDfRiCbmTE46460380;     NCjwQhcxDfRiCbmTE46460380 = NCjwQhcxDfRiCbmTE25371001;     NCjwQhcxDfRiCbmTE25371001 = NCjwQhcxDfRiCbmTE95836280;     NCjwQhcxDfRiCbmTE95836280 = NCjwQhcxDfRiCbmTE95516622;     NCjwQhcxDfRiCbmTE95516622 = NCjwQhcxDfRiCbmTE98619549;     NCjwQhcxDfRiCbmTE98619549 = NCjwQhcxDfRiCbmTE90913923;     NCjwQhcxDfRiCbmTE90913923 = NCjwQhcxDfRiCbmTE27334914;     NCjwQhcxDfRiCbmTE27334914 = NCjwQhcxDfRiCbmTE62223013;     NCjwQhcxDfRiCbmTE62223013 = NCjwQhcxDfRiCbmTE23699096;     NCjwQhcxDfRiCbmTE23699096 = NCjwQhcxDfRiCbmTE7759482;     NCjwQhcxDfRiCbmTE7759482 = NCjwQhcxDfRiCbmTE81808904;     NCjwQhcxDfRiCbmTE81808904 = NCjwQhcxDfRiCbmTE42938624;     NCjwQhcxDfRiCbmTE42938624 = NCjwQhcxDfRiCbmTE64972209;     NCjwQhcxDfRiCbmTE64972209 = NCjwQhcxDfRiCbmTE40227817;     NCjwQhcxDfRiCbmTE40227817 = NCjwQhcxDfRiCbmTE51222668;     NCjwQhcxDfRiCbmTE51222668 = NCjwQhcxDfRiCbmTE62287846;     NCjwQhcxDfRiCbmTE62287846 = NCjwQhcxDfRiCbmTE1329408;     NCjwQhcxDfRiCbmTE1329408 = NCjwQhcxDfRiCbmTE23077948;     NCjwQhcxDfRiCbmTE23077948 = NCjwQhcxDfRiCbmTE12095047;     NCjwQhcxDfRiCbmTE12095047 = NCjwQhcxDfRiCbmTE21664578;     NCjwQhcxDfRiCbmTE21664578 = NCjwQhcxDfRiCbmTE8312566;     NCjwQhcxDfRiCbmTE8312566 = NCjwQhcxDfRiCbmTE36013706;     NCjwQhcxDfRiCbmTE36013706 = NCjwQhcxDfRiCbmTE6650055;     NCjwQhcxDfRiCbmTE6650055 = NCjwQhcxDfRiCbmTE4496888;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LmlEjMxiSjQlzQij18610019() {     double hJrIZJmGMNFVbfTCc10613995 = -788728316;    double hJrIZJmGMNFVbfTCc97819647 = -634781506;    double hJrIZJmGMNFVbfTCc76709539 = -108838837;    double hJrIZJmGMNFVbfTCc64121929 = -373542552;    double hJrIZJmGMNFVbfTCc85978548 = -456714538;    double hJrIZJmGMNFVbfTCc99865005 = -407326393;    double hJrIZJmGMNFVbfTCc63836999 = -559675987;    double hJrIZJmGMNFVbfTCc21370366 = -650899137;    double hJrIZJmGMNFVbfTCc93120860 = -323052857;    double hJrIZJmGMNFVbfTCc56185827 = -455917026;    double hJrIZJmGMNFVbfTCc19298076 = -241983154;    double hJrIZJmGMNFVbfTCc8022056 = -848829654;    double hJrIZJmGMNFVbfTCc45322914 = -415177650;    double hJrIZJmGMNFVbfTCc45726806 = -842549285;    double hJrIZJmGMNFVbfTCc64475417 = -175110116;    double hJrIZJmGMNFVbfTCc66916269 = -262859610;    double hJrIZJmGMNFVbfTCc40411320 = -303945331;    double hJrIZJmGMNFVbfTCc98884317 = 44613324;    double hJrIZJmGMNFVbfTCc31578398 = -301857040;    double hJrIZJmGMNFVbfTCc3527641 = -832538590;    double hJrIZJmGMNFVbfTCc57427847 = -829526902;    double hJrIZJmGMNFVbfTCc45330785 = -121570042;    double hJrIZJmGMNFVbfTCc89433457 = -322464721;    double hJrIZJmGMNFVbfTCc26132962 = -265837558;    double hJrIZJmGMNFVbfTCc22211493 = -408785842;    double hJrIZJmGMNFVbfTCc8528835 = -829828840;    double hJrIZJmGMNFVbfTCc43729856 = -202554694;    double hJrIZJmGMNFVbfTCc6213982 = -341068011;    double hJrIZJmGMNFVbfTCc45850002 = -557818402;    double hJrIZJmGMNFVbfTCc34515346 = -943462609;    double hJrIZJmGMNFVbfTCc35830137 = -543140472;    double hJrIZJmGMNFVbfTCc46678876 = -933781083;    double hJrIZJmGMNFVbfTCc32613815 = -777699951;    double hJrIZJmGMNFVbfTCc72455485 = -545611351;    double hJrIZJmGMNFVbfTCc72133987 = -907707598;    double hJrIZJmGMNFVbfTCc9557433 = -238161531;    double hJrIZJmGMNFVbfTCc51011371 = -910814915;    double hJrIZJmGMNFVbfTCc54913364 = -588658928;    double hJrIZJmGMNFVbfTCc8215562 = -657174783;    double hJrIZJmGMNFVbfTCc73757610 = -546155917;    double hJrIZJmGMNFVbfTCc28005448 = -283872836;    double hJrIZJmGMNFVbfTCc45707763 = -720762760;    double hJrIZJmGMNFVbfTCc48958199 = -625886449;    double hJrIZJmGMNFVbfTCc94790025 = -320076532;    double hJrIZJmGMNFVbfTCc28722051 = -582658284;    double hJrIZJmGMNFVbfTCc94614757 = -38286677;    double hJrIZJmGMNFVbfTCc6069111 = -480039630;    double hJrIZJmGMNFVbfTCc54607470 = 85581487;    double hJrIZJmGMNFVbfTCc11868662 = -386465560;    double hJrIZJmGMNFVbfTCc65294486 = -443293322;    double hJrIZJmGMNFVbfTCc39377374 = -921847939;    double hJrIZJmGMNFVbfTCc30698212 = -172551935;    double hJrIZJmGMNFVbfTCc89557230 = -704263426;    double hJrIZJmGMNFVbfTCc37062842 = -348683715;    double hJrIZJmGMNFVbfTCc61182172 = -553341738;    double hJrIZJmGMNFVbfTCc65283209 = -567158275;    double hJrIZJmGMNFVbfTCc8386191 = -212316786;    double hJrIZJmGMNFVbfTCc50576578 = -843001279;    double hJrIZJmGMNFVbfTCc41910437 = -964756710;    double hJrIZJmGMNFVbfTCc77449713 = -626885699;    double hJrIZJmGMNFVbfTCc56135149 = -104771700;    double hJrIZJmGMNFVbfTCc57623017 = -118607976;    double hJrIZJmGMNFVbfTCc75520364 = 6919265;    double hJrIZJmGMNFVbfTCc58605514 = -379590249;    double hJrIZJmGMNFVbfTCc20355690 = -912776555;    double hJrIZJmGMNFVbfTCc72619200 = -308202071;    double hJrIZJmGMNFVbfTCc75408241 = 28870296;    double hJrIZJmGMNFVbfTCc72867429 = -869566299;    double hJrIZJmGMNFVbfTCc73592818 = -934841687;    double hJrIZJmGMNFVbfTCc54917984 = -936948586;    double hJrIZJmGMNFVbfTCc15904899 = -352044696;    double hJrIZJmGMNFVbfTCc85497955 = -715286404;    double hJrIZJmGMNFVbfTCc90668755 = -298211893;    double hJrIZJmGMNFVbfTCc57820788 = -755701123;    double hJrIZJmGMNFVbfTCc75522193 = -448665754;    double hJrIZJmGMNFVbfTCc11720084 = -8764142;    double hJrIZJmGMNFVbfTCc96372586 = -495683593;    double hJrIZJmGMNFVbfTCc94643431 = 97611811;    double hJrIZJmGMNFVbfTCc97410910 = -683179274;    double hJrIZJmGMNFVbfTCc27596735 = -270499165;    double hJrIZJmGMNFVbfTCc2459724 = -249789211;    double hJrIZJmGMNFVbfTCc89122386 = -188136181;    double hJrIZJmGMNFVbfTCc94345320 = -954602451;    double hJrIZJmGMNFVbfTCc80555516 = -14525081;    double hJrIZJmGMNFVbfTCc95137971 = 78385330;    double hJrIZJmGMNFVbfTCc5131926 = -270588538;    double hJrIZJmGMNFVbfTCc57121646 = -129517657;    double hJrIZJmGMNFVbfTCc95550972 = -329016236;    double hJrIZJmGMNFVbfTCc11273313 = -992269614;    double hJrIZJmGMNFVbfTCc6850779 = -240549324;    double hJrIZJmGMNFVbfTCc1171243 = 74155254;    double hJrIZJmGMNFVbfTCc434793 = 32186364;    double hJrIZJmGMNFVbfTCc13002928 = -623902218;    double hJrIZJmGMNFVbfTCc30765848 = 69710915;    double hJrIZJmGMNFVbfTCc17622461 = -341384218;    double hJrIZJmGMNFVbfTCc70382430 = -65264860;    double hJrIZJmGMNFVbfTCc70187399 = -627682026;    double hJrIZJmGMNFVbfTCc90352685 = -146296201;    double hJrIZJmGMNFVbfTCc74434336 = -407299978;    double hJrIZJmGMNFVbfTCc56102851 = -788728316;     hJrIZJmGMNFVbfTCc10613995 = hJrIZJmGMNFVbfTCc97819647;     hJrIZJmGMNFVbfTCc97819647 = hJrIZJmGMNFVbfTCc76709539;     hJrIZJmGMNFVbfTCc76709539 = hJrIZJmGMNFVbfTCc64121929;     hJrIZJmGMNFVbfTCc64121929 = hJrIZJmGMNFVbfTCc85978548;     hJrIZJmGMNFVbfTCc85978548 = hJrIZJmGMNFVbfTCc99865005;     hJrIZJmGMNFVbfTCc99865005 = hJrIZJmGMNFVbfTCc63836999;     hJrIZJmGMNFVbfTCc63836999 = hJrIZJmGMNFVbfTCc21370366;     hJrIZJmGMNFVbfTCc21370366 = hJrIZJmGMNFVbfTCc93120860;     hJrIZJmGMNFVbfTCc93120860 = hJrIZJmGMNFVbfTCc56185827;     hJrIZJmGMNFVbfTCc56185827 = hJrIZJmGMNFVbfTCc19298076;     hJrIZJmGMNFVbfTCc19298076 = hJrIZJmGMNFVbfTCc8022056;     hJrIZJmGMNFVbfTCc8022056 = hJrIZJmGMNFVbfTCc45322914;     hJrIZJmGMNFVbfTCc45322914 = hJrIZJmGMNFVbfTCc45726806;     hJrIZJmGMNFVbfTCc45726806 = hJrIZJmGMNFVbfTCc64475417;     hJrIZJmGMNFVbfTCc64475417 = hJrIZJmGMNFVbfTCc66916269;     hJrIZJmGMNFVbfTCc66916269 = hJrIZJmGMNFVbfTCc40411320;     hJrIZJmGMNFVbfTCc40411320 = hJrIZJmGMNFVbfTCc98884317;     hJrIZJmGMNFVbfTCc98884317 = hJrIZJmGMNFVbfTCc31578398;     hJrIZJmGMNFVbfTCc31578398 = hJrIZJmGMNFVbfTCc3527641;     hJrIZJmGMNFVbfTCc3527641 = hJrIZJmGMNFVbfTCc57427847;     hJrIZJmGMNFVbfTCc57427847 = hJrIZJmGMNFVbfTCc45330785;     hJrIZJmGMNFVbfTCc45330785 = hJrIZJmGMNFVbfTCc89433457;     hJrIZJmGMNFVbfTCc89433457 = hJrIZJmGMNFVbfTCc26132962;     hJrIZJmGMNFVbfTCc26132962 = hJrIZJmGMNFVbfTCc22211493;     hJrIZJmGMNFVbfTCc22211493 = hJrIZJmGMNFVbfTCc8528835;     hJrIZJmGMNFVbfTCc8528835 = hJrIZJmGMNFVbfTCc43729856;     hJrIZJmGMNFVbfTCc43729856 = hJrIZJmGMNFVbfTCc6213982;     hJrIZJmGMNFVbfTCc6213982 = hJrIZJmGMNFVbfTCc45850002;     hJrIZJmGMNFVbfTCc45850002 = hJrIZJmGMNFVbfTCc34515346;     hJrIZJmGMNFVbfTCc34515346 = hJrIZJmGMNFVbfTCc35830137;     hJrIZJmGMNFVbfTCc35830137 = hJrIZJmGMNFVbfTCc46678876;     hJrIZJmGMNFVbfTCc46678876 = hJrIZJmGMNFVbfTCc32613815;     hJrIZJmGMNFVbfTCc32613815 = hJrIZJmGMNFVbfTCc72455485;     hJrIZJmGMNFVbfTCc72455485 = hJrIZJmGMNFVbfTCc72133987;     hJrIZJmGMNFVbfTCc72133987 = hJrIZJmGMNFVbfTCc9557433;     hJrIZJmGMNFVbfTCc9557433 = hJrIZJmGMNFVbfTCc51011371;     hJrIZJmGMNFVbfTCc51011371 = hJrIZJmGMNFVbfTCc54913364;     hJrIZJmGMNFVbfTCc54913364 = hJrIZJmGMNFVbfTCc8215562;     hJrIZJmGMNFVbfTCc8215562 = hJrIZJmGMNFVbfTCc73757610;     hJrIZJmGMNFVbfTCc73757610 = hJrIZJmGMNFVbfTCc28005448;     hJrIZJmGMNFVbfTCc28005448 = hJrIZJmGMNFVbfTCc45707763;     hJrIZJmGMNFVbfTCc45707763 = hJrIZJmGMNFVbfTCc48958199;     hJrIZJmGMNFVbfTCc48958199 = hJrIZJmGMNFVbfTCc94790025;     hJrIZJmGMNFVbfTCc94790025 = hJrIZJmGMNFVbfTCc28722051;     hJrIZJmGMNFVbfTCc28722051 = hJrIZJmGMNFVbfTCc94614757;     hJrIZJmGMNFVbfTCc94614757 = hJrIZJmGMNFVbfTCc6069111;     hJrIZJmGMNFVbfTCc6069111 = hJrIZJmGMNFVbfTCc54607470;     hJrIZJmGMNFVbfTCc54607470 = hJrIZJmGMNFVbfTCc11868662;     hJrIZJmGMNFVbfTCc11868662 = hJrIZJmGMNFVbfTCc65294486;     hJrIZJmGMNFVbfTCc65294486 = hJrIZJmGMNFVbfTCc39377374;     hJrIZJmGMNFVbfTCc39377374 = hJrIZJmGMNFVbfTCc30698212;     hJrIZJmGMNFVbfTCc30698212 = hJrIZJmGMNFVbfTCc89557230;     hJrIZJmGMNFVbfTCc89557230 = hJrIZJmGMNFVbfTCc37062842;     hJrIZJmGMNFVbfTCc37062842 = hJrIZJmGMNFVbfTCc61182172;     hJrIZJmGMNFVbfTCc61182172 = hJrIZJmGMNFVbfTCc65283209;     hJrIZJmGMNFVbfTCc65283209 = hJrIZJmGMNFVbfTCc8386191;     hJrIZJmGMNFVbfTCc8386191 = hJrIZJmGMNFVbfTCc50576578;     hJrIZJmGMNFVbfTCc50576578 = hJrIZJmGMNFVbfTCc41910437;     hJrIZJmGMNFVbfTCc41910437 = hJrIZJmGMNFVbfTCc77449713;     hJrIZJmGMNFVbfTCc77449713 = hJrIZJmGMNFVbfTCc56135149;     hJrIZJmGMNFVbfTCc56135149 = hJrIZJmGMNFVbfTCc57623017;     hJrIZJmGMNFVbfTCc57623017 = hJrIZJmGMNFVbfTCc75520364;     hJrIZJmGMNFVbfTCc75520364 = hJrIZJmGMNFVbfTCc58605514;     hJrIZJmGMNFVbfTCc58605514 = hJrIZJmGMNFVbfTCc20355690;     hJrIZJmGMNFVbfTCc20355690 = hJrIZJmGMNFVbfTCc72619200;     hJrIZJmGMNFVbfTCc72619200 = hJrIZJmGMNFVbfTCc75408241;     hJrIZJmGMNFVbfTCc75408241 = hJrIZJmGMNFVbfTCc72867429;     hJrIZJmGMNFVbfTCc72867429 = hJrIZJmGMNFVbfTCc73592818;     hJrIZJmGMNFVbfTCc73592818 = hJrIZJmGMNFVbfTCc54917984;     hJrIZJmGMNFVbfTCc54917984 = hJrIZJmGMNFVbfTCc15904899;     hJrIZJmGMNFVbfTCc15904899 = hJrIZJmGMNFVbfTCc85497955;     hJrIZJmGMNFVbfTCc85497955 = hJrIZJmGMNFVbfTCc90668755;     hJrIZJmGMNFVbfTCc90668755 = hJrIZJmGMNFVbfTCc57820788;     hJrIZJmGMNFVbfTCc57820788 = hJrIZJmGMNFVbfTCc75522193;     hJrIZJmGMNFVbfTCc75522193 = hJrIZJmGMNFVbfTCc11720084;     hJrIZJmGMNFVbfTCc11720084 = hJrIZJmGMNFVbfTCc96372586;     hJrIZJmGMNFVbfTCc96372586 = hJrIZJmGMNFVbfTCc94643431;     hJrIZJmGMNFVbfTCc94643431 = hJrIZJmGMNFVbfTCc97410910;     hJrIZJmGMNFVbfTCc97410910 = hJrIZJmGMNFVbfTCc27596735;     hJrIZJmGMNFVbfTCc27596735 = hJrIZJmGMNFVbfTCc2459724;     hJrIZJmGMNFVbfTCc2459724 = hJrIZJmGMNFVbfTCc89122386;     hJrIZJmGMNFVbfTCc89122386 = hJrIZJmGMNFVbfTCc94345320;     hJrIZJmGMNFVbfTCc94345320 = hJrIZJmGMNFVbfTCc80555516;     hJrIZJmGMNFVbfTCc80555516 = hJrIZJmGMNFVbfTCc95137971;     hJrIZJmGMNFVbfTCc95137971 = hJrIZJmGMNFVbfTCc5131926;     hJrIZJmGMNFVbfTCc5131926 = hJrIZJmGMNFVbfTCc57121646;     hJrIZJmGMNFVbfTCc57121646 = hJrIZJmGMNFVbfTCc95550972;     hJrIZJmGMNFVbfTCc95550972 = hJrIZJmGMNFVbfTCc11273313;     hJrIZJmGMNFVbfTCc11273313 = hJrIZJmGMNFVbfTCc6850779;     hJrIZJmGMNFVbfTCc6850779 = hJrIZJmGMNFVbfTCc1171243;     hJrIZJmGMNFVbfTCc1171243 = hJrIZJmGMNFVbfTCc434793;     hJrIZJmGMNFVbfTCc434793 = hJrIZJmGMNFVbfTCc13002928;     hJrIZJmGMNFVbfTCc13002928 = hJrIZJmGMNFVbfTCc30765848;     hJrIZJmGMNFVbfTCc30765848 = hJrIZJmGMNFVbfTCc17622461;     hJrIZJmGMNFVbfTCc17622461 = hJrIZJmGMNFVbfTCc70382430;     hJrIZJmGMNFVbfTCc70382430 = hJrIZJmGMNFVbfTCc70187399;     hJrIZJmGMNFVbfTCc70187399 = hJrIZJmGMNFVbfTCc90352685;     hJrIZJmGMNFVbfTCc90352685 = hJrIZJmGMNFVbfTCc74434336;     hJrIZJmGMNFVbfTCc74434336 = hJrIZJmGMNFVbfTCc56102851;     hJrIZJmGMNFVbfTCc56102851 = hJrIZJmGMNFVbfTCc10613995;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zrgrYDKRYkPlsZNR85901618() {     double urKRikQZCgFTDuwdH46072963 = -155005216;    double urKRikQZCgFTDuwdH11091726 = -515333113;    double urKRikQZCgFTDuwdH14096017 = -874505912;    double urKRikQZCgFTDuwdH36345285 = -194693623;    double urKRikQZCgFTDuwdH42263817 = -562313384;    double urKRikQZCgFTDuwdH36087878 = -669674288;    double urKRikQZCgFTDuwdH74494405 = -404401721;    double urKRikQZCgFTDuwdH83501927 = -295133277;    double urKRikQZCgFTDuwdH94638465 = -811384264;    double urKRikQZCgFTDuwdH71088539 = -120604024;    double urKRikQZCgFTDuwdH57479811 = -43990883;    double urKRikQZCgFTDuwdH8590205 = -712950426;    double urKRikQZCgFTDuwdH83557559 = -967190155;    double urKRikQZCgFTDuwdH98867669 = -818912694;    double urKRikQZCgFTDuwdH22669934 = -678535131;    double urKRikQZCgFTDuwdH50457570 = -998671217;    double urKRikQZCgFTDuwdH96247255 = -830919884;    double urKRikQZCgFTDuwdH98644508 = -28814744;    double urKRikQZCgFTDuwdH32128728 = -618420461;    double urKRikQZCgFTDuwdH97364858 = -159099656;    double urKRikQZCgFTDuwdH70514709 = -486801591;    double urKRikQZCgFTDuwdH48752310 = -208206336;    double urKRikQZCgFTDuwdH59958837 = 39903095;    double urKRikQZCgFTDuwdH19925907 = -776662162;    double urKRikQZCgFTDuwdH30229444 = -432771041;    double urKRikQZCgFTDuwdH19227007 = -518405647;    double urKRikQZCgFTDuwdH4150431 = -8838834;    double urKRikQZCgFTDuwdH46942407 = -510840186;    double urKRikQZCgFTDuwdH92518216 = -31913205;    double urKRikQZCgFTDuwdH40048757 = -977969780;    double urKRikQZCgFTDuwdH54413368 = 4879040;    double urKRikQZCgFTDuwdH84501894 = -706752497;    double urKRikQZCgFTDuwdH44831825 = -925295928;    double urKRikQZCgFTDuwdH2203912 = -610305165;    double urKRikQZCgFTDuwdH37942441 = -858807849;    double urKRikQZCgFTDuwdH61118277 = -452726837;    double urKRikQZCgFTDuwdH423188 = -775510955;    double urKRikQZCgFTDuwdH6128768 = -571314509;    double urKRikQZCgFTDuwdH38606123 = -308051059;    double urKRikQZCgFTDuwdH38029142 = -11418463;    double urKRikQZCgFTDuwdH61457055 = -53071750;    double urKRikQZCgFTDuwdH35072890 = -315308051;    double urKRikQZCgFTDuwdH38341577 = -179067235;    double urKRikQZCgFTDuwdH34921364 = -924408942;    double urKRikQZCgFTDuwdH28738510 = -409169446;    double urKRikQZCgFTDuwdH40576086 = -77210310;    double urKRikQZCgFTDuwdH25414275 = -964025990;    double urKRikQZCgFTDuwdH72333782 = -80945106;    double urKRikQZCgFTDuwdH34761328 = -50202440;    double urKRikQZCgFTDuwdH89824075 = -934320138;    double urKRikQZCgFTDuwdH20148098 = -796124497;    double urKRikQZCgFTDuwdH14183684 = -386204281;    double urKRikQZCgFTDuwdH58178379 = -460308980;    double urKRikQZCgFTDuwdH53054689 = -540192215;    double urKRikQZCgFTDuwdH15386909 = -899831883;    double urKRikQZCgFTDuwdH97320652 = -946798881;    double urKRikQZCgFTDuwdH51132889 = -455236208;    double urKRikQZCgFTDuwdH94170109 = 2156250;    double urKRikQZCgFTDuwdH6115841 = -761922583;    double urKRikQZCgFTDuwdH23036811 = 56092263;    double urKRikQZCgFTDuwdH31937447 = -560835455;    double urKRikQZCgFTDuwdH27551999 = -893561536;    double urKRikQZCgFTDuwdH90983710 = -163220073;    double urKRikQZCgFTDuwdH54589708 = -833414485;    double urKRikQZCgFTDuwdH16675171 = -25483065;    double urKRikQZCgFTDuwdH72977916 = -337238386;    double urKRikQZCgFTDuwdH63758379 = -787654498;    double urKRikQZCgFTDuwdH81353648 = -256884990;    double urKRikQZCgFTDuwdH60925228 = -960104845;    double urKRikQZCgFTDuwdH61551657 = -125808295;    double urKRikQZCgFTDuwdH50034383 = -123160263;    double urKRikQZCgFTDuwdH90118488 = -159605375;    double urKRikQZCgFTDuwdH60038386 = -720763686;    double urKRikQZCgFTDuwdH94099585 = -507001998;    double urKRikQZCgFTDuwdH35907804 = -6027906;    double urKRikQZCgFTDuwdH35441820 = -71493540;    double urKRikQZCgFTDuwdH10410733 = 70860899;    double urKRikQZCgFTDuwdH25037473 = -35687963;    double urKRikQZCgFTDuwdH91187396 = -267492717;    double urKRikQZCgFTDuwdH89653357 = -255560731;    double urKRikQZCgFTDuwdH93812732 = -554379657;    double urKRikQZCgFTDuwdH31816649 = -927893729;    double urKRikQZCgFTDuwdH12181079 = -360637746;    double urKRikQZCgFTDuwdH2694142 = -97593067;    double urKRikQZCgFTDuwdH19900659 = -81845283;    double urKRikQZCgFTDuwdH40229685 = -608916679;    double urKRikQZCgFTDuwdH26323516 = -146443518;    double urKRikQZCgFTDuwdH91777136 = -285103714;    double urKRikQZCgFTDuwdH86817003 = -710473282;    double urKRikQZCgFTDuwdH40621788 = -912008968;    double urKRikQZCgFTDuwdH9985388 = -997490629;    double urKRikQZCgFTDuwdH6253078 = -677667205;    double urKRikQZCgFTDuwdH12928 = -809391926;    double urKRikQZCgFTDuwdH15569312 = -264143323;    double urKRikQZCgFTDuwdH6091696 = -450583009;    double urKRikQZCgFTDuwdH33905057 = -159510215;    double urKRikQZCgFTDuwdH44089180 = -52087979;    double urKRikQZCgFTDuwdH83751869 = -345652751;    double urKRikQZCgFTDuwdH18246194 = -798925877;    double urKRikQZCgFTDuwdH55760593 = -155005216;     urKRikQZCgFTDuwdH46072963 = urKRikQZCgFTDuwdH11091726;     urKRikQZCgFTDuwdH11091726 = urKRikQZCgFTDuwdH14096017;     urKRikQZCgFTDuwdH14096017 = urKRikQZCgFTDuwdH36345285;     urKRikQZCgFTDuwdH36345285 = urKRikQZCgFTDuwdH42263817;     urKRikQZCgFTDuwdH42263817 = urKRikQZCgFTDuwdH36087878;     urKRikQZCgFTDuwdH36087878 = urKRikQZCgFTDuwdH74494405;     urKRikQZCgFTDuwdH74494405 = urKRikQZCgFTDuwdH83501927;     urKRikQZCgFTDuwdH83501927 = urKRikQZCgFTDuwdH94638465;     urKRikQZCgFTDuwdH94638465 = urKRikQZCgFTDuwdH71088539;     urKRikQZCgFTDuwdH71088539 = urKRikQZCgFTDuwdH57479811;     urKRikQZCgFTDuwdH57479811 = urKRikQZCgFTDuwdH8590205;     urKRikQZCgFTDuwdH8590205 = urKRikQZCgFTDuwdH83557559;     urKRikQZCgFTDuwdH83557559 = urKRikQZCgFTDuwdH98867669;     urKRikQZCgFTDuwdH98867669 = urKRikQZCgFTDuwdH22669934;     urKRikQZCgFTDuwdH22669934 = urKRikQZCgFTDuwdH50457570;     urKRikQZCgFTDuwdH50457570 = urKRikQZCgFTDuwdH96247255;     urKRikQZCgFTDuwdH96247255 = urKRikQZCgFTDuwdH98644508;     urKRikQZCgFTDuwdH98644508 = urKRikQZCgFTDuwdH32128728;     urKRikQZCgFTDuwdH32128728 = urKRikQZCgFTDuwdH97364858;     urKRikQZCgFTDuwdH97364858 = urKRikQZCgFTDuwdH70514709;     urKRikQZCgFTDuwdH70514709 = urKRikQZCgFTDuwdH48752310;     urKRikQZCgFTDuwdH48752310 = urKRikQZCgFTDuwdH59958837;     urKRikQZCgFTDuwdH59958837 = urKRikQZCgFTDuwdH19925907;     urKRikQZCgFTDuwdH19925907 = urKRikQZCgFTDuwdH30229444;     urKRikQZCgFTDuwdH30229444 = urKRikQZCgFTDuwdH19227007;     urKRikQZCgFTDuwdH19227007 = urKRikQZCgFTDuwdH4150431;     urKRikQZCgFTDuwdH4150431 = urKRikQZCgFTDuwdH46942407;     urKRikQZCgFTDuwdH46942407 = urKRikQZCgFTDuwdH92518216;     urKRikQZCgFTDuwdH92518216 = urKRikQZCgFTDuwdH40048757;     urKRikQZCgFTDuwdH40048757 = urKRikQZCgFTDuwdH54413368;     urKRikQZCgFTDuwdH54413368 = urKRikQZCgFTDuwdH84501894;     urKRikQZCgFTDuwdH84501894 = urKRikQZCgFTDuwdH44831825;     urKRikQZCgFTDuwdH44831825 = urKRikQZCgFTDuwdH2203912;     urKRikQZCgFTDuwdH2203912 = urKRikQZCgFTDuwdH37942441;     urKRikQZCgFTDuwdH37942441 = urKRikQZCgFTDuwdH61118277;     urKRikQZCgFTDuwdH61118277 = urKRikQZCgFTDuwdH423188;     urKRikQZCgFTDuwdH423188 = urKRikQZCgFTDuwdH6128768;     urKRikQZCgFTDuwdH6128768 = urKRikQZCgFTDuwdH38606123;     urKRikQZCgFTDuwdH38606123 = urKRikQZCgFTDuwdH38029142;     urKRikQZCgFTDuwdH38029142 = urKRikQZCgFTDuwdH61457055;     urKRikQZCgFTDuwdH61457055 = urKRikQZCgFTDuwdH35072890;     urKRikQZCgFTDuwdH35072890 = urKRikQZCgFTDuwdH38341577;     urKRikQZCgFTDuwdH38341577 = urKRikQZCgFTDuwdH34921364;     urKRikQZCgFTDuwdH34921364 = urKRikQZCgFTDuwdH28738510;     urKRikQZCgFTDuwdH28738510 = urKRikQZCgFTDuwdH40576086;     urKRikQZCgFTDuwdH40576086 = urKRikQZCgFTDuwdH25414275;     urKRikQZCgFTDuwdH25414275 = urKRikQZCgFTDuwdH72333782;     urKRikQZCgFTDuwdH72333782 = urKRikQZCgFTDuwdH34761328;     urKRikQZCgFTDuwdH34761328 = urKRikQZCgFTDuwdH89824075;     urKRikQZCgFTDuwdH89824075 = urKRikQZCgFTDuwdH20148098;     urKRikQZCgFTDuwdH20148098 = urKRikQZCgFTDuwdH14183684;     urKRikQZCgFTDuwdH14183684 = urKRikQZCgFTDuwdH58178379;     urKRikQZCgFTDuwdH58178379 = urKRikQZCgFTDuwdH53054689;     urKRikQZCgFTDuwdH53054689 = urKRikQZCgFTDuwdH15386909;     urKRikQZCgFTDuwdH15386909 = urKRikQZCgFTDuwdH97320652;     urKRikQZCgFTDuwdH97320652 = urKRikQZCgFTDuwdH51132889;     urKRikQZCgFTDuwdH51132889 = urKRikQZCgFTDuwdH94170109;     urKRikQZCgFTDuwdH94170109 = urKRikQZCgFTDuwdH6115841;     urKRikQZCgFTDuwdH6115841 = urKRikQZCgFTDuwdH23036811;     urKRikQZCgFTDuwdH23036811 = urKRikQZCgFTDuwdH31937447;     urKRikQZCgFTDuwdH31937447 = urKRikQZCgFTDuwdH27551999;     urKRikQZCgFTDuwdH27551999 = urKRikQZCgFTDuwdH90983710;     urKRikQZCgFTDuwdH90983710 = urKRikQZCgFTDuwdH54589708;     urKRikQZCgFTDuwdH54589708 = urKRikQZCgFTDuwdH16675171;     urKRikQZCgFTDuwdH16675171 = urKRikQZCgFTDuwdH72977916;     urKRikQZCgFTDuwdH72977916 = urKRikQZCgFTDuwdH63758379;     urKRikQZCgFTDuwdH63758379 = urKRikQZCgFTDuwdH81353648;     urKRikQZCgFTDuwdH81353648 = urKRikQZCgFTDuwdH60925228;     urKRikQZCgFTDuwdH60925228 = urKRikQZCgFTDuwdH61551657;     urKRikQZCgFTDuwdH61551657 = urKRikQZCgFTDuwdH50034383;     urKRikQZCgFTDuwdH50034383 = urKRikQZCgFTDuwdH90118488;     urKRikQZCgFTDuwdH90118488 = urKRikQZCgFTDuwdH60038386;     urKRikQZCgFTDuwdH60038386 = urKRikQZCgFTDuwdH94099585;     urKRikQZCgFTDuwdH94099585 = urKRikQZCgFTDuwdH35907804;     urKRikQZCgFTDuwdH35907804 = urKRikQZCgFTDuwdH35441820;     urKRikQZCgFTDuwdH35441820 = urKRikQZCgFTDuwdH10410733;     urKRikQZCgFTDuwdH10410733 = urKRikQZCgFTDuwdH25037473;     urKRikQZCgFTDuwdH25037473 = urKRikQZCgFTDuwdH91187396;     urKRikQZCgFTDuwdH91187396 = urKRikQZCgFTDuwdH89653357;     urKRikQZCgFTDuwdH89653357 = urKRikQZCgFTDuwdH93812732;     urKRikQZCgFTDuwdH93812732 = urKRikQZCgFTDuwdH31816649;     urKRikQZCgFTDuwdH31816649 = urKRikQZCgFTDuwdH12181079;     urKRikQZCgFTDuwdH12181079 = urKRikQZCgFTDuwdH2694142;     urKRikQZCgFTDuwdH2694142 = urKRikQZCgFTDuwdH19900659;     urKRikQZCgFTDuwdH19900659 = urKRikQZCgFTDuwdH40229685;     urKRikQZCgFTDuwdH40229685 = urKRikQZCgFTDuwdH26323516;     urKRikQZCgFTDuwdH26323516 = urKRikQZCgFTDuwdH91777136;     urKRikQZCgFTDuwdH91777136 = urKRikQZCgFTDuwdH86817003;     urKRikQZCgFTDuwdH86817003 = urKRikQZCgFTDuwdH40621788;     urKRikQZCgFTDuwdH40621788 = urKRikQZCgFTDuwdH9985388;     urKRikQZCgFTDuwdH9985388 = urKRikQZCgFTDuwdH6253078;     urKRikQZCgFTDuwdH6253078 = urKRikQZCgFTDuwdH12928;     urKRikQZCgFTDuwdH12928 = urKRikQZCgFTDuwdH15569312;     urKRikQZCgFTDuwdH15569312 = urKRikQZCgFTDuwdH6091696;     urKRikQZCgFTDuwdH6091696 = urKRikQZCgFTDuwdH33905057;     urKRikQZCgFTDuwdH33905057 = urKRikQZCgFTDuwdH44089180;     urKRikQZCgFTDuwdH44089180 = urKRikQZCgFTDuwdH83751869;     urKRikQZCgFTDuwdH83751869 = urKRikQZCgFTDuwdH18246194;     urKRikQZCgFTDuwdH18246194 = urKRikQZCgFTDuwdH55760593;     urKRikQZCgFTDuwdH55760593 = urKRikQZCgFTDuwdH46072963;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void vhPvjudpieDzrxLK950686() {     double YTbkqMdSuelCFxZJY52190069 = -266907105;    double YTbkqMdSuelCFxZJY54465005 = -224932893;    double YTbkqMdSuelCFxZJY30138200 = 74661167;    double YTbkqMdSuelCFxZJY63420065 = -385442253;    double YTbkqMdSuelCFxZJY11159306 = 17036745;    double YTbkqMdSuelCFxZJY27505080 = -934170523;    double YTbkqMdSuelCFxZJY34983371 = -543366698;    double YTbkqMdSuelCFxZJY33261818 = -582007191;    double YTbkqMdSuelCFxZJY59280960 = -579416930;    double YTbkqMdSuelCFxZJY12318199 = -449579502;    double YTbkqMdSuelCFxZJY17590101 = 78571531;    double YTbkqMdSuelCFxZJY40413247 = -57356627;    double YTbkqMdSuelCFxZJY69766678 = -307081244;    double YTbkqMdSuelCFxZJY82011346 = -251151262;    double YTbkqMdSuelCFxZJY18412361 = -268997298;    double YTbkqMdSuelCFxZJY91962964 = -971670760;    double YTbkqMdSuelCFxZJY90083258 = -347464997;    double YTbkqMdSuelCFxZJY95697661 = -794337387;    double YTbkqMdSuelCFxZJY93958569 = 75810442;    double YTbkqMdSuelCFxZJY77915023 = -516002575;    double YTbkqMdSuelCFxZJY59067903 = -342289046;    double YTbkqMdSuelCFxZJY35273334 = -116136052;    double YTbkqMdSuelCFxZJY95549628 = -806614392;    double YTbkqMdSuelCFxZJY3918281 = -998967150;    double YTbkqMdSuelCFxZJY73929094 = -805967074;    double YTbkqMdSuelCFxZJY43948682 = -397060970;    double YTbkqMdSuelCFxZJY2610209 = 90498963;    double YTbkqMdSuelCFxZJY60049532 = -576769139;    double YTbkqMdSuelCFxZJY58650472 = -254295909;    double YTbkqMdSuelCFxZJY8664394 = -795440647;    double YTbkqMdSuelCFxZJY46655918 = 91558102;    double YTbkqMdSuelCFxZJY82112516 = -416199313;    double YTbkqMdSuelCFxZJY54542774 = -817909299;    double YTbkqMdSuelCFxZJY97034475 = -361846930;    double YTbkqMdSuelCFxZJY99417093 = -397540954;    double YTbkqMdSuelCFxZJY29844190 = -666752944;    double YTbkqMdSuelCFxZJY81685122 = -729146824;    double YTbkqMdSuelCFxZJY40218980 = -342820518;    double YTbkqMdSuelCFxZJY12216377 = -301974556;    double YTbkqMdSuelCFxZJY25531080 = -484857686;    double YTbkqMdSuelCFxZJY67126279 = -118345645;    double YTbkqMdSuelCFxZJY67223347 = -585450329;    double YTbkqMdSuelCFxZJY16408556 = -259871819;    double YTbkqMdSuelCFxZJY22329105 = -943751880;    double YTbkqMdSuelCFxZJY40690975 = -71067600;    double YTbkqMdSuelCFxZJY52515281 = -62426426;    double YTbkqMdSuelCFxZJY43192849 = 17198472;    double YTbkqMdSuelCFxZJY80290722 = -90840639;    double YTbkqMdSuelCFxZJY44437057 = -135939428;    double YTbkqMdSuelCFxZJY2735728 = 78669054;    double YTbkqMdSuelCFxZJY55848776 = -638418168;    double YTbkqMdSuelCFxZJY24993404 = -186930601;    double YTbkqMdSuelCFxZJY6426836 = -252098568;    double YTbkqMdSuelCFxZJY49023569 = -860662360;    double YTbkqMdSuelCFxZJY41882783 = -500363666;    double YTbkqMdSuelCFxZJY16916736 = -50771053;    double YTbkqMdSuelCFxZJY58915376 = -418318502;    double YTbkqMdSuelCFxZJY26219920 = 73628317;    double YTbkqMdSuelCFxZJY89490971 = -579475180;    double YTbkqMdSuelCFxZJY67210624 = -585902285;    double YTbkqMdSuelCFxZJY24894871 = -924669487;    double YTbkqMdSuelCFxZJY74933839 = -966597560;    double YTbkqMdSuelCFxZJY74611346 = -227711282;    double YTbkqMdSuelCFxZJY50616567 = -783976283;    double YTbkqMdSuelCFxZJY65662280 = -441137605;    double YTbkqMdSuelCFxZJY35477585 = -505229157;    double YTbkqMdSuelCFxZJY85870472 = -239447329;    double YTbkqMdSuelCFxZJY72732202 = -945234314;    double YTbkqMdSuelCFxZJY82594253 = -853610308;    double YTbkqMdSuelCFxZJY88568170 = -602244354;    double YTbkqMdSuelCFxZJY10277842 = -142523936;    double YTbkqMdSuelCFxZJY49864279 = 95355521;    double YTbkqMdSuelCFxZJY83481285 = -392362831;    double YTbkqMdSuelCFxZJY68427489 = -439331873;    double YTbkqMdSuelCFxZJY10788745 = -297656931;    double YTbkqMdSuelCFxZJY91844556 = -756838717;    double YTbkqMdSuelCFxZJY18864779 = -856264234;    double YTbkqMdSuelCFxZJY73220524 = -862862512;    double YTbkqMdSuelCFxZJY63227305 = -827899551;    double YTbkqMdSuelCFxZJY21413813 = -643540648;    double YTbkqMdSuelCFxZJY755834 = -314259443;    double YTbkqMdSuelCFxZJY22319486 = -818660399;    double YTbkqMdSuelCFxZJY15612476 = -340829711;    double YTbkqMdSuelCFxZJY55914744 = -232964964;    double YTbkqMdSuelCFxZJY52815617 = -57022480;    double YTbkqMdSuelCFxZJY21662515 = -721511297;    double YTbkqMdSuelCFxZJY75685680 = -64100746;    double YTbkqMdSuelCFxZJY5519205 = -957246939;    double YTbkqMdSuelCFxZJY55151693 = -861483265;    double YTbkqMdSuelCFxZJY82500357 = -246769901;    double YTbkqMdSuelCFxZJY70928813 = -148434443;    double YTbkqMdSuelCFxZJY55465203 = -702775142;    double YTbkqMdSuelCFxZJY50728009 = -763345339;    double YTbkqMdSuelCFxZJY45005752 = -716072271;    double YTbkqMdSuelCFxZJY636209 = -560188200;    double YTbkqMdSuelCFxZJY92192439 = -151748086;    double YTbkqMdSuelCFxZJY92612001 = -257739048;    double YTbkqMdSuelCFxZJY65791988 = -475895536;    double YTbkqMdSuelCFxZJY56666824 = -402614276;    double YTbkqMdSuelCFxZJY5213390 = -266907105;     YTbkqMdSuelCFxZJY52190069 = YTbkqMdSuelCFxZJY54465005;     YTbkqMdSuelCFxZJY54465005 = YTbkqMdSuelCFxZJY30138200;     YTbkqMdSuelCFxZJY30138200 = YTbkqMdSuelCFxZJY63420065;     YTbkqMdSuelCFxZJY63420065 = YTbkqMdSuelCFxZJY11159306;     YTbkqMdSuelCFxZJY11159306 = YTbkqMdSuelCFxZJY27505080;     YTbkqMdSuelCFxZJY27505080 = YTbkqMdSuelCFxZJY34983371;     YTbkqMdSuelCFxZJY34983371 = YTbkqMdSuelCFxZJY33261818;     YTbkqMdSuelCFxZJY33261818 = YTbkqMdSuelCFxZJY59280960;     YTbkqMdSuelCFxZJY59280960 = YTbkqMdSuelCFxZJY12318199;     YTbkqMdSuelCFxZJY12318199 = YTbkqMdSuelCFxZJY17590101;     YTbkqMdSuelCFxZJY17590101 = YTbkqMdSuelCFxZJY40413247;     YTbkqMdSuelCFxZJY40413247 = YTbkqMdSuelCFxZJY69766678;     YTbkqMdSuelCFxZJY69766678 = YTbkqMdSuelCFxZJY82011346;     YTbkqMdSuelCFxZJY82011346 = YTbkqMdSuelCFxZJY18412361;     YTbkqMdSuelCFxZJY18412361 = YTbkqMdSuelCFxZJY91962964;     YTbkqMdSuelCFxZJY91962964 = YTbkqMdSuelCFxZJY90083258;     YTbkqMdSuelCFxZJY90083258 = YTbkqMdSuelCFxZJY95697661;     YTbkqMdSuelCFxZJY95697661 = YTbkqMdSuelCFxZJY93958569;     YTbkqMdSuelCFxZJY93958569 = YTbkqMdSuelCFxZJY77915023;     YTbkqMdSuelCFxZJY77915023 = YTbkqMdSuelCFxZJY59067903;     YTbkqMdSuelCFxZJY59067903 = YTbkqMdSuelCFxZJY35273334;     YTbkqMdSuelCFxZJY35273334 = YTbkqMdSuelCFxZJY95549628;     YTbkqMdSuelCFxZJY95549628 = YTbkqMdSuelCFxZJY3918281;     YTbkqMdSuelCFxZJY3918281 = YTbkqMdSuelCFxZJY73929094;     YTbkqMdSuelCFxZJY73929094 = YTbkqMdSuelCFxZJY43948682;     YTbkqMdSuelCFxZJY43948682 = YTbkqMdSuelCFxZJY2610209;     YTbkqMdSuelCFxZJY2610209 = YTbkqMdSuelCFxZJY60049532;     YTbkqMdSuelCFxZJY60049532 = YTbkqMdSuelCFxZJY58650472;     YTbkqMdSuelCFxZJY58650472 = YTbkqMdSuelCFxZJY8664394;     YTbkqMdSuelCFxZJY8664394 = YTbkqMdSuelCFxZJY46655918;     YTbkqMdSuelCFxZJY46655918 = YTbkqMdSuelCFxZJY82112516;     YTbkqMdSuelCFxZJY82112516 = YTbkqMdSuelCFxZJY54542774;     YTbkqMdSuelCFxZJY54542774 = YTbkqMdSuelCFxZJY97034475;     YTbkqMdSuelCFxZJY97034475 = YTbkqMdSuelCFxZJY99417093;     YTbkqMdSuelCFxZJY99417093 = YTbkqMdSuelCFxZJY29844190;     YTbkqMdSuelCFxZJY29844190 = YTbkqMdSuelCFxZJY81685122;     YTbkqMdSuelCFxZJY81685122 = YTbkqMdSuelCFxZJY40218980;     YTbkqMdSuelCFxZJY40218980 = YTbkqMdSuelCFxZJY12216377;     YTbkqMdSuelCFxZJY12216377 = YTbkqMdSuelCFxZJY25531080;     YTbkqMdSuelCFxZJY25531080 = YTbkqMdSuelCFxZJY67126279;     YTbkqMdSuelCFxZJY67126279 = YTbkqMdSuelCFxZJY67223347;     YTbkqMdSuelCFxZJY67223347 = YTbkqMdSuelCFxZJY16408556;     YTbkqMdSuelCFxZJY16408556 = YTbkqMdSuelCFxZJY22329105;     YTbkqMdSuelCFxZJY22329105 = YTbkqMdSuelCFxZJY40690975;     YTbkqMdSuelCFxZJY40690975 = YTbkqMdSuelCFxZJY52515281;     YTbkqMdSuelCFxZJY52515281 = YTbkqMdSuelCFxZJY43192849;     YTbkqMdSuelCFxZJY43192849 = YTbkqMdSuelCFxZJY80290722;     YTbkqMdSuelCFxZJY80290722 = YTbkqMdSuelCFxZJY44437057;     YTbkqMdSuelCFxZJY44437057 = YTbkqMdSuelCFxZJY2735728;     YTbkqMdSuelCFxZJY2735728 = YTbkqMdSuelCFxZJY55848776;     YTbkqMdSuelCFxZJY55848776 = YTbkqMdSuelCFxZJY24993404;     YTbkqMdSuelCFxZJY24993404 = YTbkqMdSuelCFxZJY6426836;     YTbkqMdSuelCFxZJY6426836 = YTbkqMdSuelCFxZJY49023569;     YTbkqMdSuelCFxZJY49023569 = YTbkqMdSuelCFxZJY41882783;     YTbkqMdSuelCFxZJY41882783 = YTbkqMdSuelCFxZJY16916736;     YTbkqMdSuelCFxZJY16916736 = YTbkqMdSuelCFxZJY58915376;     YTbkqMdSuelCFxZJY58915376 = YTbkqMdSuelCFxZJY26219920;     YTbkqMdSuelCFxZJY26219920 = YTbkqMdSuelCFxZJY89490971;     YTbkqMdSuelCFxZJY89490971 = YTbkqMdSuelCFxZJY67210624;     YTbkqMdSuelCFxZJY67210624 = YTbkqMdSuelCFxZJY24894871;     YTbkqMdSuelCFxZJY24894871 = YTbkqMdSuelCFxZJY74933839;     YTbkqMdSuelCFxZJY74933839 = YTbkqMdSuelCFxZJY74611346;     YTbkqMdSuelCFxZJY74611346 = YTbkqMdSuelCFxZJY50616567;     YTbkqMdSuelCFxZJY50616567 = YTbkqMdSuelCFxZJY65662280;     YTbkqMdSuelCFxZJY65662280 = YTbkqMdSuelCFxZJY35477585;     YTbkqMdSuelCFxZJY35477585 = YTbkqMdSuelCFxZJY85870472;     YTbkqMdSuelCFxZJY85870472 = YTbkqMdSuelCFxZJY72732202;     YTbkqMdSuelCFxZJY72732202 = YTbkqMdSuelCFxZJY82594253;     YTbkqMdSuelCFxZJY82594253 = YTbkqMdSuelCFxZJY88568170;     YTbkqMdSuelCFxZJY88568170 = YTbkqMdSuelCFxZJY10277842;     YTbkqMdSuelCFxZJY10277842 = YTbkqMdSuelCFxZJY49864279;     YTbkqMdSuelCFxZJY49864279 = YTbkqMdSuelCFxZJY83481285;     YTbkqMdSuelCFxZJY83481285 = YTbkqMdSuelCFxZJY68427489;     YTbkqMdSuelCFxZJY68427489 = YTbkqMdSuelCFxZJY10788745;     YTbkqMdSuelCFxZJY10788745 = YTbkqMdSuelCFxZJY91844556;     YTbkqMdSuelCFxZJY91844556 = YTbkqMdSuelCFxZJY18864779;     YTbkqMdSuelCFxZJY18864779 = YTbkqMdSuelCFxZJY73220524;     YTbkqMdSuelCFxZJY73220524 = YTbkqMdSuelCFxZJY63227305;     YTbkqMdSuelCFxZJY63227305 = YTbkqMdSuelCFxZJY21413813;     YTbkqMdSuelCFxZJY21413813 = YTbkqMdSuelCFxZJY755834;     YTbkqMdSuelCFxZJY755834 = YTbkqMdSuelCFxZJY22319486;     YTbkqMdSuelCFxZJY22319486 = YTbkqMdSuelCFxZJY15612476;     YTbkqMdSuelCFxZJY15612476 = YTbkqMdSuelCFxZJY55914744;     YTbkqMdSuelCFxZJY55914744 = YTbkqMdSuelCFxZJY52815617;     YTbkqMdSuelCFxZJY52815617 = YTbkqMdSuelCFxZJY21662515;     YTbkqMdSuelCFxZJY21662515 = YTbkqMdSuelCFxZJY75685680;     YTbkqMdSuelCFxZJY75685680 = YTbkqMdSuelCFxZJY5519205;     YTbkqMdSuelCFxZJY5519205 = YTbkqMdSuelCFxZJY55151693;     YTbkqMdSuelCFxZJY55151693 = YTbkqMdSuelCFxZJY82500357;     YTbkqMdSuelCFxZJY82500357 = YTbkqMdSuelCFxZJY70928813;     YTbkqMdSuelCFxZJY70928813 = YTbkqMdSuelCFxZJY55465203;     YTbkqMdSuelCFxZJY55465203 = YTbkqMdSuelCFxZJY50728009;     YTbkqMdSuelCFxZJY50728009 = YTbkqMdSuelCFxZJY45005752;     YTbkqMdSuelCFxZJY45005752 = YTbkqMdSuelCFxZJY636209;     YTbkqMdSuelCFxZJY636209 = YTbkqMdSuelCFxZJY92192439;     YTbkqMdSuelCFxZJY92192439 = YTbkqMdSuelCFxZJY92612001;     YTbkqMdSuelCFxZJY92612001 = YTbkqMdSuelCFxZJY65791988;     YTbkqMdSuelCFxZJY65791988 = YTbkqMdSuelCFxZJY56666824;     YTbkqMdSuelCFxZJY56666824 = YTbkqMdSuelCFxZJY5213390;     YTbkqMdSuelCFxZJY5213390 = YTbkqMdSuelCFxZJY52190069;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NmcPZAhTmoQedohE98558712() {     double pkdJDPtYIuzRVjbND55381758 = 69939616;    double pkdJDPtYIuzRVjbND5086965 = -574462740;    double pkdJDPtYIuzRVjbND35561309 = -545275139;    double pkdJDPtYIuzRVjbND79215167 = -206898445;    double pkdJDPtYIuzRVjbND91167158 = 92816138;    double pkdJDPtYIuzRVjbND64436671 = -927975961;    double pkdJDPtYIuzRVjbND16695813 = 91812934;    double pkdJDPtYIuzRVjbND41852134 = -196269743;    double pkdJDPtYIuzRVjbND70187286 = -905091005;    double pkdJDPtYIuzRVjbND36352510 = -678206563;    double pkdJDPtYIuzRVjbND14702401 = -194704027;    double pkdJDPtYIuzRVjbND34119631 = 14201397;    double pkdJDPtYIuzRVjbND11192190 = -574270764;    double pkdJDPtYIuzRVjbND84800530 = -43119850;    double pkdJDPtYIuzRVjbND83118080 = -492778394;    double pkdJDPtYIuzRVjbND29992642 = -258990345;    double pkdJDPtYIuzRVjbND95910782 = -678119541;    double pkdJDPtYIuzRVjbND33837683 = -155943679;    double pkdJDPtYIuzRVjbND34569930 = -90043557;    double pkdJDPtYIuzRVjbND32633970 = -849831948;    double pkdJDPtYIuzRVjbND38863485 = -522967892;    double pkdJDPtYIuzRVjbND51257488 = -541094552;    double pkdJDPtYIuzRVjbND32898500 = -33583747;    double pkdJDPtYIuzRVjbND22782644 = -174743795;    double pkdJDPtYIuzRVjbND21734677 = -78597944;    double pkdJDPtYIuzRVjbND1708901 = -525823217;    double pkdJDPtYIuzRVjbND8130279 = -18527392;    double pkdJDPtYIuzRVjbND53440407 = -526943907;    double pkdJDPtYIuzRVjbND72313570 = -792402956;    double pkdJDPtYIuzRVjbND28919575 = -459485716;    double pkdJDPtYIuzRVjbND57824426 = 7133989;    double pkdJDPtYIuzRVjbND46485115 = -429745554;    double pkdJDPtYIuzRVjbND80143578 = -938331157;    double pkdJDPtYIuzRVjbND91515696 = 29453216;    double pkdJDPtYIuzRVjbND6950755 = -194534368;    double pkdJDPtYIuzRVjbND35771361 = -948718030;    double pkdJDPtYIuzRVjbND49832163 = -476364196;    double pkdJDPtYIuzRVjbND32083246 = -319172551;    double pkdJDPtYIuzRVjbND40145421 = 28051738;    double pkdJDPtYIuzRVjbND86001932 = -428035662;    double pkdJDPtYIuzRVjbND73375857 = -278172067;    double pkdJDPtYIuzRVjbND8422207 = -430372225;    double pkdJDPtYIuzRVjbND20341943 = -960077871;    double pkdJDPtYIuzRVjbND52910163 = -971768273;    double pkdJDPtYIuzRVjbND48706637 = -674204641;    double pkdJDPtYIuzRVjbND51243290 = -214789539;    double pkdJDPtYIuzRVjbND58361698 = 81859242;    double pkdJDPtYIuzRVjbND19188400 = -36249851;    double pkdJDPtYIuzRVjbND19446861 = -949662818;    double pkdJDPtYIuzRVjbND79507400 = -314358726;    double pkdJDPtYIuzRVjbND70375177 = -110555500;    double pkdJDPtYIuzRVjbND92947983 = -316336247;    double pkdJDPtYIuzRVjbND21634385 = 59860105;    double pkdJDPtYIuzRVjbND67886203 = -331965184;    double pkdJDPtYIuzRVjbND59695227 = -591649246;    double pkdJDPtYIuzRVjbND4124270 = -388965833;    double pkdJDPtYIuzRVjbND72188464 = -440878994;    double pkdJDPtYIuzRVjbND12778665 = -270531344;    double pkdJDPtYIuzRVjbND57480491 = -28300501;    double pkdJDPtYIuzRVjbND89458257 = -381360646;    double pkdJDPtYIuzRVjbND56306393 = -809448569;    double pkdJDPtYIuzRVjbND63255406 = -381243160;    double pkdJDPtYIuzRVjbND69538563 = -403866788;    double pkdJDPtYIuzRVjbND41267711 = -345605290;    double pkdJDPtYIuzRVjbND78528083 = -585340553;    double pkdJDPtYIuzRVjbND68217285 = -764958474;    double pkdJDPtYIuzRVjbND53976052 = -47467447;    double pkdJDPtYIuzRVjbND19676493 = -503723980;    double pkdJDPtYIuzRVjbND77849776 = -848585483;    double pkdJDPtYIuzRVjbND47346720 = -544060365;    double pkdJDPtYIuzRVjbND80160478 = -782626150;    double pkdJDPtYIuzRVjbND63827537 = -258946991;    double pkdJDPtYIuzRVjbND93692262 = -83995417;    double pkdJDPtYIuzRVjbND48567998 = -662007895;    double pkdJDPtYIuzRVjbND59258113 = -471659882;    double pkdJDPtYIuzRVjbND30441279 = 7404332;    double pkdJDPtYIuzRVjbND30915545 = -581016681;    double pkdJDPtYIuzRVjbND79988337 = -61815474;    double pkdJDPtYIuzRVjbND74076007 = -500539154;    double pkdJDPtYIuzRVjbND70491386 = -863808406;    double pkdJDPtYIuzRVjbND43347203 = -507682459;    double pkdJDPtYIuzRVjbND88941878 = -982277542;    double pkdJDPtYIuzRVjbND33993546 = -577281089;    double pkdJDPtYIuzRVjbND92806170 = -378044230;    double pkdJDPtYIuzRVjbND58544398 = -248930216;    double pkdJDPtYIuzRVjbND64876442 = -676529765;    double pkdJDPtYIuzRVjbND24850731 = -389605660;    double pkdJDPtYIuzRVjbND12257375 = -506365973;    double pkdJDPtYIuzRVjbND31820470 = -378897539;    double pkdJDPtYIuzRVjbND2826485 = -805568535;    double pkdJDPtYIuzRVjbND63582896 = -407839036;    double pkdJDPtYIuzRVjbND37053498 = -105832852;    double pkdJDPtYIuzRVjbND74602754 = -190872050;    double pkdJDPtYIuzRVjbND50687163 = -590587617;    double pkdJDPtYIuzRVjbND29695539 = -618587093;    double pkdJDPtYIuzRVjbND10120451 = -896928907;    double pkdJDPtYIuzRVjbND38883643 = 73494562;    double pkdJDPtYIuzRVjbND79074231 = -514472582;    double pkdJDPtYIuzRVjbND74382079 = -286427721;    double pkdJDPtYIuzRVjbND80489351 = 69939616;     pkdJDPtYIuzRVjbND55381758 = pkdJDPtYIuzRVjbND5086965;     pkdJDPtYIuzRVjbND5086965 = pkdJDPtYIuzRVjbND35561309;     pkdJDPtYIuzRVjbND35561309 = pkdJDPtYIuzRVjbND79215167;     pkdJDPtYIuzRVjbND79215167 = pkdJDPtYIuzRVjbND91167158;     pkdJDPtYIuzRVjbND91167158 = pkdJDPtYIuzRVjbND64436671;     pkdJDPtYIuzRVjbND64436671 = pkdJDPtYIuzRVjbND16695813;     pkdJDPtYIuzRVjbND16695813 = pkdJDPtYIuzRVjbND41852134;     pkdJDPtYIuzRVjbND41852134 = pkdJDPtYIuzRVjbND70187286;     pkdJDPtYIuzRVjbND70187286 = pkdJDPtYIuzRVjbND36352510;     pkdJDPtYIuzRVjbND36352510 = pkdJDPtYIuzRVjbND14702401;     pkdJDPtYIuzRVjbND14702401 = pkdJDPtYIuzRVjbND34119631;     pkdJDPtYIuzRVjbND34119631 = pkdJDPtYIuzRVjbND11192190;     pkdJDPtYIuzRVjbND11192190 = pkdJDPtYIuzRVjbND84800530;     pkdJDPtYIuzRVjbND84800530 = pkdJDPtYIuzRVjbND83118080;     pkdJDPtYIuzRVjbND83118080 = pkdJDPtYIuzRVjbND29992642;     pkdJDPtYIuzRVjbND29992642 = pkdJDPtYIuzRVjbND95910782;     pkdJDPtYIuzRVjbND95910782 = pkdJDPtYIuzRVjbND33837683;     pkdJDPtYIuzRVjbND33837683 = pkdJDPtYIuzRVjbND34569930;     pkdJDPtYIuzRVjbND34569930 = pkdJDPtYIuzRVjbND32633970;     pkdJDPtYIuzRVjbND32633970 = pkdJDPtYIuzRVjbND38863485;     pkdJDPtYIuzRVjbND38863485 = pkdJDPtYIuzRVjbND51257488;     pkdJDPtYIuzRVjbND51257488 = pkdJDPtYIuzRVjbND32898500;     pkdJDPtYIuzRVjbND32898500 = pkdJDPtYIuzRVjbND22782644;     pkdJDPtYIuzRVjbND22782644 = pkdJDPtYIuzRVjbND21734677;     pkdJDPtYIuzRVjbND21734677 = pkdJDPtYIuzRVjbND1708901;     pkdJDPtYIuzRVjbND1708901 = pkdJDPtYIuzRVjbND8130279;     pkdJDPtYIuzRVjbND8130279 = pkdJDPtYIuzRVjbND53440407;     pkdJDPtYIuzRVjbND53440407 = pkdJDPtYIuzRVjbND72313570;     pkdJDPtYIuzRVjbND72313570 = pkdJDPtYIuzRVjbND28919575;     pkdJDPtYIuzRVjbND28919575 = pkdJDPtYIuzRVjbND57824426;     pkdJDPtYIuzRVjbND57824426 = pkdJDPtYIuzRVjbND46485115;     pkdJDPtYIuzRVjbND46485115 = pkdJDPtYIuzRVjbND80143578;     pkdJDPtYIuzRVjbND80143578 = pkdJDPtYIuzRVjbND91515696;     pkdJDPtYIuzRVjbND91515696 = pkdJDPtYIuzRVjbND6950755;     pkdJDPtYIuzRVjbND6950755 = pkdJDPtYIuzRVjbND35771361;     pkdJDPtYIuzRVjbND35771361 = pkdJDPtYIuzRVjbND49832163;     pkdJDPtYIuzRVjbND49832163 = pkdJDPtYIuzRVjbND32083246;     pkdJDPtYIuzRVjbND32083246 = pkdJDPtYIuzRVjbND40145421;     pkdJDPtYIuzRVjbND40145421 = pkdJDPtYIuzRVjbND86001932;     pkdJDPtYIuzRVjbND86001932 = pkdJDPtYIuzRVjbND73375857;     pkdJDPtYIuzRVjbND73375857 = pkdJDPtYIuzRVjbND8422207;     pkdJDPtYIuzRVjbND8422207 = pkdJDPtYIuzRVjbND20341943;     pkdJDPtYIuzRVjbND20341943 = pkdJDPtYIuzRVjbND52910163;     pkdJDPtYIuzRVjbND52910163 = pkdJDPtYIuzRVjbND48706637;     pkdJDPtYIuzRVjbND48706637 = pkdJDPtYIuzRVjbND51243290;     pkdJDPtYIuzRVjbND51243290 = pkdJDPtYIuzRVjbND58361698;     pkdJDPtYIuzRVjbND58361698 = pkdJDPtYIuzRVjbND19188400;     pkdJDPtYIuzRVjbND19188400 = pkdJDPtYIuzRVjbND19446861;     pkdJDPtYIuzRVjbND19446861 = pkdJDPtYIuzRVjbND79507400;     pkdJDPtYIuzRVjbND79507400 = pkdJDPtYIuzRVjbND70375177;     pkdJDPtYIuzRVjbND70375177 = pkdJDPtYIuzRVjbND92947983;     pkdJDPtYIuzRVjbND92947983 = pkdJDPtYIuzRVjbND21634385;     pkdJDPtYIuzRVjbND21634385 = pkdJDPtYIuzRVjbND67886203;     pkdJDPtYIuzRVjbND67886203 = pkdJDPtYIuzRVjbND59695227;     pkdJDPtYIuzRVjbND59695227 = pkdJDPtYIuzRVjbND4124270;     pkdJDPtYIuzRVjbND4124270 = pkdJDPtYIuzRVjbND72188464;     pkdJDPtYIuzRVjbND72188464 = pkdJDPtYIuzRVjbND12778665;     pkdJDPtYIuzRVjbND12778665 = pkdJDPtYIuzRVjbND57480491;     pkdJDPtYIuzRVjbND57480491 = pkdJDPtYIuzRVjbND89458257;     pkdJDPtYIuzRVjbND89458257 = pkdJDPtYIuzRVjbND56306393;     pkdJDPtYIuzRVjbND56306393 = pkdJDPtYIuzRVjbND63255406;     pkdJDPtYIuzRVjbND63255406 = pkdJDPtYIuzRVjbND69538563;     pkdJDPtYIuzRVjbND69538563 = pkdJDPtYIuzRVjbND41267711;     pkdJDPtYIuzRVjbND41267711 = pkdJDPtYIuzRVjbND78528083;     pkdJDPtYIuzRVjbND78528083 = pkdJDPtYIuzRVjbND68217285;     pkdJDPtYIuzRVjbND68217285 = pkdJDPtYIuzRVjbND53976052;     pkdJDPtYIuzRVjbND53976052 = pkdJDPtYIuzRVjbND19676493;     pkdJDPtYIuzRVjbND19676493 = pkdJDPtYIuzRVjbND77849776;     pkdJDPtYIuzRVjbND77849776 = pkdJDPtYIuzRVjbND47346720;     pkdJDPtYIuzRVjbND47346720 = pkdJDPtYIuzRVjbND80160478;     pkdJDPtYIuzRVjbND80160478 = pkdJDPtYIuzRVjbND63827537;     pkdJDPtYIuzRVjbND63827537 = pkdJDPtYIuzRVjbND93692262;     pkdJDPtYIuzRVjbND93692262 = pkdJDPtYIuzRVjbND48567998;     pkdJDPtYIuzRVjbND48567998 = pkdJDPtYIuzRVjbND59258113;     pkdJDPtYIuzRVjbND59258113 = pkdJDPtYIuzRVjbND30441279;     pkdJDPtYIuzRVjbND30441279 = pkdJDPtYIuzRVjbND30915545;     pkdJDPtYIuzRVjbND30915545 = pkdJDPtYIuzRVjbND79988337;     pkdJDPtYIuzRVjbND79988337 = pkdJDPtYIuzRVjbND74076007;     pkdJDPtYIuzRVjbND74076007 = pkdJDPtYIuzRVjbND70491386;     pkdJDPtYIuzRVjbND70491386 = pkdJDPtYIuzRVjbND43347203;     pkdJDPtYIuzRVjbND43347203 = pkdJDPtYIuzRVjbND88941878;     pkdJDPtYIuzRVjbND88941878 = pkdJDPtYIuzRVjbND33993546;     pkdJDPtYIuzRVjbND33993546 = pkdJDPtYIuzRVjbND92806170;     pkdJDPtYIuzRVjbND92806170 = pkdJDPtYIuzRVjbND58544398;     pkdJDPtYIuzRVjbND58544398 = pkdJDPtYIuzRVjbND64876442;     pkdJDPtYIuzRVjbND64876442 = pkdJDPtYIuzRVjbND24850731;     pkdJDPtYIuzRVjbND24850731 = pkdJDPtYIuzRVjbND12257375;     pkdJDPtYIuzRVjbND12257375 = pkdJDPtYIuzRVjbND31820470;     pkdJDPtYIuzRVjbND31820470 = pkdJDPtYIuzRVjbND2826485;     pkdJDPtYIuzRVjbND2826485 = pkdJDPtYIuzRVjbND63582896;     pkdJDPtYIuzRVjbND63582896 = pkdJDPtYIuzRVjbND37053498;     pkdJDPtYIuzRVjbND37053498 = pkdJDPtYIuzRVjbND74602754;     pkdJDPtYIuzRVjbND74602754 = pkdJDPtYIuzRVjbND50687163;     pkdJDPtYIuzRVjbND50687163 = pkdJDPtYIuzRVjbND29695539;     pkdJDPtYIuzRVjbND29695539 = pkdJDPtYIuzRVjbND10120451;     pkdJDPtYIuzRVjbND10120451 = pkdJDPtYIuzRVjbND38883643;     pkdJDPtYIuzRVjbND38883643 = pkdJDPtYIuzRVjbND79074231;     pkdJDPtYIuzRVjbND79074231 = pkdJDPtYIuzRVjbND74382079;     pkdJDPtYIuzRVjbND74382079 = pkdJDPtYIuzRVjbND80489351;     pkdJDPtYIuzRVjbND80489351 = pkdJDPtYIuzRVjbND55381758;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UKglSXleXEOSQPov13607781() {     double raFKgHMKnKsNjheSI61498864 = -41962273;    double raFKgHMKnKsNjheSI48460244 = -284062521;    double raFKgHMKnKsNjheSI51603493 = -696108060;    double raFKgHMKnKsNjheSI6289948 = -397647075;    double raFKgHMKnKsNjheSI60062647 = -427833734;    double raFKgHMKnKsNjheSI55853874 = -92472196;    double raFKgHMKnKsNjheSI77184778 = -47152043;    double raFKgHMKnKsNjheSI91612025 = -483143656;    double raFKgHMKnKsNjheSI34829782 = -673123671;    double raFKgHMKnKsNjheSI77582169 = 92817959;    double raFKgHMKnKsNjheSI74812691 = -72141614;    double raFKgHMKnKsNjheSI65942673 = -430204805;    double raFKgHMKnKsNjheSI97401307 = 85838147;    double raFKgHMKnKsNjheSI67944207 = -575358418;    double raFKgHMKnKsNjheSI78860507 = -83240561;    double raFKgHMKnKsNjheSI71498035 = -231989889;    double raFKgHMKnKsNjheSI89746785 = -194664653;    double raFKgHMKnKsNjheSI30890836 = -921466322;    double raFKgHMKnKsNjheSI96399771 = -495812654;    double raFKgHMKnKsNjheSI13184135 = -106734868;    double raFKgHMKnKsNjheSI27416679 = -378455348;    double raFKgHMKnKsNjheSI37778512 = -449024268;    double raFKgHMKnKsNjheSI68489292 = -880101234;    double raFKgHMKnKsNjheSI6775018 = -397048783;    double raFKgHMKnKsNjheSI65434326 = -451793977;    double raFKgHMKnKsNjheSI26430576 = -404478540;    double raFKgHMKnKsNjheSI6590057 = 80810405;    double raFKgHMKnKsNjheSI66547532 = -592872860;    double raFKgHMKnKsNjheSI38445826 = 85214340;    double raFKgHMKnKsNjheSI97535211 = -276956583;    double raFKgHMKnKsNjheSI50066975 = 93813051;    double raFKgHMKnKsNjheSI44095737 = -139192370;    double raFKgHMKnKsNjheSI89854527 = -830944527;    double raFKgHMKnKsNjheSI86346261 = -822088549;    double raFKgHMKnKsNjheSI68425407 = -833267473;    double raFKgHMKnKsNjheSI4497274 = -62744138;    double raFKgHMKnKsNjheSI31094098 = -430000065;    double raFKgHMKnKsNjheSI66173458 = -90678560;    double raFKgHMKnKsNjheSI13755675 = 34128241;    double raFKgHMKnKsNjheSI73503869 = -901474885;    double raFKgHMKnKsNjheSI79045080 = -343445961;    double raFKgHMKnKsNjheSI40572664 = -700514503;    double raFKgHMKnKsNjheSI98408921 = 59117545;    double raFKgHMKnKsNjheSI40317903 = -991111212;    double raFKgHMKnKsNjheSI60659102 = -336102796;    double raFKgHMKnKsNjheSI63182485 = -200005655;    double raFKgHMKnKsNjheSI76140272 = -36916295;    double raFKgHMKnKsNjheSI27145341 = -46145383;    double raFKgHMKnKsNjheSI29122590 = 64600194;    double raFKgHMKnKsNjheSI92419052 = -401369535;    double raFKgHMKnKsNjheSI6075856 = 47150828;    double raFKgHMKnKsNjheSI3757703 = -117062567;    double raFKgHMKnKsNjheSI69882842 = -831929482;    double raFKgHMKnKsNjheSI63855084 = -652435329;    double raFKgHMKnKsNjheSI86191102 = -192181028;    double raFKgHMKnKsNjheSI23720353 = -592938005;    double raFKgHMKnKsNjheSI79970952 = -403961288;    double raFKgHMKnKsNjheSI44828475 = -199059277;    double raFKgHMKnKsNjheSI40855622 = -945853099;    double raFKgHMKnKsNjheSI33632071 = 76644806;    double raFKgHMKnKsNjheSI49263817 = -73282601;    double raFKgHMKnKsNjheSI10637247 = -454279183;    double raFKgHMKnKsNjheSI53166199 = -468357996;    double raFKgHMKnKsNjheSI37294570 = -296167088;    double raFKgHMKnKsNjheSI27515194 = 99004908;    double raFKgHMKnKsNjheSI30716954 = -932949245;    double raFKgHMKnKsNjheSI76088146 = -599260279;    double raFKgHMKnKsNjheSI11055047 = -92073304;    double raFKgHMKnKsNjheSI99518800 = -742090945;    double raFKgHMKnKsNjheSI74363233 = 79503576;    double raFKgHMKnKsNjheSI40403938 = -801989824;    double raFKgHMKnKsNjheSI23573328 = -3986094;    double raFKgHMKnKsNjheSI17135162 = -855594563;    double raFKgHMKnKsNjheSI22895902 = -594337770;    double raFKgHMKnKsNjheSI34139054 = -763288908;    double raFKgHMKnKsNjheSI86844015 = -677940846;    double raFKgHMKnKsNjheSI39369591 = -408141814;    double raFKgHMKnKsNjheSI28171389 = -888990023;    double raFKgHMKnKsNjheSI46115916 = 39054012;    double raFKgHMKnKsNjheSI2251842 = -151788323;    double raFKgHMKnKsNjheSI50290304 = -267562245;    double raFKgHMKnKsNjheSI79444716 = -873044213;    double raFKgHMKnKsNjheSI37424942 = -557473054;    double raFKgHMKnKsNjheSI46026773 = -513416126;    double raFKgHMKnKsNjheSI91459356 = -224107412;    double raFKgHMKnKsNjheSI46309272 = -789124382;    double raFKgHMKnKsNjheSI74212895 = -307262888;    double raFKgHMKnKsNjheSI25999443 = -78509198;    double raFKgHMKnKsNjheSI155159 = -529907522;    double raFKgHMKnKsNjheSI44705054 = -140329468;    double raFKgHMKnKsNjheSI24526322 = -658782850;    double raFKgHMKnKsNjheSI86265623 = -130940789;    double raFKgHMKnKsNjheSI25317837 = -144825462;    double raFKgHMKnKsNjheSI80123603 = 57483435;    double raFKgHMKnKsNjheSI24240053 = -728192284;    double raFKgHMKnKsNjheSI68407834 = -889166778;    double raFKgHMKnKsNjheSI87406464 = -132156507;    double raFKgHMKnKsNjheSI61114351 = -644715367;    double raFKgHMKnKsNjheSI12802710 = -990116120;    double raFKgHMKnKsNjheSI29942148 = -41962273;     raFKgHMKnKsNjheSI61498864 = raFKgHMKnKsNjheSI48460244;     raFKgHMKnKsNjheSI48460244 = raFKgHMKnKsNjheSI51603493;     raFKgHMKnKsNjheSI51603493 = raFKgHMKnKsNjheSI6289948;     raFKgHMKnKsNjheSI6289948 = raFKgHMKnKsNjheSI60062647;     raFKgHMKnKsNjheSI60062647 = raFKgHMKnKsNjheSI55853874;     raFKgHMKnKsNjheSI55853874 = raFKgHMKnKsNjheSI77184778;     raFKgHMKnKsNjheSI77184778 = raFKgHMKnKsNjheSI91612025;     raFKgHMKnKsNjheSI91612025 = raFKgHMKnKsNjheSI34829782;     raFKgHMKnKsNjheSI34829782 = raFKgHMKnKsNjheSI77582169;     raFKgHMKnKsNjheSI77582169 = raFKgHMKnKsNjheSI74812691;     raFKgHMKnKsNjheSI74812691 = raFKgHMKnKsNjheSI65942673;     raFKgHMKnKsNjheSI65942673 = raFKgHMKnKsNjheSI97401307;     raFKgHMKnKsNjheSI97401307 = raFKgHMKnKsNjheSI67944207;     raFKgHMKnKsNjheSI67944207 = raFKgHMKnKsNjheSI78860507;     raFKgHMKnKsNjheSI78860507 = raFKgHMKnKsNjheSI71498035;     raFKgHMKnKsNjheSI71498035 = raFKgHMKnKsNjheSI89746785;     raFKgHMKnKsNjheSI89746785 = raFKgHMKnKsNjheSI30890836;     raFKgHMKnKsNjheSI30890836 = raFKgHMKnKsNjheSI96399771;     raFKgHMKnKsNjheSI96399771 = raFKgHMKnKsNjheSI13184135;     raFKgHMKnKsNjheSI13184135 = raFKgHMKnKsNjheSI27416679;     raFKgHMKnKsNjheSI27416679 = raFKgHMKnKsNjheSI37778512;     raFKgHMKnKsNjheSI37778512 = raFKgHMKnKsNjheSI68489292;     raFKgHMKnKsNjheSI68489292 = raFKgHMKnKsNjheSI6775018;     raFKgHMKnKsNjheSI6775018 = raFKgHMKnKsNjheSI65434326;     raFKgHMKnKsNjheSI65434326 = raFKgHMKnKsNjheSI26430576;     raFKgHMKnKsNjheSI26430576 = raFKgHMKnKsNjheSI6590057;     raFKgHMKnKsNjheSI6590057 = raFKgHMKnKsNjheSI66547532;     raFKgHMKnKsNjheSI66547532 = raFKgHMKnKsNjheSI38445826;     raFKgHMKnKsNjheSI38445826 = raFKgHMKnKsNjheSI97535211;     raFKgHMKnKsNjheSI97535211 = raFKgHMKnKsNjheSI50066975;     raFKgHMKnKsNjheSI50066975 = raFKgHMKnKsNjheSI44095737;     raFKgHMKnKsNjheSI44095737 = raFKgHMKnKsNjheSI89854527;     raFKgHMKnKsNjheSI89854527 = raFKgHMKnKsNjheSI86346261;     raFKgHMKnKsNjheSI86346261 = raFKgHMKnKsNjheSI68425407;     raFKgHMKnKsNjheSI68425407 = raFKgHMKnKsNjheSI4497274;     raFKgHMKnKsNjheSI4497274 = raFKgHMKnKsNjheSI31094098;     raFKgHMKnKsNjheSI31094098 = raFKgHMKnKsNjheSI66173458;     raFKgHMKnKsNjheSI66173458 = raFKgHMKnKsNjheSI13755675;     raFKgHMKnKsNjheSI13755675 = raFKgHMKnKsNjheSI73503869;     raFKgHMKnKsNjheSI73503869 = raFKgHMKnKsNjheSI79045080;     raFKgHMKnKsNjheSI79045080 = raFKgHMKnKsNjheSI40572664;     raFKgHMKnKsNjheSI40572664 = raFKgHMKnKsNjheSI98408921;     raFKgHMKnKsNjheSI98408921 = raFKgHMKnKsNjheSI40317903;     raFKgHMKnKsNjheSI40317903 = raFKgHMKnKsNjheSI60659102;     raFKgHMKnKsNjheSI60659102 = raFKgHMKnKsNjheSI63182485;     raFKgHMKnKsNjheSI63182485 = raFKgHMKnKsNjheSI76140272;     raFKgHMKnKsNjheSI76140272 = raFKgHMKnKsNjheSI27145341;     raFKgHMKnKsNjheSI27145341 = raFKgHMKnKsNjheSI29122590;     raFKgHMKnKsNjheSI29122590 = raFKgHMKnKsNjheSI92419052;     raFKgHMKnKsNjheSI92419052 = raFKgHMKnKsNjheSI6075856;     raFKgHMKnKsNjheSI6075856 = raFKgHMKnKsNjheSI3757703;     raFKgHMKnKsNjheSI3757703 = raFKgHMKnKsNjheSI69882842;     raFKgHMKnKsNjheSI69882842 = raFKgHMKnKsNjheSI63855084;     raFKgHMKnKsNjheSI63855084 = raFKgHMKnKsNjheSI86191102;     raFKgHMKnKsNjheSI86191102 = raFKgHMKnKsNjheSI23720353;     raFKgHMKnKsNjheSI23720353 = raFKgHMKnKsNjheSI79970952;     raFKgHMKnKsNjheSI79970952 = raFKgHMKnKsNjheSI44828475;     raFKgHMKnKsNjheSI44828475 = raFKgHMKnKsNjheSI40855622;     raFKgHMKnKsNjheSI40855622 = raFKgHMKnKsNjheSI33632071;     raFKgHMKnKsNjheSI33632071 = raFKgHMKnKsNjheSI49263817;     raFKgHMKnKsNjheSI49263817 = raFKgHMKnKsNjheSI10637247;     raFKgHMKnKsNjheSI10637247 = raFKgHMKnKsNjheSI53166199;     raFKgHMKnKsNjheSI53166199 = raFKgHMKnKsNjheSI37294570;     raFKgHMKnKsNjheSI37294570 = raFKgHMKnKsNjheSI27515194;     raFKgHMKnKsNjheSI27515194 = raFKgHMKnKsNjheSI30716954;     raFKgHMKnKsNjheSI30716954 = raFKgHMKnKsNjheSI76088146;     raFKgHMKnKsNjheSI76088146 = raFKgHMKnKsNjheSI11055047;     raFKgHMKnKsNjheSI11055047 = raFKgHMKnKsNjheSI99518800;     raFKgHMKnKsNjheSI99518800 = raFKgHMKnKsNjheSI74363233;     raFKgHMKnKsNjheSI74363233 = raFKgHMKnKsNjheSI40403938;     raFKgHMKnKsNjheSI40403938 = raFKgHMKnKsNjheSI23573328;     raFKgHMKnKsNjheSI23573328 = raFKgHMKnKsNjheSI17135162;     raFKgHMKnKsNjheSI17135162 = raFKgHMKnKsNjheSI22895902;     raFKgHMKnKsNjheSI22895902 = raFKgHMKnKsNjheSI34139054;     raFKgHMKnKsNjheSI34139054 = raFKgHMKnKsNjheSI86844015;     raFKgHMKnKsNjheSI86844015 = raFKgHMKnKsNjheSI39369591;     raFKgHMKnKsNjheSI39369591 = raFKgHMKnKsNjheSI28171389;     raFKgHMKnKsNjheSI28171389 = raFKgHMKnKsNjheSI46115916;     raFKgHMKnKsNjheSI46115916 = raFKgHMKnKsNjheSI2251842;     raFKgHMKnKsNjheSI2251842 = raFKgHMKnKsNjheSI50290304;     raFKgHMKnKsNjheSI50290304 = raFKgHMKnKsNjheSI79444716;     raFKgHMKnKsNjheSI79444716 = raFKgHMKnKsNjheSI37424942;     raFKgHMKnKsNjheSI37424942 = raFKgHMKnKsNjheSI46026773;     raFKgHMKnKsNjheSI46026773 = raFKgHMKnKsNjheSI91459356;     raFKgHMKnKsNjheSI91459356 = raFKgHMKnKsNjheSI46309272;     raFKgHMKnKsNjheSI46309272 = raFKgHMKnKsNjheSI74212895;     raFKgHMKnKsNjheSI74212895 = raFKgHMKnKsNjheSI25999443;     raFKgHMKnKsNjheSI25999443 = raFKgHMKnKsNjheSI155159;     raFKgHMKnKsNjheSI155159 = raFKgHMKnKsNjheSI44705054;     raFKgHMKnKsNjheSI44705054 = raFKgHMKnKsNjheSI24526322;     raFKgHMKnKsNjheSI24526322 = raFKgHMKnKsNjheSI86265623;     raFKgHMKnKsNjheSI86265623 = raFKgHMKnKsNjheSI25317837;     raFKgHMKnKsNjheSI25317837 = raFKgHMKnKsNjheSI80123603;     raFKgHMKnKsNjheSI80123603 = raFKgHMKnKsNjheSI24240053;     raFKgHMKnKsNjheSI24240053 = raFKgHMKnKsNjheSI68407834;     raFKgHMKnKsNjheSI68407834 = raFKgHMKnKsNjheSI87406464;     raFKgHMKnKsNjheSI87406464 = raFKgHMKnKsNjheSI61114351;     raFKgHMKnKsNjheSI61114351 = raFKgHMKnKsNjheSI12802710;     raFKgHMKnKsNjheSI12802710 = raFKgHMKnKsNjheSI29942148;     raFKgHMKnKsNjheSI29942148 = raFKgHMKnKsNjheSI61498864;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nRhDFzTgtJYZuLiE80899380() {     double kHBmsCUXQWrFBhAed96957833 = -508239173;    double kHBmsCUXQWrFBhAed61732322 = -164614128;    double kHBmsCUXQWrFBhAed88989969 = -361775135;    double kHBmsCUXQWrFBhAed78513303 = -218798147;    double kHBmsCUXQWrFBhAed16347917 = -533432579;    double kHBmsCUXQWrFBhAed92076746 = -354820091;    double kHBmsCUXQWrFBhAed87842185 = -991877777;    double kHBmsCUXQWrFBhAed53743586 = -127377796;    double kHBmsCUXQWrFBhAed36347387 = -61455078;    double kHBmsCUXQWrFBhAed92484881 = -671869039;    double kHBmsCUXQWrFBhAed12994426 = -974149343;    double kHBmsCUXQWrFBhAed66510821 = -294325577;    double kHBmsCUXQWrFBhAed35635953 = -466174357;    double kHBmsCUXQWrFBhAed21085071 = -551721827;    double kHBmsCUXQWrFBhAed37055023 = -586665576;    double kHBmsCUXQWrFBhAed55039336 = -967801496;    double kHBmsCUXQWrFBhAed45582722 = -721639206;    double kHBmsCUXQWrFBhAed30651027 = -994894390;    double kHBmsCUXQWrFBhAed96950101 = -812376075;    double kHBmsCUXQWrFBhAed7021354 = -533295934;    double kHBmsCUXQWrFBhAed40503541 = -35730037;    double kHBmsCUXQWrFBhAed41200036 = -535660562;    double kHBmsCUXQWrFBhAed39014672 = -517733418;    double kHBmsCUXQWrFBhAed567964 = -907873387;    double kHBmsCUXQWrFBhAed73452277 = -475779176;    double kHBmsCUXQWrFBhAed37128748 = -93055347;    double kHBmsCUXQWrFBhAed67010631 = -825473736;    double kHBmsCUXQWrFBhAed7275958 = -762645035;    double kHBmsCUXQWrFBhAed85114040 = -488880462;    double kHBmsCUXQWrFBhAed3068623 = -311463754;    double kHBmsCUXQWrFBhAed68650207 = -458167436;    double kHBmsCUXQWrFBhAed81918755 = 87836216;    double kHBmsCUXQWrFBhAed2072538 = -978540504;    double kHBmsCUXQWrFBhAed16094688 = -886782363;    double kHBmsCUXQWrFBhAed34233861 = -784367724;    double kHBmsCUXQWrFBhAed56058118 = -277309443;    double kHBmsCUXQWrFBhAed80505914 = -294696106;    double kHBmsCUXQWrFBhAed17388862 = -73334141;    double kHBmsCUXQWrFBhAed44146236 = -716748036;    double kHBmsCUXQWrFBhAed37775402 = -366737431;    double kHBmsCUXQWrFBhAed12496689 = -112644875;    double kHBmsCUXQWrFBhAed29937791 = -295059794;    double kHBmsCUXQWrFBhAed87792299 = -594063241;    double kHBmsCUXQWrFBhAed80449241 = -495443622;    double kHBmsCUXQWrFBhAed60675560 = -162613957;    double kHBmsCUXQWrFBhAed9143814 = -238929288;    double kHBmsCUXQWrFBhAed95485436 = -520902656;    double kHBmsCUXQWrFBhAed44871653 = -212671976;    double kHBmsCUXQWrFBhAed52015256 = -699136686;    double kHBmsCUXQWrFBhAed16948642 = -892396350;    double kHBmsCUXQWrFBhAed86846579 = -927125729;    double kHBmsCUXQWrFBhAed87243175 = -330714914;    double kHBmsCUXQWrFBhAed38503991 = -587975036;    double kHBmsCUXQWrFBhAed79846930 = -843943829;    double kHBmsCUXQWrFBhAed40395839 = -538671174;    double kHBmsCUXQWrFBhAed55757797 = -972578611;    double kHBmsCUXQWrFBhAed22717651 = -646880710;    double kHBmsCUXQWrFBhAed88422006 = -453901748;    double kHBmsCUXQWrFBhAed5061026 = -743018971;    double kHBmsCUXQWrFBhAed79219168 = -340377233;    double kHBmsCUXQWrFBhAed25066115 = -529346356;    double kHBmsCUXQWrFBhAed80566228 = -129232743;    double kHBmsCUXQWrFBhAed68629545 = -638497334;    double kHBmsCUXQWrFBhAed33278764 = -749991325;    double kHBmsCUXQWrFBhAed23834674 = -113701603;    double kHBmsCUXQWrFBhAed31075671 = -961985560;    double kHBmsCUXQWrFBhAed64438284 = -315785073;    double kHBmsCUXQWrFBhAed19541266 = -579391995;    double kHBmsCUXQWrFBhAed86851210 = -767354104;    double kHBmsCUXQWrFBhAed80996905 = -209356133;    double kHBmsCUXQWrFBhAed74533422 = -573105391;    double kHBmsCUXQWrFBhAed28193860 = -548305065;    double kHBmsCUXQWrFBhAed86504791 = -178146355;    double kHBmsCUXQWrFBhAed59174699 = -345638645;    double kHBmsCUXQWrFBhAed94524664 = -320651059;    double kHBmsCUXQWrFBhAed10565751 = -740670243;    double kHBmsCUXQWrFBhAed53407737 = -941597322;    double kHBmsCUXQWrFBhAed58565430 = 77710203;    double kHBmsCUXQWrFBhAed39892403 = -645259431;    double kHBmsCUXQWrFBhAed64308464 = -136849889;    double kHBmsCUXQWrFBhAed41643312 = -572152691;    double kHBmsCUXQWrFBhAed22138979 = -512801760;    double kHBmsCUXQWrFBhAed55260701 = 36491651;    double kHBmsCUXQWrFBhAed68165399 = -596484113;    double kHBmsCUXQWrFBhAed16222044 = -384338025;    double kHBmsCUXQWrFBhAed81407031 = -27452523;    double kHBmsCUXQWrFBhAed43414765 = -324188748;    double kHBmsCUXQWrFBhAed22225608 = -34596676;    double kHBmsCUXQWrFBhAed75698849 = -248111189;    double kHBmsCUXQWrFBhAed78476063 = -811789113;    double kHBmsCUXQWrFBhAed33340467 = -630428734;    double kHBmsCUXQWrFBhAed92083908 = -840794358;    double kHBmsCUXQWrFBhAed12327837 = -330315170;    double kHBmsCUXQWrFBhAed64927067 = -276370803;    double kHBmsCUXQWrFBhAed12709287 = -837391076;    double kHBmsCUXQWrFBhAed31930461 = -983412132;    double kHBmsCUXQWrFBhAed61308245 = -656562460;    double kHBmsCUXQWrFBhAed54513535 = -844071917;    double kHBmsCUXQWrFBhAed56614567 = -281742019;    double kHBmsCUXQWrFBhAed29599890 = -508239173;     kHBmsCUXQWrFBhAed96957833 = kHBmsCUXQWrFBhAed61732322;     kHBmsCUXQWrFBhAed61732322 = kHBmsCUXQWrFBhAed88989969;     kHBmsCUXQWrFBhAed88989969 = kHBmsCUXQWrFBhAed78513303;     kHBmsCUXQWrFBhAed78513303 = kHBmsCUXQWrFBhAed16347917;     kHBmsCUXQWrFBhAed16347917 = kHBmsCUXQWrFBhAed92076746;     kHBmsCUXQWrFBhAed92076746 = kHBmsCUXQWrFBhAed87842185;     kHBmsCUXQWrFBhAed87842185 = kHBmsCUXQWrFBhAed53743586;     kHBmsCUXQWrFBhAed53743586 = kHBmsCUXQWrFBhAed36347387;     kHBmsCUXQWrFBhAed36347387 = kHBmsCUXQWrFBhAed92484881;     kHBmsCUXQWrFBhAed92484881 = kHBmsCUXQWrFBhAed12994426;     kHBmsCUXQWrFBhAed12994426 = kHBmsCUXQWrFBhAed66510821;     kHBmsCUXQWrFBhAed66510821 = kHBmsCUXQWrFBhAed35635953;     kHBmsCUXQWrFBhAed35635953 = kHBmsCUXQWrFBhAed21085071;     kHBmsCUXQWrFBhAed21085071 = kHBmsCUXQWrFBhAed37055023;     kHBmsCUXQWrFBhAed37055023 = kHBmsCUXQWrFBhAed55039336;     kHBmsCUXQWrFBhAed55039336 = kHBmsCUXQWrFBhAed45582722;     kHBmsCUXQWrFBhAed45582722 = kHBmsCUXQWrFBhAed30651027;     kHBmsCUXQWrFBhAed30651027 = kHBmsCUXQWrFBhAed96950101;     kHBmsCUXQWrFBhAed96950101 = kHBmsCUXQWrFBhAed7021354;     kHBmsCUXQWrFBhAed7021354 = kHBmsCUXQWrFBhAed40503541;     kHBmsCUXQWrFBhAed40503541 = kHBmsCUXQWrFBhAed41200036;     kHBmsCUXQWrFBhAed41200036 = kHBmsCUXQWrFBhAed39014672;     kHBmsCUXQWrFBhAed39014672 = kHBmsCUXQWrFBhAed567964;     kHBmsCUXQWrFBhAed567964 = kHBmsCUXQWrFBhAed73452277;     kHBmsCUXQWrFBhAed73452277 = kHBmsCUXQWrFBhAed37128748;     kHBmsCUXQWrFBhAed37128748 = kHBmsCUXQWrFBhAed67010631;     kHBmsCUXQWrFBhAed67010631 = kHBmsCUXQWrFBhAed7275958;     kHBmsCUXQWrFBhAed7275958 = kHBmsCUXQWrFBhAed85114040;     kHBmsCUXQWrFBhAed85114040 = kHBmsCUXQWrFBhAed3068623;     kHBmsCUXQWrFBhAed3068623 = kHBmsCUXQWrFBhAed68650207;     kHBmsCUXQWrFBhAed68650207 = kHBmsCUXQWrFBhAed81918755;     kHBmsCUXQWrFBhAed81918755 = kHBmsCUXQWrFBhAed2072538;     kHBmsCUXQWrFBhAed2072538 = kHBmsCUXQWrFBhAed16094688;     kHBmsCUXQWrFBhAed16094688 = kHBmsCUXQWrFBhAed34233861;     kHBmsCUXQWrFBhAed34233861 = kHBmsCUXQWrFBhAed56058118;     kHBmsCUXQWrFBhAed56058118 = kHBmsCUXQWrFBhAed80505914;     kHBmsCUXQWrFBhAed80505914 = kHBmsCUXQWrFBhAed17388862;     kHBmsCUXQWrFBhAed17388862 = kHBmsCUXQWrFBhAed44146236;     kHBmsCUXQWrFBhAed44146236 = kHBmsCUXQWrFBhAed37775402;     kHBmsCUXQWrFBhAed37775402 = kHBmsCUXQWrFBhAed12496689;     kHBmsCUXQWrFBhAed12496689 = kHBmsCUXQWrFBhAed29937791;     kHBmsCUXQWrFBhAed29937791 = kHBmsCUXQWrFBhAed87792299;     kHBmsCUXQWrFBhAed87792299 = kHBmsCUXQWrFBhAed80449241;     kHBmsCUXQWrFBhAed80449241 = kHBmsCUXQWrFBhAed60675560;     kHBmsCUXQWrFBhAed60675560 = kHBmsCUXQWrFBhAed9143814;     kHBmsCUXQWrFBhAed9143814 = kHBmsCUXQWrFBhAed95485436;     kHBmsCUXQWrFBhAed95485436 = kHBmsCUXQWrFBhAed44871653;     kHBmsCUXQWrFBhAed44871653 = kHBmsCUXQWrFBhAed52015256;     kHBmsCUXQWrFBhAed52015256 = kHBmsCUXQWrFBhAed16948642;     kHBmsCUXQWrFBhAed16948642 = kHBmsCUXQWrFBhAed86846579;     kHBmsCUXQWrFBhAed86846579 = kHBmsCUXQWrFBhAed87243175;     kHBmsCUXQWrFBhAed87243175 = kHBmsCUXQWrFBhAed38503991;     kHBmsCUXQWrFBhAed38503991 = kHBmsCUXQWrFBhAed79846930;     kHBmsCUXQWrFBhAed79846930 = kHBmsCUXQWrFBhAed40395839;     kHBmsCUXQWrFBhAed40395839 = kHBmsCUXQWrFBhAed55757797;     kHBmsCUXQWrFBhAed55757797 = kHBmsCUXQWrFBhAed22717651;     kHBmsCUXQWrFBhAed22717651 = kHBmsCUXQWrFBhAed88422006;     kHBmsCUXQWrFBhAed88422006 = kHBmsCUXQWrFBhAed5061026;     kHBmsCUXQWrFBhAed5061026 = kHBmsCUXQWrFBhAed79219168;     kHBmsCUXQWrFBhAed79219168 = kHBmsCUXQWrFBhAed25066115;     kHBmsCUXQWrFBhAed25066115 = kHBmsCUXQWrFBhAed80566228;     kHBmsCUXQWrFBhAed80566228 = kHBmsCUXQWrFBhAed68629545;     kHBmsCUXQWrFBhAed68629545 = kHBmsCUXQWrFBhAed33278764;     kHBmsCUXQWrFBhAed33278764 = kHBmsCUXQWrFBhAed23834674;     kHBmsCUXQWrFBhAed23834674 = kHBmsCUXQWrFBhAed31075671;     kHBmsCUXQWrFBhAed31075671 = kHBmsCUXQWrFBhAed64438284;     kHBmsCUXQWrFBhAed64438284 = kHBmsCUXQWrFBhAed19541266;     kHBmsCUXQWrFBhAed19541266 = kHBmsCUXQWrFBhAed86851210;     kHBmsCUXQWrFBhAed86851210 = kHBmsCUXQWrFBhAed80996905;     kHBmsCUXQWrFBhAed80996905 = kHBmsCUXQWrFBhAed74533422;     kHBmsCUXQWrFBhAed74533422 = kHBmsCUXQWrFBhAed28193860;     kHBmsCUXQWrFBhAed28193860 = kHBmsCUXQWrFBhAed86504791;     kHBmsCUXQWrFBhAed86504791 = kHBmsCUXQWrFBhAed59174699;     kHBmsCUXQWrFBhAed59174699 = kHBmsCUXQWrFBhAed94524664;     kHBmsCUXQWrFBhAed94524664 = kHBmsCUXQWrFBhAed10565751;     kHBmsCUXQWrFBhAed10565751 = kHBmsCUXQWrFBhAed53407737;     kHBmsCUXQWrFBhAed53407737 = kHBmsCUXQWrFBhAed58565430;     kHBmsCUXQWrFBhAed58565430 = kHBmsCUXQWrFBhAed39892403;     kHBmsCUXQWrFBhAed39892403 = kHBmsCUXQWrFBhAed64308464;     kHBmsCUXQWrFBhAed64308464 = kHBmsCUXQWrFBhAed41643312;     kHBmsCUXQWrFBhAed41643312 = kHBmsCUXQWrFBhAed22138979;     kHBmsCUXQWrFBhAed22138979 = kHBmsCUXQWrFBhAed55260701;     kHBmsCUXQWrFBhAed55260701 = kHBmsCUXQWrFBhAed68165399;     kHBmsCUXQWrFBhAed68165399 = kHBmsCUXQWrFBhAed16222044;     kHBmsCUXQWrFBhAed16222044 = kHBmsCUXQWrFBhAed81407031;     kHBmsCUXQWrFBhAed81407031 = kHBmsCUXQWrFBhAed43414765;     kHBmsCUXQWrFBhAed43414765 = kHBmsCUXQWrFBhAed22225608;     kHBmsCUXQWrFBhAed22225608 = kHBmsCUXQWrFBhAed75698849;     kHBmsCUXQWrFBhAed75698849 = kHBmsCUXQWrFBhAed78476063;     kHBmsCUXQWrFBhAed78476063 = kHBmsCUXQWrFBhAed33340467;     kHBmsCUXQWrFBhAed33340467 = kHBmsCUXQWrFBhAed92083908;     kHBmsCUXQWrFBhAed92083908 = kHBmsCUXQWrFBhAed12327837;     kHBmsCUXQWrFBhAed12327837 = kHBmsCUXQWrFBhAed64927067;     kHBmsCUXQWrFBhAed64927067 = kHBmsCUXQWrFBhAed12709287;     kHBmsCUXQWrFBhAed12709287 = kHBmsCUXQWrFBhAed31930461;     kHBmsCUXQWrFBhAed31930461 = kHBmsCUXQWrFBhAed61308245;     kHBmsCUXQWrFBhAed61308245 = kHBmsCUXQWrFBhAed54513535;     kHBmsCUXQWrFBhAed54513535 = kHBmsCUXQWrFBhAed56614567;     kHBmsCUXQWrFBhAed56614567 = kHBmsCUXQWrFBhAed29599890;     kHBmsCUXQWrFBhAed29599890 = kHBmsCUXQWrFBhAed96957833;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void PmsiRGNZiDbNeyij95948447() {     double bzywjAxCFjbuDGxAk3074940 = -620141061;    double bzywjAxCFjbuDGxAk5105602 = -974213908;    double bzywjAxCFjbuDGxAk5032154 = -512608056;    double bzywjAxCFjbuDGxAk5588084 = -409546777;    double bzywjAxCFjbuDGxAk85243405 = 45917550;    double bzywjAxCFjbuDGxAk83493948 = -619316326;    double bzywjAxCFjbuDGxAk48331151 = -30842754;    double bzywjAxCFjbuDGxAk3503477 = -414251709;    double bzywjAxCFjbuDGxAk989883 = -929487744;    double bzywjAxCFjbuDGxAk33714541 = 99155483;    double bzywjAxCFjbuDGxAk73104716 = -851586930;    double bzywjAxCFjbuDGxAk98333864 = -738731779;    double bzywjAxCFjbuDGxAk21845072 = -906065446;    double bzywjAxCFjbuDGxAk4228748 = 16039605;    double bzywjAxCFjbuDGxAk32797450 = -177127742;    double bzywjAxCFjbuDGxAk96544730 = -940801039;    double bzywjAxCFjbuDGxAk39418725 = -238184319;    double bzywjAxCFjbuDGxAk27704180 = -660417033;    double bzywjAxCFjbuDGxAk58779943 = -118145172;    double bzywjAxCFjbuDGxAk87571518 = -890198853;    double bzywjAxCFjbuDGxAk29056735 = -991217492;    double bzywjAxCFjbuDGxAk27721060 = -443590278;    double bzywjAxCFjbuDGxAk74605463 = -264250905;    double bzywjAxCFjbuDGxAk84560337 = -30178375;    double bzywjAxCFjbuDGxAk17151928 = -848975209;    double bzywjAxCFjbuDGxAk61850423 = 28289330;    double bzywjAxCFjbuDGxAk65470409 = -726135939;    double bzywjAxCFjbuDGxAk20383083 = -828573988;    double bzywjAxCFjbuDGxAk51246296 = -711263167;    double bzywjAxCFjbuDGxAk71684259 = -128934621;    double bzywjAxCFjbuDGxAk60892756 = -371488374;    double bzywjAxCFjbuDGxAk79529377 = -721610600;    double bzywjAxCFjbuDGxAk11783487 = -871153875;    double bzywjAxCFjbuDGxAk10925252 = -638324128;    double bzywjAxCFjbuDGxAk95708512 = -323100829;    double bzywjAxCFjbuDGxAk24784031 = -491335551;    double bzywjAxCFjbuDGxAk61767849 = -248331975;    double bzywjAxCFjbuDGxAk51479074 = -944840151;    double bzywjAxCFjbuDGxAk17756490 = -710671533;    double bzywjAxCFjbuDGxAk25277340 = -840176653;    double bzywjAxCFjbuDGxAk18165913 = -177918769;    double bzywjAxCFjbuDGxAk62088247 = -565202072;    double bzywjAxCFjbuDGxAk65859278 = -674867825;    double bzywjAxCFjbuDGxAk67856982 = -514786560;    double bzywjAxCFjbuDGxAk72628025 = -924512111;    double bzywjAxCFjbuDGxAk21083009 = -224145404;    double bzywjAxCFjbuDGxAk13264011 = -639678194;    double bzywjAxCFjbuDGxAk52828594 = -222567509;    double bzywjAxCFjbuDGxAk61690985 = -784873675;    double bzywjAxCFjbuDGxAk29860295 = -979407158;    double bzywjAxCFjbuDGxAk22547258 = -769419400;    double bzywjAxCFjbuDGxAk98052894 = -131441234;    double bzywjAxCFjbuDGxAk86752448 = -379764624;    double bzywjAxCFjbuDGxAk75815810 = -64413974;    double bzywjAxCFjbuDGxAk66891713 = -139202956;    double bzywjAxCFjbuDGxAk75353879 = -76550784;    double bzywjAxCFjbuDGxAk30500139 = -609963004;    double bzywjAxCFjbuDGxAk20471817 = -382429681;    double bzywjAxCFjbuDGxAk88436156 = -560571569;    double bzywjAxCFjbuDGxAk23392982 = -982371781;    double bzywjAxCFjbuDGxAk18023540 = -893180388;    double bzywjAxCFjbuDGxAk27948069 = -202268767;    double bzywjAxCFjbuDGxAk52257180 = -702988543;    double bzywjAxCFjbuDGxAk29305623 = -700553123;    double bzywjAxCFjbuDGxAk72821784 = -529356143;    double bzywjAxCFjbuDGxAk93575338 = -29976331;    double bzywjAxCFjbuDGxAk86550377 = -867577904;    double bzywjAxCFjbuDGxAk10919820 = -167741319;    double bzywjAxCFjbuDGxAk8520235 = -660859566;    double bzywjAxCFjbuDGxAk8013420 = -685792192;    double bzywjAxCFjbuDGxAk34776881 = -592469064;    double bzywjAxCFjbuDGxAk87939650 = -293344168;    double bzywjAxCFjbuDGxAk9947691 = -949745501;    double bzywjAxCFjbuDGxAk33502604 = -277968519;    double bzywjAxCFjbuDGxAk69405605 = -612280084;    double bzywjAxCFjbuDGxAk66968487 = -326015421;    double bzywjAxCFjbuDGxAk61861782 = -768722454;    double bzywjAxCFjbuDGxAk6748482 = -749464345;    double bzywjAxCFjbuDGxAk11932312 = -105666264;    double bzywjAxCFjbuDGxAk96068919 = -524829805;    double bzywjAxCFjbuDGxAk48586413 = -332032477;    double bzywjAxCFjbuDGxAk12641816 = -403568431;    double bzywjAxCFjbuDGxAk58692097 = 56299686;    double bzywjAxCFjbuDGxAk21386002 = -731856009;    double bzywjAxCFjbuDGxAk49137002 = -359515221;    double bzywjAxCFjbuDGxAk62839861 = -140047141;    double bzywjAxCFjbuDGxAk92776929 = -241845976;    double bzywjAxCFjbuDGxAk35967676 = -706739901;    double bzywjAxCFjbuDGxAk44033539 = -399121172;    double bzywjAxCFjbuDGxAk20354633 = -146550046;    double bzywjAxCFjbuDGxAk94283892 = -881372547;    double bzywjAxCFjbuDGxAk41296033 = -865902294;    double bzywjAxCFjbuDGxAk63042918 = -284268583;    double bzywjAxCFjbuDGxAk94363507 = -728299752;    double bzywjAxCFjbuDGxAk7253801 = -946996266;    double bzywjAxCFjbuDGxAk90217843 = -975650003;    double bzywjAxCFjbuDGxAk9831067 = -862213530;    double bzywjAxCFjbuDGxAk36553655 = -974314702;    double bzywjAxCFjbuDGxAk95035197 = -985430417;    double bzywjAxCFjbuDGxAk79052686 = -620141061;     bzywjAxCFjbuDGxAk3074940 = bzywjAxCFjbuDGxAk5105602;     bzywjAxCFjbuDGxAk5105602 = bzywjAxCFjbuDGxAk5032154;     bzywjAxCFjbuDGxAk5032154 = bzywjAxCFjbuDGxAk5588084;     bzywjAxCFjbuDGxAk5588084 = bzywjAxCFjbuDGxAk85243405;     bzywjAxCFjbuDGxAk85243405 = bzywjAxCFjbuDGxAk83493948;     bzywjAxCFjbuDGxAk83493948 = bzywjAxCFjbuDGxAk48331151;     bzywjAxCFjbuDGxAk48331151 = bzywjAxCFjbuDGxAk3503477;     bzywjAxCFjbuDGxAk3503477 = bzywjAxCFjbuDGxAk989883;     bzywjAxCFjbuDGxAk989883 = bzywjAxCFjbuDGxAk33714541;     bzywjAxCFjbuDGxAk33714541 = bzywjAxCFjbuDGxAk73104716;     bzywjAxCFjbuDGxAk73104716 = bzywjAxCFjbuDGxAk98333864;     bzywjAxCFjbuDGxAk98333864 = bzywjAxCFjbuDGxAk21845072;     bzywjAxCFjbuDGxAk21845072 = bzywjAxCFjbuDGxAk4228748;     bzywjAxCFjbuDGxAk4228748 = bzywjAxCFjbuDGxAk32797450;     bzywjAxCFjbuDGxAk32797450 = bzywjAxCFjbuDGxAk96544730;     bzywjAxCFjbuDGxAk96544730 = bzywjAxCFjbuDGxAk39418725;     bzywjAxCFjbuDGxAk39418725 = bzywjAxCFjbuDGxAk27704180;     bzywjAxCFjbuDGxAk27704180 = bzywjAxCFjbuDGxAk58779943;     bzywjAxCFjbuDGxAk58779943 = bzywjAxCFjbuDGxAk87571518;     bzywjAxCFjbuDGxAk87571518 = bzywjAxCFjbuDGxAk29056735;     bzywjAxCFjbuDGxAk29056735 = bzywjAxCFjbuDGxAk27721060;     bzywjAxCFjbuDGxAk27721060 = bzywjAxCFjbuDGxAk74605463;     bzywjAxCFjbuDGxAk74605463 = bzywjAxCFjbuDGxAk84560337;     bzywjAxCFjbuDGxAk84560337 = bzywjAxCFjbuDGxAk17151928;     bzywjAxCFjbuDGxAk17151928 = bzywjAxCFjbuDGxAk61850423;     bzywjAxCFjbuDGxAk61850423 = bzywjAxCFjbuDGxAk65470409;     bzywjAxCFjbuDGxAk65470409 = bzywjAxCFjbuDGxAk20383083;     bzywjAxCFjbuDGxAk20383083 = bzywjAxCFjbuDGxAk51246296;     bzywjAxCFjbuDGxAk51246296 = bzywjAxCFjbuDGxAk71684259;     bzywjAxCFjbuDGxAk71684259 = bzywjAxCFjbuDGxAk60892756;     bzywjAxCFjbuDGxAk60892756 = bzywjAxCFjbuDGxAk79529377;     bzywjAxCFjbuDGxAk79529377 = bzywjAxCFjbuDGxAk11783487;     bzywjAxCFjbuDGxAk11783487 = bzywjAxCFjbuDGxAk10925252;     bzywjAxCFjbuDGxAk10925252 = bzywjAxCFjbuDGxAk95708512;     bzywjAxCFjbuDGxAk95708512 = bzywjAxCFjbuDGxAk24784031;     bzywjAxCFjbuDGxAk24784031 = bzywjAxCFjbuDGxAk61767849;     bzywjAxCFjbuDGxAk61767849 = bzywjAxCFjbuDGxAk51479074;     bzywjAxCFjbuDGxAk51479074 = bzywjAxCFjbuDGxAk17756490;     bzywjAxCFjbuDGxAk17756490 = bzywjAxCFjbuDGxAk25277340;     bzywjAxCFjbuDGxAk25277340 = bzywjAxCFjbuDGxAk18165913;     bzywjAxCFjbuDGxAk18165913 = bzywjAxCFjbuDGxAk62088247;     bzywjAxCFjbuDGxAk62088247 = bzywjAxCFjbuDGxAk65859278;     bzywjAxCFjbuDGxAk65859278 = bzywjAxCFjbuDGxAk67856982;     bzywjAxCFjbuDGxAk67856982 = bzywjAxCFjbuDGxAk72628025;     bzywjAxCFjbuDGxAk72628025 = bzywjAxCFjbuDGxAk21083009;     bzywjAxCFjbuDGxAk21083009 = bzywjAxCFjbuDGxAk13264011;     bzywjAxCFjbuDGxAk13264011 = bzywjAxCFjbuDGxAk52828594;     bzywjAxCFjbuDGxAk52828594 = bzywjAxCFjbuDGxAk61690985;     bzywjAxCFjbuDGxAk61690985 = bzywjAxCFjbuDGxAk29860295;     bzywjAxCFjbuDGxAk29860295 = bzywjAxCFjbuDGxAk22547258;     bzywjAxCFjbuDGxAk22547258 = bzywjAxCFjbuDGxAk98052894;     bzywjAxCFjbuDGxAk98052894 = bzywjAxCFjbuDGxAk86752448;     bzywjAxCFjbuDGxAk86752448 = bzywjAxCFjbuDGxAk75815810;     bzywjAxCFjbuDGxAk75815810 = bzywjAxCFjbuDGxAk66891713;     bzywjAxCFjbuDGxAk66891713 = bzywjAxCFjbuDGxAk75353879;     bzywjAxCFjbuDGxAk75353879 = bzywjAxCFjbuDGxAk30500139;     bzywjAxCFjbuDGxAk30500139 = bzywjAxCFjbuDGxAk20471817;     bzywjAxCFjbuDGxAk20471817 = bzywjAxCFjbuDGxAk88436156;     bzywjAxCFjbuDGxAk88436156 = bzywjAxCFjbuDGxAk23392982;     bzywjAxCFjbuDGxAk23392982 = bzywjAxCFjbuDGxAk18023540;     bzywjAxCFjbuDGxAk18023540 = bzywjAxCFjbuDGxAk27948069;     bzywjAxCFjbuDGxAk27948069 = bzywjAxCFjbuDGxAk52257180;     bzywjAxCFjbuDGxAk52257180 = bzywjAxCFjbuDGxAk29305623;     bzywjAxCFjbuDGxAk29305623 = bzywjAxCFjbuDGxAk72821784;     bzywjAxCFjbuDGxAk72821784 = bzywjAxCFjbuDGxAk93575338;     bzywjAxCFjbuDGxAk93575338 = bzywjAxCFjbuDGxAk86550377;     bzywjAxCFjbuDGxAk86550377 = bzywjAxCFjbuDGxAk10919820;     bzywjAxCFjbuDGxAk10919820 = bzywjAxCFjbuDGxAk8520235;     bzywjAxCFjbuDGxAk8520235 = bzywjAxCFjbuDGxAk8013420;     bzywjAxCFjbuDGxAk8013420 = bzywjAxCFjbuDGxAk34776881;     bzywjAxCFjbuDGxAk34776881 = bzywjAxCFjbuDGxAk87939650;     bzywjAxCFjbuDGxAk87939650 = bzywjAxCFjbuDGxAk9947691;     bzywjAxCFjbuDGxAk9947691 = bzywjAxCFjbuDGxAk33502604;     bzywjAxCFjbuDGxAk33502604 = bzywjAxCFjbuDGxAk69405605;     bzywjAxCFjbuDGxAk69405605 = bzywjAxCFjbuDGxAk66968487;     bzywjAxCFjbuDGxAk66968487 = bzywjAxCFjbuDGxAk61861782;     bzywjAxCFjbuDGxAk61861782 = bzywjAxCFjbuDGxAk6748482;     bzywjAxCFjbuDGxAk6748482 = bzywjAxCFjbuDGxAk11932312;     bzywjAxCFjbuDGxAk11932312 = bzywjAxCFjbuDGxAk96068919;     bzywjAxCFjbuDGxAk96068919 = bzywjAxCFjbuDGxAk48586413;     bzywjAxCFjbuDGxAk48586413 = bzywjAxCFjbuDGxAk12641816;     bzywjAxCFjbuDGxAk12641816 = bzywjAxCFjbuDGxAk58692097;     bzywjAxCFjbuDGxAk58692097 = bzywjAxCFjbuDGxAk21386002;     bzywjAxCFjbuDGxAk21386002 = bzywjAxCFjbuDGxAk49137002;     bzywjAxCFjbuDGxAk49137002 = bzywjAxCFjbuDGxAk62839861;     bzywjAxCFjbuDGxAk62839861 = bzywjAxCFjbuDGxAk92776929;     bzywjAxCFjbuDGxAk92776929 = bzywjAxCFjbuDGxAk35967676;     bzywjAxCFjbuDGxAk35967676 = bzywjAxCFjbuDGxAk44033539;     bzywjAxCFjbuDGxAk44033539 = bzywjAxCFjbuDGxAk20354633;     bzywjAxCFjbuDGxAk20354633 = bzywjAxCFjbuDGxAk94283892;     bzywjAxCFjbuDGxAk94283892 = bzywjAxCFjbuDGxAk41296033;     bzywjAxCFjbuDGxAk41296033 = bzywjAxCFjbuDGxAk63042918;     bzywjAxCFjbuDGxAk63042918 = bzywjAxCFjbuDGxAk94363507;     bzywjAxCFjbuDGxAk94363507 = bzywjAxCFjbuDGxAk7253801;     bzywjAxCFjbuDGxAk7253801 = bzywjAxCFjbuDGxAk90217843;     bzywjAxCFjbuDGxAk90217843 = bzywjAxCFjbuDGxAk9831067;     bzywjAxCFjbuDGxAk9831067 = bzywjAxCFjbuDGxAk36553655;     bzywjAxCFjbuDGxAk36553655 = bzywjAxCFjbuDGxAk95035197;     bzywjAxCFjbuDGxAk95035197 = bzywjAxCFjbuDGxAk79052686;     bzywjAxCFjbuDGxAk79052686 = bzywjAxCFjbuDGxAk3074940;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void odYCipXYOBZhspSf63240047() {     double NNszOclWAWSmiFUjX38533908 = 13582039;    double NNszOclWAWSmiFUjX18377681 = -854765515;    double NNszOclWAWSmiFUjX42418630 = -178275131;    double NNszOclWAWSmiFUjX77811438 = -230697848;    double NNszOclWAWSmiFUjX41528674 = -59681296;    double NNszOclWAWSmiFUjX19716821 = -881664221;    double NNszOclWAWSmiFUjX58988557 = -975568489;    double NNszOclWAWSmiFUjX65635038 = -58485850;    double NNszOclWAWSmiFUjX2507488 = -317819150;    double NNszOclWAWSmiFUjX48617253 = -665531515;    double NNszOclWAWSmiFUjX11286451 = -653594659;    double NNszOclWAWSmiFUjX98902012 = -602852550;    double NNszOclWAWSmiFUjX60079717 = -358077951;    double NNszOclWAWSmiFUjX57369611 = 39676196;    double NNszOclWAWSmiFUjX90991966 = -680552757;    double NNszOclWAWSmiFUjX80086031 = -576612646;    double NNszOclWAWSmiFUjX95254660 = -765158872;    double NNszOclWAWSmiFUjX27464372 = -733845101;    double NNszOclWAWSmiFUjX59330273 = -434708594;    double NNszOclWAWSmiFUjX81408736 = -216759919;    double NNszOclWAWSmiFUjX42143598 = -648492181;    double NNszOclWAWSmiFUjX31142585 = -530226572;    double NNszOclWAWSmiFUjX45130843 = 98116911;    double NNszOclWAWSmiFUjX78353282 = -541002979;    double NNszOclWAWSmiFUjX25169879 = -872960407;    double NNszOclWAWSmiFUjX72548595 = -760287477;    double NNszOclWAWSmiFUjX25890983 = -532420080;    double NNszOclWAWSmiFUjX61111507 = -998346163;    double NNszOclWAWSmiFUjX97914510 = -185357969;    double NNszOclWAWSmiFUjX77217670 = -163441792;    double NNszOclWAWSmiFUjX79475988 = -923468861;    double NNszOclWAWSmiFUjX17352396 = -494582014;    double NNszOclWAWSmiFUjX24001498 = 81250148;    double NNszOclWAWSmiFUjX40673678 = -703017942;    double NNszOclWAWSmiFUjX61516966 = -274201080;    double NNszOclWAWSmiFUjX76344874 = -705900857;    double NNszOclWAWSmiFUjX11179666 = -113028016;    double NNszOclWAWSmiFUjX2694478 = -927495732;    double NNszOclWAWSmiFUjX48147051 = -361547809;    double NNszOclWAWSmiFUjX89548871 = -305439199;    double NNszOclWAWSmiFUjX51617520 = 52882317;    double NNszOclWAWSmiFUjX51453374 = -159747363;    double NNszOclWAWSmiFUjX55242656 = -228048611;    double NNszOclWAWSmiFUjX7988320 = -19118970;    double NNszOclWAWSmiFUjX72644484 = -751023273;    double NNszOclWAWSmiFUjX67044337 = -263069036;    double NNszOclWAWSmiFUjX32609174 = -23664554;    double NNszOclWAWSmiFUjX70554905 = -389094102;    double NNszOclWAWSmiFUjX84583651 = -448610555;    double NNszOclWAWSmiFUjX54389884 = -370433974;    double NNszOclWAWSmiFUjX3317982 = -643695958;    double NNszOclWAWSmiFUjX81538367 = -345093580;    double NNszOclWAWSmiFUjX55373597 = -135810178;    double NNszOclWAWSmiFUjX91807657 = -255922474;    double NNszOclWAWSmiFUjX21096450 = -485693102;    double NNszOclWAWSmiFUjX7391324 = -456191389;    double NNszOclWAWSmiFUjX73246837 = -852882427;    double NNszOclWAWSmiFUjX64065348 = -637272152;    double NNszOclWAWSmiFUjX52641560 = -357737442;    double NNszOclWAWSmiFUjX68980079 = -299393820;    double NNszOclWAWSmiFUjX93825837 = -249244142;    double NNszOclWAWSmiFUjX97877049 = -977222327;    double NNszOclWAWSmiFUjX67720527 = -873127881;    double NNszOclWAWSmiFUjX25289817 = -54377359;    double NNszOclWAWSmiFUjX69141264 = -742062654;    double NNszOclWAWSmiFUjX93934055 = -59012645;    double NNszOclWAWSmiFUjX74900515 = -584102699;    double NNszOclWAWSmiFUjX19406039 = -655060010;    double NNszOclWAWSmiFUjX95852644 = -686122725;    double NNszOclWAWSmiFUjX14647092 = -974651901;    double NNszOclWAWSmiFUjX68906365 = -363584631;    double NNszOclWAWSmiFUjX92560183 = -837663140;    double NNszOclWAWSmiFUjX79317320 = -272297293;    double NNszOclWAWSmiFUjX69781401 = -29269395;    double NNszOclWAWSmiFUjX29791217 = -169642236;    double NNszOclWAWSmiFUjX90690223 = -388744819;    double NNszOclWAWSmiFUjX75899928 = -202177962;    double NNszOclWAWSmiFUjX37142523 = -882764120;    double NNszOclWAWSmiFUjX5708798 = -789979707;    double NNszOclWAWSmiFUjX58125542 = -509891371;    double NNszOclWAWSmiFUjX39939421 = -636622923;    double NNszOclWAWSmiFUjX55336078 = -43325978;    double NNszOclWAWSmiFUjX76527856 = -449735608;    double NNszOclWAWSmiFUjX43524627 = -814923996;    double NNszOclWAWSmiFUjX73899689 = -519745834;    double NNszOclWAWSmiFUjX97937620 = -478375282;    double NNszOclWAWSmiFUjX61978799 = -258771836;    double NNszOclWAWSmiFUjX32193840 = -662827379;    double NNszOclWAWSmiFUjX19577229 = -117324840;    double NNszOclWAWSmiFUjX54125643 = -818009691;    double NNszOclWAWSmiFUjX3098038 = -853018431;    double NNszOclWAWSmiFUjX47114318 = -475755864;    double NNszOclWAWSmiFUjX50052918 = -469758291;    double NNszOclWAWSmiFUjX79166971 = 37846010;    double NNszOclWAWSmiFUjX95723034 = 43804942;    double NNszOclWAWSmiFUjX53740470 = 30104643;    double NNszOclWAWSmiFUjX83732847 = -286619483;    double NNszOclWAWSmiFUjX29952839 = -73671252;    double NNszOclWAWSmiFUjX38847055 = -277056317;    double NNszOclWAWSmiFUjX78710429 = 13582039;     NNszOclWAWSmiFUjX38533908 = NNszOclWAWSmiFUjX18377681;     NNszOclWAWSmiFUjX18377681 = NNszOclWAWSmiFUjX42418630;     NNszOclWAWSmiFUjX42418630 = NNszOclWAWSmiFUjX77811438;     NNszOclWAWSmiFUjX77811438 = NNszOclWAWSmiFUjX41528674;     NNszOclWAWSmiFUjX41528674 = NNszOclWAWSmiFUjX19716821;     NNszOclWAWSmiFUjX19716821 = NNszOclWAWSmiFUjX58988557;     NNszOclWAWSmiFUjX58988557 = NNszOclWAWSmiFUjX65635038;     NNszOclWAWSmiFUjX65635038 = NNszOclWAWSmiFUjX2507488;     NNszOclWAWSmiFUjX2507488 = NNszOclWAWSmiFUjX48617253;     NNszOclWAWSmiFUjX48617253 = NNszOclWAWSmiFUjX11286451;     NNszOclWAWSmiFUjX11286451 = NNszOclWAWSmiFUjX98902012;     NNszOclWAWSmiFUjX98902012 = NNszOclWAWSmiFUjX60079717;     NNszOclWAWSmiFUjX60079717 = NNszOclWAWSmiFUjX57369611;     NNszOclWAWSmiFUjX57369611 = NNszOclWAWSmiFUjX90991966;     NNszOclWAWSmiFUjX90991966 = NNszOclWAWSmiFUjX80086031;     NNszOclWAWSmiFUjX80086031 = NNszOclWAWSmiFUjX95254660;     NNszOclWAWSmiFUjX95254660 = NNszOclWAWSmiFUjX27464372;     NNszOclWAWSmiFUjX27464372 = NNszOclWAWSmiFUjX59330273;     NNszOclWAWSmiFUjX59330273 = NNszOclWAWSmiFUjX81408736;     NNszOclWAWSmiFUjX81408736 = NNszOclWAWSmiFUjX42143598;     NNszOclWAWSmiFUjX42143598 = NNszOclWAWSmiFUjX31142585;     NNszOclWAWSmiFUjX31142585 = NNszOclWAWSmiFUjX45130843;     NNszOclWAWSmiFUjX45130843 = NNszOclWAWSmiFUjX78353282;     NNszOclWAWSmiFUjX78353282 = NNszOclWAWSmiFUjX25169879;     NNszOclWAWSmiFUjX25169879 = NNszOclWAWSmiFUjX72548595;     NNszOclWAWSmiFUjX72548595 = NNszOclWAWSmiFUjX25890983;     NNszOclWAWSmiFUjX25890983 = NNszOclWAWSmiFUjX61111507;     NNszOclWAWSmiFUjX61111507 = NNszOclWAWSmiFUjX97914510;     NNszOclWAWSmiFUjX97914510 = NNszOclWAWSmiFUjX77217670;     NNszOclWAWSmiFUjX77217670 = NNszOclWAWSmiFUjX79475988;     NNszOclWAWSmiFUjX79475988 = NNszOclWAWSmiFUjX17352396;     NNszOclWAWSmiFUjX17352396 = NNszOclWAWSmiFUjX24001498;     NNszOclWAWSmiFUjX24001498 = NNszOclWAWSmiFUjX40673678;     NNszOclWAWSmiFUjX40673678 = NNszOclWAWSmiFUjX61516966;     NNszOclWAWSmiFUjX61516966 = NNszOclWAWSmiFUjX76344874;     NNszOclWAWSmiFUjX76344874 = NNszOclWAWSmiFUjX11179666;     NNszOclWAWSmiFUjX11179666 = NNszOclWAWSmiFUjX2694478;     NNszOclWAWSmiFUjX2694478 = NNszOclWAWSmiFUjX48147051;     NNszOclWAWSmiFUjX48147051 = NNszOclWAWSmiFUjX89548871;     NNszOclWAWSmiFUjX89548871 = NNszOclWAWSmiFUjX51617520;     NNszOclWAWSmiFUjX51617520 = NNszOclWAWSmiFUjX51453374;     NNszOclWAWSmiFUjX51453374 = NNszOclWAWSmiFUjX55242656;     NNszOclWAWSmiFUjX55242656 = NNszOclWAWSmiFUjX7988320;     NNszOclWAWSmiFUjX7988320 = NNszOclWAWSmiFUjX72644484;     NNszOclWAWSmiFUjX72644484 = NNszOclWAWSmiFUjX67044337;     NNszOclWAWSmiFUjX67044337 = NNszOclWAWSmiFUjX32609174;     NNszOclWAWSmiFUjX32609174 = NNszOclWAWSmiFUjX70554905;     NNszOclWAWSmiFUjX70554905 = NNszOclWAWSmiFUjX84583651;     NNszOclWAWSmiFUjX84583651 = NNszOclWAWSmiFUjX54389884;     NNszOclWAWSmiFUjX54389884 = NNszOclWAWSmiFUjX3317982;     NNszOclWAWSmiFUjX3317982 = NNszOclWAWSmiFUjX81538367;     NNszOclWAWSmiFUjX81538367 = NNszOclWAWSmiFUjX55373597;     NNszOclWAWSmiFUjX55373597 = NNszOclWAWSmiFUjX91807657;     NNszOclWAWSmiFUjX91807657 = NNszOclWAWSmiFUjX21096450;     NNszOclWAWSmiFUjX21096450 = NNszOclWAWSmiFUjX7391324;     NNszOclWAWSmiFUjX7391324 = NNszOclWAWSmiFUjX73246837;     NNszOclWAWSmiFUjX73246837 = NNszOclWAWSmiFUjX64065348;     NNszOclWAWSmiFUjX64065348 = NNszOclWAWSmiFUjX52641560;     NNszOclWAWSmiFUjX52641560 = NNszOclWAWSmiFUjX68980079;     NNszOclWAWSmiFUjX68980079 = NNszOclWAWSmiFUjX93825837;     NNszOclWAWSmiFUjX93825837 = NNszOclWAWSmiFUjX97877049;     NNszOclWAWSmiFUjX97877049 = NNszOclWAWSmiFUjX67720527;     NNszOclWAWSmiFUjX67720527 = NNszOclWAWSmiFUjX25289817;     NNszOclWAWSmiFUjX25289817 = NNszOclWAWSmiFUjX69141264;     NNszOclWAWSmiFUjX69141264 = NNszOclWAWSmiFUjX93934055;     NNszOclWAWSmiFUjX93934055 = NNszOclWAWSmiFUjX74900515;     NNszOclWAWSmiFUjX74900515 = NNszOclWAWSmiFUjX19406039;     NNszOclWAWSmiFUjX19406039 = NNszOclWAWSmiFUjX95852644;     NNszOclWAWSmiFUjX95852644 = NNszOclWAWSmiFUjX14647092;     NNszOclWAWSmiFUjX14647092 = NNszOclWAWSmiFUjX68906365;     NNszOclWAWSmiFUjX68906365 = NNszOclWAWSmiFUjX92560183;     NNszOclWAWSmiFUjX92560183 = NNszOclWAWSmiFUjX79317320;     NNszOclWAWSmiFUjX79317320 = NNszOclWAWSmiFUjX69781401;     NNszOclWAWSmiFUjX69781401 = NNszOclWAWSmiFUjX29791217;     NNszOclWAWSmiFUjX29791217 = NNszOclWAWSmiFUjX90690223;     NNszOclWAWSmiFUjX90690223 = NNszOclWAWSmiFUjX75899928;     NNszOclWAWSmiFUjX75899928 = NNszOclWAWSmiFUjX37142523;     NNszOclWAWSmiFUjX37142523 = NNszOclWAWSmiFUjX5708798;     NNszOclWAWSmiFUjX5708798 = NNszOclWAWSmiFUjX58125542;     NNszOclWAWSmiFUjX58125542 = NNszOclWAWSmiFUjX39939421;     NNszOclWAWSmiFUjX39939421 = NNszOclWAWSmiFUjX55336078;     NNszOclWAWSmiFUjX55336078 = NNszOclWAWSmiFUjX76527856;     NNszOclWAWSmiFUjX76527856 = NNszOclWAWSmiFUjX43524627;     NNszOclWAWSmiFUjX43524627 = NNszOclWAWSmiFUjX73899689;     NNszOclWAWSmiFUjX73899689 = NNszOclWAWSmiFUjX97937620;     NNszOclWAWSmiFUjX97937620 = NNszOclWAWSmiFUjX61978799;     NNszOclWAWSmiFUjX61978799 = NNszOclWAWSmiFUjX32193840;     NNszOclWAWSmiFUjX32193840 = NNszOclWAWSmiFUjX19577229;     NNszOclWAWSmiFUjX19577229 = NNszOclWAWSmiFUjX54125643;     NNszOclWAWSmiFUjX54125643 = NNszOclWAWSmiFUjX3098038;     NNszOclWAWSmiFUjX3098038 = NNszOclWAWSmiFUjX47114318;     NNszOclWAWSmiFUjX47114318 = NNszOclWAWSmiFUjX50052918;     NNszOclWAWSmiFUjX50052918 = NNszOclWAWSmiFUjX79166971;     NNszOclWAWSmiFUjX79166971 = NNszOclWAWSmiFUjX95723034;     NNszOclWAWSmiFUjX95723034 = NNszOclWAWSmiFUjX53740470;     NNszOclWAWSmiFUjX53740470 = NNszOclWAWSmiFUjX83732847;     NNszOclWAWSmiFUjX83732847 = NNszOclWAWSmiFUjX29952839;     NNszOclWAWSmiFUjX29952839 = NNszOclWAWSmiFUjX38847055;     NNszOclWAWSmiFUjX38847055 = NNszOclWAWSmiFUjX78710429;     NNszOclWAWSmiFUjX78710429 = NNszOclWAWSmiFUjX38533908;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zFRKROmkmZWJwOKY78289115() {     double zbkSrNslQjMbPtLkz44651015 = -98319850;    double zbkSrNslQjMbPtLkz61750960 = -564365296;    double zbkSrNslQjMbPtLkz58460813 = -329108052;    double zbkSrNslQjMbPtLkz4886220 = -421446479;    double zbkSrNslQjMbPtLkz10424163 = -580331167;    double zbkSrNslQjMbPtLkz11134023 = -46160456;    double zbkSrNslQjMbPtLkz19477524 = -14533466;    double zbkSrNslQjMbPtLkz15394929 = -345359763;    double zbkSrNslQjMbPtLkz67149983 = -85851816;    double zbkSrNslQjMbPtLkz89846911 = -994506992;    double zbkSrNslQjMbPtLkz71396741 = -531032246;    double zbkSrNslQjMbPtLkz30725055 = 52741248;    double zbkSrNslQjMbPtLkz46288836 = -797969040;    double zbkSrNslQjMbPtLkz40513287 = -492562372;    double zbkSrNslQjMbPtLkz86734393 = -271014924;    double zbkSrNslQjMbPtLkz21591425 = -549612189;    double zbkSrNslQjMbPtLkz89090663 = -281703984;    double zbkSrNslQjMbPtLkz24517525 = -399367744;    double zbkSrNslQjMbPtLkz21160115 = -840477690;    double zbkSrNslQjMbPtLkz61958901 = -573662838;    double zbkSrNslQjMbPtLkz30696791 = -503979636;    double zbkSrNslQjMbPtLkz17663609 = -438156289;    double zbkSrNslQjMbPtLkz80721635 = -748400576;    double zbkSrNslQjMbPtLkz62345656 = -763307967;    double zbkSrNslQjMbPtLkz68869529 = -146156440;    double zbkSrNslQjMbPtLkz97270270 = -638942800;    double zbkSrNslQjMbPtLkz24350762 = -433082283;    double zbkSrNslQjMbPtLkz74218632 = 35724884;    double zbkSrNslQjMbPtLkz64046766 = -407740674;    double zbkSrNslQjMbPtLkz45833307 = 19087341;    double zbkSrNslQjMbPtLkz71718537 = -836789799;    double zbkSrNslQjMbPtLkz14963018 = -204028830;    double zbkSrNslQjMbPtLkz33712446 = -911363222;    double zbkSrNslQjMbPtLkz35504243 = -454559707;    double zbkSrNslQjMbPtLkz22991618 = -912934185;    double zbkSrNslQjMbPtLkz45070788 = -919926964;    double zbkSrNslQjMbPtLkz92441600 = -66663885;    double zbkSrNslQjMbPtLkz36784690 = -699001742;    double zbkSrNslQjMbPtLkz21757305 = -355471306;    double zbkSrNslQjMbPtLkz77050809 = -778878422;    double zbkSrNslQjMbPtLkz57286744 = -12391578;    double zbkSrNslQjMbPtLkz83603831 = -429889641;    double zbkSrNslQjMbPtLkz33309634 = -308853195;    double zbkSrNslQjMbPtLkz95396060 = -38461908;    double zbkSrNslQjMbPtLkz84596949 = -412921427;    double zbkSrNslQjMbPtLkz78983532 = -248285153;    double zbkSrNslQjMbPtLkz50387748 = -142440092;    double zbkSrNslQjMbPtLkz78511846 = -398989635;    double zbkSrNslQjMbPtLkz94259379 = -534347543;    double zbkSrNslQjMbPtLkz67301536 = -457444782;    double zbkSrNslQjMbPtLkz39018660 = -485989629;    double zbkSrNslQjMbPtLkz92348086 = -145819900;    double zbkSrNslQjMbPtLkz3622054 = 72400234;    double zbkSrNslQjMbPtLkz87776537 = -576392619;    double zbkSrNslQjMbPtLkz47592324 = -86224885;    double zbkSrNslQjMbPtLkz26987407 = -660163562;    double zbkSrNslQjMbPtLkz81029324 = -815964721;    double zbkSrNslQjMbPtLkz96115157 = -565800086;    double zbkSrNslQjMbPtLkz36016691 = -175290039;    double zbkSrNslQjMbPtLkz13153893 = -941388368;    double zbkSrNslQjMbPtLkz86783261 = -613078174;    double zbkSrNslQjMbPtLkz45258891 = 49741650;    double zbkSrNslQjMbPtLkz51348162 = -937619089;    double zbkSrNslQjMbPtLkz21316676 = -4939157;    double zbkSrNslQjMbPtLkz18128375 = -57717193;    double zbkSrNslQjMbPtLkz56433724 = -227003416;    double zbkSrNslQjMbPtLkz97012609 = -35895530;    double zbkSrNslQjMbPtLkz10784594 = -243409334;    double zbkSrNslQjMbPtLkz17521670 = -579628187;    double zbkSrNslQjMbPtLkz41663605 = -351087960;    double zbkSrNslQjMbPtLkz29149825 = -382948305;    double zbkSrNslQjMbPtLkz52305974 = -582702243;    double zbkSrNslQjMbPtLkz2760220 = 56103561;    double zbkSrNslQjMbPtLkz44109305 = 38400731;    double zbkSrNslQjMbPtLkz4672158 = -461271261;    double zbkSrNslQjMbPtLkz47092960 = 25910004;    double zbkSrNslQjMbPtLkz84353974 = -29303095;    double zbkSrNslQjMbPtLkz85325574 = -609938668;    double zbkSrNslQjMbPtLkz77748706 = -250386540;    double zbkSrNslQjMbPtLkz89885996 = -897871288;    double zbkSrNslQjMbPtLkz46882522 = -396502709;    double zbkSrNslQjMbPtLkz45838915 = 65907352;    double zbkSrNslQjMbPtLkz79959252 = -429927574;    double zbkSrNslQjMbPtLkz96745229 = -950295892;    double zbkSrNslQjMbPtLkz6814648 = -494923031;    double zbkSrNslQjMbPtLkz79370450 = -590969900;    double zbkSrNslQjMbPtLkz11340964 = -176429064;    double zbkSrNslQjMbPtLkz45935908 = -234970604;    double zbkSrNslQjMbPtLkz87911918 = -268334823;    double zbkSrNslQjMbPtLkz96004211 = -152770624;    double zbkSrNslQjMbPtLkz64041463 = -3962244;    double zbkSrNslQjMbPtLkz96326442 = -500863800;    double zbkSrNslQjMbPtLkz768000 = -423711703;    double zbkSrNslQjMbPtLkz8603413 = -414082939;    double zbkSrNslQjMbPtLkz90267548 = -65800248;    double zbkSrNslQjMbPtLkz12027854 = 37866772;    double zbkSrNslQjMbPtLkz32255669 = -492270552;    double zbkSrNslQjMbPtLkz11992959 = -203914038;    double zbkSrNslQjMbPtLkz77267686 = -980744715;    double zbkSrNslQjMbPtLkz28163226 = -98319850;     zbkSrNslQjMbPtLkz44651015 = zbkSrNslQjMbPtLkz61750960;     zbkSrNslQjMbPtLkz61750960 = zbkSrNslQjMbPtLkz58460813;     zbkSrNslQjMbPtLkz58460813 = zbkSrNslQjMbPtLkz4886220;     zbkSrNslQjMbPtLkz4886220 = zbkSrNslQjMbPtLkz10424163;     zbkSrNslQjMbPtLkz10424163 = zbkSrNslQjMbPtLkz11134023;     zbkSrNslQjMbPtLkz11134023 = zbkSrNslQjMbPtLkz19477524;     zbkSrNslQjMbPtLkz19477524 = zbkSrNslQjMbPtLkz15394929;     zbkSrNslQjMbPtLkz15394929 = zbkSrNslQjMbPtLkz67149983;     zbkSrNslQjMbPtLkz67149983 = zbkSrNslQjMbPtLkz89846911;     zbkSrNslQjMbPtLkz89846911 = zbkSrNslQjMbPtLkz71396741;     zbkSrNslQjMbPtLkz71396741 = zbkSrNslQjMbPtLkz30725055;     zbkSrNslQjMbPtLkz30725055 = zbkSrNslQjMbPtLkz46288836;     zbkSrNslQjMbPtLkz46288836 = zbkSrNslQjMbPtLkz40513287;     zbkSrNslQjMbPtLkz40513287 = zbkSrNslQjMbPtLkz86734393;     zbkSrNslQjMbPtLkz86734393 = zbkSrNslQjMbPtLkz21591425;     zbkSrNslQjMbPtLkz21591425 = zbkSrNslQjMbPtLkz89090663;     zbkSrNslQjMbPtLkz89090663 = zbkSrNslQjMbPtLkz24517525;     zbkSrNslQjMbPtLkz24517525 = zbkSrNslQjMbPtLkz21160115;     zbkSrNslQjMbPtLkz21160115 = zbkSrNslQjMbPtLkz61958901;     zbkSrNslQjMbPtLkz61958901 = zbkSrNslQjMbPtLkz30696791;     zbkSrNslQjMbPtLkz30696791 = zbkSrNslQjMbPtLkz17663609;     zbkSrNslQjMbPtLkz17663609 = zbkSrNslQjMbPtLkz80721635;     zbkSrNslQjMbPtLkz80721635 = zbkSrNslQjMbPtLkz62345656;     zbkSrNslQjMbPtLkz62345656 = zbkSrNslQjMbPtLkz68869529;     zbkSrNslQjMbPtLkz68869529 = zbkSrNslQjMbPtLkz97270270;     zbkSrNslQjMbPtLkz97270270 = zbkSrNslQjMbPtLkz24350762;     zbkSrNslQjMbPtLkz24350762 = zbkSrNslQjMbPtLkz74218632;     zbkSrNslQjMbPtLkz74218632 = zbkSrNslQjMbPtLkz64046766;     zbkSrNslQjMbPtLkz64046766 = zbkSrNslQjMbPtLkz45833307;     zbkSrNslQjMbPtLkz45833307 = zbkSrNslQjMbPtLkz71718537;     zbkSrNslQjMbPtLkz71718537 = zbkSrNslQjMbPtLkz14963018;     zbkSrNslQjMbPtLkz14963018 = zbkSrNslQjMbPtLkz33712446;     zbkSrNslQjMbPtLkz33712446 = zbkSrNslQjMbPtLkz35504243;     zbkSrNslQjMbPtLkz35504243 = zbkSrNslQjMbPtLkz22991618;     zbkSrNslQjMbPtLkz22991618 = zbkSrNslQjMbPtLkz45070788;     zbkSrNslQjMbPtLkz45070788 = zbkSrNslQjMbPtLkz92441600;     zbkSrNslQjMbPtLkz92441600 = zbkSrNslQjMbPtLkz36784690;     zbkSrNslQjMbPtLkz36784690 = zbkSrNslQjMbPtLkz21757305;     zbkSrNslQjMbPtLkz21757305 = zbkSrNslQjMbPtLkz77050809;     zbkSrNslQjMbPtLkz77050809 = zbkSrNslQjMbPtLkz57286744;     zbkSrNslQjMbPtLkz57286744 = zbkSrNslQjMbPtLkz83603831;     zbkSrNslQjMbPtLkz83603831 = zbkSrNslQjMbPtLkz33309634;     zbkSrNslQjMbPtLkz33309634 = zbkSrNslQjMbPtLkz95396060;     zbkSrNslQjMbPtLkz95396060 = zbkSrNslQjMbPtLkz84596949;     zbkSrNslQjMbPtLkz84596949 = zbkSrNslQjMbPtLkz78983532;     zbkSrNslQjMbPtLkz78983532 = zbkSrNslQjMbPtLkz50387748;     zbkSrNslQjMbPtLkz50387748 = zbkSrNslQjMbPtLkz78511846;     zbkSrNslQjMbPtLkz78511846 = zbkSrNslQjMbPtLkz94259379;     zbkSrNslQjMbPtLkz94259379 = zbkSrNslQjMbPtLkz67301536;     zbkSrNslQjMbPtLkz67301536 = zbkSrNslQjMbPtLkz39018660;     zbkSrNslQjMbPtLkz39018660 = zbkSrNslQjMbPtLkz92348086;     zbkSrNslQjMbPtLkz92348086 = zbkSrNslQjMbPtLkz3622054;     zbkSrNslQjMbPtLkz3622054 = zbkSrNslQjMbPtLkz87776537;     zbkSrNslQjMbPtLkz87776537 = zbkSrNslQjMbPtLkz47592324;     zbkSrNslQjMbPtLkz47592324 = zbkSrNslQjMbPtLkz26987407;     zbkSrNslQjMbPtLkz26987407 = zbkSrNslQjMbPtLkz81029324;     zbkSrNslQjMbPtLkz81029324 = zbkSrNslQjMbPtLkz96115157;     zbkSrNslQjMbPtLkz96115157 = zbkSrNslQjMbPtLkz36016691;     zbkSrNslQjMbPtLkz36016691 = zbkSrNslQjMbPtLkz13153893;     zbkSrNslQjMbPtLkz13153893 = zbkSrNslQjMbPtLkz86783261;     zbkSrNslQjMbPtLkz86783261 = zbkSrNslQjMbPtLkz45258891;     zbkSrNslQjMbPtLkz45258891 = zbkSrNslQjMbPtLkz51348162;     zbkSrNslQjMbPtLkz51348162 = zbkSrNslQjMbPtLkz21316676;     zbkSrNslQjMbPtLkz21316676 = zbkSrNslQjMbPtLkz18128375;     zbkSrNslQjMbPtLkz18128375 = zbkSrNslQjMbPtLkz56433724;     zbkSrNslQjMbPtLkz56433724 = zbkSrNslQjMbPtLkz97012609;     zbkSrNslQjMbPtLkz97012609 = zbkSrNslQjMbPtLkz10784594;     zbkSrNslQjMbPtLkz10784594 = zbkSrNslQjMbPtLkz17521670;     zbkSrNslQjMbPtLkz17521670 = zbkSrNslQjMbPtLkz41663605;     zbkSrNslQjMbPtLkz41663605 = zbkSrNslQjMbPtLkz29149825;     zbkSrNslQjMbPtLkz29149825 = zbkSrNslQjMbPtLkz52305974;     zbkSrNslQjMbPtLkz52305974 = zbkSrNslQjMbPtLkz2760220;     zbkSrNslQjMbPtLkz2760220 = zbkSrNslQjMbPtLkz44109305;     zbkSrNslQjMbPtLkz44109305 = zbkSrNslQjMbPtLkz4672158;     zbkSrNslQjMbPtLkz4672158 = zbkSrNslQjMbPtLkz47092960;     zbkSrNslQjMbPtLkz47092960 = zbkSrNslQjMbPtLkz84353974;     zbkSrNslQjMbPtLkz84353974 = zbkSrNslQjMbPtLkz85325574;     zbkSrNslQjMbPtLkz85325574 = zbkSrNslQjMbPtLkz77748706;     zbkSrNslQjMbPtLkz77748706 = zbkSrNslQjMbPtLkz89885996;     zbkSrNslQjMbPtLkz89885996 = zbkSrNslQjMbPtLkz46882522;     zbkSrNslQjMbPtLkz46882522 = zbkSrNslQjMbPtLkz45838915;     zbkSrNslQjMbPtLkz45838915 = zbkSrNslQjMbPtLkz79959252;     zbkSrNslQjMbPtLkz79959252 = zbkSrNslQjMbPtLkz96745229;     zbkSrNslQjMbPtLkz96745229 = zbkSrNslQjMbPtLkz6814648;     zbkSrNslQjMbPtLkz6814648 = zbkSrNslQjMbPtLkz79370450;     zbkSrNslQjMbPtLkz79370450 = zbkSrNslQjMbPtLkz11340964;     zbkSrNslQjMbPtLkz11340964 = zbkSrNslQjMbPtLkz45935908;     zbkSrNslQjMbPtLkz45935908 = zbkSrNslQjMbPtLkz87911918;     zbkSrNslQjMbPtLkz87911918 = zbkSrNslQjMbPtLkz96004211;     zbkSrNslQjMbPtLkz96004211 = zbkSrNslQjMbPtLkz64041463;     zbkSrNslQjMbPtLkz64041463 = zbkSrNslQjMbPtLkz96326442;     zbkSrNslQjMbPtLkz96326442 = zbkSrNslQjMbPtLkz768000;     zbkSrNslQjMbPtLkz768000 = zbkSrNslQjMbPtLkz8603413;     zbkSrNslQjMbPtLkz8603413 = zbkSrNslQjMbPtLkz90267548;     zbkSrNslQjMbPtLkz90267548 = zbkSrNslQjMbPtLkz12027854;     zbkSrNslQjMbPtLkz12027854 = zbkSrNslQjMbPtLkz32255669;     zbkSrNslQjMbPtLkz32255669 = zbkSrNslQjMbPtLkz11992959;     zbkSrNslQjMbPtLkz11992959 = zbkSrNslQjMbPtLkz77267686;     zbkSrNslQjMbPtLkz77267686 = zbkSrNslQjMbPtLkz28163226;     zbkSrNslQjMbPtLkz28163226 = zbkSrNslQjMbPtLkz44651015;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void pCAEPewRbWMtDHPv45580714() {     double umPpBHhMugEEyIokf80109983 = -564596750;    double umPpBHhMugEEyIokf75023038 = -444916902;    double umPpBHhMugEEyIokf95847290 = 5224873;    double umPpBHhMugEEyIokf77109574 = -242597550;    double umPpBHhMugEEyIokf66709432 = -685930012;    double umPpBHhMugEEyIokf47356895 = -308508352;    double umPpBHhMugEEyIokf30134930 = -959259200;    double umPpBHhMugEEyIokf77526489 = 10406097;    double umPpBHhMugEEyIokf68667588 = -574183223;    double umPpBHhMugEEyIokf4749624 = -659193990;    double umPpBHhMugEEyIokf9578476 = -333039975;    double umPpBHhMugEEyIokf31293204 = -911379523;    double umPpBHhMugEEyIokf84523481 = -249981545;    double umPpBHhMugEEyIokf93654150 = -468925781;    double umPpBHhMugEEyIokf44928909 = -774439939;    double umPpBHhMugEEyIokf5132726 = -185423796;    double umPpBHhMugEEyIokf44926600 = -808678537;    double umPpBHhMugEEyIokf24277717 = -472795813;    double umPpBHhMugEEyIokf21710445 = -57041112;    double umPpBHhMugEEyIokf55796120 = 99776096;    double umPpBHhMugEEyIokf43783654 = -161254325;    double umPpBHhMugEEyIokf21085133 = -524792583;    double umPpBHhMugEEyIokf51247015 = -386032760;    double umPpBHhMugEEyIokf56138601 = -174132571;    double umPpBHhMugEEyIokf76887480 = -170141638;    double umPpBHhMugEEyIokf7968442 = -327519607;    double umPpBHhMugEEyIokf84771335 = -239366423;    double umPpBHhMugEEyIokf14947058 = -134047291;    double umPpBHhMugEEyIokf10714981 = -981835476;    double umPpBHhMugEEyIokf51366718 = -15419830;    double umPpBHhMugEEyIokf90301769 = -288770287;    double umPpBHhMugEEyIokf52786036 = 22999756;    double umPpBHhMugEEyIokf45930457 = 41040800;    double umPpBHhMugEEyIokf65252669 = -519253520;    double umPpBHhMugEEyIokf88800071 = -864034436;    double umPpBHhMugEEyIokf96631631 = -34492270;    double umPpBHhMugEEyIokf41853417 = 68640075;    double umPpBHhMugEEyIokf88000093 = -681657323;    double umPpBHhMugEEyIokf52147866 = -6347582;    double umPpBHhMugEEyIokf41322342 = -244140968;    double umPpBHhMugEEyIokf90738351 = -881590492;    double umPpBHhMugEEyIokf72968958 = -24434932;    double umPpBHhMugEEyIokf22693013 = -962033980;    double umPpBHhMugEEyIokf35527399 = -642794318;    double umPpBHhMugEEyIokf84613408 = -239432588;    double umPpBHhMugEEyIokf24944861 = -287208785;    double umPpBHhMugEEyIokf69732912 = -626426452;    double umPpBHhMugEEyIokf96238158 = -565516228;    double umPpBHhMugEEyIokf17152046 = -198084423;    double umPpBHhMugEEyIokf91831126 = -948471598;    double umPpBHhMugEEyIokf19789384 = -360266187;    double umPpBHhMugEEyIokf75833559 = -359472247;    double umPpBHhMugEEyIokf72243202 = -783645320;    double umPpBHhMugEEyIokf3768385 = -767901118;    double umPpBHhMugEEyIokf1797061 = -432715031;    double umPpBHhMugEEyIokf59024850 = 60195832;    double umPpBHhMugEEyIokf23776024 = 41115857;    double umPpBHhMugEEyIokf39708689 = -820642556;    double umPpBHhMugEEyIokf222095 = 27544088;    double umPpBHhMugEEyIokf58740990 = -258410406;    double umPpBHhMugEEyIokf62585559 = 30858071;    double umPpBHhMugEEyIokf15187872 = -725211910;    double umPpBHhMugEEyIokf66811508 = -7758427;    double umPpBHhMugEEyIokf17300870 = -458763394;    double umPpBHhMugEEyIokf14447855 = -270423704;    double umPpBHhMugEEyIokf56792440 = -256039731;    double umPpBHhMugEEyIokf85362746 = -852420324;    double umPpBHhMugEEyIokf19270812 = -730728025;    double umPpBHhMugEEyIokf4854079 = -604891346;    double umPpBHhMugEEyIokf48297278 = -639947669;    double umPpBHhMugEEyIokf63279309 = -154063871;    double umPpBHhMugEEyIokf56926506 = -27021215;    double umPpBHhMugEEyIokf72129850 = -366448231;    double umPpBHhMugEEyIokf80388103 = -812900144;    double umPpBHhMugEEyIokf65057768 = -18633413;    double umPpBHhMugEEyIokf70814696 = -36819394;    double umPpBHhMugEEyIokf98392120 = -562758603;    double umPpBHhMugEEyIokf15719617 = -743238442;    double umPpBHhMugEEyIokf71525193 = -934699983;    double umPpBHhMugEEyIokf51942620 = -882932854;    double umPpBHhMugEEyIokf38235530 = -701093155;    double umPpBHhMugEEyIokf88533177 = -673850196;    double umPpBHhMugEEyIokf97795011 = -935962868;    double umPpBHhMugEEyIokf18883855 = 66636121;    double umPpBHhMugEEyIokf31577335 = -655153644;    double umPpBHhMugEEyIokf14468210 = -929298040;    double umPpBHhMugEEyIokf80542833 = -193354925;    double umPpBHhMugEEyIokf42162073 = -191058082;    double umPpBHhMugEEyIokf63455608 = 13461510;    double umPpBHhMugEEyIokf29775222 = -824230269;    double umPpBHhMugEEyIokf72855608 = 24391872;    double umPpBHhMugEEyIokf2144728 = -110717369;    double umPpBHhMugEEyIokf87777999 = -609201411;    double umPpBHhMugEEyIokf93406876 = -747937176;    double umPpBHhMugEEyIokf78736782 = -174999040;    double umPpBHhMugEEyIokf75550480 = -56378583;    double umPpBHhMugEEyIokf6157450 = 83323495;    double umPpBHhMugEEyIokf5392143 = -403270587;    double umPpBHhMugEEyIokf21079544 = -272370614;    double umPpBHhMugEEyIokf27820968 = -564596750;     umPpBHhMugEEyIokf80109983 = umPpBHhMugEEyIokf75023038;     umPpBHhMugEEyIokf75023038 = umPpBHhMugEEyIokf95847290;     umPpBHhMugEEyIokf95847290 = umPpBHhMugEEyIokf77109574;     umPpBHhMugEEyIokf77109574 = umPpBHhMugEEyIokf66709432;     umPpBHhMugEEyIokf66709432 = umPpBHhMugEEyIokf47356895;     umPpBHhMugEEyIokf47356895 = umPpBHhMugEEyIokf30134930;     umPpBHhMugEEyIokf30134930 = umPpBHhMugEEyIokf77526489;     umPpBHhMugEEyIokf77526489 = umPpBHhMugEEyIokf68667588;     umPpBHhMugEEyIokf68667588 = umPpBHhMugEEyIokf4749624;     umPpBHhMugEEyIokf4749624 = umPpBHhMugEEyIokf9578476;     umPpBHhMugEEyIokf9578476 = umPpBHhMugEEyIokf31293204;     umPpBHhMugEEyIokf31293204 = umPpBHhMugEEyIokf84523481;     umPpBHhMugEEyIokf84523481 = umPpBHhMugEEyIokf93654150;     umPpBHhMugEEyIokf93654150 = umPpBHhMugEEyIokf44928909;     umPpBHhMugEEyIokf44928909 = umPpBHhMugEEyIokf5132726;     umPpBHhMugEEyIokf5132726 = umPpBHhMugEEyIokf44926600;     umPpBHhMugEEyIokf44926600 = umPpBHhMugEEyIokf24277717;     umPpBHhMugEEyIokf24277717 = umPpBHhMugEEyIokf21710445;     umPpBHhMugEEyIokf21710445 = umPpBHhMugEEyIokf55796120;     umPpBHhMugEEyIokf55796120 = umPpBHhMugEEyIokf43783654;     umPpBHhMugEEyIokf43783654 = umPpBHhMugEEyIokf21085133;     umPpBHhMugEEyIokf21085133 = umPpBHhMugEEyIokf51247015;     umPpBHhMugEEyIokf51247015 = umPpBHhMugEEyIokf56138601;     umPpBHhMugEEyIokf56138601 = umPpBHhMugEEyIokf76887480;     umPpBHhMugEEyIokf76887480 = umPpBHhMugEEyIokf7968442;     umPpBHhMugEEyIokf7968442 = umPpBHhMugEEyIokf84771335;     umPpBHhMugEEyIokf84771335 = umPpBHhMugEEyIokf14947058;     umPpBHhMugEEyIokf14947058 = umPpBHhMugEEyIokf10714981;     umPpBHhMugEEyIokf10714981 = umPpBHhMugEEyIokf51366718;     umPpBHhMugEEyIokf51366718 = umPpBHhMugEEyIokf90301769;     umPpBHhMugEEyIokf90301769 = umPpBHhMugEEyIokf52786036;     umPpBHhMugEEyIokf52786036 = umPpBHhMugEEyIokf45930457;     umPpBHhMugEEyIokf45930457 = umPpBHhMugEEyIokf65252669;     umPpBHhMugEEyIokf65252669 = umPpBHhMugEEyIokf88800071;     umPpBHhMugEEyIokf88800071 = umPpBHhMugEEyIokf96631631;     umPpBHhMugEEyIokf96631631 = umPpBHhMugEEyIokf41853417;     umPpBHhMugEEyIokf41853417 = umPpBHhMugEEyIokf88000093;     umPpBHhMugEEyIokf88000093 = umPpBHhMugEEyIokf52147866;     umPpBHhMugEEyIokf52147866 = umPpBHhMugEEyIokf41322342;     umPpBHhMugEEyIokf41322342 = umPpBHhMugEEyIokf90738351;     umPpBHhMugEEyIokf90738351 = umPpBHhMugEEyIokf72968958;     umPpBHhMugEEyIokf72968958 = umPpBHhMugEEyIokf22693013;     umPpBHhMugEEyIokf22693013 = umPpBHhMugEEyIokf35527399;     umPpBHhMugEEyIokf35527399 = umPpBHhMugEEyIokf84613408;     umPpBHhMugEEyIokf84613408 = umPpBHhMugEEyIokf24944861;     umPpBHhMugEEyIokf24944861 = umPpBHhMugEEyIokf69732912;     umPpBHhMugEEyIokf69732912 = umPpBHhMugEEyIokf96238158;     umPpBHhMugEEyIokf96238158 = umPpBHhMugEEyIokf17152046;     umPpBHhMugEEyIokf17152046 = umPpBHhMugEEyIokf91831126;     umPpBHhMugEEyIokf91831126 = umPpBHhMugEEyIokf19789384;     umPpBHhMugEEyIokf19789384 = umPpBHhMugEEyIokf75833559;     umPpBHhMugEEyIokf75833559 = umPpBHhMugEEyIokf72243202;     umPpBHhMugEEyIokf72243202 = umPpBHhMugEEyIokf3768385;     umPpBHhMugEEyIokf3768385 = umPpBHhMugEEyIokf1797061;     umPpBHhMugEEyIokf1797061 = umPpBHhMugEEyIokf59024850;     umPpBHhMugEEyIokf59024850 = umPpBHhMugEEyIokf23776024;     umPpBHhMugEEyIokf23776024 = umPpBHhMugEEyIokf39708689;     umPpBHhMugEEyIokf39708689 = umPpBHhMugEEyIokf222095;     umPpBHhMugEEyIokf222095 = umPpBHhMugEEyIokf58740990;     umPpBHhMugEEyIokf58740990 = umPpBHhMugEEyIokf62585559;     umPpBHhMugEEyIokf62585559 = umPpBHhMugEEyIokf15187872;     umPpBHhMugEEyIokf15187872 = umPpBHhMugEEyIokf66811508;     umPpBHhMugEEyIokf66811508 = umPpBHhMugEEyIokf17300870;     umPpBHhMugEEyIokf17300870 = umPpBHhMugEEyIokf14447855;     umPpBHhMugEEyIokf14447855 = umPpBHhMugEEyIokf56792440;     umPpBHhMugEEyIokf56792440 = umPpBHhMugEEyIokf85362746;     umPpBHhMugEEyIokf85362746 = umPpBHhMugEEyIokf19270812;     umPpBHhMugEEyIokf19270812 = umPpBHhMugEEyIokf4854079;     umPpBHhMugEEyIokf4854079 = umPpBHhMugEEyIokf48297278;     umPpBHhMugEEyIokf48297278 = umPpBHhMugEEyIokf63279309;     umPpBHhMugEEyIokf63279309 = umPpBHhMugEEyIokf56926506;     umPpBHhMugEEyIokf56926506 = umPpBHhMugEEyIokf72129850;     umPpBHhMugEEyIokf72129850 = umPpBHhMugEEyIokf80388103;     umPpBHhMugEEyIokf80388103 = umPpBHhMugEEyIokf65057768;     umPpBHhMugEEyIokf65057768 = umPpBHhMugEEyIokf70814696;     umPpBHhMugEEyIokf70814696 = umPpBHhMugEEyIokf98392120;     umPpBHhMugEEyIokf98392120 = umPpBHhMugEEyIokf15719617;     umPpBHhMugEEyIokf15719617 = umPpBHhMugEEyIokf71525193;     umPpBHhMugEEyIokf71525193 = umPpBHhMugEEyIokf51942620;     umPpBHhMugEEyIokf51942620 = umPpBHhMugEEyIokf38235530;     umPpBHhMugEEyIokf38235530 = umPpBHhMugEEyIokf88533177;     umPpBHhMugEEyIokf88533177 = umPpBHhMugEEyIokf97795011;     umPpBHhMugEEyIokf97795011 = umPpBHhMugEEyIokf18883855;     umPpBHhMugEEyIokf18883855 = umPpBHhMugEEyIokf31577335;     umPpBHhMugEEyIokf31577335 = umPpBHhMugEEyIokf14468210;     umPpBHhMugEEyIokf14468210 = umPpBHhMugEEyIokf80542833;     umPpBHhMugEEyIokf80542833 = umPpBHhMugEEyIokf42162073;     umPpBHhMugEEyIokf42162073 = umPpBHhMugEEyIokf63455608;     umPpBHhMugEEyIokf63455608 = umPpBHhMugEEyIokf29775222;     umPpBHhMugEEyIokf29775222 = umPpBHhMugEEyIokf72855608;     umPpBHhMugEEyIokf72855608 = umPpBHhMugEEyIokf2144728;     umPpBHhMugEEyIokf2144728 = umPpBHhMugEEyIokf87777999;     umPpBHhMugEEyIokf87777999 = umPpBHhMugEEyIokf93406876;     umPpBHhMugEEyIokf93406876 = umPpBHhMugEEyIokf78736782;     umPpBHhMugEEyIokf78736782 = umPpBHhMugEEyIokf75550480;     umPpBHhMugEEyIokf75550480 = umPpBHhMugEEyIokf6157450;     umPpBHhMugEEyIokf6157450 = umPpBHhMugEEyIokf5392143;     umPpBHhMugEEyIokf5392143 = umPpBHhMugEEyIokf21079544;     umPpBHhMugEEyIokf21079544 = umPpBHhMugEEyIokf27820968;     umPpBHhMugEEyIokf27820968 = umPpBHhMugEEyIokf80109983;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void SiCpBNCUTvQLIKtz60629782() {     double lplevFGFbTvbTQUkI86227090 = -676498638;    double lplevFGFbTvbTQUkI18396318 = -154516683;    double lplevFGFbTvbTQUkI11889474 = -145608048;    double lplevFGFbTvbTQUkI4184355 = -433346180;    double lplevFGFbTvbTQUkI35604921 = -106579884;    double lplevFGFbTvbTQUkI38774097 = -573004587;    double lplevFGFbTvbTQUkI90623895 = 1775823;    double lplevFGFbTvbTQUkI27286381 = -276467816;    double lplevFGFbTvbTQUkI33310084 = -342215889;    double lplevFGFbTvbTQUkI45979283 = -988169468;    double lplevFGFbTvbTQUkI69688766 = -210477561;    double lplevFGFbTvbTQUkI63116246 = -255785725;    double lplevFGFbTvbTQUkI70732599 = -689872634;    double lplevFGFbTvbTQUkI76797827 = 98835651;    double lplevFGFbTvbTQUkI40671336 = -364902105;    double lplevFGFbTvbTQUkI46638120 = -158423339;    double lplevFGFbTvbTQUkI38762603 = -325223650;    double lplevFGFbTvbTQUkI21330870 = -138318455;    double lplevFGFbTvbTQUkI83540286 = -462810209;    double lplevFGFbTvbTQUkI36346285 = -257126824;    double lplevFGFbTvbTQUkI32336847 = -16741781;    double lplevFGFbTvbTQUkI7606157 = -432722299;    double lplevFGFbTvbTQUkI86837806 = -132550247;    double lplevFGFbTvbTQUkI40130975 = -396437559;    double lplevFGFbTvbTQUkI20587131 = -543337671;    double lplevFGFbTvbTQUkI32690117 = -206174930;    double lplevFGFbTvbTQUkI83231113 = -140028627;    double lplevFGFbTvbTQUkI28054183 = -199976244;    double lplevFGFbTvbTQUkI76847236 = -104218181;    double lplevFGFbTvbTQUkI19982355 = -932890697;    double lplevFGFbTvbTQUkI82544318 = -202091225;    double lplevFGFbTvbTQUkI50396658 = -786447060;    double lplevFGFbTvbTQUkI55641405 = -951572570;    double lplevFGFbTvbTQUkI60083233 = -270795286;    double lplevFGFbTvbTQUkI50274724 = -402767541;    double lplevFGFbTvbTQUkI65357545 = -248518378;    double lplevFGFbTvbTQUkI23115352 = -984995795;    double lplevFGFbTvbTQUkI22090306 = -453163332;    double lplevFGFbTvbTQUkI25758120 = -271079;    double lplevFGFbTvbTQUkI28824280 = -717580191;    double lplevFGFbTvbTQUkI96407575 = -946864386;    double lplevFGFbTvbTQUkI5119415 = -294577210;    double lplevFGFbTvbTQUkI759991 = 57161436;    double lplevFGFbTvbTQUkI22935139 = -662137256;    double lplevFGFbTvbTQUkI96565872 = 98669257;    double lplevFGFbTvbTQUkI36884056 = -272424901;    double lplevFGFbTvbTQUkI87511486 = -745201990;    double lplevFGFbTvbTQUkI4195100 = -575411761;    double lplevFGFbTvbTQUkI26827775 = -283821411;    double lplevFGFbTvbTQUkI4742779 = 64517594;    double lplevFGFbTvbTQUkI55490062 = -202559858;    double lplevFGFbTvbTQUkI86643278 = -160198567;    double lplevFGFbTvbTQUkI20491660 = -575434908;    double lplevFGFbTvbTQUkI99737264 = 11628736;    double lplevFGFbTvbTQUkI28292935 = -33246813;    double lplevFGFbTvbTQUkI78620933 = -143776340;    double lplevFGFbTvbTQUkI31558511 = 78033563;    double lplevFGFbTvbTQUkI71758499 = -749170490;    double lplevFGFbTvbTQUkI83597224 = -890008509;    double lplevFGFbTvbTQUkI2914804 = -900404954;    double lplevFGFbTvbTQUkI55542983 = -332975961;    double lplevFGFbTvbTQUkI62569713 = -798247933;    double lplevFGFbTvbTQUkI50439144 = -72249636;    double lplevFGFbTvbTQUkI13327729 = -409325192;    double lplevFGFbTvbTQUkI63434965 = -686078243;    double lplevFGFbTvbTQUkI19292109 = -424030502;    double lplevFGFbTvbTQUkI7474841 = -304213156;    double lplevFGFbTvbTQUkI10649367 = -319077349;    double lplevFGFbTvbTQUkI26523104 = -498396808;    double lplevFGFbTvbTQUkI75313791 = -16383728;    double lplevFGFbTvbTQUkI23522768 = -173427545;    double lplevFGFbTvbTQUkI16672297 = -872060318;    double lplevFGFbTvbTQUkI95572749 = -38047377;    double lplevFGFbTvbTQUkI54716007 = -745230019;    double lplevFGFbTvbTQUkI39938709 = -310262438;    double lplevFGFbTvbTQUkI27217432 = -722164571;    double lplevFGFbTvbTQUkI6846166 = -389883735;    double lplevFGFbTvbTQUkI63902668 = -470412991;    double lplevFGFbTvbTQUkI43565102 = -395106817;    double lplevFGFbTvbTQUkI83703074 = -170912771;    double lplevFGFbTvbTQUkI45178631 = -460972941;    double lplevFGFbTvbTQUkI79036014 = -564616866;    double lplevFGFbTvbTQUkI1226409 = -916154834;    double lplevFGFbTvbTQUkI72104458 = -68735775;    double lplevFGFbTvbTQUkI64492293 = -630330840;    double lplevFGFbTvbTQUkI95901039 = 58107342;    double lplevFGFbTvbTQUkI29904998 = -111012153;    double lplevFGFbTvbTQUkI55904141 = -863201307;    double lplevFGFbTvbTQUkI31790298 = -137548473;    double lplevFGFbTvbTQUkI71653790 = -158991202;    double lplevFGFbTvbTQUkI33799034 = -226551942;    double lplevFGFbTvbTQUkI51356852 = -135825306;    double lplevFGFbTvbTQUkI38493081 = -563154823;    double lplevFGFbTvbTQUkI22843317 = -99866125;    double lplevFGFbTvbTQUkI73281296 = -284604230;    double lplevFGFbTvbTQUkI33837863 = -48616453;    double lplevFGFbTvbTQUkI54680271 = -122327574;    double lplevFGFbTvbTQUkI87432262 = -533513373;    double lplevFGFbTvbTQUkI59500174 = -976059013;    double lplevFGFbTvbTQUkI77273764 = -676498638;     lplevFGFbTvbTQUkI86227090 = lplevFGFbTvbTQUkI18396318;     lplevFGFbTvbTQUkI18396318 = lplevFGFbTvbTQUkI11889474;     lplevFGFbTvbTQUkI11889474 = lplevFGFbTvbTQUkI4184355;     lplevFGFbTvbTQUkI4184355 = lplevFGFbTvbTQUkI35604921;     lplevFGFbTvbTQUkI35604921 = lplevFGFbTvbTQUkI38774097;     lplevFGFbTvbTQUkI38774097 = lplevFGFbTvbTQUkI90623895;     lplevFGFbTvbTQUkI90623895 = lplevFGFbTvbTQUkI27286381;     lplevFGFbTvbTQUkI27286381 = lplevFGFbTvbTQUkI33310084;     lplevFGFbTvbTQUkI33310084 = lplevFGFbTvbTQUkI45979283;     lplevFGFbTvbTQUkI45979283 = lplevFGFbTvbTQUkI69688766;     lplevFGFbTvbTQUkI69688766 = lplevFGFbTvbTQUkI63116246;     lplevFGFbTvbTQUkI63116246 = lplevFGFbTvbTQUkI70732599;     lplevFGFbTvbTQUkI70732599 = lplevFGFbTvbTQUkI76797827;     lplevFGFbTvbTQUkI76797827 = lplevFGFbTvbTQUkI40671336;     lplevFGFbTvbTQUkI40671336 = lplevFGFbTvbTQUkI46638120;     lplevFGFbTvbTQUkI46638120 = lplevFGFbTvbTQUkI38762603;     lplevFGFbTvbTQUkI38762603 = lplevFGFbTvbTQUkI21330870;     lplevFGFbTvbTQUkI21330870 = lplevFGFbTvbTQUkI83540286;     lplevFGFbTvbTQUkI83540286 = lplevFGFbTvbTQUkI36346285;     lplevFGFbTvbTQUkI36346285 = lplevFGFbTvbTQUkI32336847;     lplevFGFbTvbTQUkI32336847 = lplevFGFbTvbTQUkI7606157;     lplevFGFbTvbTQUkI7606157 = lplevFGFbTvbTQUkI86837806;     lplevFGFbTvbTQUkI86837806 = lplevFGFbTvbTQUkI40130975;     lplevFGFbTvbTQUkI40130975 = lplevFGFbTvbTQUkI20587131;     lplevFGFbTvbTQUkI20587131 = lplevFGFbTvbTQUkI32690117;     lplevFGFbTvbTQUkI32690117 = lplevFGFbTvbTQUkI83231113;     lplevFGFbTvbTQUkI83231113 = lplevFGFbTvbTQUkI28054183;     lplevFGFbTvbTQUkI28054183 = lplevFGFbTvbTQUkI76847236;     lplevFGFbTvbTQUkI76847236 = lplevFGFbTvbTQUkI19982355;     lplevFGFbTvbTQUkI19982355 = lplevFGFbTvbTQUkI82544318;     lplevFGFbTvbTQUkI82544318 = lplevFGFbTvbTQUkI50396658;     lplevFGFbTvbTQUkI50396658 = lplevFGFbTvbTQUkI55641405;     lplevFGFbTvbTQUkI55641405 = lplevFGFbTvbTQUkI60083233;     lplevFGFbTvbTQUkI60083233 = lplevFGFbTvbTQUkI50274724;     lplevFGFbTvbTQUkI50274724 = lplevFGFbTvbTQUkI65357545;     lplevFGFbTvbTQUkI65357545 = lplevFGFbTvbTQUkI23115352;     lplevFGFbTvbTQUkI23115352 = lplevFGFbTvbTQUkI22090306;     lplevFGFbTvbTQUkI22090306 = lplevFGFbTvbTQUkI25758120;     lplevFGFbTvbTQUkI25758120 = lplevFGFbTvbTQUkI28824280;     lplevFGFbTvbTQUkI28824280 = lplevFGFbTvbTQUkI96407575;     lplevFGFbTvbTQUkI96407575 = lplevFGFbTvbTQUkI5119415;     lplevFGFbTvbTQUkI5119415 = lplevFGFbTvbTQUkI759991;     lplevFGFbTvbTQUkI759991 = lplevFGFbTvbTQUkI22935139;     lplevFGFbTvbTQUkI22935139 = lplevFGFbTvbTQUkI96565872;     lplevFGFbTvbTQUkI96565872 = lplevFGFbTvbTQUkI36884056;     lplevFGFbTvbTQUkI36884056 = lplevFGFbTvbTQUkI87511486;     lplevFGFbTvbTQUkI87511486 = lplevFGFbTvbTQUkI4195100;     lplevFGFbTvbTQUkI4195100 = lplevFGFbTvbTQUkI26827775;     lplevFGFbTvbTQUkI26827775 = lplevFGFbTvbTQUkI4742779;     lplevFGFbTvbTQUkI4742779 = lplevFGFbTvbTQUkI55490062;     lplevFGFbTvbTQUkI55490062 = lplevFGFbTvbTQUkI86643278;     lplevFGFbTvbTQUkI86643278 = lplevFGFbTvbTQUkI20491660;     lplevFGFbTvbTQUkI20491660 = lplevFGFbTvbTQUkI99737264;     lplevFGFbTvbTQUkI99737264 = lplevFGFbTvbTQUkI28292935;     lplevFGFbTvbTQUkI28292935 = lplevFGFbTvbTQUkI78620933;     lplevFGFbTvbTQUkI78620933 = lplevFGFbTvbTQUkI31558511;     lplevFGFbTvbTQUkI31558511 = lplevFGFbTvbTQUkI71758499;     lplevFGFbTvbTQUkI71758499 = lplevFGFbTvbTQUkI83597224;     lplevFGFbTvbTQUkI83597224 = lplevFGFbTvbTQUkI2914804;     lplevFGFbTvbTQUkI2914804 = lplevFGFbTvbTQUkI55542983;     lplevFGFbTvbTQUkI55542983 = lplevFGFbTvbTQUkI62569713;     lplevFGFbTvbTQUkI62569713 = lplevFGFbTvbTQUkI50439144;     lplevFGFbTvbTQUkI50439144 = lplevFGFbTvbTQUkI13327729;     lplevFGFbTvbTQUkI13327729 = lplevFGFbTvbTQUkI63434965;     lplevFGFbTvbTQUkI63434965 = lplevFGFbTvbTQUkI19292109;     lplevFGFbTvbTQUkI19292109 = lplevFGFbTvbTQUkI7474841;     lplevFGFbTvbTQUkI7474841 = lplevFGFbTvbTQUkI10649367;     lplevFGFbTvbTQUkI10649367 = lplevFGFbTvbTQUkI26523104;     lplevFGFbTvbTQUkI26523104 = lplevFGFbTvbTQUkI75313791;     lplevFGFbTvbTQUkI75313791 = lplevFGFbTvbTQUkI23522768;     lplevFGFbTvbTQUkI23522768 = lplevFGFbTvbTQUkI16672297;     lplevFGFbTvbTQUkI16672297 = lplevFGFbTvbTQUkI95572749;     lplevFGFbTvbTQUkI95572749 = lplevFGFbTvbTQUkI54716007;     lplevFGFbTvbTQUkI54716007 = lplevFGFbTvbTQUkI39938709;     lplevFGFbTvbTQUkI39938709 = lplevFGFbTvbTQUkI27217432;     lplevFGFbTvbTQUkI27217432 = lplevFGFbTvbTQUkI6846166;     lplevFGFbTvbTQUkI6846166 = lplevFGFbTvbTQUkI63902668;     lplevFGFbTvbTQUkI63902668 = lplevFGFbTvbTQUkI43565102;     lplevFGFbTvbTQUkI43565102 = lplevFGFbTvbTQUkI83703074;     lplevFGFbTvbTQUkI83703074 = lplevFGFbTvbTQUkI45178631;     lplevFGFbTvbTQUkI45178631 = lplevFGFbTvbTQUkI79036014;     lplevFGFbTvbTQUkI79036014 = lplevFGFbTvbTQUkI1226409;     lplevFGFbTvbTQUkI1226409 = lplevFGFbTvbTQUkI72104458;     lplevFGFbTvbTQUkI72104458 = lplevFGFbTvbTQUkI64492293;     lplevFGFbTvbTQUkI64492293 = lplevFGFbTvbTQUkI95901039;     lplevFGFbTvbTQUkI95901039 = lplevFGFbTvbTQUkI29904998;     lplevFGFbTvbTQUkI29904998 = lplevFGFbTvbTQUkI55904141;     lplevFGFbTvbTQUkI55904141 = lplevFGFbTvbTQUkI31790298;     lplevFGFbTvbTQUkI31790298 = lplevFGFbTvbTQUkI71653790;     lplevFGFbTvbTQUkI71653790 = lplevFGFbTvbTQUkI33799034;     lplevFGFbTvbTQUkI33799034 = lplevFGFbTvbTQUkI51356852;     lplevFGFbTvbTQUkI51356852 = lplevFGFbTvbTQUkI38493081;     lplevFGFbTvbTQUkI38493081 = lplevFGFbTvbTQUkI22843317;     lplevFGFbTvbTQUkI22843317 = lplevFGFbTvbTQUkI73281296;     lplevFGFbTvbTQUkI73281296 = lplevFGFbTvbTQUkI33837863;     lplevFGFbTvbTQUkI33837863 = lplevFGFbTvbTQUkI54680271;     lplevFGFbTvbTQUkI54680271 = lplevFGFbTvbTQUkI87432262;     lplevFGFbTvbTQUkI87432262 = lplevFGFbTvbTQUkI59500174;     lplevFGFbTvbTQUkI59500174 = lplevFGFbTvbTQUkI77273764;     lplevFGFbTvbTQUkI77273764 = lplevFGFbTvbTQUkI86227090;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WuvAoQccIidtBlXY27921382() {     double nfvrZpzdjKgGEIqlE21686059 = -42775538;    double nfvrZpzdjKgGEIqlE31668396 = -35068290;    double nfvrZpzdjKgGEIqlE49275951 = -911275123;    double nfvrZpzdjKgGEIqlE76407710 = -254497252;    double nfvrZpzdjKgGEIqlE91890189 = -212178729;    double nfvrZpzdjKgGEIqlE74996969 = -835352482;    double nfvrZpzdjKgGEIqlE1281302 = -942949911;    double nfvrZpzdjKgGEIqlE89417941 = 79298043;    double nfvrZpzdjKgGEIqlE34827689 = -830547296;    double nfvrZpzdjKgGEIqlE60881995 = -652856466;    double nfvrZpzdjKgGEIqlE7870501 = -12485291;    double nfvrZpzdjKgGEIqlE63684394 = -119906497;    double nfvrZpzdjKgGEIqlE8967246 = -141885138;    double nfvrZpzdjKgGEIqlE29938691 = -977527759;    double nfvrZpzdjKgGEIqlE98865852 = -868327120;    double nfvrZpzdjKgGEIqlE30179421 = -894234947;    double nfvrZpzdjKgGEIqlE94598538 = -852198203;    double nfvrZpzdjKgGEIqlE21091061 = -211746524;    double nfvrZpzdjKgGEIqlE84090616 = -779373630;    double nfvrZpzdjKgGEIqlE30183503 = -683687890;    double nfvrZpzdjKgGEIqlE45423710 = -774016469;    double nfvrZpzdjKgGEIqlE11027682 = -519358593;    double nfvrZpzdjKgGEIqlE57363186 = -870182431;    double nfvrZpzdjKgGEIqlE33923920 = -907262163;    double nfvrZpzdjKgGEIqlE28605082 = -567322870;    double nfvrZpzdjKgGEIqlE43388289 = -994751737;    double nfvrZpzdjKgGEIqlE43651688 = 53687233;    double nfvrZpzdjKgGEIqlE68782608 = -369748419;    double nfvrZpzdjKgGEIqlE23515452 = -678312983;    double nfvrZpzdjKgGEIqlE25515766 = -967397868;    double nfvrZpzdjKgGEIqlE1127550 = -754071712;    double nfvrZpzdjKgGEIqlE88219676 = -559418474;    double nfvrZpzdjKgGEIqlE67859416 = 831453;    double nfvrZpzdjKgGEIqlE89831659 = -335489099;    double nfvrZpzdjKgGEIqlE16083178 = -353867792;    double nfvrZpzdjKgGEIqlE16918389 = -463083684;    double nfvrZpzdjKgGEIqlE72527168 = -849691835;    double nfvrZpzdjKgGEIqlE73305709 = -435818914;    double nfvrZpzdjKgGEIqlE56148682 = -751147355;    double nfvrZpzdjKgGEIqlE93095811 = -182842737;    double nfvrZpzdjKgGEIqlE29859184 = -716063300;    double nfvrZpzdjKgGEIqlE94484541 = -989122501;    double nfvrZpzdjKgGEIqlE90143369 = -596019350;    double nfvrZpzdjKgGEIqlE63066477 = -166469666;    double nfvrZpzdjKgGEIqlE96582331 = -827841904;    double nfvrZpzdjKgGEIqlE82845384 = -311348533;    double nfvrZpzdjKgGEIqlE6856650 = -129188351;    double nfvrZpzdjKgGEIqlE21921411 = -741938354;    double nfvrZpzdjKgGEIqlE49720441 = 52441709;    double nfvrZpzdjKgGEIqlE29272368 = -426509222;    double nfvrZpzdjKgGEIqlE36260786 = -76836415;    double nfvrZpzdjKgGEIqlE70128751 = -373850914;    double nfvrZpzdjKgGEIqlE89112808 = -331480462;    double nfvrZpzdjKgGEIqlE15729111 = -179879763;    double nfvrZpzdjKgGEIqlE82497671 = -379736959;    double nfvrZpzdjKgGEIqlE10658377 = -523416946;    double nfvrZpzdjKgGEIqlE74305209 = -164885859;    double nfvrZpzdjKgGEIqlE15352031 = 95987039;    double nfvrZpzdjKgGEIqlE47802628 = -687174382;    double nfvrZpzdjKgGEIqlE48501901 = -217426993;    double nfvrZpzdjKgGEIqlE31345282 = -789039715;    double nfvrZpzdjKgGEIqlE32498694 = -473201493;    double nfvrZpzdjKgGEIqlE65902490 = -242388974;    double nfvrZpzdjKgGEIqlE9311923 = -863149428;    double nfvrZpzdjKgGEIqlE59754445 = -898784754;    double nfvrZpzdjKgGEIqlE19650825 = -453066817;    double nfvrZpzdjKgGEIqlE95824978 = -20737950;    double nfvrZpzdjKgGEIqlE19135586 = -806396040;    double nfvrZpzdjKgGEIqlE13855513 = -523659967;    double nfvrZpzdjKgGEIqlE81947463 = -305243437;    double nfvrZpzdjKgGEIqlE57652252 = 55456888;    double nfvrZpzdjKgGEIqlE21292830 = -316379289;    double nfvrZpzdjKgGEIqlE64942379 = -460599169;    double nfvrZpzdjKgGEIqlE90994804 = -496530894;    double nfvrZpzdjKgGEIqlE324320 = -967624590;    double nfvrZpzdjKgGEIqlE50939168 = -784893969;    double nfvrZpzdjKgGEIqlE20884313 = -923339243;    double nfvrZpzdjKgGEIqlE94296709 = -603712765;    double nfvrZpzdjKgGEIqlE37341589 = 20579740;    double nfvrZpzdjKgGEIqlE45759697 = -155974337;    double nfvrZpzdjKgGEIqlE36531639 = -765563387;    double nfvrZpzdjKgGEIqlE21730277 = -204374414;    double nfvrZpzdjKgGEIqlE19062167 = -322190128;    double nfvrZpzdjKgGEIqlE94243083 = -151803762;    double nfvrZpzdjKgGEIqlE89254980 = -790561453;    double nfvrZpzdjKgGEIqlE30998799 = -280220799;    double nfvrZpzdjKgGEIqlE99106867 = -127938013;    double nfvrZpzdjKgGEIqlE52130305 = -819288784;    double nfvrZpzdjKgGEIqlE7333989 = -955752141;    double nfvrZpzdjKgGEIqlE5424801 = -830450847;    double nfvrZpzdjKgGEIqlE42613179 = -198197825;    double nfvrZpzdjKgGEIqlE57175137 = -845678875;    double nfvrZpzdjKgGEIqlE25503081 = -748644532;    double nfvrZpzdjKgGEIqlE7646781 = -433720363;    double nfvrZpzdjKgGEIqlE61750530 = -393803022;    double nfvrZpzdjKgGEIqlE97360489 = -142861808;    double nfvrZpzdjKgGEIqlE28582051 = -646733527;    double nfvrZpzdjKgGEIqlE80831446 = -732869922;    double nfvrZpzdjKgGEIqlE3312032 = -267684912;    double nfvrZpzdjKgGEIqlE76931506 = -42775538;     nfvrZpzdjKgGEIqlE21686059 = nfvrZpzdjKgGEIqlE31668396;     nfvrZpzdjKgGEIqlE31668396 = nfvrZpzdjKgGEIqlE49275951;     nfvrZpzdjKgGEIqlE49275951 = nfvrZpzdjKgGEIqlE76407710;     nfvrZpzdjKgGEIqlE76407710 = nfvrZpzdjKgGEIqlE91890189;     nfvrZpzdjKgGEIqlE91890189 = nfvrZpzdjKgGEIqlE74996969;     nfvrZpzdjKgGEIqlE74996969 = nfvrZpzdjKgGEIqlE1281302;     nfvrZpzdjKgGEIqlE1281302 = nfvrZpzdjKgGEIqlE89417941;     nfvrZpzdjKgGEIqlE89417941 = nfvrZpzdjKgGEIqlE34827689;     nfvrZpzdjKgGEIqlE34827689 = nfvrZpzdjKgGEIqlE60881995;     nfvrZpzdjKgGEIqlE60881995 = nfvrZpzdjKgGEIqlE7870501;     nfvrZpzdjKgGEIqlE7870501 = nfvrZpzdjKgGEIqlE63684394;     nfvrZpzdjKgGEIqlE63684394 = nfvrZpzdjKgGEIqlE8967246;     nfvrZpzdjKgGEIqlE8967246 = nfvrZpzdjKgGEIqlE29938691;     nfvrZpzdjKgGEIqlE29938691 = nfvrZpzdjKgGEIqlE98865852;     nfvrZpzdjKgGEIqlE98865852 = nfvrZpzdjKgGEIqlE30179421;     nfvrZpzdjKgGEIqlE30179421 = nfvrZpzdjKgGEIqlE94598538;     nfvrZpzdjKgGEIqlE94598538 = nfvrZpzdjKgGEIqlE21091061;     nfvrZpzdjKgGEIqlE21091061 = nfvrZpzdjKgGEIqlE84090616;     nfvrZpzdjKgGEIqlE84090616 = nfvrZpzdjKgGEIqlE30183503;     nfvrZpzdjKgGEIqlE30183503 = nfvrZpzdjKgGEIqlE45423710;     nfvrZpzdjKgGEIqlE45423710 = nfvrZpzdjKgGEIqlE11027682;     nfvrZpzdjKgGEIqlE11027682 = nfvrZpzdjKgGEIqlE57363186;     nfvrZpzdjKgGEIqlE57363186 = nfvrZpzdjKgGEIqlE33923920;     nfvrZpzdjKgGEIqlE33923920 = nfvrZpzdjKgGEIqlE28605082;     nfvrZpzdjKgGEIqlE28605082 = nfvrZpzdjKgGEIqlE43388289;     nfvrZpzdjKgGEIqlE43388289 = nfvrZpzdjKgGEIqlE43651688;     nfvrZpzdjKgGEIqlE43651688 = nfvrZpzdjKgGEIqlE68782608;     nfvrZpzdjKgGEIqlE68782608 = nfvrZpzdjKgGEIqlE23515452;     nfvrZpzdjKgGEIqlE23515452 = nfvrZpzdjKgGEIqlE25515766;     nfvrZpzdjKgGEIqlE25515766 = nfvrZpzdjKgGEIqlE1127550;     nfvrZpzdjKgGEIqlE1127550 = nfvrZpzdjKgGEIqlE88219676;     nfvrZpzdjKgGEIqlE88219676 = nfvrZpzdjKgGEIqlE67859416;     nfvrZpzdjKgGEIqlE67859416 = nfvrZpzdjKgGEIqlE89831659;     nfvrZpzdjKgGEIqlE89831659 = nfvrZpzdjKgGEIqlE16083178;     nfvrZpzdjKgGEIqlE16083178 = nfvrZpzdjKgGEIqlE16918389;     nfvrZpzdjKgGEIqlE16918389 = nfvrZpzdjKgGEIqlE72527168;     nfvrZpzdjKgGEIqlE72527168 = nfvrZpzdjKgGEIqlE73305709;     nfvrZpzdjKgGEIqlE73305709 = nfvrZpzdjKgGEIqlE56148682;     nfvrZpzdjKgGEIqlE56148682 = nfvrZpzdjKgGEIqlE93095811;     nfvrZpzdjKgGEIqlE93095811 = nfvrZpzdjKgGEIqlE29859184;     nfvrZpzdjKgGEIqlE29859184 = nfvrZpzdjKgGEIqlE94484541;     nfvrZpzdjKgGEIqlE94484541 = nfvrZpzdjKgGEIqlE90143369;     nfvrZpzdjKgGEIqlE90143369 = nfvrZpzdjKgGEIqlE63066477;     nfvrZpzdjKgGEIqlE63066477 = nfvrZpzdjKgGEIqlE96582331;     nfvrZpzdjKgGEIqlE96582331 = nfvrZpzdjKgGEIqlE82845384;     nfvrZpzdjKgGEIqlE82845384 = nfvrZpzdjKgGEIqlE6856650;     nfvrZpzdjKgGEIqlE6856650 = nfvrZpzdjKgGEIqlE21921411;     nfvrZpzdjKgGEIqlE21921411 = nfvrZpzdjKgGEIqlE49720441;     nfvrZpzdjKgGEIqlE49720441 = nfvrZpzdjKgGEIqlE29272368;     nfvrZpzdjKgGEIqlE29272368 = nfvrZpzdjKgGEIqlE36260786;     nfvrZpzdjKgGEIqlE36260786 = nfvrZpzdjKgGEIqlE70128751;     nfvrZpzdjKgGEIqlE70128751 = nfvrZpzdjKgGEIqlE89112808;     nfvrZpzdjKgGEIqlE89112808 = nfvrZpzdjKgGEIqlE15729111;     nfvrZpzdjKgGEIqlE15729111 = nfvrZpzdjKgGEIqlE82497671;     nfvrZpzdjKgGEIqlE82497671 = nfvrZpzdjKgGEIqlE10658377;     nfvrZpzdjKgGEIqlE10658377 = nfvrZpzdjKgGEIqlE74305209;     nfvrZpzdjKgGEIqlE74305209 = nfvrZpzdjKgGEIqlE15352031;     nfvrZpzdjKgGEIqlE15352031 = nfvrZpzdjKgGEIqlE47802628;     nfvrZpzdjKgGEIqlE47802628 = nfvrZpzdjKgGEIqlE48501901;     nfvrZpzdjKgGEIqlE48501901 = nfvrZpzdjKgGEIqlE31345282;     nfvrZpzdjKgGEIqlE31345282 = nfvrZpzdjKgGEIqlE32498694;     nfvrZpzdjKgGEIqlE32498694 = nfvrZpzdjKgGEIqlE65902490;     nfvrZpzdjKgGEIqlE65902490 = nfvrZpzdjKgGEIqlE9311923;     nfvrZpzdjKgGEIqlE9311923 = nfvrZpzdjKgGEIqlE59754445;     nfvrZpzdjKgGEIqlE59754445 = nfvrZpzdjKgGEIqlE19650825;     nfvrZpzdjKgGEIqlE19650825 = nfvrZpzdjKgGEIqlE95824978;     nfvrZpzdjKgGEIqlE95824978 = nfvrZpzdjKgGEIqlE19135586;     nfvrZpzdjKgGEIqlE19135586 = nfvrZpzdjKgGEIqlE13855513;     nfvrZpzdjKgGEIqlE13855513 = nfvrZpzdjKgGEIqlE81947463;     nfvrZpzdjKgGEIqlE81947463 = nfvrZpzdjKgGEIqlE57652252;     nfvrZpzdjKgGEIqlE57652252 = nfvrZpzdjKgGEIqlE21292830;     nfvrZpzdjKgGEIqlE21292830 = nfvrZpzdjKgGEIqlE64942379;     nfvrZpzdjKgGEIqlE64942379 = nfvrZpzdjKgGEIqlE90994804;     nfvrZpzdjKgGEIqlE90994804 = nfvrZpzdjKgGEIqlE324320;     nfvrZpzdjKgGEIqlE324320 = nfvrZpzdjKgGEIqlE50939168;     nfvrZpzdjKgGEIqlE50939168 = nfvrZpzdjKgGEIqlE20884313;     nfvrZpzdjKgGEIqlE20884313 = nfvrZpzdjKgGEIqlE94296709;     nfvrZpzdjKgGEIqlE94296709 = nfvrZpzdjKgGEIqlE37341589;     nfvrZpzdjKgGEIqlE37341589 = nfvrZpzdjKgGEIqlE45759697;     nfvrZpzdjKgGEIqlE45759697 = nfvrZpzdjKgGEIqlE36531639;     nfvrZpzdjKgGEIqlE36531639 = nfvrZpzdjKgGEIqlE21730277;     nfvrZpzdjKgGEIqlE21730277 = nfvrZpzdjKgGEIqlE19062167;     nfvrZpzdjKgGEIqlE19062167 = nfvrZpzdjKgGEIqlE94243083;     nfvrZpzdjKgGEIqlE94243083 = nfvrZpzdjKgGEIqlE89254980;     nfvrZpzdjKgGEIqlE89254980 = nfvrZpzdjKgGEIqlE30998799;     nfvrZpzdjKgGEIqlE30998799 = nfvrZpzdjKgGEIqlE99106867;     nfvrZpzdjKgGEIqlE99106867 = nfvrZpzdjKgGEIqlE52130305;     nfvrZpzdjKgGEIqlE52130305 = nfvrZpzdjKgGEIqlE7333989;     nfvrZpzdjKgGEIqlE7333989 = nfvrZpzdjKgGEIqlE5424801;     nfvrZpzdjKgGEIqlE5424801 = nfvrZpzdjKgGEIqlE42613179;     nfvrZpzdjKgGEIqlE42613179 = nfvrZpzdjKgGEIqlE57175137;     nfvrZpzdjKgGEIqlE57175137 = nfvrZpzdjKgGEIqlE25503081;     nfvrZpzdjKgGEIqlE25503081 = nfvrZpzdjKgGEIqlE7646781;     nfvrZpzdjKgGEIqlE7646781 = nfvrZpzdjKgGEIqlE61750530;     nfvrZpzdjKgGEIqlE61750530 = nfvrZpzdjKgGEIqlE97360489;     nfvrZpzdjKgGEIqlE97360489 = nfvrZpzdjKgGEIqlE28582051;     nfvrZpzdjKgGEIqlE28582051 = nfvrZpzdjKgGEIqlE80831446;     nfvrZpzdjKgGEIqlE80831446 = nfvrZpzdjKgGEIqlE3312032;     nfvrZpzdjKgGEIqlE3312032 = nfvrZpzdjKgGEIqlE76931506;     nfvrZpzdjKgGEIqlE76931506 = nfvrZpzdjKgGEIqlE21686059;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void KfnTsghkDhZBfMWE42970449() {     double VDqrsthZHQYQYfNMP27803165 = -154677427;    double VDqrsthZHQYQYfNMP75041675 = -844668070;    double VDqrsthZHQYQYfNMP65318134 = 37891956;    double VDqrsthZHQYQYfNMP3482491 = -445245882;    double VDqrsthZHQYQYfNMP60785678 = -732828600;    double VDqrsthZHQYQYfNMP66414171 = 151283;    double VDqrsthZHQYQYfNMP61770268 = 18085112;    double VDqrsthZHQYQYfNMP39177833 = -207575870;    double VDqrsthZHQYQYfNMP99470184 = -598579961;    double VDqrsthZHQYQYfNMP2111655 = -981831943;    double VDqrsthZHQYQYfNMP67980791 = -989922877;    double VDqrsthZHQYQYfNMP95507436 = -564312699;    double VDqrsthZHQYQYfNMP95176363 = -581776227;    double VDqrsthZHQYQYfNMP13082368 = -409766326;    double VDqrsthZHQYQYfNMP94608279 = -458789286;    double VDqrsthZHQYQYfNMP71684814 = -867234490;    double VDqrsthZHQYQYfNMP88434541 = -368743315;    double VDqrsthZHQYQYfNMP18144214 = -977269167;    double VDqrsthZHQYQYfNMP45920458 = -85142727;    double VDqrsthZHQYQYfNMP10733668 = 59409191;    double VDqrsthZHQYQYfNMP33976903 = -629503925;    double VDqrsthZHQYQYfNMP97548705 = -427288309;    double VDqrsthZHQYQYfNMP92953978 = -616699918;    double VDqrsthZHQYQYfNMP17916294 = -29567151;    double VDqrsthZHQYQYfNMP72304731 = -940518903;    double VDqrsthZHQYQYfNMP68109964 = -873407060;    double VDqrsthZHQYQYfNMP42111466 = -946974970;    double VDqrsthZHQYQYfNMP81889733 = -435677372;    double VDqrsthZHQYQYfNMP89647706 = -900695688;    double VDqrsthZHQYQYfNMP94131402 = -784868735;    double VDqrsthZHQYQYfNMP93370099 = -667392650;    double VDqrsthZHQYQYfNMP85830298 = -268865290;    double VDqrsthZHQYQYfNMP77570364 = -991781918;    double VDqrsthZHQYQYfNMP84662224 = -87030864;    double VDqrsthZHQYQYfNMP77557829 = -992600897;    double VDqrsthZHQYQYfNMP85644302 = -677109791;    double VDqrsthZHQYQYfNMP53789103 = -803327705;    double VDqrsthZHQYQYfNMP7395922 = -207324923;    double VDqrsthZHQYQYfNMP29758936 = -745070852;    double VDqrsthZHQYQYfNMP80597749 = -656281959;    double VDqrsthZHQYQYfNMP35528407 = -781337194;    double VDqrsthZHQYQYfNMP26634999 = -159264779;    double VDqrsthZHQYQYfNMP68210347 = -676823934;    double VDqrsthZHQYQYfNMP50474218 = -185812604;    double VDqrsthZHQYQYfNMP8534797 = -489740058;    double VDqrsthZHQYQYfNMP94784579 = -296564650;    double VDqrsthZHQYQYfNMP24635225 = -247963888;    double VDqrsthZHQYQYfNMP29878352 = -751833886;    double VDqrsthZHQYQYfNMP59396170 = -33295279;    double VDqrsthZHQYQYfNMP42184021 = -513520030;    double VDqrsthZHQYQYfNMP71961464 = 80869913;    double VDqrsthZHQYQYfNMP80938470 = -174577234;    double VDqrsthZHQYQYfNMP37361266 = -123270049;    double VDqrsthZHQYQYfNMP11697992 = -500349908;    double VDqrsthZHQYQYfNMP8993547 = 19731259;    double VDqrsthZHQYQYfNMP30254460 = -727389118;    double VDqrsthZHQYQYfNMP82087697 = -127968153;    double VDqrsthZHQYQYfNMP47401840 = -932540894;    double VDqrsthZHQYQYfNMP31177759 = -504726980;    double VDqrsthZHQYQYfNMP92675714 = -859421541;    double VDqrsthZHQYQYfNMP24302706 = -52873747;    double VDqrsthZHQYQYfNMP79880534 = -546237517;    double VDqrsthZHQYQYfNMP49530126 = -306880183;    double VDqrsthZHQYQYfNMP5338782 = -813711227;    double VDqrsthZHQYQYfNMP8741556 = -214439294;    double VDqrsthZHQYQYfNMP82150493 = -621057588;    double VDqrsthZHQYQYfNMP17937073 = -572530782;    double VDqrsthZHQYQYfNMP10514140 = -394745363;    double VDqrsthZHQYQYfNMP35524538 = -417165429;    double VDqrsthZHQYQYfNMP8963978 = -781679496;    double VDqrsthZHQYQYfNMP17895712 = 36093215;    double VDqrsthZHQYQYfNMP81038620 = -61418393;    double VDqrsthZHQYQYfNMP88385278 = -132198315;    double VDqrsthZHQYQYfNMP65322708 = -428860768;    double VDqrsthZHQYQYfNMP75205260 = -159253615;    double VDqrsthZHQYQYfNMP7341905 = -370239146;    double VDqrsthZHQYQYfNMP29338358 = -750464376;    double VDqrsthZHQYQYfNMP42479761 = -330887314;    double VDqrsthZHQYQYfNMP9381498 = -539827093;    double VDqrsthZHQYQYfNMP77520152 = -543954253;    double VDqrsthZHQYQYfNMP43474740 = -525443172;    double VDqrsthZHQYQYfNMP12233114 = -95141084;    double VDqrsthZHQYQYfNMP22493564 = -302382093;    double VDqrsthZHQYQYfNMP47463686 = -287175659;    double VDqrsthZHQYQYfNMP22169939 = -765738649;    double VDqrsthZHQYQYfNMP12431629 = -392815417;    double VDqrsthZHQYQYfNMP48469032 = -45595241;    double VDqrsthZHQYQYfNMP65872373 = -391432010;    double VDqrsthZHQYQYfNMP75668677 = -6762124;    double VDqrsthZHQYQYfNMP47303370 = -165211780;    double VDqrsthZHQYQYfNMP3556605 = -449141639;    double VDqrsthZHQYQYfNMP6387263 = -870786811;    double VDqrsthZHQYQYfNMP76218162 = -702597944;    double VDqrsthZHQYQYfNMP37083222 = -885649312;    double VDqrsthZHQYQYfNMP56295044 = -503408212;    double VDqrsthZHQYQYfNMP55647873 = -135099678;    double VDqrsthZHQYQYfNMP77104873 = -852384597;    double VDqrsthZHQYQYfNMP62871566 = -863112708;    double VDqrsthZHQYQYfNMP41732662 = -971373311;    double VDqrsthZHQYQYfNMP26384303 = -154677427;     VDqrsthZHQYQYfNMP27803165 = VDqrsthZHQYQYfNMP75041675;     VDqrsthZHQYQYfNMP75041675 = VDqrsthZHQYQYfNMP65318134;     VDqrsthZHQYQYfNMP65318134 = VDqrsthZHQYQYfNMP3482491;     VDqrsthZHQYQYfNMP3482491 = VDqrsthZHQYQYfNMP60785678;     VDqrsthZHQYQYfNMP60785678 = VDqrsthZHQYQYfNMP66414171;     VDqrsthZHQYQYfNMP66414171 = VDqrsthZHQYQYfNMP61770268;     VDqrsthZHQYQYfNMP61770268 = VDqrsthZHQYQYfNMP39177833;     VDqrsthZHQYQYfNMP39177833 = VDqrsthZHQYQYfNMP99470184;     VDqrsthZHQYQYfNMP99470184 = VDqrsthZHQYQYfNMP2111655;     VDqrsthZHQYQYfNMP2111655 = VDqrsthZHQYQYfNMP67980791;     VDqrsthZHQYQYfNMP67980791 = VDqrsthZHQYQYfNMP95507436;     VDqrsthZHQYQYfNMP95507436 = VDqrsthZHQYQYfNMP95176363;     VDqrsthZHQYQYfNMP95176363 = VDqrsthZHQYQYfNMP13082368;     VDqrsthZHQYQYfNMP13082368 = VDqrsthZHQYQYfNMP94608279;     VDqrsthZHQYQYfNMP94608279 = VDqrsthZHQYQYfNMP71684814;     VDqrsthZHQYQYfNMP71684814 = VDqrsthZHQYQYfNMP88434541;     VDqrsthZHQYQYfNMP88434541 = VDqrsthZHQYQYfNMP18144214;     VDqrsthZHQYQYfNMP18144214 = VDqrsthZHQYQYfNMP45920458;     VDqrsthZHQYQYfNMP45920458 = VDqrsthZHQYQYfNMP10733668;     VDqrsthZHQYQYfNMP10733668 = VDqrsthZHQYQYfNMP33976903;     VDqrsthZHQYQYfNMP33976903 = VDqrsthZHQYQYfNMP97548705;     VDqrsthZHQYQYfNMP97548705 = VDqrsthZHQYQYfNMP92953978;     VDqrsthZHQYQYfNMP92953978 = VDqrsthZHQYQYfNMP17916294;     VDqrsthZHQYQYfNMP17916294 = VDqrsthZHQYQYfNMP72304731;     VDqrsthZHQYQYfNMP72304731 = VDqrsthZHQYQYfNMP68109964;     VDqrsthZHQYQYfNMP68109964 = VDqrsthZHQYQYfNMP42111466;     VDqrsthZHQYQYfNMP42111466 = VDqrsthZHQYQYfNMP81889733;     VDqrsthZHQYQYfNMP81889733 = VDqrsthZHQYQYfNMP89647706;     VDqrsthZHQYQYfNMP89647706 = VDqrsthZHQYQYfNMP94131402;     VDqrsthZHQYQYfNMP94131402 = VDqrsthZHQYQYfNMP93370099;     VDqrsthZHQYQYfNMP93370099 = VDqrsthZHQYQYfNMP85830298;     VDqrsthZHQYQYfNMP85830298 = VDqrsthZHQYQYfNMP77570364;     VDqrsthZHQYQYfNMP77570364 = VDqrsthZHQYQYfNMP84662224;     VDqrsthZHQYQYfNMP84662224 = VDqrsthZHQYQYfNMP77557829;     VDqrsthZHQYQYfNMP77557829 = VDqrsthZHQYQYfNMP85644302;     VDqrsthZHQYQYfNMP85644302 = VDqrsthZHQYQYfNMP53789103;     VDqrsthZHQYQYfNMP53789103 = VDqrsthZHQYQYfNMP7395922;     VDqrsthZHQYQYfNMP7395922 = VDqrsthZHQYQYfNMP29758936;     VDqrsthZHQYQYfNMP29758936 = VDqrsthZHQYQYfNMP80597749;     VDqrsthZHQYQYfNMP80597749 = VDqrsthZHQYQYfNMP35528407;     VDqrsthZHQYQYfNMP35528407 = VDqrsthZHQYQYfNMP26634999;     VDqrsthZHQYQYfNMP26634999 = VDqrsthZHQYQYfNMP68210347;     VDqrsthZHQYQYfNMP68210347 = VDqrsthZHQYQYfNMP50474218;     VDqrsthZHQYQYfNMP50474218 = VDqrsthZHQYQYfNMP8534797;     VDqrsthZHQYQYfNMP8534797 = VDqrsthZHQYQYfNMP94784579;     VDqrsthZHQYQYfNMP94784579 = VDqrsthZHQYQYfNMP24635225;     VDqrsthZHQYQYfNMP24635225 = VDqrsthZHQYQYfNMP29878352;     VDqrsthZHQYQYfNMP29878352 = VDqrsthZHQYQYfNMP59396170;     VDqrsthZHQYQYfNMP59396170 = VDqrsthZHQYQYfNMP42184021;     VDqrsthZHQYQYfNMP42184021 = VDqrsthZHQYQYfNMP71961464;     VDqrsthZHQYQYfNMP71961464 = VDqrsthZHQYQYfNMP80938470;     VDqrsthZHQYQYfNMP80938470 = VDqrsthZHQYQYfNMP37361266;     VDqrsthZHQYQYfNMP37361266 = VDqrsthZHQYQYfNMP11697992;     VDqrsthZHQYQYfNMP11697992 = VDqrsthZHQYQYfNMP8993547;     VDqrsthZHQYQYfNMP8993547 = VDqrsthZHQYQYfNMP30254460;     VDqrsthZHQYQYfNMP30254460 = VDqrsthZHQYQYfNMP82087697;     VDqrsthZHQYQYfNMP82087697 = VDqrsthZHQYQYfNMP47401840;     VDqrsthZHQYQYfNMP47401840 = VDqrsthZHQYQYfNMP31177759;     VDqrsthZHQYQYfNMP31177759 = VDqrsthZHQYQYfNMP92675714;     VDqrsthZHQYQYfNMP92675714 = VDqrsthZHQYQYfNMP24302706;     VDqrsthZHQYQYfNMP24302706 = VDqrsthZHQYQYfNMP79880534;     VDqrsthZHQYQYfNMP79880534 = VDqrsthZHQYQYfNMP49530126;     VDqrsthZHQYQYfNMP49530126 = VDqrsthZHQYQYfNMP5338782;     VDqrsthZHQYQYfNMP5338782 = VDqrsthZHQYQYfNMP8741556;     VDqrsthZHQYQYfNMP8741556 = VDqrsthZHQYQYfNMP82150493;     VDqrsthZHQYQYfNMP82150493 = VDqrsthZHQYQYfNMP17937073;     VDqrsthZHQYQYfNMP17937073 = VDqrsthZHQYQYfNMP10514140;     VDqrsthZHQYQYfNMP10514140 = VDqrsthZHQYQYfNMP35524538;     VDqrsthZHQYQYfNMP35524538 = VDqrsthZHQYQYfNMP8963978;     VDqrsthZHQYQYfNMP8963978 = VDqrsthZHQYQYfNMP17895712;     VDqrsthZHQYQYfNMP17895712 = VDqrsthZHQYQYfNMP81038620;     VDqrsthZHQYQYfNMP81038620 = VDqrsthZHQYQYfNMP88385278;     VDqrsthZHQYQYfNMP88385278 = VDqrsthZHQYQYfNMP65322708;     VDqrsthZHQYQYfNMP65322708 = VDqrsthZHQYQYfNMP75205260;     VDqrsthZHQYQYfNMP75205260 = VDqrsthZHQYQYfNMP7341905;     VDqrsthZHQYQYfNMP7341905 = VDqrsthZHQYQYfNMP29338358;     VDqrsthZHQYQYfNMP29338358 = VDqrsthZHQYQYfNMP42479761;     VDqrsthZHQYQYfNMP42479761 = VDqrsthZHQYQYfNMP9381498;     VDqrsthZHQYQYfNMP9381498 = VDqrsthZHQYQYfNMP77520152;     VDqrsthZHQYQYfNMP77520152 = VDqrsthZHQYQYfNMP43474740;     VDqrsthZHQYQYfNMP43474740 = VDqrsthZHQYQYfNMP12233114;     VDqrsthZHQYQYfNMP12233114 = VDqrsthZHQYQYfNMP22493564;     VDqrsthZHQYQYfNMP22493564 = VDqrsthZHQYQYfNMP47463686;     VDqrsthZHQYQYfNMP47463686 = VDqrsthZHQYQYfNMP22169939;     VDqrsthZHQYQYfNMP22169939 = VDqrsthZHQYQYfNMP12431629;     VDqrsthZHQYQYfNMP12431629 = VDqrsthZHQYQYfNMP48469032;     VDqrsthZHQYQYfNMP48469032 = VDqrsthZHQYQYfNMP65872373;     VDqrsthZHQYQYfNMP65872373 = VDqrsthZHQYQYfNMP75668677;     VDqrsthZHQYQYfNMP75668677 = VDqrsthZHQYQYfNMP47303370;     VDqrsthZHQYQYfNMP47303370 = VDqrsthZHQYQYfNMP3556605;     VDqrsthZHQYQYfNMP3556605 = VDqrsthZHQYQYfNMP6387263;     VDqrsthZHQYQYfNMP6387263 = VDqrsthZHQYQYfNMP76218162;     VDqrsthZHQYQYfNMP76218162 = VDqrsthZHQYQYfNMP37083222;     VDqrsthZHQYQYfNMP37083222 = VDqrsthZHQYQYfNMP56295044;     VDqrsthZHQYQYfNMP56295044 = VDqrsthZHQYQYfNMP55647873;     VDqrsthZHQYQYfNMP55647873 = VDqrsthZHQYQYfNMP77104873;     VDqrsthZHQYQYfNMP77104873 = VDqrsthZHQYQYfNMP62871566;     VDqrsthZHQYQYfNMP62871566 = VDqrsthZHQYQYfNMP41732662;     VDqrsthZHQYQYfNMP41732662 = VDqrsthZHQYQYfNMP26384303;     VDqrsthZHQYQYfNMP26384303 = VDqrsthZHQYQYfNMP27803165;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nAfrTslVqsheowBt10262049() {     double yOiAQIpNAMIPiHnok63262133 = -620954327;    double yOiAQIpNAMIPiHnok88313753 = -725219677;    double yOiAQIpNAMIPiHnok2704612 = -727775119;    double yOiAQIpNAMIPiHnok75705845 = -266396953;    double yOiAQIpNAMIPiHnok17070948 = -838427446;    double yOiAQIpNAMIPiHnok2637044 = -262196613;    double yOiAQIpNAMIPiHnok72427674 = -926640623;    double yOiAQIpNAMIPiHnok1309394 = -951810010;    double yOiAQIpNAMIPiHnok987790 = 13088632;    double yOiAQIpNAMIPiHnok17014367 = -646518941;    double yOiAQIpNAMIPiHnok6162527 = -791930607;    double yOiAQIpNAMIPiHnok96075585 = -428433470;    double yOiAQIpNAMIPiHnok33411009 = -33788732;    double yOiAQIpNAMIPiHnok66223230 = -386129736;    double yOiAQIpNAMIPiHnok52802796 = -962214301;    double yOiAQIpNAMIPiHnok55226115 = -503046097;    double yOiAQIpNAMIPiHnok44270477 = -895717868;    double yOiAQIpNAMIPiHnok17904406 = 49302765;    double yOiAQIpNAMIPiHnok46470788 = -401706148;    double yOiAQIpNAMIPiHnok4570887 = -367151875;    double yOiAQIpNAMIPiHnok47063766 = -286778614;    double yOiAQIpNAMIPiHnok970231 = -513924603;    double yOiAQIpNAMIPiHnok63479358 = -254332102;    double yOiAQIpNAMIPiHnok11709240 = -540391755;    double yOiAQIpNAMIPiHnok80322682 = -964504101;    double yOiAQIpNAMIPiHnok78808136 = -561983867;    double yOiAQIpNAMIPiHnok2532040 = -753259111;    double yOiAQIpNAMIPiHnok22618158 = -605449547;    double yOiAQIpNAMIPiHnok36315922 = -374790490;    double yOiAQIpNAMIPiHnok99664813 = -819375906;    double yOiAQIpNAMIPiHnok11953331 = -119373137;    double yOiAQIpNAMIPiHnok23653317 = -41836704;    double yOiAQIpNAMIPiHnok89788375 = -39377895;    double yOiAQIpNAMIPiHnok14410651 = -151724678;    double yOiAQIpNAMIPiHnok43366283 = -943701148;    double yOiAQIpNAMIPiHnok37205146 = -891675097;    double yOiAQIpNAMIPiHnok3200920 = -668023745;    double yOiAQIpNAMIPiHnok58611325 = -189980504;    double yOiAQIpNAMIPiHnok60149497 = -395947128;    double yOiAQIpNAMIPiHnok44869282 = -121544505;    double yOiAQIpNAMIPiHnok68980015 = -550536109;    double yOiAQIpNAMIPiHnok16000126 = -853810070;    double yOiAQIpNAMIPiHnok57593726 = -230004720;    double yOiAQIpNAMIPiHnok90605555 = -790145014;    double yOiAQIpNAMIPiHnok8551256 = -316251220;    double yOiAQIpNAMIPiHnok40745908 = -335488282;    double yOiAQIpNAMIPiHnok43980388 = -731950249;    double yOiAQIpNAMIPiHnok47604664 = -918360480;    double yOiAQIpNAMIPiHnok82288836 = -797032160;    double yOiAQIpNAMIPiHnok66713610 = 95453154;    double yOiAQIpNAMIPiHnok52732188 = -893406644;    double yOiAQIpNAMIPiHnok64423943 = -388229580;    double yOiAQIpNAMIPiHnok5982415 = -979315603;    double yOiAQIpNAMIPiHnok27689838 = -691858408;    double yOiAQIpNAMIPiHnok63198282 = -326758887;    double yOiAQIpNAMIPiHnok62291903 = -7029724;    double yOiAQIpNAMIPiHnok24834396 = -370887576;    double yOiAQIpNAMIPiHnok90995372 = -87383365;    double yOiAQIpNAMIPiHnok95383162 = -301892853;    double yOiAQIpNAMIPiHnok38262812 = -176443579;    double yOiAQIpNAMIPiHnok105004 = -508937502;    double yOiAQIpNAMIPiHnok49809516 = -221191077;    double yOiAQIpNAMIPiHnok64993472 = -477019521;    double yOiAQIpNAMIPiHnok1322976 = -167535463;    double yOiAQIpNAMIPiHnok5061036 = -427145805;    double yOiAQIpNAMIPiHnok82509209 = -650093903;    double yOiAQIpNAMIPiHnok6287210 = -289055576;    double yOiAQIpNAMIPiHnok19000359 = -882064055;    double yOiAQIpNAMIPiHnok22856948 = -442428588;    double yOiAQIpNAMIPiHnok15597650 = 29460795;    double yOiAQIpNAMIPiHnok52025196 = -835022352;    double yOiAQIpNAMIPiHnok85659152 = -605737364;    double yOiAQIpNAMIPiHnok57754908 = -554750107;    double yOiAQIpNAMIPiHnok1601507 = -180161644;    double yOiAQIpNAMIPiHnok35590871 = -816615767;    double yOiAQIpNAMIPiHnok31063641 = -432968544;    double yOiAQIpNAMIPiHnok43376504 = -183919884;    double yOiAQIpNAMIPiHnok72873802 = -464187088;    double yOiAQIpNAMIPiHnok3157984 = -124140536;    double yOiAQIpNAMIPiHnok39576775 = -529015819;    double yOiAQIpNAMIPiHnok34827748 = -830033619;    double yOiAQIpNAMIPiHnok54927376 = -834898632;    double yOiAQIpNAMIPiHnok40329322 = -808417388;    double yOiAQIpNAMIPiHnok69602311 = -370243645;    double yOiAQIpNAMIPiHnok46932626 = -925969262;    double yOiAQIpNAMIPiHnok47529388 = -731143557;    double yOiAQIpNAMIPiHnok17670902 = -62521102;    double yOiAQIpNAMIPiHnok62098537 = -347519487;    double yOiAQIpNAMIPiHnok51212368 = -824965791;    double yOiAQIpNAMIPiHnok81074379 = -836671425;    double yOiAQIpNAMIPiHnok12370750 = -420787522;    double yOiAQIpNAMIPiHnok12205548 = -480640381;    double yOiAQIpNAMIPiHnok63228162 = -888087652;    double yOiAQIpNAMIPiHnok21886686 = -119503549;    double yOiAQIpNAMIPiHnok44764278 = -612607004;    double yOiAQIpNAMIPiHnok19170499 = -229345033;    double yOiAQIpNAMIPiHnok51006653 = -276790550;    double yOiAQIpNAMIPiHnok56270750 = 37530743;    double yOiAQIpNAMIPiHnok85544519 = -262999210;    double yOiAQIpNAMIPiHnok26042046 = -620954327;     yOiAQIpNAMIPiHnok63262133 = yOiAQIpNAMIPiHnok88313753;     yOiAQIpNAMIPiHnok88313753 = yOiAQIpNAMIPiHnok2704612;     yOiAQIpNAMIPiHnok2704612 = yOiAQIpNAMIPiHnok75705845;     yOiAQIpNAMIPiHnok75705845 = yOiAQIpNAMIPiHnok17070948;     yOiAQIpNAMIPiHnok17070948 = yOiAQIpNAMIPiHnok2637044;     yOiAQIpNAMIPiHnok2637044 = yOiAQIpNAMIPiHnok72427674;     yOiAQIpNAMIPiHnok72427674 = yOiAQIpNAMIPiHnok1309394;     yOiAQIpNAMIPiHnok1309394 = yOiAQIpNAMIPiHnok987790;     yOiAQIpNAMIPiHnok987790 = yOiAQIpNAMIPiHnok17014367;     yOiAQIpNAMIPiHnok17014367 = yOiAQIpNAMIPiHnok6162527;     yOiAQIpNAMIPiHnok6162527 = yOiAQIpNAMIPiHnok96075585;     yOiAQIpNAMIPiHnok96075585 = yOiAQIpNAMIPiHnok33411009;     yOiAQIpNAMIPiHnok33411009 = yOiAQIpNAMIPiHnok66223230;     yOiAQIpNAMIPiHnok66223230 = yOiAQIpNAMIPiHnok52802796;     yOiAQIpNAMIPiHnok52802796 = yOiAQIpNAMIPiHnok55226115;     yOiAQIpNAMIPiHnok55226115 = yOiAQIpNAMIPiHnok44270477;     yOiAQIpNAMIPiHnok44270477 = yOiAQIpNAMIPiHnok17904406;     yOiAQIpNAMIPiHnok17904406 = yOiAQIpNAMIPiHnok46470788;     yOiAQIpNAMIPiHnok46470788 = yOiAQIpNAMIPiHnok4570887;     yOiAQIpNAMIPiHnok4570887 = yOiAQIpNAMIPiHnok47063766;     yOiAQIpNAMIPiHnok47063766 = yOiAQIpNAMIPiHnok970231;     yOiAQIpNAMIPiHnok970231 = yOiAQIpNAMIPiHnok63479358;     yOiAQIpNAMIPiHnok63479358 = yOiAQIpNAMIPiHnok11709240;     yOiAQIpNAMIPiHnok11709240 = yOiAQIpNAMIPiHnok80322682;     yOiAQIpNAMIPiHnok80322682 = yOiAQIpNAMIPiHnok78808136;     yOiAQIpNAMIPiHnok78808136 = yOiAQIpNAMIPiHnok2532040;     yOiAQIpNAMIPiHnok2532040 = yOiAQIpNAMIPiHnok22618158;     yOiAQIpNAMIPiHnok22618158 = yOiAQIpNAMIPiHnok36315922;     yOiAQIpNAMIPiHnok36315922 = yOiAQIpNAMIPiHnok99664813;     yOiAQIpNAMIPiHnok99664813 = yOiAQIpNAMIPiHnok11953331;     yOiAQIpNAMIPiHnok11953331 = yOiAQIpNAMIPiHnok23653317;     yOiAQIpNAMIPiHnok23653317 = yOiAQIpNAMIPiHnok89788375;     yOiAQIpNAMIPiHnok89788375 = yOiAQIpNAMIPiHnok14410651;     yOiAQIpNAMIPiHnok14410651 = yOiAQIpNAMIPiHnok43366283;     yOiAQIpNAMIPiHnok43366283 = yOiAQIpNAMIPiHnok37205146;     yOiAQIpNAMIPiHnok37205146 = yOiAQIpNAMIPiHnok3200920;     yOiAQIpNAMIPiHnok3200920 = yOiAQIpNAMIPiHnok58611325;     yOiAQIpNAMIPiHnok58611325 = yOiAQIpNAMIPiHnok60149497;     yOiAQIpNAMIPiHnok60149497 = yOiAQIpNAMIPiHnok44869282;     yOiAQIpNAMIPiHnok44869282 = yOiAQIpNAMIPiHnok68980015;     yOiAQIpNAMIPiHnok68980015 = yOiAQIpNAMIPiHnok16000126;     yOiAQIpNAMIPiHnok16000126 = yOiAQIpNAMIPiHnok57593726;     yOiAQIpNAMIPiHnok57593726 = yOiAQIpNAMIPiHnok90605555;     yOiAQIpNAMIPiHnok90605555 = yOiAQIpNAMIPiHnok8551256;     yOiAQIpNAMIPiHnok8551256 = yOiAQIpNAMIPiHnok40745908;     yOiAQIpNAMIPiHnok40745908 = yOiAQIpNAMIPiHnok43980388;     yOiAQIpNAMIPiHnok43980388 = yOiAQIpNAMIPiHnok47604664;     yOiAQIpNAMIPiHnok47604664 = yOiAQIpNAMIPiHnok82288836;     yOiAQIpNAMIPiHnok82288836 = yOiAQIpNAMIPiHnok66713610;     yOiAQIpNAMIPiHnok66713610 = yOiAQIpNAMIPiHnok52732188;     yOiAQIpNAMIPiHnok52732188 = yOiAQIpNAMIPiHnok64423943;     yOiAQIpNAMIPiHnok64423943 = yOiAQIpNAMIPiHnok5982415;     yOiAQIpNAMIPiHnok5982415 = yOiAQIpNAMIPiHnok27689838;     yOiAQIpNAMIPiHnok27689838 = yOiAQIpNAMIPiHnok63198282;     yOiAQIpNAMIPiHnok63198282 = yOiAQIpNAMIPiHnok62291903;     yOiAQIpNAMIPiHnok62291903 = yOiAQIpNAMIPiHnok24834396;     yOiAQIpNAMIPiHnok24834396 = yOiAQIpNAMIPiHnok90995372;     yOiAQIpNAMIPiHnok90995372 = yOiAQIpNAMIPiHnok95383162;     yOiAQIpNAMIPiHnok95383162 = yOiAQIpNAMIPiHnok38262812;     yOiAQIpNAMIPiHnok38262812 = yOiAQIpNAMIPiHnok105004;     yOiAQIpNAMIPiHnok105004 = yOiAQIpNAMIPiHnok49809516;     yOiAQIpNAMIPiHnok49809516 = yOiAQIpNAMIPiHnok64993472;     yOiAQIpNAMIPiHnok64993472 = yOiAQIpNAMIPiHnok1322976;     yOiAQIpNAMIPiHnok1322976 = yOiAQIpNAMIPiHnok5061036;     yOiAQIpNAMIPiHnok5061036 = yOiAQIpNAMIPiHnok82509209;     yOiAQIpNAMIPiHnok82509209 = yOiAQIpNAMIPiHnok6287210;     yOiAQIpNAMIPiHnok6287210 = yOiAQIpNAMIPiHnok19000359;     yOiAQIpNAMIPiHnok19000359 = yOiAQIpNAMIPiHnok22856948;     yOiAQIpNAMIPiHnok22856948 = yOiAQIpNAMIPiHnok15597650;     yOiAQIpNAMIPiHnok15597650 = yOiAQIpNAMIPiHnok52025196;     yOiAQIpNAMIPiHnok52025196 = yOiAQIpNAMIPiHnok85659152;     yOiAQIpNAMIPiHnok85659152 = yOiAQIpNAMIPiHnok57754908;     yOiAQIpNAMIPiHnok57754908 = yOiAQIpNAMIPiHnok1601507;     yOiAQIpNAMIPiHnok1601507 = yOiAQIpNAMIPiHnok35590871;     yOiAQIpNAMIPiHnok35590871 = yOiAQIpNAMIPiHnok31063641;     yOiAQIpNAMIPiHnok31063641 = yOiAQIpNAMIPiHnok43376504;     yOiAQIpNAMIPiHnok43376504 = yOiAQIpNAMIPiHnok72873802;     yOiAQIpNAMIPiHnok72873802 = yOiAQIpNAMIPiHnok3157984;     yOiAQIpNAMIPiHnok3157984 = yOiAQIpNAMIPiHnok39576775;     yOiAQIpNAMIPiHnok39576775 = yOiAQIpNAMIPiHnok34827748;     yOiAQIpNAMIPiHnok34827748 = yOiAQIpNAMIPiHnok54927376;     yOiAQIpNAMIPiHnok54927376 = yOiAQIpNAMIPiHnok40329322;     yOiAQIpNAMIPiHnok40329322 = yOiAQIpNAMIPiHnok69602311;     yOiAQIpNAMIPiHnok69602311 = yOiAQIpNAMIPiHnok46932626;     yOiAQIpNAMIPiHnok46932626 = yOiAQIpNAMIPiHnok47529388;     yOiAQIpNAMIPiHnok47529388 = yOiAQIpNAMIPiHnok17670902;     yOiAQIpNAMIPiHnok17670902 = yOiAQIpNAMIPiHnok62098537;     yOiAQIpNAMIPiHnok62098537 = yOiAQIpNAMIPiHnok51212368;     yOiAQIpNAMIPiHnok51212368 = yOiAQIpNAMIPiHnok81074379;     yOiAQIpNAMIPiHnok81074379 = yOiAQIpNAMIPiHnok12370750;     yOiAQIpNAMIPiHnok12370750 = yOiAQIpNAMIPiHnok12205548;     yOiAQIpNAMIPiHnok12205548 = yOiAQIpNAMIPiHnok63228162;     yOiAQIpNAMIPiHnok63228162 = yOiAQIpNAMIPiHnok21886686;     yOiAQIpNAMIPiHnok21886686 = yOiAQIpNAMIPiHnok44764278;     yOiAQIpNAMIPiHnok44764278 = yOiAQIpNAMIPiHnok19170499;     yOiAQIpNAMIPiHnok19170499 = yOiAQIpNAMIPiHnok51006653;     yOiAQIpNAMIPiHnok51006653 = yOiAQIpNAMIPiHnok56270750;     yOiAQIpNAMIPiHnok56270750 = yOiAQIpNAMIPiHnok85544519;     yOiAQIpNAMIPiHnok85544519 = yOiAQIpNAMIPiHnok26042046;     yOiAQIpNAMIPiHnok26042046 = yOiAQIpNAMIPiHnok63262133;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VOUQIZBfBhqXsDbc25311117() {     double pcgBfArGiSVtmrjWx69379240 = -732856216;    double pcgBfArGiSVtmrjWx31687033 = -434819457;    double pcgBfArGiSVtmrjWx18746795 = -878608041;    double pcgBfArGiSVtmrjWx2780627 = -457145583;    double pcgBfArGiSVtmrjWx85966436 = -259077317;    double pcgBfArGiSVtmrjWx94054245 = -526692848;    double pcgBfArGiSVtmrjWx32916640 = 34394400;    double pcgBfArGiSVtmrjWx51069284 = -138683924;    double pcgBfArGiSVtmrjWx65630284 = -854944034;    double pcgBfArGiSVtmrjWx58244026 = -975494419;    double pcgBfArGiSVtmrjWx66272816 = -669368193;    double pcgBfArGiSVtmrjWx27898628 = -872839672;    double pcgBfArGiSVtmrjWx19620128 = -473679821;    double pcgBfArGiSVtmrjWx49366907 = -918368304;    double pcgBfArGiSVtmrjWx48545222 = -552676468;    double pcgBfArGiSVtmrjWx96731508 = -476045640;    double pcgBfArGiSVtmrjWx38106481 = -412262981;    double pcgBfArGiSVtmrjWx14957559 = -716219878;    double pcgBfArGiSVtmrjWx8300630 = -807475245;    double pcgBfArGiSVtmrjWx85121051 = -724054794;    double pcgBfArGiSVtmrjWx35616959 = -142266069;    double pcgBfArGiSVtmrjWx87491253 = -421854320;    double pcgBfArGiSVtmrjWx99070149 = -849589;    double pcgBfArGiSVtmrjWx95701613 = -762696743;    double pcgBfArGiSVtmrjWx24022333 = -237700134;    double pcgBfArGiSVtmrjWx3529812 = -440639190;    double pcgBfArGiSVtmrjWx991818 = -653921314;    double pcgBfArGiSVtmrjWx35725284 = -671378500;    double pcgBfArGiSVtmrjWx2448177 = -597173195;    double pcgBfArGiSVtmrjWx68280450 = -636846773;    double pcgBfArGiSVtmrjWx4195881 = -32694075;    double pcgBfArGiSVtmrjWx21263938 = -851283520;    double pcgBfArGiSVtmrjWx99499323 = 68008735;    double pcgBfArGiSVtmrjWx9241215 = 96733557;    double pcgBfArGiSVtmrjWx4840936 = -482434254;    double pcgBfArGiSVtmrjWx5931059 = -5701205;    double pcgBfArGiSVtmrjWx84462854 = -621659615;    double pcgBfArGiSVtmrjWx92701537 = 38513486;    double pcgBfArGiSVtmrjWx33759751 = -389870625;    double pcgBfArGiSVtmrjWx32371220 = -594983728;    double pcgBfArGiSVtmrjWx74649239 = -615810003;    double pcgBfArGiSVtmrjWx48150582 = -23952348;    double pcgBfArGiSVtmrjWx35660704 = -310809304;    double pcgBfArGiSVtmrjWx78013296 = -809487952;    double pcgBfArGiSVtmrjWx20503721 = 21850626;    double pcgBfArGiSVtmrjWx52685103 = -320704398;    double pcgBfArGiSVtmrjWx61758962 = -850725786;    double pcgBfArGiSVtmrjWx55561605 = -928256012;    double pcgBfArGiSVtmrjWx91964565 = -882769148;    double pcgBfArGiSVtmrjWx79625262 = 8442346;    double pcgBfArGiSVtmrjWx88432865 = -735700315;    double pcgBfArGiSVtmrjWx75233662 = -188955900;    double pcgBfArGiSVtmrjWx54230872 = -771105191;    double pcgBfArGiSVtmrjWx23658718 = 87671447;    double pcgBfArGiSVtmrjWx89694157 = 72709331;    double pcgBfArGiSVtmrjWx81887986 = -211001896;    double pcgBfArGiSVtmrjWx32616884 = -333969869;    double pcgBfArGiSVtmrjWx23045182 = -15911298;    double pcgBfArGiSVtmrjWx78758293 = -119445450;    double pcgBfArGiSVtmrjWx82436624 = -818438127;    double pcgBfArGiSVtmrjWx93062427 = -872771534;    double pcgBfArGiSVtmrjWx97191356 = -294227100;    double pcgBfArGiSVtmrjWx48621107 = -541510729;    double pcgBfArGiSVtmrjWx97349834 = -118097261;    double pcgBfArGiSVtmrjWx54048146 = -842800344;    double pcgBfArGiSVtmrjWx45008878 = -818084674;    double pcgBfArGiSVtmrjWx28399304 = -840848407;    double pcgBfArGiSVtmrjWx10378913 = -470413378;    double pcgBfArGiSVtmrjWx44525972 = -335934051;    double pcgBfArGiSVtmrjWx42614163 = -446975264;    double pcgBfArGiSVtmrjWx12268655 = -854386026;    double pcgBfArGiSVtmrjWx45404943 = -350776467;    double pcgBfArGiSVtmrjWx81197807 = -226349253;    double pcgBfArGiSVtmrjWx75929410 = -112491518;    double pcgBfArGiSVtmrjWx10471812 = -8244792;    double pcgBfArGiSVtmrjWx87466377 = -18313722;    double pcgBfArGiSVtmrjWx51830550 = -11045016;    double pcgBfArGiSVtmrjWx21056854 = -191361637;    double pcgBfArGiSVtmrjWx75197892 = -684547369;    double pcgBfArGiSVtmrjWx71337230 = -916995736;    double pcgBfArGiSVtmrjWx41770849 = -589913404;    double pcgBfArGiSVtmrjWx45430213 = -725665302;    double pcgBfArGiSVtmrjWx43760719 = -788609353;    double pcgBfArGiSVtmrjWx22822915 = -505615542;    double pcgBfArGiSVtmrjWx79847584 = -901146458;    double pcgBfArGiSVtmrjWx28962218 = -843738175;    double pcgBfArGiSVtmrjWx67033066 = 19821671;    double pcgBfArGiSVtmrjWx75840605 = 80337288;    double pcgBfArGiSVtmrjWx19547058 = -975975774;    double pcgBfArGiSVtmrjWx22952949 = -171432358;    double pcgBfArGiSVtmrjWx73314175 = -671731336;    double pcgBfArGiSVtmrjWx61417672 = -505748317;    double pcgBfArGiSVtmrjWx13943244 = -842041064;    double pcgBfArGiSVtmrjWx51323126 = -571432498;    double pcgBfArGiSVtmrjWx39308792 = -722212194;    double pcgBfArGiSVtmrjWx77457882 = -221582903;    double pcgBfArGiSVtmrjWx99529474 = -482441619;    double pcgBfArGiSVtmrjWx38310870 = -92712043;    double pcgBfArGiSVtmrjWx23965151 = -966687609;    double pcgBfArGiSVtmrjWx75494842 = -732856216;     pcgBfArGiSVtmrjWx69379240 = pcgBfArGiSVtmrjWx31687033;     pcgBfArGiSVtmrjWx31687033 = pcgBfArGiSVtmrjWx18746795;     pcgBfArGiSVtmrjWx18746795 = pcgBfArGiSVtmrjWx2780627;     pcgBfArGiSVtmrjWx2780627 = pcgBfArGiSVtmrjWx85966436;     pcgBfArGiSVtmrjWx85966436 = pcgBfArGiSVtmrjWx94054245;     pcgBfArGiSVtmrjWx94054245 = pcgBfArGiSVtmrjWx32916640;     pcgBfArGiSVtmrjWx32916640 = pcgBfArGiSVtmrjWx51069284;     pcgBfArGiSVtmrjWx51069284 = pcgBfArGiSVtmrjWx65630284;     pcgBfArGiSVtmrjWx65630284 = pcgBfArGiSVtmrjWx58244026;     pcgBfArGiSVtmrjWx58244026 = pcgBfArGiSVtmrjWx66272816;     pcgBfArGiSVtmrjWx66272816 = pcgBfArGiSVtmrjWx27898628;     pcgBfArGiSVtmrjWx27898628 = pcgBfArGiSVtmrjWx19620128;     pcgBfArGiSVtmrjWx19620128 = pcgBfArGiSVtmrjWx49366907;     pcgBfArGiSVtmrjWx49366907 = pcgBfArGiSVtmrjWx48545222;     pcgBfArGiSVtmrjWx48545222 = pcgBfArGiSVtmrjWx96731508;     pcgBfArGiSVtmrjWx96731508 = pcgBfArGiSVtmrjWx38106481;     pcgBfArGiSVtmrjWx38106481 = pcgBfArGiSVtmrjWx14957559;     pcgBfArGiSVtmrjWx14957559 = pcgBfArGiSVtmrjWx8300630;     pcgBfArGiSVtmrjWx8300630 = pcgBfArGiSVtmrjWx85121051;     pcgBfArGiSVtmrjWx85121051 = pcgBfArGiSVtmrjWx35616959;     pcgBfArGiSVtmrjWx35616959 = pcgBfArGiSVtmrjWx87491253;     pcgBfArGiSVtmrjWx87491253 = pcgBfArGiSVtmrjWx99070149;     pcgBfArGiSVtmrjWx99070149 = pcgBfArGiSVtmrjWx95701613;     pcgBfArGiSVtmrjWx95701613 = pcgBfArGiSVtmrjWx24022333;     pcgBfArGiSVtmrjWx24022333 = pcgBfArGiSVtmrjWx3529812;     pcgBfArGiSVtmrjWx3529812 = pcgBfArGiSVtmrjWx991818;     pcgBfArGiSVtmrjWx991818 = pcgBfArGiSVtmrjWx35725284;     pcgBfArGiSVtmrjWx35725284 = pcgBfArGiSVtmrjWx2448177;     pcgBfArGiSVtmrjWx2448177 = pcgBfArGiSVtmrjWx68280450;     pcgBfArGiSVtmrjWx68280450 = pcgBfArGiSVtmrjWx4195881;     pcgBfArGiSVtmrjWx4195881 = pcgBfArGiSVtmrjWx21263938;     pcgBfArGiSVtmrjWx21263938 = pcgBfArGiSVtmrjWx99499323;     pcgBfArGiSVtmrjWx99499323 = pcgBfArGiSVtmrjWx9241215;     pcgBfArGiSVtmrjWx9241215 = pcgBfArGiSVtmrjWx4840936;     pcgBfArGiSVtmrjWx4840936 = pcgBfArGiSVtmrjWx5931059;     pcgBfArGiSVtmrjWx5931059 = pcgBfArGiSVtmrjWx84462854;     pcgBfArGiSVtmrjWx84462854 = pcgBfArGiSVtmrjWx92701537;     pcgBfArGiSVtmrjWx92701537 = pcgBfArGiSVtmrjWx33759751;     pcgBfArGiSVtmrjWx33759751 = pcgBfArGiSVtmrjWx32371220;     pcgBfArGiSVtmrjWx32371220 = pcgBfArGiSVtmrjWx74649239;     pcgBfArGiSVtmrjWx74649239 = pcgBfArGiSVtmrjWx48150582;     pcgBfArGiSVtmrjWx48150582 = pcgBfArGiSVtmrjWx35660704;     pcgBfArGiSVtmrjWx35660704 = pcgBfArGiSVtmrjWx78013296;     pcgBfArGiSVtmrjWx78013296 = pcgBfArGiSVtmrjWx20503721;     pcgBfArGiSVtmrjWx20503721 = pcgBfArGiSVtmrjWx52685103;     pcgBfArGiSVtmrjWx52685103 = pcgBfArGiSVtmrjWx61758962;     pcgBfArGiSVtmrjWx61758962 = pcgBfArGiSVtmrjWx55561605;     pcgBfArGiSVtmrjWx55561605 = pcgBfArGiSVtmrjWx91964565;     pcgBfArGiSVtmrjWx91964565 = pcgBfArGiSVtmrjWx79625262;     pcgBfArGiSVtmrjWx79625262 = pcgBfArGiSVtmrjWx88432865;     pcgBfArGiSVtmrjWx88432865 = pcgBfArGiSVtmrjWx75233662;     pcgBfArGiSVtmrjWx75233662 = pcgBfArGiSVtmrjWx54230872;     pcgBfArGiSVtmrjWx54230872 = pcgBfArGiSVtmrjWx23658718;     pcgBfArGiSVtmrjWx23658718 = pcgBfArGiSVtmrjWx89694157;     pcgBfArGiSVtmrjWx89694157 = pcgBfArGiSVtmrjWx81887986;     pcgBfArGiSVtmrjWx81887986 = pcgBfArGiSVtmrjWx32616884;     pcgBfArGiSVtmrjWx32616884 = pcgBfArGiSVtmrjWx23045182;     pcgBfArGiSVtmrjWx23045182 = pcgBfArGiSVtmrjWx78758293;     pcgBfArGiSVtmrjWx78758293 = pcgBfArGiSVtmrjWx82436624;     pcgBfArGiSVtmrjWx82436624 = pcgBfArGiSVtmrjWx93062427;     pcgBfArGiSVtmrjWx93062427 = pcgBfArGiSVtmrjWx97191356;     pcgBfArGiSVtmrjWx97191356 = pcgBfArGiSVtmrjWx48621107;     pcgBfArGiSVtmrjWx48621107 = pcgBfArGiSVtmrjWx97349834;     pcgBfArGiSVtmrjWx97349834 = pcgBfArGiSVtmrjWx54048146;     pcgBfArGiSVtmrjWx54048146 = pcgBfArGiSVtmrjWx45008878;     pcgBfArGiSVtmrjWx45008878 = pcgBfArGiSVtmrjWx28399304;     pcgBfArGiSVtmrjWx28399304 = pcgBfArGiSVtmrjWx10378913;     pcgBfArGiSVtmrjWx10378913 = pcgBfArGiSVtmrjWx44525972;     pcgBfArGiSVtmrjWx44525972 = pcgBfArGiSVtmrjWx42614163;     pcgBfArGiSVtmrjWx42614163 = pcgBfArGiSVtmrjWx12268655;     pcgBfArGiSVtmrjWx12268655 = pcgBfArGiSVtmrjWx45404943;     pcgBfArGiSVtmrjWx45404943 = pcgBfArGiSVtmrjWx81197807;     pcgBfArGiSVtmrjWx81197807 = pcgBfArGiSVtmrjWx75929410;     pcgBfArGiSVtmrjWx75929410 = pcgBfArGiSVtmrjWx10471812;     pcgBfArGiSVtmrjWx10471812 = pcgBfArGiSVtmrjWx87466377;     pcgBfArGiSVtmrjWx87466377 = pcgBfArGiSVtmrjWx51830550;     pcgBfArGiSVtmrjWx51830550 = pcgBfArGiSVtmrjWx21056854;     pcgBfArGiSVtmrjWx21056854 = pcgBfArGiSVtmrjWx75197892;     pcgBfArGiSVtmrjWx75197892 = pcgBfArGiSVtmrjWx71337230;     pcgBfArGiSVtmrjWx71337230 = pcgBfArGiSVtmrjWx41770849;     pcgBfArGiSVtmrjWx41770849 = pcgBfArGiSVtmrjWx45430213;     pcgBfArGiSVtmrjWx45430213 = pcgBfArGiSVtmrjWx43760719;     pcgBfArGiSVtmrjWx43760719 = pcgBfArGiSVtmrjWx22822915;     pcgBfArGiSVtmrjWx22822915 = pcgBfArGiSVtmrjWx79847584;     pcgBfArGiSVtmrjWx79847584 = pcgBfArGiSVtmrjWx28962218;     pcgBfArGiSVtmrjWx28962218 = pcgBfArGiSVtmrjWx67033066;     pcgBfArGiSVtmrjWx67033066 = pcgBfArGiSVtmrjWx75840605;     pcgBfArGiSVtmrjWx75840605 = pcgBfArGiSVtmrjWx19547058;     pcgBfArGiSVtmrjWx19547058 = pcgBfArGiSVtmrjWx22952949;     pcgBfArGiSVtmrjWx22952949 = pcgBfArGiSVtmrjWx73314175;     pcgBfArGiSVtmrjWx73314175 = pcgBfArGiSVtmrjWx61417672;     pcgBfArGiSVtmrjWx61417672 = pcgBfArGiSVtmrjWx13943244;     pcgBfArGiSVtmrjWx13943244 = pcgBfArGiSVtmrjWx51323126;     pcgBfArGiSVtmrjWx51323126 = pcgBfArGiSVtmrjWx39308792;     pcgBfArGiSVtmrjWx39308792 = pcgBfArGiSVtmrjWx77457882;     pcgBfArGiSVtmrjWx77457882 = pcgBfArGiSVtmrjWx99529474;     pcgBfArGiSVtmrjWx99529474 = pcgBfArGiSVtmrjWx38310870;     pcgBfArGiSVtmrjWx38310870 = pcgBfArGiSVtmrjWx23965151;     pcgBfArGiSVtmrjWx23965151 = pcgBfArGiSVtmrjWx75494842;     pcgBfArGiSVtmrjWx75494842 = pcgBfArGiSVtmrjWx69379240;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nPjulDxNlKlPYKvJ92602715() {     double jngiIkqXiKpsJlyJr4838209 = -99133115;    double jngiIkqXiKpsJlyJr44959112 = -315371064;    double jngiIkqXiKpsJlyJr56133271 = -544275116;    double jngiIkqXiKpsJlyJr75003981 = -278296655;    double jngiIkqXiKpsJlyJr42251705 = -364676162;    double jngiIkqXiKpsJlyJr30277118 = -789040743;    double jngiIkqXiKpsJlyJr43574046 = -910331334;    double jngiIkqXiKpsJlyJr13200846 = -882918064;    double jngiIkqXiKpsJlyJr67147890 = -243275441;    double jngiIkqXiKpsJlyJr73146738 = -640181417;    double jngiIkqXiKpsJlyJr4454552 = -471375922;    double jngiIkqXiKpsJlyJr28466776 = -736960443;    double jngiIkqXiKpsJlyJr57854773 = 74307674;    double jngiIkqXiKpsJlyJr2507771 = -894731713;    double jngiIkqXiKpsJlyJr6739739 = 43898517;    double jngiIkqXiKpsJlyJr80272809 = -111857247;    double jngiIkqXiKpsJlyJr93942416 = -939237533;    double jngiIkqXiKpsJlyJr14717750 = -789647946;    double jngiIkqXiKpsJlyJr8850960 = -24038667;    double jngiIkqXiKpsJlyJr78958269 = -50615860;    double jngiIkqXiKpsJlyJr48703822 = -899540758;    double jngiIkqXiKpsJlyJr90912778 = -508490614;    double jngiIkqXiKpsJlyJr69595529 = -738481773;    double jngiIkqXiKpsJlyJr89494558 = -173521347;    double jngiIkqXiKpsJlyJr32040284 = -261685332;    double jngiIkqXiKpsJlyJr14227983 = -129215997;    double jngiIkqXiKpsJlyJr61412392 = -460205455;    double jngiIkqXiKpsJlyJr76453708 = -841150675;    double jngiIkqXiKpsJlyJr49116392 = -71267997;    double jngiIkqXiKpsJlyJr73813861 = -671353944;    double jngiIkqXiKpsJlyJr22779112 = -584674562;    double jngiIkqXiKpsJlyJr59086956 = -624254934;    double jngiIkqXiKpsJlyJr11717335 = -79587243;    double jngiIkqXiKpsJlyJr38989641 = 32039744;    double jngiIkqXiKpsJlyJr70649389 = -433534504;    double jngiIkqXiKpsJlyJr57491903 = -220266511;    double jngiIkqXiKpsJlyJr33874671 = -486355655;    double jngiIkqXiKpsJlyJr43916941 = 55857905;    double jngiIkqXiKpsJlyJr64150312 = -40746902;    double jngiIkqXiKpsJlyJr96642751 = -60246274;    double jngiIkqXiKpsJlyJr8100847 = -385008917;    double jngiIkqXiKpsJlyJr37515709 = -718497639;    double jngiIkqXiKpsJlyJr25044083 = -963990090;    double jngiIkqXiKpsJlyJr18144635 = -313820362;    double jngiIkqXiKpsJlyJr20520179 = -904660536;    double jngiIkqXiKpsJlyJr98646431 = -359628031;    double jngiIkqXiKpsJlyJr81104126 = -234712147;    double jngiIkqXiKpsJlyJr73287916 = 5217394;    double jngiIkqXiKpsJlyJr14857231 = -546506028;    double jngiIkqXiKpsJlyJr4154852 = -482584469;    double jngiIkqXiKpsJlyJr69203589 = -609976873;    double jngiIkqXiKpsJlyJr58719135 = -402608247;    double jngiIkqXiKpsJlyJr22852021 = -527150745;    double jngiIkqXiKpsJlyJr39650565 = -103837053;    double jngiIkqXiKpsJlyJr43898894 = -273780815;    double jngiIkqXiKpsJlyJr13925431 = -590642502;    double jngiIkqXiKpsJlyJr75363582 = -576889292;    double jngiIkqXiKpsJlyJr66638713 = -270753769;    double jngiIkqXiKpsJlyJr42963697 = 83388677;    double jngiIkqXiKpsJlyJr28023722 = -135460166;    double jngiIkqXiKpsJlyJr68864726 = -228835289;    double jngiIkqXiKpsJlyJr67120338 = 30819340;    double jngiIkqXiKpsJlyJr64084454 = -711650067;    double jngiIkqXiKpsJlyJr93334028 = -571921498;    double jngiIkqXiKpsJlyJr50367626 = 44493145;    double jngiIkqXiKpsJlyJr45367595 = -847120988;    double jngiIkqXiKpsJlyJr16749442 = -557373201;    double jngiIkqXiKpsJlyJr18865132 = -957732070;    double jngiIkqXiKpsJlyJr31858382 = -361197209;    double jngiIkqXiKpsJlyJr49247836 = -735834973;    double jngiIkqXiKpsJlyJr46398139 = -625501592;    double jngiIkqXiKpsJlyJr50025475 = -895095439;    double jngiIkqXiKpsJlyJr50567438 = -648901045;    double jngiIkqXiKpsJlyJr12208208 = -963792393;    double jngiIkqXiKpsJlyJr70857423 = -665606944;    double jngiIkqXiKpsJlyJr11188114 = -81043119;    double jngiIkqXiKpsJlyJr65868696 = -544500524;    double jngiIkqXiKpsJlyJr51450895 = -324661411;    double jngiIkqXiKpsJlyJr68974379 = -268860812;    double jngiIkqXiKpsJlyJr33393853 = -902057302;    double jngiIkqXiKpsJlyJr33123857 = -894503851;    double jngiIkqXiKpsJlyJr88124475 = -365422850;    double jngiIkqXiKpsJlyJr61596477 = -194644647;    double jngiIkqXiKpsJlyJr44961540 = -588683528;    double jngiIkqXiKpsJlyJr4610272 = 38622929;    double jngiIkqXiKpsJlyJr64059977 = -82066316;    double jngiIkqXiKpsJlyJr36234936 = 2895810;    double jngiIkqXiKpsJlyJr72066770 = -975750190;    double jngiIkqXiKpsJlyJr95090747 = -694179442;    double jngiIkqXiKpsJlyJr56723958 = -842892003;    double jngiIkqXiKpsJlyJr82128320 = -643377219;    double jngiIkqXiKpsJlyJr67235957 = -115601887;    double jngiIkqXiKpsJlyJr953244 = 72469228;    double jngiIkqXiKpsJlyJr36126590 = -905286736;    double jngiIkqXiKpsJlyJr27778026 = -831410986;    double jngiIkqXiKpsJlyJr40980509 = -315828258;    double jngiIkqXiKpsJlyJr73431255 = 93152428;    double jngiIkqXiKpsJlyJr31710054 = -292068593;    double jngiIkqXiKpsJlyJr67777008 = -258313508;    double jngiIkqXiKpsJlyJr75152584 = -99133115;     jngiIkqXiKpsJlyJr4838209 = jngiIkqXiKpsJlyJr44959112;     jngiIkqXiKpsJlyJr44959112 = jngiIkqXiKpsJlyJr56133271;     jngiIkqXiKpsJlyJr56133271 = jngiIkqXiKpsJlyJr75003981;     jngiIkqXiKpsJlyJr75003981 = jngiIkqXiKpsJlyJr42251705;     jngiIkqXiKpsJlyJr42251705 = jngiIkqXiKpsJlyJr30277118;     jngiIkqXiKpsJlyJr30277118 = jngiIkqXiKpsJlyJr43574046;     jngiIkqXiKpsJlyJr43574046 = jngiIkqXiKpsJlyJr13200846;     jngiIkqXiKpsJlyJr13200846 = jngiIkqXiKpsJlyJr67147890;     jngiIkqXiKpsJlyJr67147890 = jngiIkqXiKpsJlyJr73146738;     jngiIkqXiKpsJlyJr73146738 = jngiIkqXiKpsJlyJr4454552;     jngiIkqXiKpsJlyJr4454552 = jngiIkqXiKpsJlyJr28466776;     jngiIkqXiKpsJlyJr28466776 = jngiIkqXiKpsJlyJr57854773;     jngiIkqXiKpsJlyJr57854773 = jngiIkqXiKpsJlyJr2507771;     jngiIkqXiKpsJlyJr2507771 = jngiIkqXiKpsJlyJr6739739;     jngiIkqXiKpsJlyJr6739739 = jngiIkqXiKpsJlyJr80272809;     jngiIkqXiKpsJlyJr80272809 = jngiIkqXiKpsJlyJr93942416;     jngiIkqXiKpsJlyJr93942416 = jngiIkqXiKpsJlyJr14717750;     jngiIkqXiKpsJlyJr14717750 = jngiIkqXiKpsJlyJr8850960;     jngiIkqXiKpsJlyJr8850960 = jngiIkqXiKpsJlyJr78958269;     jngiIkqXiKpsJlyJr78958269 = jngiIkqXiKpsJlyJr48703822;     jngiIkqXiKpsJlyJr48703822 = jngiIkqXiKpsJlyJr90912778;     jngiIkqXiKpsJlyJr90912778 = jngiIkqXiKpsJlyJr69595529;     jngiIkqXiKpsJlyJr69595529 = jngiIkqXiKpsJlyJr89494558;     jngiIkqXiKpsJlyJr89494558 = jngiIkqXiKpsJlyJr32040284;     jngiIkqXiKpsJlyJr32040284 = jngiIkqXiKpsJlyJr14227983;     jngiIkqXiKpsJlyJr14227983 = jngiIkqXiKpsJlyJr61412392;     jngiIkqXiKpsJlyJr61412392 = jngiIkqXiKpsJlyJr76453708;     jngiIkqXiKpsJlyJr76453708 = jngiIkqXiKpsJlyJr49116392;     jngiIkqXiKpsJlyJr49116392 = jngiIkqXiKpsJlyJr73813861;     jngiIkqXiKpsJlyJr73813861 = jngiIkqXiKpsJlyJr22779112;     jngiIkqXiKpsJlyJr22779112 = jngiIkqXiKpsJlyJr59086956;     jngiIkqXiKpsJlyJr59086956 = jngiIkqXiKpsJlyJr11717335;     jngiIkqXiKpsJlyJr11717335 = jngiIkqXiKpsJlyJr38989641;     jngiIkqXiKpsJlyJr38989641 = jngiIkqXiKpsJlyJr70649389;     jngiIkqXiKpsJlyJr70649389 = jngiIkqXiKpsJlyJr57491903;     jngiIkqXiKpsJlyJr57491903 = jngiIkqXiKpsJlyJr33874671;     jngiIkqXiKpsJlyJr33874671 = jngiIkqXiKpsJlyJr43916941;     jngiIkqXiKpsJlyJr43916941 = jngiIkqXiKpsJlyJr64150312;     jngiIkqXiKpsJlyJr64150312 = jngiIkqXiKpsJlyJr96642751;     jngiIkqXiKpsJlyJr96642751 = jngiIkqXiKpsJlyJr8100847;     jngiIkqXiKpsJlyJr8100847 = jngiIkqXiKpsJlyJr37515709;     jngiIkqXiKpsJlyJr37515709 = jngiIkqXiKpsJlyJr25044083;     jngiIkqXiKpsJlyJr25044083 = jngiIkqXiKpsJlyJr18144635;     jngiIkqXiKpsJlyJr18144635 = jngiIkqXiKpsJlyJr20520179;     jngiIkqXiKpsJlyJr20520179 = jngiIkqXiKpsJlyJr98646431;     jngiIkqXiKpsJlyJr98646431 = jngiIkqXiKpsJlyJr81104126;     jngiIkqXiKpsJlyJr81104126 = jngiIkqXiKpsJlyJr73287916;     jngiIkqXiKpsJlyJr73287916 = jngiIkqXiKpsJlyJr14857231;     jngiIkqXiKpsJlyJr14857231 = jngiIkqXiKpsJlyJr4154852;     jngiIkqXiKpsJlyJr4154852 = jngiIkqXiKpsJlyJr69203589;     jngiIkqXiKpsJlyJr69203589 = jngiIkqXiKpsJlyJr58719135;     jngiIkqXiKpsJlyJr58719135 = jngiIkqXiKpsJlyJr22852021;     jngiIkqXiKpsJlyJr22852021 = jngiIkqXiKpsJlyJr39650565;     jngiIkqXiKpsJlyJr39650565 = jngiIkqXiKpsJlyJr43898894;     jngiIkqXiKpsJlyJr43898894 = jngiIkqXiKpsJlyJr13925431;     jngiIkqXiKpsJlyJr13925431 = jngiIkqXiKpsJlyJr75363582;     jngiIkqXiKpsJlyJr75363582 = jngiIkqXiKpsJlyJr66638713;     jngiIkqXiKpsJlyJr66638713 = jngiIkqXiKpsJlyJr42963697;     jngiIkqXiKpsJlyJr42963697 = jngiIkqXiKpsJlyJr28023722;     jngiIkqXiKpsJlyJr28023722 = jngiIkqXiKpsJlyJr68864726;     jngiIkqXiKpsJlyJr68864726 = jngiIkqXiKpsJlyJr67120338;     jngiIkqXiKpsJlyJr67120338 = jngiIkqXiKpsJlyJr64084454;     jngiIkqXiKpsJlyJr64084454 = jngiIkqXiKpsJlyJr93334028;     jngiIkqXiKpsJlyJr93334028 = jngiIkqXiKpsJlyJr50367626;     jngiIkqXiKpsJlyJr50367626 = jngiIkqXiKpsJlyJr45367595;     jngiIkqXiKpsJlyJr45367595 = jngiIkqXiKpsJlyJr16749442;     jngiIkqXiKpsJlyJr16749442 = jngiIkqXiKpsJlyJr18865132;     jngiIkqXiKpsJlyJr18865132 = jngiIkqXiKpsJlyJr31858382;     jngiIkqXiKpsJlyJr31858382 = jngiIkqXiKpsJlyJr49247836;     jngiIkqXiKpsJlyJr49247836 = jngiIkqXiKpsJlyJr46398139;     jngiIkqXiKpsJlyJr46398139 = jngiIkqXiKpsJlyJr50025475;     jngiIkqXiKpsJlyJr50025475 = jngiIkqXiKpsJlyJr50567438;     jngiIkqXiKpsJlyJr50567438 = jngiIkqXiKpsJlyJr12208208;     jngiIkqXiKpsJlyJr12208208 = jngiIkqXiKpsJlyJr70857423;     jngiIkqXiKpsJlyJr70857423 = jngiIkqXiKpsJlyJr11188114;     jngiIkqXiKpsJlyJr11188114 = jngiIkqXiKpsJlyJr65868696;     jngiIkqXiKpsJlyJr65868696 = jngiIkqXiKpsJlyJr51450895;     jngiIkqXiKpsJlyJr51450895 = jngiIkqXiKpsJlyJr68974379;     jngiIkqXiKpsJlyJr68974379 = jngiIkqXiKpsJlyJr33393853;     jngiIkqXiKpsJlyJr33393853 = jngiIkqXiKpsJlyJr33123857;     jngiIkqXiKpsJlyJr33123857 = jngiIkqXiKpsJlyJr88124475;     jngiIkqXiKpsJlyJr88124475 = jngiIkqXiKpsJlyJr61596477;     jngiIkqXiKpsJlyJr61596477 = jngiIkqXiKpsJlyJr44961540;     jngiIkqXiKpsJlyJr44961540 = jngiIkqXiKpsJlyJr4610272;     jngiIkqXiKpsJlyJr4610272 = jngiIkqXiKpsJlyJr64059977;     jngiIkqXiKpsJlyJr64059977 = jngiIkqXiKpsJlyJr36234936;     jngiIkqXiKpsJlyJr36234936 = jngiIkqXiKpsJlyJr72066770;     jngiIkqXiKpsJlyJr72066770 = jngiIkqXiKpsJlyJr95090747;     jngiIkqXiKpsJlyJr95090747 = jngiIkqXiKpsJlyJr56723958;     jngiIkqXiKpsJlyJr56723958 = jngiIkqXiKpsJlyJr82128320;     jngiIkqXiKpsJlyJr82128320 = jngiIkqXiKpsJlyJr67235957;     jngiIkqXiKpsJlyJr67235957 = jngiIkqXiKpsJlyJr953244;     jngiIkqXiKpsJlyJr953244 = jngiIkqXiKpsJlyJr36126590;     jngiIkqXiKpsJlyJr36126590 = jngiIkqXiKpsJlyJr27778026;     jngiIkqXiKpsJlyJr27778026 = jngiIkqXiKpsJlyJr40980509;     jngiIkqXiKpsJlyJr40980509 = jngiIkqXiKpsJlyJr73431255;     jngiIkqXiKpsJlyJr73431255 = jngiIkqXiKpsJlyJr31710054;     jngiIkqXiKpsJlyJr31710054 = jngiIkqXiKpsJlyJr67777008;     jngiIkqXiKpsJlyJr67777008 = jngiIkqXiKpsJlyJr75152584;     jngiIkqXiKpsJlyJr75152584 = jngiIkqXiKpsJlyJr4838209;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void noAiBQvNoNxkuADY7651784() {     double UbGxTRsdhqJOtJBHY10955316 = -211035004;    double UbGxTRsdhqJOtJBHY88332391 = -24970845;    double UbGxTRsdhqJOtJBHY72175455 = -695108037;    double UbGxTRsdhqJOtJBHY2078762 = -469045285;    double UbGxTRsdhqJOtJBHY11147194 = -885326034;    double UbGxTRsdhqJOtJBHY21694320 = 46463022;    double UbGxTRsdhqJOtJBHY4063013 = 50703689;    double UbGxTRsdhqJOtJBHY62960736 = -69791977;    double UbGxTRsdhqJOtJBHY31790385 = -11308107;    double UbGxTRsdhqJOtJBHY14376398 = -969156894;    double UbGxTRsdhqJOtJBHY64564841 = -348813509;    double UbGxTRsdhqJOtJBHY60289819 = -81366645;    double UbGxTRsdhqJOtJBHY44063892 = -365583415;    double UbGxTRsdhqJOtJBHY85651447 = -326970281;    double UbGxTRsdhqJOtJBHY2482166 = -646563649;    double UbGxTRsdhqJOtJBHY21778204 = -84856790;    double UbGxTRsdhqJOtJBHY87778419 = -455782646;    double UbGxTRsdhqJOtJBHY11770903 = -455170589;    double UbGxTRsdhqJOtJBHY70680801 = -429807763;    double UbGxTRsdhqJOtJBHY59508434 = -407518780;    double UbGxTRsdhqJOtJBHY37257016 = -755028213;    double UbGxTRsdhqJOtJBHY77433802 = -416420330;    double UbGxTRsdhqJOtJBHY5186322 = -484999259;    double UbGxTRsdhqJOtJBHY73486932 = -395826335;    double UbGxTRsdhqJOtJBHY75739934 = -634881365;    double UbGxTRsdhqJOtJBHY38949658 = -7871320;    double UbGxTRsdhqJOtJBHY59872170 = -360867658;    double UbGxTRsdhqJOtJBHY89560833 = -907079628;    double UbGxTRsdhqJOtJBHY15248647 = -293650702;    double UbGxTRsdhqJOtJBHY42429498 = -488824811;    double UbGxTRsdhqJOtJBHY15021662 = -497995500;    double UbGxTRsdhqJOtJBHY56697578 = -333701750;    double UbGxTRsdhqJOtJBHY21428283 = 27799387;    double UbGxTRsdhqJOtJBHY33820205 = -819502022;    double UbGxTRsdhqJOtJBHY32124041 = 27732390;    double UbGxTRsdhqJOtJBHY26217816 = -434292618;    double UbGxTRsdhqJOtJBHY15136605 = -439991525;    double UbGxTRsdhqJOtJBHY78007153 = -815648105;    double UbGxTRsdhqJOtJBHY37760566 = -34670398;    double UbGxTRsdhqJOtJBHY84144689 = -533685497;    double UbGxTRsdhqJOtJBHY13770071 = -450282811;    double UbGxTRsdhqJOtJBHY69666166 = -988639917;    double UbGxTRsdhqJOtJBHY3111061 = 55205326;    double UbGxTRsdhqJOtJBHY5552375 = -333163300;    double UbGxTRsdhqJOtJBHY32472644 = -566558690;    double UbGxTRsdhqJOtJBHY10585627 = -344844147;    double UbGxTRsdhqJOtJBHY98882700 = -353487685;    double UbGxTRsdhqJOtJBHY81244857 = -4678138;    double UbGxTRsdhqJOtJBHY24532960 = -632243016;    double UbGxTRsdhqJOtJBHY17066505 = -569595277;    double UbGxTRsdhqJOtJBHY4904268 = -452270544;    double UbGxTRsdhqJOtJBHY69528854 = -203334567;    double UbGxTRsdhqJOtJBHY71100478 = -318940333;    double UbGxTRsdhqJOtJBHY35619445 = -424307198;    double UbGxTRsdhqJOtJBHY70394768 = -974312598;    double UbGxTRsdhqJOtJBHY33521513 = -794614675;    double UbGxTRsdhqJOtJBHY83146069 = -539971586;    double UbGxTRsdhqJOtJBHY98688523 = -199281702;    double UbGxTRsdhqJOtJBHY26338828 = -834163920;    double UbGxTRsdhqJOtJBHY72197535 = -777454714;    double UbGxTRsdhqJOtJBHY61822150 = -592669321;    double UbGxTRsdhqJOtJBHY14502179 = -42216683;    double UbGxTRsdhqJOtJBHY47712089 = -776141276;    double UbGxTRsdhqJOtJBHY89360887 = -522483296;    double UbGxTRsdhqJOtJBHY99354736 = -371161394;    double UbGxTRsdhqJOtJBHY7867263 = 84888241;    double UbGxTRsdhqJOtJBHY38861536 = -9166033;    double UbGxTRsdhqJOtJBHY10243687 = -546081393;    double UbGxTRsdhqJOtJBHY53527406 = -254702672;    double UbGxTRsdhqJOtJBHY76264349 = -112271032;    double UbGxTRsdhqJOtJBHY6641599 = -644865266;    double UbGxTRsdhqJOtJBHY9771266 = -640134542;    double UbGxTRsdhqJOtJBHY74010337 = -320500191;    double UbGxTRsdhqJOtJBHY86536112 = -896122267;    double UbGxTRsdhqJOtJBHY45738364 = -957235969;    double UbGxTRsdhqJOtJBHY67590849 = -766388297;    double UbGxTRsdhqJOtJBHY74322741 = -371625657;    double UbGxTRsdhqJOtJBHY99633946 = -51835960;    double UbGxTRsdhqJOtJBHY41014288 = -829267646;    double UbGxTRsdhqJOtJBHY65154308 = -190037219;    double UbGxTRsdhqJOtJBHY40066958 = -654383636;    double UbGxTRsdhqJOtJBHY78627312 = -256189520;    double UbGxTRsdhqJOtJBHY65027874 = -174836613;    double UbGxTRsdhqJOtJBHY98182142 = -724055425;    double UbGxTRsdhqJOtJBHY37525230 = 63445733;    double UbGxTRsdhqJOtJBHY45492807 = -194660934;    double UbGxTRsdhqJOtJBHY85597100 = 85238582;    double UbGxTRsdhqJOtJBHY85808838 = -547893415;    double UbGxTRsdhqJOtJBHY63425437 = -845189424;    double UbGxTRsdhqJOtJBHY98602527 = -177652936;    double UbGxTRsdhqJOtJBHY43071746 = -894321033;    double UbGxTRsdhqJOtJBHY16448082 = -140709823;    double UbGxTRsdhqJOtJBHY51668326 = -981484185;    double UbGxTRsdhqJOtJBHY65563030 = -257215685;    double UbGxTRsdhqJOtJBHY22322540 = -941016177;    double UbGxTRsdhqJOtJBHY99267891 = -308066128;    double UbGxTRsdhqJOtJBHY21954077 = -112498642;    double UbGxTRsdhqJOtJBHY13750174 = -422311378;    double UbGxTRsdhqJOtJBHY6197639 = -962001906;    double UbGxTRsdhqJOtJBHY24605381 = -211035004;     UbGxTRsdhqJOtJBHY10955316 = UbGxTRsdhqJOtJBHY88332391;     UbGxTRsdhqJOtJBHY88332391 = UbGxTRsdhqJOtJBHY72175455;     UbGxTRsdhqJOtJBHY72175455 = UbGxTRsdhqJOtJBHY2078762;     UbGxTRsdhqJOtJBHY2078762 = UbGxTRsdhqJOtJBHY11147194;     UbGxTRsdhqJOtJBHY11147194 = UbGxTRsdhqJOtJBHY21694320;     UbGxTRsdhqJOtJBHY21694320 = UbGxTRsdhqJOtJBHY4063013;     UbGxTRsdhqJOtJBHY4063013 = UbGxTRsdhqJOtJBHY62960736;     UbGxTRsdhqJOtJBHY62960736 = UbGxTRsdhqJOtJBHY31790385;     UbGxTRsdhqJOtJBHY31790385 = UbGxTRsdhqJOtJBHY14376398;     UbGxTRsdhqJOtJBHY14376398 = UbGxTRsdhqJOtJBHY64564841;     UbGxTRsdhqJOtJBHY64564841 = UbGxTRsdhqJOtJBHY60289819;     UbGxTRsdhqJOtJBHY60289819 = UbGxTRsdhqJOtJBHY44063892;     UbGxTRsdhqJOtJBHY44063892 = UbGxTRsdhqJOtJBHY85651447;     UbGxTRsdhqJOtJBHY85651447 = UbGxTRsdhqJOtJBHY2482166;     UbGxTRsdhqJOtJBHY2482166 = UbGxTRsdhqJOtJBHY21778204;     UbGxTRsdhqJOtJBHY21778204 = UbGxTRsdhqJOtJBHY87778419;     UbGxTRsdhqJOtJBHY87778419 = UbGxTRsdhqJOtJBHY11770903;     UbGxTRsdhqJOtJBHY11770903 = UbGxTRsdhqJOtJBHY70680801;     UbGxTRsdhqJOtJBHY70680801 = UbGxTRsdhqJOtJBHY59508434;     UbGxTRsdhqJOtJBHY59508434 = UbGxTRsdhqJOtJBHY37257016;     UbGxTRsdhqJOtJBHY37257016 = UbGxTRsdhqJOtJBHY77433802;     UbGxTRsdhqJOtJBHY77433802 = UbGxTRsdhqJOtJBHY5186322;     UbGxTRsdhqJOtJBHY5186322 = UbGxTRsdhqJOtJBHY73486932;     UbGxTRsdhqJOtJBHY73486932 = UbGxTRsdhqJOtJBHY75739934;     UbGxTRsdhqJOtJBHY75739934 = UbGxTRsdhqJOtJBHY38949658;     UbGxTRsdhqJOtJBHY38949658 = UbGxTRsdhqJOtJBHY59872170;     UbGxTRsdhqJOtJBHY59872170 = UbGxTRsdhqJOtJBHY89560833;     UbGxTRsdhqJOtJBHY89560833 = UbGxTRsdhqJOtJBHY15248647;     UbGxTRsdhqJOtJBHY15248647 = UbGxTRsdhqJOtJBHY42429498;     UbGxTRsdhqJOtJBHY42429498 = UbGxTRsdhqJOtJBHY15021662;     UbGxTRsdhqJOtJBHY15021662 = UbGxTRsdhqJOtJBHY56697578;     UbGxTRsdhqJOtJBHY56697578 = UbGxTRsdhqJOtJBHY21428283;     UbGxTRsdhqJOtJBHY21428283 = UbGxTRsdhqJOtJBHY33820205;     UbGxTRsdhqJOtJBHY33820205 = UbGxTRsdhqJOtJBHY32124041;     UbGxTRsdhqJOtJBHY32124041 = UbGxTRsdhqJOtJBHY26217816;     UbGxTRsdhqJOtJBHY26217816 = UbGxTRsdhqJOtJBHY15136605;     UbGxTRsdhqJOtJBHY15136605 = UbGxTRsdhqJOtJBHY78007153;     UbGxTRsdhqJOtJBHY78007153 = UbGxTRsdhqJOtJBHY37760566;     UbGxTRsdhqJOtJBHY37760566 = UbGxTRsdhqJOtJBHY84144689;     UbGxTRsdhqJOtJBHY84144689 = UbGxTRsdhqJOtJBHY13770071;     UbGxTRsdhqJOtJBHY13770071 = UbGxTRsdhqJOtJBHY69666166;     UbGxTRsdhqJOtJBHY69666166 = UbGxTRsdhqJOtJBHY3111061;     UbGxTRsdhqJOtJBHY3111061 = UbGxTRsdhqJOtJBHY5552375;     UbGxTRsdhqJOtJBHY5552375 = UbGxTRsdhqJOtJBHY32472644;     UbGxTRsdhqJOtJBHY32472644 = UbGxTRsdhqJOtJBHY10585627;     UbGxTRsdhqJOtJBHY10585627 = UbGxTRsdhqJOtJBHY98882700;     UbGxTRsdhqJOtJBHY98882700 = UbGxTRsdhqJOtJBHY81244857;     UbGxTRsdhqJOtJBHY81244857 = UbGxTRsdhqJOtJBHY24532960;     UbGxTRsdhqJOtJBHY24532960 = UbGxTRsdhqJOtJBHY17066505;     UbGxTRsdhqJOtJBHY17066505 = UbGxTRsdhqJOtJBHY4904268;     UbGxTRsdhqJOtJBHY4904268 = UbGxTRsdhqJOtJBHY69528854;     UbGxTRsdhqJOtJBHY69528854 = UbGxTRsdhqJOtJBHY71100478;     UbGxTRsdhqJOtJBHY71100478 = UbGxTRsdhqJOtJBHY35619445;     UbGxTRsdhqJOtJBHY35619445 = UbGxTRsdhqJOtJBHY70394768;     UbGxTRsdhqJOtJBHY70394768 = UbGxTRsdhqJOtJBHY33521513;     UbGxTRsdhqJOtJBHY33521513 = UbGxTRsdhqJOtJBHY83146069;     UbGxTRsdhqJOtJBHY83146069 = UbGxTRsdhqJOtJBHY98688523;     UbGxTRsdhqJOtJBHY98688523 = UbGxTRsdhqJOtJBHY26338828;     UbGxTRsdhqJOtJBHY26338828 = UbGxTRsdhqJOtJBHY72197535;     UbGxTRsdhqJOtJBHY72197535 = UbGxTRsdhqJOtJBHY61822150;     UbGxTRsdhqJOtJBHY61822150 = UbGxTRsdhqJOtJBHY14502179;     UbGxTRsdhqJOtJBHY14502179 = UbGxTRsdhqJOtJBHY47712089;     UbGxTRsdhqJOtJBHY47712089 = UbGxTRsdhqJOtJBHY89360887;     UbGxTRsdhqJOtJBHY89360887 = UbGxTRsdhqJOtJBHY99354736;     UbGxTRsdhqJOtJBHY99354736 = UbGxTRsdhqJOtJBHY7867263;     UbGxTRsdhqJOtJBHY7867263 = UbGxTRsdhqJOtJBHY38861536;     UbGxTRsdhqJOtJBHY38861536 = UbGxTRsdhqJOtJBHY10243687;     UbGxTRsdhqJOtJBHY10243687 = UbGxTRsdhqJOtJBHY53527406;     UbGxTRsdhqJOtJBHY53527406 = UbGxTRsdhqJOtJBHY76264349;     UbGxTRsdhqJOtJBHY76264349 = UbGxTRsdhqJOtJBHY6641599;     UbGxTRsdhqJOtJBHY6641599 = UbGxTRsdhqJOtJBHY9771266;     UbGxTRsdhqJOtJBHY9771266 = UbGxTRsdhqJOtJBHY74010337;     UbGxTRsdhqJOtJBHY74010337 = UbGxTRsdhqJOtJBHY86536112;     UbGxTRsdhqJOtJBHY86536112 = UbGxTRsdhqJOtJBHY45738364;     UbGxTRsdhqJOtJBHY45738364 = UbGxTRsdhqJOtJBHY67590849;     UbGxTRsdhqJOtJBHY67590849 = UbGxTRsdhqJOtJBHY74322741;     UbGxTRsdhqJOtJBHY74322741 = UbGxTRsdhqJOtJBHY99633946;     UbGxTRsdhqJOtJBHY99633946 = UbGxTRsdhqJOtJBHY41014288;     UbGxTRsdhqJOtJBHY41014288 = UbGxTRsdhqJOtJBHY65154308;     UbGxTRsdhqJOtJBHY65154308 = UbGxTRsdhqJOtJBHY40066958;     UbGxTRsdhqJOtJBHY40066958 = UbGxTRsdhqJOtJBHY78627312;     UbGxTRsdhqJOtJBHY78627312 = UbGxTRsdhqJOtJBHY65027874;     UbGxTRsdhqJOtJBHY65027874 = UbGxTRsdhqJOtJBHY98182142;     UbGxTRsdhqJOtJBHY98182142 = UbGxTRsdhqJOtJBHY37525230;     UbGxTRsdhqJOtJBHY37525230 = UbGxTRsdhqJOtJBHY45492807;     UbGxTRsdhqJOtJBHY45492807 = UbGxTRsdhqJOtJBHY85597100;     UbGxTRsdhqJOtJBHY85597100 = UbGxTRsdhqJOtJBHY85808838;     UbGxTRsdhqJOtJBHY85808838 = UbGxTRsdhqJOtJBHY63425437;     UbGxTRsdhqJOtJBHY63425437 = UbGxTRsdhqJOtJBHY98602527;     UbGxTRsdhqJOtJBHY98602527 = UbGxTRsdhqJOtJBHY43071746;     UbGxTRsdhqJOtJBHY43071746 = UbGxTRsdhqJOtJBHY16448082;     UbGxTRsdhqJOtJBHY16448082 = UbGxTRsdhqJOtJBHY51668326;     UbGxTRsdhqJOtJBHY51668326 = UbGxTRsdhqJOtJBHY65563030;     UbGxTRsdhqJOtJBHY65563030 = UbGxTRsdhqJOtJBHY22322540;     UbGxTRsdhqJOtJBHY22322540 = UbGxTRsdhqJOtJBHY99267891;     UbGxTRsdhqJOtJBHY99267891 = UbGxTRsdhqJOtJBHY21954077;     UbGxTRsdhqJOtJBHY21954077 = UbGxTRsdhqJOtJBHY13750174;     UbGxTRsdhqJOtJBHY13750174 = UbGxTRsdhqJOtJBHY6197639;     UbGxTRsdhqJOtJBHY6197639 = UbGxTRsdhqJOtJBHY24605381;     UbGxTRsdhqJOtJBHY24605381 = UbGxTRsdhqJOtJBHY10955316;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WCqWgvEkReXvOXLp74943383() {     double qClJFtEwMfDuBhRrw46414284 = -677311904;    double qClJFtEwMfDuBhRrw1604470 = 94477549;    double qClJFtEwMfDuBhRrw9561932 = -360775112;    double qClJFtEwMfDuBhRrw74302117 = -290196356;    double qClJFtEwMfDuBhRrw67432463 = -990924879;    double qClJFtEwMfDuBhRrw57917192 = -215884873;    double qClJFtEwMfDuBhRrw14720419 = -894022045;    double qClJFtEwMfDuBhRrw25092297 = -814026118;    double qClJFtEwMfDuBhRrw33307991 = -499639514;    double qClJFtEwMfDuBhRrw29279110 = -633843892;    double qClJFtEwMfDuBhRrw2746577 = -150821238;    double qClJFtEwMfDuBhRrw60857967 = 54512583;    double qClJFtEwMfDuBhRrw82298537 = -917595919;    double qClJFtEwMfDuBhRrw38792310 = -303333690;    double qClJFtEwMfDuBhRrw60676682 = -49988664;    double qClJFtEwMfDuBhRrw5319505 = -820668397;    double qClJFtEwMfDuBhRrw43614355 = -982757199;    double qClJFtEwMfDuBhRrw11531095 = -528598658;    double qClJFtEwMfDuBhRrw71231131 = -746371185;    double qClJFtEwMfDuBhRrw53345653 = -834079845;    double qClJFtEwMfDuBhRrw50343878 = -412302902;    double qClJFtEwMfDuBhRrw80855327 = -503056624;    double qClJFtEwMfDuBhRrw75711701 = -122631444;    double qClJFtEwMfDuBhRrw67279877 = -906650939;    double qClJFtEwMfDuBhRrw83757885 = -658866564;    double qClJFtEwMfDuBhRrw49647830 = -796448127;    double qClJFtEwMfDuBhRrw20292744 = -167151799;    double qClJFtEwMfDuBhRrw30289259 = 23148197;    double qClJFtEwMfDuBhRrw61916862 = -867745504;    double qClJFtEwMfDuBhRrw47962909 = -523331982;    double qClJFtEwMfDuBhRrw33604893 = 50024013;    double qClJFtEwMfDuBhRrw94520596 = -106673164;    double qClJFtEwMfDuBhRrw33646294 = -119796590;    double qClJFtEwMfDuBhRrw63568632 = -884195835;    double qClJFtEwMfDuBhRrw97932494 = 76632140;    double qClJFtEwMfDuBhRrw77778659 = -648857924;    double qClJFtEwMfDuBhRrw64548421 = -304687565;    double qClJFtEwMfDuBhRrw29222557 = -798303686;    double qClJFtEwMfDuBhRrw68151127 = -785546675;    double qClJFtEwMfDuBhRrw48416222 = 1051957;    double qClJFtEwMfDuBhRrw47221678 = -219481725;    double qClJFtEwMfDuBhRrw59031293 = -583185208;    double qClJFtEwMfDuBhRrw92494439 = -597975460;    double qClJFtEwMfDuBhRrw45683713 = -937495710;    double qClJFtEwMfDuBhRrw32489103 = -393069851;    double qClJFtEwMfDuBhRrw56546955 = -383767779;    double qClJFtEwMfDuBhRrw18227864 = -837474045;    double qClJFtEwMfDuBhRrw98971169 = -171204731;    double qClJFtEwMfDuBhRrw47425626 = -295979896;    double qClJFtEwMfDuBhRrw41596094 = 39377907;    double qClJFtEwMfDuBhRrw85674991 = -326547102;    double qClJFtEwMfDuBhRrw53014327 = -416986914;    double qClJFtEwMfDuBhRrw39721626 = -74985887;    double qClJFtEwMfDuBhRrw51611291 = -615815698;    double qClJFtEwMfDuBhRrw24599505 = -220802744;    double qClJFtEwMfDuBhRrw65558957 = -74255280;    double qClJFtEwMfDuBhRrw25892769 = -782891008;    double qClJFtEwMfDuBhRrw42282055 = -454124173;    double qClJFtEwMfDuBhRrw90544231 = -631329793;    double qClJFtEwMfDuBhRrw17784633 = -94476753;    double qClJFtEwMfDuBhRrw37624448 = 51266925;    double qClJFtEwMfDuBhRrw84431160 = -817170243;    double qClJFtEwMfDuBhRrw63175435 = -946280614;    double qClJFtEwMfDuBhRrw85345082 = -976307532;    double qClJFtEwMfDuBhRrw95674216 = -583867905;    double qClJFtEwMfDuBhRrw8225980 = 55851926;    double qClJFtEwMfDuBhRrw27211673 = -825690827;    double qClJFtEwMfDuBhRrw18729905 = 66599915;    double qClJFtEwMfDuBhRrw40859816 = -279965830;    double qClJFtEwMfDuBhRrw82898022 = -401130741;    double qClJFtEwMfDuBhRrw40771083 = -415980833;    double qClJFtEwMfDuBhRrw14391799 = -84453514;    double qClJFtEwMfDuBhRrw43379967 = -743051984;    double qClJFtEwMfDuBhRrw22814910 = -647423143;    double qClJFtEwMfDuBhRrw6123975 = -514598120;    double qClJFtEwMfDuBhRrw91312585 = -829117695;    double qClJFtEwMfDuBhRrw88360887 = -905081165;    double qClJFtEwMfDuBhRrw30027988 = -185135734;    double qClJFtEwMfDuBhRrw34790775 = -413581088;    double qClJFtEwMfDuBhRrw27210931 = -175098785;    double qClJFtEwMfDuBhRrw31419966 = -958974082;    double qClJFtEwMfDuBhRrw21321575 = -995947068;    double qClJFtEwMfDuBhRrw82863632 = -680871907;    double qClJFtEwMfDuBhRrw20320768 = -807123412;    double qClJFtEwMfDuBhRrw62287917 = -96784881;    double qClJFtEwMfDuBhRrw80590566 = -532989074;    double qClJFtEwMfDuBhRrw54798970 = 68312722;    double qClJFtEwMfDuBhRrw82035002 = -503980893;    double qClJFtEwMfDuBhRrw38969127 = -563393092;    double qClJFtEwMfDuBhRrw32373538 = -849112580;    double qClJFtEwMfDuBhRrw51885891 = -865966916;    double qClJFtEwMfDuBhRrw22266367 = -850563392;    double qClJFtEwMfDuBhRrw38678326 = -66973893;    double qClJFtEwMfDuBhRrw50366495 = -591069923;    double qClJFtEwMfDuBhRrw10791774 = 49785032;    double qClJFtEwMfDuBhRrw62790518 = -402311483;    double qClJFtEwMfDuBhRrw95855857 = -636904595;    double qClJFtEwMfDuBhRrw7149358 = -621667928;    double qClJFtEwMfDuBhRrw50009496 = -253627805;    double qClJFtEwMfDuBhRrw24263123 = -677311904;     qClJFtEwMfDuBhRrw46414284 = qClJFtEwMfDuBhRrw1604470;     qClJFtEwMfDuBhRrw1604470 = qClJFtEwMfDuBhRrw9561932;     qClJFtEwMfDuBhRrw9561932 = qClJFtEwMfDuBhRrw74302117;     qClJFtEwMfDuBhRrw74302117 = qClJFtEwMfDuBhRrw67432463;     qClJFtEwMfDuBhRrw67432463 = qClJFtEwMfDuBhRrw57917192;     qClJFtEwMfDuBhRrw57917192 = qClJFtEwMfDuBhRrw14720419;     qClJFtEwMfDuBhRrw14720419 = qClJFtEwMfDuBhRrw25092297;     qClJFtEwMfDuBhRrw25092297 = qClJFtEwMfDuBhRrw33307991;     qClJFtEwMfDuBhRrw33307991 = qClJFtEwMfDuBhRrw29279110;     qClJFtEwMfDuBhRrw29279110 = qClJFtEwMfDuBhRrw2746577;     qClJFtEwMfDuBhRrw2746577 = qClJFtEwMfDuBhRrw60857967;     qClJFtEwMfDuBhRrw60857967 = qClJFtEwMfDuBhRrw82298537;     qClJFtEwMfDuBhRrw82298537 = qClJFtEwMfDuBhRrw38792310;     qClJFtEwMfDuBhRrw38792310 = qClJFtEwMfDuBhRrw60676682;     qClJFtEwMfDuBhRrw60676682 = qClJFtEwMfDuBhRrw5319505;     qClJFtEwMfDuBhRrw5319505 = qClJFtEwMfDuBhRrw43614355;     qClJFtEwMfDuBhRrw43614355 = qClJFtEwMfDuBhRrw11531095;     qClJFtEwMfDuBhRrw11531095 = qClJFtEwMfDuBhRrw71231131;     qClJFtEwMfDuBhRrw71231131 = qClJFtEwMfDuBhRrw53345653;     qClJFtEwMfDuBhRrw53345653 = qClJFtEwMfDuBhRrw50343878;     qClJFtEwMfDuBhRrw50343878 = qClJFtEwMfDuBhRrw80855327;     qClJFtEwMfDuBhRrw80855327 = qClJFtEwMfDuBhRrw75711701;     qClJFtEwMfDuBhRrw75711701 = qClJFtEwMfDuBhRrw67279877;     qClJFtEwMfDuBhRrw67279877 = qClJFtEwMfDuBhRrw83757885;     qClJFtEwMfDuBhRrw83757885 = qClJFtEwMfDuBhRrw49647830;     qClJFtEwMfDuBhRrw49647830 = qClJFtEwMfDuBhRrw20292744;     qClJFtEwMfDuBhRrw20292744 = qClJFtEwMfDuBhRrw30289259;     qClJFtEwMfDuBhRrw30289259 = qClJFtEwMfDuBhRrw61916862;     qClJFtEwMfDuBhRrw61916862 = qClJFtEwMfDuBhRrw47962909;     qClJFtEwMfDuBhRrw47962909 = qClJFtEwMfDuBhRrw33604893;     qClJFtEwMfDuBhRrw33604893 = qClJFtEwMfDuBhRrw94520596;     qClJFtEwMfDuBhRrw94520596 = qClJFtEwMfDuBhRrw33646294;     qClJFtEwMfDuBhRrw33646294 = qClJFtEwMfDuBhRrw63568632;     qClJFtEwMfDuBhRrw63568632 = qClJFtEwMfDuBhRrw97932494;     qClJFtEwMfDuBhRrw97932494 = qClJFtEwMfDuBhRrw77778659;     qClJFtEwMfDuBhRrw77778659 = qClJFtEwMfDuBhRrw64548421;     qClJFtEwMfDuBhRrw64548421 = qClJFtEwMfDuBhRrw29222557;     qClJFtEwMfDuBhRrw29222557 = qClJFtEwMfDuBhRrw68151127;     qClJFtEwMfDuBhRrw68151127 = qClJFtEwMfDuBhRrw48416222;     qClJFtEwMfDuBhRrw48416222 = qClJFtEwMfDuBhRrw47221678;     qClJFtEwMfDuBhRrw47221678 = qClJFtEwMfDuBhRrw59031293;     qClJFtEwMfDuBhRrw59031293 = qClJFtEwMfDuBhRrw92494439;     qClJFtEwMfDuBhRrw92494439 = qClJFtEwMfDuBhRrw45683713;     qClJFtEwMfDuBhRrw45683713 = qClJFtEwMfDuBhRrw32489103;     qClJFtEwMfDuBhRrw32489103 = qClJFtEwMfDuBhRrw56546955;     qClJFtEwMfDuBhRrw56546955 = qClJFtEwMfDuBhRrw18227864;     qClJFtEwMfDuBhRrw18227864 = qClJFtEwMfDuBhRrw98971169;     qClJFtEwMfDuBhRrw98971169 = qClJFtEwMfDuBhRrw47425626;     qClJFtEwMfDuBhRrw47425626 = qClJFtEwMfDuBhRrw41596094;     qClJFtEwMfDuBhRrw41596094 = qClJFtEwMfDuBhRrw85674991;     qClJFtEwMfDuBhRrw85674991 = qClJFtEwMfDuBhRrw53014327;     qClJFtEwMfDuBhRrw53014327 = qClJFtEwMfDuBhRrw39721626;     qClJFtEwMfDuBhRrw39721626 = qClJFtEwMfDuBhRrw51611291;     qClJFtEwMfDuBhRrw51611291 = qClJFtEwMfDuBhRrw24599505;     qClJFtEwMfDuBhRrw24599505 = qClJFtEwMfDuBhRrw65558957;     qClJFtEwMfDuBhRrw65558957 = qClJFtEwMfDuBhRrw25892769;     qClJFtEwMfDuBhRrw25892769 = qClJFtEwMfDuBhRrw42282055;     qClJFtEwMfDuBhRrw42282055 = qClJFtEwMfDuBhRrw90544231;     qClJFtEwMfDuBhRrw90544231 = qClJFtEwMfDuBhRrw17784633;     qClJFtEwMfDuBhRrw17784633 = qClJFtEwMfDuBhRrw37624448;     qClJFtEwMfDuBhRrw37624448 = qClJFtEwMfDuBhRrw84431160;     qClJFtEwMfDuBhRrw84431160 = qClJFtEwMfDuBhRrw63175435;     qClJFtEwMfDuBhRrw63175435 = qClJFtEwMfDuBhRrw85345082;     qClJFtEwMfDuBhRrw85345082 = qClJFtEwMfDuBhRrw95674216;     qClJFtEwMfDuBhRrw95674216 = qClJFtEwMfDuBhRrw8225980;     qClJFtEwMfDuBhRrw8225980 = qClJFtEwMfDuBhRrw27211673;     qClJFtEwMfDuBhRrw27211673 = qClJFtEwMfDuBhRrw18729905;     qClJFtEwMfDuBhRrw18729905 = qClJFtEwMfDuBhRrw40859816;     qClJFtEwMfDuBhRrw40859816 = qClJFtEwMfDuBhRrw82898022;     qClJFtEwMfDuBhRrw82898022 = qClJFtEwMfDuBhRrw40771083;     qClJFtEwMfDuBhRrw40771083 = qClJFtEwMfDuBhRrw14391799;     qClJFtEwMfDuBhRrw14391799 = qClJFtEwMfDuBhRrw43379967;     qClJFtEwMfDuBhRrw43379967 = qClJFtEwMfDuBhRrw22814910;     qClJFtEwMfDuBhRrw22814910 = qClJFtEwMfDuBhRrw6123975;     qClJFtEwMfDuBhRrw6123975 = qClJFtEwMfDuBhRrw91312585;     qClJFtEwMfDuBhRrw91312585 = qClJFtEwMfDuBhRrw88360887;     qClJFtEwMfDuBhRrw88360887 = qClJFtEwMfDuBhRrw30027988;     qClJFtEwMfDuBhRrw30027988 = qClJFtEwMfDuBhRrw34790775;     qClJFtEwMfDuBhRrw34790775 = qClJFtEwMfDuBhRrw27210931;     qClJFtEwMfDuBhRrw27210931 = qClJFtEwMfDuBhRrw31419966;     qClJFtEwMfDuBhRrw31419966 = qClJFtEwMfDuBhRrw21321575;     qClJFtEwMfDuBhRrw21321575 = qClJFtEwMfDuBhRrw82863632;     qClJFtEwMfDuBhRrw82863632 = qClJFtEwMfDuBhRrw20320768;     qClJFtEwMfDuBhRrw20320768 = qClJFtEwMfDuBhRrw62287917;     qClJFtEwMfDuBhRrw62287917 = qClJFtEwMfDuBhRrw80590566;     qClJFtEwMfDuBhRrw80590566 = qClJFtEwMfDuBhRrw54798970;     qClJFtEwMfDuBhRrw54798970 = qClJFtEwMfDuBhRrw82035002;     qClJFtEwMfDuBhRrw82035002 = qClJFtEwMfDuBhRrw38969127;     qClJFtEwMfDuBhRrw38969127 = qClJFtEwMfDuBhRrw32373538;     qClJFtEwMfDuBhRrw32373538 = qClJFtEwMfDuBhRrw51885891;     qClJFtEwMfDuBhRrw51885891 = qClJFtEwMfDuBhRrw22266367;     qClJFtEwMfDuBhRrw22266367 = qClJFtEwMfDuBhRrw38678326;     qClJFtEwMfDuBhRrw38678326 = qClJFtEwMfDuBhRrw50366495;     qClJFtEwMfDuBhRrw50366495 = qClJFtEwMfDuBhRrw10791774;     qClJFtEwMfDuBhRrw10791774 = qClJFtEwMfDuBhRrw62790518;     qClJFtEwMfDuBhRrw62790518 = qClJFtEwMfDuBhRrw95855857;     qClJFtEwMfDuBhRrw95855857 = qClJFtEwMfDuBhRrw7149358;     qClJFtEwMfDuBhRrw7149358 = qClJFtEwMfDuBhRrw50009496;     qClJFtEwMfDuBhRrw50009496 = qClJFtEwMfDuBhRrw24263123;     qClJFtEwMfDuBhRrw24263123 = qClJFtEwMfDuBhRrw46414284;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rgTxziTTluaGuxZZ11588358() {     double tjwEHGoWmLyjLpsAp68801401 = -311715867;    double tjwEHGoWmLyjLpsAp85951969 = -954065506;    double tjwEHGoWmLyjLpsAp38331211 = -600428956;    double tjwEHGoWmLyjLpsAp89308805 = -296603888;    double tjwEHGoWmLyjLpsAp65606717 = -481981880;    double tjwEHGoWmLyjLpsAp72800309 = -76493251;    double tjwEHGoWmLyjLpsAp6876158 = -716009351;    double tjwEHGoWmLyjLpsAp726156 = -184622762;    double tjwEHGoWmLyjLpsAp30471122 = -383835553;    double tjwEHGoWmLyjLpsAp21042695 = -376585225;    double tjwEHGoWmLyjLpsAp40288436 = -147445639;    double tjwEHGoWmLyjLpsAp16760916 = -196232710;    double tjwEHGoWmLyjLpsAp49306717 = -436313239;    double tjwEHGoWmLyjLpsAp81407062 = -831042447;    double tjwEHGoWmLyjLpsAp97411958 = -777466377;    double tjwEHGoWmLyjLpsAp49575417 = -102335940;    double tjwEHGoWmLyjLpsAp93437707 = -160037019;    double tjwEHGoWmLyjLpsAp17507511 = -980341348;    double tjwEHGoWmLyjLpsAp12512763 = -881473310;    double tjwEHGoWmLyjLpsAp31861936 = 13285701;    double tjwEHGoWmLyjLpsAp51226986 = -403790211;    double tjwEHGoWmLyjLpsAp44670545 = 92177063;    double tjwEHGoWmLyjLpsAp79005024 = -298712036;    double tjwEHGoWmLyjLpsAp93779664 = -370643796;    double tjwEHGoWmLyjLpsAp19298132 = -280425688;    double tjwEHGoWmLyjLpsAp37950825 = -140342351;    double tjwEHGoWmLyjLpsAp67382164 = 75261709;    double tjwEHGoWmLyjLpsAp36200709 = -865306256;    double tjwEHGoWmLyjLpsAp68809422 = -112002623;    double tjwEHGoWmLyjLpsAp57120088 = -443627848;    double tjwEHGoWmLyjLpsAp77895698 = -31292139;    double tjwEHGoWmLyjLpsAp52061788 = -758744519;    double tjwEHGoWmLyjLpsAp14684965 = -649140085;    double tjwEHGoWmLyjLpsAp22957319 = -108322685;    double tjwEHGoWmLyjLpsAp74161859 = 12875717;    double tjwEHGoWmLyjLpsAp19471529 = -964253301;    double tjwEHGoWmLyjLpsAp57988134 = -37635516;    double tjwEHGoWmLyjLpsAp82848658 = -665929158;    double tjwEHGoWmLyjLpsAp16459259 = -86592706;    double tjwEHGoWmLyjLpsAp68601936 = -135172072;    double tjwEHGoWmLyjLpsAp75979049 = -722659392;    double tjwEHGoWmLyjLpsAp47539684 = -341093899;    double tjwEHGoWmLyjLpsAp98044631 = -485506043;    double tjwEHGoWmLyjLpsAp45127832 = -934859359;    double tjwEHGoWmLyjLpsAp472370 = -202213329;    double tjwEHGoWmLyjLpsAp64647236 = -565996875;    double tjwEHGoWmLyjLpsAp30525262 = -315884298;    double tjwEHGoWmLyjLpsAp43569844 = 72260278;    double tjwEHGoWmLyjLpsAp41885531 = -245696594;    double tjwEHGoWmLyjLpsAp38679840 = -102642352;    double tjwEHGoWmLyjLpsAp94544208 = -681623379;    double tjwEHGoWmLyjLpsAp26865584 = -847806196;    double tjwEHGoWmLyjLpsAp18036030 = -846897117;    double tjwEHGoWmLyjLpsAp11897837 = -891496507;    double tjwEHGoWmLyjLpsAp60361372 = -361506859;    double tjwEHGoWmLyjLpsAp24130856 = -303892930;    double tjwEHGoWmLyjLpsAp6946946 = -555353471;    double tjwEHGoWmLyjLpsAp44551546 = -129785160;    double tjwEHGoWmLyjLpsAp70010673 = 83821800;    double tjwEHGoWmLyjLpsAp27655893 = -241639530;    double tjwEHGoWmLyjLpsAp5418145 = -51754960;    double tjwEHGoWmLyjLpsAp70675449 = -850703096;    double tjwEHGoWmLyjLpsAp31916733 = 27379861;    double tjwEHGoWmLyjLpsAp73351033 = -940207705;    double tjwEHGoWmLyjLpsAp43146996 = -245293086;    double tjwEHGoWmLyjLpsAp88226648 = -388701120;    double tjwEHGoWmLyjLpsAp2075952 = -547092626;    double tjwEHGoWmLyjLpsAp26349399 = -227990554;    double tjwEHGoWmLyjLpsAp7245204 = -743918165;    double tjwEHGoWmLyjLpsAp77940430 = -813213077;    double tjwEHGoWmLyjLpsAp91587283 = 35299576;    double tjwEHGoWmLyjLpsAp10589050 = -494107862;    double tjwEHGoWmLyjLpsAp1048253 = -793748642;    double tjwEHGoWmLyjLpsAp43910826 = -646301239;    double tjwEHGoWmLyjLpsAp55882887 = -264054908;    double tjwEHGoWmLyjLpsAp3687302 = 37303688;    double tjwEHGoWmLyjLpsAp46625914 = -422316894;    double tjwEHGoWmLyjLpsAp33877192 = -363852677;    double tjwEHGoWmLyjLpsAp93307295 = -68430468;    double tjwEHGoWmLyjLpsAp54650896 = -714428814;    double tjwEHGoWmLyjLpsAp7425564 = -824458053;    double tjwEHGoWmLyjLpsAp23812321 = -996998570;    double tjwEHGoWmLyjLpsAp94315177 = -519609662;    double tjwEHGoWmLyjLpsAp30129583 = 90639728;    double tjwEHGoWmLyjLpsAp62575880 = -762004470;    double tjwEHGoWmLyjLpsAp51030114 = -183485944;    double tjwEHGoWmLyjLpsAp34025758 = -911847403;    double tjwEHGoWmLyjLpsAp2787128 = -757643579;    double tjwEHGoWmLyjLpsAp62595947 = -746815827;    double tjwEHGoWmLyjLpsAp50031003 = -683231353;    double tjwEHGoWmLyjLpsAp12524584 = -308899830;    double tjwEHGoWmLyjLpsAp13436588 = -907850357;    double tjwEHGoWmLyjLpsAp12837985 = -649750958;    double tjwEHGoWmLyjLpsAp88803366 = -844953177;    double tjwEHGoWmLyjLpsAp63183792 = 16582888;    double tjwEHGoWmLyjLpsAp5303601 = -871956296;    double tjwEHGoWmLyjLpsAp15622951 = -268473761;    double tjwEHGoWmLyjLpsAp24693598 = -545298339;    double tjwEHGoWmLyjLpsAp1980836 = -589566273;    double tjwEHGoWmLyjLpsAp12245721 = -311715867;     tjwEHGoWmLyjLpsAp68801401 = tjwEHGoWmLyjLpsAp85951969;     tjwEHGoWmLyjLpsAp85951969 = tjwEHGoWmLyjLpsAp38331211;     tjwEHGoWmLyjLpsAp38331211 = tjwEHGoWmLyjLpsAp89308805;     tjwEHGoWmLyjLpsAp89308805 = tjwEHGoWmLyjLpsAp65606717;     tjwEHGoWmLyjLpsAp65606717 = tjwEHGoWmLyjLpsAp72800309;     tjwEHGoWmLyjLpsAp72800309 = tjwEHGoWmLyjLpsAp6876158;     tjwEHGoWmLyjLpsAp6876158 = tjwEHGoWmLyjLpsAp726156;     tjwEHGoWmLyjLpsAp726156 = tjwEHGoWmLyjLpsAp30471122;     tjwEHGoWmLyjLpsAp30471122 = tjwEHGoWmLyjLpsAp21042695;     tjwEHGoWmLyjLpsAp21042695 = tjwEHGoWmLyjLpsAp40288436;     tjwEHGoWmLyjLpsAp40288436 = tjwEHGoWmLyjLpsAp16760916;     tjwEHGoWmLyjLpsAp16760916 = tjwEHGoWmLyjLpsAp49306717;     tjwEHGoWmLyjLpsAp49306717 = tjwEHGoWmLyjLpsAp81407062;     tjwEHGoWmLyjLpsAp81407062 = tjwEHGoWmLyjLpsAp97411958;     tjwEHGoWmLyjLpsAp97411958 = tjwEHGoWmLyjLpsAp49575417;     tjwEHGoWmLyjLpsAp49575417 = tjwEHGoWmLyjLpsAp93437707;     tjwEHGoWmLyjLpsAp93437707 = tjwEHGoWmLyjLpsAp17507511;     tjwEHGoWmLyjLpsAp17507511 = tjwEHGoWmLyjLpsAp12512763;     tjwEHGoWmLyjLpsAp12512763 = tjwEHGoWmLyjLpsAp31861936;     tjwEHGoWmLyjLpsAp31861936 = tjwEHGoWmLyjLpsAp51226986;     tjwEHGoWmLyjLpsAp51226986 = tjwEHGoWmLyjLpsAp44670545;     tjwEHGoWmLyjLpsAp44670545 = tjwEHGoWmLyjLpsAp79005024;     tjwEHGoWmLyjLpsAp79005024 = tjwEHGoWmLyjLpsAp93779664;     tjwEHGoWmLyjLpsAp93779664 = tjwEHGoWmLyjLpsAp19298132;     tjwEHGoWmLyjLpsAp19298132 = tjwEHGoWmLyjLpsAp37950825;     tjwEHGoWmLyjLpsAp37950825 = tjwEHGoWmLyjLpsAp67382164;     tjwEHGoWmLyjLpsAp67382164 = tjwEHGoWmLyjLpsAp36200709;     tjwEHGoWmLyjLpsAp36200709 = tjwEHGoWmLyjLpsAp68809422;     tjwEHGoWmLyjLpsAp68809422 = tjwEHGoWmLyjLpsAp57120088;     tjwEHGoWmLyjLpsAp57120088 = tjwEHGoWmLyjLpsAp77895698;     tjwEHGoWmLyjLpsAp77895698 = tjwEHGoWmLyjLpsAp52061788;     tjwEHGoWmLyjLpsAp52061788 = tjwEHGoWmLyjLpsAp14684965;     tjwEHGoWmLyjLpsAp14684965 = tjwEHGoWmLyjLpsAp22957319;     tjwEHGoWmLyjLpsAp22957319 = tjwEHGoWmLyjLpsAp74161859;     tjwEHGoWmLyjLpsAp74161859 = tjwEHGoWmLyjLpsAp19471529;     tjwEHGoWmLyjLpsAp19471529 = tjwEHGoWmLyjLpsAp57988134;     tjwEHGoWmLyjLpsAp57988134 = tjwEHGoWmLyjLpsAp82848658;     tjwEHGoWmLyjLpsAp82848658 = tjwEHGoWmLyjLpsAp16459259;     tjwEHGoWmLyjLpsAp16459259 = tjwEHGoWmLyjLpsAp68601936;     tjwEHGoWmLyjLpsAp68601936 = tjwEHGoWmLyjLpsAp75979049;     tjwEHGoWmLyjLpsAp75979049 = tjwEHGoWmLyjLpsAp47539684;     tjwEHGoWmLyjLpsAp47539684 = tjwEHGoWmLyjLpsAp98044631;     tjwEHGoWmLyjLpsAp98044631 = tjwEHGoWmLyjLpsAp45127832;     tjwEHGoWmLyjLpsAp45127832 = tjwEHGoWmLyjLpsAp472370;     tjwEHGoWmLyjLpsAp472370 = tjwEHGoWmLyjLpsAp64647236;     tjwEHGoWmLyjLpsAp64647236 = tjwEHGoWmLyjLpsAp30525262;     tjwEHGoWmLyjLpsAp30525262 = tjwEHGoWmLyjLpsAp43569844;     tjwEHGoWmLyjLpsAp43569844 = tjwEHGoWmLyjLpsAp41885531;     tjwEHGoWmLyjLpsAp41885531 = tjwEHGoWmLyjLpsAp38679840;     tjwEHGoWmLyjLpsAp38679840 = tjwEHGoWmLyjLpsAp94544208;     tjwEHGoWmLyjLpsAp94544208 = tjwEHGoWmLyjLpsAp26865584;     tjwEHGoWmLyjLpsAp26865584 = tjwEHGoWmLyjLpsAp18036030;     tjwEHGoWmLyjLpsAp18036030 = tjwEHGoWmLyjLpsAp11897837;     tjwEHGoWmLyjLpsAp11897837 = tjwEHGoWmLyjLpsAp60361372;     tjwEHGoWmLyjLpsAp60361372 = tjwEHGoWmLyjLpsAp24130856;     tjwEHGoWmLyjLpsAp24130856 = tjwEHGoWmLyjLpsAp6946946;     tjwEHGoWmLyjLpsAp6946946 = tjwEHGoWmLyjLpsAp44551546;     tjwEHGoWmLyjLpsAp44551546 = tjwEHGoWmLyjLpsAp70010673;     tjwEHGoWmLyjLpsAp70010673 = tjwEHGoWmLyjLpsAp27655893;     tjwEHGoWmLyjLpsAp27655893 = tjwEHGoWmLyjLpsAp5418145;     tjwEHGoWmLyjLpsAp5418145 = tjwEHGoWmLyjLpsAp70675449;     tjwEHGoWmLyjLpsAp70675449 = tjwEHGoWmLyjLpsAp31916733;     tjwEHGoWmLyjLpsAp31916733 = tjwEHGoWmLyjLpsAp73351033;     tjwEHGoWmLyjLpsAp73351033 = tjwEHGoWmLyjLpsAp43146996;     tjwEHGoWmLyjLpsAp43146996 = tjwEHGoWmLyjLpsAp88226648;     tjwEHGoWmLyjLpsAp88226648 = tjwEHGoWmLyjLpsAp2075952;     tjwEHGoWmLyjLpsAp2075952 = tjwEHGoWmLyjLpsAp26349399;     tjwEHGoWmLyjLpsAp26349399 = tjwEHGoWmLyjLpsAp7245204;     tjwEHGoWmLyjLpsAp7245204 = tjwEHGoWmLyjLpsAp77940430;     tjwEHGoWmLyjLpsAp77940430 = tjwEHGoWmLyjLpsAp91587283;     tjwEHGoWmLyjLpsAp91587283 = tjwEHGoWmLyjLpsAp10589050;     tjwEHGoWmLyjLpsAp10589050 = tjwEHGoWmLyjLpsAp1048253;     tjwEHGoWmLyjLpsAp1048253 = tjwEHGoWmLyjLpsAp43910826;     tjwEHGoWmLyjLpsAp43910826 = tjwEHGoWmLyjLpsAp55882887;     tjwEHGoWmLyjLpsAp55882887 = tjwEHGoWmLyjLpsAp3687302;     tjwEHGoWmLyjLpsAp3687302 = tjwEHGoWmLyjLpsAp46625914;     tjwEHGoWmLyjLpsAp46625914 = tjwEHGoWmLyjLpsAp33877192;     tjwEHGoWmLyjLpsAp33877192 = tjwEHGoWmLyjLpsAp93307295;     tjwEHGoWmLyjLpsAp93307295 = tjwEHGoWmLyjLpsAp54650896;     tjwEHGoWmLyjLpsAp54650896 = tjwEHGoWmLyjLpsAp7425564;     tjwEHGoWmLyjLpsAp7425564 = tjwEHGoWmLyjLpsAp23812321;     tjwEHGoWmLyjLpsAp23812321 = tjwEHGoWmLyjLpsAp94315177;     tjwEHGoWmLyjLpsAp94315177 = tjwEHGoWmLyjLpsAp30129583;     tjwEHGoWmLyjLpsAp30129583 = tjwEHGoWmLyjLpsAp62575880;     tjwEHGoWmLyjLpsAp62575880 = tjwEHGoWmLyjLpsAp51030114;     tjwEHGoWmLyjLpsAp51030114 = tjwEHGoWmLyjLpsAp34025758;     tjwEHGoWmLyjLpsAp34025758 = tjwEHGoWmLyjLpsAp2787128;     tjwEHGoWmLyjLpsAp2787128 = tjwEHGoWmLyjLpsAp62595947;     tjwEHGoWmLyjLpsAp62595947 = tjwEHGoWmLyjLpsAp50031003;     tjwEHGoWmLyjLpsAp50031003 = tjwEHGoWmLyjLpsAp12524584;     tjwEHGoWmLyjLpsAp12524584 = tjwEHGoWmLyjLpsAp13436588;     tjwEHGoWmLyjLpsAp13436588 = tjwEHGoWmLyjLpsAp12837985;     tjwEHGoWmLyjLpsAp12837985 = tjwEHGoWmLyjLpsAp88803366;     tjwEHGoWmLyjLpsAp88803366 = tjwEHGoWmLyjLpsAp63183792;     tjwEHGoWmLyjLpsAp63183792 = tjwEHGoWmLyjLpsAp5303601;     tjwEHGoWmLyjLpsAp5303601 = tjwEHGoWmLyjLpsAp15622951;     tjwEHGoWmLyjLpsAp15622951 = tjwEHGoWmLyjLpsAp24693598;     tjwEHGoWmLyjLpsAp24693598 = tjwEHGoWmLyjLpsAp1980836;     tjwEHGoWmLyjLpsAp1980836 = tjwEHGoWmLyjLpsAp12245721;     tjwEHGoWmLyjLpsAp12245721 = tjwEHGoWmLyjLpsAp68801401;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ncmVwPiwTHxHLgRL26637426() {     double TrCeKrPFxInPCVPGv74918508 = -423617756;    double TrCeKrPFxInPCVPGv29325249 = -663665287;    double TrCeKrPFxInPCVPGv54373394 = -751261877;    double TrCeKrPFxInPCVPGv16383586 = -487352518;    double TrCeKrPFxInPCVPGv34502206 = 97368248;    double TrCeKrPFxInPCVPGv64217511 = -340989486;    double TrCeKrPFxInPCVPGv67365123 = -854974329;    double TrCeKrPFxInPCVPGv50486047 = -471496675;    double TrCeKrPFxInPCVPGv95113616 = -151868218;    double TrCeKrPFxInPCVPGv62272354 = -705560703;    double TrCeKrPFxInPCVPGv398727 = -24883226;    double TrCeKrPFxInPCVPGv48583959 = -640638912;    double TrCeKrPFxInPCVPGv35515836 = -876204328;    double TrCeKrPFxInPCVPGv64550739 = -263281015;    double TrCeKrPFxInPCVPGv93154385 = -367928544;    double TrCeKrPFxInPCVPGv91080810 = -75335483;    double TrCeKrPFxInPCVPGv87273710 = -776582132;    double TrCeKrPFxInPCVPGv14560664 = -645863991;    double TrCeKrPFxInPCVPGv74342604 = -187242407;    double TrCeKrPFxInPCVPGv12412101 = -343617218;    double TrCeKrPFxInPCVPGv39780179 = -259277666;    double TrCeKrPFxInPCVPGv31191569 = -915752654;    double TrCeKrPFxInPCVPGv14595816 = -45229523;    double TrCeKrPFxInPCVPGv77772038 = -592948784;    double TrCeKrPFxInPCVPGv62997782 = -653621721;    double TrCeKrPFxInPCVPGv62672500 = -18997674;    double TrCeKrPFxInPCVPGv65841943 = -925400495;    double TrCeKrPFxInPCVPGv49307834 = -931235210;    double TrCeKrPFxInPCVPGv34941678 = -334385328;    double TrCeKrPFxInPCVPGv25735725 = -261098716;    double TrCeKrPFxInPCVPGv70138247 = 55386923;    double TrCeKrPFxInPCVPGv49672410 = -468191335;    double TrCeKrPFxInPCVPGv24395913 = -541753455;    double TrCeKrPFxInPCVPGv17787884 = -959864450;    double TrCeKrPFxInPCVPGv35636511 = -625857388;    double TrCeKrPFxInPCVPGv88197442 = -78279408;    double TrCeKrPFxInPCVPGv39250069 = 8728614;    double TrCeKrPFxInPCVPGv16938871 = -437435167;    double TrCeKrPFxInPCVPGv90069512 = -80516203;    double TrCeKrPFxInPCVPGv56103874 = -608611294;    double TrCeKrPFxInPCVPGv81648273 = -787933286;    double TrCeKrPFxInPCVPGv79690141 = -611236177;    double TrCeKrPFxInPCVPGv76111609 = -566310627;    double TrCeKrPFxInPCVPGv32535573 = -954202297;    double TrCeKrPFxInPCVPGv12424835 = -964111483;    double TrCeKrPFxInPCVPGv76586431 = -551212991;    double TrCeKrPFxInPCVPGv48303836 = -434659836;    double TrCeKrPFxInPCVPGv51526785 = 62364745;    double TrCeKrPFxInPCVPGv51561260 = -331433583;    double TrCeKrPFxInPCVPGv51591492 = -189653160;    double TrCeKrPFxInPCVPGv30244887 = -523917050;    double TrCeKrPFxInPCVPGv37675304 = -648532516;    double TrCeKrPFxInPCVPGv66284487 = -638686705;    double TrCeKrPFxInPCVPGv7866717 = -111966652;    double TrCeKrPFxInPCVPGv86857246 = 37961359;    double TrCeKrPFxInPCVPGv43726939 = -507865103;    double TrCeKrPFxInPCVPGv14729433 = -518435765;    double TrCeKrPFxInPCVPGv76601356 = -58313093;    double TrCeKrPFxInPCVPGv53385804 = -833730798;    double TrCeKrPFxInPCVPGv71829706 = -883634078;    double TrCeKrPFxInPCVPGv98375568 = -415588992;    double TrCeKrPFxInPCVPGv18057290 = -923739119;    double TrCeKrPFxInPCVPGv15544369 = -37111347;    double TrCeKrPFxInPCVPGv69377892 = -890769503;    double TrCeKrPFxInPCVPGv92134106 = -660947626;    double TrCeKrPFxInPCVPGv50726317 = -556691891;    double TrCeKrPFxInPCVPGv24188046 = 1114543;    double TrCeKrPFxInPCVPGv17727953 = -916339878;    double TrCeKrPFxInPCVPGv28914228 = -637423627;    double TrCeKrPFxInPCVPGv4956944 = -189649136;    double TrCeKrPFxInPCVPGv51830742 = 15935903;    double TrCeKrPFxInPCVPGv70334840 = -239146965;    double TrCeKrPFxInPCVPGv24491152 = -465347788;    double TrCeKrPFxInPCVPGv18238730 = -578631113;    double TrCeKrPFxInPCVPGv30763828 = -555683933;    double TrCeKrPFxInPCVPGv60090038 = -648041489;    double TrCeKrPFxInPCVPGv55079959 = -249442027;    double TrCeKrPFxInPCVPGv82060243 = -91027226;    double TrCeKrPFxInPCVPGv65347204 = -628837301;    double TrCeKrPFxInPCVPGv86411350 = -2408730;    double TrCeKrPFxInPCVPGv14368665 = -584337839;    double TrCeKrPFxInPCVPGv14315158 = -887765240;    double TrCeKrPFxInPCVPGv97746574 = -499801628;    double TrCeKrPFxInPCVPGv83350186 = -44732168;    double TrCeKrPFxInPCVPGv95490838 = -737181666;    double TrCeKrPFxInPCVPGv32462944 = -296080562;    double TrCeKrPFxInPCVPGv83387922 = -829504631;    double TrCeKrPFxInPCVPGv16529196 = -329786804;    double TrCeKrPFxInPCVPGv30930637 = -897825810;    double TrCeKrPFxInPCVPGv91909572 = -17992286;    double TrCeKrPFxInPCVPGv73468009 = -559843644;    double TrCeKrPFxInPCVPGv62648712 = -932958293;    double TrCeKrPFxInPCVPGv63553066 = -603704370;    double TrCeKrPFxInPCVPGv18239807 = -196882126;    double TrCeKrPFxInPCVPGv57728305 = -93022303;    double TrCeKrPFxInPCVPGv63590983 = -864194167;    double TrCeKrPFxInPCVPGv64145772 = -474124830;    double TrCeKrPFxInPCVPGv6733718 = -675541125;    double TrCeKrPFxInPCVPGv40401467 = -193254672;    double TrCeKrPFxInPCVPGv61698517 = -423617756;     TrCeKrPFxInPCVPGv74918508 = TrCeKrPFxInPCVPGv29325249;     TrCeKrPFxInPCVPGv29325249 = TrCeKrPFxInPCVPGv54373394;     TrCeKrPFxInPCVPGv54373394 = TrCeKrPFxInPCVPGv16383586;     TrCeKrPFxInPCVPGv16383586 = TrCeKrPFxInPCVPGv34502206;     TrCeKrPFxInPCVPGv34502206 = TrCeKrPFxInPCVPGv64217511;     TrCeKrPFxInPCVPGv64217511 = TrCeKrPFxInPCVPGv67365123;     TrCeKrPFxInPCVPGv67365123 = TrCeKrPFxInPCVPGv50486047;     TrCeKrPFxInPCVPGv50486047 = TrCeKrPFxInPCVPGv95113616;     TrCeKrPFxInPCVPGv95113616 = TrCeKrPFxInPCVPGv62272354;     TrCeKrPFxInPCVPGv62272354 = TrCeKrPFxInPCVPGv398727;     TrCeKrPFxInPCVPGv398727 = TrCeKrPFxInPCVPGv48583959;     TrCeKrPFxInPCVPGv48583959 = TrCeKrPFxInPCVPGv35515836;     TrCeKrPFxInPCVPGv35515836 = TrCeKrPFxInPCVPGv64550739;     TrCeKrPFxInPCVPGv64550739 = TrCeKrPFxInPCVPGv93154385;     TrCeKrPFxInPCVPGv93154385 = TrCeKrPFxInPCVPGv91080810;     TrCeKrPFxInPCVPGv91080810 = TrCeKrPFxInPCVPGv87273710;     TrCeKrPFxInPCVPGv87273710 = TrCeKrPFxInPCVPGv14560664;     TrCeKrPFxInPCVPGv14560664 = TrCeKrPFxInPCVPGv74342604;     TrCeKrPFxInPCVPGv74342604 = TrCeKrPFxInPCVPGv12412101;     TrCeKrPFxInPCVPGv12412101 = TrCeKrPFxInPCVPGv39780179;     TrCeKrPFxInPCVPGv39780179 = TrCeKrPFxInPCVPGv31191569;     TrCeKrPFxInPCVPGv31191569 = TrCeKrPFxInPCVPGv14595816;     TrCeKrPFxInPCVPGv14595816 = TrCeKrPFxInPCVPGv77772038;     TrCeKrPFxInPCVPGv77772038 = TrCeKrPFxInPCVPGv62997782;     TrCeKrPFxInPCVPGv62997782 = TrCeKrPFxInPCVPGv62672500;     TrCeKrPFxInPCVPGv62672500 = TrCeKrPFxInPCVPGv65841943;     TrCeKrPFxInPCVPGv65841943 = TrCeKrPFxInPCVPGv49307834;     TrCeKrPFxInPCVPGv49307834 = TrCeKrPFxInPCVPGv34941678;     TrCeKrPFxInPCVPGv34941678 = TrCeKrPFxInPCVPGv25735725;     TrCeKrPFxInPCVPGv25735725 = TrCeKrPFxInPCVPGv70138247;     TrCeKrPFxInPCVPGv70138247 = TrCeKrPFxInPCVPGv49672410;     TrCeKrPFxInPCVPGv49672410 = TrCeKrPFxInPCVPGv24395913;     TrCeKrPFxInPCVPGv24395913 = TrCeKrPFxInPCVPGv17787884;     TrCeKrPFxInPCVPGv17787884 = TrCeKrPFxInPCVPGv35636511;     TrCeKrPFxInPCVPGv35636511 = TrCeKrPFxInPCVPGv88197442;     TrCeKrPFxInPCVPGv88197442 = TrCeKrPFxInPCVPGv39250069;     TrCeKrPFxInPCVPGv39250069 = TrCeKrPFxInPCVPGv16938871;     TrCeKrPFxInPCVPGv16938871 = TrCeKrPFxInPCVPGv90069512;     TrCeKrPFxInPCVPGv90069512 = TrCeKrPFxInPCVPGv56103874;     TrCeKrPFxInPCVPGv56103874 = TrCeKrPFxInPCVPGv81648273;     TrCeKrPFxInPCVPGv81648273 = TrCeKrPFxInPCVPGv79690141;     TrCeKrPFxInPCVPGv79690141 = TrCeKrPFxInPCVPGv76111609;     TrCeKrPFxInPCVPGv76111609 = TrCeKrPFxInPCVPGv32535573;     TrCeKrPFxInPCVPGv32535573 = TrCeKrPFxInPCVPGv12424835;     TrCeKrPFxInPCVPGv12424835 = TrCeKrPFxInPCVPGv76586431;     TrCeKrPFxInPCVPGv76586431 = TrCeKrPFxInPCVPGv48303836;     TrCeKrPFxInPCVPGv48303836 = TrCeKrPFxInPCVPGv51526785;     TrCeKrPFxInPCVPGv51526785 = TrCeKrPFxInPCVPGv51561260;     TrCeKrPFxInPCVPGv51561260 = TrCeKrPFxInPCVPGv51591492;     TrCeKrPFxInPCVPGv51591492 = TrCeKrPFxInPCVPGv30244887;     TrCeKrPFxInPCVPGv30244887 = TrCeKrPFxInPCVPGv37675304;     TrCeKrPFxInPCVPGv37675304 = TrCeKrPFxInPCVPGv66284487;     TrCeKrPFxInPCVPGv66284487 = TrCeKrPFxInPCVPGv7866717;     TrCeKrPFxInPCVPGv7866717 = TrCeKrPFxInPCVPGv86857246;     TrCeKrPFxInPCVPGv86857246 = TrCeKrPFxInPCVPGv43726939;     TrCeKrPFxInPCVPGv43726939 = TrCeKrPFxInPCVPGv14729433;     TrCeKrPFxInPCVPGv14729433 = TrCeKrPFxInPCVPGv76601356;     TrCeKrPFxInPCVPGv76601356 = TrCeKrPFxInPCVPGv53385804;     TrCeKrPFxInPCVPGv53385804 = TrCeKrPFxInPCVPGv71829706;     TrCeKrPFxInPCVPGv71829706 = TrCeKrPFxInPCVPGv98375568;     TrCeKrPFxInPCVPGv98375568 = TrCeKrPFxInPCVPGv18057290;     TrCeKrPFxInPCVPGv18057290 = TrCeKrPFxInPCVPGv15544369;     TrCeKrPFxInPCVPGv15544369 = TrCeKrPFxInPCVPGv69377892;     TrCeKrPFxInPCVPGv69377892 = TrCeKrPFxInPCVPGv92134106;     TrCeKrPFxInPCVPGv92134106 = TrCeKrPFxInPCVPGv50726317;     TrCeKrPFxInPCVPGv50726317 = TrCeKrPFxInPCVPGv24188046;     TrCeKrPFxInPCVPGv24188046 = TrCeKrPFxInPCVPGv17727953;     TrCeKrPFxInPCVPGv17727953 = TrCeKrPFxInPCVPGv28914228;     TrCeKrPFxInPCVPGv28914228 = TrCeKrPFxInPCVPGv4956944;     TrCeKrPFxInPCVPGv4956944 = TrCeKrPFxInPCVPGv51830742;     TrCeKrPFxInPCVPGv51830742 = TrCeKrPFxInPCVPGv70334840;     TrCeKrPFxInPCVPGv70334840 = TrCeKrPFxInPCVPGv24491152;     TrCeKrPFxInPCVPGv24491152 = TrCeKrPFxInPCVPGv18238730;     TrCeKrPFxInPCVPGv18238730 = TrCeKrPFxInPCVPGv30763828;     TrCeKrPFxInPCVPGv30763828 = TrCeKrPFxInPCVPGv60090038;     TrCeKrPFxInPCVPGv60090038 = TrCeKrPFxInPCVPGv55079959;     TrCeKrPFxInPCVPGv55079959 = TrCeKrPFxInPCVPGv82060243;     TrCeKrPFxInPCVPGv82060243 = TrCeKrPFxInPCVPGv65347204;     TrCeKrPFxInPCVPGv65347204 = TrCeKrPFxInPCVPGv86411350;     TrCeKrPFxInPCVPGv86411350 = TrCeKrPFxInPCVPGv14368665;     TrCeKrPFxInPCVPGv14368665 = TrCeKrPFxInPCVPGv14315158;     TrCeKrPFxInPCVPGv14315158 = TrCeKrPFxInPCVPGv97746574;     TrCeKrPFxInPCVPGv97746574 = TrCeKrPFxInPCVPGv83350186;     TrCeKrPFxInPCVPGv83350186 = TrCeKrPFxInPCVPGv95490838;     TrCeKrPFxInPCVPGv95490838 = TrCeKrPFxInPCVPGv32462944;     TrCeKrPFxInPCVPGv32462944 = TrCeKrPFxInPCVPGv83387922;     TrCeKrPFxInPCVPGv83387922 = TrCeKrPFxInPCVPGv16529196;     TrCeKrPFxInPCVPGv16529196 = TrCeKrPFxInPCVPGv30930637;     TrCeKrPFxInPCVPGv30930637 = TrCeKrPFxInPCVPGv91909572;     TrCeKrPFxInPCVPGv91909572 = TrCeKrPFxInPCVPGv73468009;     TrCeKrPFxInPCVPGv73468009 = TrCeKrPFxInPCVPGv62648712;     TrCeKrPFxInPCVPGv62648712 = TrCeKrPFxInPCVPGv63553066;     TrCeKrPFxInPCVPGv63553066 = TrCeKrPFxInPCVPGv18239807;     TrCeKrPFxInPCVPGv18239807 = TrCeKrPFxInPCVPGv57728305;     TrCeKrPFxInPCVPGv57728305 = TrCeKrPFxInPCVPGv63590983;     TrCeKrPFxInPCVPGv63590983 = TrCeKrPFxInPCVPGv64145772;     TrCeKrPFxInPCVPGv64145772 = TrCeKrPFxInPCVPGv6733718;     TrCeKrPFxInPCVPGv6733718 = TrCeKrPFxInPCVPGv40401467;     TrCeKrPFxInPCVPGv40401467 = TrCeKrPFxInPCVPGv61698517;     TrCeKrPFxInPCVPGv61698517 = TrCeKrPFxInPCVPGv74918508;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void waYSoGPrMbFWSxMu93929024() {     double JObSWuECsXqSeGlFN10377477 = -889894656;    double JObSWuECsXqSeGlFN42597328 = -544216893;    double JObSWuECsXqSeGlFN91759871 = -416928952;    double JObSWuECsXqSeGlFN88606940 = -308503590;    double JObSWuECsXqSeGlFN90787474 = -8230597;    double JObSWuECsXqSeGlFN440384 = -603337382;    double JObSWuECsXqSeGlFN78022530 = -699700063;    double JObSWuECsXqSeGlFN12617608 = -115730815;    double JObSWuECsXqSeGlFN96631222 = -640199625;    double JObSWuECsXqSeGlFN77175066 = -370247701;    double JObSWuECsXqSeGlFN38580461 = -926890955;    double JObSWuECsXqSeGlFN49152107 = -504759683;    double JObSWuECsXqSeGlFN73750481 = -328216833;    double JObSWuECsXqSeGlFN17691603 = -239644424;    double JObSWuECsXqSeGlFN51348902 = -871353559;    double JObSWuECsXqSeGlFN74622111 = -811147090;    double JObSWuECsXqSeGlFN43109646 = -203556684;    double JObSWuECsXqSeGlFN14320856 = -719292060;    double JObSWuECsXqSeGlFN74892934 = -503805828;    double JObSWuECsXqSeGlFN6249320 = -770178284;    double JObSWuECsXqSeGlFN52867042 = 83447645;    double JObSWuECsXqSeGlFN34613094 = 97611052;    double JObSWuECsXqSeGlFN85121195 = -782861707;    double JObSWuECsXqSeGlFN71564983 = -3773388;    double JObSWuECsXqSeGlFN71015733 = -677606919;    double JObSWuECsXqSeGlFN73370671 = -807574481;    double JObSWuECsXqSeGlFN26262517 = -731684635;    double JObSWuECsXqSeGlFN90036259 = -1007384;    double JObSWuECsXqSeGlFN81609892 = -908480130;    double JObSWuECsXqSeGlFN31269136 = -295605886;    double JObSWuECsXqSeGlFN88721479 = -496593564;    double JObSWuECsXqSeGlFN87495428 = -241162749;    double JObSWuECsXqSeGlFN36613924 = -689349433;    double JObSWuECsXqSeGlFN47536310 = 75441736;    double JObSWuECsXqSeGlFN1444965 = -576957639;    double JObSWuECsXqSeGlFN39758286 = -292844714;    double JObSWuECsXqSeGlFN88661885 = -955967426;    double JObSWuECsXqSeGlFN68154274 = -420090748;    double JObSWuECsXqSeGlFN20460074 = -831392480;    double JObSWuECsXqSeGlFN20375406 = -73873841;    double JObSWuECsXqSeGlFN15099881 = -557132200;    double JObSWuECsXqSeGlFN69055267 = -205781468;    double JObSWuECsXqSeGlFN65494988 = -119491413;    double JObSWuECsXqSeGlFN72666911 = -458534707;    double JObSWuECsXqSeGlFN12441293 = -790622645;    double JObSWuECsXqSeGlFN22547760 = -590136623;    double JObSWuECsXqSeGlFN67648999 = -918646196;    double JObSWuECsXqSeGlFN69253096 = -104161848;    double JObSWuECsXqSeGlFN74453926 = 4829537;    double JObSWuECsXqSeGlFN76121081 = -680679976;    double JObSWuECsXqSeGlFN11015611 = -398193607;    double JObSWuECsXqSeGlFN21160776 = -862184862;    double JObSWuECsXqSeGlFN34905636 = -394732259;    double JObSWuECsXqSeGlFN23858564 = -303475151;    double JObSWuECsXqSeGlFN41061983 = -308528787;    double JObSWuECsXqSeGlFN75764383 = -887505708;    double JObSWuECsXqSeGlFN57476132 = -761355187;    double JObSWuECsXqSeGlFN20194888 = -313155564;    double JObSWuECsXqSeGlFN17591208 = -630896671;    double JObSWuECsXqSeGlFN17416804 = -200656117;    double JObSWuECsXqSeGlFN74177866 = -871652747;    double JObSWuECsXqSeGlFN87986270 = -598692679;    double JObSWuECsXqSeGlFN31007715 = -207250685;    double JObSWuECsXqSeGlFN65362086 = -244593740;    double JObSWuECsXqSeGlFN88453586 = -873654137;    double JObSWuECsXqSeGlFN51085033 = -585728206;    double JObSWuECsXqSeGlFN12538184 = -815410251;    double JObSWuECsXqSeGlFN26214172 = -303658569;    double JObSWuECsXqSeGlFN16246638 = -662686786;    double JObSWuECsXqSeGlFN11590616 = -478508845;    double JObSWuECsXqSeGlFN85960226 = -855179664;    double JObSWuECsXqSeGlFN74955372 = -783465936;    double JObSWuECsXqSeGlFN93860781 = -887899581;    double JObSWuECsXqSeGlFN54517528 = -329931988;    double JObSWuECsXqSeGlFN91149438 = -113046085;    double JObSWuECsXqSeGlFN83811774 = -710770887;    double JObSWuECsXqSeGlFN69118106 = -782897535;    double JObSWuECsXqSeGlFN12454285 = -224327000;    double JObSWuECsXqSeGlFN59123690 = -213150744;    double JObSWuECsXqSeGlFN48467973 = 12529703;    double JObSWuECsXqSeGlFN5721673 = -888928285;    double JObSWuECsXqSeGlFN57009420 = -527522788;    double JObSWuECsXqSeGlFN15582333 = 94163078;    double JObSWuECsXqSeGlFN5488812 = -127800155;    double JObSWuECsXqSeGlFN20253526 = -897412279;    double JObSWuECsXqSeGlFN67560703 = -634408703;    double JObSWuECsXqSeGlFN52589792 = -846430491;    double JObSWuECsXqSeGlFN12755361 = -285874282;    double JObSWuECsXqSeGlFN6474327 = -616029477;    double JObSWuECsXqSeGlFN25680582 = -689451931;    double JObSWuECsXqSeGlFN82282154 = -531489527;    double JObSWuECsXqSeGlFN68466997 = -542811863;    double JObSWuECsXqSeGlFN50563066 = -789194078;    double JObSWuECsXqSeGlFN3043271 = -530736364;    double JObSWuECsXqSeGlFN46197540 = -202221094;    double JObSWuECsXqSeGlFN27113610 = -958439521;    double JObSWuECsXqSeGlFN38047553 = -998530783;    double JObSWuECsXqSeGlFN132902 = -874897674;    double JObSWuECsXqSeGlFN84213324 = -584880571;    double JObSWuECsXqSeGlFN61356260 = -889894656;     JObSWuECsXqSeGlFN10377477 = JObSWuECsXqSeGlFN42597328;     JObSWuECsXqSeGlFN42597328 = JObSWuECsXqSeGlFN91759871;     JObSWuECsXqSeGlFN91759871 = JObSWuECsXqSeGlFN88606940;     JObSWuECsXqSeGlFN88606940 = JObSWuECsXqSeGlFN90787474;     JObSWuECsXqSeGlFN90787474 = JObSWuECsXqSeGlFN440384;     JObSWuECsXqSeGlFN440384 = JObSWuECsXqSeGlFN78022530;     JObSWuECsXqSeGlFN78022530 = JObSWuECsXqSeGlFN12617608;     JObSWuECsXqSeGlFN12617608 = JObSWuECsXqSeGlFN96631222;     JObSWuECsXqSeGlFN96631222 = JObSWuECsXqSeGlFN77175066;     JObSWuECsXqSeGlFN77175066 = JObSWuECsXqSeGlFN38580461;     JObSWuECsXqSeGlFN38580461 = JObSWuECsXqSeGlFN49152107;     JObSWuECsXqSeGlFN49152107 = JObSWuECsXqSeGlFN73750481;     JObSWuECsXqSeGlFN73750481 = JObSWuECsXqSeGlFN17691603;     JObSWuECsXqSeGlFN17691603 = JObSWuECsXqSeGlFN51348902;     JObSWuECsXqSeGlFN51348902 = JObSWuECsXqSeGlFN74622111;     JObSWuECsXqSeGlFN74622111 = JObSWuECsXqSeGlFN43109646;     JObSWuECsXqSeGlFN43109646 = JObSWuECsXqSeGlFN14320856;     JObSWuECsXqSeGlFN14320856 = JObSWuECsXqSeGlFN74892934;     JObSWuECsXqSeGlFN74892934 = JObSWuECsXqSeGlFN6249320;     JObSWuECsXqSeGlFN6249320 = JObSWuECsXqSeGlFN52867042;     JObSWuECsXqSeGlFN52867042 = JObSWuECsXqSeGlFN34613094;     JObSWuECsXqSeGlFN34613094 = JObSWuECsXqSeGlFN85121195;     JObSWuECsXqSeGlFN85121195 = JObSWuECsXqSeGlFN71564983;     JObSWuECsXqSeGlFN71564983 = JObSWuECsXqSeGlFN71015733;     JObSWuECsXqSeGlFN71015733 = JObSWuECsXqSeGlFN73370671;     JObSWuECsXqSeGlFN73370671 = JObSWuECsXqSeGlFN26262517;     JObSWuECsXqSeGlFN26262517 = JObSWuECsXqSeGlFN90036259;     JObSWuECsXqSeGlFN90036259 = JObSWuECsXqSeGlFN81609892;     JObSWuECsXqSeGlFN81609892 = JObSWuECsXqSeGlFN31269136;     JObSWuECsXqSeGlFN31269136 = JObSWuECsXqSeGlFN88721479;     JObSWuECsXqSeGlFN88721479 = JObSWuECsXqSeGlFN87495428;     JObSWuECsXqSeGlFN87495428 = JObSWuECsXqSeGlFN36613924;     JObSWuECsXqSeGlFN36613924 = JObSWuECsXqSeGlFN47536310;     JObSWuECsXqSeGlFN47536310 = JObSWuECsXqSeGlFN1444965;     JObSWuECsXqSeGlFN1444965 = JObSWuECsXqSeGlFN39758286;     JObSWuECsXqSeGlFN39758286 = JObSWuECsXqSeGlFN88661885;     JObSWuECsXqSeGlFN88661885 = JObSWuECsXqSeGlFN68154274;     JObSWuECsXqSeGlFN68154274 = JObSWuECsXqSeGlFN20460074;     JObSWuECsXqSeGlFN20460074 = JObSWuECsXqSeGlFN20375406;     JObSWuECsXqSeGlFN20375406 = JObSWuECsXqSeGlFN15099881;     JObSWuECsXqSeGlFN15099881 = JObSWuECsXqSeGlFN69055267;     JObSWuECsXqSeGlFN69055267 = JObSWuECsXqSeGlFN65494988;     JObSWuECsXqSeGlFN65494988 = JObSWuECsXqSeGlFN72666911;     JObSWuECsXqSeGlFN72666911 = JObSWuECsXqSeGlFN12441293;     JObSWuECsXqSeGlFN12441293 = JObSWuECsXqSeGlFN22547760;     JObSWuECsXqSeGlFN22547760 = JObSWuECsXqSeGlFN67648999;     JObSWuECsXqSeGlFN67648999 = JObSWuECsXqSeGlFN69253096;     JObSWuECsXqSeGlFN69253096 = JObSWuECsXqSeGlFN74453926;     JObSWuECsXqSeGlFN74453926 = JObSWuECsXqSeGlFN76121081;     JObSWuECsXqSeGlFN76121081 = JObSWuECsXqSeGlFN11015611;     JObSWuECsXqSeGlFN11015611 = JObSWuECsXqSeGlFN21160776;     JObSWuECsXqSeGlFN21160776 = JObSWuECsXqSeGlFN34905636;     JObSWuECsXqSeGlFN34905636 = JObSWuECsXqSeGlFN23858564;     JObSWuECsXqSeGlFN23858564 = JObSWuECsXqSeGlFN41061983;     JObSWuECsXqSeGlFN41061983 = JObSWuECsXqSeGlFN75764383;     JObSWuECsXqSeGlFN75764383 = JObSWuECsXqSeGlFN57476132;     JObSWuECsXqSeGlFN57476132 = JObSWuECsXqSeGlFN20194888;     JObSWuECsXqSeGlFN20194888 = JObSWuECsXqSeGlFN17591208;     JObSWuECsXqSeGlFN17591208 = JObSWuECsXqSeGlFN17416804;     JObSWuECsXqSeGlFN17416804 = JObSWuECsXqSeGlFN74177866;     JObSWuECsXqSeGlFN74177866 = JObSWuECsXqSeGlFN87986270;     JObSWuECsXqSeGlFN87986270 = JObSWuECsXqSeGlFN31007715;     JObSWuECsXqSeGlFN31007715 = JObSWuECsXqSeGlFN65362086;     JObSWuECsXqSeGlFN65362086 = JObSWuECsXqSeGlFN88453586;     JObSWuECsXqSeGlFN88453586 = JObSWuECsXqSeGlFN51085033;     JObSWuECsXqSeGlFN51085033 = JObSWuECsXqSeGlFN12538184;     JObSWuECsXqSeGlFN12538184 = JObSWuECsXqSeGlFN26214172;     JObSWuECsXqSeGlFN26214172 = JObSWuECsXqSeGlFN16246638;     JObSWuECsXqSeGlFN16246638 = JObSWuECsXqSeGlFN11590616;     JObSWuECsXqSeGlFN11590616 = JObSWuECsXqSeGlFN85960226;     JObSWuECsXqSeGlFN85960226 = JObSWuECsXqSeGlFN74955372;     JObSWuECsXqSeGlFN74955372 = JObSWuECsXqSeGlFN93860781;     JObSWuECsXqSeGlFN93860781 = JObSWuECsXqSeGlFN54517528;     JObSWuECsXqSeGlFN54517528 = JObSWuECsXqSeGlFN91149438;     JObSWuECsXqSeGlFN91149438 = JObSWuECsXqSeGlFN83811774;     JObSWuECsXqSeGlFN83811774 = JObSWuECsXqSeGlFN69118106;     JObSWuECsXqSeGlFN69118106 = JObSWuECsXqSeGlFN12454285;     JObSWuECsXqSeGlFN12454285 = JObSWuECsXqSeGlFN59123690;     JObSWuECsXqSeGlFN59123690 = JObSWuECsXqSeGlFN48467973;     JObSWuECsXqSeGlFN48467973 = JObSWuECsXqSeGlFN5721673;     JObSWuECsXqSeGlFN5721673 = JObSWuECsXqSeGlFN57009420;     JObSWuECsXqSeGlFN57009420 = JObSWuECsXqSeGlFN15582333;     JObSWuECsXqSeGlFN15582333 = JObSWuECsXqSeGlFN5488812;     JObSWuECsXqSeGlFN5488812 = JObSWuECsXqSeGlFN20253526;     JObSWuECsXqSeGlFN20253526 = JObSWuECsXqSeGlFN67560703;     JObSWuECsXqSeGlFN67560703 = JObSWuECsXqSeGlFN52589792;     JObSWuECsXqSeGlFN52589792 = JObSWuECsXqSeGlFN12755361;     JObSWuECsXqSeGlFN12755361 = JObSWuECsXqSeGlFN6474327;     JObSWuECsXqSeGlFN6474327 = JObSWuECsXqSeGlFN25680582;     JObSWuECsXqSeGlFN25680582 = JObSWuECsXqSeGlFN82282154;     JObSWuECsXqSeGlFN82282154 = JObSWuECsXqSeGlFN68466997;     JObSWuECsXqSeGlFN68466997 = JObSWuECsXqSeGlFN50563066;     JObSWuECsXqSeGlFN50563066 = JObSWuECsXqSeGlFN3043271;     JObSWuECsXqSeGlFN3043271 = JObSWuECsXqSeGlFN46197540;     JObSWuECsXqSeGlFN46197540 = JObSWuECsXqSeGlFN27113610;     JObSWuECsXqSeGlFN27113610 = JObSWuECsXqSeGlFN38047553;     JObSWuECsXqSeGlFN38047553 = JObSWuECsXqSeGlFN132902;     JObSWuECsXqSeGlFN132902 = JObSWuECsXqSeGlFN84213324;     JObSWuECsXqSeGlFN84213324 = JObSWuECsXqSeGlFN61356260;     JObSWuECsXqSeGlFN61356260 = JObSWuECsXqSeGlFN10377477;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ftJjVAANSaQNIhjM30574000() {     double JfxsLqhtcwWRYBmww32764594 = -524298619;    double JfxsLqhtcwWRYBmww26944828 = -492759948;    double JfxsLqhtcwWRYBmww20529150 = -656582796;    double JfxsLqhtcwWRYBmww3613630 = -314911121;    double JfxsLqhtcwWRYBmww88961729 = -599287598;    double JfxsLqhtcwWRYBmww15323501 = -463945759;    double JfxsLqhtcwWRYBmww70178269 = -521687369;    double JfxsLqhtcwWRYBmww88251466 = -586327460;    double JfxsLqhtcwWRYBmww93794353 = -524395664;    double JfxsLqhtcwWRYBmww68938650 = -112989034;    double JfxsLqhtcwWRYBmww76122320 = -923515356;    double JfxsLqhtcwWRYBmww5055056 = -755504977;    double JfxsLqhtcwWRYBmww40758662 = -946934152;    double JfxsLqhtcwWRYBmww60306355 = -767353181;    double JfxsLqhtcwWRYBmww88084178 = -498831272;    double JfxsLqhtcwWRYBmww18878024 = -92814632;    double JfxsLqhtcwWRYBmww92932997 = -480836504;    double JfxsLqhtcwWRYBmww20297272 = -71034750;    double JfxsLqhtcwWRYBmww16174565 = -638907954;    double JfxsLqhtcwWRYBmww84765602 = 77187262;    double JfxsLqhtcwWRYBmww53750149 = 91960337;    double JfxsLqhtcwWRYBmww98428312 = -407155261;    double JfxsLqhtcwWRYBmww88414518 = -958942299;    double JfxsLqhtcwWRYBmww98064770 = -567766245;    double JfxsLqhtcwWRYBmww6555981 = -299166044;    double JfxsLqhtcwWRYBmww61673666 = -151468705;    double JfxsLqhtcwWRYBmww73351937 = -489271128;    double JfxsLqhtcwWRYBmww95947709 = -889461838;    double JfxsLqhtcwWRYBmww88502453 = -152737249;    double JfxsLqhtcwWRYBmww40426315 = -215901753;    double JfxsLqhtcwWRYBmww33012285 = -577909716;    double JfxsLqhtcwWRYBmww45036619 = -893234104;    double JfxsLqhtcwWRYBmww17652595 = -118692928;    double JfxsLqhtcwWRYBmww6924998 = -248685114;    double JfxsLqhtcwWRYBmww77674329 = -640714061;    double JfxsLqhtcwWRYBmww81451155 = -608240090;    double JfxsLqhtcwWRYBmww82101597 = -688915378;    double JfxsLqhtcwWRYBmww21780375 = -287716220;    double JfxsLqhtcwWRYBmww68768205 = -132438511;    double JfxsLqhtcwWRYBmww40561121 = -210097870;    double JfxsLqhtcwWRYBmww43857252 = 39690134;    double JfxsLqhtcwWRYBmww57563659 = 36309841;    double JfxsLqhtcwWRYBmww71045180 = -7021997;    double JfxsLqhtcwWRYBmww72111030 = -455898356;    double JfxsLqhtcwWRYBmww80424559 = -599766122;    double JfxsLqhtcwWRYBmww30648042 = -772365719;    double JfxsLqhtcwWRYBmww79946397 = -397056449;    double JfxsLqhtcwWRYBmww13851771 = -960696839;    double JfxsLqhtcwWRYBmww68913831 = 55112839;    double JfxsLqhtcwWRYBmww73204827 = -822700235;    double JfxsLqhtcwWRYBmww19884827 = -753269884;    double JfxsLqhtcwWRYBmww95012032 = -193004144;    double JfxsLqhtcwWRYBmww13220039 = -66643489;    double JfxsLqhtcwWRYBmww84145108 = -579155960;    double JfxsLqhtcwWRYBmww76823850 = -449232902;    double JfxsLqhtcwWRYBmww34336282 = -17143358;    double JfxsLqhtcwWRYBmww38530309 = -533817650;    double JfxsLqhtcwWRYBmww22464379 = 11183449;    double JfxsLqhtcwWRYBmww97057649 = 84254922;    double JfxsLqhtcwWRYBmww27288063 = -347818894;    double JfxsLqhtcwWRYBmww41971563 = -974674632;    double JfxsLqhtcwWRYBmww74230559 = -632225532;    double JfxsLqhtcwWRYBmww99749012 = -333590211;    double JfxsLqhtcwWRYBmww53368038 = -208493912;    double JfxsLqhtcwWRYBmww35926366 = -535079318;    double JfxsLqhtcwWRYBmww31085702 = 69718748;    double JfxsLqhtcwWRYBmww87402461 = -536812050;    double JfxsLqhtcwWRYBmww33833665 = -598249039;    double JfxsLqhtcwWRYBmww82632025 = -26639120;    double JfxsLqhtcwWRYBmww6633024 = -890591182;    double JfxsLqhtcwWRYBmww36776427 = -403899255;    double JfxsLqhtcwWRYBmww71152623 = -93120284;    double JfxsLqhtcwWRYBmww51529066 = -938596240;    double JfxsLqhtcwWRYBmww75613444 = -328810084;    double JfxsLqhtcwWRYBmww40908351 = -962502872;    double JfxsLqhtcwWRYBmww96186490 = -944349505;    double JfxsLqhtcwWRYBmww27383133 = -300133264;    double JfxsLqhtcwWRYBmww16303489 = -403043943;    double JfxsLqhtcwWRYBmww17640211 = -968000124;    double JfxsLqhtcwWRYBmww75907938 = -526800326;    double JfxsLqhtcwWRYBmww81727269 = -754412256;    double JfxsLqhtcwWRYBmww59500166 = -528574290;    double JfxsLqhtcwWRYBmww27033878 = -844574677;    double JfxsLqhtcwWRYBmww15297627 = -330037015;    double JfxsLqhtcwWRYBmww20541489 = -462631869;    double JfxsLqhtcwWRYBmww38000252 = -284905573;    double JfxsLqhtcwWRYBmww31816580 = -726590616;    double JfxsLqhtcwWRYBmww33507486 = -539536968;    double JfxsLqhtcwWRYBmww30101147 = -799452212;    double JfxsLqhtcwWRYBmww43338048 = -523570704;    double JfxsLqhtcwWRYBmww42920846 = 25577559;    double JfxsLqhtcwWRYBmww59637218 = -600098827;    double JfxsLqhtcwWRYBmww24722726 = -271971143;    double JfxsLqhtcwWRYBmww41480143 = -784619618;    double JfxsLqhtcwWRYBmww98589557 = -235423238;    double JfxsLqhtcwWRYBmww69626692 = -328084335;    double JfxsLqhtcwWRYBmww57814646 = -630099949;    double JfxsLqhtcwWRYBmww17677142 = -798528085;    double JfxsLqhtcwWRYBmww36184664 = -920819039;    double JfxsLqhtcwWRYBmww49338858 = -524298619;     JfxsLqhtcwWRYBmww32764594 = JfxsLqhtcwWRYBmww26944828;     JfxsLqhtcwWRYBmww26944828 = JfxsLqhtcwWRYBmww20529150;     JfxsLqhtcwWRYBmww20529150 = JfxsLqhtcwWRYBmww3613630;     JfxsLqhtcwWRYBmww3613630 = JfxsLqhtcwWRYBmww88961729;     JfxsLqhtcwWRYBmww88961729 = JfxsLqhtcwWRYBmww15323501;     JfxsLqhtcwWRYBmww15323501 = JfxsLqhtcwWRYBmww70178269;     JfxsLqhtcwWRYBmww70178269 = JfxsLqhtcwWRYBmww88251466;     JfxsLqhtcwWRYBmww88251466 = JfxsLqhtcwWRYBmww93794353;     JfxsLqhtcwWRYBmww93794353 = JfxsLqhtcwWRYBmww68938650;     JfxsLqhtcwWRYBmww68938650 = JfxsLqhtcwWRYBmww76122320;     JfxsLqhtcwWRYBmww76122320 = JfxsLqhtcwWRYBmww5055056;     JfxsLqhtcwWRYBmww5055056 = JfxsLqhtcwWRYBmww40758662;     JfxsLqhtcwWRYBmww40758662 = JfxsLqhtcwWRYBmww60306355;     JfxsLqhtcwWRYBmww60306355 = JfxsLqhtcwWRYBmww88084178;     JfxsLqhtcwWRYBmww88084178 = JfxsLqhtcwWRYBmww18878024;     JfxsLqhtcwWRYBmww18878024 = JfxsLqhtcwWRYBmww92932997;     JfxsLqhtcwWRYBmww92932997 = JfxsLqhtcwWRYBmww20297272;     JfxsLqhtcwWRYBmww20297272 = JfxsLqhtcwWRYBmww16174565;     JfxsLqhtcwWRYBmww16174565 = JfxsLqhtcwWRYBmww84765602;     JfxsLqhtcwWRYBmww84765602 = JfxsLqhtcwWRYBmww53750149;     JfxsLqhtcwWRYBmww53750149 = JfxsLqhtcwWRYBmww98428312;     JfxsLqhtcwWRYBmww98428312 = JfxsLqhtcwWRYBmww88414518;     JfxsLqhtcwWRYBmww88414518 = JfxsLqhtcwWRYBmww98064770;     JfxsLqhtcwWRYBmww98064770 = JfxsLqhtcwWRYBmww6555981;     JfxsLqhtcwWRYBmww6555981 = JfxsLqhtcwWRYBmww61673666;     JfxsLqhtcwWRYBmww61673666 = JfxsLqhtcwWRYBmww73351937;     JfxsLqhtcwWRYBmww73351937 = JfxsLqhtcwWRYBmww95947709;     JfxsLqhtcwWRYBmww95947709 = JfxsLqhtcwWRYBmww88502453;     JfxsLqhtcwWRYBmww88502453 = JfxsLqhtcwWRYBmww40426315;     JfxsLqhtcwWRYBmww40426315 = JfxsLqhtcwWRYBmww33012285;     JfxsLqhtcwWRYBmww33012285 = JfxsLqhtcwWRYBmww45036619;     JfxsLqhtcwWRYBmww45036619 = JfxsLqhtcwWRYBmww17652595;     JfxsLqhtcwWRYBmww17652595 = JfxsLqhtcwWRYBmww6924998;     JfxsLqhtcwWRYBmww6924998 = JfxsLqhtcwWRYBmww77674329;     JfxsLqhtcwWRYBmww77674329 = JfxsLqhtcwWRYBmww81451155;     JfxsLqhtcwWRYBmww81451155 = JfxsLqhtcwWRYBmww82101597;     JfxsLqhtcwWRYBmww82101597 = JfxsLqhtcwWRYBmww21780375;     JfxsLqhtcwWRYBmww21780375 = JfxsLqhtcwWRYBmww68768205;     JfxsLqhtcwWRYBmww68768205 = JfxsLqhtcwWRYBmww40561121;     JfxsLqhtcwWRYBmww40561121 = JfxsLqhtcwWRYBmww43857252;     JfxsLqhtcwWRYBmww43857252 = JfxsLqhtcwWRYBmww57563659;     JfxsLqhtcwWRYBmww57563659 = JfxsLqhtcwWRYBmww71045180;     JfxsLqhtcwWRYBmww71045180 = JfxsLqhtcwWRYBmww72111030;     JfxsLqhtcwWRYBmww72111030 = JfxsLqhtcwWRYBmww80424559;     JfxsLqhtcwWRYBmww80424559 = JfxsLqhtcwWRYBmww30648042;     JfxsLqhtcwWRYBmww30648042 = JfxsLqhtcwWRYBmww79946397;     JfxsLqhtcwWRYBmww79946397 = JfxsLqhtcwWRYBmww13851771;     JfxsLqhtcwWRYBmww13851771 = JfxsLqhtcwWRYBmww68913831;     JfxsLqhtcwWRYBmww68913831 = JfxsLqhtcwWRYBmww73204827;     JfxsLqhtcwWRYBmww73204827 = JfxsLqhtcwWRYBmww19884827;     JfxsLqhtcwWRYBmww19884827 = JfxsLqhtcwWRYBmww95012032;     JfxsLqhtcwWRYBmww95012032 = JfxsLqhtcwWRYBmww13220039;     JfxsLqhtcwWRYBmww13220039 = JfxsLqhtcwWRYBmww84145108;     JfxsLqhtcwWRYBmww84145108 = JfxsLqhtcwWRYBmww76823850;     JfxsLqhtcwWRYBmww76823850 = JfxsLqhtcwWRYBmww34336282;     JfxsLqhtcwWRYBmww34336282 = JfxsLqhtcwWRYBmww38530309;     JfxsLqhtcwWRYBmww38530309 = JfxsLqhtcwWRYBmww22464379;     JfxsLqhtcwWRYBmww22464379 = JfxsLqhtcwWRYBmww97057649;     JfxsLqhtcwWRYBmww97057649 = JfxsLqhtcwWRYBmww27288063;     JfxsLqhtcwWRYBmww27288063 = JfxsLqhtcwWRYBmww41971563;     JfxsLqhtcwWRYBmww41971563 = JfxsLqhtcwWRYBmww74230559;     JfxsLqhtcwWRYBmww74230559 = JfxsLqhtcwWRYBmww99749012;     JfxsLqhtcwWRYBmww99749012 = JfxsLqhtcwWRYBmww53368038;     JfxsLqhtcwWRYBmww53368038 = JfxsLqhtcwWRYBmww35926366;     JfxsLqhtcwWRYBmww35926366 = JfxsLqhtcwWRYBmww31085702;     JfxsLqhtcwWRYBmww31085702 = JfxsLqhtcwWRYBmww87402461;     JfxsLqhtcwWRYBmww87402461 = JfxsLqhtcwWRYBmww33833665;     JfxsLqhtcwWRYBmww33833665 = JfxsLqhtcwWRYBmww82632025;     JfxsLqhtcwWRYBmww82632025 = JfxsLqhtcwWRYBmww6633024;     JfxsLqhtcwWRYBmww6633024 = JfxsLqhtcwWRYBmww36776427;     JfxsLqhtcwWRYBmww36776427 = JfxsLqhtcwWRYBmww71152623;     JfxsLqhtcwWRYBmww71152623 = JfxsLqhtcwWRYBmww51529066;     JfxsLqhtcwWRYBmww51529066 = JfxsLqhtcwWRYBmww75613444;     JfxsLqhtcwWRYBmww75613444 = JfxsLqhtcwWRYBmww40908351;     JfxsLqhtcwWRYBmww40908351 = JfxsLqhtcwWRYBmww96186490;     JfxsLqhtcwWRYBmww96186490 = JfxsLqhtcwWRYBmww27383133;     JfxsLqhtcwWRYBmww27383133 = JfxsLqhtcwWRYBmww16303489;     JfxsLqhtcwWRYBmww16303489 = JfxsLqhtcwWRYBmww17640211;     JfxsLqhtcwWRYBmww17640211 = JfxsLqhtcwWRYBmww75907938;     JfxsLqhtcwWRYBmww75907938 = JfxsLqhtcwWRYBmww81727269;     JfxsLqhtcwWRYBmww81727269 = JfxsLqhtcwWRYBmww59500166;     JfxsLqhtcwWRYBmww59500166 = JfxsLqhtcwWRYBmww27033878;     JfxsLqhtcwWRYBmww27033878 = JfxsLqhtcwWRYBmww15297627;     JfxsLqhtcwWRYBmww15297627 = JfxsLqhtcwWRYBmww20541489;     JfxsLqhtcwWRYBmww20541489 = JfxsLqhtcwWRYBmww38000252;     JfxsLqhtcwWRYBmww38000252 = JfxsLqhtcwWRYBmww31816580;     JfxsLqhtcwWRYBmww31816580 = JfxsLqhtcwWRYBmww33507486;     JfxsLqhtcwWRYBmww33507486 = JfxsLqhtcwWRYBmww30101147;     JfxsLqhtcwWRYBmww30101147 = JfxsLqhtcwWRYBmww43338048;     JfxsLqhtcwWRYBmww43338048 = JfxsLqhtcwWRYBmww42920846;     JfxsLqhtcwWRYBmww42920846 = JfxsLqhtcwWRYBmww59637218;     JfxsLqhtcwWRYBmww59637218 = JfxsLqhtcwWRYBmww24722726;     JfxsLqhtcwWRYBmww24722726 = JfxsLqhtcwWRYBmww41480143;     JfxsLqhtcwWRYBmww41480143 = JfxsLqhtcwWRYBmww98589557;     JfxsLqhtcwWRYBmww98589557 = JfxsLqhtcwWRYBmww69626692;     JfxsLqhtcwWRYBmww69626692 = JfxsLqhtcwWRYBmww57814646;     JfxsLqhtcwWRYBmww57814646 = JfxsLqhtcwWRYBmww17677142;     JfxsLqhtcwWRYBmww17677142 = JfxsLqhtcwWRYBmww36184664;     JfxsLqhtcwWRYBmww36184664 = JfxsLqhtcwWRYBmww49338858;     JfxsLqhtcwWRYBmww49338858 = JfxsLqhtcwWRYBmww32764594;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RwbJxpmdmaWEDQbT45623068() {     double ldHmboJmQiSoMIbOv38881701 = -636200508;    double ldHmboJmQiSoMIbOv70318107 = -202359729;    double ldHmboJmQiSoMIbOv36571333 = -807415717;    double ldHmboJmQiSoMIbOv30688410 = -505659752;    double ldHmboJmQiSoMIbOv57857217 = -19937470;    double ldHmboJmQiSoMIbOv6740703 = -728441994;    double ldHmboJmQiSoMIbOv30667235 = -660652346;    double ldHmboJmQiSoMIbOv38011357 = -873201373;    double ldHmboJmQiSoMIbOv58436848 = -292428330;    double ldHmboJmQiSoMIbOv10168310 = -441964511;    double ldHmboJmQiSoMIbOv36232611 = -800952942;    double ldHmboJmQiSoMIbOv36878099 = -99911179;    double ldHmboJmQiSoMIbOv26967781 = -286825241;    double ldHmboJmQiSoMIbOv43450031 = -199591749;    double ldHmboJmQiSoMIbOv83826605 = -89293438;    double ldHmboJmQiSoMIbOv60383418 = -65814176;    double ldHmboJmQiSoMIbOv86769000 = 2618383;    double ldHmboJmQiSoMIbOv17350425 = -836557393;    double ldHmboJmQiSoMIbOv78004406 = 55322950;    double ldHmboJmQiSoMIbOv65315767 = -279715657;    double ldHmboJmQiSoMIbOv42303342 = -863527119;    double ldHmboJmQiSoMIbOv84949335 = -315084977;    double ldHmboJmQiSoMIbOv24005311 = -705459786;    double ldHmboJmQiSoMIbOv82057144 = -790071233;    double ldHmboJmQiSoMIbOv50255630 = -672362077;    double ldHmboJmQiSoMIbOv86395341 = -30124028;    double ldHmboJmQiSoMIbOv71811715 = -389933331;    double ldHmboJmQiSoMIbOv9054835 = -955390791;    double ldHmboJmQiSoMIbOv54634709 = -375119954;    double ldHmboJmQiSoMIbOv9041952 = -33372620;    double ldHmboJmQiSoMIbOv25254834 = -491230655;    double ldHmboJmQiSoMIbOv42647241 = -602680920;    double ldHmboJmQiSoMIbOv27363543 = -11306298;    double ldHmboJmQiSoMIbOv1755562 = -226879;    double ldHmboJmQiSoMIbOv39148981 = -179447167;    double ldHmboJmQiSoMIbOv50177068 = -822266198;    double ldHmboJmQiSoMIbOv63363532 = -642551247;    double ldHmboJmQiSoMIbOv55870587 = -59222230;    double ldHmboJmQiSoMIbOv42378459 = -126362008;    double ldHmboJmQiSoMIbOv28063059 = -683537092;    double ldHmboJmQiSoMIbOv49526476 = -25583760;    double ldHmboJmQiSoMIbOv89714115 = -233832437;    double ldHmboJmQiSoMIbOv49112158 = -87826581;    double ldHmboJmQiSoMIbOv59518770 = -475241294;    double ldHmboJmQiSoMIbOv92377024 = -261664277;    double ldHmboJmQiSoMIbOv42587237 = -757581835;    double ldHmboJmQiSoMIbOv97724971 = -515831987;    double ldHmboJmQiSoMIbOv21808712 = -970592371;    double ldHmboJmQiSoMIbOv78589560 = -30624149;    double ldHmboJmQiSoMIbOv86116479 = -909711043;    double ldHmboJmQiSoMIbOv55585505 = -595563555;    double ldHmboJmQiSoMIbOv5821753 = 6269536;    double ldHmboJmQiSoMIbOv61468496 = -958433077;    double ldHmboJmQiSoMIbOv80113989 = -899626105;    double ldHmboJmQiSoMIbOv3319726 = -49764685;    double ldHmboJmQiSoMIbOv53932365 = -221115531;    double ldHmboJmQiSoMIbOv46312797 = -496899944;    double ldHmboJmQiSoMIbOv54514189 = 82655516;    double ldHmboJmQiSoMIbOv80432780 = -833297675;    double ldHmboJmQiSoMIbOv71461876 = -989813442;    double ldHmboJmQiSoMIbOv34928988 = -238508664;    double ldHmboJmQiSoMIbOv21612401 = -705261555;    double ldHmboJmQiSoMIbOv83376648 = -398081419;    double ldHmboJmQiSoMIbOv49394897 = -159055710;    double ldHmboJmQiSoMIbOv84913476 = -950733857;    double ldHmboJmQiSoMIbOv93585370 = -98272023;    double ldHmboJmQiSoMIbOv9514556 = 11395119;    double ldHmboJmQiSoMIbOv25212219 = -186598362;    double ldHmboJmQiSoMIbOv4301050 = 79855417;    double ldHmboJmQiSoMIbOv33649538 = -267027241;    double ldHmboJmQiSoMIbOv97019885 = -423262929;    double ldHmboJmQiSoMIbOv30898414 = -938159387;    double ldHmboJmQiSoMIbOv74971965 = -610195385;    double ldHmboJmQiSoMIbOv49941348 = -261139959;    double ldHmboJmQiSoMIbOv15789292 = -154131898;    double ldHmboJmQiSoMIbOv52589226 = -529694682;    double ldHmboJmQiSoMIbOv35837178 = -127258397;    double ldHmboJmQiSoMIbOv64486540 = -130218492;    double ldHmboJmQiSoMIbOv89680119 = -428406957;    double ldHmboJmQiSoMIbOv7668394 = -914780242;    double ldHmboJmQiSoMIbOv88670370 = -514292042;    double ldHmboJmQiSoMIbOv50003003 = -419340960;    double ldHmboJmQiSoMIbOv30465275 = -824766643;    double ldHmboJmQiSoMIbOv68518229 = -465408911;    double ldHmboJmQiSoMIbOv53456447 = -437809065;    double ldHmboJmQiSoMIbOv19433082 = -397500191;    double ldHmboJmQiSoMIbOv81178744 = -644247844;    double ldHmboJmQiSoMIbOv47249554 = -111680193;    double ldHmboJmQiSoMIbOv98435835 = -950462195;    double ldHmboJmQiSoMIbOv85216616 = -958331637;    double ldHmboJmQiSoMIbOv3864272 = -225366255;    double ldHmboJmQiSoMIbOv8849343 = -625206764;    double ldHmboJmQiSoMIbOv75437807 = -225924555;    double ldHmboJmQiSoMIbOv70916583 = -136548567;    double ldHmboJmQiSoMIbOv93134071 = -345028429;    double ldHmboJmQiSoMIbOv27914076 = -320322206;    double ldHmboJmQiSoMIbOv6337468 = -835751019;    double ldHmboJmQiSoMIbOv99717261 = -928770871;    double ldHmboJmQiSoMIbOv74605294 = -524507438;    double ldHmboJmQiSoMIbOv98791654 = -636200508;     ldHmboJmQiSoMIbOv38881701 = ldHmboJmQiSoMIbOv70318107;     ldHmboJmQiSoMIbOv70318107 = ldHmboJmQiSoMIbOv36571333;     ldHmboJmQiSoMIbOv36571333 = ldHmboJmQiSoMIbOv30688410;     ldHmboJmQiSoMIbOv30688410 = ldHmboJmQiSoMIbOv57857217;     ldHmboJmQiSoMIbOv57857217 = ldHmboJmQiSoMIbOv6740703;     ldHmboJmQiSoMIbOv6740703 = ldHmboJmQiSoMIbOv30667235;     ldHmboJmQiSoMIbOv30667235 = ldHmboJmQiSoMIbOv38011357;     ldHmboJmQiSoMIbOv38011357 = ldHmboJmQiSoMIbOv58436848;     ldHmboJmQiSoMIbOv58436848 = ldHmboJmQiSoMIbOv10168310;     ldHmboJmQiSoMIbOv10168310 = ldHmboJmQiSoMIbOv36232611;     ldHmboJmQiSoMIbOv36232611 = ldHmboJmQiSoMIbOv36878099;     ldHmboJmQiSoMIbOv36878099 = ldHmboJmQiSoMIbOv26967781;     ldHmboJmQiSoMIbOv26967781 = ldHmboJmQiSoMIbOv43450031;     ldHmboJmQiSoMIbOv43450031 = ldHmboJmQiSoMIbOv83826605;     ldHmboJmQiSoMIbOv83826605 = ldHmboJmQiSoMIbOv60383418;     ldHmboJmQiSoMIbOv60383418 = ldHmboJmQiSoMIbOv86769000;     ldHmboJmQiSoMIbOv86769000 = ldHmboJmQiSoMIbOv17350425;     ldHmboJmQiSoMIbOv17350425 = ldHmboJmQiSoMIbOv78004406;     ldHmboJmQiSoMIbOv78004406 = ldHmboJmQiSoMIbOv65315767;     ldHmboJmQiSoMIbOv65315767 = ldHmboJmQiSoMIbOv42303342;     ldHmboJmQiSoMIbOv42303342 = ldHmboJmQiSoMIbOv84949335;     ldHmboJmQiSoMIbOv84949335 = ldHmboJmQiSoMIbOv24005311;     ldHmboJmQiSoMIbOv24005311 = ldHmboJmQiSoMIbOv82057144;     ldHmboJmQiSoMIbOv82057144 = ldHmboJmQiSoMIbOv50255630;     ldHmboJmQiSoMIbOv50255630 = ldHmboJmQiSoMIbOv86395341;     ldHmboJmQiSoMIbOv86395341 = ldHmboJmQiSoMIbOv71811715;     ldHmboJmQiSoMIbOv71811715 = ldHmboJmQiSoMIbOv9054835;     ldHmboJmQiSoMIbOv9054835 = ldHmboJmQiSoMIbOv54634709;     ldHmboJmQiSoMIbOv54634709 = ldHmboJmQiSoMIbOv9041952;     ldHmboJmQiSoMIbOv9041952 = ldHmboJmQiSoMIbOv25254834;     ldHmboJmQiSoMIbOv25254834 = ldHmboJmQiSoMIbOv42647241;     ldHmboJmQiSoMIbOv42647241 = ldHmboJmQiSoMIbOv27363543;     ldHmboJmQiSoMIbOv27363543 = ldHmboJmQiSoMIbOv1755562;     ldHmboJmQiSoMIbOv1755562 = ldHmboJmQiSoMIbOv39148981;     ldHmboJmQiSoMIbOv39148981 = ldHmboJmQiSoMIbOv50177068;     ldHmboJmQiSoMIbOv50177068 = ldHmboJmQiSoMIbOv63363532;     ldHmboJmQiSoMIbOv63363532 = ldHmboJmQiSoMIbOv55870587;     ldHmboJmQiSoMIbOv55870587 = ldHmboJmQiSoMIbOv42378459;     ldHmboJmQiSoMIbOv42378459 = ldHmboJmQiSoMIbOv28063059;     ldHmboJmQiSoMIbOv28063059 = ldHmboJmQiSoMIbOv49526476;     ldHmboJmQiSoMIbOv49526476 = ldHmboJmQiSoMIbOv89714115;     ldHmboJmQiSoMIbOv89714115 = ldHmboJmQiSoMIbOv49112158;     ldHmboJmQiSoMIbOv49112158 = ldHmboJmQiSoMIbOv59518770;     ldHmboJmQiSoMIbOv59518770 = ldHmboJmQiSoMIbOv92377024;     ldHmboJmQiSoMIbOv92377024 = ldHmboJmQiSoMIbOv42587237;     ldHmboJmQiSoMIbOv42587237 = ldHmboJmQiSoMIbOv97724971;     ldHmboJmQiSoMIbOv97724971 = ldHmboJmQiSoMIbOv21808712;     ldHmboJmQiSoMIbOv21808712 = ldHmboJmQiSoMIbOv78589560;     ldHmboJmQiSoMIbOv78589560 = ldHmboJmQiSoMIbOv86116479;     ldHmboJmQiSoMIbOv86116479 = ldHmboJmQiSoMIbOv55585505;     ldHmboJmQiSoMIbOv55585505 = ldHmboJmQiSoMIbOv5821753;     ldHmboJmQiSoMIbOv5821753 = ldHmboJmQiSoMIbOv61468496;     ldHmboJmQiSoMIbOv61468496 = ldHmboJmQiSoMIbOv80113989;     ldHmboJmQiSoMIbOv80113989 = ldHmboJmQiSoMIbOv3319726;     ldHmboJmQiSoMIbOv3319726 = ldHmboJmQiSoMIbOv53932365;     ldHmboJmQiSoMIbOv53932365 = ldHmboJmQiSoMIbOv46312797;     ldHmboJmQiSoMIbOv46312797 = ldHmboJmQiSoMIbOv54514189;     ldHmboJmQiSoMIbOv54514189 = ldHmboJmQiSoMIbOv80432780;     ldHmboJmQiSoMIbOv80432780 = ldHmboJmQiSoMIbOv71461876;     ldHmboJmQiSoMIbOv71461876 = ldHmboJmQiSoMIbOv34928988;     ldHmboJmQiSoMIbOv34928988 = ldHmboJmQiSoMIbOv21612401;     ldHmboJmQiSoMIbOv21612401 = ldHmboJmQiSoMIbOv83376648;     ldHmboJmQiSoMIbOv83376648 = ldHmboJmQiSoMIbOv49394897;     ldHmboJmQiSoMIbOv49394897 = ldHmboJmQiSoMIbOv84913476;     ldHmboJmQiSoMIbOv84913476 = ldHmboJmQiSoMIbOv93585370;     ldHmboJmQiSoMIbOv93585370 = ldHmboJmQiSoMIbOv9514556;     ldHmboJmQiSoMIbOv9514556 = ldHmboJmQiSoMIbOv25212219;     ldHmboJmQiSoMIbOv25212219 = ldHmboJmQiSoMIbOv4301050;     ldHmboJmQiSoMIbOv4301050 = ldHmboJmQiSoMIbOv33649538;     ldHmboJmQiSoMIbOv33649538 = ldHmboJmQiSoMIbOv97019885;     ldHmboJmQiSoMIbOv97019885 = ldHmboJmQiSoMIbOv30898414;     ldHmboJmQiSoMIbOv30898414 = ldHmboJmQiSoMIbOv74971965;     ldHmboJmQiSoMIbOv74971965 = ldHmboJmQiSoMIbOv49941348;     ldHmboJmQiSoMIbOv49941348 = ldHmboJmQiSoMIbOv15789292;     ldHmboJmQiSoMIbOv15789292 = ldHmboJmQiSoMIbOv52589226;     ldHmboJmQiSoMIbOv52589226 = ldHmboJmQiSoMIbOv35837178;     ldHmboJmQiSoMIbOv35837178 = ldHmboJmQiSoMIbOv64486540;     ldHmboJmQiSoMIbOv64486540 = ldHmboJmQiSoMIbOv89680119;     ldHmboJmQiSoMIbOv89680119 = ldHmboJmQiSoMIbOv7668394;     ldHmboJmQiSoMIbOv7668394 = ldHmboJmQiSoMIbOv88670370;     ldHmboJmQiSoMIbOv88670370 = ldHmboJmQiSoMIbOv50003003;     ldHmboJmQiSoMIbOv50003003 = ldHmboJmQiSoMIbOv30465275;     ldHmboJmQiSoMIbOv30465275 = ldHmboJmQiSoMIbOv68518229;     ldHmboJmQiSoMIbOv68518229 = ldHmboJmQiSoMIbOv53456447;     ldHmboJmQiSoMIbOv53456447 = ldHmboJmQiSoMIbOv19433082;     ldHmboJmQiSoMIbOv19433082 = ldHmboJmQiSoMIbOv81178744;     ldHmboJmQiSoMIbOv81178744 = ldHmboJmQiSoMIbOv47249554;     ldHmboJmQiSoMIbOv47249554 = ldHmboJmQiSoMIbOv98435835;     ldHmboJmQiSoMIbOv98435835 = ldHmboJmQiSoMIbOv85216616;     ldHmboJmQiSoMIbOv85216616 = ldHmboJmQiSoMIbOv3864272;     ldHmboJmQiSoMIbOv3864272 = ldHmboJmQiSoMIbOv8849343;     ldHmboJmQiSoMIbOv8849343 = ldHmboJmQiSoMIbOv75437807;     ldHmboJmQiSoMIbOv75437807 = ldHmboJmQiSoMIbOv70916583;     ldHmboJmQiSoMIbOv70916583 = ldHmboJmQiSoMIbOv93134071;     ldHmboJmQiSoMIbOv93134071 = ldHmboJmQiSoMIbOv27914076;     ldHmboJmQiSoMIbOv27914076 = ldHmboJmQiSoMIbOv6337468;     ldHmboJmQiSoMIbOv6337468 = ldHmboJmQiSoMIbOv99717261;     ldHmboJmQiSoMIbOv99717261 = ldHmboJmQiSoMIbOv74605294;     ldHmboJmQiSoMIbOv74605294 = ldHmboJmQiSoMIbOv98791654;     ldHmboJmQiSoMIbOv98791654 = ldHmboJmQiSoMIbOv38881701;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void bsfYyCDvVFNMwLxS12914667() {     double qkuelRqDoDSparvWm74340669 = -2477407;    double qkuelRqDoDSparvWm83590185 = -82911335;    double qkuelRqDoDSparvWm73957810 = -473082792;    double qkuelRqDoDSparvWm2911765 = -326810823;    double qkuelRqDoDSparvWm14142487 = -125536315;    double qkuelRqDoDSparvWm42963575 = -990789890;    double qkuelRqDoDSparvWm41324641 = -505378080;    double qkuelRqDoDSparvWm142919 = -517435513;    double qkuelRqDoDSparvWm59954454 = -780759737;    double qkuelRqDoDSparvWm25071022 = -106651509;    double qkuelRqDoDSparvWm74414346 = -602960672;    double qkuelRqDoDSparvWm37446247 = 35968050;    double qkuelRqDoDSparvWm65202426 = -838837746;    double qkuelRqDoDSparvWm96590894 = -175955158;    double qkuelRqDoDSparvWm42021122 = -592718453;    double qkuelRqDoDSparvWm43924719 = -801625783;    double qkuelRqDoDSparvWm42604937 = -524356170;    double qkuelRqDoDSparvWm17110617 = -909985462;    double qkuelRqDoDSparvWm78554736 = -261240472;    double qkuelRqDoDSparvWm59152986 = -706276723;    double qkuelRqDoDSparvWm55390205 = -520801808;    double qkuelRqDoDSparvWm88370860 = -401721271;    double qkuelRqDoDSparvWm94530690 = -343091970;    double qkuelRqDoDSparvWm75850089 = -200895837;    double qkuelRqDoDSparvWm58273581 = -696347275;    double qkuelRqDoDSparvWm97093513 = -818700835;    double qkuelRqDoDSparvWm32232290 = -196217472;    double qkuelRqDoDSparvWm49783259 = -25162966;    double qkuelRqDoDSparvWm1302924 = -949214756;    double qkuelRqDoDSparvWm14575363 = -67879791;    double qkuelRqDoDSparvWm43838065 = 56788858;    double qkuelRqDoDSparvWm80470259 = -375652334;    double qkuelRqDoDSparvWm39581554 = -158902275;    double qkuelRqDoDSparvWm31503988 = -64920693;    double qkuelRqDoDSparvWm4957435 = -130547417;    double qkuelRqDoDSparvWm1737912 = 63168496;    double qkuelRqDoDSparvWm12775349 = -507247288;    double qkuelRqDoDSparvWm7085991 = -41877811;    double qkuelRqDoDSparvWm72769020 = -877238285;    double qkuelRqDoDSparvWm92334590 = -148799638;    double qkuelRqDoDSparvWm82978083 = -894782674;    double qkuelRqDoDSparvWm79079242 = -928377728;    double qkuelRqDoDSparvWm38495536 = -741007367;    double qkuelRqDoDSparvWm99650108 = 20426296;    double qkuelRqDoDSparvWm92393483 = -88175438;    double qkuelRqDoDSparvWm88548565 = -796505467;    double qkuelRqDoDSparvWm17070135 = -999818347;    double qkuelRqDoDSparvWm39535024 = -37118965;    double qkuelRqDoDSparvWm1482227 = -794361029;    double qkuelRqDoDSparvWm10646069 = -300737859;    double qkuelRqDoDSparvWm36356229 = -469840113;    double qkuelRqDoDSparvWm89307224 = -207382811;    double qkuelRqDoDSparvWm30089645 = -714478631;    double qkuelRqDoDSparvWm96105835 = 8865395;    double qkuelRqDoDSparvWm57524462 = -396254831;    double qkuelRqDoDSparvWm85969808 = -600756136;    double qkuelRqDoDSparvWm89059495 = -739819366;    double qkuelRqDoDSparvWm98107720 = -172186955;    double qkuelRqDoDSparvWm44638184 = -630463548;    double qkuelRqDoDSparvWm17048974 = -306835481;    double qkuelRqDoDSparvWm10731286 = -694572418;    double qkuelRqDoDSparvWm91541381 = -380215115;    double qkuelRqDoDSparvWm98839994 = -568220757;    double qkuelRqDoDSparvWm45379091 = -612879947;    double qkuelRqDoDSparvWm81232956 = -63440368;    double qkuelRqDoDSparvWm93944086 = -127308338;    double qkuelRqDoDSparvWm97864693 = -805129675;    double qkuelRqDoDSparvWm33698438 = -673917054;    double qkuelRqDoDSparvWm91633459 = 54592259;    double qkuelRqDoDSparvWm40283210 = -555886950;    double qkuelRqDoDSparvWm31149370 = -194378496;    double qkuelRqDoDSparvWm35518946 = -382478359;    double qkuelRqDoDSparvWm44341596 = 67252822;    double qkuelRqDoDSparvWm86220145 = -12440834;    double qkuelRqDoDSparvWm76174902 = -811494049;    double qkuelRqDoDSparvWm76310962 = -592424080;    double qkuelRqDoDSparvWm49875324 = -660713905;    double qkuelRqDoDSparvWm94880581 = -263518266;    double qkuelRqDoDSparvWm83456606 = -12720400;    double qkuelRqDoDSparvWm69725016 = -899841808;    double qkuelRqDoDSparvWm80023378 = -818882488;    double qkuelRqDoDSparvWm92697265 = -59098508;    double qkuelRqDoDSparvWm48301033 = -230801937;    double qkuelRqDoDSparvWm90656854 = -548476898;    double qkuelRqDoDSparvWm78219134 = -598039678;    double qkuelRqDoDSparvWm54530841 = -735828331;    double qkuelRqDoDSparvWm50380615 = -661173704;    double qkuelRqDoDSparvWm43475718 = -67767671;    double qkuelRqDoDSparvWm73979526 = -668665863;    double qkuelRqDoDSparvWm18987627 = -529791281;    double qkuelRqDoDSparvWm12678417 = -197012138;    double qkuelRqDoDSparvWm14667628 = -235060333;    double qkuelRqDoDSparvWm62447807 = -411414263;    double qkuelRqDoDSparvWm55720047 = -470402805;    double qkuelRqDoDSparvWm81603305 = -454227221;    double qkuelRqDoDSparvWm91436701 = -414567560;    double qkuelRqDoDSparvWm80239248 = -260156972;    double qkuelRqDoDSparvWm93116445 = -28127421;    double qkuelRqDoDSparvWm18417152 = -916133337;    double qkuelRqDoDSparvWm98449396 = -2477407;     qkuelRqDoDSparvWm74340669 = qkuelRqDoDSparvWm83590185;     qkuelRqDoDSparvWm83590185 = qkuelRqDoDSparvWm73957810;     qkuelRqDoDSparvWm73957810 = qkuelRqDoDSparvWm2911765;     qkuelRqDoDSparvWm2911765 = qkuelRqDoDSparvWm14142487;     qkuelRqDoDSparvWm14142487 = qkuelRqDoDSparvWm42963575;     qkuelRqDoDSparvWm42963575 = qkuelRqDoDSparvWm41324641;     qkuelRqDoDSparvWm41324641 = qkuelRqDoDSparvWm142919;     qkuelRqDoDSparvWm142919 = qkuelRqDoDSparvWm59954454;     qkuelRqDoDSparvWm59954454 = qkuelRqDoDSparvWm25071022;     qkuelRqDoDSparvWm25071022 = qkuelRqDoDSparvWm74414346;     qkuelRqDoDSparvWm74414346 = qkuelRqDoDSparvWm37446247;     qkuelRqDoDSparvWm37446247 = qkuelRqDoDSparvWm65202426;     qkuelRqDoDSparvWm65202426 = qkuelRqDoDSparvWm96590894;     qkuelRqDoDSparvWm96590894 = qkuelRqDoDSparvWm42021122;     qkuelRqDoDSparvWm42021122 = qkuelRqDoDSparvWm43924719;     qkuelRqDoDSparvWm43924719 = qkuelRqDoDSparvWm42604937;     qkuelRqDoDSparvWm42604937 = qkuelRqDoDSparvWm17110617;     qkuelRqDoDSparvWm17110617 = qkuelRqDoDSparvWm78554736;     qkuelRqDoDSparvWm78554736 = qkuelRqDoDSparvWm59152986;     qkuelRqDoDSparvWm59152986 = qkuelRqDoDSparvWm55390205;     qkuelRqDoDSparvWm55390205 = qkuelRqDoDSparvWm88370860;     qkuelRqDoDSparvWm88370860 = qkuelRqDoDSparvWm94530690;     qkuelRqDoDSparvWm94530690 = qkuelRqDoDSparvWm75850089;     qkuelRqDoDSparvWm75850089 = qkuelRqDoDSparvWm58273581;     qkuelRqDoDSparvWm58273581 = qkuelRqDoDSparvWm97093513;     qkuelRqDoDSparvWm97093513 = qkuelRqDoDSparvWm32232290;     qkuelRqDoDSparvWm32232290 = qkuelRqDoDSparvWm49783259;     qkuelRqDoDSparvWm49783259 = qkuelRqDoDSparvWm1302924;     qkuelRqDoDSparvWm1302924 = qkuelRqDoDSparvWm14575363;     qkuelRqDoDSparvWm14575363 = qkuelRqDoDSparvWm43838065;     qkuelRqDoDSparvWm43838065 = qkuelRqDoDSparvWm80470259;     qkuelRqDoDSparvWm80470259 = qkuelRqDoDSparvWm39581554;     qkuelRqDoDSparvWm39581554 = qkuelRqDoDSparvWm31503988;     qkuelRqDoDSparvWm31503988 = qkuelRqDoDSparvWm4957435;     qkuelRqDoDSparvWm4957435 = qkuelRqDoDSparvWm1737912;     qkuelRqDoDSparvWm1737912 = qkuelRqDoDSparvWm12775349;     qkuelRqDoDSparvWm12775349 = qkuelRqDoDSparvWm7085991;     qkuelRqDoDSparvWm7085991 = qkuelRqDoDSparvWm72769020;     qkuelRqDoDSparvWm72769020 = qkuelRqDoDSparvWm92334590;     qkuelRqDoDSparvWm92334590 = qkuelRqDoDSparvWm82978083;     qkuelRqDoDSparvWm82978083 = qkuelRqDoDSparvWm79079242;     qkuelRqDoDSparvWm79079242 = qkuelRqDoDSparvWm38495536;     qkuelRqDoDSparvWm38495536 = qkuelRqDoDSparvWm99650108;     qkuelRqDoDSparvWm99650108 = qkuelRqDoDSparvWm92393483;     qkuelRqDoDSparvWm92393483 = qkuelRqDoDSparvWm88548565;     qkuelRqDoDSparvWm88548565 = qkuelRqDoDSparvWm17070135;     qkuelRqDoDSparvWm17070135 = qkuelRqDoDSparvWm39535024;     qkuelRqDoDSparvWm39535024 = qkuelRqDoDSparvWm1482227;     qkuelRqDoDSparvWm1482227 = qkuelRqDoDSparvWm10646069;     qkuelRqDoDSparvWm10646069 = qkuelRqDoDSparvWm36356229;     qkuelRqDoDSparvWm36356229 = qkuelRqDoDSparvWm89307224;     qkuelRqDoDSparvWm89307224 = qkuelRqDoDSparvWm30089645;     qkuelRqDoDSparvWm30089645 = qkuelRqDoDSparvWm96105835;     qkuelRqDoDSparvWm96105835 = qkuelRqDoDSparvWm57524462;     qkuelRqDoDSparvWm57524462 = qkuelRqDoDSparvWm85969808;     qkuelRqDoDSparvWm85969808 = qkuelRqDoDSparvWm89059495;     qkuelRqDoDSparvWm89059495 = qkuelRqDoDSparvWm98107720;     qkuelRqDoDSparvWm98107720 = qkuelRqDoDSparvWm44638184;     qkuelRqDoDSparvWm44638184 = qkuelRqDoDSparvWm17048974;     qkuelRqDoDSparvWm17048974 = qkuelRqDoDSparvWm10731286;     qkuelRqDoDSparvWm10731286 = qkuelRqDoDSparvWm91541381;     qkuelRqDoDSparvWm91541381 = qkuelRqDoDSparvWm98839994;     qkuelRqDoDSparvWm98839994 = qkuelRqDoDSparvWm45379091;     qkuelRqDoDSparvWm45379091 = qkuelRqDoDSparvWm81232956;     qkuelRqDoDSparvWm81232956 = qkuelRqDoDSparvWm93944086;     qkuelRqDoDSparvWm93944086 = qkuelRqDoDSparvWm97864693;     qkuelRqDoDSparvWm97864693 = qkuelRqDoDSparvWm33698438;     qkuelRqDoDSparvWm33698438 = qkuelRqDoDSparvWm91633459;     qkuelRqDoDSparvWm91633459 = qkuelRqDoDSparvWm40283210;     qkuelRqDoDSparvWm40283210 = qkuelRqDoDSparvWm31149370;     qkuelRqDoDSparvWm31149370 = qkuelRqDoDSparvWm35518946;     qkuelRqDoDSparvWm35518946 = qkuelRqDoDSparvWm44341596;     qkuelRqDoDSparvWm44341596 = qkuelRqDoDSparvWm86220145;     qkuelRqDoDSparvWm86220145 = qkuelRqDoDSparvWm76174902;     qkuelRqDoDSparvWm76174902 = qkuelRqDoDSparvWm76310962;     qkuelRqDoDSparvWm76310962 = qkuelRqDoDSparvWm49875324;     qkuelRqDoDSparvWm49875324 = qkuelRqDoDSparvWm94880581;     qkuelRqDoDSparvWm94880581 = qkuelRqDoDSparvWm83456606;     qkuelRqDoDSparvWm83456606 = qkuelRqDoDSparvWm69725016;     qkuelRqDoDSparvWm69725016 = qkuelRqDoDSparvWm80023378;     qkuelRqDoDSparvWm80023378 = qkuelRqDoDSparvWm92697265;     qkuelRqDoDSparvWm92697265 = qkuelRqDoDSparvWm48301033;     qkuelRqDoDSparvWm48301033 = qkuelRqDoDSparvWm90656854;     qkuelRqDoDSparvWm90656854 = qkuelRqDoDSparvWm78219134;     qkuelRqDoDSparvWm78219134 = qkuelRqDoDSparvWm54530841;     qkuelRqDoDSparvWm54530841 = qkuelRqDoDSparvWm50380615;     qkuelRqDoDSparvWm50380615 = qkuelRqDoDSparvWm43475718;     qkuelRqDoDSparvWm43475718 = qkuelRqDoDSparvWm73979526;     qkuelRqDoDSparvWm73979526 = qkuelRqDoDSparvWm18987627;     qkuelRqDoDSparvWm18987627 = qkuelRqDoDSparvWm12678417;     qkuelRqDoDSparvWm12678417 = qkuelRqDoDSparvWm14667628;     qkuelRqDoDSparvWm14667628 = qkuelRqDoDSparvWm62447807;     qkuelRqDoDSparvWm62447807 = qkuelRqDoDSparvWm55720047;     qkuelRqDoDSparvWm55720047 = qkuelRqDoDSparvWm81603305;     qkuelRqDoDSparvWm81603305 = qkuelRqDoDSparvWm91436701;     qkuelRqDoDSparvWm91436701 = qkuelRqDoDSparvWm80239248;     qkuelRqDoDSparvWm80239248 = qkuelRqDoDSparvWm93116445;     qkuelRqDoDSparvWm93116445 = qkuelRqDoDSparvWm18417152;     qkuelRqDoDSparvWm18417152 = qkuelRqDoDSparvWm98449396;     qkuelRqDoDSparvWm98449396 = qkuelRqDoDSparvWm74340669;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ChPGRVFtoGfxqpNi27963735() {     double pbccWlsFZNpeidLBB80457775 = -114379296;    double pbccWlsFZNpeidLBB26963465 = -892511116;    double pbccWlsFZNpeidLBB89999993 = -623915713;    double pbccWlsFZNpeidLBB29986546 = -517559453;    double pbccWlsFZNpeidLBB83037975 = -646186186;    double pbccWlsFZNpeidLBB34380777 = -155286125;    double pbccWlsFZNpeidLBB1813608 = -644343057;    double pbccWlsFZNpeidLBB49902809 = -804309426;    double pbccWlsFZNpeidLBB24596949 = -548792403;    double pbccWlsFZNpeidLBB66300681 = -435626987;    double pbccWlsFZNpeidLBB34524636 = -480398258;    double pbccWlsFZNpeidLBB69269289 = -408438152;    double pbccWlsFZNpeidLBB51411544 = -178728835;    double pbccWlsFZNpeidLBB79734571 = -708193726;    double pbccWlsFZNpeidLBB37763549 = -183180620;    double pbccWlsFZNpeidLBB85430112 = -774625326;    double pbccWlsFZNpeidLBB36440940 = -40901282;    double pbccWlsFZNpeidLBB14163770 = -575508104;    double pbccWlsFZNpeidLBB40384578 = -667009569;    double pbccWlsFZNpeidLBB39703151 = 36820357;    double pbccWlsFZNpeidLBB43943398 = -376289263;    double pbccWlsFZNpeidLBB74891884 = -309650988;    double pbccWlsFZNpeidLBB30121482 = -89609457;    double pbccWlsFZNpeidLBB59842463 = -423200825;    double pbccWlsFZNpeidLBB1973232 = 30456692;    double pbccWlsFZNpeidLBB21815189 = -697356158;    double pbccWlsFZNpeidLBB30692068 = -96879675;    double pbccWlsFZNpeidLBB62890385 = -91091919;    double pbccWlsFZNpeidLBB67435179 = -71597461;    double pbccWlsFZNpeidLBB83190999 = -985350658;    double pbccWlsFZNpeidLBB36080615 = -956532080;    double pbccWlsFZNpeidLBB78080881 = -85099150;    double pbccWlsFZNpeidLBB49292502 = -51515645;    double pbccWlsFZNpeidLBB26334552 = -916462458;    double pbccWlsFZNpeidLBB66432087 = -769280523;    double pbccWlsFZNpeidLBB70463825 = -150857611;    double pbccWlsFZNpeidLBB94037283 = -460883157;    double pbccWlsFZNpeidLBB41176203 = -913383821;    double pbccWlsFZNpeidLBB46379274 = -871161781;    double pbccWlsFZNpeidLBB79836528 = -622238861;    double pbccWlsFZNpeidLBB88647307 = -960056569;    double pbccWlsFZNpeidLBB11229700 = -98520006;    double pbccWlsFZNpeidLBB16562515 = -821811951;    double pbccWlsFZNpeidLBB87057849 = 1083357;    double pbccWlsFZNpeidLBB4345949 = -850073592;    double pbccWlsFZNpeidLBB487761 = -781721584;    double pbccWlsFZNpeidLBB34848709 = -18593885;    double pbccWlsFZNpeidLBB47491965 = -47014497;    double pbccWlsFZNpeidLBB11157955 = -880098017;    double pbccWlsFZNpeidLBB23557722 = -387748667;    double pbccWlsFZNpeidLBB72056907 = -312133784;    double pbccWlsFZNpeidLBB116945 = -8109131;    double pbccWlsFZNpeidLBB78338102 = -506268218;    double pbccWlsFZNpeidLBB92074715 = -311604750;    double pbccWlsFZNpeidLBB84020336 = 3213387;    double pbccWlsFZNpeidLBB5565892 = -804728309;    double pbccWlsFZNpeidLBB96841982 = -702901660;    double pbccWlsFZNpeidLBB30157530 = -100714889;    double pbccWlsFZNpeidLBB28013314 = -448016146;    double pbccWlsFZNpeidLBB61222787 = -948830029;    double pbccWlsFZNpeidLBB3688710 = 41593550;    double pbccWlsFZNpeidLBB38923222 = -453251138;    double pbccWlsFZNpeidLBB82467630 = -632711966;    double pbccWlsFZNpeidLBB41405950 = -563441745;    double pbccWlsFZNpeidLBB30220067 = -479094907;    double pbccWlsFZNpeidLBB56443755 = -295299109;    double pbccWlsFZNpeidLBB19976788 = -256922507;    double pbccWlsFZNpeidLBB25076993 = -262266377;    double pbccWlsFZNpeidLBB13302485 = -938913204;    double pbccWlsFZNpeidLBB67299723 = 67676991;    double pbccWlsFZNpeidLBB91392829 = -213742169;    double pbccWlsFZNpeidLBB95264736 = -127517462;    double pbccWlsFZNpeidLBB67784495 = -704346324;    double pbccWlsFZNpeidLBB60548050 = 55229292;    double pbccWlsFZNpeidLBB51055843 = -3123075;    double pbccWlsFZNpeidLBB32713699 = -177769257;    double pbccWlsFZNpeidLBB58329369 = -487839038;    double pbccWlsFZNpeidLBB43063633 = 9307186;    double pbccWlsFZNpeidLBB55496515 = -573127233;    double pbccWlsFZNpeidLBB1485472 = -187821725;    double pbccWlsFZNpeidLBB86966479 = -578762274;    double pbccWlsFZNpeidLBB83200102 = 50134822;    double pbccWlsFZNpeidLBB51732430 = -210993903;    double pbccWlsFZNpeidLBB43877458 = -683848795;    double pbccWlsFZNpeidLBB11134093 = -573216874;    double pbccWlsFZNpeidLBB35963671 = -848422949;    double pbccWlsFZNpeidLBB99742778 = -578830932;    double pbccWlsFZNpeidLBB57217786 = -739910896;    double pbccWlsFZNpeidLBB42314216 = -819675846;    double pbccWlsFZNpeidLBB60866195 = -964552214;    double pbccWlsFZNpeidLBB73621842 = -447955952;    double pbccWlsFZNpeidLBB63879753 = -260168269;    double pbccWlsFZNpeidLBB13162889 = -365367676;    double pbccWlsFZNpeidLBB85156487 = -922331753;    double pbccWlsFZNpeidLBB76147819 = -563832411;    double pbccWlsFZNpeidLBB49724085 = -406805431;    double pbccWlsFZNpeidLBB28762070 = -465808041;    double pbccWlsFZNpeidLBB75156565 = -158370206;    double pbccWlsFZNpeidLBB56837783 = -519821736;    double pbccWlsFZNpeidLBB47902193 = -114379296;     pbccWlsFZNpeidLBB80457775 = pbccWlsFZNpeidLBB26963465;     pbccWlsFZNpeidLBB26963465 = pbccWlsFZNpeidLBB89999993;     pbccWlsFZNpeidLBB89999993 = pbccWlsFZNpeidLBB29986546;     pbccWlsFZNpeidLBB29986546 = pbccWlsFZNpeidLBB83037975;     pbccWlsFZNpeidLBB83037975 = pbccWlsFZNpeidLBB34380777;     pbccWlsFZNpeidLBB34380777 = pbccWlsFZNpeidLBB1813608;     pbccWlsFZNpeidLBB1813608 = pbccWlsFZNpeidLBB49902809;     pbccWlsFZNpeidLBB49902809 = pbccWlsFZNpeidLBB24596949;     pbccWlsFZNpeidLBB24596949 = pbccWlsFZNpeidLBB66300681;     pbccWlsFZNpeidLBB66300681 = pbccWlsFZNpeidLBB34524636;     pbccWlsFZNpeidLBB34524636 = pbccWlsFZNpeidLBB69269289;     pbccWlsFZNpeidLBB69269289 = pbccWlsFZNpeidLBB51411544;     pbccWlsFZNpeidLBB51411544 = pbccWlsFZNpeidLBB79734571;     pbccWlsFZNpeidLBB79734571 = pbccWlsFZNpeidLBB37763549;     pbccWlsFZNpeidLBB37763549 = pbccWlsFZNpeidLBB85430112;     pbccWlsFZNpeidLBB85430112 = pbccWlsFZNpeidLBB36440940;     pbccWlsFZNpeidLBB36440940 = pbccWlsFZNpeidLBB14163770;     pbccWlsFZNpeidLBB14163770 = pbccWlsFZNpeidLBB40384578;     pbccWlsFZNpeidLBB40384578 = pbccWlsFZNpeidLBB39703151;     pbccWlsFZNpeidLBB39703151 = pbccWlsFZNpeidLBB43943398;     pbccWlsFZNpeidLBB43943398 = pbccWlsFZNpeidLBB74891884;     pbccWlsFZNpeidLBB74891884 = pbccWlsFZNpeidLBB30121482;     pbccWlsFZNpeidLBB30121482 = pbccWlsFZNpeidLBB59842463;     pbccWlsFZNpeidLBB59842463 = pbccWlsFZNpeidLBB1973232;     pbccWlsFZNpeidLBB1973232 = pbccWlsFZNpeidLBB21815189;     pbccWlsFZNpeidLBB21815189 = pbccWlsFZNpeidLBB30692068;     pbccWlsFZNpeidLBB30692068 = pbccWlsFZNpeidLBB62890385;     pbccWlsFZNpeidLBB62890385 = pbccWlsFZNpeidLBB67435179;     pbccWlsFZNpeidLBB67435179 = pbccWlsFZNpeidLBB83190999;     pbccWlsFZNpeidLBB83190999 = pbccWlsFZNpeidLBB36080615;     pbccWlsFZNpeidLBB36080615 = pbccWlsFZNpeidLBB78080881;     pbccWlsFZNpeidLBB78080881 = pbccWlsFZNpeidLBB49292502;     pbccWlsFZNpeidLBB49292502 = pbccWlsFZNpeidLBB26334552;     pbccWlsFZNpeidLBB26334552 = pbccWlsFZNpeidLBB66432087;     pbccWlsFZNpeidLBB66432087 = pbccWlsFZNpeidLBB70463825;     pbccWlsFZNpeidLBB70463825 = pbccWlsFZNpeidLBB94037283;     pbccWlsFZNpeidLBB94037283 = pbccWlsFZNpeidLBB41176203;     pbccWlsFZNpeidLBB41176203 = pbccWlsFZNpeidLBB46379274;     pbccWlsFZNpeidLBB46379274 = pbccWlsFZNpeidLBB79836528;     pbccWlsFZNpeidLBB79836528 = pbccWlsFZNpeidLBB88647307;     pbccWlsFZNpeidLBB88647307 = pbccWlsFZNpeidLBB11229700;     pbccWlsFZNpeidLBB11229700 = pbccWlsFZNpeidLBB16562515;     pbccWlsFZNpeidLBB16562515 = pbccWlsFZNpeidLBB87057849;     pbccWlsFZNpeidLBB87057849 = pbccWlsFZNpeidLBB4345949;     pbccWlsFZNpeidLBB4345949 = pbccWlsFZNpeidLBB487761;     pbccWlsFZNpeidLBB487761 = pbccWlsFZNpeidLBB34848709;     pbccWlsFZNpeidLBB34848709 = pbccWlsFZNpeidLBB47491965;     pbccWlsFZNpeidLBB47491965 = pbccWlsFZNpeidLBB11157955;     pbccWlsFZNpeidLBB11157955 = pbccWlsFZNpeidLBB23557722;     pbccWlsFZNpeidLBB23557722 = pbccWlsFZNpeidLBB72056907;     pbccWlsFZNpeidLBB72056907 = pbccWlsFZNpeidLBB116945;     pbccWlsFZNpeidLBB116945 = pbccWlsFZNpeidLBB78338102;     pbccWlsFZNpeidLBB78338102 = pbccWlsFZNpeidLBB92074715;     pbccWlsFZNpeidLBB92074715 = pbccWlsFZNpeidLBB84020336;     pbccWlsFZNpeidLBB84020336 = pbccWlsFZNpeidLBB5565892;     pbccWlsFZNpeidLBB5565892 = pbccWlsFZNpeidLBB96841982;     pbccWlsFZNpeidLBB96841982 = pbccWlsFZNpeidLBB30157530;     pbccWlsFZNpeidLBB30157530 = pbccWlsFZNpeidLBB28013314;     pbccWlsFZNpeidLBB28013314 = pbccWlsFZNpeidLBB61222787;     pbccWlsFZNpeidLBB61222787 = pbccWlsFZNpeidLBB3688710;     pbccWlsFZNpeidLBB3688710 = pbccWlsFZNpeidLBB38923222;     pbccWlsFZNpeidLBB38923222 = pbccWlsFZNpeidLBB82467630;     pbccWlsFZNpeidLBB82467630 = pbccWlsFZNpeidLBB41405950;     pbccWlsFZNpeidLBB41405950 = pbccWlsFZNpeidLBB30220067;     pbccWlsFZNpeidLBB30220067 = pbccWlsFZNpeidLBB56443755;     pbccWlsFZNpeidLBB56443755 = pbccWlsFZNpeidLBB19976788;     pbccWlsFZNpeidLBB19976788 = pbccWlsFZNpeidLBB25076993;     pbccWlsFZNpeidLBB25076993 = pbccWlsFZNpeidLBB13302485;     pbccWlsFZNpeidLBB13302485 = pbccWlsFZNpeidLBB67299723;     pbccWlsFZNpeidLBB67299723 = pbccWlsFZNpeidLBB91392829;     pbccWlsFZNpeidLBB91392829 = pbccWlsFZNpeidLBB95264736;     pbccWlsFZNpeidLBB95264736 = pbccWlsFZNpeidLBB67784495;     pbccWlsFZNpeidLBB67784495 = pbccWlsFZNpeidLBB60548050;     pbccWlsFZNpeidLBB60548050 = pbccWlsFZNpeidLBB51055843;     pbccWlsFZNpeidLBB51055843 = pbccWlsFZNpeidLBB32713699;     pbccWlsFZNpeidLBB32713699 = pbccWlsFZNpeidLBB58329369;     pbccWlsFZNpeidLBB58329369 = pbccWlsFZNpeidLBB43063633;     pbccWlsFZNpeidLBB43063633 = pbccWlsFZNpeidLBB55496515;     pbccWlsFZNpeidLBB55496515 = pbccWlsFZNpeidLBB1485472;     pbccWlsFZNpeidLBB1485472 = pbccWlsFZNpeidLBB86966479;     pbccWlsFZNpeidLBB86966479 = pbccWlsFZNpeidLBB83200102;     pbccWlsFZNpeidLBB83200102 = pbccWlsFZNpeidLBB51732430;     pbccWlsFZNpeidLBB51732430 = pbccWlsFZNpeidLBB43877458;     pbccWlsFZNpeidLBB43877458 = pbccWlsFZNpeidLBB11134093;     pbccWlsFZNpeidLBB11134093 = pbccWlsFZNpeidLBB35963671;     pbccWlsFZNpeidLBB35963671 = pbccWlsFZNpeidLBB99742778;     pbccWlsFZNpeidLBB99742778 = pbccWlsFZNpeidLBB57217786;     pbccWlsFZNpeidLBB57217786 = pbccWlsFZNpeidLBB42314216;     pbccWlsFZNpeidLBB42314216 = pbccWlsFZNpeidLBB60866195;     pbccWlsFZNpeidLBB60866195 = pbccWlsFZNpeidLBB73621842;     pbccWlsFZNpeidLBB73621842 = pbccWlsFZNpeidLBB63879753;     pbccWlsFZNpeidLBB63879753 = pbccWlsFZNpeidLBB13162889;     pbccWlsFZNpeidLBB13162889 = pbccWlsFZNpeidLBB85156487;     pbccWlsFZNpeidLBB85156487 = pbccWlsFZNpeidLBB76147819;     pbccWlsFZNpeidLBB76147819 = pbccWlsFZNpeidLBB49724085;     pbccWlsFZNpeidLBB49724085 = pbccWlsFZNpeidLBB28762070;     pbccWlsFZNpeidLBB28762070 = pbccWlsFZNpeidLBB75156565;     pbccWlsFZNpeidLBB75156565 = pbccWlsFZNpeidLBB56837783;     pbccWlsFZNpeidLBB56837783 = pbccWlsFZNpeidLBB47902193;     pbccWlsFZNpeidLBB47902193 = pbccWlsFZNpeidLBB80457775;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wkgpSEMTZccIaBvD95255334() {     double fAVVsKkpyXrpVWBjU15916745 = -580656196;    double fAVVsKkpyXrpVWBjU40235544 = -773062722;    double fAVVsKkpyXrpVWBjU27386471 = -289582788;    double fAVVsKkpyXrpVWBjU2209901 = -338710524;    double fAVVsKkpyXrpVWBjU39323245 = -751785032;    double fAVVsKkpyXrpVWBjU70603649 = -417634020;    double fAVVsKkpyXrpVWBjU12471014 = -489068792;    double fAVVsKkpyXrpVWBjU12034370 = -448543567;    double fAVVsKkpyXrpVWBjU26114555 = 62876190;    double fAVVsKkpyXrpVWBjU81203393 = -100313985;    double fAVVsKkpyXrpVWBjU72706371 = -282405987;    double fAVVsKkpyXrpVWBjU69837438 = -272558923;    double fAVVsKkpyXrpVWBjU89646189 = -730741340;    double fAVVsKkpyXrpVWBjU32875435 = -684557135;    double fAVVsKkpyXrpVWBjU95958065 = -686605635;    double fAVVsKkpyXrpVWBjU68971413 = -410436933;    double fAVVsKkpyXrpVWBjU92276875 = -567875835;    double fAVVsKkpyXrpVWBjU13923961 = -648936173;    double fAVVsKkpyXrpVWBjU40934908 = -983572990;    double fAVVsKkpyXrpVWBjU33540369 = -389740708;    double fAVVsKkpyXrpVWBjU57030261 = -33563952;    double fAVVsKkpyXrpVWBjU78313409 = -396287282;    double fAVVsKkpyXrpVWBjU646862 = -827241641;    double fAVVsKkpyXrpVWBjU53635409 = -934025429;    double fAVVsKkpyXrpVWBjU9991183 = 6471494;    double fAVVsKkpyXrpVWBjU32513360 = -385932965;    double fAVVsKkpyXrpVWBjU91112641 = 96836184;    double fAVVsKkpyXrpVWBjU3618810 = -260864094;    double fAVVsKkpyXrpVWBjU14103394 = -645692263;    double fAVVsKkpyXrpVWBjU88724410 = 80142171;    double fAVVsKkpyXrpVWBjU54663846 = -408512567;    double fAVVsKkpyXrpVWBjU15903900 = -958070564;    double fAVVsKkpyXrpVWBjU61510513 = -199111623;    double fAVVsKkpyXrpVWBjU56082978 = -981156272;    double fAVVsKkpyXrpVWBjU32240541 = -720380773;    double fAVVsKkpyXrpVWBjU22024669 = -365422917;    double fAVVsKkpyXrpVWBjU43449100 = -325579198;    double fAVVsKkpyXrpVWBjU92391606 = -896039402;    double fAVVsKkpyXrpVWBjU76769836 = -522038058;    double fAVVsKkpyXrpVWBjU44108061 = -87501407;    double fAVVsKkpyXrpVWBjU22098915 = -729255483;    double fAVVsKkpyXrpVWBjU594827 = -793065297;    double fAVVsKkpyXrpVWBjU5945893 = -374992737;    double fAVVsKkpyXrpVWBjU27189188 = -603249053;    double fAVVsKkpyXrpVWBjU4362407 = -676584754;    double fAVVsKkpyXrpVWBjU46449089 = -820645216;    double fAVVsKkpyXrpVWBjU54193873 = -502580245;    double fAVVsKkpyXrpVWBjU65218277 = -213541091;    double fAVVsKkpyXrpVWBjU34050621 = -543834898;    double fAVVsKkpyXrpVWBjU48087311 = -878775483;    double fAVVsKkpyXrpVWBjU52827631 = -186410342;    double fAVVsKkpyXrpVWBjU83602416 = -221761478;    double fAVVsKkpyXrpVWBjU46959251 = -262313772;    double fAVVsKkpyXrpVWBjU8066563 = -503113250;    double fAVVsKkpyXrpVWBjU38225073 = -343276759;    double fAVVsKkpyXrpVWBjU37603335 = -84368915;    double fAVVsKkpyXrpVWBjU39588682 = -945821082;    double fAVVsKkpyXrpVWBjU73751062 = -355557359;    double fAVVsKkpyXrpVWBjU92218717 = -245182018;    double fAVVsKkpyXrpVWBjU6809885 = -265852067;    double fAVVsKkpyXrpVWBjU79491007 = -414470205;    double fAVVsKkpyXrpVWBjU8852204 = -128204698;    double fAVVsKkpyXrpVWBjU97930976 = -802851304;    double fAVVsKkpyXrpVWBjU37390144 = 82734019;    double fAVVsKkpyXrpVWBjU26539547 = -691801418;    double fAVVsKkpyXrpVWBjU56802471 = -324335424;    double fAVVsKkpyXrpVWBjU8326925 = 26552699;    double fAVVsKkpyXrpVWBjU33563211 = -749585069;    double fAVVsKkpyXrpVWBjU634894 = -964176362;    double fAVVsKkpyXrpVWBjU73933396 = -221182718;    double fAVVsKkpyXrpVWBjU25522314 = 15142264;    double fAVVsKkpyXrpVWBjU99885269 = -671836434;    double fAVVsKkpyXrpVWBjU37154125 = -26898116;    double fAVVsKkpyXrpVWBjU96826847 = -796071584;    double fAVVsKkpyXrpVWBjU11441455 = -660485226;    double fAVVsKkpyXrpVWBjU56435435 = -240498655;    double fAVVsKkpyXrpVWBjU72367516 = 78705454;    double fAVVsKkpyXrpVWBjU73457674 = -123992589;    double fAVVsKkpyXrpVWBjU49273002 = -157440676;    double fAVVsKkpyXrpVWBjU63542094 = -172883291;    double fAVVsKkpyXrpVWBjU78319487 = -883352720;    double fAVVsKkpyXrpVWBjU25894365 = -689622725;    double fAVVsKkpyXrpVWBjU69568188 = -717029197;    double fAVVsKkpyXrpVWBjU66016083 = -766916781;    double fAVVsKkpyXrpVWBjU35896780 = -733447487;    double fAVVsKkpyXrpVWBjU71061429 = -86751090;    double fAVVsKkpyXrpVWBjU68944649 = -595756792;    double fAVVsKkpyXrpVWBjU53443950 = -695998374;    double fAVVsKkpyXrpVWBjU17857906 = -537879513;    double fAVVsKkpyXrpVWBjU94637205 = -536011859;    double fAVVsKkpyXrpVWBjU82435987 = -419601836;    double fAVVsKkpyXrpVWBjU69698038 = -970021839;    double fAVVsKkpyXrpVWBjU172889 = -550857384;    double fAVVsKkpyXrpVWBjU69959951 = -156185991;    double fAVVsKkpyXrpVWBjU64617053 = -673031203;    double fAVVsKkpyXrpVWBjU13246712 = -501050785;    double fAVVsKkpyXrpVWBjU2663851 = -990213994;    double fAVVsKkpyXrpVWBjU68555749 = -357726756;    double fAVVsKkpyXrpVWBjU649641 = -911447635;    double fAVVsKkpyXrpVWBjU47559936 = -580656196;     fAVVsKkpyXrpVWBjU15916745 = fAVVsKkpyXrpVWBjU40235544;     fAVVsKkpyXrpVWBjU40235544 = fAVVsKkpyXrpVWBjU27386471;     fAVVsKkpyXrpVWBjU27386471 = fAVVsKkpyXrpVWBjU2209901;     fAVVsKkpyXrpVWBjU2209901 = fAVVsKkpyXrpVWBjU39323245;     fAVVsKkpyXrpVWBjU39323245 = fAVVsKkpyXrpVWBjU70603649;     fAVVsKkpyXrpVWBjU70603649 = fAVVsKkpyXrpVWBjU12471014;     fAVVsKkpyXrpVWBjU12471014 = fAVVsKkpyXrpVWBjU12034370;     fAVVsKkpyXrpVWBjU12034370 = fAVVsKkpyXrpVWBjU26114555;     fAVVsKkpyXrpVWBjU26114555 = fAVVsKkpyXrpVWBjU81203393;     fAVVsKkpyXrpVWBjU81203393 = fAVVsKkpyXrpVWBjU72706371;     fAVVsKkpyXrpVWBjU72706371 = fAVVsKkpyXrpVWBjU69837438;     fAVVsKkpyXrpVWBjU69837438 = fAVVsKkpyXrpVWBjU89646189;     fAVVsKkpyXrpVWBjU89646189 = fAVVsKkpyXrpVWBjU32875435;     fAVVsKkpyXrpVWBjU32875435 = fAVVsKkpyXrpVWBjU95958065;     fAVVsKkpyXrpVWBjU95958065 = fAVVsKkpyXrpVWBjU68971413;     fAVVsKkpyXrpVWBjU68971413 = fAVVsKkpyXrpVWBjU92276875;     fAVVsKkpyXrpVWBjU92276875 = fAVVsKkpyXrpVWBjU13923961;     fAVVsKkpyXrpVWBjU13923961 = fAVVsKkpyXrpVWBjU40934908;     fAVVsKkpyXrpVWBjU40934908 = fAVVsKkpyXrpVWBjU33540369;     fAVVsKkpyXrpVWBjU33540369 = fAVVsKkpyXrpVWBjU57030261;     fAVVsKkpyXrpVWBjU57030261 = fAVVsKkpyXrpVWBjU78313409;     fAVVsKkpyXrpVWBjU78313409 = fAVVsKkpyXrpVWBjU646862;     fAVVsKkpyXrpVWBjU646862 = fAVVsKkpyXrpVWBjU53635409;     fAVVsKkpyXrpVWBjU53635409 = fAVVsKkpyXrpVWBjU9991183;     fAVVsKkpyXrpVWBjU9991183 = fAVVsKkpyXrpVWBjU32513360;     fAVVsKkpyXrpVWBjU32513360 = fAVVsKkpyXrpVWBjU91112641;     fAVVsKkpyXrpVWBjU91112641 = fAVVsKkpyXrpVWBjU3618810;     fAVVsKkpyXrpVWBjU3618810 = fAVVsKkpyXrpVWBjU14103394;     fAVVsKkpyXrpVWBjU14103394 = fAVVsKkpyXrpVWBjU88724410;     fAVVsKkpyXrpVWBjU88724410 = fAVVsKkpyXrpVWBjU54663846;     fAVVsKkpyXrpVWBjU54663846 = fAVVsKkpyXrpVWBjU15903900;     fAVVsKkpyXrpVWBjU15903900 = fAVVsKkpyXrpVWBjU61510513;     fAVVsKkpyXrpVWBjU61510513 = fAVVsKkpyXrpVWBjU56082978;     fAVVsKkpyXrpVWBjU56082978 = fAVVsKkpyXrpVWBjU32240541;     fAVVsKkpyXrpVWBjU32240541 = fAVVsKkpyXrpVWBjU22024669;     fAVVsKkpyXrpVWBjU22024669 = fAVVsKkpyXrpVWBjU43449100;     fAVVsKkpyXrpVWBjU43449100 = fAVVsKkpyXrpVWBjU92391606;     fAVVsKkpyXrpVWBjU92391606 = fAVVsKkpyXrpVWBjU76769836;     fAVVsKkpyXrpVWBjU76769836 = fAVVsKkpyXrpVWBjU44108061;     fAVVsKkpyXrpVWBjU44108061 = fAVVsKkpyXrpVWBjU22098915;     fAVVsKkpyXrpVWBjU22098915 = fAVVsKkpyXrpVWBjU594827;     fAVVsKkpyXrpVWBjU594827 = fAVVsKkpyXrpVWBjU5945893;     fAVVsKkpyXrpVWBjU5945893 = fAVVsKkpyXrpVWBjU27189188;     fAVVsKkpyXrpVWBjU27189188 = fAVVsKkpyXrpVWBjU4362407;     fAVVsKkpyXrpVWBjU4362407 = fAVVsKkpyXrpVWBjU46449089;     fAVVsKkpyXrpVWBjU46449089 = fAVVsKkpyXrpVWBjU54193873;     fAVVsKkpyXrpVWBjU54193873 = fAVVsKkpyXrpVWBjU65218277;     fAVVsKkpyXrpVWBjU65218277 = fAVVsKkpyXrpVWBjU34050621;     fAVVsKkpyXrpVWBjU34050621 = fAVVsKkpyXrpVWBjU48087311;     fAVVsKkpyXrpVWBjU48087311 = fAVVsKkpyXrpVWBjU52827631;     fAVVsKkpyXrpVWBjU52827631 = fAVVsKkpyXrpVWBjU83602416;     fAVVsKkpyXrpVWBjU83602416 = fAVVsKkpyXrpVWBjU46959251;     fAVVsKkpyXrpVWBjU46959251 = fAVVsKkpyXrpVWBjU8066563;     fAVVsKkpyXrpVWBjU8066563 = fAVVsKkpyXrpVWBjU38225073;     fAVVsKkpyXrpVWBjU38225073 = fAVVsKkpyXrpVWBjU37603335;     fAVVsKkpyXrpVWBjU37603335 = fAVVsKkpyXrpVWBjU39588682;     fAVVsKkpyXrpVWBjU39588682 = fAVVsKkpyXrpVWBjU73751062;     fAVVsKkpyXrpVWBjU73751062 = fAVVsKkpyXrpVWBjU92218717;     fAVVsKkpyXrpVWBjU92218717 = fAVVsKkpyXrpVWBjU6809885;     fAVVsKkpyXrpVWBjU6809885 = fAVVsKkpyXrpVWBjU79491007;     fAVVsKkpyXrpVWBjU79491007 = fAVVsKkpyXrpVWBjU8852204;     fAVVsKkpyXrpVWBjU8852204 = fAVVsKkpyXrpVWBjU97930976;     fAVVsKkpyXrpVWBjU97930976 = fAVVsKkpyXrpVWBjU37390144;     fAVVsKkpyXrpVWBjU37390144 = fAVVsKkpyXrpVWBjU26539547;     fAVVsKkpyXrpVWBjU26539547 = fAVVsKkpyXrpVWBjU56802471;     fAVVsKkpyXrpVWBjU56802471 = fAVVsKkpyXrpVWBjU8326925;     fAVVsKkpyXrpVWBjU8326925 = fAVVsKkpyXrpVWBjU33563211;     fAVVsKkpyXrpVWBjU33563211 = fAVVsKkpyXrpVWBjU634894;     fAVVsKkpyXrpVWBjU634894 = fAVVsKkpyXrpVWBjU73933396;     fAVVsKkpyXrpVWBjU73933396 = fAVVsKkpyXrpVWBjU25522314;     fAVVsKkpyXrpVWBjU25522314 = fAVVsKkpyXrpVWBjU99885269;     fAVVsKkpyXrpVWBjU99885269 = fAVVsKkpyXrpVWBjU37154125;     fAVVsKkpyXrpVWBjU37154125 = fAVVsKkpyXrpVWBjU96826847;     fAVVsKkpyXrpVWBjU96826847 = fAVVsKkpyXrpVWBjU11441455;     fAVVsKkpyXrpVWBjU11441455 = fAVVsKkpyXrpVWBjU56435435;     fAVVsKkpyXrpVWBjU56435435 = fAVVsKkpyXrpVWBjU72367516;     fAVVsKkpyXrpVWBjU72367516 = fAVVsKkpyXrpVWBjU73457674;     fAVVsKkpyXrpVWBjU73457674 = fAVVsKkpyXrpVWBjU49273002;     fAVVsKkpyXrpVWBjU49273002 = fAVVsKkpyXrpVWBjU63542094;     fAVVsKkpyXrpVWBjU63542094 = fAVVsKkpyXrpVWBjU78319487;     fAVVsKkpyXrpVWBjU78319487 = fAVVsKkpyXrpVWBjU25894365;     fAVVsKkpyXrpVWBjU25894365 = fAVVsKkpyXrpVWBjU69568188;     fAVVsKkpyXrpVWBjU69568188 = fAVVsKkpyXrpVWBjU66016083;     fAVVsKkpyXrpVWBjU66016083 = fAVVsKkpyXrpVWBjU35896780;     fAVVsKkpyXrpVWBjU35896780 = fAVVsKkpyXrpVWBjU71061429;     fAVVsKkpyXrpVWBjU71061429 = fAVVsKkpyXrpVWBjU68944649;     fAVVsKkpyXrpVWBjU68944649 = fAVVsKkpyXrpVWBjU53443950;     fAVVsKkpyXrpVWBjU53443950 = fAVVsKkpyXrpVWBjU17857906;     fAVVsKkpyXrpVWBjU17857906 = fAVVsKkpyXrpVWBjU94637205;     fAVVsKkpyXrpVWBjU94637205 = fAVVsKkpyXrpVWBjU82435987;     fAVVsKkpyXrpVWBjU82435987 = fAVVsKkpyXrpVWBjU69698038;     fAVVsKkpyXrpVWBjU69698038 = fAVVsKkpyXrpVWBjU172889;     fAVVsKkpyXrpVWBjU172889 = fAVVsKkpyXrpVWBjU69959951;     fAVVsKkpyXrpVWBjU69959951 = fAVVsKkpyXrpVWBjU64617053;     fAVVsKkpyXrpVWBjU64617053 = fAVVsKkpyXrpVWBjU13246712;     fAVVsKkpyXrpVWBjU13246712 = fAVVsKkpyXrpVWBjU2663851;     fAVVsKkpyXrpVWBjU2663851 = fAVVsKkpyXrpVWBjU68555749;     fAVVsKkpyXrpVWBjU68555749 = fAVVsKkpyXrpVWBjU649641;     fAVVsKkpyXrpVWBjU649641 = fAVVsKkpyXrpVWBjU47559936;     fAVVsKkpyXrpVWBjU47559936 = fAVVsKkpyXrpVWBjU15916745;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IUHtUapcUbtsEzQm10304402() {     double xYfzjqXJWkXjPgpCk22033851 = -692558085;    double xYfzjqXJWkXjPgpCk83608823 = -482662503;    double xYfzjqXJWkXjPgpCk43428654 = -440415709;    double xYfzjqXJWkXjPgpCk29284682 = -529459155;    double xYfzjqXJWkXjPgpCk8218734 = -172434903;    double xYfzjqXJWkXjPgpCk62020851 = -682130255;    double xYfzjqXJWkXjPgpCk72959979 = -628033769;    double xYfzjqXJWkXjPgpCk61794261 = -735417480;    double xYfzjqXJWkXjPgpCk90757049 = -805156475;    double xYfzjqXJWkXjPgpCk22433053 = -429289462;    double xYfzjqXJWkXjPgpCk32816661 = -159843574;    double xYfzjqXJWkXjPgpCk1660481 = -716965125;    double xYfzjqXJWkXjPgpCk75855308 = -70632429;    double xYfzjqXJWkXjPgpCk16019111 = -116795703;    double xYfzjqXJWkXjPgpCk91700491 = -277067801;    double xYfzjqXJWkXjPgpCk10476808 = -383436476;    double xYfzjqXJWkXjPgpCk86112878 = -84420948;    double xYfzjqXJWkXjPgpCk10977114 = -314458816;    double xYfzjqXJWkXjPgpCk2764750 = -289342087;    double xYfzjqXJWkXjPgpCk14090534 = -746643628;    double xYfzjqXJWkXjPgpCk45583454 = -989051407;    double xYfzjqXJWkXjPgpCk64834432 = -304216998;    double xYfzjqXJWkXjPgpCk36237654 = -573759127;    double xYfzjqXJWkXjPgpCk37627783 = -56330417;    double xYfzjqXJWkXjPgpCk53690833 = -366724539;    double xYfzjqXJWkXjPgpCk57235036 = -264588288;    double xYfzjqXJWkXjPgpCk89572419 = -903826019;    double xYfzjqXJWkXjPgpCk16725935 = -326793048;    double xYfzjqXJWkXjPgpCk80235649 = -868074968;    double xYfzjqXJWkXjPgpCk57340047 = -837328696;    double xYfzjqXJWkXjPgpCk46906396 = -321833505;    double xYfzjqXJWkXjPgpCk13514522 = -667517380;    double xYfzjqXJWkXjPgpCk71221461 = -91724993;    double xYfzjqXJWkXjPgpCk50913543 = -732698037;    double xYfzjqXJWkXjPgpCk93715192 = -259113879;    double xYfzjqXJWkXjPgpCk90750582 = -579449025;    double xYfzjqXJWkXjPgpCk24711035 = -279215067;    double xYfzjqXJWkXjPgpCk26481819 = -667545411;    double xYfzjqXJWkXjPgpCk50380090 = -515961555;    double xYfzjqXJWkXjPgpCk31609999 = -560940630;    double xYfzjqXJWkXjPgpCk27768139 = -794529377;    double xYfzjqXJWkXjPgpCk32745283 = 36792425;    double xYfzjqXJWkXjPgpCk84012871 = -455797320;    double xYfzjqXJWkXjPgpCk14596928 = -622591991;    double xYfzjqXJWkXjPgpCk16314872 = -338482908;    double xYfzjqXJWkXjPgpCk58388284 = -805861332;    double xYfzjqXJWkXjPgpCk71972447 = -621355783;    double xYfzjqXJWkXjPgpCk73175217 = -223436623;    double xYfzjqXJWkXjPgpCk43726350 = -629571886;    double xYfzjqXJWkXjPgpCk60998963 = -965786291;    double xYfzjqXJWkXjPgpCk88528309 = -28704013;    double xYfzjqXJWkXjPgpCk94412136 = -22487798;    double xYfzjqXJWkXjPgpCk95207708 = -54103360;    double xYfzjqXJWkXjPgpCk4035443 = -823583395;    double xYfzjqXJWkXjPgpCk64720947 = 56191459;    double xYfzjqXJWkXjPgpCk57199418 = -288341087;    double xYfzjqXJWkXjPgpCk47371169 = -908903376;    double xYfzjqXJWkXjPgpCk5800872 = -284085293;    double xYfzjqXJWkXjPgpCk75593848 = -62734616;    double xYfzjqXJWkXjPgpCk50983697 = -907846615;    double xYfzjqXJWkXjPgpCk72448431 = -778304237;    double xYfzjqXJWkXjPgpCk56234044 = -201240722;    double xYfzjqXJWkXjPgpCk81558611 = -867342512;    double xYfzjqXJWkXjPgpCk33417003 = -967827780;    double xYfzjqXJWkXjPgpCk75526657 = -7455958;    double xYfzjqXJWkXjPgpCk19302140 = -492326195;    double xYfzjqXJWkXjPgpCk30439019 = -525240133;    double xYfzjqXJWkXjPgpCk24941766 = -337934392;    double xYfzjqXJWkXjPgpCk22303919 = -857681825;    double xYfzjqXJWkXjPgpCk949910 = -697618777;    double xYfzjqXJWkXjPgpCk85765773 = -4221410;    double xYfzjqXJWkXjPgpCk59631060 = -416875537;    double xYfzjqXJWkXjPgpCk60597024 = -798497262;    double xYfzjqXJWkXjPgpCk71154751 = -728401458;    double xYfzjqXJWkXjPgpCk86322395 = -952114251;    double xYfzjqXJWkXjPgpCk12838172 = -925843832;    double xYfzjqXJWkXjPgpCk80821561 = -848419678;    double xYfzjqXJWkXjPgpCk21640726 = -951167137;    double xYfzjqXJWkXjPgpCk21312911 = -717847510;    double xYfzjqXJWkXjPgpCk95302548 = -560863208;    double xYfzjqXJWkXjPgpCk85262588 = -643232506;    double xYfzjqXJWkXjPgpCk16397203 = -580389396;    double xYfzjqXJWkXjPgpCk72999585 = -697221162;    double xYfzjqXJWkXjPgpCk19236686 = -902288678;    double xYfzjqXJWkXjPgpCk68811738 = -708624684;    double xYfzjqXJWkXjPgpCk52494260 = -199345708;    double xYfzjqXJWkXjPgpCk18306814 = -513414020;    double xYfzjqXJWkXjPgpCk67186018 = -268141599;    double xYfzjqXJWkXjPgpCk86192595 = -688889496;    double xYfzjqXJWkXjPgpCk36515775 = -970772792;    double xYfzjqXJWkXjPgpCk43379413 = -670545649;    double xYfzjqXJWkXjPgpCk18910163 = -995129775;    double xYfzjqXJWkXjPgpCk50887970 = -504810796;    double xYfzjqXJWkXjPgpCk99396392 = -608114940;    double xYfzjqXJWkXjPgpCk59161567 = -782636393;    double xYfzjqXJWkXjPgpCk71534094 = -493288656;    double xYfzjqXJWkXjPgpCk51186672 = -95865064;    double xYfzjqXJWkXjPgpCk50595869 = -487969541;    double xYfzjqXJWkXjPgpCk39070271 = -515136034;    double xYfzjqXJWkXjPgpCk97012732 = -692558085;     xYfzjqXJWkXjPgpCk22033851 = xYfzjqXJWkXjPgpCk83608823;     xYfzjqXJWkXjPgpCk83608823 = xYfzjqXJWkXjPgpCk43428654;     xYfzjqXJWkXjPgpCk43428654 = xYfzjqXJWkXjPgpCk29284682;     xYfzjqXJWkXjPgpCk29284682 = xYfzjqXJWkXjPgpCk8218734;     xYfzjqXJWkXjPgpCk8218734 = xYfzjqXJWkXjPgpCk62020851;     xYfzjqXJWkXjPgpCk62020851 = xYfzjqXJWkXjPgpCk72959979;     xYfzjqXJWkXjPgpCk72959979 = xYfzjqXJWkXjPgpCk61794261;     xYfzjqXJWkXjPgpCk61794261 = xYfzjqXJWkXjPgpCk90757049;     xYfzjqXJWkXjPgpCk90757049 = xYfzjqXJWkXjPgpCk22433053;     xYfzjqXJWkXjPgpCk22433053 = xYfzjqXJWkXjPgpCk32816661;     xYfzjqXJWkXjPgpCk32816661 = xYfzjqXJWkXjPgpCk1660481;     xYfzjqXJWkXjPgpCk1660481 = xYfzjqXJWkXjPgpCk75855308;     xYfzjqXJWkXjPgpCk75855308 = xYfzjqXJWkXjPgpCk16019111;     xYfzjqXJWkXjPgpCk16019111 = xYfzjqXJWkXjPgpCk91700491;     xYfzjqXJWkXjPgpCk91700491 = xYfzjqXJWkXjPgpCk10476808;     xYfzjqXJWkXjPgpCk10476808 = xYfzjqXJWkXjPgpCk86112878;     xYfzjqXJWkXjPgpCk86112878 = xYfzjqXJWkXjPgpCk10977114;     xYfzjqXJWkXjPgpCk10977114 = xYfzjqXJWkXjPgpCk2764750;     xYfzjqXJWkXjPgpCk2764750 = xYfzjqXJWkXjPgpCk14090534;     xYfzjqXJWkXjPgpCk14090534 = xYfzjqXJWkXjPgpCk45583454;     xYfzjqXJWkXjPgpCk45583454 = xYfzjqXJWkXjPgpCk64834432;     xYfzjqXJWkXjPgpCk64834432 = xYfzjqXJWkXjPgpCk36237654;     xYfzjqXJWkXjPgpCk36237654 = xYfzjqXJWkXjPgpCk37627783;     xYfzjqXJWkXjPgpCk37627783 = xYfzjqXJWkXjPgpCk53690833;     xYfzjqXJWkXjPgpCk53690833 = xYfzjqXJWkXjPgpCk57235036;     xYfzjqXJWkXjPgpCk57235036 = xYfzjqXJWkXjPgpCk89572419;     xYfzjqXJWkXjPgpCk89572419 = xYfzjqXJWkXjPgpCk16725935;     xYfzjqXJWkXjPgpCk16725935 = xYfzjqXJWkXjPgpCk80235649;     xYfzjqXJWkXjPgpCk80235649 = xYfzjqXJWkXjPgpCk57340047;     xYfzjqXJWkXjPgpCk57340047 = xYfzjqXJWkXjPgpCk46906396;     xYfzjqXJWkXjPgpCk46906396 = xYfzjqXJWkXjPgpCk13514522;     xYfzjqXJWkXjPgpCk13514522 = xYfzjqXJWkXjPgpCk71221461;     xYfzjqXJWkXjPgpCk71221461 = xYfzjqXJWkXjPgpCk50913543;     xYfzjqXJWkXjPgpCk50913543 = xYfzjqXJWkXjPgpCk93715192;     xYfzjqXJWkXjPgpCk93715192 = xYfzjqXJWkXjPgpCk90750582;     xYfzjqXJWkXjPgpCk90750582 = xYfzjqXJWkXjPgpCk24711035;     xYfzjqXJWkXjPgpCk24711035 = xYfzjqXJWkXjPgpCk26481819;     xYfzjqXJWkXjPgpCk26481819 = xYfzjqXJWkXjPgpCk50380090;     xYfzjqXJWkXjPgpCk50380090 = xYfzjqXJWkXjPgpCk31609999;     xYfzjqXJWkXjPgpCk31609999 = xYfzjqXJWkXjPgpCk27768139;     xYfzjqXJWkXjPgpCk27768139 = xYfzjqXJWkXjPgpCk32745283;     xYfzjqXJWkXjPgpCk32745283 = xYfzjqXJWkXjPgpCk84012871;     xYfzjqXJWkXjPgpCk84012871 = xYfzjqXJWkXjPgpCk14596928;     xYfzjqXJWkXjPgpCk14596928 = xYfzjqXJWkXjPgpCk16314872;     xYfzjqXJWkXjPgpCk16314872 = xYfzjqXJWkXjPgpCk58388284;     xYfzjqXJWkXjPgpCk58388284 = xYfzjqXJWkXjPgpCk71972447;     xYfzjqXJWkXjPgpCk71972447 = xYfzjqXJWkXjPgpCk73175217;     xYfzjqXJWkXjPgpCk73175217 = xYfzjqXJWkXjPgpCk43726350;     xYfzjqXJWkXjPgpCk43726350 = xYfzjqXJWkXjPgpCk60998963;     xYfzjqXJWkXjPgpCk60998963 = xYfzjqXJWkXjPgpCk88528309;     xYfzjqXJWkXjPgpCk88528309 = xYfzjqXJWkXjPgpCk94412136;     xYfzjqXJWkXjPgpCk94412136 = xYfzjqXJWkXjPgpCk95207708;     xYfzjqXJWkXjPgpCk95207708 = xYfzjqXJWkXjPgpCk4035443;     xYfzjqXJWkXjPgpCk4035443 = xYfzjqXJWkXjPgpCk64720947;     xYfzjqXJWkXjPgpCk64720947 = xYfzjqXJWkXjPgpCk57199418;     xYfzjqXJWkXjPgpCk57199418 = xYfzjqXJWkXjPgpCk47371169;     xYfzjqXJWkXjPgpCk47371169 = xYfzjqXJWkXjPgpCk5800872;     xYfzjqXJWkXjPgpCk5800872 = xYfzjqXJWkXjPgpCk75593848;     xYfzjqXJWkXjPgpCk75593848 = xYfzjqXJWkXjPgpCk50983697;     xYfzjqXJWkXjPgpCk50983697 = xYfzjqXJWkXjPgpCk72448431;     xYfzjqXJWkXjPgpCk72448431 = xYfzjqXJWkXjPgpCk56234044;     xYfzjqXJWkXjPgpCk56234044 = xYfzjqXJWkXjPgpCk81558611;     xYfzjqXJWkXjPgpCk81558611 = xYfzjqXJWkXjPgpCk33417003;     xYfzjqXJWkXjPgpCk33417003 = xYfzjqXJWkXjPgpCk75526657;     xYfzjqXJWkXjPgpCk75526657 = xYfzjqXJWkXjPgpCk19302140;     xYfzjqXJWkXjPgpCk19302140 = xYfzjqXJWkXjPgpCk30439019;     xYfzjqXJWkXjPgpCk30439019 = xYfzjqXJWkXjPgpCk24941766;     xYfzjqXJWkXjPgpCk24941766 = xYfzjqXJWkXjPgpCk22303919;     xYfzjqXJWkXjPgpCk22303919 = xYfzjqXJWkXjPgpCk949910;     xYfzjqXJWkXjPgpCk949910 = xYfzjqXJWkXjPgpCk85765773;     xYfzjqXJWkXjPgpCk85765773 = xYfzjqXJWkXjPgpCk59631060;     xYfzjqXJWkXjPgpCk59631060 = xYfzjqXJWkXjPgpCk60597024;     xYfzjqXJWkXjPgpCk60597024 = xYfzjqXJWkXjPgpCk71154751;     xYfzjqXJWkXjPgpCk71154751 = xYfzjqXJWkXjPgpCk86322395;     xYfzjqXJWkXjPgpCk86322395 = xYfzjqXJWkXjPgpCk12838172;     xYfzjqXJWkXjPgpCk12838172 = xYfzjqXJWkXjPgpCk80821561;     xYfzjqXJWkXjPgpCk80821561 = xYfzjqXJWkXjPgpCk21640726;     xYfzjqXJWkXjPgpCk21640726 = xYfzjqXJWkXjPgpCk21312911;     xYfzjqXJWkXjPgpCk21312911 = xYfzjqXJWkXjPgpCk95302548;     xYfzjqXJWkXjPgpCk95302548 = xYfzjqXJWkXjPgpCk85262588;     xYfzjqXJWkXjPgpCk85262588 = xYfzjqXJWkXjPgpCk16397203;     xYfzjqXJWkXjPgpCk16397203 = xYfzjqXJWkXjPgpCk72999585;     xYfzjqXJWkXjPgpCk72999585 = xYfzjqXJWkXjPgpCk19236686;     xYfzjqXJWkXjPgpCk19236686 = xYfzjqXJWkXjPgpCk68811738;     xYfzjqXJWkXjPgpCk68811738 = xYfzjqXJWkXjPgpCk52494260;     xYfzjqXJWkXjPgpCk52494260 = xYfzjqXJWkXjPgpCk18306814;     xYfzjqXJWkXjPgpCk18306814 = xYfzjqXJWkXjPgpCk67186018;     xYfzjqXJWkXjPgpCk67186018 = xYfzjqXJWkXjPgpCk86192595;     xYfzjqXJWkXjPgpCk86192595 = xYfzjqXJWkXjPgpCk36515775;     xYfzjqXJWkXjPgpCk36515775 = xYfzjqXJWkXjPgpCk43379413;     xYfzjqXJWkXjPgpCk43379413 = xYfzjqXJWkXjPgpCk18910163;     xYfzjqXJWkXjPgpCk18910163 = xYfzjqXJWkXjPgpCk50887970;     xYfzjqXJWkXjPgpCk50887970 = xYfzjqXJWkXjPgpCk99396392;     xYfzjqXJWkXjPgpCk99396392 = xYfzjqXJWkXjPgpCk59161567;     xYfzjqXJWkXjPgpCk59161567 = xYfzjqXJWkXjPgpCk71534094;     xYfzjqXJWkXjPgpCk71534094 = xYfzjqXJWkXjPgpCk51186672;     xYfzjqXJWkXjPgpCk51186672 = xYfzjqXJWkXjPgpCk50595869;     xYfzjqXJWkXjPgpCk50595869 = xYfzjqXJWkXjPgpCk39070271;     xYfzjqXJWkXjPgpCk39070271 = xYfzjqXJWkXjPgpCk97012732;     xYfzjqXJWkXjPgpCk97012732 = xYfzjqXJWkXjPgpCk22033851;}
// Junk Finished

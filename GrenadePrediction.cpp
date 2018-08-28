#include "GrenadePrediction.h"
#include "Render.h"

void CCSGrenadeHint::Tick(int buttons)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	bool in_attack = buttons & IN_ATTACK;
	bool in_attack2 = buttons & IN_ATTACK2;

	act = (in_attack && in_attack2) ? ACT_LOB :
		(in_attack2) ? ACT_DROP :
		(in_attack) ? ACT_THROW :
		ACT_NONE;
}
void CCSGrenadeHint::View(CViewSetup* setup)
{
	auto local = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	if (local && local->IsAlive())
	{
		CBaseCombatWeapon* weapon = (CBaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(local->GetActiveWeaponHandle());
		if (weapon && MiscFunctions::IsGrenade(weapon) && act != ACT_NONE)
		{
			type = weapon->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
			Simulate(setup);
		}
		else
		{
			type = 0;
		}
	}

}

void CCSGrenadeHint::Paint()
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	if ((type) && path.size()>1)
	{
		Vector nadeStart, nadeEnd;

		Color lineColor(int(g_Options.Colors.color_grenadeprediction[0] * 255), int(g_Options.Colors.color_grenadeprediction[1] * 255), int(g_Options.Colors.color_grenadeprediction[2] * 255), 255);
		Vector prev = path[0];
		for (auto it = path.begin(), end = path.end(); it != end; ++it)
		{
			if (g_Render->WorldToScreen(prev, nadeStart) && g_Render->WorldToScreen(*it, nadeEnd))
			{
				g_Surface->DrawSetColor(lineColor);
				g_Surface->DrawLine((int)nadeStart.x, (int)nadeStart.y, (int)nadeEnd.x, (int)nadeEnd.y);
			}
			prev = *it;
		}
		for (auto it = OtherCollisions.begin(), end = OtherCollisions.end(); it != end; ++it)
		{
			g_Render->Draw3DCube(2.f, it->second, it->first, boxColor);
		}

		g_Render->Draw3DCube(2.f, OtherCollisions.rbegin()->second, OtherCollisions.rbegin()->first, boxColor);

		if (g_Render->WorldToScreen(prev, nadeEnd))
		{
			Color circleLine(int(g_Options.Colors.color_grenadeprediction_circle[0] * 255), int(g_Options.Colors.color_grenadeprediction_circle[1] * 255), int(g_Options.Colors.color_grenadeprediction_circle[2] * 255), 255);

			g_Surface->DrawSetColor(circleLine);
			g_Surface->DrawOutlinedCircle((int)nadeEnd.x, (int)nadeEnd.y, 10, 48);
		}
	}
}
static const constexpr auto PIRAD = 0.01745329251f;
void angle_vectors2(const Vector &angles, Vector *forward, Vector *right, Vector *up)
{
	float sr, sp, sy, cr, cp, cy;

	sp = static_cast<float>(sin(double(angles.x) * PIRAD));
	cp = static_cast<float>(cos(double(angles.x) * PIRAD));
	sy = static_cast<float>(sin(double(angles.y) * PIRAD));
	cy = static_cast<float>(cos(double(angles.y) * PIRAD));
	sr = static_cast<float>(sin(double(angles.z) * PIRAD));
	cr = static_cast<float>(cos(double(angles.z) * PIRAD));

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right)
	{
		right->x = (-1 * sr*sp*cy + -1 * cr*-sy);
		right->y = (-1 * sr*sp*sy + -1 * cr*cy);
		right->z = -1 * sr*cp;
	}

	if (up)
	{
		up->x = (cr*sp*cy + -sr * -sy);
		up->y = (cr*sp*sy + -sr * cy);
		up->z = cr * cp;
	}
}
void CCSGrenadeHint::Setup(Vector& vecSrc, Vector& vecThrow, Vector viewangles)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	Vector angThrow = viewangles;
	auto local = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	float pitch = angThrow.x;

	if (pitch <= 90.0f)
	{
		if (pitch<-90.0f)
		{
			pitch += 360.0f;
		}
	}
	else
	{
		pitch -= 360.0f;
	}
	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.x = a;

	// Gets ThrowVelocity from weapon files
	// Clamped to [15,750]
	float flVel = 750.0f * 0.9f;

	// Do magic on member of grenade object [esi+9E4h]
	// m1=1  m1+m2=0.5  m2=0
	static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	float b = power[act];
	// Clamped to [0,1]
	b = b * 0.7f;
	b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	angle_vectors2(angThrow, &vForward, &vRight, &vUp); //angThrow.ToVector(vForward, vRight, vUp);

	vecSrc = local->GetEyePosition();
	float off = (power[act] * 12.0f) - 12.0f;
	vecSrc.z += off;

	// Game calls UTIL_TraceHull here with hull and assigns vecSrc tr.endpos
	trace_t tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f; //vecDest.MultAdd(vForward, 22.0f);

	TraceHull(vecSrc, vecDest, tr);

	// After the hull trace it moves 6 units back along vForward
	// vecSrc = tr.endpos - vForward * 6
	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	// Finally calculate velocity
	vecThrow = local->GetVelocity(); vecThrow *= 1.25f;
	vecThrow += vForward * flVel; //	vecThrow.MultAdd(vForward, flVel);
}

void CCSGrenadeHint::Simulate(CViewSetup* setup)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	Vector vecSrc, vecThrow;
	Vector angles; g_Engine->GetViewAngles(angles);
	Setup(vecSrc, vecThrow, angles);

	float interval = g_Globals->interval_per_tick;

	// Log positions 20 times per sec
	int logstep = static_cast<int>(0.05f / interval);
	int logtimer = 0;


	path.clear();
	OtherCollisions.clear();
	for (unsigned int i = 0; i<path.max_size() - 1; ++i)
	{
		if (!logtimer)
			path.push_back(vecSrc);

		int s = Step(vecSrc, vecThrow, i, interval);
		if ((s & 1)) break;

		// Reset the log timer every logstep OR we bounced
		if ((s & 2) || logtimer >= logstep) logtimer = 0;
		else ++logtimer;
	}
	path.push_back(vecSrc);
}

int CCSGrenadeHint::Step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
{

	// Apply gravity
	Vector move;
	AddGravityMove(move, vecThrow, interval, false);

	// Push entity
	trace_t tr;
	PushEntity(vecSrc, move, tr);

	int result = 0;
	// Check ending conditions
	if (CheckDetonate(vecThrow, tr, tick, interval))
	{
		result |= 1;
	}

	// Resolve collisions
	if (tr.fraction != 1.0f)
	{
		result |= 2; // Collision!
		ResolveFlyCollisionCustom(tr, vecThrow, interval);
	}

	if ((result & 1) || vecThrow == Vector(0, 0, 0) || tr.fraction != 1.0f)
	{
		QAngle angles;
		VectorAngles((tr.endpos - tr.startpos).Normalized(), angles);
		OtherCollisions.push_back(std::make_pair(tr.endpos, angles));
	}

	// Set new position
	vecSrc = tr.endpos;
	return result;
}


bool CCSGrenadeHint::CheckDetonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
{
	switch (type)
	{
	case WEAPON_SMOKE:
	case WEAPON_DECOY:
		// Velocity must be <0.1, this is only checked every 0.2s
		if (vecThrow.Length2D()<0.1f)
		{
			int det_tick_mod = static_cast<int>(0.2f / interval);
			return !(tick%det_tick_mod);
		}
		return false;

	case WEAPON_MOLOTOV:
	case WEAPON_INC:
		// Detonate when hitting the floor
		if (tr.fraction != 1.0f && tr.plane.normal.z>0.7f)
			return true;
		// OR we've been flying for too long

	case WEAPON_FLASH:
	case WEAPON_HE:
		// Pure timer based, detonate at 1.5s, checked every 0.2s
		return static_cast<float>(tick)*interval>1.5f && !(tick%static_cast<int>(0.2f / interval));

	default:
		assert(false);
		return false;

	}
}

void CCSGrenadeHint::TraceHull(Vector& src, Vector& end, trace_t& tr)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;

	static const Vector hull[2] = { Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f) };

	CTraceFilter filter;
	filter.SetIgnoreClass("BaseCSGrenadeProjectile");
	filter.pSkip = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());

	Ray_t ray;
	ray.Init(src, end, hull[0], hull[1]);

	g_EngineTrace->TraceRay(ray, 0x200400B, &filter, &tr);
}

void CCSGrenadeHint::AddGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	Vector basevel(0.0f, 0.0f, 0.0f);

	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if (onground)
	{
		move.z = (vel.z + basevel.z) * frametime;
	}
	else
	{
		// Game calls GetActualGravity( this );
		float gravity = 800.0f * 0.4f;

		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;

		vel.z = newZ;
	}

}

void CCSGrenadeHint::PushEntity(Vector& src, const Vector& move, trace_t& tr)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	Vector vecAbsEnd = src;
	vecAbsEnd += move;

	// Trace through world
	TraceHull(src, vecAbsEnd, tr);

}

void CCSGrenadeHint::ResolveFlyCollisionCustom(trace_t& tr, Vector& vecVelocity, float interval)
{
	if (!g_Options.Visuals.GrenadePrediction)
		return;
	// Calculate elasticity
	float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
	float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity>0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity<0.0f) flTotalElasticity = 0.0f;

	// Calculate bounce
	Vector vecAbsVelocity;
	PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	// Stop completely once we move too slow
	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
	if (flSpeedSqr<flMinSpeedSqr)
	{
		//vecAbsVelocity.Zero();
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	// Stop if on ground
	if (tr.plane.normal.z>0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval); //vecAbsVelocity.Mult((1.0f - tr.fraction) * interval);
		PushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
	{
		vecVelocity = vecAbsVelocity;
	}

}

int CCSGrenadeHint::PhysicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float    backoff;
	float    change;
	float    angle;
	int        i, blocked;

	blocked = 0;

	angle = normal[2];

	if (angle > 0)
	{
		blocked |= 1;        // floor
	}
	if (!angle)
	{
		blocked |= 2;        // step
	}

	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i<3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
		{
			out[i] = 0;
		}
	}
	return blocked;
}




































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// Junk Code By Troll Face & Thaisen's Gen
void EbbQQngsjFNlhNkB63309599() {     double uWeBwBwkjnWwbnSOc69602815 = -376738159;    double uWeBwBwkjnWwbnSOc64053986 = -610574321;    double uWeBwBwkjnWwbnSOc6322888 = -484775153;    double uWeBwBwkjnWwbnSOc48455336 = -163266206;    double uWeBwBwkjnWwbnSOc98837713 = -544271901;    double uWeBwBwkjnWwbnSOc63089732 = -829547483;    double uWeBwBwkjnWwbnSOc55825781 = -334654458;    double uWeBwBwkjnWwbnSOc98250143 = -82206880;    double uWeBwBwkjnWwbnSOc27600251 = 34910594;    double uWeBwBwkjnWwbnSOc30533814 = -334777487;    double uWeBwBwkjnWwbnSOc87631642 = 96595464;    double uWeBwBwkjnWwbnSOc15351931 = -687866368;    double uWeBwBwkjnWwbnSOc54898388 = -603957587;    double uWeBwBwkjnWwbnSOc85090551 = -11579267;    double uWeBwBwkjnWwbnSOc52015956 = -881858729;    double uWeBwBwkjnWwbnSOc38154761 = -593349461;    double uWeBwBwkjnWwbnSOc47113673 = -151880767;    double uWeBwBwkjnWwbnSOc45522086 = -746457738;    double uWeBwBwkjnWwbnSOc5842634 = -741490990;    double uWeBwBwkjnWwbnSOc26546898 = -910464002;    double uWeBwBwkjnWwbnSOc99516612 = -476173363;    double uWeBwBwkjnWwbnSOc54801477 = -561019180;    double uWeBwBwkjnWwbnSOc77139204 = -458368287;    double uWeBwBwkjnWwbnSOc37569808 = -786601958;    double uWeBwBwkjnWwbnSOc32103472 = -822266763;    double uWeBwBwkjnWwbnSOc71836128 = -279305406;    double uWeBwBwkjnWwbnSOc58902321 = -726390798;    double uWeBwBwkjnWwbnSOc22710056 = -29373104;    double uWeBwBwkjnWwbnSOc92045180 = -438652097;    double uWeBwBwkjnWwbnSOc23706401 = -635566243;    double uWeBwBwkjnWwbnSOc18129896 = -853427452;    double uWeBwBwkjnWwbnSOc49895101 = -127545377;    double uWeBwBwkjnWwbnSOc66404061 = -424230215;    double uWeBwBwkjnWwbnSOc34726064 = -277682996;    double uWeBwBwkjnWwbnSOc40246034 = -231812062;    double uWeBwBwkjnWwbnSOc61386585 = -110549514;    double uWeBwBwkjnWwbnSOc70695075 = -775813860;    double uWeBwBwkjnWwbnSOc19295987 = -120580051;    double uWeBwBwkjnWwbnSOc92142431 = -541015761;    double uWeBwBwkjnWwbnSOc29499209 = -286129177;    double uWeBwBwkjnWwbnSOc63266141 = -518438436;    double uWeBwBwkjnWwbnSOc96198399 = -926517805;    double uWeBwBwkjnWwbnSOc39690635 = -835464848;    double uWeBwBwkjnWwbnSOc18600208 = -884958664;    double uWeBwBwkjnWwbnSOc4820584 = -716703817;    double uWeBwBwkjnWwbnSOc5608038 = -492943794;    double uWeBwBwkjnWwbnSOc55574659 = -274680464;    double uWeBwBwkjnWwbnSOc91683139 = -856035389;    double uWeBwBwkjnWwbnSOc66696079 = -401591968;    double uWeBwBwkjnWwbnSOc8889513 = -28220772;    double uWeBwBwkjnWwbnSOc43313370 = -416464662;    double uWeBwBwkjnWwbnSOc13865613 = -263614469;    double uWeBwBwkjnWwbnSOc59779163 = -864744375;    double uWeBwBwkjnWwbnSOc57363538 = 78623180;    double uWeBwBwkjnWwbnSOc63792987 = -785902175;    double uWeBwBwkjnWwbnSOc14801339 = -815718979;    double uWeBwBwkjnWwbnSOc86914781 = -52206034;    double uWeBwBwkjnWwbnSOc68753080 = -698173195;    double uWeBwBwkjnWwbnSOc16351865 = -340999443;    double uWeBwBwkjnWwbnSOc27001585 = -164966496;    double uWeBwBwkjnWwbnSOc4187412 = -3156685;    double uWeBwBwkjnWwbnSOc33115725 = -205281354;    double uWeBwBwkjnWwbnSOc6204964 = -643554783;    double uWeBwBwkjnWwbnSOc3893851 = -329523163;    double uWeBwBwkjnWwbnSOc12403919 = -481350035;    double uWeBwBwkjnWwbnSOc37736541 = -775859159;    double uWeBwBwkjnWwbnSOc48947870 = -163636153;    double uWeBwBwkjnWwbnSOc20172324 = -226274592;    double uWeBwBwkjnWwbnSOc44844517 = -779767205;    double uWeBwBwkjnWwbnSOc90629370 = -671309215;    double uWeBwBwkjnWwbnSOc67459686 = -817535602;    double uWeBwBwkjnWwbnSOc27817686 = 68699283;    double uWeBwBwkjnWwbnSOc53379655 = -105441977;    double uWeBwBwkjnWwbnSOc76343424 = -355361814;    double uWeBwBwkjnWwbnSOc63280757 = -292025567;    double uWeBwBwkjnWwbnSOc3318213 = -549655559;    double uWeBwBwkjnWwbnSOc15110842 = -725554332;    double uWeBwBwkjnWwbnSOc58538997 = -573409623;    double uWeBwBwkjnWwbnSOc32749224 = 30101859;    double uWeBwBwkjnWwbnSOc26495435 = -229322969;    double uWeBwBwkjnWwbnSOc16261470 = 95375058;    double uWeBwBwkjnWwbnSOc67219181 = -870355409;    double uWeBwBwkjnWwbnSOc56013977 = -627781137;    double uWeBwBwkjnWwbnSOc83155667 = -310431325;    double uWeBwBwkjnWwbnSOc80393031 = -119101582;    double uWeBwBwkjnWwbnSOc4264283 = -489812983;    double uWeBwBwkjnWwbnSOc90115937 = -262801002;    double uWeBwBwkjnWwbnSOc9040523 = -402853396;    double uWeBwBwkjnWwbnSOc70933077 = -491780821;    double uWeBwBwkjnWwbnSOc25444696 = -416093083;    double uWeBwBwkjnWwbnSOc74471803 = 41656520;    double uWeBwBwkjnWwbnSOc1941996 = 22359335;    double uWeBwBwkjnWwbnSOc2944123 = -779580608;    double uWeBwBwkjnWwbnSOc65140846 = -276049266;    double uWeBwBwkjnWwbnSOc25311797 = -182972493;    double uWeBwBwkjnWwbnSOc30150416 = -213157082;    double uWeBwBwkjnWwbnSOc89993435 = -182963022;    double uWeBwBwkjnWwbnSOc35796785 = -405941686;    double uWeBwBwkjnWwbnSOc6196289 = -303608629;    double uWeBwBwkjnWwbnSOc67084042 = -376738159;     uWeBwBwkjnWwbnSOc69602815 = uWeBwBwkjnWwbnSOc64053986;     uWeBwBwkjnWwbnSOc64053986 = uWeBwBwkjnWwbnSOc6322888;     uWeBwBwkjnWwbnSOc6322888 = uWeBwBwkjnWwbnSOc48455336;     uWeBwBwkjnWwbnSOc48455336 = uWeBwBwkjnWwbnSOc98837713;     uWeBwBwkjnWwbnSOc98837713 = uWeBwBwkjnWwbnSOc63089732;     uWeBwBwkjnWwbnSOc63089732 = uWeBwBwkjnWwbnSOc55825781;     uWeBwBwkjnWwbnSOc55825781 = uWeBwBwkjnWwbnSOc98250143;     uWeBwBwkjnWwbnSOc98250143 = uWeBwBwkjnWwbnSOc27600251;     uWeBwBwkjnWwbnSOc27600251 = uWeBwBwkjnWwbnSOc30533814;     uWeBwBwkjnWwbnSOc30533814 = uWeBwBwkjnWwbnSOc87631642;     uWeBwBwkjnWwbnSOc87631642 = uWeBwBwkjnWwbnSOc15351931;     uWeBwBwkjnWwbnSOc15351931 = uWeBwBwkjnWwbnSOc54898388;     uWeBwBwkjnWwbnSOc54898388 = uWeBwBwkjnWwbnSOc85090551;     uWeBwBwkjnWwbnSOc85090551 = uWeBwBwkjnWwbnSOc52015956;     uWeBwBwkjnWwbnSOc52015956 = uWeBwBwkjnWwbnSOc38154761;     uWeBwBwkjnWwbnSOc38154761 = uWeBwBwkjnWwbnSOc47113673;     uWeBwBwkjnWwbnSOc47113673 = uWeBwBwkjnWwbnSOc45522086;     uWeBwBwkjnWwbnSOc45522086 = uWeBwBwkjnWwbnSOc5842634;     uWeBwBwkjnWwbnSOc5842634 = uWeBwBwkjnWwbnSOc26546898;     uWeBwBwkjnWwbnSOc26546898 = uWeBwBwkjnWwbnSOc99516612;     uWeBwBwkjnWwbnSOc99516612 = uWeBwBwkjnWwbnSOc54801477;     uWeBwBwkjnWwbnSOc54801477 = uWeBwBwkjnWwbnSOc77139204;     uWeBwBwkjnWwbnSOc77139204 = uWeBwBwkjnWwbnSOc37569808;     uWeBwBwkjnWwbnSOc37569808 = uWeBwBwkjnWwbnSOc32103472;     uWeBwBwkjnWwbnSOc32103472 = uWeBwBwkjnWwbnSOc71836128;     uWeBwBwkjnWwbnSOc71836128 = uWeBwBwkjnWwbnSOc58902321;     uWeBwBwkjnWwbnSOc58902321 = uWeBwBwkjnWwbnSOc22710056;     uWeBwBwkjnWwbnSOc22710056 = uWeBwBwkjnWwbnSOc92045180;     uWeBwBwkjnWwbnSOc92045180 = uWeBwBwkjnWwbnSOc23706401;     uWeBwBwkjnWwbnSOc23706401 = uWeBwBwkjnWwbnSOc18129896;     uWeBwBwkjnWwbnSOc18129896 = uWeBwBwkjnWwbnSOc49895101;     uWeBwBwkjnWwbnSOc49895101 = uWeBwBwkjnWwbnSOc66404061;     uWeBwBwkjnWwbnSOc66404061 = uWeBwBwkjnWwbnSOc34726064;     uWeBwBwkjnWwbnSOc34726064 = uWeBwBwkjnWwbnSOc40246034;     uWeBwBwkjnWwbnSOc40246034 = uWeBwBwkjnWwbnSOc61386585;     uWeBwBwkjnWwbnSOc61386585 = uWeBwBwkjnWwbnSOc70695075;     uWeBwBwkjnWwbnSOc70695075 = uWeBwBwkjnWwbnSOc19295987;     uWeBwBwkjnWwbnSOc19295987 = uWeBwBwkjnWwbnSOc92142431;     uWeBwBwkjnWwbnSOc92142431 = uWeBwBwkjnWwbnSOc29499209;     uWeBwBwkjnWwbnSOc29499209 = uWeBwBwkjnWwbnSOc63266141;     uWeBwBwkjnWwbnSOc63266141 = uWeBwBwkjnWwbnSOc96198399;     uWeBwBwkjnWwbnSOc96198399 = uWeBwBwkjnWwbnSOc39690635;     uWeBwBwkjnWwbnSOc39690635 = uWeBwBwkjnWwbnSOc18600208;     uWeBwBwkjnWwbnSOc18600208 = uWeBwBwkjnWwbnSOc4820584;     uWeBwBwkjnWwbnSOc4820584 = uWeBwBwkjnWwbnSOc5608038;     uWeBwBwkjnWwbnSOc5608038 = uWeBwBwkjnWwbnSOc55574659;     uWeBwBwkjnWwbnSOc55574659 = uWeBwBwkjnWwbnSOc91683139;     uWeBwBwkjnWwbnSOc91683139 = uWeBwBwkjnWwbnSOc66696079;     uWeBwBwkjnWwbnSOc66696079 = uWeBwBwkjnWwbnSOc8889513;     uWeBwBwkjnWwbnSOc8889513 = uWeBwBwkjnWwbnSOc43313370;     uWeBwBwkjnWwbnSOc43313370 = uWeBwBwkjnWwbnSOc13865613;     uWeBwBwkjnWwbnSOc13865613 = uWeBwBwkjnWwbnSOc59779163;     uWeBwBwkjnWwbnSOc59779163 = uWeBwBwkjnWwbnSOc57363538;     uWeBwBwkjnWwbnSOc57363538 = uWeBwBwkjnWwbnSOc63792987;     uWeBwBwkjnWwbnSOc63792987 = uWeBwBwkjnWwbnSOc14801339;     uWeBwBwkjnWwbnSOc14801339 = uWeBwBwkjnWwbnSOc86914781;     uWeBwBwkjnWwbnSOc86914781 = uWeBwBwkjnWwbnSOc68753080;     uWeBwBwkjnWwbnSOc68753080 = uWeBwBwkjnWwbnSOc16351865;     uWeBwBwkjnWwbnSOc16351865 = uWeBwBwkjnWwbnSOc27001585;     uWeBwBwkjnWwbnSOc27001585 = uWeBwBwkjnWwbnSOc4187412;     uWeBwBwkjnWwbnSOc4187412 = uWeBwBwkjnWwbnSOc33115725;     uWeBwBwkjnWwbnSOc33115725 = uWeBwBwkjnWwbnSOc6204964;     uWeBwBwkjnWwbnSOc6204964 = uWeBwBwkjnWwbnSOc3893851;     uWeBwBwkjnWwbnSOc3893851 = uWeBwBwkjnWwbnSOc12403919;     uWeBwBwkjnWwbnSOc12403919 = uWeBwBwkjnWwbnSOc37736541;     uWeBwBwkjnWwbnSOc37736541 = uWeBwBwkjnWwbnSOc48947870;     uWeBwBwkjnWwbnSOc48947870 = uWeBwBwkjnWwbnSOc20172324;     uWeBwBwkjnWwbnSOc20172324 = uWeBwBwkjnWwbnSOc44844517;     uWeBwBwkjnWwbnSOc44844517 = uWeBwBwkjnWwbnSOc90629370;     uWeBwBwkjnWwbnSOc90629370 = uWeBwBwkjnWwbnSOc67459686;     uWeBwBwkjnWwbnSOc67459686 = uWeBwBwkjnWwbnSOc27817686;     uWeBwBwkjnWwbnSOc27817686 = uWeBwBwkjnWwbnSOc53379655;     uWeBwBwkjnWwbnSOc53379655 = uWeBwBwkjnWwbnSOc76343424;     uWeBwBwkjnWwbnSOc76343424 = uWeBwBwkjnWwbnSOc63280757;     uWeBwBwkjnWwbnSOc63280757 = uWeBwBwkjnWwbnSOc3318213;     uWeBwBwkjnWwbnSOc3318213 = uWeBwBwkjnWwbnSOc15110842;     uWeBwBwkjnWwbnSOc15110842 = uWeBwBwkjnWwbnSOc58538997;     uWeBwBwkjnWwbnSOc58538997 = uWeBwBwkjnWwbnSOc32749224;     uWeBwBwkjnWwbnSOc32749224 = uWeBwBwkjnWwbnSOc26495435;     uWeBwBwkjnWwbnSOc26495435 = uWeBwBwkjnWwbnSOc16261470;     uWeBwBwkjnWwbnSOc16261470 = uWeBwBwkjnWwbnSOc67219181;     uWeBwBwkjnWwbnSOc67219181 = uWeBwBwkjnWwbnSOc56013977;     uWeBwBwkjnWwbnSOc56013977 = uWeBwBwkjnWwbnSOc83155667;     uWeBwBwkjnWwbnSOc83155667 = uWeBwBwkjnWwbnSOc80393031;     uWeBwBwkjnWwbnSOc80393031 = uWeBwBwkjnWwbnSOc4264283;     uWeBwBwkjnWwbnSOc4264283 = uWeBwBwkjnWwbnSOc90115937;     uWeBwBwkjnWwbnSOc90115937 = uWeBwBwkjnWwbnSOc9040523;     uWeBwBwkjnWwbnSOc9040523 = uWeBwBwkjnWwbnSOc70933077;     uWeBwBwkjnWwbnSOc70933077 = uWeBwBwkjnWwbnSOc25444696;     uWeBwBwkjnWwbnSOc25444696 = uWeBwBwkjnWwbnSOc74471803;     uWeBwBwkjnWwbnSOc74471803 = uWeBwBwkjnWwbnSOc1941996;     uWeBwBwkjnWwbnSOc1941996 = uWeBwBwkjnWwbnSOc2944123;     uWeBwBwkjnWwbnSOc2944123 = uWeBwBwkjnWwbnSOc65140846;     uWeBwBwkjnWwbnSOc65140846 = uWeBwBwkjnWwbnSOc25311797;     uWeBwBwkjnWwbnSOc25311797 = uWeBwBwkjnWwbnSOc30150416;     uWeBwBwkjnWwbnSOc30150416 = uWeBwBwkjnWwbnSOc89993435;     uWeBwBwkjnWwbnSOc89993435 = uWeBwBwkjnWwbnSOc35796785;     uWeBwBwkjnWwbnSOc35796785 = uWeBwBwkjnWwbnSOc6196289;     uWeBwBwkjnWwbnSOc6196289 = uWeBwBwkjnWwbnSOc67084042;     uWeBwBwkjnWwbnSOc67084042 = uWeBwBwkjnWwbnSOc69602815;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void XUpUYbVkDvdJxnDN78358667() {     double tqjRUhkVYnXpfBaPI75719922 = -488640048;    double tqjRUhkVYnXpfBaPI7427266 = -320174101;    double tqjRUhkVYnXpfBaPI22365072 = -635608074;    double tqjRUhkVYnXpfBaPI75530117 = -354014836;    double tqjRUhkVYnXpfBaPI67733202 = 35078227;    double tqjRUhkVYnXpfBaPI54506935 = 5956282;    double tqjRUhkVYnXpfBaPI16314747 = -473619435;    double tqjRUhkVYnXpfBaPI48010035 = -369080793;    double tqjRUhkVYnXpfBaPI92242746 = -833122071;    double tqjRUhkVYnXpfBaPI71763473 = -663752964;    double tqjRUhkVYnXpfBaPI47741932 = -880842123;    double tqjRUhkVYnXpfBaPI47174973 = -32272570;    double tqjRUhkVYnXpfBaPI41107507 = 56151324;    double tqjRUhkVYnXpfBaPI68234228 = -543817835;    double tqjRUhkVYnXpfBaPI47758382 = -472320895;    double tqjRUhkVYnXpfBaPI79660155 = -566349004;    double tqjRUhkVYnXpfBaPI40949676 = -768425880;    double tqjRUhkVYnXpfBaPI42575239 = -411980380;    double tqjRUhkVYnXpfBaPI67672475 = -47260087;    double tqjRUhkVYnXpfBaPI7097063 = -167366922;    double tqjRUhkVYnXpfBaPI88069805 = -331660819;    double tqjRUhkVYnXpfBaPI41322501 = -468948897;    double tqjRUhkVYnXpfBaPI12729997 = -204885773;    double tqjRUhkVYnXpfBaPI21562182 = 91093054;    double tqjRUhkVYnXpfBaPI75803121 = -95462796;    double tqjRUhkVYnXpfBaPI96557803 = -157960730;    double tqjRUhkVYnXpfBaPI57362099 = -627053001;    double tqjRUhkVYnXpfBaPI35817181 = -95302057;    double tqjRUhkVYnXpfBaPI58177436 = -661034801;    double tqjRUhkVYnXpfBaPI92322037 = -453037111;    double tqjRUhkVYnXpfBaPI10372445 = -766748390;    double tqjRUhkVYnXpfBaPI47505723 = -936992193;    double tqjRUhkVYnXpfBaPI76115009 = -316843586;    double tqjRUhkVYnXpfBaPI29556629 = -29224761;    double tqjRUhkVYnXpfBaPI1720686 = -870545168;    double tqjRUhkVYnXpfBaPI30112498 = -324575622;    double tqjRUhkVYnXpfBaPI51957010 = -729449729;    double tqjRUhkVYnXpfBaPI53386200 = -992086061;    double tqjRUhkVYnXpfBaPI65752685 = -534939258;    double tqjRUhkVYnXpfBaPI17001147 = -759568400;    double tqjRUhkVYnXpfBaPI68935365 = -583712330;    double tqjRUhkVYnXpfBaPI28348857 = -96660083;    double tqjRUhkVYnXpfBaPI17757614 = -916269432;    double tqjRUhkVYnXpfBaPI6007949 = -904301602;    double tqjRUhkVYnXpfBaPI16773049 = -378601971;    double tqjRUhkVYnXpfBaPI17547232 = -478159910;    double tqjRUhkVYnXpfBaPI73353233 = -393456002;    double tqjRUhkVYnXpfBaPI99640080 = -865930922;    double tqjRUhkVYnXpfBaPI76371808 = -487328956;    double tqjRUhkVYnXpfBaPI21801166 = -115231581;    double tqjRUhkVYnXpfBaPI79014048 = -258758333;    double tqjRUhkVYnXpfBaPI24675333 = -64340789;    double tqjRUhkVYnXpfBaPI8027621 = -656533963;    double tqjRUhkVYnXpfBaPI53332419 = -241846965;    double tqjRUhkVYnXpfBaPI90288861 = -386433958;    double tqjRUhkVYnXpfBaPI34397422 = 80308848;    double tqjRUhkVYnXpfBaPI94697269 = -15288328;    double tqjRUhkVYnXpfBaPI802890 = -626701129;    double tqjRUhkVYnXpfBaPI99726995 = -158552041;    double tqjRUhkVYnXpfBaPI71175398 = -806961044;    double tqjRUhkVYnXpfBaPI97144835 = -366990717;    double tqjRUhkVYnXpfBaPI80497566 = -278317378;    double tqjRUhkVYnXpfBaPI89832599 = -708045992;    double tqjRUhkVYnXpfBaPI99920708 = -280084961;    double tqjRUhkVYnXpfBaPI61391028 = -897004575;    double tqjRUhkVYnXpfBaPI236210 = -943849930;    double tqjRUhkVYnXpfBaPI71059964 = -715428984;    double tqjRUhkVYnXpfBaPI11550879 = -914623916;    double tqjRUhkVYnXpfBaPI66513542 = -673272668;    double tqjRUhkVYnXpfBaPI17645884 = -47745274;    double tqjRUhkVYnXpfBaPI27703145 = -836899276;    double tqjRUhkVYnXpfBaPI87563476 = -776339820;    double tqjRUhkVYnXpfBaPI76822554 = -877041123;    double tqjRUhkVYnXpfBaPI50671329 = -287691688;    double tqjRUhkVYnXpfBaPI38161698 = -583654592;    double tqjRUhkVYnXpfBaPI59720949 = -135000736;    double tqjRUhkVYnXpfBaPI23564887 = -552679465;    double tqjRUhkVYnXpfBaPI6722049 = -300584172;    double tqjRUhkVYnXpfBaPI4789134 = -530304975;    double tqjRUhkVYnXpfBaPI58255889 = -617302886;    double tqjRUhkVYnXpfBaPI23204571 = -764504728;    double tqjRUhkVYnXpfBaPI57722018 = -761122080;    double tqjRUhkVYnXpfBaPI59445373 = -607973102;    double tqjRUhkVYnXpfBaPI36376270 = -445803221;    double tqjRUhkVYnXpfBaPI13307990 = -94278778;    double tqjRUhkVYnXpfBaPI85697112 = -602407601;    double tqjRUhkVYnXpfBaPI39478102 = -180458230;    double tqjRUhkVYnXpfBaPI22782591 = 25003378;    double tqjRUhkVYnXpfBaPI39267767 = -642790803;    double tqjRUhkVYnXpfBaPI67323264 = -850854016;    double tqjRUhkVYnXpfBaPI35415229 = -209287294;    double tqjRUhkVYnXpfBaPI51154120 = -2748601;    double tqjRUhkVYnXpfBaPI53659204 = -733534021;    double tqjRUhkVYnXpfBaPI94577286 = -727978215;    double tqjRUhkVYnXpfBaPI19856311 = -292577683;    double tqjRUhkVYnXpfBaPI88437799 = -205394953;    double tqjRUhkVYnXpfBaPI38516258 = -388614091;    double tqjRUhkVYnXpfBaPI17836905 = -536184472;    double tqjRUhkVYnXpfBaPI44616920 = 92702972;    double tqjRUhkVYnXpfBaPI16536839 = -488640048;     tqjRUhkVYnXpfBaPI75719922 = tqjRUhkVYnXpfBaPI7427266;     tqjRUhkVYnXpfBaPI7427266 = tqjRUhkVYnXpfBaPI22365072;     tqjRUhkVYnXpfBaPI22365072 = tqjRUhkVYnXpfBaPI75530117;     tqjRUhkVYnXpfBaPI75530117 = tqjRUhkVYnXpfBaPI67733202;     tqjRUhkVYnXpfBaPI67733202 = tqjRUhkVYnXpfBaPI54506935;     tqjRUhkVYnXpfBaPI54506935 = tqjRUhkVYnXpfBaPI16314747;     tqjRUhkVYnXpfBaPI16314747 = tqjRUhkVYnXpfBaPI48010035;     tqjRUhkVYnXpfBaPI48010035 = tqjRUhkVYnXpfBaPI92242746;     tqjRUhkVYnXpfBaPI92242746 = tqjRUhkVYnXpfBaPI71763473;     tqjRUhkVYnXpfBaPI71763473 = tqjRUhkVYnXpfBaPI47741932;     tqjRUhkVYnXpfBaPI47741932 = tqjRUhkVYnXpfBaPI47174973;     tqjRUhkVYnXpfBaPI47174973 = tqjRUhkVYnXpfBaPI41107507;     tqjRUhkVYnXpfBaPI41107507 = tqjRUhkVYnXpfBaPI68234228;     tqjRUhkVYnXpfBaPI68234228 = tqjRUhkVYnXpfBaPI47758382;     tqjRUhkVYnXpfBaPI47758382 = tqjRUhkVYnXpfBaPI79660155;     tqjRUhkVYnXpfBaPI79660155 = tqjRUhkVYnXpfBaPI40949676;     tqjRUhkVYnXpfBaPI40949676 = tqjRUhkVYnXpfBaPI42575239;     tqjRUhkVYnXpfBaPI42575239 = tqjRUhkVYnXpfBaPI67672475;     tqjRUhkVYnXpfBaPI67672475 = tqjRUhkVYnXpfBaPI7097063;     tqjRUhkVYnXpfBaPI7097063 = tqjRUhkVYnXpfBaPI88069805;     tqjRUhkVYnXpfBaPI88069805 = tqjRUhkVYnXpfBaPI41322501;     tqjRUhkVYnXpfBaPI41322501 = tqjRUhkVYnXpfBaPI12729997;     tqjRUhkVYnXpfBaPI12729997 = tqjRUhkVYnXpfBaPI21562182;     tqjRUhkVYnXpfBaPI21562182 = tqjRUhkVYnXpfBaPI75803121;     tqjRUhkVYnXpfBaPI75803121 = tqjRUhkVYnXpfBaPI96557803;     tqjRUhkVYnXpfBaPI96557803 = tqjRUhkVYnXpfBaPI57362099;     tqjRUhkVYnXpfBaPI57362099 = tqjRUhkVYnXpfBaPI35817181;     tqjRUhkVYnXpfBaPI35817181 = tqjRUhkVYnXpfBaPI58177436;     tqjRUhkVYnXpfBaPI58177436 = tqjRUhkVYnXpfBaPI92322037;     tqjRUhkVYnXpfBaPI92322037 = tqjRUhkVYnXpfBaPI10372445;     tqjRUhkVYnXpfBaPI10372445 = tqjRUhkVYnXpfBaPI47505723;     tqjRUhkVYnXpfBaPI47505723 = tqjRUhkVYnXpfBaPI76115009;     tqjRUhkVYnXpfBaPI76115009 = tqjRUhkVYnXpfBaPI29556629;     tqjRUhkVYnXpfBaPI29556629 = tqjRUhkVYnXpfBaPI1720686;     tqjRUhkVYnXpfBaPI1720686 = tqjRUhkVYnXpfBaPI30112498;     tqjRUhkVYnXpfBaPI30112498 = tqjRUhkVYnXpfBaPI51957010;     tqjRUhkVYnXpfBaPI51957010 = tqjRUhkVYnXpfBaPI53386200;     tqjRUhkVYnXpfBaPI53386200 = tqjRUhkVYnXpfBaPI65752685;     tqjRUhkVYnXpfBaPI65752685 = tqjRUhkVYnXpfBaPI17001147;     tqjRUhkVYnXpfBaPI17001147 = tqjRUhkVYnXpfBaPI68935365;     tqjRUhkVYnXpfBaPI68935365 = tqjRUhkVYnXpfBaPI28348857;     tqjRUhkVYnXpfBaPI28348857 = tqjRUhkVYnXpfBaPI17757614;     tqjRUhkVYnXpfBaPI17757614 = tqjRUhkVYnXpfBaPI6007949;     tqjRUhkVYnXpfBaPI6007949 = tqjRUhkVYnXpfBaPI16773049;     tqjRUhkVYnXpfBaPI16773049 = tqjRUhkVYnXpfBaPI17547232;     tqjRUhkVYnXpfBaPI17547232 = tqjRUhkVYnXpfBaPI73353233;     tqjRUhkVYnXpfBaPI73353233 = tqjRUhkVYnXpfBaPI99640080;     tqjRUhkVYnXpfBaPI99640080 = tqjRUhkVYnXpfBaPI76371808;     tqjRUhkVYnXpfBaPI76371808 = tqjRUhkVYnXpfBaPI21801166;     tqjRUhkVYnXpfBaPI21801166 = tqjRUhkVYnXpfBaPI79014048;     tqjRUhkVYnXpfBaPI79014048 = tqjRUhkVYnXpfBaPI24675333;     tqjRUhkVYnXpfBaPI24675333 = tqjRUhkVYnXpfBaPI8027621;     tqjRUhkVYnXpfBaPI8027621 = tqjRUhkVYnXpfBaPI53332419;     tqjRUhkVYnXpfBaPI53332419 = tqjRUhkVYnXpfBaPI90288861;     tqjRUhkVYnXpfBaPI90288861 = tqjRUhkVYnXpfBaPI34397422;     tqjRUhkVYnXpfBaPI34397422 = tqjRUhkVYnXpfBaPI94697269;     tqjRUhkVYnXpfBaPI94697269 = tqjRUhkVYnXpfBaPI802890;     tqjRUhkVYnXpfBaPI802890 = tqjRUhkVYnXpfBaPI99726995;     tqjRUhkVYnXpfBaPI99726995 = tqjRUhkVYnXpfBaPI71175398;     tqjRUhkVYnXpfBaPI71175398 = tqjRUhkVYnXpfBaPI97144835;     tqjRUhkVYnXpfBaPI97144835 = tqjRUhkVYnXpfBaPI80497566;     tqjRUhkVYnXpfBaPI80497566 = tqjRUhkVYnXpfBaPI89832599;     tqjRUhkVYnXpfBaPI89832599 = tqjRUhkVYnXpfBaPI99920708;     tqjRUhkVYnXpfBaPI99920708 = tqjRUhkVYnXpfBaPI61391028;     tqjRUhkVYnXpfBaPI61391028 = tqjRUhkVYnXpfBaPI236210;     tqjRUhkVYnXpfBaPI236210 = tqjRUhkVYnXpfBaPI71059964;     tqjRUhkVYnXpfBaPI71059964 = tqjRUhkVYnXpfBaPI11550879;     tqjRUhkVYnXpfBaPI11550879 = tqjRUhkVYnXpfBaPI66513542;     tqjRUhkVYnXpfBaPI66513542 = tqjRUhkVYnXpfBaPI17645884;     tqjRUhkVYnXpfBaPI17645884 = tqjRUhkVYnXpfBaPI27703145;     tqjRUhkVYnXpfBaPI27703145 = tqjRUhkVYnXpfBaPI87563476;     tqjRUhkVYnXpfBaPI87563476 = tqjRUhkVYnXpfBaPI76822554;     tqjRUhkVYnXpfBaPI76822554 = tqjRUhkVYnXpfBaPI50671329;     tqjRUhkVYnXpfBaPI50671329 = tqjRUhkVYnXpfBaPI38161698;     tqjRUhkVYnXpfBaPI38161698 = tqjRUhkVYnXpfBaPI59720949;     tqjRUhkVYnXpfBaPI59720949 = tqjRUhkVYnXpfBaPI23564887;     tqjRUhkVYnXpfBaPI23564887 = tqjRUhkVYnXpfBaPI6722049;     tqjRUhkVYnXpfBaPI6722049 = tqjRUhkVYnXpfBaPI4789134;     tqjRUhkVYnXpfBaPI4789134 = tqjRUhkVYnXpfBaPI58255889;     tqjRUhkVYnXpfBaPI58255889 = tqjRUhkVYnXpfBaPI23204571;     tqjRUhkVYnXpfBaPI23204571 = tqjRUhkVYnXpfBaPI57722018;     tqjRUhkVYnXpfBaPI57722018 = tqjRUhkVYnXpfBaPI59445373;     tqjRUhkVYnXpfBaPI59445373 = tqjRUhkVYnXpfBaPI36376270;     tqjRUhkVYnXpfBaPI36376270 = tqjRUhkVYnXpfBaPI13307990;     tqjRUhkVYnXpfBaPI13307990 = tqjRUhkVYnXpfBaPI85697112;     tqjRUhkVYnXpfBaPI85697112 = tqjRUhkVYnXpfBaPI39478102;     tqjRUhkVYnXpfBaPI39478102 = tqjRUhkVYnXpfBaPI22782591;     tqjRUhkVYnXpfBaPI22782591 = tqjRUhkVYnXpfBaPI39267767;     tqjRUhkVYnXpfBaPI39267767 = tqjRUhkVYnXpfBaPI67323264;     tqjRUhkVYnXpfBaPI67323264 = tqjRUhkVYnXpfBaPI35415229;     tqjRUhkVYnXpfBaPI35415229 = tqjRUhkVYnXpfBaPI51154120;     tqjRUhkVYnXpfBaPI51154120 = tqjRUhkVYnXpfBaPI53659204;     tqjRUhkVYnXpfBaPI53659204 = tqjRUhkVYnXpfBaPI94577286;     tqjRUhkVYnXpfBaPI94577286 = tqjRUhkVYnXpfBaPI19856311;     tqjRUhkVYnXpfBaPI19856311 = tqjRUhkVYnXpfBaPI88437799;     tqjRUhkVYnXpfBaPI88437799 = tqjRUhkVYnXpfBaPI38516258;     tqjRUhkVYnXpfBaPI38516258 = tqjRUhkVYnXpfBaPI17836905;     tqjRUhkVYnXpfBaPI17836905 = tqjRUhkVYnXpfBaPI44616920;     tqjRUhkVYnXpfBaPI44616920 = tqjRUhkVYnXpfBaPI16536839;     tqjRUhkVYnXpfBaPI16536839 = tqjRUhkVYnXpfBaPI75719922;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void sMHVsOGnYxYdXdcI45650267() {     double hukWWEYHSRfAopXGS11178891 = -954916948;    double hukWWEYHSRfAopXGS20699344 = -200725708;    double hukWWEYHSRfAopXGS59751548 = -301275149;    double hukWWEYHSRfAopXGS47753472 = -175165908;    double hukWWEYHSRfAopXGS24018472 = -70520618;    double hukWWEYHSRfAopXGS90729806 = -256391613;    double hukWWEYHSRfAopXGS26972154 = -318345169;    double hukWWEYHSRfAopXGS10141596 = -13314933;    double hukWWEYHSRfAopXGS93760351 = -221453478;    double hukWWEYHSRfAopXGS86666185 = -328439962;    double hukWWEYHSRfAopXGS85923667 = -682849852;    double hukWWEYHSRfAopXGS47743122 = -996393341;    double hukWWEYHSRfAopXGS79342152 = -495861181;    double hukWWEYHSRfAopXGS21375091 = -520181244;    double hukWWEYHSRfAopXGS5952899 = -975745910;    double hukWWEYHSRfAopXGS63201456 = -202160611;    double hukWWEYHSRfAopXGS96785612 = -195400433;    double hukWWEYHSRfAopXGS42335430 = -485408449;    double hukWWEYHSRfAopXGS68222805 = -363823508;    double hukWWEYHSRfAopXGS934282 = -593927988;    double hukWWEYHSRfAopXGS1156669 = 11064492;    double hukWWEYHSRfAopXGS44744026 = -555585191;    double hukWWEYHSRfAopXGS83255376 = -942517958;    double hukWWEYHSRfAopXGS15355127 = -419731550;    double hukWWEYHSRfAopXGS83821073 = -119447994;    double hukWWEYHSRfAopXGS7255976 = -946537536;    double hukWWEYHSRfAopXGS17782673 = -433337142;    double hukWWEYHSRfAopXGS76545606 = -265074232;    double hukWWEYHSRfAopXGS4845651 = -135129604;    double hukWWEYHSRfAopXGS97855448 = -487544281;    double hukWWEYHSRfAopXGS28955677 = -218728877;    double hukWWEYHSRfAopXGS85328741 = -709963607;    double hukWWEYHSRfAopXGS88333020 = -464439563;    double hukWWEYHSRfAopXGS59305055 = -93918574;    double hukWWEYHSRfAopXGS67529139 = -821645418;    double hukWWEYHSRfAopXGS81673342 = -539140928;    double hukWWEYHSRfAopXGS1368827 = -594145769;    double hukWWEYHSRfAopXGS4601603 = -974741642;    double hukWWEYHSRfAopXGS96143246 = -185815534;    double hukWWEYHSRfAopXGS81272678 = -224830946;    double hukWWEYHSRfAopXGS2386973 = -352911244;    double hukWWEYHSRfAopXGS17713984 = -791205374;    double hukWWEYHSRfAopXGS7140992 = -469450218;    double hukWWEYHSRfAopXGS46139286 = -408634012;    double hukWWEYHSRfAopXGS16789507 = -205113133;    double hukWWEYHSRfAopXGS63508560 = -517083543;    double hukWWEYHSRfAopXGS92698396 = -877442363;    double hukWWEYHSRfAopXGS17366393 = 67542485;    double hukWWEYHSRfAopXGS99264474 = -151065836;    double hukWWEYHSRfAopXGS46330755 = -606258396;    double hukWWEYHSRfAopXGS59784772 = -133034891;    double hukWWEYHSRfAopXGS8160805 = -277993136;    double hukWWEYHSRfAopXGS76648769 = -412579517;    double hukWWEYHSRfAopXGS69324265 = -433355464;    double hukWWEYHSRfAopXGS44493598 = -732924104;    double hukWWEYHSRfAopXGS66434865 = -299331758;    double hukWWEYHSRfAopXGS37443968 = -258207751;    double hukWWEYHSRfAopXGS44396421 = -881543600;    double hukWWEYHSRfAopXGS63932399 = 44282086;    double hukWWEYHSRfAopXGS16762496 = -123983082;    double hukWWEYHSRfAopXGS72947134 = -823054471;    double hukWWEYHSRfAopXGS50426547 = 46729062;    double hukWWEYHSRfAopXGS5295946 = -878185330;    double hukWWEYHSRfAopXGS95904903 = -733909197;    double hukWWEYHSRfAopXGS57710509 = -9711085;    double hukWWEYHSRfAopXGS594927 = -972886245;    double hukWWEYHSRfAopXGS59410101 = -431953779;    double hukWWEYHSRfAopXGS20037098 = -301942607;    double hukWWEYHSRfAopXGS53845952 = -698535826;    double hukWWEYHSRfAopXGS24279557 = -336604983;    double hukWWEYHSRfAopXGS61832629 = -608014843;    double hukWWEYHSRfAopXGS92184009 = -220658791;    double hukWWEYHSRfAopXGS46192184 = -199592915;    double hukWWEYHSRfAopXGS86950126 = -38992563;    double hukWWEYHSRfAopXGS98547308 = -141016744;    double hukWWEYHSRfAopXGS83442685 = -197730134;    double hukWWEYHSRfAopXGS37603034 = 13865027;    double hukWWEYHSRfAopXGS37116090 = -433883946;    double hukWWEYHSRfAopXGS98565619 = -114618418;    double hukWWEYHSRfAopXGS20312513 = -602364452;    double hukWWEYHSRfAopXGS14557579 = 30904826;    double hukWWEYHSRfAopXGS416281 = -400879627;    double hukWWEYHSRfAopXGS77281132 = -14008396;    double hukWWEYHSRfAopXGS58514895 = -528871208;    double hukWWEYHSRfAopXGS38070677 = -254509391;    double hukWWEYHSRfAopXGS20794872 = -940735742;    double hukWWEYHSRfAopXGS8679973 = -197384091;    double hukWWEYHSRfAopXGS19008755 = 68915901;    double hukWWEYHSRfAopXGS14811457 = -360994471;    double hukWWEYHSRfAopXGS1094275 = -422313661;    double hukWWEYHSRfAopXGS44229374 = -180933177;    double hukWWEYHSRfAopXGS56972405 = -712602170;    double hukWWEYHSRfAopXGS40669204 = -919023729;    double hukWWEYHSRfAopXGS79380750 = 38167548;    double hukWWEYHSRfAopXGS8325545 = -401776475;    double hukWWEYHSRfAopXGS51960425 = -299640307;    double hukWWEYHSRfAopXGS12418038 = -913020044;    double hukWWEYHSRfAopXGS11236089 = -735541021;    double hukWWEYHSRfAopXGS88428777 = -298922927;    double hukWWEYHSRfAopXGS16194581 = -954916948;     hukWWEYHSRfAopXGS11178891 = hukWWEYHSRfAopXGS20699344;     hukWWEYHSRfAopXGS20699344 = hukWWEYHSRfAopXGS59751548;     hukWWEYHSRfAopXGS59751548 = hukWWEYHSRfAopXGS47753472;     hukWWEYHSRfAopXGS47753472 = hukWWEYHSRfAopXGS24018472;     hukWWEYHSRfAopXGS24018472 = hukWWEYHSRfAopXGS90729806;     hukWWEYHSRfAopXGS90729806 = hukWWEYHSRfAopXGS26972154;     hukWWEYHSRfAopXGS26972154 = hukWWEYHSRfAopXGS10141596;     hukWWEYHSRfAopXGS10141596 = hukWWEYHSRfAopXGS93760351;     hukWWEYHSRfAopXGS93760351 = hukWWEYHSRfAopXGS86666185;     hukWWEYHSRfAopXGS86666185 = hukWWEYHSRfAopXGS85923667;     hukWWEYHSRfAopXGS85923667 = hukWWEYHSRfAopXGS47743122;     hukWWEYHSRfAopXGS47743122 = hukWWEYHSRfAopXGS79342152;     hukWWEYHSRfAopXGS79342152 = hukWWEYHSRfAopXGS21375091;     hukWWEYHSRfAopXGS21375091 = hukWWEYHSRfAopXGS5952899;     hukWWEYHSRfAopXGS5952899 = hukWWEYHSRfAopXGS63201456;     hukWWEYHSRfAopXGS63201456 = hukWWEYHSRfAopXGS96785612;     hukWWEYHSRfAopXGS96785612 = hukWWEYHSRfAopXGS42335430;     hukWWEYHSRfAopXGS42335430 = hukWWEYHSRfAopXGS68222805;     hukWWEYHSRfAopXGS68222805 = hukWWEYHSRfAopXGS934282;     hukWWEYHSRfAopXGS934282 = hukWWEYHSRfAopXGS1156669;     hukWWEYHSRfAopXGS1156669 = hukWWEYHSRfAopXGS44744026;     hukWWEYHSRfAopXGS44744026 = hukWWEYHSRfAopXGS83255376;     hukWWEYHSRfAopXGS83255376 = hukWWEYHSRfAopXGS15355127;     hukWWEYHSRfAopXGS15355127 = hukWWEYHSRfAopXGS83821073;     hukWWEYHSRfAopXGS83821073 = hukWWEYHSRfAopXGS7255976;     hukWWEYHSRfAopXGS7255976 = hukWWEYHSRfAopXGS17782673;     hukWWEYHSRfAopXGS17782673 = hukWWEYHSRfAopXGS76545606;     hukWWEYHSRfAopXGS76545606 = hukWWEYHSRfAopXGS4845651;     hukWWEYHSRfAopXGS4845651 = hukWWEYHSRfAopXGS97855448;     hukWWEYHSRfAopXGS97855448 = hukWWEYHSRfAopXGS28955677;     hukWWEYHSRfAopXGS28955677 = hukWWEYHSRfAopXGS85328741;     hukWWEYHSRfAopXGS85328741 = hukWWEYHSRfAopXGS88333020;     hukWWEYHSRfAopXGS88333020 = hukWWEYHSRfAopXGS59305055;     hukWWEYHSRfAopXGS59305055 = hukWWEYHSRfAopXGS67529139;     hukWWEYHSRfAopXGS67529139 = hukWWEYHSRfAopXGS81673342;     hukWWEYHSRfAopXGS81673342 = hukWWEYHSRfAopXGS1368827;     hukWWEYHSRfAopXGS1368827 = hukWWEYHSRfAopXGS4601603;     hukWWEYHSRfAopXGS4601603 = hukWWEYHSRfAopXGS96143246;     hukWWEYHSRfAopXGS96143246 = hukWWEYHSRfAopXGS81272678;     hukWWEYHSRfAopXGS81272678 = hukWWEYHSRfAopXGS2386973;     hukWWEYHSRfAopXGS2386973 = hukWWEYHSRfAopXGS17713984;     hukWWEYHSRfAopXGS17713984 = hukWWEYHSRfAopXGS7140992;     hukWWEYHSRfAopXGS7140992 = hukWWEYHSRfAopXGS46139286;     hukWWEYHSRfAopXGS46139286 = hukWWEYHSRfAopXGS16789507;     hukWWEYHSRfAopXGS16789507 = hukWWEYHSRfAopXGS63508560;     hukWWEYHSRfAopXGS63508560 = hukWWEYHSRfAopXGS92698396;     hukWWEYHSRfAopXGS92698396 = hukWWEYHSRfAopXGS17366393;     hukWWEYHSRfAopXGS17366393 = hukWWEYHSRfAopXGS99264474;     hukWWEYHSRfAopXGS99264474 = hukWWEYHSRfAopXGS46330755;     hukWWEYHSRfAopXGS46330755 = hukWWEYHSRfAopXGS59784772;     hukWWEYHSRfAopXGS59784772 = hukWWEYHSRfAopXGS8160805;     hukWWEYHSRfAopXGS8160805 = hukWWEYHSRfAopXGS76648769;     hukWWEYHSRfAopXGS76648769 = hukWWEYHSRfAopXGS69324265;     hukWWEYHSRfAopXGS69324265 = hukWWEYHSRfAopXGS44493598;     hukWWEYHSRfAopXGS44493598 = hukWWEYHSRfAopXGS66434865;     hukWWEYHSRfAopXGS66434865 = hukWWEYHSRfAopXGS37443968;     hukWWEYHSRfAopXGS37443968 = hukWWEYHSRfAopXGS44396421;     hukWWEYHSRfAopXGS44396421 = hukWWEYHSRfAopXGS63932399;     hukWWEYHSRfAopXGS63932399 = hukWWEYHSRfAopXGS16762496;     hukWWEYHSRfAopXGS16762496 = hukWWEYHSRfAopXGS72947134;     hukWWEYHSRfAopXGS72947134 = hukWWEYHSRfAopXGS50426547;     hukWWEYHSRfAopXGS50426547 = hukWWEYHSRfAopXGS5295946;     hukWWEYHSRfAopXGS5295946 = hukWWEYHSRfAopXGS95904903;     hukWWEYHSRfAopXGS95904903 = hukWWEYHSRfAopXGS57710509;     hukWWEYHSRfAopXGS57710509 = hukWWEYHSRfAopXGS594927;     hukWWEYHSRfAopXGS594927 = hukWWEYHSRfAopXGS59410101;     hukWWEYHSRfAopXGS59410101 = hukWWEYHSRfAopXGS20037098;     hukWWEYHSRfAopXGS20037098 = hukWWEYHSRfAopXGS53845952;     hukWWEYHSRfAopXGS53845952 = hukWWEYHSRfAopXGS24279557;     hukWWEYHSRfAopXGS24279557 = hukWWEYHSRfAopXGS61832629;     hukWWEYHSRfAopXGS61832629 = hukWWEYHSRfAopXGS92184009;     hukWWEYHSRfAopXGS92184009 = hukWWEYHSRfAopXGS46192184;     hukWWEYHSRfAopXGS46192184 = hukWWEYHSRfAopXGS86950126;     hukWWEYHSRfAopXGS86950126 = hukWWEYHSRfAopXGS98547308;     hukWWEYHSRfAopXGS98547308 = hukWWEYHSRfAopXGS83442685;     hukWWEYHSRfAopXGS83442685 = hukWWEYHSRfAopXGS37603034;     hukWWEYHSRfAopXGS37603034 = hukWWEYHSRfAopXGS37116090;     hukWWEYHSRfAopXGS37116090 = hukWWEYHSRfAopXGS98565619;     hukWWEYHSRfAopXGS98565619 = hukWWEYHSRfAopXGS20312513;     hukWWEYHSRfAopXGS20312513 = hukWWEYHSRfAopXGS14557579;     hukWWEYHSRfAopXGS14557579 = hukWWEYHSRfAopXGS416281;     hukWWEYHSRfAopXGS416281 = hukWWEYHSRfAopXGS77281132;     hukWWEYHSRfAopXGS77281132 = hukWWEYHSRfAopXGS58514895;     hukWWEYHSRfAopXGS58514895 = hukWWEYHSRfAopXGS38070677;     hukWWEYHSRfAopXGS38070677 = hukWWEYHSRfAopXGS20794872;     hukWWEYHSRfAopXGS20794872 = hukWWEYHSRfAopXGS8679973;     hukWWEYHSRfAopXGS8679973 = hukWWEYHSRfAopXGS19008755;     hukWWEYHSRfAopXGS19008755 = hukWWEYHSRfAopXGS14811457;     hukWWEYHSRfAopXGS14811457 = hukWWEYHSRfAopXGS1094275;     hukWWEYHSRfAopXGS1094275 = hukWWEYHSRfAopXGS44229374;     hukWWEYHSRfAopXGS44229374 = hukWWEYHSRfAopXGS56972405;     hukWWEYHSRfAopXGS56972405 = hukWWEYHSRfAopXGS40669204;     hukWWEYHSRfAopXGS40669204 = hukWWEYHSRfAopXGS79380750;     hukWWEYHSRfAopXGS79380750 = hukWWEYHSRfAopXGS8325545;     hukWWEYHSRfAopXGS8325545 = hukWWEYHSRfAopXGS51960425;     hukWWEYHSRfAopXGS51960425 = hukWWEYHSRfAopXGS12418038;     hukWWEYHSRfAopXGS12418038 = hukWWEYHSRfAopXGS11236089;     hukWWEYHSRfAopXGS11236089 = hukWWEYHSRfAopXGS88428777;     hukWWEYHSRfAopXGS88428777 = hukWWEYHSRfAopXGS16194581;     hukWWEYHSRfAopXGS16194581 = hukWWEYHSRfAopXGS11178891;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VIiBXnzsSIjVUriq60699335() {     double FCwTzIrwAxmJZlSnK17295998 = 33181163;    double FCwTzIrwAxmJZlSnK64072623 = 89674512;    double FCwTzIrwAxmJZlSnK75793731 = -452108070;    double FCwTzIrwAxmJZlSnK74828253 = -365914538;    double FCwTzIrwAxmJZlSnK92913959 = -591170489;    double FCwTzIrwAxmJZlSnK82147009 = -520887848;    double FCwTzIrwAxmJZlSnK87461119 = -457310146;    double FCwTzIrwAxmJZlSnK59901487 = -300188846;    double FCwTzIrwAxmJZlSnK58402847 = 10513856;    double FCwTzIrwAxmJZlSnK27895845 = -657415440;    double FCwTzIrwAxmJZlSnK46033957 = -560287438;    double FCwTzIrwAxmJZlSnK79566164 = -340799543;    double FCwTzIrwAxmJZlSnK65551271 = -935752269;    double FCwTzIrwAxmJZlSnK4518768 = 47580188;    double FCwTzIrwAxmJZlSnK1695326 = -566208077;    double FCwTzIrwAxmJZlSnK4706850 = -175160155;    double FCwTzIrwAxmJZlSnK90621615 = -811945546;    double FCwTzIrwAxmJZlSnK39388583 = -150931092;    double FCwTzIrwAxmJZlSnK30052647 = -769592605;    double FCwTzIrwAxmJZlSnK81484446 = -950830907;    double FCwTzIrwAxmJZlSnK89709862 = -944422963;    double FCwTzIrwAxmJZlSnK31265049 = -463514907;    double FCwTzIrwAxmJZlSnK18846168 = -689035444;    double FCwTzIrwAxmJZlSnK99347500 = -642036538;    double FCwTzIrwAxmJZlSnK27520723 = -492644027;    double FCwTzIrwAxmJZlSnK31977651 = -825192860;    double FCwTzIrwAxmJZlSnK16242451 = -333999345;    double FCwTzIrwAxmJZlSnK89652731 = -331003186;    double FCwTzIrwAxmJZlSnK70977906 = -357512308;    double FCwTzIrwAxmJZlSnK66471085 = -305015149;    double FCwTzIrwAxmJZlSnK21198226 = -132049815;    double FCwTzIrwAxmJZlSnK82939363 = -419410423;    double FCwTzIrwAxmJZlSnK98043968 = -357052933;    double FCwTzIrwAxmJZlSnK54135619 = -945460339;    double FCwTzIrwAxmJZlSnK29003792 = -360378524;    double FCwTzIrwAxmJZlSnK50399255 = -753167035;    double FCwTzIrwAxmJZlSnK82630761 = -547781639;    double FCwTzIrwAxmJZlSnK38691816 = -746247652;    double FCwTzIrwAxmJZlSnK69753500 = -179739031;    double FCwTzIrwAxmJZlSnK68774616 = -698270168;    double FCwTzIrwAxmJZlSnK8056197 = -418185138;    double FCwTzIrwAxmJZlSnK49864440 = 38652348;    double FCwTzIrwAxmJZlSnK85207970 = -550254802;    double FCwTzIrwAxmJZlSnK33547027 = -427976950;    double FCwTzIrwAxmJZlSnK28741972 = -967011287;    double FCwTzIrwAxmJZlSnK75447755 = -502299659;    double FCwTzIrwAxmJZlSnK10476971 = -996217900;    double FCwTzIrwAxmJZlSnK25323334 = 57646952;    double FCwTzIrwAxmJZlSnK8940204 = -236802824;    double FCwTzIrwAxmJZlSnK59242407 = -693269204;    double FCwTzIrwAxmJZlSnK95485449 = 24671438;    double FCwTzIrwAxmJZlSnK18970525 = -78719456;    double FCwTzIrwAxmJZlSnK24897226 = -204369105;    double FCwTzIrwAxmJZlSnK65293145 = -753825609;    double FCwTzIrwAxmJZlSnK70989472 = -333455886;    double FCwTzIrwAxmJZlSnK86030948 = -503303930;    double FCwTzIrwAxmJZlSnK45226455 = -221290044;    double FCwTzIrwAxmJZlSnK76446231 = -810071533;    double FCwTzIrwAxmJZlSnK47307530 = -873270511;    double FCwTzIrwAxmJZlSnK60936309 = -765977630;    double FCwTzIrwAxmJZlSnK65904558 = -86888503;    double FCwTzIrwAxmJZlSnK97808387 = -26306961;    double FCwTzIrwAxmJZlSnK88923581 = -942676539;    double FCwTzIrwAxmJZlSnK91931761 = -684470996;    double FCwTzIrwAxmJZlSnK6697619 = -425365625;    double FCwTzIrwAxmJZlSnK63094594 = -40877016;    double FCwTzIrwAxmJZlSnK81522195 = -983746610;    double FCwTzIrwAxmJZlSnK11415652 = -990291930;    double FCwTzIrwAxmJZlSnK75514976 = -592041289;    double FCwTzIrwAxmJZlSnK51296070 = -813041042;    double FCwTzIrwAxmJZlSnK22076089 = -627378516;    double FCwTzIrwAxmJZlSnK51929800 = 34302106;    double FCwTzIrwAxmJZlSnK69635083 = -971192061;    double FCwTzIrwAxmJZlSnK61278030 = 28677563;    double FCwTzIrwAxmJZlSnK73428249 = -432645769;    double FCwTzIrwAxmJZlSnK39845422 = -883075312;    double FCwTzIrwAxmJZlSnK46057079 = -913260105;    double FCwTzIrwAxmJZlSnK85299141 = -161058495;    double FCwTzIrwAxmJZlSnK70605528 = -675025251;    double FCwTzIrwAxmJZlSnK52072967 = -990344369;    double FCwTzIrwAxmJZlSnK21500680 = -828974960;    double FCwTzIrwAxmJZlSnK90919117 = -291646298;    double FCwTzIrwAxmJZlSnK80712528 = 5799638;    double FCwTzIrwAxmJZlSnK11735499 = -664243104;    double FCwTzIrwAxmJZlSnK70985635 = -229686587;    double FCwTzIrwAxmJZlSnK2227702 = 46669640;    double FCwTzIrwAxmJZlSnK58042137 = -115041319;    double FCwTzIrwAxmJZlSnK32750823 = -603227324;    double FCwTzIrwAxmJZlSnK83146146 = -512004454;    double FCwTzIrwAxmJZlSnK42972843 = -857074594;    double FCwTzIrwAxmJZlSnK5172800 = -431876991;    double FCwTzIrwAxmJZlSnK6184531 = -737710107;    double FCwTzIrwAxmJZlSnK91384285 = -872977141;    double FCwTzIrwAxmJZlSnK8817191 = -413761401;    double FCwTzIrwAxmJZlSnK2870059 = -511381665;    double FCwTzIrwAxmJZlSnK10247809 = -291878178;    double FCwTzIrwAxmJZlSnK60940859 = -18671114;    double FCwTzIrwAxmJZlSnK93276208 = -865783807;    double FCwTzIrwAxmJZlSnK26849408 = 97388674;    double FCwTzIrwAxmJZlSnK65647377 = 33181163;     FCwTzIrwAxmJZlSnK17295998 = FCwTzIrwAxmJZlSnK64072623;     FCwTzIrwAxmJZlSnK64072623 = FCwTzIrwAxmJZlSnK75793731;     FCwTzIrwAxmJZlSnK75793731 = FCwTzIrwAxmJZlSnK74828253;     FCwTzIrwAxmJZlSnK74828253 = FCwTzIrwAxmJZlSnK92913959;     FCwTzIrwAxmJZlSnK92913959 = FCwTzIrwAxmJZlSnK82147009;     FCwTzIrwAxmJZlSnK82147009 = FCwTzIrwAxmJZlSnK87461119;     FCwTzIrwAxmJZlSnK87461119 = FCwTzIrwAxmJZlSnK59901487;     FCwTzIrwAxmJZlSnK59901487 = FCwTzIrwAxmJZlSnK58402847;     FCwTzIrwAxmJZlSnK58402847 = FCwTzIrwAxmJZlSnK27895845;     FCwTzIrwAxmJZlSnK27895845 = FCwTzIrwAxmJZlSnK46033957;     FCwTzIrwAxmJZlSnK46033957 = FCwTzIrwAxmJZlSnK79566164;     FCwTzIrwAxmJZlSnK79566164 = FCwTzIrwAxmJZlSnK65551271;     FCwTzIrwAxmJZlSnK65551271 = FCwTzIrwAxmJZlSnK4518768;     FCwTzIrwAxmJZlSnK4518768 = FCwTzIrwAxmJZlSnK1695326;     FCwTzIrwAxmJZlSnK1695326 = FCwTzIrwAxmJZlSnK4706850;     FCwTzIrwAxmJZlSnK4706850 = FCwTzIrwAxmJZlSnK90621615;     FCwTzIrwAxmJZlSnK90621615 = FCwTzIrwAxmJZlSnK39388583;     FCwTzIrwAxmJZlSnK39388583 = FCwTzIrwAxmJZlSnK30052647;     FCwTzIrwAxmJZlSnK30052647 = FCwTzIrwAxmJZlSnK81484446;     FCwTzIrwAxmJZlSnK81484446 = FCwTzIrwAxmJZlSnK89709862;     FCwTzIrwAxmJZlSnK89709862 = FCwTzIrwAxmJZlSnK31265049;     FCwTzIrwAxmJZlSnK31265049 = FCwTzIrwAxmJZlSnK18846168;     FCwTzIrwAxmJZlSnK18846168 = FCwTzIrwAxmJZlSnK99347500;     FCwTzIrwAxmJZlSnK99347500 = FCwTzIrwAxmJZlSnK27520723;     FCwTzIrwAxmJZlSnK27520723 = FCwTzIrwAxmJZlSnK31977651;     FCwTzIrwAxmJZlSnK31977651 = FCwTzIrwAxmJZlSnK16242451;     FCwTzIrwAxmJZlSnK16242451 = FCwTzIrwAxmJZlSnK89652731;     FCwTzIrwAxmJZlSnK89652731 = FCwTzIrwAxmJZlSnK70977906;     FCwTzIrwAxmJZlSnK70977906 = FCwTzIrwAxmJZlSnK66471085;     FCwTzIrwAxmJZlSnK66471085 = FCwTzIrwAxmJZlSnK21198226;     FCwTzIrwAxmJZlSnK21198226 = FCwTzIrwAxmJZlSnK82939363;     FCwTzIrwAxmJZlSnK82939363 = FCwTzIrwAxmJZlSnK98043968;     FCwTzIrwAxmJZlSnK98043968 = FCwTzIrwAxmJZlSnK54135619;     FCwTzIrwAxmJZlSnK54135619 = FCwTzIrwAxmJZlSnK29003792;     FCwTzIrwAxmJZlSnK29003792 = FCwTzIrwAxmJZlSnK50399255;     FCwTzIrwAxmJZlSnK50399255 = FCwTzIrwAxmJZlSnK82630761;     FCwTzIrwAxmJZlSnK82630761 = FCwTzIrwAxmJZlSnK38691816;     FCwTzIrwAxmJZlSnK38691816 = FCwTzIrwAxmJZlSnK69753500;     FCwTzIrwAxmJZlSnK69753500 = FCwTzIrwAxmJZlSnK68774616;     FCwTzIrwAxmJZlSnK68774616 = FCwTzIrwAxmJZlSnK8056197;     FCwTzIrwAxmJZlSnK8056197 = FCwTzIrwAxmJZlSnK49864440;     FCwTzIrwAxmJZlSnK49864440 = FCwTzIrwAxmJZlSnK85207970;     FCwTzIrwAxmJZlSnK85207970 = FCwTzIrwAxmJZlSnK33547027;     FCwTzIrwAxmJZlSnK33547027 = FCwTzIrwAxmJZlSnK28741972;     FCwTzIrwAxmJZlSnK28741972 = FCwTzIrwAxmJZlSnK75447755;     FCwTzIrwAxmJZlSnK75447755 = FCwTzIrwAxmJZlSnK10476971;     FCwTzIrwAxmJZlSnK10476971 = FCwTzIrwAxmJZlSnK25323334;     FCwTzIrwAxmJZlSnK25323334 = FCwTzIrwAxmJZlSnK8940204;     FCwTzIrwAxmJZlSnK8940204 = FCwTzIrwAxmJZlSnK59242407;     FCwTzIrwAxmJZlSnK59242407 = FCwTzIrwAxmJZlSnK95485449;     FCwTzIrwAxmJZlSnK95485449 = FCwTzIrwAxmJZlSnK18970525;     FCwTzIrwAxmJZlSnK18970525 = FCwTzIrwAxmJZlSnK24897226;     FCwTzIrwAxmJZlSnK24897226 = FCwTzIrwAxmJZlSnK65293145;     FCwTzIrwAxmJZlSnK65293145 = FCwTzIrwAxmJZlSnK70989472;     FCwTzIrwAxmJZlSnK70989472 = FCwTzIrwAxmJZlSnK86030948;     FCwTzIrwAxmJZlSnK86030948 = FCwTzIrwAxmJZlSnK45226455;     FCwTzIrwAxmJZlSnK45226455 = FCwTzIrwAxmJZlSnK76446231;     FCwTzIrwAxmJZlSnK76446231 = FCwTzIrwAxmJZlSnK47307530;     FCwTzIrwAxmJZlSnK47307530 = FCwTzIrwAxmJZlSnK60936309;     FCwTzIrwAxmJZlSnK60936309 = FCwTzIrwAxmJZlSnK65904558;     FCwTzIrwAxmJZlSnK65904558 = FCwTzIrwAxmJZlSnK97808387;     FCwTzIrwAxmJZlSnK97808387 = FCwTzIrwAxmJZlSnK88923581;     FCwTzIrwAxmJZlSnK88923581 = FCwTzIrwAxmJZlSnK91931761;     FCwTzIrwAxmJZlSnK91931761 = FCwTzIrwAxmJZlSnK6697619;     FCwTzIrwAxmJZlSnK6697619 = FCwTzIrwAxmJZlSnK63094594;     FCwTzIrwAxmJZlSnK63094594 = FCwTzIrwAxmJZlSnK81522195;     FCwTzIrwAxmJZlSnK81522195 = FCwTzIrwAxmJZlSnK11415652;     FCwTzIrwAxmJZlSnK11415652 = FCwTzIrwAxmJZlSnK75514976;     FCwTzIrwAxmJZlSnK75514976 = FCwTzIrwAxmJZlSnK51296070;     FCwTzIrwAxmJZlSnK51296070 = FCwTzIrwAxmJZlSnK22076089;     FCwTzIrwAxmJZlSnK22076089 = FCwTzIrwAxmJZlSnK51929800;     FCwTzIrwAxmJZlSnK51929800 = FCwTzIrwAxmJZlSnK69635083;     FCwTzIrwAxmJZlSnK69635083 = FCwTzIrwAxmJZlSnK61278030;     FCwTzIrwAxmJZlSnK61278030 = FCwTzIrwAxmJZlSnK73428249;     FCwTzIrwAxmJZlSnK73428249 = FCwTzIrwAxmJZlSnK39845422;     FCwTzIrwAxmJZlSnK39845422 = FCwTzIrwAxmJZlSnK46057079;     FCwTzIrwAxmJZlSnK46057079 = FCwTzIrwAxmJZlSnK85299141;     FCwTzIrwAxmJZlSnK85299141 = FCwTzIrwAxmJZlSnK70605528;     FCwTzIrwAxmJZlSnK70605528 = FCwTzIrwAxmJZlSnK52072967;     FCwTzIrwAxmJZlSnK52072967 = FCwTzIrwAxmJZlSnK21500680;     FCwTzIrwAxmJZlSnK21500680 = FCwTzIrwAxmJZlSnK90919117;     FCwTzIrwAxmJZlSnK90919117 = FCwTzIrwAxmJZlSnK80712528;     FCwTzIrwAxmJZlSnK80712528 = FCwTzIrwAxmJZlSnK11735499;     FCwTzIrwAxmJZlSnK11735499 = FCwTzIrwAxmJZlSnK70985635;     FCwTzIrwAxmJZlSnK70985635 = FCwTzIrwAxmJZlSnK2227702;     FCwTzIrwAxmJZlSnK2227702 = FCwTzIrwAxmJZlSnK58042137;     FCwTzIrwAxmJZlSnK58042137 = FCwTzIrwAxmJZlSnK32750823;     FCwTzIrwAxmJZlSnK32750823 = FCwTzIrwAxmJZlSnK83146146;     FCwTzIrwAxmJZlSnK83146146 = FCwTzIrwAxmJZlSnK42972843;     FCwTzIrwAxmJZlSnK42972843 = FCwTzIrwAxmJZlSnK5172800;     FCwTzIrwAxmJZlSnK5172800 = FCwTzIrwAxmJZlSnK6184531;     FCwTzIrwAxmJZlSnK6184531 = FCwTzIrwAxmJZlSnK91384285;     FCwTzIrwAxmJZlSnK91384285 = FCwTzIrwAxmJZlSnK8817191;     FCwTzIrwAxmJZlSnK8817191 = FCwTzIrwAxmJZlSnK2870059;     FCwTzIrwAxmJZlSnK2870059 = FCwTzIrwAxmJZlSnK10247809;     FCwTzIrwAxmJZlSnK10247809 = FCwTzIrwAxmJZlSnK60940859;     FCwTzIrwAxmJZlSnK60940859 = FCwTzIrwAxmJZlSnK93276208;     FCwTzIrwAxmJZlSnK93276208 = FCwTzIrwAxmJZlSnK26849408;     FCwTzIrwAxmJZlSnK26849408 = FCwTzIrwAxmJZlSnK65647377;     FCwTzIrwAxmJZlSnK65647377 = FCwTzIrwAxmJZlSnK17295998;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ipHkHAtrDqXeEkmE27990934() {     double qKwBIRQakFCfJhlph52754966 = -433095736;    double qKwBIRQakFCfJhlph77344701 = -890877095;    double qKwBIRQakFCfJhlph13180209 = -117775145;    double qKwBIRQakFCfJhlph47051608 = -187065609;    double qKwBIRQakFCfJhlph49199229 = -696769335;    double qKwBIRQakFCfJhlph18369881 = -783235743;    double qKwBIRQakFCfJhlph98118525 = -302035880;    double qKwBIRQakFCfJhlph22033048 = 55577013;    double qKwBIRQakFCfJhlph59920452 = -477817551;    double qKwBIRQakFCfJhlph42798557 = -322102438;    double qKwBIRQakFCfJhlph84215692 = -362295168;    double qKwBIRQakFCfJhlph80134312 = -204920315;    double qKwBIRQakFCfJhlph3785917 = -387764774;    double qKwBIRQakFCfJhlph57659631 = 71216779;    double qKwBIRQakFCfJhlph59889842 = 30366908;    double qKwBIRQakFCfJhlph88248150 = -910971762;    double qKwBIRQakFCfJhlph46457551 = -238920098;    double qKwBIRQakFCfJhlph39148775 = -224359160;    double qKwBIRQakFCfJhlph30602977 = 13843974;    double qKwBIRQakFCfJhlph75321664 = -277391973;    double qKwBIRQakFCfJhlph2796725 = -601697652;    double qKwBIRQakFCfJhlph34686574 = -550151201;    double qKwBIRQakFCfJhlph89371547 = -326667629;    double qKwBIRQakFCfJhlph93140445 = -52861142;    double qKwBIRQakFCfJhlph35538674 = -516629226;    double qKwBIRQakFCfJhlph42675822 = -513769666;    double qKwBIRQakFCfJhlph76663025 = -140283486;    double qKwBIRQakFCfJhlph30381157 = -500775360;    double qKwBIRQakFCfJhlph17646121 = -931607111;    double qKwBIRQakFCfJhlph72004496 = -339522319;    double qKwBIRQakFCfJhlph39781458 = -684030302;    double qKwBIRQakFCfJhlph20762382 = -192381837;    double qKwBIRQakFCfJhlph10261980 = -504648911;    double qKwBIRQakFCfJhlph83884045 = 89845847;    double qKwBIRQakFCfJhlph94812245 = -311478774;    double qKwBIRQakFCfJhlph1960100 = -967732341;    double qKwBIRQakFCfJhlph32042578 = -412477679;    double qKwBIRQakFCfJhlph89907219 = -728903233;    double qKwBIRQakFCfJhlph144062 = -930615307;    double qKwBIRQakFCfJhlph33046149 = -163532714;    double qKwBIRQakFCfJhlph41507804 = -187384053;    double qKwBIRQakFCfJhlph39229567 = -655892943;    double qKwBIRQakFCfJhlph74591348 = -103435588;    double qKwBIRQakFCfJhlph73678365 = 67690640;    double qKwBIRQakFCfJhlph28758431 = -793522448;    double qKwBIRQakFCfJhlph21409084 = -541223291;    double qKwBIRQakFCfJhlph29822135 = -380204261;    double qKwBIRQakFCfJhlph43049645 = -108879641;    double qKwBIRQakFCfJhlph31832869 = 99460296;    double qKwBIRQakFCfJhlph83771997 = -84296020;    double qKwBIRQakFCfJhlph76256173 = -949605119;    double qKwBIRQakFCfJhlph2455997 = -292371803;    double qKwBIRQakFCfJhlph93518374 = 39585341;    double qKwBIRQakFCfJhlph81284992 = -945334109;    double qKwBIRQakFCfJhlph25194209 = -679946032;    double qKwBIRQakFCfJhlph18068392 = -882944536;    double qKwBIRQakFCfJhlph87973154 = -464209467;    double qKwBIRQakFCfJhlph20039763 = 35085996;    double qKwBIRQakFCfJhlph11512934 = -670436384;    double qKwBIRQakFCfJhlph6523407 = -82999669;    double qKwBIRQakFCfJhlph41706856 = -542952258;    double qKwBIRQakFCfJhlph67737369 = -801260521;    double qKwBIRQakFCfJhlph4386928 = -12815877;    double qKwBIRQakFCfJhlph87915956 = -38295232;    double qKwBIRQakFCfJhlph3017100 = -638072136;    double qKwBIRQakFCfJhlph63453311 = -69913331;    double qKwBIRQakFCfJhlph69872333 = -700271404;    double qKwBIRQakFCfJhlph19901871 = -377610622;    double qKwBIRQakFCfJhlph62847386 = -617304447;    double qKwBIRQakFCfJhlph57929743 = -1900751;    double qKwBIRQakFCfJhlph56205573 = -398494083;    double qKwBIRQakFCfJhlph56550332 = -510016866;    double qKwBIRQakFCfJhlph39004713 = -293743853;    double qKwBIRQakFCfJhlph97556828 = -822623313;    double qKwBIRQakFCfJhlph33813861 = 9992079;    double qKwBIRQakFCfJhlph63567158 = -945804709;    double qKwBIRQakFCfJhlph60095225 = -346715613;    double qKwBIRQakFCfJhlph15693183 = -294358269;    double qKwBIRQakFCfJhlph64382015 = -259338694;    double qKwBIRQakFCfJhlph14129590 = -975405935;    double qKwBIRQakFCfJhlph12853688 = -33565406;    double qKwBIRQakFCfJhlph33613380 = 68596155;    double qKwBIRQakFCfJhlph98548287 = -500235656;    double qKwBIRQakFCfJhlph33874124 = -747311091;    double qKwBIRQakFCfJhlph95748322 = -389917201;    double qKwBIRQakFCfJhlph37325461 = -291658500;    double qKwBIRQakFCfJhlph27244007 = -131967179;    double qKwBIRQakFCfJhlph28976988 = -559314802;    double qKwBIRQakFCfJhlph58689837 = -230208122;    double qKwBIRQakFCfJhlph76743853 = -428534239;    double qKwBIRQakFCfJhlph13986945 = -403522875;    double qKwBIRQakFCfJhlph12002815 = -347563676;    double qKwBIRQakFCfJhlph78394285 = 41533151;    double qKwBIRQakFCfJhlph93620655 = -747615639;    double qKwBIRQakFCfJhlph91339292 = -620580457;    double qKwBIRQakFCfJhlph73770435 = -386123532;    double qKwBIRQakFCfJhlph34842640 = -543077067;    double qKwBIRQakFCfJhlph86675392 = 34859644;    double qKwBIRQakFCfJhlph70661265 = -294237225;    double qKwBIRQakFCfJhlph65305120 = -433095736;     qKwBIRQakFCfJhlph52754966 = qKwBIRQakFCfJhlph77344701;     qKwBIRQakFCfJhlph77344701 = qKwBIRQakFCfJhlph13180209;     qKwBIRQakFCfJhlph13180209 = qKwBIRQakFCfJhlph47051608;     qKwBIRQakFCfJhlph47051608 = qKwBIRQakFCfJhlph49199229;     qKwBIRQakFCfJhlph49199229 = qKwBIRQakFCfJhlph18369881;     qKwBIRQakFCfJhlph18369881 = qKwBIRQakFCfJhlph98118525;     qKwBIRQakFCfJhlph98118525 = qKwBIRQakFCfJhlph22033048;     qKwBIRQakFCfJhlph22033048 = qKwBIRQakFCfJhlph59920452;     qKwBIRQakFCfJhlph59920452 = qKwBIRQakFCfJhlph42798557;     qKwBIRQakFCfJhlph42798557 = qKwBIRQakFCfJhlph84215692;     qKwBIRQakFCfJhlph84215692 = qKwBIRQakFCfJhlph80134312;     qKwBIRQakFCfJhlph80134312 = qKwBIRQakFCfJhlph3785917;     qKwBIRQakFCfJhlph3785917 = qKwBIRQakFCfJhlph57659631;     qKwBIRQakFCfJhlph57659631 = qKwBIRQakFCfJhlph59889842;     qKwBIRQakFCfJhlph59889842 = qKwBIRQakFCfJhlph88248150;     qKwBIRQakFCfJhlph88248150 = qKwBIRQakFCfJhlph46457551;     qKwBIRQakFCfJhlph46457551 = qKwBIRQakFCfJhlph39148775;     qKwBIRQakFCfJhlph39148775 = qKwBIRQakFCfJhlph30602977;     qKwBIRQakFCfJhlph30602977 = qKwBIRQakFCfJhlph75321664;     qKwBIRQakFCfJhlph75321664 = qKwBIRQakFCfJhlph2796725;     qKwBIRQakFCfJhlph2796725 = qKwBIRQakFCfJhlph34686574;     qKwBIRQakFCfJhlph34686574 = qKwBIRQakFCfJhlph89371547;     qKwBIRQakFCfJhlph89371547 = qKwBIRQakFCfJhlph93140445;     qKwBIRQakFCfJhlph93140445 = qKwBIRQakFCfJhlph35538674;     qKwBIRQakFCfJhlph35538674 = qKwBIRQakFCfJhlph42675822;     qKwBIRQakFCfJhlph42675822 = qKwBIRQakFCfJhlph76663025;     qKwBIRQakFCfJhlph76663025 = qKwBIRQakFCfJhlph30381157;     qKwBIRQakFCfJhlph30381157 = qKwBIRQakFCfJhlph17646121;     qKwBIRQakFCfJhlph17646121 = qKwBIRQakFCfJhlph72004496;     qKwBIRQakFCfJhlph72004496 = qKwBIRQakFCfJhlph39781458;     qKwBIRQakFCfJhlph39781458 = qKwBIRQakFCfJhlph20762382;     qKwBIRQakFCfJhlph20762382 = qKwBIRQakFCfJhlph10261980;     qKwBIRQakFCfJhlph10261980 = qKwBIRQakFCfJhlph83884045;     qKwBIRQakFCfJhlph83884045 = qKwBIRQakFCfJhlph94812245;     qKwBIRQakFCfJhlph94812245 = qKwBIRQakFCfJhlph1960100;     qKwBIRQakFCfJhlph1960100 = qKwBIRQakFCfJhlph32042578;     qKwBIRQakFCfJhlph32042578 = qKwBIRQakFCfJhlph89907219;     qKwBIRQakFCfJhlph89907219 = qKwBIRQakFCfJhlph144062;     qKwBIRQakFCfJhlph144062 = qKwBIRQakFCfJhlph33046149;     qKwBIRQakFCfJhlph33046149 = qKwBIRQakFCfJhlph41507804;     qKwBIRQakFCfJhlph41507804 = qKwBIRQakFCfJhlph39229567;     qKwBIRQakFCfJhlph39229567 = qKwBIRQakFCfJhlph74591348;     qKwBIRQakFCfJhlph74591348 = qKwBIRQakFCfJhlph73678365;     qKwBIRQakFCfJhlph73678365 = qKwBIRQakFCfJhlph28758431;     qKwBIRQakFCfJhlph28758431 = qKwBIRQakFCfJhlph21409084;     qKwBIRQakFCfJhlph21409084 = qKwBIRQakFCfJhlph29822135;     qKwBIRQakFCfJhlph29822135 = qKwBIRQakFCfJhlph43049645;     qKwBIRQakFCfJhlph43049645 = qKwBIRQakFCfJhlph31832869;     qKwBIRQakFCfJhlph31832869 = qKwBIRQakFCfJhlph83771997;     qKwBIRQakFCfJhlph83771997 = qKwBIRQakFCfJhlph76256173;     qKwBIRQakFCfJhlph76256173 = qKwBIRQakFCfJhlph2455997;     qKwBIRQakFCfJhlph2455997 = qKwBIRQakFCfJhlph93518374;     qKwBIRQakFCfJhlph93518374 = qKwBIRQakFCfJhlph81284992;     qKwBIRQakFCfJhlph81284992 = qKwBIRQakFCfJhlph25194209;     qKwBIRQakFCfJhlph25194209 = qKwBIRQakFCfJhlph18068392;     qKwBIRQakFCfJhlph18068392 = qKwBIRQakFCfJhlph87973154;     qKwBIRQakFCfJhlph87973154 = qKwBIRQakFCfJhlph20039763;     qKwBIRQakFCfJhlph20039763 = qKwBIRQakFCfJhlph11512934;     qKwBIRQakFCfJhlph11512934 = qKwBIRQakFCfJhlph6523407;     qKwBIRQakFCfJhlph6523407 = qKwBIRQakFCfJhlph41706856;     qKwBIRQakFCfJhlph41706856 = qKwBIRQakFCfJhlph67737369;     qKwBIRQakFCfJhlph67737369 = qKwBIRQakFCfJhlph4386928;     qKwBIRQakFCfJhlph4386928 = qKwBIRQakFCfJhlph87915956;     qKwBIRQakFCfJhlph87915956 = qKwBIRQakFCfJhlph3017100;     qKwBIRQakFCfJhlph3017100 = qKwBIRQakFCfJhlph63453311;     qKwBIRQakFCfJhlph63453311 = qKwBIRQakFCfJhlph69872333;     qKwBIRQakFCfJhlph69872333 = qKwBIRQakFCfJhlph19901871;     qKwBIRQakFCfJhlph19901871 = qKwBIRQakFCfJhlph62847386;     qKwBIRQakFCfJhlph62847386 = qKwBIRQakFCfJhlph57929743;     qKwBIRQakFCfJhlph57929743 = qKwBIRQakFCfJhlph56205573;     qKwBIRQakFCfJhlph56205573 = qKwBIRQakFCfJhlph56550332;     qKwBIRQakFCfJhlph56550332 = qKwBIRQakFCfJhlph39004713;     qKwBIRQakFCfJhlph39004713 = qKwBIRQakFCfJhlph97556828;     qKwBIRQakFCfJhlph97556828 = qKwBIRQakFCfJhlph33813861;     qKwBIRQakFCfJhlph33813861 = qKwBIRQakFCfJhlph63567158;     qKwBIRQakFCfJhlph63567158 = qKwBIRQakFCfJhlph60095225;     qKwBIRQakFCfJhlph60095225 = qKwBIRQakFCfJhlph15693183;     qKwBIRQakFCfJhlph15693183 = qKwBIRQakFCfJhlph64382015;     qKwBIRQakFCfJhlph64382015 = qKwBIRQakFCfJhlph14129590;     qKwBIRQakFCfJhlph14129590 = qKwBIRQakFCfJhlph12853688;     qKwBIRQakFCfJhlph12853688 = qKwBIRQakFCfJhlph33613380;     qKwBIRQakFCfJhlph33613380 = qKwBIRQakFCfJhlph98548287;     qKwBIRQakFCfJhlph98548287 = qKwBIRQakFCfJhlph33874124;     qKwBIRQakFCfJhlph33874124 = qKwBIRQakFCfJhlph95748322;     qKwBIRQakFCfJhlph95748322 = qKwBIRQakFCfJhlph37325461;     qKwBIRQakFCfJhlph37325461 = qKwBIRQakFCfJhlph27244007;     qKwBIRQakFCfJhlph27244007 = qKwBIRQakFCfJhlph28976988;     qKwBIRQakFCfJhlph28976988 = qKwBIRQakFCfJhlph58689837;     qKwBIRQakFCfJhlph58689837 = qKwBIRQakFCfJhlph76743853;     qKwBIRQakFCfJhlph76743853 = qKwBIRQakFCfJhlph13986945;     qKwBIRQakFCfJhlph13986945 = qKwBIRQakFCfJhlph12002815;     qKwBIRQakFCfJhlph12002815 = qKwBIRQakFCfJhlph78394285;     qKwBIRQakFCfJhlph78394285 = qKwBIRQakFCfJhlph93620655;     qKwBIRQakFCfJhlph93620655 = qKwBIRQakFCfJhlph91339292;     qKwBIRQakFCfJhlph91339292 = qKwBIRQakFCfJhlph73770435;     qKwBIRQakFCfJhlph73770435 = qKwBIRQakFCfJhlph34842640;     qKwBIRQakFCfJhlph34842640 = qKwBIRQakFCfJhlph86675392;     qKwBIRQakFCfJhlph86675392 = qKwBIRQakFCfJhlph70661265;     qKwBIRQakFCfJhlph70661265 = qKwBIRQakFCfJhlph65305120;     qKwBIRQakFCfJhlph65305120 = qKwBIRQakFCfJhlph52754966;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hIIoaVHJtnEXrbEW43040002() {     double wzxDDyDdwhafVPAJO58872073 = -544997625;    double wzxDDyDdwhafVPAJO20717981 = -600476876;    double wzxDDyDdwhafVPAJO29222392 = -268608066;    double wzxDDyDdwhafVPAJO74126388 = -377814239;    double wzxDDyDdwhafVPAJO18094718 = -117419206;    double wzxDDyDdwhafVPAJO9787084 = 52268022;    double wzxDDyDdwhafVPAJO58607492 = -441000858;    double wzxDDyDdwhafVPAJO71792938 = -231296900;    double wzxDDyDdwhafVPAJO24562948 = -245850217;    double wzxDDyDdwhafVPAJO84028216 = -651077915;    double wzxDDyDdwhafVPAJO44325983 = -239732754;    double wzxDDyDdwhafVPAJO11957355 = -649326516;    double wzxDDyDdwhafVPAJO89995034 = -827655863;    double wzxDDyDdwhafVPAJO40803308 = -461021789;    double wzxDDyDdwhafVPAJO55632268 = -660095258;    double wzxDDyDdwhafVPAJO29753545 = -883971305;    double wzxDDyDdwhafVPAJO40293554 = -855465211;    double wzxDDyDdwhafVPAJO36201928 = -989881803;    double wzxDDyDdwhafVPAJO92432818 = -391925123;    double wzxDDyDdwhafVPAJO55871829 = -634294892;    double wzxDDyDdwhafVPAJO91349918 = -457185107;    double wzxDDyDdwhafVPAJO21207598 = -458080917;    double wzxDDyDdwhafVPAJO24962340 = -73185115;    double wzxDDyDdwhafVPAJO77132819 = -275166130;    double wzxDDyDdwhafVPAJO79238324 = -889825259;    double wzxDDyDdwhafVPAJO67397497 = -392424990;    double wzxDDyDdwhafVPAJO75122803 = -40945689;    double wzxDDyDdwhafVPAJO43488282 = -566704314;    double wzxDDyDdwhafVPAJO83778376 = -53989815;    double wzxDDyDdwhafVPAJO40620133 = -156993187;    double wzxDDyDdwhafVPAJO32024007 = -597351240;    double wzxDDyDdwhafVPAJO18373003 = 98171347;    double wzxDDyDdwhafVPAJO19972928 = -397262281;    double wzxDDyDdwhafVPAJO78714610 = -761695918;    double wzxDDyDdwhafVPAJO56286897 = -950211880;    double wzxDDyDdwhafVPAJO70686012 = -81758449;    double wzxDDyDdwhafVPAJO13304513 = -366113549;    double wzxDDyDdwhafVPAJO23997432 = -500409242;    double wzxDDyDdwhafVPAJO73754315 = -924538804;    double wzxDDyDdwhafVPAJO20548087 = -636971937;    double wzxDDyDdwhafVPAJO47177028 = -252657947;    double wzxDDyDdwhafVPAJO71380024 = -926035221;    double wzxDDyDdwhafVPAJO52658327 = -184240172;    double wzxDDyDdwhafVPAJO61086105 = 48347702;    double wzxDDyDdwhafVPAJO40710896 = -455420603;    double wzxDDyDdwhafVPAJO33348279 = -526439408;    double wzxDDyDdwhafVPAJO47600709 = -498979798;    double wzxDDyDdwhafVPAJO51006586 = -118775173;    double wzxDDyDdwhafVPAJO41508598 = 13723308;    double wzxDDyDdwhafVPAJO96683649 = -171306828;    double wzxDDyDdwhafVPAJO11956852 = -791898791;    double wzxDDyDdwhafVPAJO13265717 = -93098123;    double wzxDDyDdwhafVPAJO41766832 = -852204246;    double wzxDDyDdwhafVPAJO77253872 = -165804254;    double wzxDDyDdwhafVPAJO51690084 = -280477814;    double wzxDDyDdwhafVPAJO37664475 = 13083292;    double wzxDDyDdwhafVPAJO95755641 = -427291761;    double wzxDDyDdwhafVPAJO52089573 = -993441937;    double wzxDDyDdwhafVPAJO94888064 = -487988981;    double wzxDDyDdwhafVPAJO50697220 = -724994217;    double wzxDDyDdwhafVPAJO34664280 = -906786290;    double wzxDDyDdwhafVPAJO15119210 = -874296545;    double wzxDDyDdwhafVPAJO88014562 = -77307085;    double wzxDDyDdwhafVPAJO83942814 = 11142970;    double wzxDDyDdwhafVPAJO52004209 = 46273325;    double wzxDDyDdwhafVPAJO25952980 = -237904102;    double wzxDDyDdwhafVPAJO91984426 = -152064236;    double wzxDDyDdwhafVPAJO11280425 = 34040055;    double wzxDDyDdwhafVPAJO84516410 = -510809910;    double wzxDDyDdwhafVPAJO84946256 = -478336810;    double wzxDDyDdwhafVPAJO16449032 = -417857757;    double wzxDDyDdwhafVPAJO16296123 = -255055969;    double wzxDDyDdwhafVPAJO62447612 = 34657001;    double wzxDDyDdwhafVPAJO71884732 = -754953187;    double wzxDDyDdwhafVPAJO8694802 = -281636946;    double wzxDDyDdwhafVPAJO19969894 = -531149887;    double wzxDDyDdwhafVPAJO68549271 = -173840746;    double wzxDDyDdwhafVPAJO63876234 = -21532818;    double wzxDDyDdwhafVPAJO36421924 = -819745527;    double wzxDDyDdwhafVPAJO45890045 = -263385851;    double wzxDDyDdwhafVPAJO19796789 = -893445192;    double wzxDDyDdwhafVPAJO24116217 = -922170516;    double wzxDDyDdwhafVPAJO1979684 = -480427622;    double wzxDDyDdwhafVPAJO87094726 = -882682987;    double wzxDDyDdwhafVPAJO28663281 = -365094397;    double wzxDDyDdwhafVPAJO18758291 = -404253118;    double wzxDDyDdwhafVPAJO76606171 = -49624407;    double wzxDDyDdwhafVPAJO42719056 = -131458027;    double wzxDDyDdwhafVPAJO27024526 = -381218104;    double wzxDDyDdwhafVPAJO18622422 = -863295172;    double wzxDDyDdwhafVPAJO74930370 = -654466688;    double wzxDDyDdwhafVPAJO61214940 = -372671612;    double wzxDDyDdwhafVPAJO29109367 = 87579739;    double wzxDDyDdwhafVPAJO23057096 = -99544588;    double wzxDDyDdwhafVPAJO85883806 = -730185647;    double wzxDDyDdwhafVPAJO32057818 = -378361403;    double wzxDDyDdwhafVPAJO83365461 = -748728136;    double wzxDDyDdwhafVPAJO68715512 = -95383142;    double wzxDDyDdwhafVPAJO9081896 = -997925623;    double wzxDDyDdwhafVPAJO14757917 = -544997625;     wzxDDyDdwhafVPAJO58872073 = wzxDDyDdwhafVPAJO20717981;     wzxDDyDdwhafVPAJO20717981 = wzxDDyDdwhafVPAJO29222392;     wzxDDyDdwhafVPAJO29222392 = wzxDDyDdwhafVPAJO74126388;     wzxDDyDdwhafVPAJO74126388 = wzxDDyDdwhafVPAJO18094718;     wzxDDyDdwhafVPAJO18094718 = wzxDDyDdwhafVPAJO9787084;     wzxDDyDdwhafVPAJO9787084 = wzxDDyDdwhafVPAJO58607492;     wzxDDyDdwhafVPAJO58607492 = wzxDDyDdwhafVPAJO71792938;     wzxDDyDdwhafVPAJO71792938 = wzxDDyDdwhafVPAJO24562948;     wzxDDyDdwhafVPAJO24562948 = wzxDDyDdwhafVPAJO84028216;     wzxDDyDdwhafVPAJO84028216 = wzxDDyDdwhafVPAJO44325983;     wzxDDyDdwhafVPAJO44325983 = wzxDDyDdwhafVPAJO11957355;     wzxDDyDdwhafVPAJO11957355 = wzxDDyDdwhafVPAJO89995034;     wzxDDyDdwhafVPAJO89995034 = wzxDDyDdwhafVPAJO40803308;     wzxDDyDdwhafVPAJO40803308 = wzxDDyDdwhafVPAJO55632268;     wzxDDyDdwhafVPAJO55632268 = wzxDDyDdwhafVPAJO29753545;     wzxDDyDdwhafVPAJO29753545 = wzxDDyDdwhafVPAJO40293554;     wzxDDyDdwhafVPAJO40293554 = wzxDDyDdwhafVPAJO36201928;     wzxDDyDdwhafVPAJO36201928 = wzxDDyDdwhafVPAJO92432818;     wzxDDyDdwhafVPAJO92432818 = wzxDDyDdwhafVPAJO55871829;     wzxDDyDdwhafVPAJO55871829 = wzxDDyDdwhafVPAJO91349918;     wzxDDyDdwhafVPAJO91349918 = wzxDDyDdwhafVPAJO21207598;     wzxDDyDdwhafVPAJO21207598 = wzxDDyDdwhafVPAJO24962340;     wzxDDyDdwhafVPAJO24962340 = wzxDDyDdwhafVPAJO77132819;     wzxDDyDdwhafVPAJO77132819 = wzxDDyDdwhafVPAJO79238324;     wzxDDyDdwhafVPAJO79238324 = wzxDDyDdwhafVPAJO67397497;     wzxDDyDdwhafVPAJO67397497 = wzxDDyDdwhafVPAJO75122803;     wzxDDyDdwhafVPAJO75122803 = wzxDDyDdwhafVPAJO43488282;     wzxDDyDdwhafVPAJO43488282 = wzxDDyDdwhafVPAJO83778376;     wzxDDyDdwhafVPAJO83778376 = wzxDDyDdwhafVPAJO40620133;     wzxDDyDdwhafVPAJO40620133 = wzxDDyDdwhafVPAJO32024007;     wzxDDyDdwhafVPAJO32024007 = wzxDDyDdwhafVPAJO18373003;     wzxDDyDdwhafVPAJO18373003 = wzxDDyDdwhafVPAJO19972928;     wzxDDyDdwhafVPAJO19972928 = wzxDDyDdwhafVPAJO78714610;     wzxDDyDdwhafVPAJO78714610 = wzxDDyDdwhafVPAJO56286897;     wzxDDyDdwhafVPAJO56286897 = wzxDDyDdwhafVPAJO70686012;     wzxDDyDdwhafVPAJO70686012 = wzxDDyDdwhafVPAJO13304513;     wzxDDyDdwhafVPAJO13304513 = wzxDDyDdwhafVPAJO23997432;     wzxDDyDdwhafVPAJO23997432 = wzxDDyDdwhafVPAJO73754315;     wzxDDyDdwhafVPAJO73754315 = wzxDDyDdwhafVPAJO20548087;     wzxDDyDdwhafVPAJO20548087 = wzxDDyDdwhafVPAJO47177028;     wzxDDyDdwhafVPAJO47177028 = wzxDDyDdwhafVPAJO71380024;     wzxDDyDdwhafVPAJO71380024 = wzxDDyDdwhafVPAJO52658327;     wzxDDyDdwhafVPAJO52658327 = wzxDDyDdwhafVPAJO61086105;     wzxDDyDdwhafVPAJO61086105 = wzxDDyDdwhafVPAJO40710896;     wzxDDyDdwhafVPAJO40710896 = wzxDDyDdwhafVPAJO33348279;     wzxDDyDdwhafVPAJO33348279 = wzxDDyDdwhafVPAJO47600709;     wzxDDyDdwhafVPAJO47600709 = wzxDDyDdwhafVPAJO51006586;     wzxDDyDdwhafVPAJO51006586 = wzxDDyDdwhafVPAJO41508598;     wzxDDyDdwhafVPAJO41508598 = wzxDDyDdwhafVPAJO96683649;     wzxDDyDdwhafVPAJO96683649 = wzxDDyDdwhafVPAJO11956852;     wzxDDyDdwhafVPAJO11956852 = wzxDDyDdwhafVPAJO13265717;     wzxDDyDdwhafVPAJO13265717 = wzxDDyDdwhafVPAJO41766832;     wzxDDyDdwhafVPAJO41766832 = wzxDDyDdwhafVPAJO77253872;     wzxDDyDdwhafVPAJO77253872 = wzxDDyDdwhafVPAJO51690084;     wzxDDyDdwhafVPAJO51690084 = wzxDDyDdwhafVPAJO37664475;     wzxDDyDdwhafVPAJO37664475 = wzxDDyDdwhafVPAJO95755641;     wzxDDyDdwhafVPAJO95755641 = wzxDDyDdwhafVPAJO52089573;     wzxDDyDdwhafVPAJO52089573 = wzxDDyDdwhafVPAJO94888064;     wzxDDyDdwhafVPAJO94888064 = wzxDDyDdwhafVPAJO50697220;     wzxDDyDdwhafVPAJO50697220 = wzxDDyDdwhafVPAJO34664280;     wzxDDyDdwhafVPAJO34664280 = wzxDDyDdwhafVPAJO15119210;     wzxDDyDdwhafVPAJO15119210 = wzxDDyDdwhafVPAJO88014562;     wzxDDyDdwhafVPAJO88014562 = wzxDDyDdwhafVPAJO83942814;     wzxDDyDdwhafVPAJO83942814 = wzxDDyDdwhafVPAJO52004209;     wzxDDyDdwhafVPAJO52004209 = wzxDDyDdwhafVPAJO25952980;     wzxDDyDdwhafVPAJO25952980 = wzxDDyDdwhafVPAJO91984426;     wzxDDyDdwhafVPAJO91984426 = wzxDDyDdwhafVPAJO11280425;     wzxDDyDdwhafVPAJO11280425 = wzxDDyDdwhafVPAJO84516410;     wzxDDyDdwhafVPAJO84516410 = wzxDDyDdwhafVPAJO84946256;     wzxDDyDdwhafVPAJO84946256 = wzxDDyDdwhafVPAJO16449032;     wzxDDyDdwhafVPAJO16449032 = wzxDDyDdwhafVPAJO16296123;     wzxDDyDdwhafVPAJO16296123 = wzxDDyDdwhafVPAJO62447612;     wzxDDyDdwhafVPAJO62447612 = wzxDDyDdwhafVPAJO71884732;     wzxDDyDdwhafVPAJO71884732 = wzxDDyDdwhafVPAJO8694802;     wzxDDyDdwhafVPAJO8694802 = wzxDDyDdwhafVPAJO19969894;     wzxDDyDdwhafVPAJO19969894 = wzxDDyDdwhafVPAJO68549271;     wzxDDyDdwhafVPAJO68549271 = wzxDDyDdwhafVPAJO63876234;     wzxDDyDdwhafVPAJO63876234 = wzxDDyDdwhafVPAJO36421924;     wzxDDyDdwhafVPAJO36421924 = wzxDDyDdwhafVPAJO45890045;     wzxDDyDdwhafVPAJO45890045 = wzxDDyDdwhafVPAJO19796789;     wzxDDyDdwhafVPAJO19796789 = wzxDDyDdwhafVPAJO24116217;     wzxDDyDdwhafVPAJO24116217 = wzxDDyDdwhafVPAJO1979684;     wzxDDyDdwhafVPAJO1979684 = wzxDDyDdwhafVPAJO87094726;     wzxDDyDdwhafVPAJO87094726 = wzxDDyDdwhafVPAJO28663281;     wzxDDyDdwhafVPAJO28663281 = wzxDDyDdwhafVPAJO18758291;     wzxDDyDdwhafVPAJO18758291 = wzxDDyDdwhafVPAJO76606171;     wzxDDyDdwhafVPAJO76606171 = wzxDDyDdwhafVPAJO42719056;     wzxDDyDdwhafVPAJO42719056 = wzxDDyDdwhafVPAJO27024526;     wzxDDyDdwhafVPAJO27024526 = wzxDDyDdwhafVPAJO18622422;     wzxDDyDdwhafVPAJO18622422 = wzxDDyDdwhafVPAJO74930370;     wzxDDyDdwhafVPAJO74930370 = wzxDDyDdwhafVPAJO61214940;     wzxDDyDdwhafVPAJO61214940 = wzxDDyDdwhafVPAJO29109367;     wzxDDyDdwhafVPAJO29109367 = wzxDDyDdwhafVPAJO23057096;     wzxDDyDdwhafVPAJO23057096 = wzxDDyDdwhafVPAJO85883806;     wzxDDyDdwhafVPAJO85883806 = wzxDDyDdwhafVPAJO32057818;     wzxDDyDdwhafVPAJO32057818 = wzxDDyDdwhafVPAJO83365461;     wzxDDyDdwhafVPAJO83365461 = wzxDDyDdwhafVPAJO68715512;     wzxDDyDdwhafVPAJO68715512 = wzxDDyDdwhafVPAJO9081896;     wzxDDyDdwhafVPAJO9081896 = wzxDDyDdwhafVPAJO14757917;     wzxDDyDdwhafVPAJO14757917 = wzxDDyDdwhafVPAJO58872073;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void TjDqJHXDmSrdAWJl10331602() {     double wuxcBhGXOkpupPGUe94331041 = 88725475;    double wuxcBhGXOkpupPGUe33990060 = -481028482;    double wuxcBhGXOkpupPGUe66608869 = 65724859;    double wuxcBhGXOkpupPGUe46349743 = -198965311;    double wuxcBhGXOkpupPGUe74379987 = -223018051;    double wuxcBhGXOkpupPGUe46009955 = -210079874;    double wuxcBhGXOkpupPGUe69264898 = -285726592;    double wuxcBhGXOkpupPGUe33924500 = -975531040;    double wuxcBhGXOkpupPGUe26080553 = -734181624;    double wuxcBhGXOkpupPGUe98930928 = -315764913;    double wuxcBhGXOkpupPGUe82507717 = -41740483;    double wuxcBhGXOkpupPGUe12525504 = -513447288;    double wuxcBhGXOkpupPGUe28229680 = -279668368;    double wuxcBhGXOkpupPGUe93944170 = -437385198;    double wuxcBhGXOkpupPGUe13826785 = -63520273;    double wuxcBhGXOkpupPGUe13294846 = -519782912;    double wuxcBhGXOkpupPGUe96129490 = -282439764;    double wuxcBhGXOkpupPGUe35962120 = 36690129;    double wuxcBhGXOkpupPGUe92983148 = -708488545;    double wuxcBhGXOkpupPGUe49709048 = 39144042;    double wuxcBhGXOkpupPGUe4436782 = -114459796;    double wuxcBhGXOkpupPGUe24629123 = -544717211;    double wuxcBhGXOkpupPGUe95487719 = -810817300;    double wuxcBhGXOkpupPGUe70925764 = -785990734;    double wuxcBhGXOkpupPGUe87256275 = -913810457;    double wuxcBhGXOkpupPGUe78095669 = -81001796;    double wuxcBhGXOkpupPGUe35543377 = -947229830;    double wuxcBhGXOkpupPGUe84216706 = -736476488;    double wuxcBhGXOkpupPGUe30446591 = -628084618;    double wuxcBhGXOkpupPGUe46153543 = -191500357;    double wuxcBhGXOkpupPGUe50607239 = -49331728;    double wuxcBhGXOkpupPGUe56196021 = -774800067;    double wuxcBhGXOkpupPGUe32190939 = -544858258;    double wuxcBhGXOkpupPGUe8463037 = -826389732;    double wuxcBhGXOkpupPGUe22095351 = -901312130;    double wuxcBhGXOkpupPGUe22246856 = -296323754;    double wuxcBhGXOkpupPGUe62716329 = -230809589;    double wuxcBhGXOkpupPGUe75212835 = -483064823;    double wuxcBhGXOkpupPGUe4144877 = -575415080;    double wuxcBhGXOkpupPGUe84819618 = -102234483;    double wuxcBhGXOkpupPGUe80628635 = -21856861;    double wuxcBhGXOkpupPGUe60745151 = -520580512;    double wuxcBhGXOkpupPGUe42041705 = -837420958;    double wuxcBhGXOkpupPGUe1217444 = -555984708;    double wuxcBhGXOkpupPGUe40727354 = -281931764;    double wuxcBhGXOkpupPGUe79309607 = -565363040;    double wuxcBhGXOkpupPGUe66945872 = -982966159;    double wuxcBhGXOkpupPGUe68732898 = -285301767;    double wuxcBhGXOkpupPGUe64401264 = -750013573;    double wuxcBhGXOkpupPGUe21213239 = -662333644;    double wuxcBhGXOkpupPGUe92727575 = -666175348;    double wuxcBhGXOkpupPGUe96751188 = -306750469;    double wuxcBhGXOkpupPGUe10387981 = -608249800;    double wuxcBhGXOkpupPGUe93245719 = -357312754;    double wuxcBhGXOkpupPGUe5894821 = -626967960;    double wuxcBhGXOkpupPGUe69701919 = -366557314;    double wuxcBhGXOkpupPGUe38502340 = -670211183;    double wuxcBhGXOkpupPGUe95683104 = -148284408;    double wuxcBhGXOkpupPGUe59093468 = -285154854;    double wuxcBhGXOkpupPGUe96284317 = -42016255;    double wuxcBhGXOkpupPGUe10466578 = -262850045;    double wuxcBhGXOkpupPGUe85048191 = -549250104;    double wuxcBhGXOkpupPGUe3477909 = -247446423;    double wuxcBhGXOkpupPGUe79927009 = -442681267;    double wuxcBhGXOkpupPGUe48323690 = -166433186;    double wuxcBhGXOkpupPGUe26311696 = -266940417;    double wuxcBhGXOkpupPGUe80334564 = -968589030;    double wuxcBhGXOkpupPGUe19766644 = -453278637;    double wuxcBhGXOkpupPGUe71848820 = -536073068;    double wuxcBhGXOkpupPGUe91579928 = -767196519;    double wuxcBhGXOkpupPGUe50578516 = -188973323;    double wuxcBhGXOkpupPGUe20916655 = -799374941;    double wuxcBhGXOkpupPGUe31817243 = -387894791;    double wuxcBhGXOkpupPGUe8163530 = -506254062;    double wuxcBhGXOkpupPGUe69080412 = -938999098;    double wuxcBhGXOkpupPGUe43691630 = -593879285;    double wuxcBhGXOkpupPGUe82587417 = -707296254;    double wuxcBhGXOkpupPGUe94270275 = -154832592;    double wuxcBhGXOkpupPGUe30198411 = -404058970;    double wuxcBhGXOkpupPGUe7946668 = -248447418;    double wuxcBhGXOkpupPGUe11149797 = -98035638;    double wuxcBhGXOkpupPGUe66810479 = -561928063;    double wuxcBhGXOkpupPGUe19815443 = -986462916;    double wuxcBhGXOkpupPGUe9233352 = -965750974;    double wuxcBhGXOkpupPGUe53425968 = -525325010;    double wuxcBhGXOkpupPGUe53856050 = -742581259;    double wuxcBhGXOkpupPGUe45808041 = -66550268;    double wuxcBhGXOkpupPGUe38945220 = -87545505;    double wuxcBhGXOkpupPGUe2568217 = -99421772;    double wuxcBhGXOkpupPGUe52393432 = -434754817;    double wuxcBhGXOkpupPGUe83744515 = -626112572;    double wuxcBhGXOkpupPGUe67033225 = 17474818;    double wuxcBhGXOkpupPGUe16119367 = -97909970;    double wuxcBhGXOkpupPGUe7860560 = -433398825;    double wuxcBhGXOkpupPGUe74353040 = -839384439;    double wuxcBhGXOkpupPGUe95580444 = -472606757;    double wuxcBhGXOkpupPGUe57267242 = -173134089;    double wuxcBhGXOkpupPGUe62114696 = -294739692;    double wuxcBhGXOkpupPGUe52893753 = -289551522;    double wuxcBhGXOkpupPGUe14415659 = 88725475;     wuxcBhGXOkpupPGUe94331041 = wuxcBhGXOkpupPGUe33990060;     wuxcBhGXOkpupPGUe33990060 = wuxcBhGXOkpupPGUe66608869;     wuxcBhGXOkpupPGUe66608869 = wuxcBhGXOkpupPGUe46349743;     wuxcBhGXOkpupPGUe46349743 = wuxcBhGXOkpupPGUe74379987;     wuxcBhGXOkpupPGUe74379987 = wuxcBhGXOkpupPGUe46009955;     wuxcBhGXOkpupPGUe46009955 = wuxcBhGXOkpupPGUe69264898;     wuxcBhGXOkpupPGUe69264898 = wuxcBhGXOkpupPGUe33924500;     wuxcBhGXOkpupPGUe33924500 = wuxcBhGXOkpupPGUe26080553;     wuxcBhGXOkpupPGUe26080553 = wuxcBhGXOkpupPGUe98930928;     wuxcBhGXOkpupPGUe98930928 = wuxcBhGXOkpupPGUe82507717;     wuxcBhGXOkpupPGUe82507717 = wuxcBhGXOkpupPGUe12525504;     wuxcBhGXOkpupPGUe12525504 = wuxcBhGXOkpupPGUe28229680;     wuxcBhGXOkpupPGUe28229680 = wuxcBhGXOkpupPGUe93944170;     wuxcBhGXOkpupPGUe93944170 = wuxcBhGXOkpupPGUe13826785;     wuxcBhGXOkpupPGUe13826785 = wuxcBhGXOkpupPGUe13294846;     wuxcBhGXOkpupPGUe13294846 = wuxcBhGXOkpupPGUe96129490;     wuxcBhGXOkpupPGUe96129490 = wuxcBhGXOkpupPGUe35962120;     wuxcBhGXOkpupPGUe35962120 = wuxcBhGXOkpupPGUe92983148;     wuxcBhGXOkpupPGUe92983148 = wuxcBhGXOkpupPGUe49709048;     wuxcBhGXOkpupPGUe49709048 = wuxcBhGXOkpupPGUe4436782;     wuxcBhGXOkpupPGUe4436782 = wuxcBhGXOkpupPGUe24629123;     wuxcBhGXOkpupPGUe24629123 = wuxcBhGXOkpupPGUe95487719;     wuxcBhGXOkpupPGUe95487719 = wuxcBhGXOkpupPGUe70925764;     wuxcBhGXOkpupPGUe70925764 = wuxcBhGXOkpupPGUe87256275;     wuxcBhGXOkpupPGUe87256275 = wuxcBhGXOkpupPGUe78095669;     wuxcBhGXOkpupPGUe78095669 = wuxcBhGXOkpupPGUe35543377;     wuxcBhGXOkpupPGUe35543377 = wuxcBhGXOkpupPGUe84216706;     wuxcBhGXOkpupPGUe84216706 = wuxcBhGXOkpupPGUe30446591;     wuxcBhGXOkpupPGUe30446591 = wuxcBhGXOkpupPGUe46153543;     wuxcBhGXOkpupPGUe46153543 = wuxcBhGXOkpupPGUe50607239;     wuxcBhGXOkpupPGUe50607239 = wuxcBhGXOkpupPGUe56196021;     wuxcBhGXOkpupPGUe56196021 = wuxcBhGXOkpupPGUe32190939;     wuxcBhGXOkpupPGUe32190939 = wuxcBhGXOkpupPGUe8463037;     wuxcBhGXOkpupPGUe8463037 = wuxcBhGXOkpupPGUe22095351;     wuxcBhGXOkpupPGUe22095351 = wuxcBhGXOkpupPGUe22246856;     wuxcBhGXOkpupPGUe22246856 = wuxcBhGXOkpupPGUe62716329;     wuxcBhGXOkpupPGUe62716329 = wuxcBhGXOkpupPGUe75212835;     wuxcBhGXOkpupPGUe75212835 = wuxcBhGXOkpupPGUe4144877;     wuxcBhGXOkpupPGUe4144877 = wuxcBhGXOkpupPGUe84819618;     wuxcBhGXOkpupPGUe84819618 = wuxcBhGXOkpupPGUe80628635;     wuxcBhGXOkpupPGUe80628635 = wuxcBhGXOkpupPGUe60745151;     wuxcBhGXOkpupPGUe60745151 = wuxcBhGXOkpupPGUe42041705;     wuxcBhGXOkpupPGUe42041705 = wuxcBhGXOkpupPGUe1217444;     wuxcBhGXOkpupPGUe1217444 = wuxcBhGXOkpupPGUe40727354;     wuxcBhGXOkpupPGUe40727354 = wuxcBhGXOkpupPGUe79309607;     wuxcBhGXOkpupPGUe79309607 = wuxcBhGXOkpupPGUe66945872;     wuxcBhGXOkpupPGUe66945872 = wuxcBhGXOkpupPGUe68732898;     wuxcBhGXOkpupPGUe68732898 = wuxcBhGXOkpupPGUe64401264;     wuxcBhGXOkpupPGUe64401264 = wuxcBhGXOkpupPGUe21213239;     wuxcBhGXOkpupPGUe21213239 = wuxcBhGXOkpupPGUe92727575;     wuxcBhGXOkpupPGUe92727575 = wuxcBhGXOkpupPGUe96751188;     wuxcBhGXOkpupPGUe96751188 = wuxcBhGXOkpupPGUe10387981;     wuxcBhGXOkpupPGUe10387981 = wuxcBhGXOkpupPGUe93245719;     wuxcBhGXOkpupPGUe93245719 = wuxcBhGXOkpupPGUe5894821;     wuxcBhGXOkpupPGUe5894821 = wuxcBhGXOkpupPGUe69701919;     wuxcBhGXOkpupPGUe69701919 = wuxcBhGXOkpupPGUe38502340;     wuxcBhGXOkpupPGUe38502340 = wuxcBhGXOkpupPGUe95683104;     wuxcBhGXOkpupPGUe95683104 = wuxcBhGXOkpupPGUe59093468;     wuxcBhGXOkpupPGUe59093468 = wuxcBhGXOkpupPGUe96284317;     wuxcBhGXOkpupPGUe96284317 = wuxcBhGXOkpupPGUe10466578;     wuxcBhGXOkpupPGUe10466578 = wuxcBhGXOkpupPGUe85048191;     wuxcBhGXOkpupPGUe85048191 = wuxcBhGXOkpupPGUe3477909;     wuxcBhGXOkpupPGUe3477909 = wuxcBhGXOkpupPGUe79927009;     wuxcBhGXOkpupPGUe79927009 = wuxcBhGXOkpupPGUe48323690;     wuxcBhGXOkpupPGUe48323690 = wuxcBhGXOkpupPGUe26311696;     wuxcBhGXOkpupPGUe26311696 = wuxcBhGXOkpupPGUe80334564;     wuxcBhGXOkpupPGUe80334564 = wuxcBhGXOkpupPGUe19766644;     wuxcBhGXOkpupPGUe19766644 = wuxcBhGXOkpupPGUe71848820;     wuxcBhGXOkpupPGUe71848820 = wuxcBhGXOkpupPGUe91579928;     wuxcBhGXOkpupPGUe91579928 = wuxcBhGXOkpupPGUe50578516;     wuxcBhGXOkpupPGUe50578516 = wuxcBhGXOkpupPGUe20916655;     wuxcBhGXOkpupPGUe20916655 = wuxcBhGXOkpupPGUe31817243;     wuxcBhGXOkpupPGUe31817243 = wuxcBhGXOkpupPGUe8163530;     wuxcBhGXOkpupPGUe8163530 = wuxcBhGXOkpupPGUe69080412;     wuxcBhGXOkpupPGUe69080412 = wuxcBhGXOkpupPGUe43691630;     wuxcBhGXOkpupPGUe43691630 = wuxcBhGXOkpupPGUe82587417;     wuxcBhGXOkpupPGUe82587417 = wuxcBhGXOkpupPGUe94270275;     wuxcBhGXOkpupPGUe94270275 = wuxcBhGXOkpupPGUe30198411;     wuxcBhGXOkpupPGUe30198411 = wuxcBhGXOkpupPGUe7946668;     wuxcBhGXOkpupPGUe7946668 = wuxcBhGXOkpupPGUe11149797;     wuxcBhGXOkpupPGUe11149797 = wuxcBhGXOkpupPGUe66810479;     wuxcBhGXOkpupPGUe66810479 = wuxcBhGXOkpupPGUe19815443;     wuxcBhGXOkpupPGUe19815443 = wuxcBhGXOkpupPGUe9233352;     wuxcBhGXOkpupPGUe9233352 = wuxcBhGXOkpupPGUe53425968;     wuxcBhGXOkpupPGUe53425968 = wuxcBhGXOkpupPGUe53856050;     wuxcBhGXOkpupPGUe53856050 = wuxcBhGXOkpupPGUe45808041;     wuxcBhGXOkpupPGUe45808041 = wuxcBhGXOkpupPGUe38945220;     wuxcBhGXOkpupPGUe38945220 = wuxcBhGXOkpupPGUe2568217;     wuxcBhGXOkpupPGUe2568217 = wuxcBhGXOkpupPGUe52393432;     wuxcBhGXOkpupPGUe52393432 = wuxcBhGXOkpupPGUe83744515;     wuxcBhGXOkpupPGUe83744515 = wuxcBhGXOkpupPGUe67033225;     wuxcBhGXOkpupPGUe67033225 = wuxcBhGXOkpupPGUe16119367;     wuxcBhGXOkpupPGUe16119367 = wuxcBhGXOkpupPGUe7860560;     wuxcBhGXOkpupPGUe7860560 = wuxcBhGXOkpupPGUe74353040;     wuxcBhGXOkpupPGUe74353040 = wuxcBhGXOkpupPGUe95580444;     wuxcBhGXOkpupPGUe95580444 = wuxcBhGXOkpupPGUe57267242;     wuxcBhGXOkpupPGUe57267242 = wuxcBhGXOkpupPGUe62114696;     wuxcBhGXOkpupPGUe62114696 = wuxcBhGXOkpupPGUe52893753;     wuxcBhGXOkpupPGUe52893753 = wuxcBhGXOkpupPGUe14415659;     wuxcBhGXOkpupPGUe14415659 = wuxcBhGXOkpupPGUe94331041;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LSPyykPGdapdelCD25380669() {     double WhoUdRPoZsjyQndWp448148 = -23176414;    double WhoUdRPoZsjyQndWp77363339 = -190628263;    double WhoUdRPoZsjyQndWp82651052 = -85108062;    double WhoUdRPoZsjyQndWp73424524 = -389713941;    double WhoUdRPoZsjyQndWp43275475 = -743667923;    double WhoUdRPoZsjyQndWp37427158 = -474576109;    double WhoUdRPoZsjyQndWp29753864 = -424691569;    double WhoUdRPoZsjyQndWp83684390 = -162404953;    double WhoUdRPoZsjyQndWp90723048 = -502214289;    double WhoUdRPoZsjyQndWp40160588 = -644740391;    double WhoUdRPoZsjyQndWp42618008 = 80821930;    double WhoUdRPoZsjyQndWp44348546 = -957853490;    double WhoUdRPoZsjyQndWp14438799 = -719559457;    double WhoUdRPoZsjyQndWp77087847 = -969623766;    double WhoUdRPoZsjyQndWp9569212 = -753982440;    double WhoUdRPoZsjyQndWp54800239 = -492782455;    double WhoUdRPoZsjyQndWp89965493 = -898984876;    double WhoUdRPoZsjyQndWp33015273 = -728832514;    double WhoUdRPoZsjyQndWp54812990 = -14257642;    double WhoUdRPoZsjyQndWp30259213 = -317758878;    double WhoUdRPoZsjyQndWp92989974 = 30052748;    double WhoUdRPoZsjyQndWp11150146 = -452646928;    double WhoUdRPoZsjyQndWp31078511 = -557334786;    double WhoUdRPoZsjyQndWp54918138 = 91704278;    double WhoUdRPoZsjyQndWp30955926 = -187006490;    double WhoUdRPoZsjyQndWp2817345 = 40342880;    double WhoUdRPoZsjyQndWp34003156 = -847892033;    double WhoUdRPoZsjyQndWp97323832 = -802405442;    double WhoUdRPoZsjyQndWp96578846 = -850467322;    double WhoUdRPoZsjyQndWp14769181 = -8971225;    double WhoUdRPoZsjyQndWp42849788 = 37347334;    double WhoUdRPoZsjyQndWp53806643 = -484246883;    double WhoUdRPoZsjyQndWp41901888 = -437471629;    double WhoUdRPoZsjyQndWp3293601 = -577931497;    double WhoUdRPoZsjyQndWp83570003 = -440045236;    double WhoUdRPoZsjyQndWp90972769 = -510349862;    double WhoUdRPoZsjyQndWp43978264 = -184445459;    double WhoUdRPoZsjyQndWp9303048 = -254570833;    double WhoUdRPoZsjyQndWp77755130 = -569338577;    double WhoUdRPoZsjyQndWp72321556 = -575673706;    double WhoUdRPoZsjyQndWp86297859 = -87130755;    double WhoUdRPoZsjyQndWp92895607 = -790722790;    double WhoUdRPoZsjyQndWp20108684 = -918225542;    double WhoUdRPoZsjyQndWp88625184 = -575327646;    double WhoUdRPoZsjyQndWp52679819 = 56170081;    double WhoUdRPoZsjyQndWp91248802 = -550579156;    double WhoUdRPoZsjyQndWp84724447 = -1741697;    double WhoUdRPoZsjyQndWp76689839 = -295197299;    double WhoUdRPoZsjyQndWp74076993 = -835750561;    double WhoUdRPoZsjyQndWp34124892 = -749344452;    double WhoUdRPoZsjyQndWp28428254 = -508469019;    double WhoUdRPoZsjyQndWp7560909 = -107476789;    double WhoUdRPoZsjyQndWp58636438 = -400039388;    double WhoUdRPoZsjyQndWp89214599 = -677782899;    double WhoUdRPoZsjyQndWp32390695 = -227499743;    double WhoUdRPoZsjyQndWp89298001 = -570529486;    double WhoUdRPoZsjyQndWp46284828 = -633293477;    double WhoUdRPoZsjyQndWp27732914 = -76812341;    double WhoUdRPoZsjyQndWp42468599 = -102707452;    double WhoUdRPoZsjyQndWp40458131 = -684010803;    double WhoUdRPoZsjyQndWp3424003 = -626684077;    double WhoUdRPoZsjyQndWp32430032 = -622286128;    double WhoUdRPoZsjyQndWp87105544 = -311937632;    double WhoUdRPoZsjyQndWp75953868 = -393243065;    double WhoUdRPoZsjyQndWp97310799 = -582087726;    double WhoUdRPoZsjyQndWp88811364 = -434931188;    double WhoUdRPoZsjyQndWp2446659 = -420381862;    double WhoUdRPoZsjyQndWp11145198 = -41627960;    double WhoUdRPoZsjyQndWp93517844 = -429578531;    double WhoUdRPoZsjyQndWp18596443 = -143632578;    double WhoUdRPoZsjyQndWp10821976 = -208336997;    double WhoUdRPoZsjyQndWp80662445 = -544414044;    double WhoUdRPoZsjyQndWp55260142 = -59493937;    double WhoUdRPoZsjyQndWp82491434 = -438583937;    double WhoUdRPoZsjyQndWp43961353 = -130628123;    double WhoUdRPoZsjyQndWp94367 = -179224462;    double WhoUdRPoZsjyQndWp91041462 = -534421387;    double WhoUdRPoZsjyQndWp42453327 = -982007141;    double WhoUdRPoZsjyQndWp2238320 = -964465804;    double WhoUdRPoZsjyQndWp39707123 = -636427334;    double WhoUdRPoZsjyQndWp18092898 = -957915424;    double WhoUdRPoZsjyQndWp57313316 = -452694734;    double WhoUdRPoZsjyQndWp23246839 = -966654881;    double WhoUdRPoZsjyQndWp62453955 = -1122871;    double WhoUdRPoZsjyQndWp86340926 = -500502206;    double WhoUdRPoZsjyQndWp35288880 = -855175877;    double WhoUdRPoZsjyQndWp95170205 = 15792505;    double WhoUdRPoZsjyQndWp52687288 = -759688730;    double WhoUdRPoZsjyQndWp70902906 = -250431755;    double WhoUdRPoZsjyQndWp94272001 = -869515750;    double WhoUdRPoZsjyQndWp44687941 = -877056385;    double WhoUdRPoZsjyQndWp16245350 = -7633118;    double WhoUdRPoZsjyQndWp66834449 = -51863382;    double WhoUdRPoZsjyQndWp37297000 = -885327774;    double WhoUdRPoZsjyQndWp68897554 = -948989629;    double WhoUdRPoZsjyQndWp53867828 = -464844628;    double WhoUdRPoZsjyQndWp5790064 = -378785159;    double WhoUdRPoZsjyQndWp44154816 = -424982477;    double WhoUdRPoZsjyQndWp91314384 = -993239921;    double WhoUdRPoZsjyQndWp63868455 = -23176414;     WhoUdRPoZsjyQndWp448148 = WhoUdRPoZsjyQndWp77363339;     WhoUdRPoZsjyQndWp77363339 = WhoUdRPoZsjyQndWp82651052;     WhoUdRPoZsjyQndWp82651052 = WhoUdRPoZsjyQndWp73424524;     WhoUdRPoZsjyQndWp73424524 = WhoUdRPoZsjyQndWp43275475;     WhoUdRPoZsjyQndWp43275475 = WhoUdRPoZsjyQndWp37427158;     WhoUdRPoZsjyQndWp37427158 = WhoUdRPoZsjyQndWp29753864;     WhoUdRPoZsjyQndWp29753864 = WhoUdRPoZsjyQndWp83684390;     WhoUdRPoZsjyQndWp83684390 = WhoUdRPoZsjyQndWp90723048;     WhoUdRPoZsjyQndWp90723048 = WhoUdRPoZsjyQndWp40160588;     WhoUdRPoZsjyQndWp40160588 = WhoUdRPoZsjyQndWp42618008;     WhoUdRPoZsjyQndWp42618008 = WhoUdRPoZsjyQndWp44348546;     WhoUdRPoZsjyQndWp44348546 = WhoUdRPoZsjyQndWp14438799;     WhoUdRPoZsjyQndWp14438799 = WhoUdRPoZsjyQndWp77087847;     WhoUdRPoZsjyQndWp77087847 = WhoUdRPoZsjyQndWp9569212;     WhoUdRPoZsjyQndWp9569212 = WhoUdRPoZsjyQndWp54800239;     WhoUdRPoZsjyQndWp54800239 = WhoUdRPoZsjyQndWp89965493;     WhoUdRPoZsjyQndWp89965493 = WhoUdRPoZsjyQndWp33015273;     WhoUdRPoZsjyQndWp33015273 = WhoUdRPoZsjyQndWp54812990;     WhoUdRPoZsjyQndWp54812990 = WhoUdRPoZsjyQndWp30259213;     WhoUdRPoZsjyQndWp30259213 = WhoUdRPoZsjyQndWp92989974;     WhoUdRPoZsjyQndWp92989974 = WhoUdRPoZsjyQndWp11150146;     WhoUdRPoZsjyQndWp11150146 = WhoUdRPoZsjyQndWp31078511;     WhoUdRPoZsjyQndWp31078511 = WhoUdRPoZsjyQndWp54918138;     WhoUdRPoZsjyQndWp54918138 = WhoUdRPoZsjyQndWp30955926;     WhoUdRPoZsjyQndWp30955926 = WhoUdRPoZsjyQndWp2817345;     WhoUdRPoZsjyQndWp2817345 = WhoUdRPoZsjyQndWp34003156;     WhoUdRPoZsjyQndWp34003156 = WhoUdRPoZsjyQndWp97323832;     WhoUdRPoZsjyQndWp97323832 = WhoUdRPoZsjyQndWp96578846;     WhoUdRPoZsjyQndWp96578846 = WhoUdRPoZsjyQndWp14769181;     WhoUdRPoZsjyQndWp14769181 = WhoUdRPoZsjyQndWp42849788;     WhoUdRPoZsjyQndWp42849788 = WhoUdRPoZsjyQndWp53806643;     WhoUdRPoZsjyQndWp53806643 = WhoUdRPoZsjyQndWp41901888;     WhoUdRPoZsjyQndWp41901888 = WhoUdRPoZsjyQndWp3293601;     WhoUdRPoZsjyQndWp3293601 = WhoUdRPoZsjyQndWp83570003;     WhoUdRPoZsjyQndWp83570003 = WhoUdRPoZsjyQndWp90972769;     WhoUdRPoZsjyQndWp90972769 = WhoUdRPoZsjyQndWp43978264;     WhoUdRPoZsjyQndWp43978264 = WhoUdRPoZsjyQndWp9303048;     WhoUdRPoZsjyQndWp9303048 = WhoUdRPoZsjyQndWp77755130;     WhoUdRPoZsjyQndWp77755130 = WhoUdRPoZsjyQndWp72321556;     WhoUdRPoZsjyQndWp72321556 = WhoUdRPoZsjyQndWp86297859;     WhoUdRPoZsjyQndWp86297859 = WhoUdRPoZsjyQndWp92895607;     WhoUdRPoZsjyQndWp92895607 = WhoUdRPoZsjyQndWp20108684;     WhoUdRPoZsjyQndWp20108684 = WhoUdRPoZsjyQndWp88625184;     WhoUdRPoZsjyQndWp88625184 = WhoUdRPoZsjyQndWp52679819;     WhoUdRPoZsjyQndWp52679819 = WhoUdRPoZsjyQndWp91248802;     WhoUdRPoZsjyQndWp91248802 = WhoUdRPoZsjyQndWp84724447;     WhoUdRPoZsjyQndWp84724447 = WhoUdRPoZsjyQndWp76689839;     WhoUdRPoZsjyQndWp76689839 = WhoUdRPoZsjyQndWp74076993;     WhoUdRPoZsjyQndWp74076993 = WhoUdRPoZsjyQndWp34124892;     WhoUdRPoZsjyQndWp34124892 = WhoUdRPoZsjyQndWp28428254;     WhoUdRPoZsjyQndWp28428254 = WhoUdRPoZsjyQndWp7560909;     WhoUdRPoZsjyQndWp7560909 = WhoUdRPoZsjyQndWp58636438;     WhoUdRPoZsjyQndWp58636438 = WhoUdRPoZsjyQndWp89214599;     WhoUdRPoZsjyQndWp89214599 = WhoUdRPoZsjyQndWp32390695;     WhoUdRPoZsjyQndWp32390695 = WhoUdRPoZsjyQndWp89298001;     WhoUdRPoZsjyQndWp89298001 = WhoUdRPoZsjyQndWp46284828;     WhoUdRPoZsjyQndWp46284828 = WhoUdRPoZsjyQndWp27732914;     WhoUdRPoZsjyQndWp27732914 = WhoUdRPoZsjyQndWp42468599;     WhoUdRPoZsjyQndWp42468599 = WhoUdRPoZsjyQndWp40458131;     WhoUdRPoZsjyQndWp40458131 = WhoUdRPoZsjyQndWp3424003;     WhoUdRPoZsjyQndWp3424003 = WhoUdRPoZsjyQndWp32430032;     WhoUdRPoZsjyQndWp32430032 = WhoUdRPoZsjyQndWp87105544;     WhoUdRPoZsjyQndWp87105544 = WhoUdRPoZsjyQndWp75953868;     WhoUdRPoZsjyQndWp75953868 = WhoUdRPoZsjyQndWp97310799;     WhoUdRPoZsjyQndWp97310799 = WhoUdRPoZsjyQndWp88811364;     WhoUdRPoZsjyQndWp88811364 = WhoUdRPoZsjyQndWp2446659;     WhoUdRPoZsjyQndWp2446659 = WhoUdRPoZsjyQndWp11145198;     WhoUdRPoZsjyQndWp11145198 = WhoUdRPoZsjyQndWp93517844;     WhoUdRPoZsjyQndWp93517844 = WhoUdRPoZsjyQndWp18596443;     WhoUdRPoZsjyQndWp18596443 = WhoUdRPoZsjyQndWp10821976;     WhoUdRPoZsjyQndWp10821976 = WhoUdRPoZsjyQndWp80662445;     WhoUdRPoZsjyQndWp80662445 = WhoUdRPoZsjyQndWp55260142;     WhoUdRPoZsjyQndWp55260142 = WhoUdRPoZsjyQndWp82491434;     WhoUdRPoZsjyQndWp82491434 = WhoUdRPoZsjyQndWp43961353;     WhoUdRPoZsjyQndWp43961353 = WhoUdRPoZsjyQndWp94367;     WhoUdRPoZsjyQndWp94367 = WhoUdRPoZsjyQndWp91041462;     WhoUdRPoZsjyQndWp91041462 = WhoUdRPoZsjyQndWp42453327;     WhoUdRPoZsjyQndWp42453327 = WhoUdRPoZsjyQndWp2238320;     WhoUdRPoZsjyQndWp2238320 = WhoUdRPoZsjyQndWp39707123;     WhoUdRPoZsjyQndWp39707123 = WhoUdRPoZsjyQndWp18092898;     WhoUdRPoZsjyQndWp18092898 = WhoUdRPoZsjyQndWp57313316;     WhoUdRPoZsjyQndWp57313316 = WhoUdRPoZsjyQndWp23246839;     WhoUdRPoZsjyQndWp23246839 = WhoUdRPoZsjyQndWp62453955;     WhoUdRPoZsjyQndWp62453955 = WhoUdRPoZsjyQndWp86340926;     WhoUdRPoZsjyQndWp86340926 = WhoUdRPoZsjyQndWp35288880;     WhoUdRPoZsjyQndWp35288880 = WhoUdRPoZsjyQndWp95170205;     WhoUdRPoZsjyQndWp95170205 = WhoUdRPoZsjyQndWp52687288;     WhoUdRPoZsjyQndWp52687288 = WhoUdRPoZsjyQndWp70902906;     WhoUdRPoZsjyQndWp70902906 = WhoUdRPoZsjyQndWp94272001;     WhoUdRPoZsjyQndWp94272001 = WhoUdRPoZsjyQndWp44687941;     WhoUdRPoZsjyQndWp44687941 = WhoUdRPoZsjyQndWp16245350;     WhoUdRPoZsjyQndWp16245350 = WhoUdRPoZsjyQndWp66834449;     WhoUdRPoZsjyQndWp66834449 = WhoUdRPoZsjyQndWp37297000;     WhoUdRPoZsjyQndWp37297000 = WhoUdRPoZsjyQndWp68897554;     WhoUdRPoZsjyQndWp68897554 = WhoUdRPoZsjyQndWp53867828;     WhoUdRPoZsjyQndWp53867828 = WhoUdRPoZsjyQndWp5790064;     WhoUdRPoZsjyQndWp5790064 = WhoUdRPoZsjyQndWp44154816;     WhoUdRPoZsjyQndWp44154816 = WhoUdRPoZsjyQndWp91314384;     WhoUdRPoZsjyQndWp91314384 = WhoUdRPoZsjyQndWp63868455;     WhoUdRPoZsjyQndWp63868455 = WhoUdRPoZsjyQndWp448148;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void CnZuqzPmnnPfctvl22988696() {     double mMndTjaxTJKwFshxn3639837 = -786329693;    double mMndTjaxTJKwFshxn27985298 = -540158110;    double mMndTjaxTJKwFshxn88074161 = -705044368;    double mMndTjaxTJKwFshxn89219626 = -211170133;    double mMndTjaxTJKwFshxn23283328 = -667888530;    double mMndTjaxTJKwFshxn74358749 = -468381546;    double mMndTjaxTJKwFshxn11466306 = -889511937;    double mMndTjaxTJKwFshxn92274706 = -876667505;    double mMndTjaxTJKwFshxn1629374 = -827888365;    double mMndTjaxTJKwFshxn64194899 = -873367452;    double mMndTjaxTJKwFshxn39730307 = -192453628;    double mMndTjaxTJKwFshxn38054930 = -886295466;    double mMndTjaxTJKwFshxn55864310 = -986748977;    double mMndTjaxTJKwFshxn79877032 = -761592355;    double mMndTjaxTJKwFshxn74274931 = -977763536;    double mMndTjaxTJKwFshxn92829916 = -880102040;    double mMndTjaxTJKwFshxn95793017 = -129639421;    double mMndTjaxTJKwFshxn71155293 = -90438806;    double mMndTjaxTJKwFshxn95424350 = -180111640;    double mMndTjaxTJKwFshxn84978158 = -651588251;    double mMndTjaxTJKwFshxn72785556 = -150626098;    double mMndTjaxTJKwFshxn27134300 = -877605427;    double mMndTjaxTJKwFshxn68427382 = -884304142;    double mMndTjaxTJKwFshxn73782502 = -184072367;    double mMndTjaxTJKwFshxn78761507 = -559637361;    double mMndTjaxTJKwFshxn60577564 = -88419366;    double mMndTjaxTJKwFshxn39523226 = -956918387;    double mMndTjaxTJKwFshxn90714707 = -752580209;    double mMndTjaxTJKwFshxn10241945 = -288574368;    double mMndTjaxTJKwFshxn35024362 = -773016294;    double mMndTjaxTJKwFshxn54018296 = -47076779;    double mMndTjaxTJKwFshxn18179243 = -497793124;    double mMndTjaxTJKwFshxn67502692 = -557893487;    double mMndTjaxTJKwFshxn97774821 = -186631351;    double mMndTjaxTJKwFshxn91103664 = -237038650;    double mMndTjaxTJKwFshxn96899940 = -792314948;    double mMndTjaxTJKwFshxn12125305 = 68337170;    double mMndTjaxTJKwFshxn1167313 = -230922865;    double mMndTjaxTJKwFshxn5684175 = -239312284;    double mMndTjaxTJKwFshxn32792408 = -518851682;    double mMndTjaxTJKwFshxn92547437 = -246957177;    double mMndTjaxTJKwFshxn34094468 = -635644685;    double mMndTjaxTJKwFshxn24042071 = -518431594;    double mMndTjaxTJKwFshxn19206242 = -603344039;    double mMndTjaxTJKwFshxn60695481 = -546966960;    double mMndTjaxTJKwFshxn89976811 = -702942269;    double mMndTjaxTJKwFshxn99893296 = 62919074;    double mMndTjaxTJKwFshxn15587517 = -240606511;    double mMndTjaxTJKwFshxn49086798 = -549473950;    double mMndTjaxTJKwFshxn10896564 = -42372232;    double mMndTjaxTJKwFshxn42954655 = 19393648;    double mMndTjaxTJKwFshxn75515488 = -236882435;    double mMndTjaxTJKwFshxn73843987 = -88080715;    double mMndTjaxTJKwFshxn8077234 = -149085723;    double mMndTjaxTJKwFshxn50203139 = -318785323;    double mMndTjaxTJKwFshxn76505536 = -908724266;    double mMndTjaxTJKwFshxn59557916 = -655853969;    double mMndTjaxTJKwFshxn14291660 = -420972002;    double mMndTjaxTJKwFshxn10458119 = -651532773;    double mMndTjaxTJKwFshxn62705764 = -479469165;    double mMndTjaxTJKwFshxn34835524 = -511463159;    double mMndTjaxTJKwFshxn20751599 = -36931728;    double mMndTjaxTJKwFshxn82032762 = -488093138;    double mMndTjaxTJKwFshxn66605012 = 45127928;    double mMndTjaxTJKwFshxn10176604 = -726290674;    double mMndTjaxTJKwFshxn21551065 = -694660505;    double mMndTjaxTJKwFshxn70552238 = -228401979;    double mMndTjaxTJKwFshxn58089488 = -700117626;    double mMndTjaxTJKwFshxn88773368 = -424553705;    double mMndTjaxTJKwFshxn77374991 = -85448589;    double mMndTjaxTJKwFshxn80704612 = -848439211;    double mMndTjaxTJKwFshxn94625704 = -898716556;    double mMndTjaxTJKwFshxn65471119 = -851126523;    double mMndTjaxTJKwFshxn62631942 = -661259959;    double mMndTjaxTJKwFshxn92430721 = -304631074;    double mMndTjaxTJKwFshxn38691089 = -514981413;    double mMndTjaxTJKwFshxn3092230 = -259173834;    double mMndTjaxTJKwFshxn49221140 = -180960103;    double mMndTjaxTJKwFshxn13087021 = -637105407;    double mMndTjaxTJKwFshxn88784696 = -856695092;    double mMndTjaxTJKwFshxn60684267 = -51338440;    double mMndTjaxTJKwFshxn23935710 = -616311877;    double mMndTjaxTJKwFshxn41627909 = -103106259;    double mMndTjaxTJKwFshxn99345380 = -146202136;    double mMndTjaxTJKwFshxn92069706 = -692409942;    double mMndTjaxTJKwFshxn78502808 = -810194344;    double mMndTjaxTJKwFshxn44335256 = -309712409;    double mMndTjaxTJKwFshxn59425458 = -308807764;    double mMndTjaxTJKwFshxn47571683 = -867846029;    double mMndTjaxTJKwFshxn14598129 = -328314384;    double mMndTjaxTJKwFshxn37342024 = -36460979;    double mMndTjaxTJKwFshxn97833645 = -510690829;    double mMndTjaxTJKwFshxn90709194 = -579390093;    double mMndTjaxTJKwFshxn42978410 = -759843119;    double mMndTjaxTJKwFshxn97956884 = 92611477;    double mMndTjaxTJKwFshxn71795839 = -110025450;    double mMndTjaxTJKwFshxn52061705 = -47551548;    double mMndTjaxTJKwFshxn57437059 = -463559523;    double mMndTjaxTJKwFshxn9029639 = -877053366;    double mMndTjaxTJKwFshxn39144417 = -786329693;     mMndTjaxTJKwFshxn3639837 = mMndTjaxTJKwFshxn27985298;     mMndTjaxTJKwFshxn27985298 = mMndTjaxTJKwFshxn88074161;     mMndTjaxTJKwFshxn88074161 = mMndTjaxTJKwFshxn89219626;     mMndTjaxTJKwFshxn89219626 = mMndTjaxTJKwFshxn23283328;     mMndTjaxTJKwFshxn23283328 = mMndTjaxTJKwFshxn74358749;     mMndTjaxTJKwFshxn74358749 = mMndTjaxTJKwFshxn11466306;     mMndTjaxTJKwFshxn11466306 = mMndTjaxTJKwFshxn92274706;     mMndTjaxTJKwFshxn92274706 = mMndTjaxTJKwFshxn1629374;     mMndTjaxTJKwFshxn1629374 = mMndTjaxTJKwFshxn64194899;     mMndTjaxTJKwFshxn64194899 = mMndTjaxTJKwFshxn39730307;     mMndTjaxTJKwFshxn39730307 = mMndTjaxTJKwFshxn38054930;     mMndTjaxTJKwFshxn38054930 = mMndTjaxTJKwFshxn55864310;     mMndTjaxTJKwFshxn55864310 = mMndTjaxTJKwFshxn79877032;     mMndTjaxTJKwFshxn79877032 = mMndTjaxTJKwFshxn74274931;     mMndTjaxTJKwFshxn74274931 = mMndTjaxTJKwFshxn92829916;     mMndTjaxTJKwFshxn92829916 = mMndTjaxTJKwFshxn95793017;     mMndTjaxTJKwFshxn95793017 = mMndTjaxTJKwFshxn71155293;     mMndTjaxTJKwFshxn71155293 = mMndTjaxTJKwFshxn95424350;     mMndTjaxTJKwFshxn95424350 = mMndTjaxTJKwFshxn84978158;     mMndTjaxTJKwFshxn84978158 = mMndTjaxTJKwFshxn72785556;     mMndTjaxTJKwFshxn72785556 = mMndTjaxTJKwFshxn27134300;     mMndTjaxTJKwFshxn27134300 = mMndTjaxTJKwFshxn68427382;     mMndTjaxTJKwFshxn68427382 = mMndTjaxTJKwFshxn73782502;     mMndTjaxTJKwFshxn73782502 = mMndTjaxTJKwFshxn78761507;     mMndTjaxTJKwFshxn78761507 = mMndTjaxTJKwFshxn60577564;     mMndTjaxTJKwFshxn60577564 = mMndTjaxTJKwFshxn39523226;     mMndTjaxTJKwFshxn39523226 = mMndTjaxTJKwFshxn90714707;     mMndTjaxTJKwFshxn90714707 = mMndTjaxTJKwFshxn10241945;     mMndTjaxTJKwFshxn10241945 = mMndTjaxTJKwFshxn35024362;     mMndTjaxTJKwFshxn35024362 = mMndTjaxTJKwFshxn54018296;     mMndTjaxTJKwFshxn54018296 = mMndTjaxTJKwFshxn18179243;     mMndTjaxTJKwFshxn18179243 = mMndTjaxTJKwFshxn67502692;     mMndTjaxTJKwFshxn67502692 = mMndTjaxTJKwFshxn97774821;     mMndTjaxTJKwFshxn97774821 = mMndTjaxTJKwFshxn91103664;     mMndTjaxTJKwFshxn91103664 = mMndTjaxTJKwFshxn96899940;     mMndTjaxTJKwFshxn96899940 = mMndTjaxTJKwFshxn12125305;     mMndTjaxTJKwFshxn12125305 = mMndTjaxTJKwFshxn1167313;     mMndTjaxTJKwFshxn1167313 = mMndTjaxTJKwFshxn5684175;     mMndTjaxTJKwFshxn5684175 = mMndTjaxTJKwFshxn32792408;     mMndTjaxTJKwFshxn32792408 = mMndTjaxTJKwFshxn92547437;     mMndTjaxTJKwFshxn92547437 = mMndTjaxTJKwFshxn34094468;     mMndTjaxTJKwFshxn34094468 = mMndTjaxTJKwFshxn24042071;     mMndTjaxTJKwFshxn24042071 = mMndTjaxTJKwFshxn19206242;     mMndTjaxTJKwFshxn19206242 = mMndTjaxTJKwFshxn60695481;     mMndTjaxTJKwFshxn60695481 = mMndTjaxTJKwFshxn89976811;     mMndTjaxTJKwFshxn89976811 = mMndTjaxTJKwFshxn99893296;     mMndTjaxTJKwFshxn99893296 = mMndTjaxTJKwFshxn15587517;     mMndTjaxTJKwFshxn15587517 = mMndTjaxTJKwFshxn49086798;     mMndTjaxTJKwFshxn49086798 = mMndTjaxTJKwFshxn10896564;     mMndTjaxTJKwFshxn10896564 = mMndTjaxTJKwFshxn42954655;     mMndTjaxTJKwFshxn42954655 = mMndTjaxTJKwFshxn75515488;     mMndTjaxTJKwFshxn75515488 = mMndTjaxTJKwFshxn73843987;     mMndTjaxTJKwFshxn73843987 = mMndTjaxTJKwFshxn8077234;     mMndTjaxTJKwFshxn8077234 = mMndTjaxTJKwFshxn50203139;     mMndTjaxTJKwFshxn50203139 = mMndTjaxTJKwFshxn76505536;     mMndTjaxTJKwFshxn76505536 = mMndTjaxTJKwFshxn59557916;     mMndTjaxTJKwFshxn59557916 = mMndTjaxTJKwFshxn14291660;     mMndTjaxTJKwFshxn14291660 = mMndTjaxTJKwFshxn10458119;     mMndTjaxTJKwFshxn10458119 = mMndTjaxTJKwFshxn62705764;     mMndTjaxTJKwFshxn62705764 = mMndTjaxTJKwFshxn34835524;     mMndTjaxTJKwFshxn34835524 = mMndTjaxTJKwFshxn20751599;     mMndTjaxTJKwFshxn20751599 = mMndTjaxTJKwFshxn82032762;     mMndTjaxTJKwFshxn82032762 = mMndTjaxTJKwFshxn66605012;     mMndTjaxTJKwFshxn66605012 = mMndTjaxTJKwFshxn10176604;     mMndTjaxTJKwFshxn10176604 = mMndTjaxTJKwFshxn21551065;     mMndTjaxTJKwFshxn21551065 = mMndTjaxTJKwFshxn70552238;     mMndTjaxTJKwFshxn70552238 = mMndTjaxTJKwFshxn58089488;     mMndTjaxTJKwFshxn58089488 = mMndTjaxTJKwFshxn88773368;     mMndTjaxTJKwFshxn88773368 = mMndTjaxTJKwFshxn77374991;     mMndTjaxTJKwFshxn77374991 = mMndTjaxTJKwFshxn80704612;     mMndTjaxTJKwFshxn80704612 = mMndTjaxTJKwFshxn94625704;     mMndTjaxTJKwFshxn94625704 = mMndTjaxTJKwFshxn65471119;     mMndTjaxTJKwFshxn65471119 = mMndTjaxTJKwFshxn62631942;     mMndTjaxTJKwFshxn62631942 = mMndTjaxTJKwFshxn92430721;     mMndTjaxTJKwFshxn92430721 = mMndTjaxTJKwFshxn38691089;     mMndTjaxTJKwFshxn38691089 = mMndTjaxTJKwFshxn3092230;     mMndTjaxTJKwFshxn3092230 = mMndTjaxTJKwFshxn49221140;     mMndTjaxTJKwFshxn49221140 = mMndTjaxTJKwFshxn13087021;     mMndTjaxTJKwFshxn13087021 = mMndTjaxTJKwFshxn88784696;     mMndTjaxTJKwFshxn88784696 = mMndTjaxTJKwFshxn60684267;     mMndTjaxTJKwFshxn60684267 = mMndTjaxTJKwFshxn23935710;     mMndTjaxTJKwFshxn23935710 = mMndTjaxTJKwFshxn41627909;     mMndTjaxTJKwFshxn41627909 = mMndTjaxTJKwFshxn99345380;     mMndTjaxTJKwFshxn99345380 = mMndTjaxTJKwFshxn92069706;     mMndTjaxTJKwFshxn92069706 = mMndTjaxTJKwFshxn78502808;     mMndTjaxTJKwFshxn78502808 = mMndTjaxTJKwFshxn44335256;     mMndTjaxTJKwFshxn44335256 = mMndTjaxTJKwFshxn59425458;     mMndTjaxTJKwFshxn59425458 = mMndTjaxTJKwFshxn47571683;     mMndTjaxTJKwFshxn47571683 = mMndTjaxTJKwFshxn14598129;     mMndTjaxTJKwFshxn14598129 = mMndTjaxTJKwFshxn37342024;     mMndTjaxTJKwFshxn37342024 = mMndTjaxTJKwFshxn97833645;     mMndTjaxTJKwFshxn97833645 = mMndTjaxTJKwFshxn90709194;     mMndTjaxTJKwFshxn90709194 = mMndTjaxTJKwFshxn42978410;     mMndTjaxTJKwFshxn42978410 = mMndTjaxTJKwFshxn97956884;     mMndTjaxTJKwFshxn97956884 = mMndTjaxTJKwFshxn71795839;     mMndTjaxTJKwFshxn71795839 = mMndTjaxTJKwFshxn52061705;     mMndTjaxTJKwFshxn52061705 = mMndTjaxTJKwFshxn57437059;     mMndTjaxTJKwFshxn57437059 = mMndTjaxTJKwFshxn9029639;     mMndTjaxTJKwFshxn9029639 = mMndTjaxTJKwFshxn39144417;     mMndTjaxTJKwFshxn39144417 = mMndTjaxTJKwFshxn3639837;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LlnssogQECEFMYxo38037764() {     double HjIUSjjPCcntScCGt9756943 = -898231581;    double HjIUSjjPCcntScCGt71358577 = -249757891;    double HjIUSjjPCcntScCGt4116346 = -855877289;    double HjIUSjjPCcntScCGt16294407 = -401918763;    double HjIUSjjPCcntScCGt92178816 = -88538401;    double HjIUSjjPCcntScCGt65775952 = -732877781;    double HjIUSjjPCcntScCGt71955271 = 71523086;    double HjIUSjjPCcntScCGt42034598 = -63541419;    double HjIUSjjPCcntScCGt66271869 = -595921030;    double HjIUSjjPCcntScCGt5424559 = -102342930;    double HjIUSjjPCcntScCGt99840597 = -69891214;    double HjIUSjjPCcntScCGt69877972 = -230701667;    double HjIUSjjPCcntScCGt42073428 = -326640066;    double HjIUSjjPCcntScCGt63020709 = -193830922;    double HjIUSjjPCcntScCGt70017358 = -568225703;    double HjIUSjjPCcntScCGt34335311 = -853101584;    double HjIUSjjPCcntScCGt89629020 = -746184533;    double HjIUSjjPCcntScCGt68208446 = -855961449;    double HjIUSjjPCcntScCGt57254192 = -585880737;    double HjIUSjjPCcntScCGt65528323 = 91508830;    double HjIUSjjPCcntScCGt61338750 = -6113554;    double HjIUSjjPCcntScCGt13655324 = -785535143;    double HjIUSjjPCcntScCGt4018174 = -630821628;    double HjIUSjjPCcntScCGt57774876 = -406377355;    double HjIUSjjPCcntScCGt22461158 = -932833394;    double HjIUSjjPCcntScCGt85299239 = 32925311;    double HjIUSjjPCcntScCGt37983004 = -857580590;    double HjIUSjjPCcntScCGt3821833 = -818509163;    double HjIUSjjPCcntScCGt76374200 = -510957073;    double HjIUSjjPCcntScCGt3639999 = -590487161;    double HjIUSjjPCcntScCGt46260845 = 39602283;    double HjIUSjjPCcntScCGt15789864 = -207239939;    double HjIUSjjPCcntScCGt77213640 = -450506857;    double HjIUSjjPCcntScCGt92605385 = 61826884;    double HjIUSjjPCcntScCGt52578316 = -875771755;    double HjIUSjjPCcntScCGt65625853 = 93658945;    double HjIUSjjPCcntScCGt93387239 = -985298700;    double HjIUSjjPCcntScCGt35257525 = -2428875;    double HjIUSjjPCcntScCGt79294428 = -233235780;    double HjIUSjjPCcntScCGt20294346 = -992290904;    double HjIUSjjPCcntScCGt98216661 = -312231072;    double HjIUSjjPCcntScCGt66244924 = -905786964;    double HjIUSjjPCcntScCGt2109050 = -599236177;    double HjIUSjjPCcntScCGt6613983 = -622686978;    double HjIUSjjPCcntScCGt72647946 = -208865114;    double HjIUSjjPCcntScCGt1916007 = -688158386;    double HjIUSjjPCcntScCGt17671871 = -55856464;    double HjIUSjjPCcntScCGt23544458 = -250502044;    double HjIUSjjPCcntScCGt58762526 = -635210938;    double HjIUSjjPCcntScCGt23808217 = -129383041;    double HjIUSjjPCcntScCGt78655333 = -922900023;    double HjIUSjjPCcntScCGt86325207 = -37608755;    double HjIUSjjPCcntScCGt22092445 = -979870303;    double HjIUSjjPCcntScCGt4046114 = -469555868;    double HjIUSjjPCcntScCGt76699013 = 80682895;    double HjIUSjjPCcntScCGt96101618 = -12696439;    double HjIUSjjPCcntScCGt67340403 = -618936263;    double HjIUSjjPCcntScCGt46341469 = -349499935;    double HjIUSjjPCcntScCGt93833249 = -469085370;    double HjIUSjjPCcntScCGt6879578 = -21463713;    double HjIUSjjPCcntScCGt27792948 = -875297191;    double HjIUSjjPCcntScCGt68133439 = -109967752;    double HjIUSjjPCcntScCGt65660397 = -552584346;    double HjIUSjjPCcntScCGt62631871 = 94566130;    double HjIUSjjPCcntScCGt59163713 = -41945213;    double HjIUSjjPCcntScCGt84050733 = -862651276;    double HjIUSjjPCcntScCGt92664331 = -780194811;    double HjIUSjjPCcntScCGt49468042 = -288466950;    double HjIUSjjPCcntScCGt10442393 = -318059168;    double HjIUSjjPCcntScCGt4391505 = -561884648;    double HjIUSjjPCcntScCGt40948071 = -867802884;    double HjIUSjjPCcntScCGt54371495 = -643755659;    double HjIUSjjPCcntScCGt88914018 = -522725669;    double HjIUSjjPCcntScCGt36959846 = -593589834;    double HjIUSjjPCcntScCGt67311662 = -596260099;    double HjIUSjjPCcntScCGt95093825 = -100326590;    double HjIUSjjPCcntScCGt11546275 = -86298967;    double HjIUSjjPCcntScCGt97404191 = 91865349;    double HjIUSjjPCcntScCGt85126929 = -97512241;    double HjIUSjjPCcntScCGt20545152 = -144675009;    double HjIUSjjPCcntScCGt67627368 = -911218226;    double HjIUSjjPCcntScCGt14438547 = -507078547;    double HjIUSjjPCcntScCGt45059306 = -83298225;    double HjIUSjjPCcntScCGt52565984 = -281574033;    double HjIUSjjPCcntScCGt24984665 = -667587138;    double HjIUSjjPCcntScCGt59935638 = -922788962;    double HjIUSjjPCcntScCGt93697419 = -227369637;    double HjIUSjjPCcntScCGt73167526 = -980950989;    double HjIUSjjPCcntScCGt15906373 = 81143988;    double HjIUSjjPCcntScCGt56476697 = -763075317;    double HjIUSjjPCcntScCGt98285449 = -287404793;    double HjIUSjjPCcntScCGt47045770 = -535798765;    double HjIUSjjPCcntScCGt41424276 = -533343505;    double HjIUSjjPCcntScCGt72414851 = -111772068;    double HjIUSjjPCcntScCGt92501397 = -16993714;    double HjIUSjjPCcntScCGt30083222 = -102263320;    double HjIUSjjPCcntScCGt584528 = -253202618;    double HjIUSjjPCcntScCGt39477178 = -593802308;    double HjIUSjjPCcntScCGt47450269 = -480741765;    double HjIUSjjPCcntScCGt88597213 = -898231581;     HjIUSjjPCcntScCGt9756943 = HjIUSjjPCcntScCGt71358577;     HjIUSjjPCcntScCGt71358577 = HjIUSjjPCcntScCGt4116346;     HjIUSjjPCcntScCGt4116346 = HjIUSjjPCcntScCGt16294407;     HjIUSjjPCcntScCGt16294407 = HjIUSjjPCcntScCGt92178816;     HjIUSjjPCcntScCGt92178816 = HjIUSjjPCcntScCGt65775952;     HjIUSjjPCcntScCGt65775952 = HjIUSjjPCcntScCGt71955271;     HjIUSjjPCcntScCGt71955271 = HjIUSjjPCcntScCGt42034598;     HjIUSjjPCcntScCGt42034598 = HjIUSjjPCcntScCGt66271869;     HjIUSjjPCcntScCGt66271869 = HjIUSjjPCcntScCGt5424559;     HjIUSjjPCcntScCGt5424559 = HjIUSjjPCcntScCGt99840597;     HjIUSjjPCcntScCGt99840597 = HjIUSjjPCcntScCGt69877972;     HjIUSjjPCcntScCGt69877972 = HjIUSjjPCcntScCGt42073428;     HjIUSjjPCcntScCGt42073428 = HjIUSjjPCcntScCGt63020709;     HjIUSjjPCcntScCGt63020709 = HjIUSjjPCcntScCGt70017358;     HjIUSjjPCcntScCGt70017358 = HjIUSjjPCcntScCGt34335311;     HjIUSjjPCcntScCGt34335311 = HjIUSjjPCcntScCGt89629020;     HjIUSjjPCcntScCGt89629020 = HjIUSjjPCcntScCGt68208446;     HjIUSjjPCcntScCGt68208446 = HjIUSjjPCcntScCGt57254192;     HjIUSjjPCcntScCGt57254192 = HjIUSjjPCcntScCGt65528323;     HjIUSjjPCcntScCGt65528323 = HjIUSjjPCcntScCGt61338750;     HjIUSjjPCcntScCGt61338750 = HjIUSjjPCcntScCGt13655324;     HjIUSjjPCcntScCGt13655324 = HjIUSjjPCcntScCGt4018174;     HjIUSjjPCcntScCGt4018174 = HjIUSjjPCcntScCGt57774876;     HjIUSjjPCcntScCGt57774876 = HjIUSjjPCcntScCGt22461158;     HjIUSjjPCcntScCGt22461158 = HjIUSjjPCcntScCGt85299239;     HjIUSjjPCcntScCGt85299239 = HjIUSjjPCcntScCGt37983004;     HjIUSjjPCcntScCGt37983004 = HjIUSjjPCcntScCGt3821833;     HjIUSjjPCcntScCGt3821833 = HjIUSjjPCcntScCGt76374200;     HjIUSjjPCcntScCGt76374200 = HjIUSjjPCcntScCGt3639999;     HjIUSjjPCcntScCGt3639999 = HjIUSjjPCcntScCGt46260845;     HjIUSjjPCcntScCGt46260845 = HjIUSjjPCcntScCGt15789864;     HjIUSjjPCcntScCGt15789864 = HjIUSjjPCcntScCGt77213640;     HjIUSjjPCcntScCGt77213640 = HjIUSjjPCcntScCGt92605385;     HjIUSjjPCcntScCGt92605385 = HjIUSjjPCcntScCGt52578316;     HjIUSjjPCcntScCGt52578316 = HjIUSjjPCcntScCGt65625853;     HjIUSjjPCcntScCGt65625853 = HjIUSjjPCcntScCGt93387239;     HjIUSjjPCcntScCGt93387239 = HjIUSjjPCcntScCGt35257525;     HjIUSjjPCcntScCGt35257525 = HjIUSjjPCcntScCGt79294428;     HjIUSjjPCcntScCGt79294428 = HjIUSjjPCcntScCGt20294346;     HjIUSjjPCcntScCGt20294346 = HjIUSjjPCcntScCGt98216661;     HjIUSjjPCcntScCGt98216661 = HjIUSjjPCcntScCGt66244924;     HjIUSjjPCcntScCGt66244924 = HjIUSjjPCcntScCGt2109050;     HjIUSjjPCcntScCGt2109050 = HjIUSjjPCcntScCGt6613983;     HjIUSjjPCcntScCGt6613983 = HjIUSjjPCcntScCGt72647946;     HjIUSjjPCcntScCGt72647946 = HjIUSjjPCcntScCGt1916007;     HjIUSjjPCcntScCGt1916007 = HjIUSjjPCcntScCGt17671871;     HjIUSjjPCcntScCGt17671871 = HjIUSjjPCcntScCGt23544458;     HjIUSjjPCcntScCGt23544458 = HjIUSjjPCcntScCGt58762526;     HjIUSjjPCcntScCGt58762526 = HjIUSjjPCcntScCGt23808217;     HjIUSjjPCcntScCGt23808217 = HjIUSjjPCcntScCGt78655333;     HjIUSjjPCcntScCGt78655333 = HjIUSjjPCcntScCGt86325207;     HjIUSjjPCcntScCGt86325207 = HjIUSjjPCcntScCGt22092445;     HjIUSjjPCcntScCGt22092445 = HjIUSjjPCcntScCGt4046114;     HjIUSjjPCcntScCGt4046114 = HjIUSjjPCcntScCGt76699013;     HjIUSjjPCcntScCGt76699013 = HjIUSjjPCcntScCGt96101618;     HjIUSjjPCcntScCGt96101618 = HjIUSjjPCcntScCGt67340403;     HjIUSjjPCcntScCGt67340403 = HjIUSjjPCcntScCGt46341469;     HjIUSjjPCcntScCGt46341469 = HjIUSjjPCcntScCGt93833249;     HjIUSjjPCcntScCGt93833249 = HjIUSjjPCcntScCGt6879578;     HjIUSjjPCcntScCGt6879578 = HjIUSjjPCcntScCGt27792948;     HjIUSjjPCcntScCGt27792948 = HjIUSjjPCcntScCGt68133439;     HjIUSjjPCcntScCGt68133439 = HjIUSjjPCcntScCGt65660397;     HjIUSjjPCcntScCGt65660397 = HjIUSjjPCcntScCGt62631871;     HjIUSjjPCcntScCGt62631871 = HjIUSjjPCcntScCGt59163713;     HjIUSjjPCcntScCGt59163713 = HjIUSjjPCcntScCGt84050733;     HjIUSjjPCcntScCGt84050733 = HjIUSjjPCcntScCGt92664331;     HjIUSjjPCcntScCGt92664331 = HjIUSjjPCcntScCGt49468042;     HjIUSjjPCcntScCGt49468042 = HjIUSjjPCcntScCGt10442393;     HjIUSjjPCcntScCGt10442393 = HjIUSjjPCcntScCGt4391505;     HjIUSjjPCcntScCGt4391505 = HjIUSjjPCcntScCGt40948071;     HjIUSjjPCcntScCGt40948071 = HjIUSjjPCcntScCGt54371495;     HjIUSjjPCcntScCGt54371495 = HjIUSjjPCcntScCGt88914018;     HjIUSjjPCcntScCGt88914018 = HjIUSjjPCcntScCGt36959846;     HjIUSjjPCcntScCGt36959846 = HjIUSjjPCcntScCGt67311662;     HjIUSjjPCcntScCGt67311662 = HjIUSjjPCcntScCGt95093825;     HjIUSjjPCcntScCGt95093825 = HjIUSjjPCcntScCGt11546275;     HjIUSjjPCcntScCGt11546275 = HjIUSjjPCcntScCGt97404191;     HjIUSjjPCcntScCGt97404191 = HjIUSjjPCcntScCGt85126929;     HjIUSjjPCcntScCGt85126929 = HjIUSjjPCcntScCGt20545152;     HjIUSjjPCcntScCGt20545152 = HjIUSjjPCcntScCGt67627368;     HjIUSjjPCcntScCGt67627368 = HjIUSjjPCcntScCGt14438547;     HjIUSjjPCcntScCGt14438547 = HjIUSjjPCcntScCGt45059306;     HjIUSjjPCcntScCGt45059306 = HjIUSjjPCcntScCGt52565984;     HjIUSjjPCcntScCGt52565984 = HjIUSjjPCcntScCGt24984665;     HjIUSjjPCcntScCGt24984665 = HjIUSjjPCcntScCGt59935638;     HjIUSjjPCcntScCGt59935638 = HjIUSjjPCcntScCGt93697419;     HjIUSjjPCcntScCGt93697419 = HjIUSjjPCcntScCGt73167526;     HjIUSjjPCcntScCGt73167526 = HjIUSjjPCcntScCGt15906373;     HjIUSjjPCcntScCGt15906373 = HjIUSjjPCcntScCGt56476697;     HjIUSjjPCcntScCGt56476697 = HjIUSjjPCcntScCGt98285449;     HjIUSjjPCcntScCGt98285449 = HjIUSjjPCcntScCGt47045770;     HjIUSjjPCcntScCGt47045770 = HjIUSjjPCcntScCGt41424276;     HjIUSjjPCcntScCGt41424276 = HjIUSjjPCcntScCGt72414851;     HjIUSjjPCcntScCGt72414851 = HjIUSjjPCcntScCGt92501397;     HjIUSjjPCcntScCGt92501397 = HjIUSjjPCcntScCGt30083222;     HjIUSjjPCcntScCGt30083222 = HjIUSjjPCcntScCGt584528;     HjIUSjjPCcntScCGt584528 = HjIUSjjPCcntScCGt39477178;     HjIUSjjPCcntScCGt39477178 = HjIUSjjPCcntScCGt47450269;     HjIUSjjPCcntScCGt47450269 = HjIUSjjPCcntScCGt88597213;     HjIUSjjPCcntScCGt88597213 = HjIUSjjPCcntScCGt9756943;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void nLVLeTCQiWbWCOmh5329363() {     double VYkmtaVHjxrZHflNE45215911 = -264508481;    double VYkmtaVHjxrZHflNE84630656 = -130309498;    double VYkmtaVHjxrZHflNE41502822 = -521544364;    double VYkmtaVHjxrZHflNE88517761 = -223069834;    double VYkmtaVHjxrZHflNE48464086 = -194137247;    double VYkmtaVHjxrZHflNE1998824 = -995225676;    double VYkmtaVHjxrZHflNE82612677 = -873202648;    double VYkmtaVHjxrZHflNE4166159 = -807775559;    double VYkmtaVHjxrZHflNE67789474 = 15747563;    double VYkmtaVHjxrZHflNE20327271 = -867029928;    double VYkmtaVHjxrZHflNE38022332 = -971898944;    double VYkmtaVHjxrZHflNE70446121 = -94822439;    double VYkmtaVHjxrZHflNE80308073 = -878652571;    double VYkmtaVHjxrZHflNE16161573 = -170194332;    double VYkmtaVHjxrZHflNE28211875 = 28349282;    double VYkmtaVHjxrZHflNE17876612 = -488913191;    double VYkmtaVHjxrZHflNE45464956 = -173159086;    double VYkmtaVHjxrZHflNE67968638 = -929389517;    double VYkmtaVHjxrZHflNE57804522 = -902444159;    double VYkmtaVHjxrZHflNE59365542 = -335052236;    double VYkmtaVHjxrZHflNE74425613 = -763388242;    double VYkmtaVHjxrZHflNE17076849 = -872171437;    double VYkmtaVHjxrZHflNE74543554 = -268453813;    double VYkmtaVHjxrZHflNE51567821 = -917201959;    double VYkmtaVHjxrZHflNE30479109 = -956818592;    double VYkmtaVHjxrZHflNE95997410 = -755651496;    double VYkmtaVHjxrZHflNE98403577 = -663864731;    double VYkmtaVHjxrZHflNE44550257 = -988281337;    double VYkmtaVHjxrZHflNE23042415 = 14948125;    double VYkmtaVHjxrZHflNE9173410 = -624994332;    double VYkmtaVHjxrZHflNE64844077 = -512378204;    double VYkmtaVHjxrZHflNE53612883 = 19788646;    double VYkmtaVHjxrZHflNE89431651 = -598102834;    double VYkmtaVHjxrZHflNE22353813 = -2866930;    double VYkmtaVHjxrZHflNE18386770 = -826872006;    double VYkmtaVHjxrZHflNE17186697 = -120906361;    double VYkmtaVHjxrZHflNE42799056 = -849994740;    double VYkmtaVHjxrZHflNE86472928 = 14915544;    double VYkmtaVHjxrZHflNE9684990 = -984112057;    double VYkmtaVHjxrZHflNE84565878 = -457553450;    double VYkmtaVHjxrZHflNE31668269 = -81429986;    double VYkmtaVHjxrZHflNE55610051 = -500332254;    double VYkmtaVHjxrZHflNE91492427 = -152416963;    double VYkmtaVHjxrZHflNE46745321 = -127019388;    double VYkmtaVHjxrZHflNE72664405 = -35376275;    double VYkmtaVHjxrZHflNE47877335 = -727082018;    double VYkmtaVHjxrZHflNE37017034 = -539842825;    double VYkmtaVHjxrZHflNE41270769 = -417028637;    double VYkmtaVHjxrZHflNE81655192 = -298947819;    double VYkmtaVHjxrZHflNE48337806 = -620409856;    double VYkmtaVHjxrZHflNE59426057 = -797176581;    double VYkmtaVHjxrZHflNE69810680 = -251261102;    double VYkmtaVHjxrZHflNE90713592 = -735915857;    double VYkmtaVHjxrZHflNE20037961 = -661064368;    double VYkmtaVHjxrZHflNE30903750 = -265807251;    double VYkmtaVHjxrZHflNE28139063 = -392337044;    double VYkmtaVHjxrZHflNE10087103 = -861855685;    double VYkmtaVHjxrZHflNE89935000 = -604342406;    double VYkmtaVHjxrZHflNE58038653 = -266251243;    double VYkmtaVHjxrZHflNE52466675 = -438485751;    double VYkmtaVHjxrZHflNE3595247 = -231360946;    double VYkmtaVHjxrZHflNE38062421 = -884921312;    double VYkmtaVHjxrZHflNE81123743 = -722723684;    double VYkmtaVHjxrZHflNE58616065 = -359258106;    double VYkmtaVHjxrZHflNE55483194 = -254651724;    double VYkmtaVHjxrZHflNE84409449 = -891687590;    double VYkmtaVHjxrZHflNE81014469 = -496719605;    double VYkmtaVHjxrZHflNE57954261 = -775785641;    double VYkmtaVHjxrZHflNE97774802 = -343322327;    double VYkmtaVHjxrZHflNE11025178 = -850744357;    double VYkmtaVHjxrZHflNE75077555 = -638918451;    double VYkmtaVHjxrZHflNE58992027 = -88074631;    double VYkmtaVHjxrZHflNE58283648 = -945277461;    double VYkmtaVHjxrZHflNE73238643 = -344890709;    double VYkmtaVHjxrZHflNE27697273 = -153622251;    double VYkmtaVHjxrZHflNE18815562 = -163055988;    double VYkmtaVHjxrZHflNE25584421 = -619754475;    double VYkmtaVHjxrZHflNE27798233 = -41434425;    double VYkmtaVHjxrZHflNE78903416 = -781825684;    double VYkmtaVHjxrZHflNE82601774 = -129736575;    double VYkmtaVHjxrZHflNE58980376 = -115808672;    double VYkmtaVHjxrZHflNE57132809 = -146836095;    double VYkmtaVHjxrZHflNE62895064 = -589333519;    double VYkmtaVHjxrZHflNE74704609 = -364642019;    double VYkmtaVHjxrZHflNE49747352 = -827817752;    double VYkmtaVHjxrZHflNE95033397 = -161117103;    double VYkmtaVHjxrZHflNE62899290 = -244295498;    double VYkmtaVHjxrZHflNE69393691 = -937038467;    double VYkmtaVHjxrZHflNE91450062 = -737059679;    double VYkmtaVHjxrZHflNE90247707 = -334534962;    double VYkmtaVHjxrZHflNE7099595 = -259050676;    double VYkmtaVHjxrZHflNE52864055 = -145652334;    double VYkmtaVHjxrZHflNE28434276 = -718833214;    double VYkmtaVHjxrZHflNE57218315 = -445626306;    double VYkmtaVHjxrZHflNE80970632 = -126192505;    double VYkmtaVHjxrZHflNE93605848 = -196508675;    double VYkmtaVHjxrZHflNE74486307 = -777608571;    double VYkmtaVHjxrZHflNE32876363 = -793158858;    double VYkmtaVHjxrZHflNE91262127 = -872367664;    double VYkmtaVHjxrZHflNE88254955 = -264508481;     VYkmtaVHjxrZHflNE45215911 = VYkmtaVHjxrZHflNE84630656;     VYkmtaVHjxrZHflNE84630656 = VYkmtaVHjxrZHflNE41502822;     VYkmtaVHjxrZHflNE41502822 = VYkmtaVHjxrZHflNE88517761;     VYkmtaVHjxrZHflNE88517761 = VYkmtaVHjxrZHflNE48464086;     VYkmtaVHjxrZHflNE48464086 = VYkmtaVHjxrZHflNE1998824;     VYkmtaVHjxrZHflNE1998824 = VYkmtaVHjxrZHflNE82612677;     VYkmtaVHjxrZHflNE82612677 = VYkmtaVHjxrZHflNE4166159;     VYkmtaVHjxrZHflNE4166159 = VYkmtaVHjxrZHflNE67789474;     VYkmtaVHjxrZHflNE67789474 = VYkmtaVHjxrZHflNE20327271;     VYkmtaVHjxrZHflNE20327271 = VYkmtaVHjxrZHflNE38022332;     VYkmtaVHjxrZHflNE38022332 = VYkmtaVHjxrZHflNE70446121;     VYkmtaVHjxrZHflNE70446121 = VYkmtaVHjxrZHflNE80308073;     VYkmtaVHjxrZHflNE80308073 = VYkmtaVHjxrZHflNE16161573;     VYkmtaVHjxrZHflNE16161573 = VYkmtaVHjxrZHflNE28211875;     VYkmtaVHjxrZHflNE28211875 = VYkmtaVHjxrZHflNE17876612;     VYkmtaVHjxrZHflNE17876612 = VYkmtaVHjxrZHflNE45464956;     VYkmtaVHjxrZHflNE45464956 = VYkmtaVHjxrZHflNE67968638;     VYkmtaVHjxrZHflNE67968638 = VYkmtaVHjxrZHflNE57804522;     VYkmtaVHjxrZHflNE57804522 = VYkmtaVHjxrZHflNE59365542;     VYkmtaVHjxrZHflNE59365542 = VYkmtaVHjxrZHflNE74425613;     VYkmtaVHjxrZHflNE74425613 = VYkmtaVHjxrZHflNE17076849;     VYkmtaVHjxrZHflNE17076849 = VYkmtaVHjxrZHflNE74543554;     VYkmtaVHjxrZHflNE74543554 = VYkmtaVHjxrZHflNE51567821;     VYkmtaVHjxrZHflNE51567821 = VYkmtaVHjxrZHflNE30479109;     VYkmtaVHjxrZHflNE30479109 = VYkmtaVHjxrZHflNE95997410;     VYkmtaVHjxrZHflNE95997410 = VYkmtaVHjxrZHflNE98403577;     VYkmtaVHjxrZHflNE98403577 = VYkmtaVHjxrZHflNE44550257;     VYkmtaVHjxrZHflNE44550257 = VYkmtaVHjxrZHflNE23042415;     VYkmtaVHjxrZHflNE23042415 = VYkmtaVHjxrZHflNE9173410;     VYkmtaVHjxrZHflNE9173410 = VYkmtaVHjxrZHflNE64844077;     VYkmtaVHjxrZHflNE64844077 = VYkmtaVHjxrZHflNE53612883;     VYkmtaVHjxrZHflNE53612883 = VYkmtaVHjxrZHflNE89431651;     VYkmtaVHjxrZHflNE89431651 = VYkmtaVHjxrZHflNE22353813;     VYkmtaVHjxrZHflNE22353813 = VYkmtaVHjxrZHflNE18386770;     VYkmtaVHjxrZHflNE18386770 = VYkmtaVHjxrZHflNE17186697;     VYkmtaVHjxrZHflNE17186697 = VYkmtaVHjxrZHflNE42799056;     VYkmtaVHjxrZHflNE42799056 = VYkmtaVHjxrZHflNE86472928;     VYkmtaVHjxrZHflNE86472928 = VYkmtaVHjxrZHflNE9684990;     VYkmtaVHjxrZHflNE9684990 = VYkmtaVHjxrZHflNE84565878;     VYkmtaVHjxrZHflNE84565878 = VYkmtaVHjxrZHflNE31668269;     VYkmtaVHjxrZHflNE31668269 = VYkmtaVHjxrZHflNE55610051;     VYkmtaVHjxrZHflNE55610051 = VYkmtaVHjxrZHflNE91492427;     VYkmtaVHjxrZHflNE91492427 = VYkmtaVHjxrZHflNE46745321;     VYkmtaVHjxrZHflNE46745321 = VYkmtaVHjxrZHflNE72664405;     VYkmtaVHjxrZHflNE72664405 = VYkmtaVHjxrZHflNE47877335;     VYkmtaVHjxrZHflNE47877335 = VYkmtaVHjxrZHflNE37017034;     VYkmtaVHjxrZHflNE37017034 = VYkmtaVHjxrZHflNE41270769;     VYkmtaVHjxrZHflNE41270769 = VYkmtaVHjxrZHflNE81655192;     VYkmtaVHjxrZHflNE81655192 = VYkmtaVHjxrZHflNE48337806;     VYkmtaVHjxrZHflNE48337806 = VYkmtaVHjxrZHflNE59426057;     VYkmtaVHjxrZHflNE59426057 = VYkmtaVHjxrZHflNE69810680;     VYkmtaVHjxrZHflNE69810680 = VYkmtaVHjxrZHflNE90713592;     VYkmtaVHjxrZHflNE90713592 = VYkmtaVHjxrZHflNE20037961;     VYkmtaVHjxrZHflNE20037961 = VYkmtaVHjxrZHflNE30903750;     VYkmtaVHjxrZHflNE30903750 = VYkmtaVHjxrZHflNE28139063;     VYkmtaVHjxrZHflNE28139063 = VYkmtaVHjxrZHflNE10087103;     VYkmtaVHjxrZHflNE10087103 = VYkmtaVHjxrZHflNE89935000;     VYkmtaVHjxrZHflNE89935000 = VYkmtaVHjxrZHflNE58038653;     VYkmtaVHjxrZHflNE58038653 = VYkmtaVHjxrZHflNE52466675;     VYkmtaVHjxrZHflNE52466675 = VYkmtaVHjxrZHflNE3595247;     VYkmtaVHjxrZHflNE3595247 = VYkmtaVHjxrZHflNE38062421;     VYkmtaVHjxrZHflNE38062421 = VYkmtaVHjxrZHflNE81123743;     VYkmtaVHjxrZHflNE81123743 = VYkmtaVHjxrZHflNE58616065;     VYkmtaVHjxrZHflNE58616065 = VYkmtaVHjxrZHflNE55483194;     VYkmtaVHjxrZHflNE55483194 = VYkmtaVHjxrZHflNE84409449;     VYkmtaVHjxrZHflNE84409449 = VYkmtaVHjxrZHflNE81014469;     VYkmtaVHjxrZHflNE81014469 = VYkmtaVHjxrZHflNE57954261;     VYkmtaVHjxrZHflNE57954261 = VYkmtaVHjxrZHflNE97774802;     VYkmtaVHjxrZHflNE97774802 = VYkmtaVHjxrZHflNE11025178;     VYkmtaVHjxrZHflNE11025178 = VYkmtaVHjxrZHflNE75077555;     VYkmtaVHjxrZHflNE75077555 = VYkmtaVHjxrZHflNE58992027;     VYkmtaVHjxrZHflNE58992027 = VYkmtaVHjxrZHflNE58283648;     VYkmtaVHjxrZHflNE58283648 = VYkmtaVHjxrZHflNE73238643;     VYkmtaVHjxrZHflNE73238643 = VYkmtaVHjxrZHflNE27697273;     VYkmtaVHjxrZHflNE27697273 = VYkmtaVHjxrZHflNE18815562;     VYkmtaVHjxrZHflNE18815562 = VYkmtaVHjxrZHflNE25584421;     VYkmtaVHjxrZHflNE25584421 = VYkmtaVHjxrZHflNE27798233;     VYkmtaVHjxrZHflNE27798233 = VYkmtaVHjxrZHflNE78903416;     VYkmtaVHjxrZHflNE78903416 = VYkmtaVHjxrZHflNE82601774;     VYkmtaVHjxrZHflNE82601774 = VYkmtaVHjxrZHflNE58980376;     VYkmtaVHjxrZHflNE58980376 = VYkmtaVHjxrZHflNE57132809;     VYkmtaVHjxrZHflNE57132809 = VYkmtaVHjxrZHflNE62895064;     VYkmtaVHjxrZHflNE62895064 = VYkmtaVHjxrZHflNE74704609;     VYkmtaVHjxrZHflNE74704609 = VYkmtaVHjxrZHflNE49747352;     VYkmtaVHjxrZHflNE49747352 = VYkmtaVHjxrZHflNE95033397;     VYkmtaVHjxrZHflNE95033397 = VYkmtaVHjxrZHflNE62899290;     VYkmtaVHjxrZHflNE62899290 = VYkmtaVHjxrZHflNE69393691;     VYkmtaVHjxrZHflNE69393691 = VYkmtaVHjxrZHflNE91450062;     VYkmtaVHjxrZHflNE91450062 = VYkmtaVHjxrZHflNE90247707;     VYkmtaVHjxrZHflNE90247707 = VYkmtaVHjxrZHflNE7099595;     VYkmtaVHjxrZHflNE7099595 = VYkmtaVHjxrZHflNE52864055;     VYkmtaVHjxrZHflNE52864055 = VYkmtaVHjxrZHflNE28434276;     VYkmtaVHjxrZHflNE28434276 = VYkmtaVHjxrZHflNE57218315;     VYkmtaVHjxrZHflNE57218315 = VYkmtaVHjxrZHflNE80970632;     VYkmtaVHjxrZHflNE80970632 = VYkmtaVHjxrZHflNE93605848;     VYkmtaVHjxrZHflNE93605848 = VYkmtaVHjxrZHflNE74486307;     VYkmtaVHjxrZHflNE74486307 = VYkmtaVHjxrZHflNE32876363;     VYkmtaVHjxrZHflNE32876363 = VYkmtaVHjxrZHflNE91262127;     VYkmtaVHjxrZHflNE91262127 = VYkmtaVHjxrZHflNE88254955;     VYkmtaVHjxrZHflNE88254955 = VYkmtaVHjxrZHflNE45215911;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DIEYhZdLtGyrWHqP20378431() {     double DxrXBxYOOBduXWYgf51333018 = -376410370;    double DxrXBxYOOBduXWYgf28003936 = -939909278;    double DxrXBxYOOBduXWYgf57545006 = -672377285;    double DxrXBxYOOBduXWYgf15592543 = -413818465;    double DxrXBxYOOBduXWYgf17359575 = -714787118;    double DxrXBxYOOBduXWYgf93416026 = -159721911;    double DxrXBxYOOBduXWYgf43101644 = 87832375;    double DxrXBxYOOBduXWYgf53926049 = 5350528;    double DxrXBxYOOBduXWYgf32431970 = -852285103;    double DxrXBxYOOBduXWYgf61556930 = -96005405;    double DxrXBxYOOBduXWYgf98132622 = -849336530;    double DxrXBxYOOBduXWYgf2269164 = -539228641;    double DxrXBxYOOBduXWYgf66517192 = -218543659;    double DxrXBxYOOBduXWYgf99305248 = -702432900;    double DxrXBxYOOBduXWYgf23954302 = -662112884;    double DxrXBxYOOBduXWYgf59382005 = -461912734;    double DxrXBxYOOBduXWYgf39300959 = -789704199;    double DxrXBxYOOBduXWYgf65021791 = -594912160;    double DxrXBxYOOBduXWYgf19634364 = -208213256;    double DxrXBxYOOBduXWYgf39915707 = -691955156;    double DxrXBxYOOBduXWYgf62978806 = -618875698;    double DxrXBxYOOBduXWYgf3597873 = -780101154;    double DxrXBxYOOBduXWYgf10134346 = -14971299;    double DxrXBxYOOBduXWYgf35560195 = -39506947;    double DxrXBxYOOBduXWYgf74178759 = -230014625;    double DxrXBxYOOBduXWYgf20719086 = -634306819;    double DxrXBxYOOBduXWYgf96863356 = -564526934;    double DxrXBxYOOBduXWYgf57657382 = 45789709;    double DxrXBxYOOBduXWYgf89174670 = -207434580;    double DxrXBxYOOBduXWYgf77789046 = -442465199;    double DxrXBxYOOBduXWYgf57086626 = -425699142;    double DxrXBxYOOBduXWYgf51223504 = -789658170;    double DxrXBxYOOBduXWYgf99142600 = -490716205;    double DxrXBxYOOBduXWYgf17184377 = -854408695;    double DxrXBxYOOBduXWYgf79861422 = -365605111;    double DxrXBxYOOBduXWYgf85912610 = -334932469;    double DxrXBxYOOBduXWYgf24060991 = -803630610;    double DxrXBxYOOBduXWYgf20563141 = -856590466;    double DxrXBxYOOBduXWYgf83295243 = -978035554;    double DxrXBxYOOBduXWYgf72067816 = -930992673;    double DxrXBxYOOBduXWYgf37337493 = -146703880;    double DxrXBxYOOBduXWYgf87760508 = -770474533;    double DxrXBxYOOBduXWYgf69559406 = -233221547;    double DxrXBxYOOBduXWYgf34153061 = -146362326;    double DxrXBxYOOBduXWYgf84616870 = -797274430;    double DxrXBxYOOBduXWYgf59816530 = -712298134;    double DxrXBxYOOBduXWYgf54795608 = -658618362;    double DxrXBxYOOBduXWYgf49227710 = -426924169;    double DxrXBxYOOBduXWYgf91330921 = -384684807;    double DxrXBxYOOBduXWYgf61249458 = -707420664;    double DxrXBxYOOBduXWYgf95126735 = -639470252;    double DxrXBxYOOBduXWYgf80620399 = -51987422;    double DxrXBxYOOBduXWYgf38962050 = -527705444;    double DxrXBxYOOBduXWYgf16006841 = -981534513;    double DxrXBxYOOBduXWYgf57399625 = -966339033;    double DxrXBxYOOBduXWYgf47735146 = -596309217;    double DxrXBxYOOBduXWYgf17869590 = -824937979;    double DxrXBxYOOBduXWYgf21984811 = -532870339;    double DxrXBxYOOBduXWYgf41413784 = -83803840;    double DxrXBxYOOBduXWYgf96640488 = 19519701;    double DxrXBxYOOBduXWYgf96552670 = -595194978;    double DxrXBxYOOBduXWYgf85444261 = -957957335;    double DxrXBxYOOBduXWYgf64751379 = -787214893;    double DxrXBxYOOBduXWYgf54642924 = -309819904;    double DxrXBxYOOBduXWYgf4470304 = -670306263;    double DxrXBxYOOBduXWYgf46909118 = 40321639;    double DxrXBxYOOBduXWYgf3126564 = 51487563;    double DxrXBxYOOBduXWYgf49332816 = -364134965;    double DxrXBxYOOBduXWYgf19443827 = -236827789;    double DxrXBxYOOBduXWYgf38041691 = -227180416;    double DxrXBxYOOBduXWYgf35321015 = -658282125;    double DxrXBxYOOBduXWYgf18737818 = -933113734;    double DxrXBxYOOBduXWYgf81726547 = -616876607;    double DxrXBxYOOBduXWYgf47566548 = -277220583;    double DxrXBxYOOBduXWYgf2578214 = -445251276;    double DxrXBxYOOBduXWYgf75218298 = -848401166;    double DxrXBxYOOBduXWYgf34038467 = -446879607;    double DxrXBxYOOBduXWYgf75981284 = -868608974;    double DxrXBxYOOBduXWYgf50943325 = -242232517;    double DxrXBxYOOBduXWYgf14362229 = -517716491;    double DxrXBxYOOBduXWYgf65923477 = -975688457;    double DxrXBxYOOBduXWYgf47635646 = -37602765;    double DxrXBxYOOBduXWYgf66326461 = -569525484;    double DxrXBxYOOBduXWYgf27925212 = -500013916;    double DxrXBxYOOBduXWYgf82662310 = -802994948;    double DxrXBxYOOBduXWYgf76466227 = -273711721;    double DxrXBxYOOBduXWYgf12261455 = -161952726;    double DxrXBxYOOBduXWYgf83135759 = -509181692;    double DxrXBxYOOBduXWYgf59784752 = -888069662;    double DxrXBxYOOBduXWYgf32126277 = -769295895;    double DxrXBxYOOBduXWYgf68043020 = -509994490;    double DxrXBxYOOBduXWYgf2076180 = -170760271;    double DxrXBxYOOBduXWYgf79149357 = -672786626;    double DxrXBxYOOBduXWYgf86654755 = -897555255;    double DxrXBxYOOBduXWYgf75515145 = -235797696;    double DxrXBxYOOBduXWYgf51893232 = -188746545;    double DxrXBxYOOBduXWYgf23009129 = -983259640;    double DxrXBxYOOBduXWYgf14916482 = -923401643;    double DxrXBxYOOBduXWYgf29682758 = -476056063;    double DxrXBxYOOBduXWYgf37707752 = -376410370;     DxrXBxYOOBduXWYgf51333018 = DxrXBxYOOBduXWYgf28003936;     DxrXBxYOOBduXWYgf28003936 = DxrXBxYOOBduXWYgf57545006;     DxrXBxYOOBduXWYgf57545006 = DxrXBxYOOBduXWYgf15592543;     DxrXBxYOOBduXWYgf15592543 = DxrXBxYOOBduXWYgf17359575;     DxrXBxYOOBduXWYgf17359575 = DxrXBxYOOBduXWYgf93416026;     DxrXBxYOOBduXWYgf93416026 = DxrXBxYOOBduXWYgf43101644;     DxrXBxYOOBduXWYgf43101644 = DxrXBxYOOBduXWYgf53926049;     DxrXBxYOOBduXWYgf53926049 = DxrXBxYOOBduXWYgf32431970;     DxrXBxYOOBduXWYgf32431970 = DxrXBxYOOBduXWYgf61556930;     DxrXBxYOOBduXWYgf61556930 = DxrXBxYOOBduXWYgf98132622;     DxrXBxYOOBduXWYgf98132622 = DxrXBxYOOBduXWYgf2269164;     DxrXBxYOOBduXWYgf2269164 = DxrXBxYOOBduXWYgf66517192;     DxrXBxYOOBduXWYgf66517192 = DxrXBxYOOBduXWYgf99305248;     DxrXBxYOOBduXWYgf99305248 = DxrXBxYOOBduXWYgf23954302;     DxrXBxYOOBduXWYgf23954302 = DxrXBxYOOBduXWYgf59382005;     DxrXBxYOOBduXWYgf59382005 = DxrXBxYOOBduXWYgf39300959;     DxrXBxYOOBduXWYgf39300959 = DxrXBxYOOBduXWYgf65021791;     DxrXBxYOOBduXWYgf65021791 = DxrXBxYOOBduXWYgf19634364;     DxrXBxYOOBduXWYgf19634364 = DxrXBxYOOBduXWYgf39915707;     DxrXBxYOOBduXWYgf39915707 = DxrXBxYOOBduXWYgf62978806;     DxrXBxYOOBduXWYgf62978806 = DxrXBxYOOBduXWYgf3597873;     DxrXBxYOOBduXWYgf3597873 = DxrXBxYOOBduXWYgf10134346;     DxrXBxYOOBduXWYgf10134346 = DxrXBxYOOBduXWYgf35560195;     DxrXBxYOOBduXWYgf35560195 = DxrXBxYOOBduXWYgf74178759;     DxrXBxYOOBduXWYgf74178759 = DxrXBxYOOBduXWYgf20719086;     DxrXBxYOOBduXWYgf20719086 = DxrXBxYOOBduXWYgf96863356;     DxrXBxYOOBduXWYgf96863356 = DxrXBxYOOBduXWYgf57657382;     DxrXBxYOOBduXWYgf57657382 = DxrXBxYOOBduXWYgf89174670;     DxrXBxYOOBduXWYgf89174670 = DxrXBxYOOBduXWYgf77789046;     DxrXBxYOOBduXWYgf77789046 = DxrXBxYOOBduXWYgf57086626;     DxrXBxYOOBduXWYgf57086626 = DxrXBxYOOBduXWYgf51223504;     DxrXBxYOOBduXWYgf51223504 = DxrXBxYOOBduXWYgf99142600;     DxrXBxYOOBduXWYgf99142600 = DxrXBxYOOBduXWYgf17184377;     DxrXBxYOOBduXWYgf17184377 = DxrXBxYOOBduXWYgf79861422;     DxrXBxYOOBduXWYgf79861422 = DxrXBxYOOBduXWYgf85912610;     DxrXBxYOOBduXWYgf85912610 = DxrXBxYOOBduXWYgf24060991;     DxrXBxYOOBduXWYgf24060991 = DxrXBxYOOBduXWYgf20563141;     DxrXBxYOOBduXWYgf20563141 = DxrXBxYOOBduXWYgf83295243;     DxrXBxYOOBduXWYgf83295243 = DxrXBxYOOBduXWYgf72067816;     DxrXBxYOOBduXWYgf72067816 = DxrXBxYOOBduXWYgf37337493;     DxrXBxYOOBduXWYgf37337493 = DxrXBxYOOBduXWYgf87760508;     DxrXBxYOOBduXWYgf87760508 = DxrXBxYOOBduXWYgf69559406;     DxrXBxYOOBduXWYgf69559406 = DxrXBxYOOBduXWYgf34153061;     DxrXBxYOOBduXWYgf34153061 = DxrXBxYOOBduXWYgf84616870;     DxrXBxYOOBduXWYgf84616870 = DxrXBxYOOBduXWYgf59816530;     DxrXBxYOOBduXWYgf59816530 = DxrXBxYOOBduXWYgf54795608;     DxrXBxYOOBduXWYgf54795608 = DxrXBxYOOBduXWYgf49227710;     DxrXBxYOOBduXWYgf49227710 = DxrXBxYOOBduXWYgf91330921;     DxrXBxYOOBduXWYgf91330921 = DxrXBxYOOBduXWYgf61249458;     DxrXBxYOOBduXWYgf61249458 = DxrXBxYOOBduXWYgf95126735;     DxrXBxYOOBduXWYgf95126735 = DxrXBxYOOBduXWYgf80620399;     DxrXBxYOOBduXWYgf80620399 = DxrXBxYOOBduXWYgf38962050;     DxrXBxYOOBduXWYgf38962050 = DxrXBxYOOBduXWYgf16006841;     DxrXBxYOOBduXWYgf16006841 = DxrXBxYOOBduXWYgf57399625;     DxrXBxYOOBduXWYgf57399625 = DxrXBxYOOBduXWYgf47735146;     DxrXBxYOOBduXWYgf47735146 = DxrXBxYOOBduXWYgf17869590;     DxrXBxYOOBduXWYgf17869590 = DxrXBxYOOBduXWYgf21984811;     DxrXBxYOOBduXWYgf21984811 = DxrXBxYOOBduXWYgf41413784;     DxrXBxYOOBduXWYgf41413784 = DxrXBxYOOBduXWYgf96640488;     DxrXBxYOOBduXWYgf96640488 = DxrXBxYOOBduXWYgf96552670;     DxrXBxYOOBduXWYgf96552670 = DxrXBxYOOBduXWYgf85444261;     DxrXBxYOOBduXWYgf85444261 = DxrXBxYOOBduXWYgf64751379;     DxrXBxYOOBduXWYgf64751379 = DxrXBxYOOBduXWYgf54642924;     DxrXBxYOOBduXWYgf54642924 = DxrXBxYOOBduXWYgf4470304;     DxrXBxYOOBduXWYgf4470304 = DxrXBxYOOBduXWYgf46909118;     DxrXBxYOOBduXWYgf46909118 = DxrXBxYOOBduXWYgf3126564;     DxrXBxYOOBduXWYgf3126564 = DxrXBxYOOBduXWYgf49332816;     DxrXBxYOOBduXWYgf49332816 = DxrXBxYOOBduXWYgf19443827;     DxrXBxYOOBduXWYgf19443827 = DxrXBxYOOBduXWYgf38041691;     DxrXBxYOOBduXWYgf38041691 = DxrXBxYOOBduXWYgf35321015;     DxrXBxYOOBduXWYgf35321015 = DxrXBxYOOBduXWYgf18737818;     DxrXBxYOOBduXWYgf18737818 = DxrXBxYOOBduXWYgf81726547;     DxrXBxYOOBduXWYgf81726547 = DxrXBxYOOBduXWYgf47566548;     DxrXBxYOOBduXWYgf47566548 = DxrXBxYOOBduXWYgf2578214;     DxrXBxYOOBduXWYgf2578214 = DxrXBxYOOBduXWYgf75218298;     DxrXBxYOOBduXWYgf75218298 = DxrXBxYOOBduXWYgf34038467;     DxrXBxYOOBduXWYgf34038467 = DxrXBxYOOBduXWYgf75981284;     DxrXBxYOOBduXWYgf75981284 = DxrXBxYOOBduXWYgf50943325;     DxrXBxYOOBduXWYgf50943325 = DxrXBxYOOBduXWYgf14362229;     DxrXBxYOOBduXWYgf14362229 = DxrXBxYOOBduXWYgf65923477;     DxrXBxYOOBduXWYgf65923477 = DxrXBxYOOBduXWYgf47635646;     DxrXBxYOOBduXWYgf47635646 = DxrXBxYOOBduXWYgf66326461;     DxrXBxYOOBduXWYgf66326461 = DxrXBxYOOBduXWYgf27925212;     DxrXBxYOOBduXWYgf27925212 = DxrXBxYOOBduXWYgf82662310;     DxrXBxYOOBduXWYgf82662310 = DxrXBxYOOBduXWYgf76466227;     DxrXBxYOOBduXWYgf76466227 = DxrXBxYOOBduXWYgf12261455;     DxrXBxYOOBduXWYgf12261455 = DxrXBxYOOBduXWYgf83135759;     DxrXBxYOOBduXWYgf83135759 = DxrXBxYOOBduXWYgf59784752;     DxrXBxYOOBduXWYgf59784752 = DxrXBxYOOBduXWYgf32126277;     DxrXBxYOOBduXWYgf32126277 = DxrXBxYOOBduXWYgf68043020;     DxrXBxYOOBduXWYgf68043020 = DxrXBxYOOBduXWYgf2076180;     DxrXBxYOOBduXWYgf2076180 = DxrXBxYOOBduXWYgf79149357;     DxrXBxYOOBduXWYgf79149357 = DxrXBxYOOBduXWYgf86654755;     DxrXBxYOOBduXWYgf86654755 = DxrXBxYOOBduXWYgf75515145;     DxrXBxYOOBduXWYgf75515145 = DxrXBxYOOBduXWYgf51893232;     DxrXBxYOOBduXWYgf51893232 = DxrXBxYOOBduXWYgf23009129;     DxrXBxYOOBduXWYgf23009129 = DxrXBxYOOBduXWYgf14916482;     DxrXBxYOOBduXWYgf14916482 = DxrXBxYOOBduXWYgf29682758;     DxrXBxYOOBduXWYgf29682758 = DxrXBxYOOBduXWYgf37707752;     DxrXBxYOOBduXWYgf37707752 = DxrXBxYOOBduXWYgf51333018;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void DmBrfNpxFjMWnhZo87670030() {     double SHjiiIKRhWQcMwMgF86791986 = -842687270;    double SHjiiIKRhWQcMwMgF41276014 = -820460885;    double SHjiiIKRhWQcMwMgF94931482 = -338044360;    double SHjiiIKRhWQcMwMgF87815897 = -234969536;    double SHjiiIKRhWQcMwMgF73644843 = -820385963;    double SHjiiIKRhWQcMwMgF29638898 = -422069807;    double SHjiiIKRhWQcMwMgF53759050 = -856893359;    double SHjiiIKRhWQcMwMgF16057611 = -738883613;    double SHjiiIKRhWQcMwMgF33949575 = -240616510;    double SHjiiIKRhWQcMwMgF76459642 = -860692403;    double SHjiiIKRhWQcMwMgF36314357 = -651344260;    double SHjiiIKRhWQcMwMgF2837312 = -403349412;    double SHjiiIKRhWQcMwMgF4751838 = -770556164;    double SHjiiIKRhWQcMwMgF52446112 = -678796309;    double SHjiiIKRhWQcMwMgF82148817 = -65537899;    double SHjiiIKRhWQcMwMgF42923306 = -97724341;    double SHjiiIKRhWQcMwMgF95136895 = -216678751;    double SHjiiIKRhWQcMwMgF64781982 = -668340229;    double SHjiiIKRhWQcMwMgF20184694 = -524776677;    double SHjiiIKRhWQcMwMgF33752925 = -18516221;    double SHjiiIKRhWQcMwMgF76065669 = -276150387;    double SHjiiIKRhWQcMwMgF7019397 = -866737448;    double SHjiiIKRhWQcMwMgF80659725 = -752603484;    double SHjiiIKRhWQcMwMgF29353140 = -550331551;    double SHjiiIKRhWQcMwMgF82196710 = -253999823;    double SHjiiIKRhWQcMwMgF31417258 = -322883626;    double SHjiiIKRhWQcMwMgF57283930 = -370811075;    double SHjiiIKRhWQcMwMgF98385807 = -123982465;    double SHjiiIKRhWQcMwMgF35842885 = -781529382;    double SHjiiIKRhWQcMwMgF83322456 = -476972370;    double SHjiiIKRhWQcMwMgF75669858 = -977679629;    double SHjiiIKRhWQcMwMgF89046522 = -562629584;    double SHjiiIKRhWQcMwMgF11360611 = -638312182;    double SHjiiIKRhWQcMwMgF46932803 = -919102508;    double SHjiiIKRhWQcMwMgF45669876 = -316705362;    double SHjiiIKRhWQcMwMgF37473454 = -549497775;    double SHjiiIKRhWQcMwMgF73472807 = -668326650;    double SHjiiIKRhWQcMwMgF71778544 = -839246047;    double SHjiiIKRhWQcMwMgF13685806 = -628911830;    double SHjiiIKRhWQcMwMgF36339348 = -396255219;    double SHjiiIKRhWQcMwMgF70789100 = 84097206;    double SHjiiIKRhWQcMwMgF77125635 = -365019824;    double SHjiiIKRhWQcMwMgF58942784 = -886402333;    double SHjiiIKRhWQcMwMgF74284399 = -750694736;    double SHjiiIKRhWQcMwMgF84633328 = -623785591;    double SHjiiIKRhWQcMwMgF5777859 = -751221766;    double SHjiiIKRhWQcMwMgF74140772 = -42604723;    double SHjiiIKRhWQcMwMgF66954022 = -593450763;    double SHjiiIKRhWQcMwMgF14223588 = -48421687;    double SHjiiIKRhWQcMwMgF85779047 = -98447480;    double SHjiiIKRhWQcMwMgF75897459 = -513746809;    double SHjiiIKRhWQcMwMgF64105872 = -265639768;    double SHjiiIKRhWQcMwMgF7583199 = -283750998;    double SHjiiIKRhWQcMwMgF31998688 = -73043013;    double SHjiiIKRhWQcMwMgF11604362 = -212829179;    double SHjiiIKRhWQcMwMgF79772589 = -975949823;    double SHjiiIKRhWQcMwMgF60616288 = 32142598;    double SHjiiIKRhWQcMwMgF65578342 = -787712810;    double SHjiiIKRhWQcMwMgF5619188 = -980969713;    double SHjiiIKRhWQcMwMgF42227586 = -397502338;    double SHjiiIKRhWQcMwMgF72354968 = 48741268;    double SHjiiIKRhWQcMwMgF55373242 = -632910895;    double SHjiiIKRhWQcMwMgF80214725 = -957354231;    double SHjiiIKRhWQcMwMgF50627118 = -763644141;    double SHjiiIKRhWQcMwMgF789785 = -883012774;    double SHjiiIKRhWQcMwMgF47267834 = 11285324;    double SHjiiIKRhWQcMwMgF91476701 = -765037231;    double SHjiiIKRhWQcMwMgF57819034 = -851453656;    double SHjiiIKRhWQcMwMgF6776237 = -262090948;    double SHjiiIKRhWQcMwMgF44675364 = -516040125;    double SHjiiIKRhWQcMwMgF69450499 = -429397692;    double SHjiiIKRhWQcMwMgF23358351 = -377432705;    double SHjiiIKRhWQcMwMgF51096177 = 60571601;    double SHjiiIKRhWQcMwMgF83845345 = -28521459;    double SHjiiIKRhWQcMwMgF62963825 = -2613428;    double SHjiiIKRhWQcMwMgF98940034 = -911130563;    double SHjiiIKRhWQcMwMgF48076613 = -980335115;    double SHjiiIKRhWQcMwMgF6375326 = 98091252;    double SHjiiIKRhWQcMwMgF44719812 = -926545960;    double SHjiiIKRhWQcMwMgF76418852 = -502778057;    double SHjiiIKRhWQcMwMgF57276486 = -180278904;    double SHjiiIKRhWQcMwMgF90329908 = -777360313;    double SHjiiIKRhWQcMwMgF84162219 = 24439221;    double SHjiiIKRhWQcMwMgF50063837 = -583081903;    double SHjiiIKRhWQcMwMgF7424998 = -963225561;    double SHjiiIKRhWQcMwMgF11563987 = -612039862;    double SHjiiIKRhWQcMwMgF81463324 = -178878586;    double SHjiiIKRhWQcMwMgF79361923 = -465269170;    double SHjiiIKRhWQcMwMgF35328442 = -606273330;    double SHjiiIKRhWQcMwMgF65897286 = -340755540;    double SHjiiIKRhWQcMwMgF76857165 = -481640373;    double SHjiiIKRhWQcMwMgF7894465 = -880613840;    double SHjiiIKRhWQcMwMgF66159357 = -858276334;    double SHjiiIKRhWQcMwMgF71458219 = -131409493;    double SHjiiIKRhWQcMwMgF63984380 = -344996487;    double SHjiiIKRhWQcMwMgF15415859 = -282991900;    double SHjiiIKRhWQcMwMgF96910909 = -407665593;    double SHjiiIKRhWQcMwMgF8315666 = -22758193;    double SHjiiIKRhWQcMwMgF73494615 = -867681962;    double SHjiiIKRhWQcMwMgF37365494 = -842687270;     SHjiiIKRhWQcMwMgF86791986 = SHjiiIKRhWQcMwMgF41276014;     SHjiiIKRhWQcMwMgF41276014 = SHjiiIKRhWQcMwMgF94931482;     SHjiiIKRhWQcMwMgF94931482 = SHjiiIKRhWQcMwMgF87815897;     SHjiiIKRhWQcMwMgF87815897 = SHjiiIKRhWQcMwMgF73644843;     SHjiiIKRhWQcMwMgF73644843 = SHjiiIKRhWQcMwMgF29638898;     SHjiiIKRhWQcMwMgF29638898 = SHjiiIKRhWQcMwMgF53759050;     SHjiiIKRhWQcMwMgF53759050 = SHjiiIKRhWQcMwMgF16057611;     SHjiiIKRhWQcMwMgF16057611 = SHjiiIKRhWQcMwMgF33949575;     SHjiiIKRhWQcMwMgF33949575 = SHjiiIKRhWQcMwMgF76459642;     SHjiiIKRhWQcMwMgF76459642 = SHjiiIKRhWQcMwMgF36314357;     SHjiiIKRhWQcMwMgF36314357 = SHjiiIKRhWQcMwMgF2837312;     SHjiiIKRhWQcMwMgF2837312 = SHjiiIKRhWQcMwMgF4751838;     SHjiiIKRhWQcMwMgF4751838 = SHjiiIKRhWQcMwMgF52446112;     SHjiiIKRhWQcMwMgF52446112 = SHjiiIKRhWQcMwMgF82148817;     SHjiiIKRhWQcMwMgF82148817 = SHjiiIKRhWQcMwMgF42923306;     SHjiiIKRhWQcMwMgF42923306 = SHjiiIKRhWQcMwMgF95136895;     SHjiiIKRhWQcMwMgF95136895 = SHjiiIKRhWQcMwMgF64781982;     SHjiiIKRhWQcMwMgF64781982 = SHjiiIKRhWQcMwMgF20184694;     SHjiiIKRhWQcMwMgF20184694 = SHjiiIKRhWQcMwMgF33752925;     SHjiiIKRhWQcMwMgF33752925 = SHjiiIKRhWQcMwMgF76065669;     SHjiiIKRhWQcMwMgF76065669 = SHjiiIKRhWQcMwMgF7019397;     SHjiiIKRhWQcMwMgF7019397 = SHjiiIKRhWQcMwMgF80659725;     SHjiiIKRhWQcMwMgF80659725 = SHjiiIKRhWQcMwMgF29353140;     SHjiiIKRhWQcMwMgF29353140 = SHjiiIKRhWQcMwMgF82196710;     SHjiiIKRhWQcMwMgF82196710 = SHjiiIKRhWQcMwMgF31417258;     SHjiiIKRhWQcMwMgF31417258 = SHjiiIKRhWQcMwMgF57283930;     SHjiiIKRhWQcMwMgF57283930 = SHjiiIKRhWQcMwMgF98385807;     SHjiiIKRhWQcMwMgF98385807 = SHjiiIKRhWQcMwMgF35842885;     SHjiiIKRhWQcMwMgF35842885 = SHjiiIKRhWQcMwMgF83322456;     SHjiiIKRhWQcMwMgF83322456 = SHjiiIKRhWQcMwMgF75669858;     SHjiiIKRhWQcMwMgF75669858 = SHjiiIKRhWQcMwMgF89046522;     SHjiiIKRhWQcMwMgF89046522 = SHjiiIKRhWQcMwMgF11360611;     SHjiiIKRhWQcMwMgF11360611 = SHjiiIKRhWQcMwMgF46932803;     SHjiiIKRhWQcMwMgF46932803 = SHjiiIKRhWQcMwMgF45669876;     SHjiiIKRhWQcMwMgF45669876 = SHjiiIKRhWQcMwMgF37473454;     SHjiiIKRhWQcMwMgF37473454 = SHjiiIKRhWQcMwMgF73472807;     SHjiiIKRhWQcMwMgF73472807 = SHjiiIKRhWQcMwMgF71778544;     SHjiiIKRhWQcMwMgF71778544 = SHjiiIKRhWQcMwMgF13685806;     SHjiiIKRhWQcMwMgF13685806 = SHjiiIKRhWQcMwMgF36339348;     SHjiiIKRhWQcMwMgF36339348 = SHjiiIKRhWQcMwMgF70789100;     SHjiiIKRhWQcMwMgF70789100 = SHjiiIKRhWQcMwMgF77125635;     SHjiiIKRhWQcMwMgF77125635 = SHjiiIKRhWQcMwMgF58942784;     SHjiiIKRhWQcMwMgF58942784 = SHjiiIKRhWQcMwMgF74284399;     SHjiiIKRhWQcMwMgF74284399 = SHjiiIKRhWQcMwMgF84633328;     SHjiiIKRhWQcMwMgF84633328 = SHjiiIKRhWQcMwMgF5777859;     SHjiiIKRhWQcMwMgF5777859 = SHjiiIKRhWQcMwMgF74140772;     SHjiiIKRhWQcMwMgF74140772 = SHjiiIKRhWQcMwMgF66954022;     SHjiiIKRhWQcMwMgF66954022 = SHjiiIKRhWQcMwMgF14223588;     SHjiiIKRhWQcMwMgF14223588 = SHjiiIKRhWQcMwMgF85779047;     SHjiiIKRhWQcMwMgF85779047 = SHjiiIKRhWQcMwMgF75897459;     SHjiiIKRhWQcMwMgF75897459 = SHjiiIKRhWQcMwMgF64105872;     SHjiiIKRhWQcMwMgF64105872 = SHjiiIKRhWQcMwMgF7583199;     SHjiiIKRhWQcMwMgF7583199 = SHjiiIKRhWQcMwMgF31998688;     SHjiiIKRhWQcMwMgF31998688 = SHjiiIKRhWQcMwMgF11604362;     SHjiiIKRhWQcMwMgF11604362 = SHjiiIKRhWQcMwMgF79772589;     SHjiiIKRhWQcMwMgF79772589 = SHjiiIKRhWQcMwMgF60616288;     SHjiiIKRhWQcMwMgF60616288 = SHjiiIKRhWQcMwMgF65578342;     SHjiiIKRhWQcMwMgF65578342 = SHjiiIKRhWQcMwMgF5619188;     SHjiiIKRhWQcMwMgF5619188 = SHjiiIKRhWQcMwMgF42227586;     SHjiiIKRhWQcMwMgF42227586 = SHjiiIKRhWQcMwMgF72354968;     SHjiiIKRhWQcMwMgF72354968 = SHjiiIKRhWQcMwMgF55373242;     SHjiiIKRhWQcMwMgF55373242 = SHjiiIKRhWQcMwMgF80214725;     SHjiiIKRhWQcMwMgF80214725 = SHjiiIKRhWQcMwMgF50627118;     SHjiiIKRhWQcMwMgF50627118 = SHjiiIKRhWQcMwMgF789785;     SHjiiIKRhWQcMwMgF789785 = SHjiiIKRhWQcMwMgF47267834;     SHjiiIKRhWQcMwMgF47267834 = SHjiiIKRhWQcMwMgF91476701;     SHjiiIKRhWQcMwMgF91476701 = SHjiiIKRhWQcMwMgF57819034;     SHjiiIKRhWQcMwMgF57819034 = SHjiiIKRhWQcMwMgF6776237;     SHjiiIKRhWQcMwMgF6776237 = SHjiiIKRhWQcMwMgF44675364;     SHjiiIKRhWQcMwMgF44675364 = SHjiiIKRhWQcMwMgF69450499;     SHjiiIKRhWQcMwMgF69450499 = SHjiiIKRhWQcMwMgF23358351;     SHjiiIKRhWQcMwMgF23358351 = SHjiiIKRhWQcMwMgF51096177;     SHjiiIKRhWQcMwMgF51096177 = SHjiiIKRhWQcMwMgF83845345;     SHjiiIKRhWQcMwMgF83845345 = SHjiiIKRhWQcMwMgF62963825;     SHjiiIKRhWQcMwMgF62963825 = SHjiiIKRhWQcMwMgF98940034;     SHjiiIKRhWQcMwMgF98940034 = SHjiiIKRhWQcMwMgF48076613;     SHjiiIKRhWQcMwMgF48076613 = SHjiiIKRhWQcMwMgF6375326;     SHjiiIKRhWQcMwMgF6375326 = SHjiiIKRhWQcMwMgF44719812;     SHjiiIKRhWQcMwMgF44719812 = SHjiiIKRhWQcMwMgF76418852;     SHjiiIKRhWQcMwMgF76418852 = SHjiiIKRhWQcMwMgF57276486;     SHjiiIKRhWQcMwMgF57276486 = SHjiiIKRhWQcMwMgF90329908;     SHjiiIKRhWQcMwMgF90329908 = SHjiiIKRhWQcMwMgF84162219;     SHjiiIKRhWQcMwMgF84162219 = SHjiiIKRhWQcMwMgF50063837;     SHjiiIKRhWQcMwMgF50063837 = SHjiiIKRhWQcMwMgF7424998;     SHjiiIKRhWQcMwMgF7424998 = SHjiiIKRhWQcMwMgF11563987;     SHjiiIKRhWQcMwMgF11563987 = SHjiiIKRhWQcMwMgF81463324;     SHjiiIKRhWQcMwMgF81463324 = SHjiiIKRhWQcMwMgF79361923;     SHjiiIKRhWQcMwMgF79361923 = SHjiiIKRhWQcMwMgF35328442;     SHjiiIKRhWQcMwMgF35328442 = SHjiiIKRhWQcMwMgF65897286;     SHjiiIKRhWQcMwMgF65897286 = SHjiiIKRhWQcMwMgF76857165;     SHjiiIKRhWQcMwMgF76857165 = SHjiiIKRhWQcMwMgF7894465;     SHjiiIKRhWQcMwMgF7894465 = SHjiiIKRhWQcMwMgF66159357;     SHjiiIKRhWQcMwMgF66159357 = SHjiiIKRhWQcMwMgF71458219;     SHjiiIKRhWQcMwMgF71458219 = SHjiiIKRhWQcMwMgF63984380;     SHjiiIKRhWQcMwMgF63984380 = SHjiiIKRhWQcMwMgF15415859;     SHjiiIKRhWQcMwMgF15415859 = SHjiiIKRhWQcMwMgF96910909;     SHjiiIKRhWQcMwMgF96910909 = SHjiiIKRhWQcMwMgF8315666;     SHjiiIKRhWQcMwMgF8315666 = SHjiiIKRhWQcMwMgF73494615;     SHjiiIKRhWQcMwMgF73494615 = SHjiiIKRhWQcMwMgF37365494;     SHjiiIKRhWQcMwMgF37365494 = SHjiiIKRhWQcMwMgF86791986;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BCBUdpPYFmKkRXws2719099() {     double nDjyVxCcZCzEAojtI92909093 = -954589159;    double nDjyVxCcZCzEAojtI84649293 = -530060665;    double nDjyVxCcZCzEAojtI10973666 = -488877282;    double nDjyVxCcZCzEAojtI14890679 = -425718166;    double nDjyVxCcZCzEAojtI42540332 = -241035835;    double nDjyVxCcZCzEAojtI21056101 = -686566042;    double nDjyVxCcZCzEAojtI14248016 = -995858337;    double nDjyVxCcZCzEAojtI65817501 = 74242474;    double nDjyVxCcZCzEAojtI98592070 = -8649176;    double nDjyVxCcZCzEAojtI17689302 = -89667881;    double nDjyVxCcZCzEAojtI96424647 = -528781846;    double nDjyVxCcZCzEAojtI34660355 = -847755614;    double nDjyVxCcZCzEAojtI90960956 = -110447253;    double nDjyVxCcZCzEAojtI35589789 = -111034877;    double nDjyVxCcZCzEAojtI77891244 = -756000066;    double nDjyVxCcZCzEAojtI84428700 = -70723884;    double nDjyVxCcZCzEAojtI88972898 = -833223864;    double nDjyVxCcZCzEAojtI61835135 = -333862871;    double nDjyVxCcZCzEAojtI82014535 = -930545774;    double nDjyVxCcZCzEAojtI14303090 = -375419141;    double nDjyVxCcZCzEAojtI64618862 = -131637842;    double nDjyVxCcZCzEAojtI93540420 = -774667164;    double nDjyVxCcZCzEAojtI16250517 = -499120970;    double nDjyVxCcZCzEAojtI13345514 = -772636538;    double nDjyVxCcZCzEAojtI25896360 = -627195856;    double nDjyVxCcZCzEAojtI56138933 = -201538949;    double nDjyVxCcZCzEAojtI55743708 = -271473278;    double nDjyVxCcZCzEAojtI11492933 = -189911419;    double nDjyVxCcZCzEAojtI1975141 = 96087913;    double nDjyVxCcZCzEAojtI51938093 = -294443237;    double nDjyVxCcZCzEAojtI67912407 = -891000567;    double nDjyVxCcZCzEAojtI86657144 = -272076400;    double nDjyVxCcZCzEAojtI21071560 = -530925552;    double nDjyVxCcZCzEAojtI41763367 = -670644274;    double nDjyVxCcZCzEAojtI7144528 = -955438467;    double nDjyVxCcZCzEAojtI6199368 = -763523882;    double nDjyVxCcZCzEAojtI54734742 = -621962520;    double nDjyVxCcZCzEAojtI5868758 = -610752056;    double nDjyVxCcZCzEAojtI87296059 = -622835327;    double nDjyVxCcZCzEAojtI23841286 = -869694441;    double nDjyVxCcZCzEAojtI76458324 = 18823312;    double nDjyVxCcZCzEAojtI9276092 = -635162102;    double nDjyVxCcZCzEAojtI37009762 = -967206917;    double nDjyVxCcZCzEAojtI61692140 = -770037674;    double nDjyVxCcZCzEAojtI96585793 = -285683745;    double nDjyVxCcZCzEAojtI17717054 = -736437883;    double nDjyVxCcZCzEAojtI91919346 = -161380260;    double nDjyVxCcZCzEAojtI74910963 = -603346295;    double nDjyVxCcZCzEAojtI23899317 = -134158675;    double nDjyVxCcZCzEAojtI98690700 = -185458288;    double nDjyVxCcZCzEAojtI11598138 = -356040481;    double nDjyVxCcZCzEAojtI74915591 = -66366088;    double nDjyVxCcZCzEAojtI55831656 = -75540586;    double nDjyVxCcZCzEAojtI27967568 = -393513158;    double nDjyVxCcZCzEAojtI38100236 = -913360961;    double nDjyVxCcZCzEAojtI99368672 = -79921995;    double nDjyVxCcZCzEAojtI68398776 = 69060304;    double nDjyVxCcZCzEAojtI97628152 = -716240744;    double nDjyVxCcZCzEAojtI88994318 = -798522311;    double nDjyVxCcZCzEAojtI86401399 = 60503114;    double nDjyVxCcZCzEAojtI65312392 = -315092764;    double nDjyVxCcZCzEAojtI2755084 = -705946918;    double nDjyVxCcZCzEAojtI63842361 = 78154561;    double nDjyVxCcZCzEAojtI46653977 = -714205939;    double nDjyVxCcZCzEAojtI49776894 = -198667314;    double nDjyVxCcZCzEAojtI9767503 = -156705447;    double nDjyVxCcZCzEAojtI13588795 = -216830062;    double nDjyVxCcZCzEAojtI49197589 = -439802980;    double nDjyVxCcZCzEAojtI28445261 = -155596410;    double nDjyVxCcZCzEAojtI71691877 = -992476184;    double nDjyVxCcZCzEAojtI29693958 = -448761365;    double nDjyVxCcZCzEAojtI83104141 = -122471809;    double nDjyVxCcZCzEAojtI74539076 = -711027545;    double nDjyVxCcZCzEAojtI58173249 = 39148667;    double nDjyVxCcZCzEAojtI37844766 = -294242453;    double nDjyVxCcZCzEAojtI55342770 = -496475741;    double nDjyVxCcZCzEAojtI56530658 = -807460248;    double nDjyVxCcZCzEAojtI54558377 = -729083297;    double nDjyVxCcZCzEAojtI16759721 = -386952793;    double nDjyVxCcZCzEAojtI8179307 = -890757974;    double nDjyVxCcZCzEAojtI64219587 = 59841311;    double nDjyVxCcZCzEAojtI80832745 = -668126983;    double nDjyVxCcZCzEAojtI87593616 = 44247256;    double nDjyVxCcZCzEAojtI3284440 = -718453799;    double nDjyVxCcZCzEAojtI40339956 = -938402757;    double nDjyVxCcZCzEAojtI92996816 = -724634479;    double nDjyVxCcZCzEAojtI30825489 = -96535814;    double nDjyVxCcZCzEAojtI93103991 = -37412395;    double nDjyVxCcZCzEAojtI3663132 = -757283313;    double nDjyVxCcZCzEAojtI7775856 = -775516473;    double nDjyVxCcZCzEAojtI37800591 = -732584187;    double nDjyVxCcZCzEAojtI57106590 = -905721776;    double nDjyVxCcZCzEAojtI16874439 = -812229746;    double nDjyVxCcZCzEAojtI894661 = -583338441;    double nDjyVxCcZCzEAojtI58528893 = -454601678;    double nDjyVxCcZCzEAojtI73703241 = -275229770;    double nDjyVxCcZCzEAojtI45433731 = -613316663;    double nDjyVxCcZCzEAojtI90355785 = -153000978;    double nDjyVxCcZCzEAojtI11915246 = -471370361;    double nDjyVxCcZCzEAojtI86818290 = -954589159;     nDjyVxCcZCzEAojtI92909093 = nDjyVxCcZCzEAojtI84649293;     nDjyVxCcZCzEAojtI84649293 = nDjyVxCcZCzEAojtI10973666;     nDjyVxCcZCzEAojtI10973666 = nDjyVxCcZCzEAojtI14890679;     nDjyVxCcZCzEAojtI14890679 = nDjyVxCcZCzEAojtI42540332;     nDjyVxCcZCzEAojtI42540332 = nDjyVxCcZCzEAojtI21056101;     nDjyVxCcZCzEAojtI21056101 = nDjyVxCcZCzEAojtI14248016;     nDjyVxCcZCzEAojtI14248016 = nDjyVxCcZCzEAojtI65817501;     nDjyVxCcZCzEAojtI65817501 = nDjyVxCcZCzEAojtI98592070;     nDjyVxCcZCzEAojtI98592070 = nDjyVxCcZCzEAojtI17689302;     nDjyVxCcZCzEAojtI17689302 = nDjyVxCcZCzEAojtI96424647;     nDjyVxCcZCzEAojtI96424647 = nDjyVxCcZCzEAojtI34660355;     nDjyVxCcZCzEAojtI34660355 = nDjyVxCcZCzEAojtI90960956;     nDjyVxCcZCzEAojtI90960956 = nDjyVxCcZCzEAojtI35589789;     nDjyVxCcZCzEAojtI35589789 = nDjyVxCcZCzEAojtI77891244;     nDjyVxCcZCzEAojtI77891244 = nDjyVxCcZCzEAojtI84428700;     nDjyVxCcZCzEAojtI84428700 = nDjyVxCcZCzEAojtI88972898;     nDjyVxCcZCzEAojtI88972898 = nDjyVxCcZCzEAojtI61835135;     nDjyVxCcZCzEAojtI61835135 = nDjyVxCcZCzEAojtI82014535;     nDjyVxCcZCzEAojtI82014535 = nDjyVxCcZCzEAojtI14303090;     nDjyVxCcZCzEAojtI14303090 = nDjyVxCcZCzEAojtI64618862;     nDjyVxCcZCzEAojtI64618862 = nDjyVxCcZCzEAojtI93540420;     nDjyVxCcZCzEAojtI93540420 = nDjyVxCcZCzEAojtI16250517;     nDjyVxCcZCzEAojtI16250517 = nDjyVxCcZCzEAojtI13345514;     nDjyVxCcZCzEAojtI13345514 = nDjyVxCcZCzEAojtI25896360;     nDjyVxCcZCzEAojtI25896360 = nDjyVxCcZCzEAojtI56138933;     nDjyVxCcZCzEAojtI56138933 = nDjyVxCcZCzEAojtI55743708;     nDjyVxCcZCzEAojtI55743708 = nDjyVxCcZCzEAojtI11492933;     nDjyVxCcZCzEAojtI11492933 = nDjyVxCcZCzEAojtI1975141;     nDjyVxCcZCzEAojtI1975141 = nDjyVxCcZCzEAojtI51938093;     nDjyVxCcZCzEAojtI51938093 = nDjyVxCcZCzEAojtI67912407;     nDjyVxCcZCzEAojtI67912407 = nDjyVxCcZCzEAojtI86657144;     nDjyVxCcZCzEAojtI86657144 = nDjyVxCcZCzEAojtI21071560;     nDjyVxCcZCzEAojtI21071560 = nDjyVxCcZCzEAojtI41763367;     nDjyVxCcZCzEAojtI41763367 = nDjyVxCcZCzEAojtI7144528;     nDjyVxCcZCzEAojtI7144528 = nDjyVxCcZCzEAojtI6199368;     nDjyVxCcZCzEAojtI6199368 = nDjyVxCcZCzEAojtI54734742;     nDjyVxCcZCzEAojtI54734742 = nDjyVxCcZCzEAojtI5868758;     nDjyVxCcZCzEAojtI5868758 = nDjyVxCcZCzEAojtI87296059;     nDjyVxCcZCzEAojtI87296059 = nDjyVxCcZCzEAojtI23841286;     nDjyVxCcZCzEAojtI23841286 = nDjyVxCcZCzEAojtI76458324;     nDjyVxCcZCzEAojtI76458324 = nDjyVxCcZCzEAojtI9276092;     nDjyVxCcZCzEAojtI9276092 = nDjyVxCcZCzEAojtI37009762;     nDjyVxCcZCzEAojtI37009762 = nDjyVxCcZCzEAojtI61692140;     nDjyVxCcZCzEAojtI61692140 = nDjyVxCcZCzEAojtI96585793;     nDjyVxCcZCzEAojtI96585793 = nDjyVxCcZCzEAojtI17717054;     nDjyVxCcZCzEAojtI17717054 = nDjyVxCcZCzEAojtI91919346;     nDjyVxCcZCzEAojtI91919346 = nDjyVxCcZCzEAojtI74910963;     nDjyVxCcZCzEAojtI74910963 = nDjyVxCcZCzEAojtI23899317;     nDjyVxCcZCzEAojtI23899317 = nDjyVxCcZCzEAojtI98690700;     nDjyVxCcZCzEAojtI98690700 = nDjyVxCcZCzEAojtI11598138;     nDjyVxCcZCzEAojtI11598138 = nDjyVxCcZCzEAojtI74915591;     nDjyVxCcZCzEAojtI74915591 = nDjyVxCcZCzEAojtI55831656;     nDjyVxCcZCzEAojtI55831656 = nDjyVxCcZCzEAojtI27967568;     nDjyVxCcZCzEAojtI27967568 = nDjyVxCcZCzEAojtI38100236;     nDjyVxCcZCzEAojtI38100236 = nDjyVxCcZCzEAojtI99368672;     nDjyVxCcZCzEAojtI99368672 = nDjyVxCcZCzEAojtI68398776;     nDjyVxCcZCzEAojtI68398776 = nDjyVxCcZCzEAojtI97628152;     nDjyVxCcZCzEAojtI97628152 = nDjyVxCcZCzEAojtI88994318;     nDjyVxCcZCzEAojtI88994318 = nDjyVxCcZCzEAojtI86401399;     nDjyVxCcZCzEAojtI86401399 = nDjyVxCcZCzEAojtI65312392;     nDjyVxCcZCzEAojtI65312392 = nDjyVxCcZCzEAojtI2755084;     nDjyVxCcZCzEAojtI2755084 = nDjyVxCcZCzEAojtI63842361;     nDjyVxCcZCzEAojtI63842361 = nDjyVxCcZCzEAojtI46653977;     nDjyVxCcZCzEAojtI46653977 = nDjyVxCcZCzEAojtI49776894;     nDjyVxCcZCzEAojtI49776894 = nDjyVxCcZCzEAojtI9767503;     nDjyVxCcZCzEAojtI9767503 = nDjyVxCcZCzEAojtI13588795;     nDjyVxCcZCzEAojtI13588795 = nDjyVxCcZCzEAojtI49197589;     nDjyVxCcZCzEAojtI49197589 = nDjyVxCcZCzEAojtI28445261;     nDjyVxCcZCzEAojtI28445261 = nDjyVxCcZCzEAojtI71691877;     nDjyVxCcZCzEAojtI71691877 = nDjyVxCcZCzEAojtI29693958;     nDjyVxCcZCzEAojtI29693958 = nDjyVxCcZCzEAojtI83104141;     nDjyVxCcZCzEAojtI83104141 = nDjyVxCcZCzEAojtI74539076;     nDjyVxCcZCzEAojtI74539076 = nDjyVxCcZCzEAojtI58173249;     nDjyVxCcZCzEAojtI58173249 = nDjyVxCcZCzEAojtI37844766;     nDjyVxCcZCzEAojtI37844766 = nDjyVxCcZCzEAojtI55342770;     nDjyVxCcZCzEAojtI55342770 = nDjyVxCcZCzEAojtI56530658;     nDjyVxCcZCzEAojtI56530658 = nDjyVxCcZCzEAojtI54558377;     nDjyVxCcZCzEAojtI54558377 = nDjyVxCcZCzEAojtI16759721;     nDjyVxCcZCzEAojtI16759721 = nDjyVxCcZCzEAojtI8179307;     nDjyVxCcZCzEAojtI8179307 = nDjyVxCcZCzEAojtI64219587;     nDjyVxCcZCzEAojtI64219587 = nDjyVxCcZCzEAojtI80832745;     nDjyVxCcZCzEAojtI80832745 = nDjyVxCcZCzEAojtI87593616;     nDjyVxCcZCzEAojtI87593616 = nDjyVxCcZCzEAojtI3284440;     nDjyVxCcZCzEAojtI3284440 = nDjyVxCcZCzEAojtI40339956;     nDjyVxCcZCzEAojtI40339956 = nDjyVxCcZCzEAojtI92996816;     nDjyVxCcZCzEAojtI92996816 = nDjyVxCcZCzEAojtI30825489;     nDjyVxCcZCzEAojtI30825489 = nDjyVxCcZCzEAojtI93103991;     nDjyVxCcZCzEAojtI93103991 = nDjyVxCcZCzEAojtI3663132;     nDjyVxCcZCzEAojtI3663132 = nDjyVxCcZCzEAojtI7775856;     nDjyVxCcZCzEAojtI7775856 = nDjyVxCcZCzEAojtI37800591;     nDjyVxCcZCzEAojtI37800591 = nDjyVxCcZCzEAojtI57106590;     nDjyVxCcZCzEAojtI57106590 = nDjyVxCcZCzEAojtI16874439;     nDjyVxCcZCzEAojtI16874439 = nDjyVxCcZCzEAojtI894661;     nDjyVxCcZCzEAojtI894661 = nDjyVxCcZCzEAojtI58528893;     nDjyVxCcZCzEAojtI58528893 = nDjyVxCcZCzEAojtI73703241;     nDjyVxCcZCzEAojtI73703241 = nDjyVxCcZCzEAojtI45433731;     nDjyVxCcZCzEAojtI45433731 = nDjyVxCcZCzEAojtI90355785;     nDjyVxCcZCzEAojtI90355785 = nDjyVxCcZCzEAojtI11915246;     nDjyVxCcZCzEAojtI11915246 = nDjyVxCcZCzEAojtI86818290;     nDjyVxCcZCzEAojtI86818290 = nDjyVxCcZCzEAojtI92909093;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AYAnAqafPxvyvPLh70010697() {     double trfYsRscQuBIhwkjH28368062 = -320866058;    double trfYsRscQuBIhwkjH97921371 = -410612272;    double trfYsRscQuBIhwkjH48360143 = -154544357;    double trfYsRscQuBIhwkjH87114033 = -246869238;    double trfYsRscQuBIhwkjH98825601 = -346634680;    double trfYsRscQuBIhwkjH57278973 = -948913937;    double trfYsRscQuBIhwkjH24905422 = -840584071;    double trfYsRscQuBIhwkjH27949062 = -669991666;    double trfYsRscQuBIhwkjH109676 = -496980582;    double trfYsRscQuBIhwkjH32592014 = -854354879;    double trfYsRscQuBIhwkjH34606383 = -330789575;    double trfYsRscQuBIhwkjH35228503 = -711876386;    double trfYsRscQuBIhwkjH29195602 = -662459758;    double trfYsRscQuBIhwkjH88730652 = -87398286;    double trfYsRscQuBIhwkjH36085761 = -159425081;    double trfYsRscQuBIhwkjH67970001 = -806535491;    double trfYsRscQuBIhwkjH44808834 = -260198417;    double trfYsRscQuBIhwkjH61595327 = -407290940;    double trfYsRscQuBIhwkjH82564865 = -147109195;    double trfYsRscQuBIhwkjH8140309 = -801980207;    double trfYsRscQuBIhwkjH77705725 = -888912531;    double trfYsRscQuBIhwkjH96961945 = -861303458;    double trfYsRscQuBIhwkjH86775897 = -136753155;    double trfYsRscQuBIhwkjH7138460 = -183461143;    double trfYsRscQuBIhwkjH33914311 = -651181055;    double trfYsRscQuBIhwkjH66837105 = -990115756;    double trfYsRscQuBIhwkjH16164283 = -77757419;    double trfYsRscQuBIhwkjH52221358 = -359683593;    double trfYsRscQuBIhwkjH48643355 = -478006889;    double trfYsRscQuBIhwkjH57471504 = -328950407;    double trfYsRscQuBIhwkjH86495639 = -342981055;    double trfYsRscQuBIhwkjH24480163 = -45047814;    double trfYsRscQuBIhwkjH33289570 = -678521530;    double trfYsRscQuBIhwkjH71511794 = -735338087;    double trfYsRscQuBIhwkjH72952981 = -906538718;    double trfYsRscQuBIhwkjH57760211 = -978089188;    double trfYsRscQuBIhwkjH4146559 = -486658560;    double trfYsRscQuBIhwkjH57084160 = -593407637;    double trfYsRscQuBIhwkjH17686621 = -273711603;    double trfYsRscQuBIhwkjH88112818 = -334956987;    double trfYsRscQuBIhwkjH9909933 = -850375603;    double trfYsRscQuBIhwkjH98641218 = -229707393;    double trfYsRscQuBIhwkjH26393141 = -520387703;    double trfYsRscQuBIhwkjH1823478 = -274370084;    double trfYsRscQuBIhwkjH96602252 = -112194907;    double trfYsRscQuBIhwkjH63678382 = -775361515;    double trfYsRscQuBIhwkjH11264511 = -645366621;    double trfYsRscQuBIhwkjH92637274 = -769872889;    double trfYsRscQuBIhwkjH46791983 = -897895555;    double trfYsRscQuBIhwkjH23220290 = -676485104;    double trfYsRscQuBIhwkjH92368861 = -230317038;    double trfYsRscQuBIhwkjH58401064 = -280018435;    double trfYsRscQuBIhwkjH24452805 = -931586140;    double trfYsRscQuBIhwkjH43959414 = -585021658;    double trfYsRscQuBIhwkjH92304972 = -159851107;    double trfYsRscQuBIhwkjH31406116 = -459562601;    double trfYsRscQuBIhwkjH11145475 = -173859118;    double trfYsRscQuBIhwkjH41221684 = -971083214;    double trfYsRscQuBIhwkjH53199722 = -595688183;    double trfYsRscQuBIhwkjH31988497 = -356518924;    double trfYsRscQuBIhwkjH41114690 = -771156519;    double trfYsRscQuBIhwkjH72684064 = -380900478;    double trfYsRscQuBIhwkjH79305707 = -91984777;    double trfYsRscQuBIhwkjH42638171 = -68030175;    double trfYsRscQuBIhwkjH46096375 = -411373825;    double trfYsRscQuBIhwkjH10126220 = -185741762;    double trfYsRscQuBIhwkjH1938933 = 66645143;    double trfYsRscQuBIhwkjH57683808 = -927121671;    double trfYsRscQuBIhwkjH15777671 = -180859569;    double trfYsRscQuBIhwkjH78325549 = -181335893;    double trfYsRscQuBIhwkjH63823442 = -219876932;    double trfYsRscQuBIhwkjH87724673 = -666790780;    double trfYsRscQuBIhwkjH43908707 = -33579337;    double trfYsRscQuBIhwkjH94452047 = -812152208;    double trfYsRscQuBIhwkjH98230376 = -951604605;    double trfYsRscQuBIhwkjH79064506 = -559205139;    double trfYsRscQuBIhwkjH70568805 = -240915756;    double trfYsRscQuBIhwkjH84952419 = -862383071;    double trfYsRscQuBIhwkjH10536207 = 28733764;    double trfYsRscQuBIhwkjH70235929 = -875819540;    double trfYsRscQuBIhwkjH55572595 = -244749136;    double trfYsRscQuBIhwkjH23527008 = -307884530;    double trfYsRscQuBIhwkjH5429376 = -461788038;    double trfYsRscQuBIhwkjH25423066 = -801521786;    double trfYsRscQuBIhwkjH65102643 = 1366630;    double trfYsRscQuBIhwkjH28094575 = 37037380;    double trfYsRscQuBIhwkjH27359 = -113461674;    double trfYsRscQuBIhwkjH89330156 = 6500128;    double trfYsRscQuBIhwkjH79206821 = -475486980;    double trfYsRscQuBIhwkjH41546865 = -346976117;    double trfYsRscQuBIhwkjH46614736 = -704230070;    double trfYsRscQuBIhwkjH62924875 = -515575346;    double trfYsRscQuBIhwkjH3884439 = -997719454;    double trfYsRscQuBIhwkjH85698124 = -917192679;    double trfYsRscQuBIhwkjH46998128 = -563800469;    double trfYsRscQuBIhwkjH37225868 = -369475125;    double trfYsRscQuBIhwkjH19335512 = -37722616;    double trfYsRscQuBIhwkjH83754969 = -352357528;    double trfYsRscQuBIhwkjH55727103 = -862996260;    double trfYsRscQuBIhwkjH86476033 = -320866058;     trfYsRscQuBIhwkjH28368062 = trfYsRscQuBIhwkjH97921371;     trfYsRscQuBIhwkjH97921371 = trfYsRscQuBIhwkjH48360143;     trfYsRscQuBIhwkjH48360143 = trfYsRscQuBIhwkjH87114033;     trfYsRscQuBIhwkjH87114033 = trfYsRscQuBIhwkjH98825601;     trfYsRscQuBIhwkjH98825601 = trfYsRscQuBIhwkjH57278973;     trfYsRscQuBIhwkjH57278973 = trfYsRscQuBIhwkjH24905422;     trfYsRscQuBIhwkjH24905422 = trfYsRscQuBIhwkjH27949062;     trfYsRscQuBIhwkjH27949062 = trfYsRscQuBIhwkjH109676;     trfYsRscQuBIhwkjH109676 = trfYsRscQuBIhwkjH32592014;     trfYsRscQuBIhwkjH32592014 = trfYsRscQuBIhwkjH34606383;     trfYsRscQuBIhwkjH34606383 = trfYsRscQuBIhwkjH35228503;     trfYsRscQuBIhwkjH35228503 = trfYsRscQuBIhwkjH29195602;     trfYsRscQuBIhwkjH29195602 = trfYsRscQuBIhwkjH88730652;     trfYsRscQuBIhwkjH88730652 = trfYsRscQuBIhwkjH36085761;     trfYsRscQuBIhwkjH36085761 = trfYsRscQuBIhwkjH67970001;     trfYsRscQuBIhwkjH67970001 = trfYsRscQuBIhwkjH44808834;     trfYsRscQuBIhwkjH44808834 = trfYsRscQuBIhwkjH61595327;     trfYsRscQuBIhwkjH61595327 = trfYsRscQuBIhwkjH82564865;     trfYsRscQuBIhwkjH82564865 = trfYsRscQuBIhwkjH8140309;     trfYsRscQuBIhwkjH8140309 = trfYsRscQuBIhwkjH77705725;     trfYsRscQuBIhwkjH77705725 = trfYsRscQuBIhwkjH96961945;     trfYsRscQuBIhwkjH96961945 = trfYsRscQuBIhwkjH86775897;     trfYsRscQuBIhwkjH86775897 = trfYsRscQuBIhwkjH7138460;     trfYsRscQuBIhwkjH7138460 = trfYsRscQuBIhwkjH33914311;     trfYsRscQuBIhwkjH33914311 = trfYsRscQuBIhwkjH66837105;     trfYsRscQuBIhwkjH66837105 = trfYsRscQuBIhwkjH16164283;     trfYsRscQuBIhwkjH16164283 = trfYsRscQuBIhwkjH52221358;     trfYsRscQuBIhwkjH52221358 = trfYsRscQuBIhwkjH48643355;     trfYsRscQuBIhwkjH48643355 = trfYsRscQuBIhwkjH57471504;     trfYsRscQuBIhwkjH57471504 = trfYsRscQuBIhwkjH86495639;     trfYsRscQuBIhwkjH86495639 = trfYsRscQuBIhwkjH24480163;     trfYsRscQuBIhwkjH24480163 = trfYsRscQuBIhwkjH33289570;     trfYsRscQuBIhwkjH33289570 = trfYsRscQuBIhwkjH71511794;     trfYsRscQuBIhwkjH71511794 = trfYsRscQuBIhwkjH72952981;     trfYsRscQuBIhwkjH72952981 = trfYsRscQuBIhwkjH57760211;     trfYsRscQuBIhwkjH57760211 = trfYsRscQuBIhwkjH4146559;     trfYsRscQuBIhwkjH4146559 = trfYsRscQuBIhwkjH57084160;     trfYsRscQuBIhwkjH57084160 = trfYsRscQuBIhwkjH17686621;     trfYsRscQuBIhwkjH17686621 = trfYsRscQuBIhwkjH88112818;     trfYsRscQuBIhwkjH88112818 = trfYsRscQuBIhwkjH9909933;     trfYsRscQuBIhwkjH9909933 = trfYsRscQuBIhwkjH98641218;     trfYsRscQuBIhwkjH98641218 = trfYsRscQuBIhwkjH26393141;     trfYsRscQuBIhwkjH26393141 = trfYsRscQuBIhwkjH1823478;     trfYsRscQuBIhwkjH1823478 = trfYsRscQuBIhwkjH96602252;     trfYsRscQuBIhwkjH96602252 = trfYsRscQuBIhwkjH63678382;     trfYsRscQuBIhwkjH63678382 = trfYsRscQuBIhwkjH11264511;     trfYsRscQuBIhwkjH11264511 = trfYsRscQuBIhwkjH92637274;     trfYsRscQuBIhwkjH92637274 = trfYsRscQuBIhwkjH46791983;     trfYsRscQuBIhwkjH46791983 = trfYsRscQuBIhwkjH23220290;     trfYsRscQuBIhwkjH23220290 = trfYsRscQuBIhwkjH92368861;     trfYsRscQuBIhwkjH92368861 = trfYsRscQuBIhwkjH58401064;     trfYsRscQuBIhwkjH58401064 = trfYsRscQuBIhwkjH24452805;     trfYsRscQuBIhwkjH24452805 = trfYsRscQuBIhwkjH43959414;     trfYsRscQuBIhwkjH43959414 = trfYsRscQuBIhwkjH92304972;     trfYsRscQuBIhwkjH92304972 = trfYsRscQuBIhwkjH31406116;     trfYsRscQuBIhwkjH31406116 = trfYsRscQuBIhwkjH11145475;     trfYsRscQuBIhwkjH11145475 = trfYsRscQuBIhwkjH41221684;     trfYsRscQuBIhwkjH41221684 = trfYsRscQuBIhwkjH53199722;     trfYsRscQuBIhwkjH53199722 = trfYsRscQuBIhwkjH31988497;     trfYsRscQuBIhwkjH31988497 = trfYsRscQuBIhwkjH41114690;     trfYsRscQuBIhwkjH41114690 = trfYsRscQuBIhwkjH72684064;     trfYsRscQuBIhwkjH72684064 = trfYsRscQuBIhwkjH79305707;     trfYsRscQuBIhwkjH79305707 = trfYsRscQuBIhwkjH42638171;     trfYsRscQuBIhwkjH42638171 = trfYsRscQuBIhwkjH46096375;     trfYsRscQuBIhwkjH46096375 = trfYsRscQuBIhwkjH10126220;     trfYsRscQuBIhwkjH10126220 = trfYsRscQuBIhwkjH1938933;     trfYsRscQuBIhwkjH1938933 = trfYsRscQuBIhwkjH57683808;     trfYsRscQuBIhwkjH57683808 = trfYsRscQuBIhwkjH15777671;     trfYsRscQuBIhwkjH15777671 = trfYsRscQuBIhwkjH78325549;     trfYsRscQuBIhwkjH78325549 = trfYsRscQuBIhwkjH63823442;     trfYsRscQuBIhwkjH63823442 = trfYsRscQuBIhwkjH87724673;     trfYsRscQuBIhwkjH87724673 = trfYsRscQuBIhwkjH43908707;     trfYsRscQuBIhwkjH43908707 = trfYsRscQuBIhwkjH94452047;     trfYsRscQuBIhwkjH94452047 = trfYsRscQuBIhwkjH98230376;     trfYsRscQuBIhwkjH98230376 = trfYsRscQuBIhwkjH79064506;     trfYsRscQuBIhwkjH79064506 = trfYsRscQuBIhwkjH70568805;     trfYsRscQuBIhwkjH70568805 = trfYsRscQuBIhwkjH84952419;     trfYsRscQuBIhwkjH84952419 = trfYsRscQuBIhwkjH10536207;     trfYsRscQuBIhwkjH10536207 = trfYsRscQuBIhwkjH70235929;     trfYsRscQuBIhwkjH70235929 = trfYsRscQuBIhwkjH55572595;     trfYsRscQuBIhwkjH55572595 = trfYsRscQuBIhwkjH23527008;     trfYsRscQuBIhwkjH23527008 = trfYsRscQuBIhwkjH5429376;     trfYsRscQuBIhwkjH5429376 = trfYsRscQuBIhwkjH25423066;     trfYsRscQuBIhwkjH25423066 = trfYsRscQuBIhwkjH65102643;     trfYsRscQuBIhwkjH65102643 = trfYsRscQuBIhwkjH28094575;     trfYsRscQuBIhwkjH28094575 = trfYsRscQuBIhwkjH27359;     trfYsRscQuBIhwkjH27359 = trfYsRscQuBIhwkjH89330156;     trfYsRscQuBIhwkjH89330156 = trfYsRscQuBIhwkjH79206821;     trfYsRscQuBIhwkjH79206821 = trfYsRscQuBIhwkjH41546865;     trfYsRscQuBIhwkjH41546865 = trfYsRscQuBIhwkjH46614736;     trfYsRscQuBIhwkjH46614736 = trfYsRscQuBIhwkjH62924875;     trfYsRscQuBIhwkjH62924875 = trfYsRscQuBIhwkjH3884439;     trfYsRscQuBIhwkjH3884439 = trfYsRscQuBIhwkjH85698124;     trfYsRscQuBIhwkjH85698124 = trfYsRscQuBIhwkjH46998128;     trfYsRscQuBIhwkjH46998128 = trfYsRscQuBIhwkjH37225868;     trfYsRscQuBIhwkjH37225868 = trfYsRscQuBIhwkjH19335512;     trfYsRscQuBIhwkjH19335512 = trfYsRscQuBIhwkjH83754969;     trfYsRscQuBIhwkjH83754969 = trfYsRscQuBIhwkjH55727103;     trfYsRscQuBIhwkjH55727103 = trfYsRscQuBIhwkjH86476033;     trfYsRscQuBIhwkjH86476033 = trfYsRscQuBIhwkjH28368062;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void IZMhQmjsKxotJPCf85059765() {     double JDJoUsldyrzrtqnvp34485168 = -432767947;    double JDJoUsldyrzrtqnvp41294651 = -120212053;    double JDJoUsldyrzrtqnvp64402326 = -305377278;    double JDJoUsldyrzrtqnvp14188814 = -437617868;    double JDJoUsldyrzrtqnvp67721090 = -867284551;    double JDJoUsldyrzrtqnvp48696175 = -113410172;    double JDJoUsldyrzrtqnvp85394388 = -979549048;    double JDJoUsldyrzrtqnvp77708953 = -956865579;    double JDJoUsldyrzrtqnvp64752171 = -265013248;    double JDJoUsldyrzrtqnvp73821673 = -83330356;    double JDJoUsldyrzrtqnvp94716672 = -208227162;    double JDJoUsldyrzrtqnvp67051545 = -56282588;    double JDJoUsldyrzrtqnvp15404720 = -2350847;    double JDJoUsldyrzrtqnvp71874329 = -619636854;    double JDJoUsldyrzrtqnvp31828188 = -849887247;    double JDJoUsldyrzrtqnvp9475395 = -779535034;    double JDJoUsldyrzrtqnvp38644837 = -876743530;    double JDJoUsldyrzrtqnvp58648480 = -72813583;    double JDJoUsldyrzrtqnvp44394707 = -552878292;    double JDJoUsldyrzrtqnvp88690473 = -58883126;    double JDJoUsldyrzrtqnvp66258918 = -744399986;    double JDJoUsldyrzrtqnvp83482969 = -769233175;    double JDJoUsldyrzrtqnvp22366689 = -983270641;    double JDJoUsldyrzrtqnvp91130833 = -405766130;    double JDJoUsldyrzrtqnvp77613961 = 75622912;    double JDJoUsldyrzrtqnvp91558780 = -868771079;    double JDJoUsldyrzrtqnvp14624061 = 21580378;    double JDJoUsldyrzrtqnvp65328483 = -425612547;    double JDJoUsldyrzrtqnvp14775611 = -700389594;    double JDJoUsldyrzrtqnvp26087141 = -146421275;    double JDJoUsldyrzrtqnvp78738188 = -256301993;    double JDJoUsldyrzrtqnvp22090785 = -854494630;    double JDJoUsldyrzrtqnvp43000519 = -571134900;    double JDJoUsldyrzrtqnvp66342358 = -486879852;    double JDJoUsldyrzrtqnvp34427634 = -445271823;    double JDJoUsldyrzrtqnvp26486125 = -92115296;    double JDJoUsldyrzrtqnvp85408493 = -440294429;    double JDJoUsldyrzrtqnvp91174373 = -364913647;    double JDJoUsldyrzrtqnvp91296874 = -267635100;    double JDJoUsldyrzrtqnvp75614756 = -808396210;    double JDJoUsldyrzrtqnvp15579157 = -915649497;    double JDJoUsldyrzrtqnvp30791676 = -499849671;    double JDJoUsldyrzrtqnvp4460119 = -601192287;    double JDJoUsldyrzrtqnvp89231218 = -293713022;    double JDJoUsldyrzrtqnvp8554718 = -874093061;    double JDJoUsldyrzrtqnvp75617577 = -760577631;    double JDJoUsldyrzrtqnvp29043085 = -764142159;    double JDJoUsldyrzrtqnvp594216 = -779768421;    double JDJoUsldyrzrtqnvp56467712 = -983632543;    double JDJoUsldyrzrtqnvp36131943 = -763495912;    double JDJoUsldyrzrtqnvp28069540 = -72610709;    double JDJoUsldyrzrtqnvp69210783 = -80744755;    double JDJoUsldyrzrtqnvp72701262 = -723375728;    double JDJoUsldyrzrtqnvp39928295 = -905491803;    double JDJoUsldyrzrtqnvp18800847 = -860382890;    double JDJoUsldyrzrtqnvp51002199 = -663534773;    double JDJoUsldyrzrtqnvp18927963 = -136941412;    double JDJoUsldyrzrtqnvp73271493 = -899611148;    double JDJoUsldyrzrtqnvp36574853 = -413240781;    double JDJoUsldyrzrtqnvp76162309 = -998513472;    double JDJoUsldyrzrtqnvp34072115 = -34990551;    double JDJoUsldyrzrtqnvp20065905 = -453936502;    double JDJoUsldyrzrtqnvp62933342 = -156475986;    double JDJoUsldyrzrtqnvp38665030 = -18591974;    double JDJoUsldyrzrtqnvp95083484 = -827028364;    double JDJoUsldyrzrtqnvp72625888 = -353732533;    double JDJoUsldyrzrtqnvp24051027 = -485147688;    double JDJoUsldyrzrtqnvp49062362 = -515470995;    double JDJoUsldyrzrtqnvp37446695 = -74365031;    double JDJoUsldyrzrtqnvp5342064 = -657771952;    double JDJoUsldyrzrtqnvp24066902 = -239240606;    double JDJoUsldyrzrtqnvp47470464 = -411829883;    double JDJoUsldyrzrtqnvp67351606 = -805178483;    double JDJoUsldyrzrtqnvp68779951 = -744482083;    double JDJoUsldyrzrtqnvp73111317 = -143233630;    double JDJoUsldyrzrtqnvp35467243 = -144550316;    double JDJoUsldyrzrtqnvp79022850 = -68040888;    double JDJoUsldyrzrtqnvp33135470 = -589557620;    double JDJoUsldyrzrtqnvp82576115 = -531673070;    double JDJoUsldyrzrtqnvp1996385 = -163799457;    double JDJoUsldyrzrtqnvp62515696 = -4628921;    double JDJoUsldyrzrtqnvp14029845 = -198651201;    double JDJoUsldyrzrtqnvp8860772 = -441980004;    double JDJoUsldyrzrtqnvp78643668 = -936893682;    double JDJoUsldyrzrtqnvp98017601 = 26189434;    double JDJoUsldyrzrtqnvp9527406 = -75557238;    double JDJoUsldyrzrtqnvp49389523 = -31118902;    double JDJoUsldyrzrtqnvp3072225 = -665643098;    double JDJoUsldyrzrtqnvp47541511 = -626496963;    double JDJoUsldyrzrtqnvp83425434 = -781737050;    double JDJoUsldyrzrtqnvp7558162 = -955173884;    double JDJoUsldyrzrtqnvp12137000 = -540683282;    double JDJoUsldyrzrtqnvp54599520 = -951672867;    double JDJoUsldyrzrtqnvp15134565 = -269121628;    double JDJoUsldyrzrtqnvp41542641 = -673405660;    double JDJoUsldyrzrtqnvp95513251 = -361712996;    double JDJoUsldyrzrtqnvp67858333 = -243373685;    double JDJoUsldyrzrtqnvp65795089 = -482600314;    double JDJoUsldyrzrtqnvp94147733 = -466684658;    double JDJoUsldyrzrtqnvp35928830 = -432767947;     JDJoUsldyrzrtqnvp34485168 = JDJoUsldyrzrtqnvp41294651;     JDJoUsldyrzrtqnvp41294651 = JDJoUsldyrzrtqnvp64402326;     JDJoUsldyrzrtqnvp64402326 = JDJoUsldyrzrtqnvp14188814;     JDJoUsldyrzrtqnvp14188814 = JDJoUsldyrzrtqnvp67721090;     JDJoUsldyrzrtqnvp67721090 = JDJoUsldyrzrtqnvp48696175;     JDJoUsldyrzrtqnvp48696175 = JDJoUsldyrzrtqnvp85394388;     JDJoUsldyrzrtqnvp85394388 = JDJoUsldyrzrtqnvp77708953;     JDJoUsldyrzrtqnvp77708953 = JDJoUsldyrzrtqnvp64752171;     JDJoUsldyrzrtqnvp64752171 = JDJoUsldyrzrtqnvp73821673;     JDJoUsldyrzrtqnvp73821673 = JDJoUsldyrzrtqnvp94716672;     JDJoUsldyrzrtqnvp94716672 = JDJoUsldyrzrtqnvp67051545;     JDJoUsldyrzrtqnvp67051545 = JDJoUsldyrzrtqnvp15404720;     JDJoUsldyrzrtqnvp15404720 = JDJoUsldyrzrtqnvp71874329;     JDJoUsldyrzrtqnvp71874329 = JDJoUsldyrzrtqnvp31828188;     JDJoUsldyrzrtqnvp31828188 = JDJoUsldyrzrtqnvp9475395;     JDJoUsldyrzrtqnvp9475395 = JDJoUsldyrzrtqnvp38644837;     JDJoUsldyrzrtqnvp38644837 = JDJoUsldyrzrtqnvp58648480;     JDJoUsldyrzrtqnvp58648480 = JDJoUsldyrzrtqnvp44394707;     JDJoUsldyrzrtqnvp44394707 = JDJoUsldyrzrtqnvp88690473;     JDJoUsldyrzrtqnvp88690473 = JDJoUsldyrzrtqnvp66258918;     JDJoUsldyrzrtqnvp66258918 = JDJoUsldyrzrtqnvp83482969;     JDJoUsldyrzrtqnvp83482969 = JDJoUsldyrzrtqnvp22366689;     JDJoUsldyrzrtqnvp22366689 = JDJoUsldyrzrtqnvp91130833;     JDJoUsldyrzrtqnvp91130833 = JDJoUsldyrzrtqnvp77613961;     JDJoUsldyrzrtqnvp77613961 = JDJoUsldyrzrtqnvp91558780;     JDJoUsldyrzrtqnvp91558780 = JDJoUsldyrzrtqnvp14624061;     JDJoUsldyrzrtqnvp14624061 = JDJoUsldyrzrtqnvp65328483;     JDJoUsldyrzrtqnvp65328483 = JDJoUsldyrzrtqnvp14775611;     JDJoUsldyrzrtqnvp14775611 = JDJoUsldyrzrtqnvp26087141;     JDJoUsldyrzrtqnvp26087141 = JDJoUsldyrzrtqnvp78738188;     JDJoUsldyrzrtqnvp78738188 = JDJoUsldyrzrtqnvp22090785;     JDJoUsldyrzrtqnvp22090785 = JDJoUsldyrzrtqnvp43000519;     JDJoUsldyrzrtqnvp43000519 = JDJoUsldyrzrtqnvp66342358;     JDJoUsldyrzrtqnvp66342358 = JDJoUsldyrzrtqnvp34427634;     JDJoUsldyrzrtqnvp34427634 = JDJoUsldyrzrtqnvp26486125;     JDJoUsldyrzrtqnvp26486125 = JDJoUsldyrzrtqnvp85408493;     JDJoUsldyrzrtqnvp85408493 = JDJoUsldyrzrtqnvp91174373;     JDJoUsldyrzrtqnvp91174373 = JDJoUsldyrzrtqnvp91296874;     JDJoUsldyrzrtqnvp91296874 = JDJoUsldyrzrtqnvp75614756;     JDJoUsldyrzrtqnvp75614756 = JDJoUsldyrzrtqnvp15579157;     JDJoUsldyrzrtqnvp15579157 = JDJoUsldyrzrtqnvp30791676;     JDJoUsldyrzrtqnvp30791676 = JDJoUsldyrzrtqnvp4460119;     JDJoUsldyrzrtqnvp4460119 = JDJoUsldyrzrtqnvp89231218;     JDJoUsldyrzrtqnvp89231218 = JDJoUsldyrzrtqnvp8554718;     JDJoUsldyrzrtqnvp8554718 = JDJoUsldyrzrtqnvp75617577;     JDJoUsldyrzrtqnvp75617577 = JDJoUsldyrzrtqnvp29043085;     JDJoUsldyrzrtqnvp29043085 = JDJoUsldyrzrtqnvp594216;     JDJoUsldyrzrtqnvp594216 = JDJoUsldyrzrtqnvp56467712;     JDJoUsldyrzrtqnvp56467712 = JDJoUsldyrzrtqnvp36131943;     JDJoUsldyrzrtqnvp36131943 = JDJoUsldyrzrtqnvp28069540;     JDJoUsldyrzrtqnvp28069540 = JDJoUsldyrzrtqnvp69210783;     JDJoUsldyrzrtqnvp69210783 = JDJoUsldyrzrtqnvp72701262;     JDJoUsldyrzrtqnvp72701262 = JDJoUsldyrzrtqnvp39928295;     JDJoUsldyrzrtqnvp39928295 = JDJoUsldyrzrtqnvp18800847;     JDJoUsldyrzrtqnvp18800847 = JDJoUsldyrzrtqnvp51002199;     JDJoUsldyrzrtqnvp51002199 = JDJoUsldyrzrtqnvp18927963;     JDJoUsldyrzrtqnvp18927963 = JDJoUsldyrzrtqnvp73271493;     JDJoUsldyrzrtqnvp73271493 = JDJoUsldyrzrtqnvp36574853;     JDJoUsldyrzrtqnvp36574853 = JDJoUsldyrzrtqnvp76162309;     JDJoUsldyrzrtqnvp76162309 = JDJoUsldyrzrtqnvp34072115;     JDJoUsldyrzrtqnvp34072115 = JDJoUsldyrzrtqnvp20065905;     JDJoUsldyrzrtqnvp20065905 = JDJoUsldyrzrtqnvp62933342;     JDJoUsldyrzrtqnvp62933342 = JDJoUsldyrzrtqnvp38665030;     JDJoUsldyrzrtqnvp38665030 = JDJoUsldyrzrtqnvp95083484;     JDJoUsldyrzrtqnvp95083484 = JDJoUsldyrzrtqnvp72625888;     JDJoUsldyrzrtqnvp72625888 = JDJoUsldyrzrtqnvp24051027;     JDJoUsldyrzrtqnvp24051027 = JDJoUsldyrzrtqnvp49062362;     JDJoUsldyrzrtqnvp49062362 = JDJoUsldyrzrtqnvp37446695;     JDJoUsldyrzrtqnvp37446695 = JDJoUsldyrzrtqnvp5342064;     JDJoUsldyrzrtqnvp5342064 = JDJoUsldyrzrtqnvp24066902;     JDJoUsldyrzrtqnvp24066902 = JDJoUsldyrzrtqnvp47470464;     JDJoUsldyrzrtqnvp47470464 = JDJoUsldyrzrtqnvp67351606;     JDJoUsldyrzrtqnvp67351606 = JDJoUsldyrzrtqnvp68779951;     JDJoUsldyrzrtqnvp68779951 = JDJoUsldyrzrtqnvp73111317;     JDJoUsldyrzrtqnvp73111317 = JDJoUsldyrzrtqnvp35467243;     JDJoUsldyrzrtqnvp35467243 = JDJoUsldyrzrtqnvp79022850;     JDJoUsldyrzrtqnvp79022850 = JDJoUsldyrzrtqnvp33135470;     JDJoUsldyrzrtqnvp33135470 = JDJoUsldyrzrtqnvp82576115;     JDJoUsldyrzrtqnvp82576115 = JDJoUsldyrzrtqnvp1996385;     JDJoUsldyrzrtqnvp1996385 = JDJoUsldyrzrtqnvp62515696;     JDJoUsldyrzrtqnvp62515696 = JDJoUsldyrzrtqnvp14029845;     JDJoUsldyrzrtqnvp14029845 = JDJoUsldyrzrtqnvp8860772;     JDJoUsldyrzrtqnvp8860772 = JDJoUsldyrzrtqnvp78643668;     JDJoUsldyrzrtqnvp78643668 = JDJoUsldyrzrtqnvp98017601;     JDJoUsldyrzrtqnvp98017601 = JDJoUsldyrzrtqnvp9527406;     JDJoUsldyrzrtqnvp9527406 = JDJoUsldyrzrtqnvp49389523;     JDJoUsldyrzrtqnvp49389523 = JDJoUsldyrzrtqnvp3072225;     JDJoUsldyrzrtqnvp3072225 = JDJoUsldyrzrtqnvp47541511;     JDJoUsldyrzrtqnvp47541511 = JDJoUsldyrzrtqnvp83425434;     JDJoUsldyrzrtqnvp83425434 = JDJoUsldyrzrtqnvp7558162;     JDJoUsldyrzrtqnvp7558162 = JDJoUsldyrzrtqnvp12137000;     JDJoUsldyrzrtqnvp12137000 = JDJoUsldyrzrtqnvp54599520;     JDJoUsldyrzrtqnvp54599520 = JDJoUsldyrzrtqnvp15134565;     JDJoUsldyrzrtqnvp15134565 = JDJoUsldyrzrtqnvp41542641;     JDJoUsldyrzrtqnvp41542641 = JDJoUsldyrzrtqnvp95513251;     JDJoUsldyrzrtqnvp95513251 = JDJoUsldyrzrtqnvp67858333;     JDJoUsldyrzrtqnvp67858333 = JDJoUsldyrzrtqnvp65795089;     JDJoUsldyrzrtqnvp65795089 = JDJoUsldyrzrtqnvp94147733;     JDJoUsldyrzrtqnvp94147733 = JDJoUsldyrzrtqnvp35928830;     JDJoUsldyrzrtqnvp35928830 = JDJoUsldyrzrtqnvp34485168;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NaVNCbpCSxdjKPmH52351365() {     double CnBaDMbPaCxlWzZHj69944137 = -899044847;    double CnBaDMbPaCxlWzZHj54566729 = -763659;    double CnBaDMbPaCxlWzZHj1788804 = 28955647;    double CnBaDMbPaCxlWzZHj86412168 = -258768939;    double CnBaDMbPaCxlWzZHj24006359 = -972883397;    double CnBaDMbPaCxlWzZHj84919047 = -375758067;    double CnBaDMbPaCxlWzZHj96051794 = -824274782;    double CnBaDMbPaCxlWzZHj39840514 = -601099720;    double CnBaDMbPaCxlWzZHj66269776 = -753344655;    double CnBaDMbPaCxlWzZHj88724385 = -848017354;    double CnBaDMbPaCxlWzZHj32898408 = -10234891;    double CnBaDMbPaCxlWzZHj67619694 = 79596641;    double CnBaDMbPaCxlWzZHj53639365 = -554363352;    double CnBaDMbPaCxlWzZHj25015192 = -596000263;    double CnBaDMbPaCxlWzZHj90022703 = -253312262;    double CnBaDMbPaCxlWzZHj93016695 = -415346642;    double CnBaDMbPaCxlWzZHj94480772 = -303718082;    double CnBaDMbPaCxlWzZHj58408672 = -146241651;    double CnBaDMbPaCxlWzZHj44945037 = -869441714;    double CnBaDMbPaCxlWzZHj82527692 = -485444192;    double CnBaDMbPaCxlWzZHj79345781 = -401674675;    double CnBaDMbPaCxlWzZHj86904493 = -855869469;    double CnBaDMbPaCxlWzZHj92892068 = -620902825;    double CnBaDMbPaCxlWzZHj84923778 = -916590735;    double CnBaDMbPaCxlWzZHj85631912 = 51637714;    double CnBaDMbPaCxlWzZHj2256952 = -557347886;    double CnBaDMbPaCxlWzZHj75044634 = -884703762;    double CnBaDMbPaCxlWzZHj6056908 = -595384721;    double CnBaDMbPaCxlWzZHj61443825 = -174484396;    double CnBaDMbPaCxlWzZHj31620552 = -180928445;    double CnBaDMbPaCxlWzZHj97321420 = -808282480;    double CnBaDMbPaCxlWzZHj59913803 = -627466044;    double CnBaDMbPaCxlWzZHj55218530 = -718730877;    double CnBaDMbPaCxlWzZHj96090784 = -551573666;    double CnBaDMbPaCxlWzZHj236088 = -396372074;    double CnBaDMbPaCxlWzZHj78046968 = -306680601;    double CnBaDMbPaCxlWzZHj34820310 = -304990470;    double CnBaDMbPaCxlWzZHj42389777 = -347569228;    double CnBaDMbPaCxlWzZHj21687436 = 81488624;    double CnBaDMbPaCxlWzZHj39886288 = -273658756;    double CnBaDMbPaCxlWzZHj49030764 = -684848411;    double CnBaDMbPaCxlWzZHj20156803 = -94394962;    double CnBaDMbPaCxlWzZHj93843497 = -154373073;    double CnBaDMbPaCxlWzZHj29362557 = -898045432;    double CnBaDMbPaCxlWzZHj8571176 = -700604223;    double CnBaDMbPaCxlWzZHj21578906 = -799501264;    double CnBaDMbPaCxlWzZHj48388248 = -148128519;    double CnBaDMbPaCxlWzZHj18320528 = -946295015;    double CnBaDMbPaCxlWzZHj79360377 = -647369423;    double CnBaDMbPaCxlWzZHj60661532 = -154522728;    double CnBaDMbPaCxlWzZHj8840264 = 53112733;    double CnBaDMbPaCxlWzZHj52696256 = -294397102;    double CnBaDMbPaCxlWzZHj41322411 = -479421282;    double CnBaDMbPaCxlWzZHj55920141 = 2999698;    double CnBaDMbPaCxlWzZHj73005583 = -106873036;    double CnBaDMbPaCxlWzZHj83039643 = 56824621;    double CnBaDMbPaCxlWzZHj61674661 = -379860834;    double CnBaDMbPaCxlWzZHj16865025 = -54453619;    double CnBaDMbPaCxlWzZHj780257 = -210406654;    double CnBaDMbPaCxlWzZHj21749407 = -315535511;    double CnBaDMbPaCxlWzZHj9874413 = -491054305;    double CnBaDMbPaCxlWzZHj89994886 = -128890062;    double CnBaDMbPaCxlWzZHj78396689 = -326615324;    double CnBaDMbPaCxlWzZHj34649224 = -472416210;    double CnBaDMbPaCxlWzZHj91402965 = 60265125;    double CnBaDMbPaCxlWzZHj72984604 = -382768848;    double CnBaDMbPaCxlWzZHj12401164 = -201672482;    double CnBaDMbPaCxlWzZHj57548581 = 97210314;    double CnBaDMbPaCxlWzZHj24779105 = -99628190;    double CnBaDMbPaCxlWzZHj11975736 = -946631661;    double CnBaDMbPaCxlWzZHj58196386 = -10356172;    double CnBaDMbPaCxlWzZHj52090996 = -956148855;    double CnBaDMbPaCxlWzZHj36721236 = -127730275;    double CnBaDMbPaCxlWzZHj5058749 = -495782958;    double CnBaDMbPaCxlWzZHj33496928 = -800595782;    double CnBaDMbPaCxlWzZHj59188979 = -207279714;    double CnBaDMbPaCxlWzZHj93060996 = -601496396;    double CnBaDMbPaCxlWzZHj63529512 = -722857394;    double CnBaDMbPaCxlWzZHj76352602 = -115986513;    double CnBaDMbPaCxlWzZHj64053007 = -148861023;    double CnBaDMbPaCxlWzZHj53868704 = -309219367;    double CnBaDMbPaCxlWzZHj56724107 = -938408748;    double CnBaDMbPaCxlWzZHj26696531 = -948015298;    double CnBaDMbPaCxlWzZHj782294 = 80038331;    double CnBaDMbPaCxlWzZHj22780289 = -134041179;    double CnBaDMbPaCxlWzZHj44625164 = -413885379;    double CnBaDMbPaCxlWzZHj18591393 = -48044763;    double CnBaDMbPaCxlWzZHj99298388 = -621730575;    double CnBaDMbPaCxlWzZHj23085202 = -344700631;    double CnBaDMbPaCxlWzZHj17196445 = -353196695;    double CnBaDMbPaCxlWzZHj16372307 = -926819768;    double CnBaDMbPaCxlWzZHj17955285 = -150536852;    double CnBaDMbPaCxlWzZHj41609520 = -37162575;    double CnBaDMbPaCxlWzZHj99938028 = -602975866;    double CnBaDMbPaCxlWzZHj30011876 = -782604451;    double CnBaDMbPaCxlWzZHj59035877 = -455958350;    double CnBaDMbPaCxlWzZHj41760114 = -767779638;    double CnBaDMbPaCxlWzZHj59194273 = -681956863;    double CnBaDMbPaCxlWzZHj37959592 = -858310557;    double CnBaDMbPaCxlWzZHj35586572 = -899044847;     CnBaDMbPaCxlWzZHj69944137 = CnBaDMbPaCxlWzZHj54566729;     CnBaDMbPaCxlWzZHj54566729 = CnBaDMbPaCxlWzZHj1788804;     CnBaDMbPaCxlWzZHj1788804 = CnBaDMbPaCxlWzZHj86412168;     CnBaDMbPaCxlWzZHj86412168 = CnBaDMbPaCxlWzZHj24006359;     CnBaDMbPaCxlWzZHj24006359 = CnBaDMbPaCxlWzZHj84919047;     CnBaDMbPaCxlWzZHj84919047 = CnBaDMbPaCxlWzZHj96051794;     CnBaDMbPaCxlWzZHj96051794 = CnBaDMbPaCxlWzZHj39840514;     CnBaDMbPaCxlWzZHj39840514 = CnBaDMbPaCxlWzZHj66269776;     CnBaDMbPaCxlWzZHj66269776 = CnBaDMbPaCxlWzZHj88724385;     CnBaDMbPaCxlWzZHj88724385 = CnBaDMbPaCxlWzZHj32898408;     CnBaDMbPaCxlWzZHj32898408 = CnBaDMbPaCxlWzZHj67619694;     CnBaDMbPaCxlWzZHj67619694 = CnBaDMbPaCxlWzZHj53639365;     CnBaDMbPaCxlWzZHj53639365 = CnBaDMbPaCxlWzZHj25015192;     CnBaDMbPaCxlWzZHj25015192 = CnBaDMbPaCxlWzZHj90022703;     CnBaDMbPaCxlWzZHj90022703 = CnBaDMbPaCxlWzZHj93016695;     CnBaDMbPaCxlWzZHj93016695 = CnBaDMbPaCxlWzZHj94480772;     CnBaDMbPaCxlWzZHj94480772 = CnBaDMbPaCxlWzZHj58408672;     CnBaDMbPaCxlWzZHj58408672 = CnBaDMbPaCxlWzZHj44945037;     CnBaDMbPaCxlWzZHj44945037 = CnBaDMbPaCxlWzZHj82527692;     CnBaDMbPaCxlWzZHj82527692 = CnBaDMbPaCxlWzZHj79345781;     CnBaDMbPaCxlWzZHj79345781 = CnBaDMbPaCxlWzZHj86904493;     CnBaDMbPaCxlWzZHj86904493 = CnBaDMbPaCxlWzZHj92892068;     CnBaDMbPaCxlWzZHj92892068 = CnBaDMbPaCxlWzZHj84923778;     CnBaDMbPaCxlWzZHj84923778 = CnBaDMbPaCxlWzZHj85631912;     CnBaDMbPaCxlWzZHj85631912 = CnBaDMbPaCxlWzZHj2256952;     CnBaDMbPaCxlWzZHj2256952 = CnBaDMbPaCxlWzZHj75044634;     CnBaDMbPaCxlWzZHj75044634 = CnBaDMbPaCxlWzZHj6056908;     CnBaDMbPaCxlWzZHj6056908 = CnBaDMbPaCxlWzZHj61443825;     CnBaDMbPaCxlWzZHj61443825 = CnBaDMbPaCxlWzZHj31620552;     CnBaDMbPaCxlWzZHj31620552 = CnBaDMbPaCxlWzZHj97321420;     CnBaDMbPaCxlWzZHj97321420 = CnBaDMbPaCxlWzZHj59913803;     CnBaDMbPaCxlWzZHj59913803 = CnBaDMbPaCxlWzZHj55218530;     CnBaDMbPaCxlWzZHj55218530 = CnBaDMbPaCxlWzZHj96090784;     CnBaDMbPaCxlWzZHj96090784 = CnBaDMbPaCxlWzZHj236088;     CnBaDMbPaCxlWzZHj236088 = CnBaDMbPaCxlWzZHj78046968;     CnBaDMbPaCxlWzZHj78046968 = CnBaDMbPaCxlWzZHj34820310;     CnBaDMbPaCxlWzZHj34820310 = CnBaDMbPaCxlWzZHj42389777;     CnBaDMbPaCxlWzZHj42389777 = CnBaDMbPaCxlWzZHj21687436;     CnBaDMbPaCxlWzZHj21687436 = CnBaDMbPaCxlWzZHj39886288;     CnBaDMbPaCxlWzZHj39886288 = CnBaDMbPaCxlWzZHj49030764;     CnBaDMbPaCxlWzZHj49030764 = CnBaDMbPaCxlWzZHj20156803;     CnBaDMbPaCxlWzZHj20156803 = CnBaDMbPaCxlWzZHj93843497;     CnBaDMbPaCxlWzZHj93843497 = CnBaDMbPaCxlWzZHj29362557;     CnBaDMbPaCxlWzZHj29362557 = CnBaDMbPaCxlWzZHj8571176;     CnBaDMbPaCxlWzZHj8571176 = CnBaDMbPaCxlWzZHj21578906;     CnBaDMbPaCxlWzZHj21578906 = CnBaDMbPaCxlWzZHj48388248;     CnBaDMbPaCxlWzZHj48388248 = CnBaDMbPaCxlWzZHj18320528;     CnBaDMbPaCxlWzZHj18320528 = CnBaDMbPaCxlWzZHj79360377;     CnBaDMbPaCxlWzZHj79360377 = CnBaDMbPaCxlWzZHj60661532;     CnBaDMbPaCxlWzZHj60661532 = CnBaDMbPaCxlWzZHj8840264;     CnBaDMbPaCxlWzZHj8840264 = CnBaDMbPaCxlWzZHj52696256;     CnBaDMbPaCxlWzZHj52696256 = CnBaDMbPaCxlWzZHj41322411;     CnBaDMbPaCxlWzZHj41322411 = CnBaDMbPaCxlWzZHj55920141;     CnBaDMbPaCxlWzZHj55920141 = CnBaDMbPaCxlWzZHj73005583;     CnBaDMbPaCxlWzZHj73005583 = CnBaDMbPaCxlWzZHj83039643;     CnBaDMbPaCxlWzZHj83039643 = CnBaDMbPaCxlWzZHj61674661;     CnBaDMbPaCxlWzZHj61674661 = CnBaDMbPaCxlWzZHj16865025;     CnBaDMbPaCxlWzZHj16865025 = CnBaDMbPaCxlWzZHj780257;     CnBaDMbPaCxlWzZHj780257 = CnBaDMbPaCxlWzZHj21749407;     CnBaDMbPaCxlWzZHj21749407 = CnBaDMbPaCxlWzZHj9874413;     CnBaDMbPaCxlWzZHj9874413 = CnBaDMbPaCxlWzZHj89994886;     CnBaDMbPaCxlWzZHj89994886 = CnBaDMbPaCxlWzZHj78396689;     CnBaDMbPaCxlWzZHj78396689 = CnBaDMbPaCxlWzZHj34649224;     CnBaDMbPaCxlWzZHj34649224 = CnBaDMbPaCxlWzZHj91402965;     CnBaDMbPaCxlWzZHj91402965 = CnBaDMbPaCxlWzZHj72984604;     CnBaDMbPaCxlWzZHj72984604 = CnBaDMbPaCxlWzZHj12401164;     CnBaDMbPaCxlWzZHj12401164 = CnBaDMbPaCxlWzZHj57548581;     CnBaDMbPaCxlWzZHj57548581 = CnBaDMbPaCxlWzZHj24779105;     CnBaDMbPaCxlWzZHj24779105 = CnBaDMbPaCxlWzZHj11975736;     CnBaDMbPaCxlWzZHj11975736 = CnBaDMbPaCxlWzZHj58196386;     CnBaDMbPaCxlWzZHj58196386 = CnBaDMbPaCxlWzZHj52090996;     CnBaDMbPaCxlWzZHj52090996 = CnBaDMbPaCxlWzZHj36721236;     CnBaDMbPaCxlWzZHj36721236 = CnBaDMbPaCxlWzZHj5058749;     CnBaDMbPaCxlWzZHj5058749 = CnBaDMbPaCxlWzZHj33496928;     CnBaDMbPaCxlWzZHj33496928 = CnBaDMbPaCxlWzZHj59188979;     CnBaDMbPaCxlWzZHj59188979 = CnBaDMbPaCxlWzZHj93060996;     CnBaDMbPaCxlWzZHj93060996 = CnBaDMbPaCxlWzZHj63529512;     CnBaDMbPaCxlWzZHj63529512 = CnBaDMbPaCxlWzZHj76352602;     CnBaDMbPaCxlWzZHj76352602 = CnBaDMbPaCxlWzZHj64053007;     CnBaDMbPaCxlWzZHj64053007 = CnBaDMbPaCxlWzZHj53868704;     CnBaDMbPaCxlWzZHj53868704 = CnBaDMbPaCxlWzZHj56724107;     CnBaDMbPaCxlWzZHj56724107 = CnBaDMbPaCxlWzZHj26696531;     CnBaDMbPaCxlWzZHj26696531 = CnBaDMbPaCxlWzZHj782294;     CnBaDMbPaCxlWzZHj782294 = CnBaDMbPaCxlWzZHj22780289;     CnBaDMbPaCxlWzZHj22780289 = CnBaDMbPaCxlWzZHj44625164;     CnBaDMbPaCxlWzZHj44625164 = CnBaDMbPaCxlWzZHj18591393;     CnBaDMbPaCxlWzZHj18591393 = CnBaDMbPaCxlWzZHj99298388;     CnBaDMbPaCxlWzZHj99298388 = CnBaDMbPaCxlWzZHj23085202;     CnBaDMbPaCxlWzZHj23085202 = CnBaDMbPaCxlWzZHj17196445;     CnBaDMbPaCxlWzZHj17196445 = CnBaDMbPaCxlWzZHj16372307;     CnBaDMbPaCxlWzZHj16372307 = CnBaDMbPaCxlWzZHj17955285;     CnBaDMbPaCxlWzZHj17955285 = CnBaDMbPaCxlWzZHj41609520;     CnBaDMbPaCxlWzZHj41609520 = CnBaDMbPaCxlWzZHj99938028;     CnBaDMbPaCxlWzZHj99938028 = CnBaDMbPaCxlWzZHj30011876;     CnBaDMbPaCxlWzZHj30011876 = CnBaDMbPaCxlWzZHj59035877;     CnBaDMbPaCxlWzZHj59035877 = CnBaDMbPaCxlWzZHj41760114;     CnBaDMbPaCxlWzZHj41760114 = CnBaDMbPaCxlWzZHj59194273;     CnBaDMbPaCxlWzZHj59194273 = CnBaDMbPaCxlWzZHj37959592;     CnBaDMbPaCxlWzZHj37959592 = CnBaDMbPaCxlWzZHj35586572;     CnBaDMbPaCxlWzZHj35586572 = CnBaDMbPaCxlWzZHj69944137;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void hTyUrwizaolizbHd67400432() {     double fPGIZrzkjxpKMuRbQ76061243 = 89053264;    double fPGIZrzkjxpKMuRbQ97940008 = -810363440;    double fPGIZrzkjxpKMuRbQ17830987 = -121877274;    double fPGIZrzkjxpKMuRbQ13486950 = -449517570;    double fPGIZrzkjxpKMuRbQ92901847 = -393533268;    double fPGIZrzkjxpKMuRbQ76336249 = -640254302;    double fPGIZrzkjxpKMuRbQ56540760 = -963239759;    double fPGIZrzkjxpKMuRbQ89600405 = -887973633;    double fPGIZrzkjxpKMuRbQ30912272 = -521377321;    double fPGIZrzkjxpKMuRbQ29954045 = -76992832;    double fPGIZrzkjxpKMuRbQ93008697 = -987672478;    double fPGIZrzkjxpKMuRbQ99442736 = -364809561;    double fPGIZrzkjxpKMuRbQ39848484 = -994254440;    double fPGIZrzkjxpKMuRbQ8158869 = -28238831;    double fPGIZrzkjxpKMuRbQ85765130 = -943774428;    double fPGIZrzkjxpKMuRbQ34522089 = -388346185;    double fPGIZrzkjxpKMuRbQ88316776 = -920263195;    double fPGIZrzkjxpKMuRbQ55461825 = -911764294;    double fPGIZrzkjxpKMuRbQ6774879 = -175210810;    double fPGIZrzkjxpKMuRbQ63077856 = -842347111;    double fPGIZrzkjxpKMuRbQ67898974 = -257162131;    double fPGIZrzkjxpKMuRbQ73425517 = -763799185;    double fPGIZrzkjxpKMuRbQ28482860 = -367420312;    double fPGIZrzkjxpKMuRbQ68916152 = -38895722;    double fPGIZrzkjxpKMuRbQ29331563 = -321558319;    double fPGIZrzkjxpKMuRbQ26978628 = -436003209;    double fPGIZrzkjxpKMuRbQ73504412 = -785365966;    double fPGIZrzkjxpKMuRbQ19164034 = -661313675;    double fPGIZrzkjxpKMuRbQ27576081 = -396867101;    double fPGIZrzkjxpKMuRbQ236189 = 1600687;    double fPGIZrzkjxpKMuRbQ89563969 = -721603418;    double fPGIZrzkjxpKMuRbQ57524425 = -336912860;    double fPGIZrzkjxpKMuRbQ64929478 = -611344247;    double fPGIZrzkjxpKMuRbQ90921348 = -303115431;    double fPGIZrzkjxpKMuRbQ61710739 = 64894821;    double fPGIZrzkjxpKMuRbQ46772881 = -520706709;    double fPGIZrzkjxpKMuRbQ16082245 = -258626339;    double fPGIZrzkjxpKMuRbQ76479989 = -119075238;    double fPGIZrzkjxpKMuRbQ95297689 = 87565127;    double fPGIZrzkjxpKMuRbQ27388226 = -747097979;    double fPGIZrzkjxpKMuRbQ54699988 = -750122305;    double fPGIZrzkjxpKMuRbQ52307259 = -364537240;    double fPGIZrzkjxpKMuRbQ71910475 = -235177657;    double fPGIZrzkjxpKMuRbQ16770297 = -917388370;    double fPGIZrzkjxpKMuRbQ20523641 = -362502377;    double fPGIZrzkjxpKMuRbQ33518101 = -784717380;    double fPGIZrzkjxpKMuRbQ66166822 = -266904057;    double fPGIZrzkjxpKMuRbQ26277469 = -956190547;    double fPGIZrzkjxpKMuRbQ89036106 = -733106412;    double fPGIZrzkjxpKMuRbQ73573184 = -241533536;    double fPGIZrzkjxpKMuRbQ44540942 = -889180938;    double fPGIZrzkjxpKMuRbQ63505975 = -95123422;    double fPGIZrzkjxpKMuRbQ89570868 = -271210869;    double fPGIZrzkjxpKMuRbQ51889021 = -317470448;    double fPGIZrzkjxpKMuRbQ99501457 = -807404818;    double fPGIZrzkjxpKMuRbQ2635726 = -147147551;    double fPGIZrzkjxpKMuRbQ69457148 = -342943128;    double fPGIZrzkjxpKMuRbQ48914835 = 17018448;    double fPGIZrzkjxpKMuRbQ84155387 = -27959251;    double fPGIZrzkjxpKMuRbQ65923220 = -957530059;    double fPGIZrzkjxpKMuRbQ2831837 = -854888337;    double fPGIZrzkjxpKMuRbQ37376727 = -201926085;    double fPGIZrzkjxpKMuRbQ62024324 = -391106533;    double fPGIZrzkjxpKMuRbQ30676083 = -422978008;    double fPGIZrzkjxpKMuRbQ40390075 = -355389414;    double fPGIZrzkjxpKMuRbQ35484273 = -550759619;    double fPGIZrzkjxpKMuRbQ34513258 = -753465314;    double fPGIZrzkjxpKMuRbQ48927135 = -591139010;    double fPGIZrzkjxpKMuRbQ46448130 = 6866348;    double fPGIZrzkjxpKMuRbQ38992249 = -323067720;    double fPGIZrzkjxpKMuRbQ18439845 = -29719846;    double fPGIZrzkjxpKMuRbQ11836787 = -701187958;    double fPGIZrzkjxpKMuRbQ60164135 = -899329421;    double fPGIZrzkjxpKMuRbQ79386652 = -428112832;    double fPGIZrzkjxpKMuRbQ8377869 = 7775193;    double fPGIZrzkjxpKMuRbQ15591716 = -892624891;    double fPGIZrzkjxpKMuRbQ1515042 = -428621529;    double fPGIZrzkjxpKMuRbQ11712564 = -450031943;    double fPGIZrzkjxpKMuRbQ48392511 = -676393346;    double fPGIZrzkjxpKMuRbQ95813462 = -536840939;    double fPGIZrzkjxpKMuRbQ60811805 = -69099153;    double fPGIZrzkjxpKMuRbQ47226944 = -829175419;    double fPGIZrzkjxpKMuRbQ30127927 = -928207264;    double fPGIZrzkjxpKMuRbQ54002896 = -55333565;    double fPGIZrzkjxpKMuRbQ55695247 = -109218375;    double fPGIZrzkjxpKMuRbQ26057994 = -526479997;    double fPGIZrzkjxpKMuRbQ67953557 = 34298009;    double fPGIZrzkjxpKMuRbQ13040457 = -193873800;    double fPGIZrzkjxpKMuRbQ91419890 = -495710613;    double fPGIZrzkjxpKMuRbQ59075013 = -787957628;    double fPGIZrzkjxpKMuRbQ77315732 = -77763581;    double fPGIZrzkjxpKMuRbQ67167409 = -175644788;    double fPGIZrzkjxpKMuRbQ92324602 = 8884013;    double fPGIZrzkjxpKMuRbQ29374469 = 45095185;    double fPGIZrzkjxpKMuRbQ24556389 = -892209642;    double fPGIZrzkjxpKMuRbQ17323261 = -448196221;    double fPGIZrzkjxpKMuRbQ90282935 = -973430708;    double fPGIZrzkjxpKMuRbQ41234393 = -812199649;    double fPGIZrzkjxpKMuRbQ76380222 = -461998956;    double fPGIZrzkjxpKMuRbQ85039368 = 89053264;     fPGIZrzkjxpKMuRbQ76061243 = fPGIZrzkjxpKMuRbQ97940008;     fPGIZrzkjxpKMuRbQ97940008 = fPGIZrzkjxpKMuRbQ17830987;     fPGIZrzkjxpKMuRbQ17830987 = fPGIZrzkjxpKMuRbQ13486950;     fPGIZrzkjxpKMuRbQ13486950 = fPGIZrzkjxpKMuRbQ92901847;     fPGIZrzkjxpKMuRbQ92901847 = fPGIZrzkjxpKMuRbQ76336249;     fPGIZrzkjxpKMuRbQ76336249 = fPGIZrzkjxpKMuRbQ56540760;     fPGIZrzkjxpKMuRbQ56540760 = fPGIZrzkjxpKMuRbQ89600405;     fPGIZrzkjxpKMuRbQ89600405 = fPGIZrzkjxpKMuRbQ30912272;     fPGIZrzkjxpKMuRbQ30912272 = fPGIZrzkjxpKMuRbQ29954045;     fPGIZrzkjxpKMuRbQ29954045 = fPGIZrzkjxpKMuRbQ93008697;     fPGIZrzkjxpKMuRbQ93008697 = fPGIZrzkjxpKMuRbQ99442736;     fPGIZrzkjxpKMuRbQ99442736 = fPGIZrzkjxpKMuRbQ39848484;     fPGIZrzkjxpKMuRbQ39848484 = fPGIZrzkjxpKMuRbQ8158869;     fPGIZrzkjxpKMuRbQ8158869 = fPGIZrzkjxpKMuRbQ85765130;     fPGIZrzkjxpKMuRbQ85765130 = fPGIZrzkjxpKMuRbQ34522089;     fPGIZrzkjxpKMuRbQ34522089 = fPGIZrzkjxpKMuRbQ88316776;     fPGIZrzkjxpKMuRbQ88316776 = fPGIZrzkjxpKMuRbQ55461825;     fPGIZrzkjxpKMuRbQ55461825 = fPGIZrzkjxpKMuRbQ6774879;     fPGIZrzkjxpKMuRbQ6774879 = fPGIZrzkjxpKMuRbQ63077856;     fPGIZrzkjxpKMuRbQ63077856 = fPGIZrzkjxpKMuRbQ67898974;     fPGIZrzkjxpKMuRbQ67898974 = fPGIZrzkjxpKMuRbQ73425517;     fPGIZrzkjxpKMuRbQ73425517 = fPGIZrzkjxpKMuRbQ28482860;     fPGIZrzkjxpKMuRbQ28482860 = fPGIZrzkjxpKMuRbQ68916152;     fPGIZrzkjxpKMuRbQ68916152 = fPGIZrzkjxpKMuRbQ29331563;     fPGIZrzkjxpKMuRbQ29331563 = fPGIZrzkjxpKMuRbQ26978628;     fPGIZrzkjxpKMuRbQ26978628 = fPGIZrzkjxpKMuRbQ73504412;     fPGIZrzkjxpKMuRbQ73504412 = fPGIZrzkjxpKMuRbQ19164034;     fPGIZrzkjxpKMuRbQ19164034 = fPGIZrzkjxpKMuRbQ27576081;     fPGIZrzkjxpKMuRbQ27576081 = fPGIZrzkjxpKMuRbQ236189;     fPGIZrzkjxpKMuRbQ236189 = fPGIZrzkjxpKMuRbQ89563969;     fPGIZrzkjxpKMuRbQ89563969 = fPGIZrzkjxpKMuRbQ57524425;     fPGIZrzkjxpKMuRbQ57524425 = fPGIZrzkjxpKMuRbQ64929478;     fPGIZrzkjxpKMuRbQ64929478 = fPGIZrzkjxpKMuRbQ90921348;     fPGIZrzkjxpKMuRbQ90921348 = fPGIZrzkjxpKMuRbQ61710739;     fPGIZrzkjxpKMuRbQ61710739 = fPGIZrzkjxpKMuRbQ46772881;     fPGIZrzkjxpKMuRbQ46772881 = fPGIZrzkjxpKMuRbQ16082245;     fPGIZrzkjxpKMuRbQ16082245 = fPGIZrzkjxpKMuRbQ76479989;     fPGIZrzkjxpKMuRbQ76479989 = fPGIZrzkjxpKMuRbQ95297689;     fPGIZrzkjxpKMuRbQ95297689 = fPGIZrzkjxpKMuRbQ27388226;     fPGIZrzkjxpKMuRbQ27388226 = fPGIZrzkjxpKMuRbQ54699988;     fPGIZrzkjxpKMuRbQ54699988 = fPGIZrzkjxpKMuRbQ52307259;     fPGIZrzkjxpKMuRbQ52307259 = fPGIZrzkjxpKMuRbQ71910475;     fPGIZrzkjxpKMuRbQ71910475 = fPGIZrzkjxpKMuRbQ16770297;     fPGIZrzkjxpKMuRbQ16770297 = fPGIZrzkjxpKMuRbQ20523641;     fPGIZrzkjxpKMuRbQ20523641 = fPGIZrzkjxpKMuRbQ33518101;     fPGIZrzkjxpKMuRbQ33518101 = fPGIZrzkjxpKMuRbQ66166822;     fPGIZrzkjxpKMuRbQ66166822 = fPGIZrzkjxpKMuRbQ26277469;     fPGIZrzkjxpKMuRbQ26277469 = fPGIZrzkjxpKMuRbQ89036106;     fPGIZrzkjxpKMuRbQ89036106 = fPGIZrzkjxpKMuRbQ73573184;     fPGIZrzkjxpKMuRbQ73573184 = fPGIZrzkjxpKMuRbQ44540942;     fPGIZrzkjxpKMuRbQ44540942 = fPGIZrzkjxpKMuRbQ63505975;     fPGIZrzkjxpKMuRbQ63505975 = fPGIZrzkjxpKMuRbQ89570868;     fPGIZrzkjxpKMuRbQ89570868 = fPGIZrzkjxpKMuRbQ51889021;     fPGIZrzkjxpKMuRbQ51889021 = fPGIZrzkjxpKMuRbQ99501457;     fPGIZrzkjxpKMuRbQ99501457 = fPGIZrzkjxpKMuRbQ2635726;     fPGIZrzkjxpKMuRbQ2635726 = fPGIZrzkjxpKMuRbQ69457148;     fPGIZrzkjxpKMuRbQ69457148 = fPGIZrzkjxpKMuRbQ48914835;     fPGIZrzkjxpKMuRbQ48914835 = fPGIZrzkjxpKMuRbQ84155387;     fPGIZrzkjxpKMuRbQ84155387 = fPGIZrzkjxpKMuRbQ65923220;     fPGIZrzkjxpKMuRbQ65923220 = fPGIZrzkjxpKMuRbQ2831837;     fPGIZrzkjxpKMuRbQ2831837 = fPGIZrzkjxpKMuRbQ37376727;     fPGIZrzkjxpKMuRbQ37376727 = fPGIZrzkjxpKMuRbQ62024324;     fPGIZrzkjxpKMuRbQ62024324 = fPGIZrzkjxpKMuRbQ30676083;     fPGIZrzkjxpKMuRbQ30676083 = fPGIZrzkjxpKMuRbQ40390075;     fPGIZrzkjxpKMuRbQ40390075 = fPGIZrzkjxpKMuRbQ35484273;     fPGIZrzkjxpKMuRbQ35484273 = fPGIZrzkjxpKMuRbQ34513258;     fPGIZrzkjxpKMuRbQ34513258 = fPGIZrzkjxpKMuRbQ48927135;     fPGIZrzkjxpKMuRbQ48927135 = fPGIZrzkjxpKMuRbQ46448130;     fPGIZrzkjxpKMuRbQ46448130 = fPGIZrzkjxpKMuRbQ38992249;     fPGIZrzkjxpKMuRbQ38992249 = fPGIZrzkjxpKMuRbQ18439845;     fPGIZrzkjxpKMuRbQ18439845 = fPGIZrzkjxpKMuRbQ11836787;     fPGIZrzkjxpKMuRbQ11836787 = fPGIZrzkjxpKMuRbQ60164135;     fPGIZrzkjxpKMuRbQ60164135 = fPGIZrzkjxpKMuRbQ79386652;     fPGIZrzkjxpKMuRbQ79386652 = fPGIZrzkjxpKMuRbQ8377869;     fPGIZrzkjxpKMuRbQ8377869 = fPGIZrzkjxpKMuRbQ15591716;     fPGIZrzkjxpKMuRbQ15591716 = fPGIZrzkjxpKMuRbQ1515042;     fPGIZrzkjxpKMuRbQ1515042 = fPGIZrzkjxpKMuRbQ11712564;     fPGIZrzkjxpKMuRbQ11712564 = fPGIZrzkjxpKMuRbQ48392511;     fPGIZrzkjxpKMuRbQ48392511 = fPGIZrzkjxpKMuRbQ95813462;     fPGIZrzkjxpKMuRbQ95813462 = fPGIZrzkjxpKMuRbQ60811805;     fPGIZrzkjxpKMuRbQ60811805 = fPGIZrzkjxpKMuRbQ47226944;     fPGIZrzkjxpKMuRbQ47226944 = fPGIZrzkjxpKMuRbQ30127927;     fPGIZrzkjxpKMuRbQ30127927 = fPGIZrzkjxpKMuRbQ54002896;     fPGIZrzkjxpKMuRbQ54002896 = fPGIZrzkjxpKMuRbQ55695247;     fPGIZrzkjxpKMuRbQ55695247 = fPGIZrzkjxpKMuRbQ26057994;     fPGIZrzkjxpKMuRbQ26057994 = fPGIZrzkjxpKMuRbQ67953557;     fPGIZrzkjxpKMuRbQ67953557 = fPGIZrzkjxpKMuRbQ13040457;     fPGIZrzkjxpKMuRbQ13040457 = fPGIZrzkjxpKMuRbQ91419890;     fPGIZrzkjxpKMuRbQ91419890 = fPGIZrzkjxpKMuRbQ59075013;     fPGIZrzkjxpKMuRbQ59075013 = fPGIZrzkjxpKMuRbQ77315732;     fPGIZrzkjxpKMuRbQ77315732 = fPGIZrzkjxpKMuRbQ67167409;     fPGIZrzkjxpKMuRbQ67167409 = fPGIZrzkjxpKMuRbQ92324602;     fPGIZrzkjxpKMuRbQ92324602 = fPGIZrzkjxpKMuRbQ29374469;     fPGIZrzkjxpKMuRbQ29374469 = fPGIZrzkjxpKMuRbQ24556389;     fPGIZrzkjxpKMuRbQ24556389 = fPGIZrzkjxpKMuRbQ17323261;     fPGIZrzkjxpKMuRbQ17323261 = fPGIZrzkjxpKMuRbQ90282935;     fPGIZrzkjxpKMuRbQ90282935 = fPGIZrzkjxpKMuRbQ41234393;     fPGIZrzkjxpKMuRbQ41234393 = fPGIZrzkjxpKMuRbQ76380222;     fPGIZrzkjxpKMuRbQ76380222 = fPGIZrzkjxpKMuRbQ85039368;     fPGIZrzkjxpKMuRbQ85039368 = fPGIZrzkjxpKMuRbQ76061243;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cniZKVhjEhCZGyDI34692032() {     double SYsdQVmTjVpdxwDzv11520212 = -377223636;    double SYsdQVmTjVpdxwDzv11212088 = -690915047;    double SYsdQVmTjVpdxwDzv55217463 = -887544349;    double SYsdQVmTjVpdxwDzv85710304 = -270668641;    double SYsdQVmTjVpdxwDzv49187117 = -499132113;    double SYsdQVmTjVpdxwDzv12559122 = -902602198;    double SYsdQVmTjVpdxwDzv67198167 = -807965493;    double SYsdQVmTjVpdxwDzv51731966 = -532207773;    double SYsdQVmTjVpdxwDzv32429877 = 90291272;    double SYsdQVmTjVpdxwDzv44856757 = -841679830;    double SYsdQVmTjVpdxwDzv31190433 = -789680207;    double SYsdQVmTjVpdxwDzv10885 = -228930332;    double SYsdQVmTjVpdxwDzv78083129 = -446266945;    double SYsdQVmTjVpdxwDzv61299732 = -4602240;    double SYsdQVmTjVpdxwDzv43959647 = -347199444;    double SYsdQVmTjVpdxwDzv18063390 = -24157792;    double SYsdQVmTjVpdxwDzv44152712 = -347237748;    double SYsdQVmTjVpdxwDzv55222016 = -985192362;    double SYsdQVmTjVpdxwDzv7325209 = -491774232;    double SYsdQVmTjVpdxwDzv56915075 = -168908177;    double SYsdQVmTjVpdxwDzv80985837 = 85563181;    double SYsdQVmTjVpdxwDzv76847042 = -850435479;    double SYsdQVmTjVpdxwDzv99008240 = -5052496;    double SYsdQVmTjVpdxwDzv62709097 = -549720327;    double SYsdQVmTjVpdxwDzv37349514 = -345543517;    double SYsdQVmTjVpdxwDzv37676799 = -124580016;    double SYsdQVmTjVpdxwDzv33924987 = -591650106;    double SYsdQVmTjVpdxwDzv59892458 = -831085849;    double SYsdQVmTjVpdxwDzv74244295 = -970961903;    double SYsdQVmTjVpdxwDzv5769600 = -32906483;    double SYsdQVmTjVpdxwDzv8147202 = -173583905;    double SYsdQVmTjVpdxwDzv95347443 = -109884274;    double SYsdQVmTjVpdxwDzv77147489 = -758940225;    double SYsdQVmTjVpdxwDzv20669776 = -367809244;    double SYsdQVmTjVpdxwDzv27519193 = -986205430;    double SYsdQVmTjVpdxwDzv98333725 = -735272015;    double SYsdQVmTjVpdxwDzv65494061 = -123322380;    double SYsdQVmTjVpdxwDzv27695393 = -101730819;    double SYsdQVmTjVpdxwDzv25688251 = -663311150;    double SYsdQVmTjVpdxwDzv91659758 = -212360525;    double SYsdQVmTjVpdxwDzv88151595 = -519321219;    double SYsdQVmTjVpdxwDzv41672386 = 40917469;    double SYsdQVmTjVpdxwDzv61293854 = -888358443;    double SYsdQVmTjVpdxwDzv56901635 = -421720780;    double SYsdQVmTjVpdxwDzv20540100 = -189013538;    double SYsdQVmTjVpdxwDzv79479429 = -823641012;    double SYsdQVmTjVpdxwDzv85511986 = -750890417;    double SYsdQVmTjVpdxwDzv44003780 = -22717140;    double SYsdQVmTjVpdxwDzv11928773 = -396843292;    double SYsdQVmTjVpdxwDzv98102773 = -732560352;    double SYsdQVmTjVpdxwDzv25311665 = -763457495;    double SYsdQVmTjVpdxwDzv46991448 = -308775768;    double SYsdQVmTjVpdxwDzv58192017 = -27256423;    double SYsdQVmTjVpdxwDzv67880868 = -508978947;    double SYsdQVmTjVpdxwDzv53706194 = -53894964;    double SYsdQVmTjVpdxwDzv34673170 = -526788157;    double SYsdQVmTjVpdxwDzv12203848 = -585862551;    double SYsdQVmTjVpdxwDzv92508366 = -237824023;    double SYsdQVmTjVpdxwDzv48360791 = -925125124;    double SYsdQVmTjVpdxwDzv11510318 = -274552098;    double SYsdQVmTjVpdxwDzv78634134 = -210952092;    double SYsdQVmTjVpdxwDzv7305709 = -976879645;    double SYsdQVmTjVpdxwDzv77487670 = -561245871;    double SYsdQVmTjVpdxwDzv26660277 = -876802245;    double SYsdQVmTjVpdxwDzv36709556 = -568095925;    double SYsdQVmTjVpdxwDzv35842989 = -579795933;    double SYsdQVmTjVpdxwDzv22863396 = -469990108;    double SYsdQVmTjVpdxwDzv57413354 = 21542299;    double SYsdQVmTjVpdxwDzv33780539 = -18396811;    double SYsdQVmTjVpdxwDzv45625922 = -611927429;    double SYsdQVmTjVpdxwDzv52569329 = -900835413;    double SYsdQVmTjVpdxwDzv16457320 = -145506929;    double SYsdQVmTjVpdxwDzv29533765 = -221881213;    double SYsdQVmTjVpdxwDzv15665451 = -179413708;    double SYsdQVmTjVpdxwDzv68763479 = -649586958;    double SYsdQVmTjVpdxwDzv39313452 = -955354289;    double SYsdQVmTjVpdxwDzv15553189 = -962077037;    double SYsdQVmTjVpdxwDzv42106605 = -583331717;    double SYsdQVmTjVpdxwDzv42168998 = -260706789;    double SYsdQVmTjVpdxwDzv57870085 = -521902505;    double SYsdQVmTjVpdxwDzv52164813 = -373689599;    double SYsdQVmTjVpdxwDzv89921206 = -468932966;    double SYsdQVmTjVpdxwDzv47963686 = -334242558;    double SYsdQVmTjVpdxwDzv76141521 = -138401552;    double SYsdQVmTjVpdxwDzv80457934 = -269448988;    double SYsdQVmTjVpdxwDzv61155753 = -864808137;    double SYsdQVmTjVpdxwDzv37155427 = 17372149;    double SYsdQVmTjVpdxwDzv9266621 = -149961278;    double SYsdQVmTjVpdxwDzv66963581 = -213914281;    double SYsdQVmTjVpdxwDzv92846023 = -359417273;    double SYsdQVmTjVpdxwDzv86129877 = -49409465;    double SYsdQVmTjVpdxwDzv72985694 = -885498357;    double SYsdQVmTjVpdxwDzv79334602 = -176605695;    double SYsdQVmTjVpdxwDzv14177934 = -288759052;    double SYsdQVmTjVpdxwDzv13025624 = 98591567;    double SYsdQVmTjVpdxwDzv80845887 = -542441575;    double SYsdQVmTjVpdxwDzv64184715 = -397836661;    double SYsdQVmTjVpdxwDzv34633577 = 88443802;    double SYsdQVmTjVpdxwDzv20192080 = -853624855;    double SYsdQVmTjVpdxwDzv84697110 = -377223636;     SYsdQVmTjVpdxwDzv11520212 = SYsdQVmTjVpdxwDzv11212088;     SYsdQVmTjVpdxwDzv11212088 = SYsdQVmTjVpdxwDzv55217463;     SYsdQVmTjVpdxwDzv55217463 = SYsdQVmTjVpdxwDzv85710304;     SYsdQVmTjVpdxwDzv85710304 = SYsdQVmTjVpdxwDzv49187117;     SYsdQVmTjVpdxwDzv49187117 = SYsdQVmTjVpdxwDzv12559122;     SYsdQVmTjVpdxwDzv12559122 = SYsdQVmTjVpdxwDzv67198167;     SYsdQVmTjVpdxwDzv67198167 = SYsdQVmTjVpdxwDzv51731966;     SYsdQVmTjVpdxwDzv51731966 = SYsdQVmTjVpdxwDzv32429877;     SYsdQVmTjVpdxwDzv32429877 = SYsdQVmTjVpdxwDzv44856757;     SYsdQVmTjVpdxwDzv44856757 = SYsdQVmTjVpdxwDzv31190433;     SYsdQVmTjVpdxwDzv31190433 = SYsdQVmTjVpdxwDzv10885;     SYsdQVmTjVpdxwDzv10885 = SYsdQVmTjVpdxwDzv78083129;     SYsdQVmTjVpdxwDzv78083129 = SYsdQVmTjVpdxwDzv61299732;     SYsdQVmTjVpdxwDzv61299732 = SYsdQVmTjVpdxwDzv43959647;     SYsdQVmTjVpdxwDzv43959647 = SYsdQVmTjVpdxwDzv18063390;     SYsdQVmTjVpdxwDzv18063390 = SYsdQVmTjVpdxwDzv44152712;     SYsdQVmTjVpdxwDzv44152712 = SYsdQVmTjVpdxwDzv55222016;     SYsdQVmTjVpdxwDzv55222016 = SYsdQVmTjVpdxwDzv7325209;     SYsdQVmTjVpdxwDzv7325209 = SYsdQVmTjVpdxwDzv56915075;     SYsdQVmTjVpdxwDzv56915075 = SYsdQVmTjVpdxwDzv80985837;     SYsdQVmTjVpdxwDzv80985837 = SYsdQVmTjVpdxwDzv76847042;     SYsdQVmTjVpdxwDzv76847042 = SYsdQVmTjVpdxwDzv99008240;     SYsdQVmTjVpdxwDzv99008240 = SYsdQVmTjVpdxwDzv62709097;     SYsdQVmTjVpdxwDzv62709097 = SYsdQVmTjVpdxwDzv37349514;     SYsdQVmTjVpdxwDzv37349514 = SYsdQVmTjVpdxwDzv37676799;     SYsdQVmTjVpdxwDzv37676799 = SYsdQVmTjVpdxwDzv33924987;     SYsdQVmTjVpdxwDzv33924987 = SYsdQVmTjVpdxwDzv59892458;     SYsdQVmTjVpdxwDzv59892458 = SYsdQVmTjVpdxwDzv74244295;     SYsdQVmTjVpdxwDzv74244295 = SYsdQVmTjVpdxwDzv5769600;     SYsdQVmTjVpdxwDzv5769600 = SYsdQVmTjVpdxwDzv8147202;     SYsdQVmTjVpdxwDzv8147202 = SYsdQVmTjVpdxwDzv95347443;     SYsdQVmTjVpdxwDzv95347443 = SYsdQVmTjVpdxwDzv77147489;     SYsdQVmTjVpdxwDzv77147489 = SYsdQVmTjVpdxwDzv20669776;     SYsdQVmTjVpdxwDzv20669776 = SYsdQVmTjVpdxwDzv27519193;     SYsdQVmTjVpdxwDzv27519193 = SYsdQVmTjVpdxwDzv98333725;     SYsdQVmTjVpdxwDzv98333725 = SYsdQVmTjVpdxwDzv65494061;     SYsdQVmTjVpdxwDzv65494061 = SYsdQVmTjVpdxwDzv27695393;     SYsdQVmTjVpdxwDzv27695393 = SYsdQVmTjVpdxwDzv25688251;     SYsdQVmTjVpdxwDzv25688251 = SYsdQVmTjVpdxwDzv91659758;     SYsdQVmTjVpdxwDzv91659758 = SYsdQVmTjVpdxwDzv88151595;     SYsdQVmTjVpdxwDzv88151595 = SYsdQVmTjVpdxwDzv41672386;     SYsdQVmTjVpdxwDzv41672386 = SYsdQVmTjVpdxwDzv61293854;     SYsdQVmTjVpdxwDzv61293854 = SYsdQVmTjVpdxwDzv56901635;     SYsdQVmTjVpdxwDzv56901635 = SYsdQVmTjVpdxwDzv20540100;     SYsdQVmTjVpdxwDzv20540100 = SYsdQVmTjVpdxwDzv79479429;     SYsdQVmTjVpdxwDzv79479429 = SYsdQVmTjVpdxwDzv85511986;     SYsdQVmTjVpdxwDzv85511986 = SYsdQVmTjVpdxwDzv44003780;     SYsdQVmTjVpdxwDzv44003780 = SYsdQVmTjVpdxwDzv11928773;     SYsdQVmTjVpdxwDzv11928773 = SYsdQVmTjVpdxwDzv98102773;     SYsdQVmTjVpdxwDzv98102773 = SYsdQVmTjVpdxwDzv25311665;     SYsdQVmTjVpdxwDzv25311665 = SYsdQVmTjVpdxwDzv46991448;     SYsdQVmTjVpdxwDzv46991448 = SYsdQVmTjVpdxwDzv58192017;     SYsdQVmTjVpdxwDzv58192017 = SYsdQVmTjVpdxwDzv67880868;     SYsdQVmTjVpdxwDzv67880868 = SYsdQVmTjVpdxwDzv53706194;     SYsdQVmTjVpdxwDzv53706194 = SYsdQVmTjVpdxwDzv34673170;     SYsdQVmTjVpdxwDzv34673170 = SYsdQVmTjVpdxwDzv12203848;     SYsdQVmTjVpdxwDzv12203848 = SYsdQVmTjVpdxwDzv92508366;     SYsdQVmTjVpdxwDzv92508366 = SYsdQVmTjVpdxwDzv48360791;     SYsdQVmTjVpdxwDzv48360791 = SYsdQVmTjVpdxwDzv11510318;     SYsdQVmTjVpdxwDzv11510318 = SYsdQVmTjVpdxwDzv78634134;     SYsdQVmTjVpdxwDzv78634134 = SYsdQVmTjVpdxwDzv7305709;     SYsdQVmTjVpdxwDzv7305709 = SYsdQVmTjVpdxwDzv77487670;     SYsdQVmTjVpdxwDzv77487670 = SYsdQVmTjVpdxwDzv26660277;     SYsdQVmTjVpdxwDzv26660277 = SYsdQVmTjVpdxwDzv36709556;     SYsdQVmTjVpdxwDzv36709556 = SYsdQVmTjVpdxwDzv35842989;     SYsdQVmTjVpdxwDzv35842989 = SYsdQVmTjVpdxwDzv22863396;     SYsdQVmTjVpdxwDzv22863396 = SYsdQVmTjVpdxwDzv57413354;     SYsdQVmTjVpdxwDzv57413354 = SYsdQVmTjVpdxwDzv33780539;     SYsdQVmTjVpdxwDzv33780539 = SYsdQVmTjVpdxwDzv45625922;     SYsdQVmTjVpdxwDzv45625922 = SYsdQVmTjVpdxwDzv52569329;     SYsdQVmTjVpdxwDzv52569329 = SYsdQVmTjVpdxwDzv16457320;     SYsdQVmTjVpdxwDzv16457320 = SYsdQVmTjVpdxwDzv29533765;     SYsdQVmTjVpdxwDzv29533765 = SYsdQVmTjVpdxwDzv15665451;     SYsdQVmTjVpdxwDzv15665451 = SYsdQVmTjVpdxwDzv68763479;     SYsdQVmTjVpdxwDzv68763479 = SYsdQVmTjVpdxwDzv39313452;     SYsdQVmTjVpdxwDzv39313452 = SYsdQVmTjVpdxwDzv15553189;     SYsdQVmTjVpdxwDzv15553189 = SYsdQVmTjVpdxwDzv42106605;     SYsdQVmTjVpdxwDzv42106605 = SYsdQVmTjVpdxwDzv42168998;     SYsdQVmTjVpdxwDzv42168998 = SYsdQVmTjVpdxwDzv57870085;     SYsdQVmTjVpdxwDzv57870085 = SYsdQVmTjVpdxwDzv52164813;     SYsdQVmTjVpdxwDzv52164813 = SYsdQVmTjVpdxwDzv89921206;     SYsdQVmTjVpdxwDzv89921206 = SYsdQVmTjVpdxwDzv47963686;     SYsdQVmTjVpdxwDzv47963686 = SYsdQVmTjVpdxwDzv76141521;     SYsdQVmTjVpdxwDzv76141521 = SYsdQVmTjVpdxwDzv80457934;     SYsdQVmTjVpdxwDzv80457934 = SYsdQVmTjVpdxwDzv61155753;     SYsdQVmTjVpdxwDzv61155753 = SYsdQVmTjVpdxwDzv37155427;     SYsdQVmTjVpdxwDzv37155427 = SYsdQVmTjVpdxwDzv9266621;     SYsdQVmTjVpdxwDzv9266621 = SYsdQVmTjVpdxwDzv66963581;     SYsdQVmTjVpdxwDzv66963581 = SYsdQVmTjVpdxwDzv92846023;     SYsdQVmTjVpdxwDzv92846023 = SYsdQVmTjVpdxwDzv86129877;     SYsdQVmTjVpdxwDzv86129877 = SYsdQVmTjVpdxwDzv72985694;     SYsdQVmTjVpdxwDzv72985694 = SYsdQVmTjVpdxwDzv79334602;     SYsdQVmTjVpdxwDzv79334602 = SYsdQVmTjVpdxwDzv14177934;     SYsdQVmTjVpdxwDzv14177934 = SYsdQVmTjVpdxwDzv13025624;     SYsdQVmTjVpdxwDzv13025624 = SYsdQVmTjVpdxwDzv80845887;     SYsdQVmTjVpdxwDzv80845887 = SYsdQVmTjVpdxwDzv64184715;     SYsdQVmTjVpdxwDzv64184715 = SYsdQVmTjVpdxwDzv34633577;     SYsdQVmTjVpdxwDzv34633577 = SYsdQVmTjVpdxwDzv20192080;     SYsdQVmTjVpdxwDzv20192080 = SYsdQVmTjVpdxwDzv84697110;     SYsdQVmTjVpdxwDzv84697110 = SYsdQVmTjVpdxwDzv11520212;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NXqTxtSJHCQVpfIw49741100() {     double PFwotkpPyDhJeLuAO17637319 = -489125524;    double PFwotkpPyDhJeLuAO54585367 = -400514827;    double PFwotkpPyDhJeLuAO71259647 = 61622730;    double PFwotkpPyDhJeLuAO12785086 = -461417271;    double PFwotkpPyDhJeLuAO18082606 = 80218015;    double PFwotkpPyDhJeLuAO3976324 = -67098433;    double PFwotkpPyDhJeLuAO27687133 = -946930471;    double PFwotkpPyDhJeLuAO1491857 = -819081686;    double PFwotkpPyDhJeLuAO97072372 = -777741393;    double PFwotkpPyDhJeLuAO86086416 = -70655307;    double PFwotkpPyDhJeLuAO91300722 = -667117794;    double PFwotkpPyDhJeLuAO31833927 = -673336534;    double PFwotkpPyDhJeLuAO64292248 = -886158034;    double PFwotkpPyDhJeLuAO44443409 = -536840808;    double PFwotkpPyDhJeLuAO39702074 = 62338390;    double PFwotkpPyDhJeLuAO59568784 = 2842665;    double PFwotkpPyDhJeLuAO37988715 = -963782861;    double PFwotkpPyDhJeLuAO52275169 = -650715005;    double PFwotkpPyDhJeLuAO69155050 = -897543329;    double PFwotkpPyDhJeLuAO37465240 = -525811097;    double PFwotkpPyDhJeLuAO69539031 = -869924275;    double PFwotkpPyDhJeLuAO63368066 = -758365195;    double PFwotkpPyDhJeLuAO34599032 = -851569983;    double PFwotkpPyDhJeLuAO46701471 = -772025314;    double PFwotkpPyDhJeLuAO81049164 = -718739550;    double PFwotkpPyDhJeLuAO62398474 = -3235339;    double PFwotkpPyDhJeLuAO32384765 = -492312309;    double PFwotkpPyDhJeLuAO72999583 = -897014803;    double PFwotkpPyDhJeLuAO40376551 = -93344608;    double PFwotkpPyDhJeLuAO74385236 = -950377351;    double PFwotkpPyDhJeLuAO389751 = -86904843;    double PFwotkpPyDhJeLuAO92958065 = -919331090;    double PFwotkpPyDhJeLuAO86858437 = -651553595;    double PFwotkpPyDhJeLuAO15500340 = -119351010;    double PFwotkpPyDhJeLuAO88993844 = -524938535;    double PFwotkpPyDhJeLuAO67059638 = -949298122;    double PFwotkpPyDhJeLuAO46755995 = -76958249;    double PFwotkpPyDhJeLuAO61785605 = -973236828;    double PFwotkpPyDhJeLuAO99298504 = -657234646;    double PFwotkpPyDhJeLuAO79161696 = -685799747;    double PFwotkpPyDhJeLuAO93820819 = -584595114;    double PFwotkpPyDhJeLuAO73822843 = -229224809;    double PFwotkpPyDhJeLuAO39360832 = -969163026;    double PFwotkpPyDhJeLuAO44309376 = -441063718;    double PFwotkpPyDhJeLuAO32492565 = -950911693;    double PFwotkpPyDhJeLuAO91418623 = -808857129;    double PFwotkpPyDhJeLuAO3290561 = -869665955;    double PFwotkpPyDhJeLuAO51960721 = -32612673;    double PFwotkpPyDhJeLuAO21604502 = -482580280;    double PFwotkpPyDhJeLuAO11014427 = -819571160;    double PFwotkpPyDhJeLuAO61012343 = -605751167;    double PFwotkpPyDhJeLuAO57801167 = -109502088;    double PFwotkpPyDhJeLuAO6440474 = -919046011;    double PFwotkpPyDhJeLuAO63849748 = -829449092;    double PFwotkpPyDhJeLuAO80202069 = -754426746;    double PFwotkpPyDhJeLuAO54269253 = -730760330;    double PFwotkpPyDhJeLuAO19986335 = -548944845;    double PFwotkpPyDhJeLuAO24558176 = -166351956;    double PFwotkpPyDhJeLuAO31735921 = -742677721;    double PFwotkpPyDhJeLuAO55684131 = -916546646;    double PFwotkpPyDhJeLuAO71591559 = -574786124;    double PFwotkpPyDhJeLuAO54687549 = 50084332;    double PFwotkpPyDhJeLuAO61115306 = -625737079;    double PFwotkpPyDhJeLuAO22687136 = -827364043;    double PFwotkpPyDhJeLuAO85696665 = -983750465;    double PFwotkpPyDhJeLuAO98342657 = -747786704;    double PFwotkpPyDhJeLuAO44975490 = 78217060;    double PFwotkpPyDhJeLuAO48791908 = -666807025;    double PFwotkpPyDhJeLuAO55449564 = 88097727;    double PFwotkpPyDhJeLuAO72642435 = 11636512;    double PFwotkpPyDhJeLuAO12812789 = -920199086;    double PFwotkpPyDhJeLuAO76203110 = -990546033;    double PFwotkpPyDhJeLuAO52976664 = -993480359;    double PFwotkpPyDhJeLuAO89993354 = -111743582;    double PFwotkpPyDhJeLuAO43644420 = -941215984;    double PFwotkpPyDhJeLuAO95716187 = -540699467;    double PFwotkpPyDhJeLuAO24007234 = -789202169;    double PFwotkpPyDhJeLuAO90289656 = -310506266;    double PFwotkpPyDhJeLuAO14208907 = -821113622;    double PFwotkpPyDhJeLuAO89630540 = -909882422;    double PFwotkpPyDhJeLuAO59107914 = -133569385;    double PFwotkpPyDhJeLuAO80424044 = -359699637;    double PFwotkpPyDhJeLuAO51395082 = -314434523;    double PFwotkpPyDhJeLuAO29362125 = -273773448;    double PFwotkpPyDhJeLuAO13372893 = -244626185;    double PFwotkpPyDhJeLuAO42588583 = -977402755;    double PFwotkpPyDhJeLuAO86517591 = 99714921;    double PFwotkpPyDhJeLuAO23008689 = -822104503;    double PFwotkpPyDhJeLuAO35298271 = -364924264;    double PFwotkpPyDhJeLuAO34724592 = -794178206;    double PFwotkpPyDhJeLuAO47073303 = -300353278;    double PFwotkpPyDhJeLuAO22197820 = -910606294;    double PFwotkpPyDhJeLuAO30049684 = -130559107;    double PFwotkpPyDhJeLuAO43614374 = -740688001;    double PFwotkpPyDhJeLuAO7570137 = -11013624;    double PFwotkpPyDhJeLuAO39133270 = -534679446;    double PFwotkpPyDhJeLuAO12707537 = -603487730;    double PFwotkpPyDhJeLuAO16673697 = -41798984;    double PFwotkpPyDhJeLuAO58612710 = -457313254;    double PFwotkpPyDhJeLuAO34149907 = -489125524;     PFwotkpPyDhJeLuAO17637319 = PFwotkpPyDhJeLuAO54585367;     PFwotkpPyDhJeLuAO54585367 = PFwotkpPyDhJeLuAO71259647;     PFwotkpPyDhJeLuAO71259647 = PFwotkpPyDhJeLuAO12785086;     PFwotkpPyDhJeLuAO12785086 = PFwotkpPyDhJeLuAO18082606;     PFwotkpPyDhJeLuAO18082606 = PFwotkpPyDhJeLuAO3976324;     PFwotkpPyDhJeLuAO3976324 = PFwotkpPyDhJeLuAO27687133;     PFwotkpPyDhJeLuAO27687133 = PFwotkpPyDhJeLuAO1491857;     PFwotkpPyDhJeLuAO1491857 = PFwotkpPyDhJeLuAO97072372;     PFwotkpPyDhJeLuAO97072372 = PFwotkpPyDhJeLuAO86086416;     PFwotkpPyDhJeLuAO86086416 = PFwotkpPyDhJeLuAO91300722;     PFwotkpPyDhJeLuAO91300722 = PFwotkpPyDhJeLuAO31833927;     PFwotkpPyDhJeLuAO31833927 = PFwotkpPyDhJeLuAO64292248;     PFwotkpPyDhJeLuAO64292248 = PFwotkpPyDhJeLuAO44443409;     PFwotkpPyDhJeLuAO44443409 = PFwotkpPyDhJeLuAO39702074;     PFwotkpPyDhJeLuAO39702074 = PFwotkpPyDhJeLuAO59568784;     PFwotkpPyDhJeLuAO59568784 = PFwotkpPyDhJeLuAO37988715;     PFwotkpPyDhJeLuAO37988715 = PFwotkpPyDhJeLuAO52275169;     PFwotkpPyDhJeLuAO52275169 = PFwotkpPyDhJeLuAO69155050;     PFwotkpPyDhJeLuAO69155050 = PFwotkpPyDhJeLuAO37465240;     PFwotkpPyDhJeLuAO37465240 = PFwotkpPyDhJeLuAO69539031;     PFwotkpPyDhJeLuAO69539031 = PFwotkpPyDhJeLuAO63368066;     PFwotkpPyDhJeLuAO63368066 = PFwotkpPyDhJeLuAO34599032;     PFwotkpPyDhJeLuAO34599032 = PFwotkpPyDhJeLuAO46701471;     PFwotkpPyDhJeLuAO46701471 = PFwotkpPyDhJeLuAO81049164;     PFwotkpPyDhJeLuAO81049164 = PFwotkpPyDhJeLuAO62398474;     PFwotkpPyDhJeLuAO62398474 = PFwotkpPyDhJeLuAO32384765;     PFwotkpPyDhJeLuAO32384765 = PFwotkpPyDhJeLuAO72999583;     PFwotkpPyDhJeLuAO72999583 = PFwotkpPyDhJeLuAO40376551;     PFwotkpPyDhJeLuAO40376551 = PFwotkpPyDhJeLuAO74385236;     PFwotkpPyDhJeLuAO74385236 = PFwotkpPyDhJeLuAO389751;     PFwotkpPyDhJeLuAO389751 = PFwotkpPyDhJeLuAO92958065;     PFwotkpPyDhJeLuAO92958065 = PFwotkpPyDhJeLuAO86858437;     PFwotkpPyDhJeLuAO86858437 = PFwotkpPyDhJeLuAO15500340;     PFwotkpPyDhJeLuAO15500340 = PFwotkpPyDhJeLuAO88993844;     PFwotkpPyDhJeLuAO88993844 = PFwotkpPyDhJeLuAO67059638;     PFwotkpPyDhJeLuAO67059638 = PFwotkpPyDhJeLuAO46755995;     PFwotkpPyDhJeLuAO46755995 = PFwotkpPyDhJeLuAO61785605;     PFwotkpPyDhJeLuAO61785605 = PFwotkpPyDhJeLuAO99298504;     PFwotkpPyDhJeLuAO99298504 = PFwotkpPyDhJeLuAO79161696;     PFwotkpPyDhJeLuAO79161696 = PFwotkpPyDhJeLuAO93820819;     PFwotkpPyDhJeLuAO93820819 = PFwotkpPyDhJeLuAO73822843;     PFwotkpPyDhJeLuAO73822843 = PFwotkpPyDhJeLuAO39360832;     PFwotkpPyDhJeLuAO39360832 = PFwotkpPyDhJeLuAO44309376;     PFwotkpPyDhJeLuAO44309376 = PFwotkpPyDhJeLuAO32492565;     PFwotkpPyDhJeLuAO32492565 = PFwotkpPyDhJeLuAO91418623;     PFwotkpPyDhJeLuAO91418623 = PFwotkpPyDhJeLuAO3290561;     PFwotkpPyDhJeLuAO3290561 = PFwotkpPyDhJeLuAO51960721;     PFwotkpPyDhJeLuAO51960721 = PFwotkpPyDhJeLuAO21604502;     PFwotkpPyDhJeLuAO21604502 = PFwotkpPyDhJeLuAO11014427;     PFwotkpPyDhJeLuAO11014427 = PFwotkpPyDhJeLuAO61012343;     PFwotkpPyDhJeLuAO61012343 = PFwotkpPyDhJeLuAO57801167;     PFwotkpPyDhJeLuAO57801167 = PFwotkpPyDhJeLuAO6440474;     PFwotkpPyDhJeLuAO6440474 = PFwotkpPyDhJeLuAO63849748;     PFwotkpPyDhJeLuAO63849748 = PFwotkpPyDhJeLuAO80202069;     PFwotkpPyDhJeLuAO80202069 = PFwotkpPyDhJeLuAO54269253;     PFwotkpPyDhJeLuAO54269253 = PFwotkpPyDhJeLuAO19986335;     PFwotkpPyDhJeLuAO19986335 = PFwotkpPyDhJeLuAO24558176;     PFwotkpPyDhJeLuAO24558176 = PFwotkpPyDhJeLuAO31735921;     PFwotkpPyDhJeLuAO31735921 = PFwotkpPyDhJeLuAO55684131;     PFwotkpPyDhJeLuAO55684131 = PFwotkpPyDhJeLuAO71591559;     PFwotkpPyDhJeLuAO71591559 = PFwotkpPyDhJeLuAO54687549;     PFwotkpPyDhJeLuAO54687549 = PFwotkpPyDhJeLuAO61115306;     PFwotkpPyDhJeLuAO61115306 = PFwotkpPyDhJeLuAO22687136;     PFwotkpPyDhJeLuAO22687136 = PFwotkpPyDhJeLuAO85696665;     PFwotkpPyDhJeLuAO85696665 = PFwotkpPyDhJeLuAO98342657;     PFwotkpPyDhJeLuAO98342657 = PFwotkpPyDhJeLuAO44975490;     PFwotkpPyDhJeLuAO44975490 = PFwotkpPyDhJeLuAO48791908;     PFwotkpPyDhJeLuAO48791908 = PFwotkpPyDhJeLuAO55449564;     PFwotkpPyDhJeLuAO55449564 = PFwotkpPyDhJeLuAO72642435;     PFwotkpPyDhJeLuAO72642435 = PFwotkpPyDhJeLuAO12812789;     PFwotkpPyDhJeLuAO12812789 = PFwotkpPyDhJeLuAO76203110;     PFwotkpPyDhJeLuAO76203110 = PFwotkpPyDhJeLuAO52976664;     PFwotkpPyDhJeLuAO52976664 = PFwotkpPyDhJeLuAO89993354;     PFwotkpPyDhJeLuAO89993354 = PFwotkpPyDhJeLuAO43644420;     PFwotkpPyDhJeLuAO43644420 = PFwotkpPyDhJeLuAO95716187;     PFwotkpPyDhJeLuAO95716187 = PFwotkpPyDhJeLuAO24007234;     PFwotkpPyDhJeLuAO24007234 = PFwotkpPyDhJeLuAO90289656;     PFwotkpPyDhJeLuAO90289656 = PFwotkpPyDhJeLuAO14208907;     PFwotkpPyDhJeLuAO14208907 = PFwotkpPyDhJeLuAO89630540;     PFwotkpPyDhJeLuAO89630540 = PFwotkpPyDhJeLuAO59107914;     PFwotkpPyDhJeLuAO59107914 = PFwotkpPyDhJeLuAO80424044;     PFwotkpPyDhJeLuAO80424044 = PFwotkpPyDhJeLuAO51395082;     PFwotkpPyDhJeLuAO51395082 = PFwotkpPyDhJeLuAO29362125;     PFwotkpPyDhJeLuAO29362125 = PFwotkpPyDhJeLuAO13372893;     PFwotkpPyDhJeLuAO13372893 = PFwotkpPyDhJeLuAO42588583;     PFwotkpPyDhJeLuAO42588583 = PFwotkpPyDhJeLuAO86517591;     PFwotkpPyDhJeLuAO86517591 = PFwotkpPyDhJeLuAO23008689;     PFwotkpPyDhJeLuAO23008689 = PFwotkpPyDhJeLuAO35298271;     PFwotkpPyDhJeLuAO35298271 = PFwotkpPyDhJeLuAO34724592;     PFwotkpPyDhJeLuAO34724592 = PFwotkpPyDhJeLuAO47073303;     PFwotkpPyDhJeLuAO47073303 = PFwotkpPyDhJeLuAO22197820;     PFwotkpPyDhJeLuAO22197820 = PFwotkpPyDhJeLuAO30049684;     PFwotkpPyDhJeLuAO30049684 = PFwotkpPyDhJeLuAO43614374;     PFwotkpPyDhJeLuAO43614374 = PFwotkpPyDhJeLuAO7570137;     PFwotkpPyDhJeLuAO7570137 = PFwotkpPyDhJeLuAO39133270;     PFwotkpPyDhJeLuAO39133270 = PFwotkpPyDhJeLuAO12707537;     PFwotkpPyDhJeLuAO12707537 = PFwotkpPyDhJeLuAO16673697;     PFwotkpPyDhJeLuAO16673697 = PFwotkpPyDhJeLuAO58612710;     PFwotkpPyDhJeLuAO58612710 = PFwotkpPyDhJeLuAO34149907;     PFwotkpPyDhJeLuAO34149907 = PFwotkpPyDhJeLuAO17637319;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wVHtQDIjeFjUldFi17032699() {     double yFxaXxkpWbsbFoHZk53096287 = -955402424;    double yFxaXxkpWbsbFoHZk67857445 = -281066434;    double yFxaXxkpWbsbFoHZk8646124 = -704044345;    double yFxaXxkpWbsbFoHZk85008440 = -282568342;    double yFxaXxkpWbsbFoHZk74367874 = -25380830;    double yFxaXxkpWbsbFoHZk40199196 = -329446328;    double yFxaXxkpWbsbFoHZk38344539 = -791656205;    double yFxaXxkpWbsbFoHZk63623418 = -463315827;    double yFxaXxkpWbsbFoHZk98589977 = -166072800;    double yFxaXxkpWbsbFoHZk989129 = -835342305;    double yFxaXxkpWbsbFoHZk29482458 = -469125523;    double yFxaXxkpWbsbFoHZk32402076 = -537457306;    double yFxaXxkpWbsbFoHZk2526894 = -338170539;    double yFxaXxkpWbsbFoHZk97584271 = -513204218;    double yFxaXxkpWbsbFoHZk97896590 = -441086625;    double yFxaXxkpWbsbFoHZk43110085 = -732968942;    double yFxaXxkpWbsbFoHZk93824650 = -390757413;    double yFxaXxkpWbsbFoHZk52035361 = -724143074;    double yFxaXxkpWbsbFoHZk69705380 = -114106750;    double yFxaXxkpWbsbFoHZk31302459 = -952372163;    double yFxaXxkpWbsbFoHZk82625893 = -527198964;    double yFxaXxkpWbsbFoHZk66789591 = -845001489;    double yFxaXxkpWbsbFoHZk5124412 = -489202167;    double yFxaXxkpWbsbFoHZk40494416 = -182849919;    double yFxaXxkpWbsbFoHZk89067115 = -742724749;    double yFxaXxkpWbsbFoHZk73096646 = -791812146;    double yFxaXxkpWbsbFoHZk92805338 = -298596450;    double yFxaXxkpWbsbFoHZk13728009 = 33213023;    double yFxaXxkpWbsbFoHZk87044765 = -667439410;    double yFxaXxkpWbsbFoHZk79918647 = -984884521;    double yFxaXxkpWbsbFoHZk18972982 = -638885330;    double yFxaXxkpWbsbFoHZk30781084 = -692302504;    double yFxaXxkpWbsbFoHZk99076448 = -799149572;    double yFxaXxkpWbsbFoHZk45248766 = -184044823;    double yFxaXxkpWbsbFoHZk54802298 = -476038786;    double yFxaXxkpWbsbFoHZk18620482 = -63863428;    double yFxaXxkpWbsbFoHZk96167811 = 58345711;    double yFxaXxkpWbsbFoHZk13001009 = -955892410;    double yFxaXxkpWbsbFoHZk29689067 = -308110923;    double yFxaXxkpWbsbFoHZk43433228 = -151062293;    double yFxaXxkpWbsbFoHZk27272427 = -353794028;    double yFxaXxkpWbsbFoHZk63187970 = -923770100;    double yFxaXxkpWbsbFoHZk28744211 = -522343812;    double yFxaXxkpWbsbFoHZk84440714 = 54603872;    double yFxaXxkpWbsbFoHZk32509024 = -777422854;    double yFxaXxkpWbsbFoHZk37379952 = -847780761;    double yFxaXxkpWbsbFoHZk22635725 = -253652316;    double yFxaXxkpWbsbFoHZk69687033 = -199139266;    double yFxaXxkpWbsbFoHZk44497168 = -146317160;    double yFxaXxkpWbsbFoHZk35544016 = -210597975;    double yFxaXxkpWbsbFoHZk41783067 = -480027724;    double yFxaXxkpWbsbFoHZk41286640 = -323154435;    double yFxaXxkpWbsbFoHZk75061622 = -675091565;    double yFxaXxkpWbsbFoHZk79841595 = 79042408;    double yFxaXxkpWbsbFoHZk34406805 = -916892;    double yFxaXxkpWbsbFoHZk86306696 = -10400935;    double yFxaXxkpWbsbFoHZk62733033 = -791864267;    double yFxaXxkpWbsbFoHZk68151707 = -421194427;    double yFxaXxkpWbsbFoHZk95941324 = -539843594;    double yFxaXxkpWbsbFoHZk1271229 = -233568684;    double yFxaXxkpWbsbFoHZk47393857 = 69150121;    double yFxaXxkpWbsbFoHZk24616531 = -724869228;    double yFxaXxkpWbsbFoHZk76578652 = -795876417;    double yFxaXxkpWbsbFoHZk18671330 = -181188279;    double yFxaXxkpWbsbFoHZk82016146 = -96456976;    double yFxaXxkpWbsbFoHZk98701373 = -776823019;    double yFxaXxkpWbsbFoHZk33325627 = -738307734;    double yFxaXxkpWbsbFoHZk57278127 = -54125716;    double yFxaXxkpWbsbFoHZk42781973 = 62834568;    double yFxaXxkpWbsbFoHZk79276108 = -277223197;    double yFxaXxkpWbsbFoHZk46942273 = -691314653;    double yFxaXxkpWbsbFoHZk80823642 = -434865004;    double yFxaXxkpWbsbFoHZk22346295 = -316032151;    double yFxaXxkpWbsbFoHZk26272152 = -963044457;    double yFxaXxkpWbsbFoHZk4030032 = -498578135;    double yFxaXxkpWbsbFoHZk19437924 = -603428864;    double yFxaXxkpWbsbFoHZk38045380 = -222657677;    double yFxaXxkpWbsbFoHZk20683698 = -443806040;    double yFxaXxkpWbsbFoHZk7985393 = -405427065;    double yFxaXxkpWbsbFoHZk51687163 = -894943988;    double yFxaXxkpWbsbFoHZk50460922 = -438159831;    double yFxaXxkpWbsbFoHZk23118306 = 542816;    double yFxaXxkpWbsbFoHZk69230841 = -820469818;    double yFxaXxkpWbsbFoHZk51500750 = -356841435;    double yFxaXxkpWbsbFoHZk38135580 = -404856798;    double yFxaXxkpWbsbFoHZk77686342 = -215730896;    double yFxaXxkpWbsbFoHZk55719461 = 82789060;    double yFxaXxkpWbsbFoHZk19234854 = -778191981;    double yFxaXxkpWbsbFoHZk10841961 = -83127932;    double yFxaXxkpWbsbFoHZk68495602 = -365637851;    double yFxaXxkpWbsbFoHZk55887449 = -271999162;    double yFxaXxkpWbsbFoHZk28016104 = -520459863;    double yFxaXxkpWbsbFoHZk17059684 = -316048816;    double yFxaXxkpWbsbFoHZk28417838 = 25457761;    double yFxaXxkpWbsbFoHZk96039371 = -120212415;    double yFxaXxkpWbsbFoHZk2655897 = -628924800;    double yFxaXxkpWbsbFoHZk86609317 = -27893683;    double yFxaXxkpWbsbFoHZk10072881 = -241155533;    double yFxaXxkpWbsbFoHZk2424568 = -848939153;    double yFxaXxkpWbsbFoHZk33807650 = -955402424;     yFxaXxkpWbsbFoHZk53096287 = yFxaXxkpWbsbFoHZk67857445;     yFxaXxkpWbsbFoHZk67857445 = yFxaXxkpWbsbFoHZk8646124;     yFxaXxkpWbsbFoHZk8646124 = yFxaXxkpWbsbFoHZk85008440;     yFxaXxkpWbsbFoHZk85008440 = yFxaXxkpWbsbFoHZk74367874;     yFxaXxkpWbsbFoHZk74367874 = yFxaXxkpWbsbFoHZk40199196;     yFxaXxkpWbsbFoHZk40199196 = yFxaXxkpWbsbFoHZk38344539;     yFxaXxkpWbsbFoHZk38344539 = yFxaXxkpWbsbFoHZk63623418;     yFxaXxkpWbsbFoHZk63623418 = yFxaXxkpWbsbFoHZk98589977;     yFxaXxkpWbsbFoHZk98589977 = yFxaXxkpWbsbFoHZk989129;     yFxaXxkpWbsbFoHZk989129 = yFxaXxkpWbsbFoHZk29482458;     yFxaXxkpWbsbFoHZk29482458 = yFxaXxkpWbsbFoHZk32402076;     yFxaXxkpWbsbFoHZk32402076 = yFxaXxkpWbsbFoHZk2526894;     yFxaXxkpWbsbFoHZk2526894 = yFxaXxkpWbsbFoHZk97584271;     yFxaXxkpWbsbFoHZk97584271 = yFxaXxkpWbsbFoHZk97896590;     yFxaXxkpWbsbFoHZk97896590 = yFxaXxkpWbsbFoHZk43110085;     yFxaXxkpWbsbFoHZk43110085 = yFxaXxkpWbsbFoHZk93824650;     yFxaXxkpWbsbFoHZk93824650 = yFxaXxkpWbsbFoHZk52035361;     yFxaXxkpWbsbFoHZk52035361 = yFxaXxkpWbsbFoHZk69705380;     yFxaXxkpWbsbFoHZk69705380 = yFxaXxkpWbsbFoHZk31302459;     yFxaXxkpWbsbFoHZk31302459 = yFxaXxkpWbsbFoHZk82625893;     yFxaXxkpWbsbFoHZk82625893 = yFxaXxkpWbsbFoHZk66789591;     yFxaXxkpWbsbFoHZk66789591 = yFxaXxkpWbsbFoHZk5124412;     yFxaXxkpWbsbFoHZk5124412 = yFxaXxkpWbsbFoHZk40494416;     yFxaXxkpWbsbFoHZk40494416 = yFxaXxkpWbsbFoHZk89067115;     yFxaXxkpWbsbFoHZk89067115 = yFxaXxkpWbsbFoHZk73096646;     yFxaXxkpWbsbFoHZk73096646 = yFxaXxkpWbsbFoHZk92805338;     yFxaXxkpWbsbFoHZk92805338 = yFxaXxkpWbsbFoHZk13728009;     yFxaXxkpWbsbFoHZk13728009 = yFxaXxkpWbsbFoHZk87044765;     yFxaXxkpWbsbFoHZk87044765 = yFxaXxkpWbsbFoHZk79918647;     yFxaXxkpWbsbFoHZk79918647 = yFxaXxkpWbsbFoHZk18972982;     yFxaXxkpWbsbFoHZk18972982 = yFxaXxkpWbsbFoHZk30781084;     yFxaXxkpWbsbFoHZk30781084 = yFxaXxkpWbsbFoHZk99076448;     yFxaXxkpWbsbFoHZk99076448 = yFxaXxkpWbsbFoHZk45248766;     yFxaXxkpWbsbFoHZk45248766 = yFxaXxkpWbsbFoHZk54802298;     yFxaXxkpWbsbFoHZk54802298 = yFxaXxkpWbsbFoHZk18620482;     yFxaXxkpWbsbFoHZk18620482 = yFxaXxkpWbsbFoHZk96167811;     yFxaXxkpWbsbFoHZk96167811 = yFxaXxkpWbsbFoHZk13001009;     yFxaXxkpWbsbFoHZk13001009 = yFxaXxkpWbsbFoHZk29689067;     yFxaXxkpWbsbFoHZk29689067 = yFxaXxkpWbsbFoHZk43433228;     yFxaXxkpWbsbFoHZk43433228 = yFxaXxkpWbsbFoHZk27272427;     yFxaXxkpWbsbFoHZk27272427 = yFxaXxkpWbsbFoHZk63187970;     yFxaXxkpWbsbFoHZk63187970 = yFxaXxkpWbsbFoHZk28744211;     yFxaXxkpWbsbFoHZk28744211 = yFxaXxkpWbsbFoHZk84440714;     yFxaXxkpWbsbFoHZk84440714 = yFxaXxkpWbsbFoHZk32509024;     yFxaXxkpWbsbFoHZk32509024 = yFxaXxkpWbsbFoHZk37379952;     yFxaXxkpWbsbFoHZk37379952 = yFxaXxkpWbsbFoHZk22635725;     yFxaXxkpWbsbFoHZk22635725 = yFxaXxkpWbsbFoHZk69687033;     yFxaXxkpWbsbFoHZk69687033 = yFxaXxkpWbsbFoHZk44497168;     yFxaXxkpWbsbFoHZk44497168 = yFxaXxkpWbsbFoHZk35544016;     yFxaXxkpWbsbFoHZk35544016 = yFxaXxkpWbsbFoHZk41783067;     yFxaXxkpWbsbFoHZk41783067 = yFxaXxkpWbsbFoHZk41286640;     yFxaXxkpWbsbFoHZk41286640 = yFxaXxkpWbsbFoHZk75061622;     yFxaXxkpWbsbFoHZk75061622 = yFxaXxkpWbsbFoHZk79841595;     yFxaXxkpWbsbFoHZk79841595 = yFxaXxkpWbsbFoHZk34406805;     yFxaXxkpWbsbFoHZk34406805 = yFxaXxkpWbsbFoHZk86306696;     yFxaXxkpWbsbFoHZk86306696 = yFxaXxkpWbsbFoHZk62733033;     yFxaXxkpWbsbFoHZk62733033 = yFxaXxkpWbsbFoHZk68151707;     yFxaXxkpWbsbFoHZk68151707 = yFxaXxkpWbsbFoHZk95941324;     yFxaXxkpWbsbFoHZk95941324 = yFxaXxkpWbsbFoHZk1271229;     yFxaXxkpWbsbFoHZk1271229 = yFxaXxkpWbsbFoHZk47393857;     yFxaXxkpWbsbFoHZk47393857 = yFxaXxkpWbsbFoHZk24616531;     yFxaXxkpWbsbFoHZk24616531 = yFxaXxkpWbsbFoHZk76578652;     yFxaXxkpWbsbFoHZk76578652 = yFxaXxkpWbsbFoHZk18671330;     yFxaXxkpWbsbFoHZk18671330 = yFxaXxkpWbsbFoHZk82016146;     yFxaXxkpWbsbFoHZk82016146 = yFxaXxkpWbsbFoHZk98701373;     yFxaXxkpWbsbFoHZk98701373 = yFxaXxkpWbsbFoHZk33325627;     yFxaXxkpWbsbFoHZk33325627 = yFxaXxkpWbsbFoHZk57278127;     yFxaXxkpWbsbFoHZk57278127 = yFxaXxkpWbsbFoHZk42781973;     yFxaXxkpWbsbFoHZk42781973 = yFxaXxkpWbsbFoHZk79276108;     yFxaXxkpWbsbFoHZk79276108 = yFxaXxkpWbsbFoHZk46942273;     yFxaXxkpWbsbFoHZk46942273 = yFxaXxkpWbsbFoHZk80823642;     yFxaXxkpWbsbFoHZk80823642 = yFxaXxkpWbsbFoHZk22346295;     yFxaXxkpWbsbFoHZk22346295 = yFxaXxkpWbsbFoHZk26272152;     yFxaXxkpWbsbFoHZk26272152 = yFxaXxkpWbsbFoHZk4030032;     yFxaXxkpWbsbFoHZk4030032 = yFxaXxkpWbsbFoHZk19437924;     yFxaXxkpWbsbFoHZk19437924 = yFxaXxkpWbsbFoHZk38045380;     yFxaXxkpWbsbFoHZk38045380 = yFxaXxkpWbsbFoHZk20683698;     yFxaXxkpWbsbFoHZk20683698 = yFxaXxkpWbsbFoHZk7985393;     yFxaXxkpWbsbFoHZk7985393 = yFxaXxkpWbsbFoHZk51687163;     yFxaXxkpWbsbFoHZk51687163 = yFxaXxkpWbsbFoHZk50460922;     yFxaXxkpWbsbFoHZk50460922 = yFxaXxkpWbsbFoHZk23118306;     yFxaXxkpWbsbFoHZk23118306 = yFxaXxkpWbsbFoHZk69230841;     yFxaXxkpWbsbFoHZk69230841 = yFxaXxkpWbsbFoHZk51500750;     yFxaXxkpWbsbFoHZk51500750 = yFxaXxkpWbsbFoHZk38135580;     yFxaXxkpWbsbFoHZk38135580 = yFxaXxkpWbsbFoHZk77686342;     yFxaXxkpWbsbFoHZk77686342 = yFxaXxkpWbsbFoHZk55719461;     yFxaXxkpWbsbFoHZk55719461 = yFxaXxkpWbsbFoHZk19234854;     yFxaXxkpWbsbFoHZk19234854 = yFxaXxkpWbsbFoHZk10841961;     yFxaXxkpWbsbFoHZk10841961 = yFxaXxkpWbsbFoHZk68495602;     yFxaXxkpWbsbFoHZk68495602 = yFxaXxkpWbsbFoHZk55887449;     yFxaXxkpWbsbFoHZk55887449 = yFxaXxkpWbsbFoHZk28016104;     yFxaXxkpWbsbFoHZk28016104 = yFxaXxkpWbsbFoHZk17059684;     yFxaXxkpWbsbFoHZk17059684 = yFxaXxkpWbsbFoHZk28417838;     yFxaXxkpWbsbFoHZk28417838 = yFxaXxkpWbsbFoHZk96039371;     yFxaXxkpWbsbFoHZk96039371 = yFxaXxkpWbsbFoHZk2655897;     yFxaXxkpWbsbFoHZk2655897 = yFxaXxkpWbsbFoHZk86609317;     yFxaXxkpWbsbFoHZk86609317 = yFxaXxkpWbsbFoHZk10072881;     yFxaXxkpWbsbFoHZk10072881 = yFxaXxkpWbsbFoHZk2424568;     yFxaXxkpWbsbFoHZk2424568 = yFxaXxkpWbsbFoHZk33807650;     yFxaXxkpWbsbFoHZk33807650 = yFxaXxkpWbsbFoHZk53096287;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void GyEpzZdQLimvNaKU32081767() {     double JCagMHnanNCxDgOoC59213394 = 32695687;    double JCagMHnanNCxDgOoC11230725 = 9333786;    double JCagMHnanNCxDgOoC24688308 = -854877266;    double JCagMHnanNCxDgOoC12083221 = -473316973;    double JCagMHnanNCxDgOoC43263363 = -546030701;    double JCagMHnanNCxDgOoC31616398 = -593942563;    double JCagMHnanNCxDgOoC98833505 = -930621182;    double JCagMHnanNCxDgOoC13383309 = -750189740;    double JCagMHnanNCxDgOoC63232472 = 65894534;    double JCagMHnanNCxDgOoC42218788 = -64317783;    double JCagMHnanNCxDgOoC89592748 = -346563110;    double JCagMHnanNCxDgOoC64225118 = -981863508;    double JCagMHnanNCxDgOoC88736011 = -778061628;    double JCagMHnanNCxDgOoC80727948 = 54557215;    double JCagMHnanNCxDgOoC93639016 = -31548791;    double JCagMHnanNCxDgOoC84615478 = -705968485;    double JCagMHnanNCxDgOoC87660654 = 92697474;    double JCagMHnanNCxDgOoC49088514 = -389665716;    double JCagMHnanNCxDgOoC31535222 = -519875847;    double JCagMHnanNCxDgOoC11852623 = -209275082;    double JCagMHnanNCxDgOoC71179087 = -382686419;    double JCagMHnanNCxDgOoC53310614 = -752931206;    double JCagMHnanNCxDgOoC40715203 = -235719654;    double JCagMHnanNCxDgOoC24486790 = -405154906;    double JCagMHnanNCxDgOoC32766765 = -15920782;    double JCagMHnanNCxDgOoC97818321 = -670467470;    double JCagMHnanNCxDgOoC91265117 = -199258653;    double JCagMHnanNCxDgOoC26835134 = -32715931;    double JCagMHnanNCxDgOoC53177021 = -889822115;    double JCagMHnanNCxDgOoC48534284 = -802355389;    double JCagMHnanNCxDgOoC11215532 = -552206268;    double JCagMHnanNCxDgOoC28391706 = -401749320;    double JCagMHnanNCxDgOoC8787397 = -691762943;    double JCagMHnanNCxDgOoC40079330 = 64413412;    double JCagMHnanNCxDgOoC16276951 = -14771891;    double JCagMHnanNCxDgOoC87346395 = -277889536;    double JCagMHnanNCxDgOoC77429746 = -995290159;    double JCagMHnanNCxDgOoC47091221 = -727398419;    double JCagMHnanNCxDgOoC3299321 = -302034420;    double JCagMHnanNCxDgOoC30935166 = -624501516;    double JCagMHnanNCxDgOoC32941651 = -419067922;    double JCagMHnanNCxDgOoC95338426 = -93912378;    double JCagMHnanNCxDgOoC6811189 = -603148396;    double JCagMHnanNCxDgOoC71848454 = 35260934;    double JCagMHnanNCxDgOoC44461488 = -439321008;    double JCagMHnanNCxDgOoC49319147 = -832996877;    double JCagMHnanNCxDgOoC40414299 = -372427853;    double JCagMHnanNCxDgOoC77643974 = -209034799;    double JCagMHnanNCxDgOoC54172897 = -232054148;    double JCagMHnanNCxDgOoC48455668 = -297608783;    double JCagMHnanNCxDgOoC77483745 = -322321395;    double JCagMHnanNCxDgOoC52096359 = -123880755;    double JCagMHnanNCxDgOoC23310080 = -466881153;    double JCagMHnanNCxDgOoC75810475 = -241427737;    double JCagMHnanNCxDgOoC60902680 = -701448675;    double JCagMHnanNCxDgOoC5902780 = -214373108;    double JCagMHnanNCxDgOoC70515521 = -754946561;    double JCagMHnanNCxDgOoC201518 = -349722360;    double JCagMHnanNCxDgOoC79316455 = -357396192;    double JCagMHnanNCxDgOoC45445042 = -875563232;    double JCagMHnanNCxDgOoC40351281 = -294683911;    double JCagMHnanNCxDgOoC71998371 = -797905252;    double JCagMHnanNCxDgOoC60206288 = -860367626;    double JCagMHnanNCxDgOoC14698189 = -131750078;    double JCagMHnanNCxDgOoC31003256 = -512111515;    double JCagMHnanNCxDgOoC61201042 = -944813790;    double JCagMHnanNCxDgOoC55437721 = -190100565;    double JCagMHnanNCxDgOoC48656682 = -742475040;    double JCagMHnanNCxDgOoC64450998 = -930670895;    double JCagMHnanNCxDgOoC6292622 = -753659256;    double JCagMHnanNCxDgOoC7185732 = -710678327;    double JCagMHnanNCxDgOoC40569433 = -179904107;    double JCagMHnanNCxDgOoC45789194 = 12368703;    double JCagMHnanNCxDgOoC600057 = -895374331;    double JCagMHnanNCxDgOoC78910972 = -790207161;    double JCagMHnanNCxDgOoC75840660 = -188774042;    double JCagMHnanNCxDgOoC46499426 = -49782810;    double JCagMHnanNCxDgOoC68866749 = -170980588;    double JCagMHnanNCxDgOoC80025301 = -965833899;    double JCagMHnanNCxDgOoC83447618 = -182923905;    double JCagMHnanNCxDgOoC57404023 = -198039617;    double JCagMHnanNCxDgOoC13621144 = -990223855;    double JCagMHnanNCxDgOoC72662237 = -800661783;    double JCagMHnanNCxDgOoC4721353 = -492213332;    double JCagMHnanNCxDgOoC71050538 = -380033994;    double JCagMHnanNCxDgOoC59119172 = -328325514;    double JCagMHnanNCxDgOoC5081626 = -934868167;    double JCagMHnanNCxDgOoC32976922 = -350335206;    double JCagMHnanNCxDgOoC79176650 = -234137914;    double JCagMHnanNCxDgOoC10374172 = -800398784;    double JCagMHnanNCxDgOoC16830875 = -522942976;    double JCagMHnanNCxDgOoC77228229 = -545567799;    double JCagMHnanNCxDgOoC67774765 = -270002228;    double JCagMHnanNCxDgOoC57854278 = -426471188;    double JCagMHnanNCxDgOoC90583884 = -229817606;    double JCagMHnanNCxDgOoC60943280 = -621162671;    double JCagMHnanNCxDgOoC35132139 = -233544752;    double JCagMHnanNCxDgOoC92113000 = -371398319;    double JCagMHnanNCxDgOoC40845198 = -452627552;    double JCagMHnanNCxDgOoC83260446 = 32695687;     JCagMHnanNCxDgOoC59213394 = JCagMHnanNCxDgOoC11230725;     JCagMHnanNCxDgOoC11230725 = JCagMHnanNCxDgOoC24688308;     JCagMHnanNCxDgOoC24688308 = JCagMHnanNCxDgOoC12083221;     JCagMHnanNCxDgOoC12083221 = JCagMHnanNCxDgOoC43263363;     JCagMHnanNCxDgOoC43263363 = JCagMHnanNCxDgOoC31616398;     JCagMHnanNCxDgOoC31616398 = JCagMHnanNCxDgOoC98833505;     JCagMHnanNCxDgOoC98833505 = JCagMHnanNCxDgOoC13383309;     JCagMHnanNCxDgOoC13383309 = JCagMHnanNCxDgOoC63232472;     JCagMHnanNCxDgOoC63232472 = JCagMHnanNCxDgOoC42218788;     JCagMHnanNCxDgOoC42218788 = JCagMHnanNCxDgOoC89592748;     JCagMHnanNCxDgOoC89592748 = JCagMHnanNCxDgOoC64225118;     JCagMHnanNCxDgOoC64225118 = JCagMHnanNCxDgOoC88736011;     JCagMHnanNCxDgOoC88736011 = JCagMHnanNCxDgOoC80727948;     JCagMHnanNCxDgOoC80727948 = JCagMHnanNCxDgOoC93639016;     JCagMHnanNCxDgOoC93639016 = JCagMHnanNCxDgOoC84615478;     JCagMHnanNCxDgOoC84615478 = JCagMHnanNCxDgOoC87660654;     JCagMHnanNCxDgOoC87660654 = JCagMHnanNCxDgOoC49088514;     JCagMHnanNCxDgOoC49088514 = JCagMHnanNCxDgOoC31535222;     JCagMHnanNCxDgOoC31535222 = JCagMHnanNCxDgOoC11852623;     JCagMHnanNCxDgOoC11852623 = JCagMHnanNCxDgOoC71179087;     JCagMHnanNCxDgOoC71179087 = JCagMHnanNCxDgOoC53310614;     JCagMHnanNCxDgOoC53310614 = JCagMHnanNCxDgOoC40715203;     JCagMHnanNCxDgOoC40715203 = JCagMHnanNCxDgOoC24486790;     JCagMHnanNCxDgOoC24486790 = JCagMHnanNCxDgOoC32766765;     JCagMHnanNCxDgOoC32766765 = JCagMHnanNCxDgOoC97818321;     JCagMHnanNCxDgOoC97818321 = JCagMHnanNCxDgOoC91265117;     JCagMHnanNCxDgOoC91265117 = JCagMHnanNCxDgOoC26835134;     JCagMHnanNCxDgOoC26835134 = JCagMHnanNCxDgOoC53177021;     JCagMHnanNCxDgOoC53177021 = JCagMHnanNCxDgOoC48534284;     JCagMHnanNCxDgOoC48534284 = JCagMHnanNCxDgOoC11215532;     JCagMHnanNCxDgOoC11215532 = JCagMHnanNCxDgOoC28391706;     JCagMHnanNCxDgOoC28391706 = JCagMHnanNCxDgOoC8787397;     JCagMHnanNCxDgOoC8787397 = JCagMHnanNCxDgOoC40079330;     JCagMHnanNCxDgOoC40079330 = JCagMHnanNCxDgOoC16276951;     JCagMHnanNCxDgOoC16276951 = JCagMHnanNCxDgOoC87346395;     JCagMHnanNCxDgOoC87346395 = JCagMHnanNCxDgOoC77429746;     JCagMHnanNCxDgOoC77429746 = JCagMHnanNCxDgOoC47091221;     JCagMHnanNCxDgOoC47091221 = JCagMHnanNCxDgOoC3299321;     JCagMHnanNCxDgOoC3299321 = JCagMHnanNCxDgOoC30935166;     JCagMHnanNCxDgOoC30935166 = JCagMHnanNCxDgOoC32941651;     JCagMHnanNCxDgOoC32941651 = JCagMHnanNCxDgOoC95338426;     JCagMHnanNCxDgOoC95338426 = JCagMHnanNCxDgOoC6811189;     JCagMHnanNCxDgOoC6811189 = JCagMHnanNCxDgOoC71848454;     JCagMHnanNCxDgOoC71848454 = JCagMHnanNCxDgOoC44461488;     JCagMHnanNCxDgOoC44461488 = JCagMHnanNCxDgOoC49319147;     JCagMHnanNCxDgOoC49319147 = JCagMHnanNCxDgOoC40414299;     JCagMHnanNCxDgOoC40414299 = JCagMHnanNCxDgOoC77643974;     JCagMHnanNCxDgOoC77643974 = JCagMHnanNCxDgOoC54172897;     JCagMHnanNCxDgOoC54172897 = JCagMHnanNCxDgOoC48455668;     JCagMHnanNCxDgOoC48455668 = JCagMHnanNCxDgOoC77483745;     JCagMHnanNCxDgOoC77483745 = JCagMHnanNCxDgOoC52096359;     JCagMHnanNCxDgOoC52096359 = JCagMHnanNCxDgOoC23310080;     JCagMHnanNCxDgOoC23310080 = JCagMHnanNCxDgOoC75810475;     JCagMHnanNCxDgOoC75810475 = JCagMHnanNCxDgOoC60902680;     JCagMHnanNCxDgOoC60902680 = JCagMHnanNCxDgOoC5902780;     JCagMHnanNCxDgOoC5902780 = JCagMHnanNCxDgOoC70515521;     JCagMHnanNCxDgOoC70515521 = JCagMHnanNCxDgOoC201518;     JCagMHnanNCxDgOoC201518 = JCagMHnanNCxDgOoC79316455;     JCagMHnanNCxDgOoC79316455 = JCagMHnanNCxDgOoC45445042;     JCagMHnanNCxDgOoC45445042 = JCagMHnanNCxDgOoC40351281;     JCagMHnanNCxDgOoC40351281 = JCagMHnanNCxDgOoC71998371;     JCagMHnanNCxDgOoC71998371 = JCagMHnanNCxDgOoC60206288;     JCagMHnanNCxDgOoC60206288 = JCagMHnanNCxDgOoC14698189;     JCagMHnanNCxDgOoC14698189 = JCagMHnanNCxDgOoC31003256;     JCagMHnanNCxDgOoC31003256 = JCagMHnanNCxDgOoC61201042;     JCagMHnanNCxDgOoC61201042 = JCagMHnanNCxDgOoC55437721;     JCagMHnanNCxDgOoC55437721 = JCagMHnanNCxDgOoC48656682;     JCagMHnanNCxDgOoC48656682 = JCagMHnanNCxDgOoC64450998;     JCagMHnanNCxDgOoC64450998 = JCagMHnanNCxDgOoC6292622;     JCagMHnanNCxDgOoC6292622 = JCagMHnanNCxDgOoC7185732;     JCagMHnanNCxDgOoC7185732 = JCagMHnanNCxDgOoC40569433;     JCagMHnanNCxDgOoC40569433 = JCagMHnanNCxDgOoC45789194;     JCagMHnanNCxDgOoC45789194 = JCagMHnanNCxDgOoC600057;     JCagMHnanNCxDgOoC600057 = JCagMHnanNCxDgOoC78910972;     JCagMHnanNCxDgOoC78910972 = JCagMHnanNCxDgOoC75840660;     JCagMHnanNCxDgOoC75840660 = JCagMHnanNCxDgOoC46499426;     JCagMHnanNCxDgOoC46499426 = JCagMHnanNCxDgOoC68866749;     JCagMHnanNCxDgOoC68866749 = JCagMHnanNCxDgOoC80025301;     JCagMHnanNCxDgOoC80025301 = JCagMHnanNCxDgOoC83447618;     JCagMHnanNCxDgOoC83447618 = JCagMHnanNCxDgOoC57404023;     JCagMHnanNCxDgOoC57404023 = JCagMHnanNCxDgOoC13621144;     JCagMHnanNCxDgOoC13621144 = JCagMHnanNCxDgOoC72662237;     JCagMHnanNCxDgOoC72662237 = JCagMHnanNCxDgOoC4721353;     JCagMHnanNCxDgOoC4721353 = JCagMHnanNCxDgOoC71050538;     JCagMHnanNCxDgOoC71050538 = JCagMHnanNCxDgOoC59119172;     JCagMHnanNCxDgOoC59119172 = JCagMHnanNCxDgOoC5081626;     JCagMHnanNCxDgOoC5081626 = JCagMHnanNCxDgOoC32976922;     JCagMHnanNCxDgOoC32976922 = JCagMHnanNCxDgOoC79176650;     JCagMHnanNCxDgOoC79176650 = JCagMHnanNCxDgOoC10374172;     JCagMHnanNCxDgOoC10374172 = JCagMHnanNCxDgOoC16830875;     JCagMHnanNCxDgOoC16830875 = JCagMHnanNCxDgOoC77228229;     JCagMHnanNCxDgOoC77228229 = JCagMHnanNCxDgOoC67774765;     JCagMHnanNCxDgOoC67774765 = JCagMHnanNCxDgOoC57854278;     JCagMHnanNCxDgOoC57854278 = JCagMHnanNCxDgOoC90583884;     JCagMHnanNCxDgOoC90583884 = JCagMHnanNCxDgOoC60943280;     JCagMHnanNCxDgOoC60943280 = JCagMHnanNCxDgOoC35132139;     JCagMHnanNCxDgOoC35132139 = JCagMHnanNCxDgOoC92113000;     JCagMHnanNCxDgOoC92113000 = JCagMHnanNCxDgOoC40845198;     JCagMHnanNCxDgOoC40845198 = JCagMHnanNCxDgOoC83260446;     JCagMHnanNCxDgOoC83260446 = JCagMHnanNCxDgOoC59213394;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void RCMldHtbfKASzhdd68726741() {     double QyztzTpUYlMyVbvHR81600511 = -701708276;    double QyztzTpUYlMyVbvHR95578224 = 60790731;    double QyztzTpUYlMyVbvHR53457586 = 5468890;    double QyztzTpUYlMyVbvHR27089910 = -479724504;    double QyztzTpUYlMyVbvHR41437617 = -37087703;    double QyztzTpUYlMyVbvHR46499515 = -454550941;    double QyztzTpUYlMyVbvHR90989244 = -752608488;    double QyztzTpUYlMyVbvHR89017167 = -120786384;    double QyztzTpUYlMyVbvHR60395604 = -918301505;    double QyztzTpUYlMyVbvHR33982372 = -907059116;    double QyztzTpUYlMyVbvHR27134608 = -343187510;    double QyztzTpUYlMyVbvHR20128067 = -132608801;    double QyztzTpUYlMyVbvHR55744192 = -296778947;    double QyztzTpUYlMyVbvHR23342701 = -473151542;    double QyztzTpUYlMyVbvHR30374294 = -759026504;    double QyztzTpUYlMyVbvHR28871391 = 12363972;    double QyztzTpUYlMyVbvHR37484006 = -184582346;    double QyztzTpUYlMyVbvHR55064930 = -841408407;    double QyztzTpUYlMyVbvHR72816853 = -654977972;    double QyztzTpUYlMyVbvHR90368906 = -461909536;    double QyztzTpUYlMyVbvHR72062194 = -374173728;    double QyztzTpUYlMyVbvHR17125833 = -157697519;    double QyztzTpUYlMyVbvHR44008527 = -411800246;    double QyztzTpUYlMyVbvHR50986577 = -969147764;    double QyztzTpUYlMyVbvHR68307012 = -737479906;    double QyztzTpUYlMyVbvHR86121316 = -14361693;    double QyztzTpUYlMyVbvHR38354538 = 43154854;    double QyztzTpUYlMyVbvHR32746584 = -921170384;    double QyztzTpUYlMyVbvHR60069582 = -134079234;    double QyztzTpUYlMyVbvHR57691463 = -722651255;    double QyztzTpUYlMyVbvHR55506337 = -633522420;    double QyztzTpUYlMyVbvHR85932896 = 46179325;    double QyztzTpUYlMyVbvHR89826067 = -121106438;    double QyztzTpUYlMyVbvHR99468017 = -259713438;    double QyztzTpUYlMyVbvHR92506315 = -78528314;    double QyztzTpUYlMyVbvHR29039265 = -593284912;    double QyztzTpUYlMyVbvHR70869459 = -728238111;    double QyztzTpUYlMyVbvHR717322 = -595023891;    double QyztzTpUYlMyVbvHR51607451 = -703080451;    double QyztzTpUYlMyVbvHR51120880 = -760725545;    double QyztzTpUYlMyVbvHR61699022 = -922245588;    double QyztzTpUYlMyVbvHR83846818 = -951821069;    double QyztzTpUYlMyVbvHR12361381 = -490678980;    double QyztzTpUYlMyVbvHR71292573 = 37897285;    double QyztzTpUYlMyVbvHR12444755 = -248464486;    double QyztzTpUYlMyVbvHR57419429 = 84774027;    double QyztzTpUYlMyVbvHR52711696 = -950838106;    double QyztzTpUYlMyVbvHR22242649 = 34430211;    double QyztzTpUYlMyVbvHR48632802 = -181770846;    double QyztzTpUYlMyVbvHR45539414 = -439629042;    double QyztzTpUYlMyVbvHR86352962 = -677397672;    double QyztzTpUYlMyVbvHR25947617 = -554700037;    double QyztzTpUYlMyVbvHR1624484 = -138792383;    double QyztzTpUYlMyVbvHR36097020 = -517108546;    double QyztzTpUYlMyVbvHR96664547 = -842152790;    double QyztzTpUYlMyVbvHR64474678 = -444010758;    double QyztzTpUYlMyVbvHR51569698 = -527409023;    double QyztzTpUYlMyVbvHR2471009 = -25383347;    double QyztzTpUYlMyVbvHR58782897 = -742244599;    double QyztzTpUYlMyVbvHR55316301 = 77273990;    double QyztzTpUYlMyVbvHR8144978 = -397705796;    double QyztzTpUYlMyVbvHR58242660 = -831438104;    double QyztzTpUYlMyVbvHR28947586 = -986707151;    double QyztzTpUYlMyVbvHR2704141 = -95650250;    double QyztzTpUYlMyVbvHR78476035 = -173536696;    double QyztzTpUYlMyVbvHR41201711 = -289366836;    double QyztzTpUYlMyVbvHR30302000 = 88497636;    double QyztzTpUYlMyVbvHR56276175 = 62934491;    double QyztzTpUYlMyVbvHR30836386 = -294623229;    double QyztzTpUYlMyVbvHR1335030 = -65741593;    double QyztzTpUYlMyVbvHR58001932 = -259397918;    double QyztzTpUYlMyVbvHR36766684 = -589558455;    double QyztzTpUYlMyVbvHR3457479 = -38327956;    double QyztzTpUYlMyVbvHR21695973 = -894252427;    double QyztzTpUYlMyVbvHR28669885 = -539663948;    double QyztzTpUYlMyVbvHR88215376 = -422352659;    double QyztzTpUYlMyVbvHR4764452 = -667018539;    double QyztzTpUYlMyVbvHR72715953 = -349697532;    double QyztzTpUYlMyVbvHR38541822 = -620683278;    double QyztzTpUYlMyVbvHR10887583 = -722253934;    double QyztzTpUYlMyVbvHR33409620 = -63523588;    double QyztzTpUYlMyVbvHR16111889 = -991275357;    double QyztzTpUYlMyVbvHR84113782 = -639399538;    double QyztzTpUYlMyVbvHR14530168 = -694450192;    double QyztzTpUYlMyVbvHR71338501 = 54746417;    double QyztzTpUYlMyVbvHR29558721 = 21177616;    double QyztzTpUYlMyVbvHR84308413 = -815028292;    double QyztzTpUYlMyVbvHR53729047 = -603997892;    double QyztzTpUYlMyVbvHR2803470 = -417560649;    double QyztzTpUYlMyVbvHR28031637 = -634517557;    double QyztzTpUYlMyVbvHR77469566 = 34124111;    double QyztzTpUYlMyVbvHR68398450 = -602854764;    double QyztzTpUYlMyVbvHR41934425 = -852779293;    double QyztzTpUYlMyVbvHR96291150 = -680354442;    double QyztzTpUYlMyVbvHR42975903 = -263019750;    double QyztzTpUYlMyVbvHR3456363 = 9192516;    double QyztzTpUYlMyVbvHR54899232 = -965113918;    double QyztzTpUYlMyVbvHR9657241 = -295028730;    double QyztzTpUYlMyVbvHR92816538 = -788566020;    double QyztzTpUYlMyVbvHR71243044 = -701708276;     QyztzTpUYlMyVbvHR81600511 = QyztzTpUYlMyVbvHR95578224;     QyztzTpUYlMyVbvHR95578224 = QyztzTpUYlMyVbvHR53457586;     QyztzTpUYlMyVbvHR53457586 = QyztzTpUYlMyVbvHR27089910;     QyztzTpUYlMyVbvHR27089910 = QyztzTpUYlMyVbvHR41437617;     QyztzTpUYlMyVbvHR41437617 = QyztzTpUYlMyVbvHR46499515;     QyztzTpUYlMyVbvHR46499515 = QyztzTpUYlMyVbvHR90989244;     QyztzTpUYlMyVbvHR90989244 = QyztzTpUYlMyVbvHR89017167;     QyztzTpUYlMyVbvHR89017167 = QyztzTpUYlMyVbvHR60395604;     QyztzTpUYlMyVbvHR60395604 = QyztzTpUYlMyVbvHR33982372;     QyztzTpUYlMyVbvHR33982372 = QyztzTpUYlMyVbvHR27134608;     QyztzTpUYlMyVbvHR27134608 = QyztzTpUYlMyVbvHR20128067;     QyztzTpUYlMyVbvHR20128067 = QyztzTpUYlMyVbvHR55744192;     QyztzTpUYlMyVbvHR55744192 = QyztzTpUYlMyVbvHR23342701;     QyztzTpUYlMyVbvHR23342701 = QyztzTpUYlMyVbvHR30374294;     QyztzTpUYlMyVbvHR30374294 = QyztzTpUYlMyVbvHR28871391;     QyztzTpUYlMyVbvHR28871391 = QyztzTpUYlMyVbvHR37484006;     QyztzTpUYlMyVbvHR37484006 = QyztzTpUYlMyVbvHR55064930;     QyztzTpUYlMyVbvHR55064930 = QyztzTpUYlMyVbvHR72816853;     QyztzTpUYlMyVbvHR72816853 = QyztzTpUYlMyVbvHR90368906;     QyztzTpUYlMyVbvHR90368906 = QyztzTpUYlMyVbvHR72062194;     QyztzTpUYlMyVbvHR72062194 = QyztzTpUYlMyVbvHR17125833;     QyztzTpUYlMyVbvHR17125833 = QyztzTpUYlMyVbvHR44008527;     QyztzTpUYlMyVbvHR44008527 = QyztzTpUYlMyVbvHR50986577;     QyztzTpUYlMyVbvHR50986577 = QyztzTpUYlMyVbvHR68307012;     QyztzTpUYlMyVbvHR68307012 = QyztzTpUYlMyVbvHR86121316;     QyztzTpUYlMyVbvHR86121316 = QyztzTpUYlMyVbvHR38354538;     QyztzTpUYlMyVbvHR38354538 = QyztzTpUYlMyVbvHR32746584;     QyztzTpUYlMyVbvHR32746584 = QyztzTpUYlMyVbvHR60069582;     QyztzTpUYlMyVbvHR60069582 = QyztzTpUYlMyVbvHR57691463;     QyztzTpUYlMyVbvHR57691463 = QyztzTpUYlMyVbvHR55506337;     QyztzTpUYlMyVbvHR55506337 = QyztzTpUYlMyVbvHR85932896;     QyztzTpUYlMyVbvHR85932896 = QyztzTpUYlMyVbvHR89826067;     QyztzTpUYlMyVbvHR89826067 = QyztzTpUYlMyVbvHR99468017;     QyztzTpUYlMyVbvHR99468017 = QyztzTpUYlMyVbvHR92506315;     QyztzTpUYlMyVbvHR92506315 = QyztzTpUYlMyVbvHR29039265;     QyztzTpUYlMyVbvHR29039265 = QyztzTpUYlMyVbvHR70869459;     QyztzTpUYlMyVbvHR70869459 = QyztzTpUYlMyVbvHR717322;     QyztzTpUYlMyVbvHR717322 = QyztzTpUYlMyVbvHR51607451;     QyztzTpUYlMyVbvHR51607451 = QyztzTpUYlMyVbvHR51120880;     QyztzTpUYlMyVbvHR51120880 = QyztzTpUYlMyVbvHR61699022;     QyztzTpUYlMyVbvHR61699022 = QyztzTpUYlMyVbvHR83846818;     QyztzTpUYlMyVbvHR83846818 = QyztzTpUYlMyVbvHR12361381;     QyztzTpUYlMyVbvHR12361381 = QyztzTpUYlMyVbvHR71292573;     QyztzTpUYlMyVbvHR71292573 = QyztzTpUYlMyVbvHR12444755;     QyztzTpUYlMyVbvHR12444755 = QyztzTpUYlMyVbvHR57419429;     QyztzTpUYlMyVbvHR57419429 = QyztzTpUYlMyVbvHR52711696;     QyztzTpUYlMyVbvHR52711696 = QyztzTpUYlMyVbvHR22242649;     QyztzTpUYlMyVbvHR22242649 = QyztzTpUYlMyVbvHR48632802;     QyztzTpUYlMyVbvHR48632802 = QyztzTpUYlMyVbvHR45539414;     QyztzTpUYlMyVbvHR45539414 = QyztzTpUYlMyVbvHR86352962;     QyztzTpUYlMyVbvHR86352962 = QyztzTpUYlMyVbvHR25947617;     QyztzTpUYlMyVbvHR25947617 = QyztzTpUYlMyVbvHR1624484;     QyztzTpUYlMyVbvHR1624484 = QyztzTpUYlMyVbvHR36097020;     QyztzTpUYlMyVbvHR36097020 = QyztzTpUYlMyVbvHR96664547;     QyztzTpUYlMyVbvHR96664547 = QyztzTpUYlMyVbvHR64474678;     QyztzTpUYlMyVbvHR64474678 = QyztzTpUYlMyVbvHR51569698;     QyztzTpUYlMyVbvHR51569698 = QyztzTpUYlMyVbvHR2471009;     QyztzTpUYlMyVbvHR2471009 = QyztzTpUYlMyVbvHR58782897;     QyztzTpUYlMyVbvHR58782897 = QyztzTpUYlMyVbvHR55316301;     QyztzTpUYlMyVbvHR55316301 = QyztzTpUYlMyVbvHR8144978;     QyztzTpUYlMyVbvHR8144978 = QyztzTpUYlMyVbvHR58242660;     QyztzTpUYlMyVbvHR58242660 = QyztzTpUYlMyVbvHR28947586;     QyztzTpUYlMyVbvHR28947586 = QyztzTpUYlMyVbvHR2704141;     QyztzTpUYlMyVbvHR2704141 = QyztzTpUYlMyVbvHR78476035;     QyztzTpUYlMyVbvHR78476035 = QyztzTpUYlMyVbvHR41201711;     QyztzTpUYlMyVbvHR41201711 = QyztzTpUYlMyVbvHR30302000;     QyztzTpUYlMyVbvHR30302000 = QyztzTpUYlMyVbvHR56276175;     QyztzTpUYlMyVbvHR56276175 = QyztzTpUYlMyVbvHR30836386;     QyztzTpUYlMyVbvHR30836386 = QyztzTpUYlMyVbvHR1335030;     QyztzTpUYlMyVbvHR1335030 = QyztzTpUYlMyVbvHR58001932;     QyztzTpUYlMyVbvHR58001932 = QyztzTpUYlMyVbvHR36766684;     QyztzTpUYlMyVbvHR36766684 = QyztzTpUYlMyVbvHR3457479;     QyztzTpUYlMyVbvHR3457479 = QyztzTpUYlMyVbvHR21695973;     QyztzTpUYlMyVbvHR21695973 = QyztzTpUYlMyVbvHR28669885;     QyztzTpUYlMyVbvHR28669885 = QyztzTpUYlMyVbvHR88215376;     QyztzTpUYlMyVbvHR88215376 = QyztzTpUYlMyVbvHR4764452;     QyztzTpUYlMyVbvHR4764452 = QyztzTpUYlMyVbvHR72715953;     QyztzTpUYlMyVbvHR72715953 = QyztzTpUYlMyVbvHR38541822;     QyztzTpUYlMyVbvHR38541822 = QyztzTpUYlMyVbvHR10887583;     QyztzTpUYlMyVbvHR10887583 = QyztzTpUYlMyVbvHR33409620;     QyztzTpUYlMyVbvHR33409620 = QyztzTpUYlMyVbvHR16111889;     QyztzTpUYlMyVbvHR16111889 = QyztzTpUYlMyVbvHR84113782;     QyztzTpUYlMyVbvHR84113782 = QyztzTpUYlMyVbvHR14530168;     QyztzTpUYlMyVbvHR14530168 = QyztzTpUYlMyVbvHR71338501;     QyztzTpUYlMyVbvHR71338501 = QyztzTpUYlMyVbvHR29558721;     QyztzTpUYlMyVbvHR29558721 = QyztzTpUYlMyVbvHR84308413;     QyztzTpUYlMyVbvHR84308413 = QyztzTpUYlMyVbvHR53729047;     QyztzTpUYlMyVbvHR53729047 = QyztzTpUYlMyVbvHR2803470;     QyztzTpUYlMyVbvHR2803470 = QyztzTpUYlMyVbvHR28031637;     QyztzTpUYlMyVbvHR28031637 = QyztzTpUYlMyVbvHR77469566;     QyztzTpUYlMyVbvHR77469566 = QyztzTpUYlMyVbvHR68398450;     QyztzTpUYlMyVbvHR68398450 = QyztzTpUYlMyVbvHR41934425;     QyztzTpUYlMyVbvHR41934425 = QyztzTpUYlMyVbvHR96291150;     QyztzTpUYlMyVbvHR96291150 = QyztzTpUYlMyVbvHR42975903;     QyztzTpUYlMyVbvHR42975903 = QyztzTpUYlMyVbvHR3456363;     QyztzTpUYlMyVbvHR3456363 = QyztzTpUYlMyVbvHR54899232;     QyztzTpUYlMyVbvHR54899232 = QyztzTpUYlMyVbvHR9657241;     QyztzTpUYlMyVbvHR9657241 = QyztzTpUYlMyVbvHR92816538;     QyztzTpUYlMyVbvHR92816538 = QyztzTpUYlMyVbvHR71243044;     QyztzTpUYlMyVbvHR71243044 = QyztzTpUYlMyVbvHR81600511;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BKgIDJAyTSupznnQ36018341() {     double ePteECByTViXTkBKT17059480 = -67985176;    double ePteECByTViXTkBKT8850304 = -919760876;    double ePteECByTViXTkBKT90844063 = -760198185;    double ePteECByTViXTkBKT99313264 = -300875576;    double ePteECByTViXTkBKT97722886 = -142686548;    double ePteECByTViXTkBKT82722387 = -716898836;    double ePteECByTViXTkBKT1646651 = -597334222;    double ePteECByTViXTkBKT51148728 = -865020525;    double ePteECByTViXTkBKT61913209 = -306632912;    double ePteECByTViXTkBKT48885084 = -571746114;    double ePteECByTViXTkBKT65316342 = -145195240;    double ePteECByTViXTkBKT20696216 = 3270428;    double ePteECByTViXTkBKT93978837 = -848791452;    double ePteECByTViXTkBKT76483564 = -449514952;    double ePteECByTViXTkBKT88568810 = -162451519;    double ePteECByTViXTkBKT12412692 = -723447635;    double ePteECByTViXTkBKT93319941 = -711556899;    double ePteECByTViXTkBKT54825122 = -914836476;    double ePteECByTViXTkBKT73367183 = -971541394;    double ePteECByTViXTkBKT84206125 = -888470601;    double ePteECByTViXTkBKT85149057 = -31448416;    double ePteECByTViXTkBKT20547358 = -244333813;    double ePteECByTViXTkBKT14533907 = -49432430;    double ePteECByTViXTkBKT44779523 = -379972368;    double ePteECByTViXTkBKT76324963 = -761465104;    double ePteECByTViXTkBKT96819487 = -802938500;    double ePteECByTViXTkBKT98775111 = -863129287;    double ePteECByTViXTkBKT73475009 = 9057441;    double ePteECByTViXTkBKT6737797 = -708174036;    double ePteECByTViXTkBKT63224874 = -757158426;    double ePteECByTViXTkBKT74089568 = -85502907;    double ePteECByTViXTkBKT23755915 = -826792089;    double ePteECByTViXTkBKT2044079 = -268702415;    double ePteECByTViXTkBKT29216444 = -324407252;    double ePteECByTViXTkBKT58314769 = -29628564;    double ePteECByTViXTkBKT80600108 = -807850218;    double ePteECByTViXTkBKT20281276 = -592934151;    double ePteECByTViXTkBKT51932725 = -577679472;    double ePteECByTViXTkBKT81998013 = -353956728;    double ePteECByTViXTkBKT15392413 = -225988091;    double ePteECByTViXTkBKT95150629 = -691444502;    double ePteECByTViXTkBKT73211945 = -546366360;    double ePteECByTViXTkBKT1744760 = -43859766;    double ePteECByTViXTkBKT11423912 = -566435125;    double ePteECByTViXTkBKT12461214 = -74975647;    double ePteECByTViXTkBKT3380758 = 45850395;    double ePteECByTViXTkBKT72056859 = -334824467;    double ePteECByTViXTkBKT39968960 = -132096383;    double ePteECByTViXTkBKT71525467 = -945507727;    double ePteECByTViXTkBKT70069003 = -930655858;    double ePteECByTViXTkBKT67123686 = -551674230;    double ePteECByTViXTkBKT9433089 = -768352384;    double ePteECByTViXTkBKT70245631 = -994837937;    double ePteECByTViXTkBKT52088867 = -708617046;    double ePteECByTViXTkBKT50869284 = -88642936;    double ePteECByTViXTkBKT96512122 = -823651363;    double ePteECByTViXTkBKT94316397 = -770328446;    double ePteECByTViXTkBKT46064541 = -280225818;    double ePteECByTViXTkBKT22988301 = -539410472;    double ePteECByTViXTkBKT903399 = -339748048;    double ePteECByTViXTkBKT83947275 = -853769550;    double ePteECByTViXTkBKT28171642 = -506391664;    double ePteECByTViXTkBKT44410932 = -56846489;    double ePteECByTViXTkBKT98688334 = -549474487;    double ePteECByTViXTkBKT74795516 = -386243207;    double ePteECByTViXTkBKT41560427 = -318403151;    double ePteECByTViXTkBKT18652138 = -728027158;    double ePteECByTViXTkBKT64762394 = -424384201;    double ePteECByTViXTkBKT18168795 = -319886388;    double ePteECByTViXTkBKT7968702 = -354601302;    double ePteECByTViXTkBKT92131416 = -30513484;    double ePteECByTViXTkBKT41387216 = -33877427;    double ePteECByTViXTkBKT72827109 = -460879748;    double ePteECByTViXTkBKT57974770 = -645553303;    double ePteECByTViXTkBKT89055495 = -97026100;    double ePteECByTViXTkBKT11937113 = -485082057;    double ePteECByTViXTkBKT18802599 = -100474047;    double ePteECByTViXTkBKT3109995 = -482997306;    double ePteECByTViXTkBKT32318309 = -204996721;    double ePteECByTViXTkBKT72944205 = -707315500;    double ePteECByTViXTkBKT24762628 = -368114034;    double ePteECByTViXTkBKT58806151 = -631032904;    double ePteECByTViXTkBKT1949542 = -45434832;    double ePteECByTViXTkBKT36668793 = -777518178;    double ePteECByTViXTkBKT96101188 = -105484197;    double ePteECByTViXTkBKT64656480 = -317150524;    double ePteECByTViXTkBKT53510283 = -831954152;    double ePteECByTViXTkBKT49955211 = -560085370;    double ePteECByTViXTkBKT78347160 = -135764317;    double ePteECByTViXTkBKT61802647 = -205977202;    double ePteECByTViXTkBKT86283711 = 62478227;    double ePteECByTViXTkBKT74216735 = -212708333;    double ePteECByTViXTkBKT28944424 = 61730999;    double ePteECByTViXTkBKT81094614 = 85791320;    double ePteECByTViXTkBKT31445137 = -372218542;    double ePteECByTViXTkBKT66978988 = -85052839;    double ePteECByTViXTkBKT28801013 = -389519871;    double ePteECByTViXTkBKT3056425 = -494385280;    double ePteECByTViXTkBKT36628396 = -80191919;    double ePteECByTViXTkBKT70900786 = -67985176;     ePteECByTViXTkBKT17059480 = ePteECByTViXTkBKT8850304;     ePteECByTViXTkBKT8850304 = ePteECByTViXTkBKT90844063;     ePteECByTViXTkBKT90844063 = ePteECByTViXTkBKT99313264;     ePteECByTViXTkBKT99313264 = ePteECByTViXTkBKT97722886;     ePteECByTViXTkBKT97722886 = ePteECByTViXTkBKT82722387;     ePteECByTViXTkBKT82722387 = ePteECByTViXTkBKT1646651;     ePteECByTViXTkBKT1646651 = ePteECByTViXTkBKT51148728;     ePteECByTViXTkBKT51148728 = ePteECByTViXTkBKT61913209;     ePteECByTViXTkBKT61913209 = ePteECByTViXTkBKT48885084;     ePteECByTViXTkBKT48885084 = ePteECByTViXTkBKT65316342;     ePteECByTViXTkBKT65316342 = ePteECByTViXTkBKT20696216;     ePteECByTViXTkBKT20696216 = ePteECByTViXTkBKT93978837;     ePteECByTViXTkBKT93978837 = ePteECByTViXTkBKT76483564;     ePteECByTViXTkBKT76483564 = ePteECByTViXTkBKT88568810;     ePteECByTViXTkBKT88568810 = ePteECByTViXTkBKT12412692;     ePteECByTViXTkBKT12412692 = ePteECByTViXTkBKT93319941;     ePteECByTViXTkBKT93319941 = ePteECByTViXTkBKT54825122;     ePteECByTViXTkBKT54825122 = ePteECByTViXTkBKT73367183;     ePteECByTViXTkBKT73367183 = ePteECByTViXTkBKT84206125;     ePteECByTViXTkBKT84206125 = ePteECByTViXTkBKT85149057;     ePteECByTViXTkBKT85149057 = ePteECByTViXTkBKT20547358;     ePteECByTViXTkBKT20547358 = ePteECByTViXTkBKT14533907;     ePteECByTViXTkBKT14533907 = ePteECByTViXTkBKT44779523;     ePteECByTViXTkBKT44779523 = ePteECByTViXTkBKT76324963;     ePteECByTViXTkBKT76324963 = ePteECByTViXTkBKT96819487;     ePteECByTViXTkBKT96819487 = ePteECByTViXTkBKT98775111;     ePteECByTViXTkBKT98775111 = ePteECByTViXTkBKT73475009;     ePteECByTViXTkBKT73475009 = ePteECByTViXTkBKT6737797;     ePteECByTViXTkBKT6737797 = ePteECByTViXTkBKT63224874;     ePteECByTViXTkBKT63224874 = ePteECByTViXTkBKT74089568;     ePteECByTViXTkBKT74089568 = ePteECByTViXTkBKT23755915;     ePteECByTViXTkBKT23755915 = ePteECByTViXTkBKT2044079;     ePteECByTViXTkBKT2044079 = ePteECByTViXTkBKT29216444;     ePteECByTViXTkBKT29216444 = ePteECByTViXTkBKT58314769;     ePteECByTViXTkBKT58314769 = ePteECByTViXTkBKT80600108;     ePteECByTViXTkBKT80600108 = ePteECByTViXTkBKT20281276;     ePteECByTViXTkBKT20281276 = ePteECByTViXTkBKT51932725;     ePteECByTViXTkBKT51932725 = ePteECByTViXTkBKT81998013;     ePteECByTViXTkBKT81998013 = ePteECByTViXTkBKT15392413;     ePteECByTViXTkBKT15392413 = ePteECByTViXTkBKT95150629;     ePteECByTViXTkBKT95150629 = ePteECByTViXTkBKT73211945;     ePteECByTViXTkBKT73211945 = ePteECByTViXTkBKT1744760;     ePteECByTViXTkBKT1744760 = ePteECByTViXTkBKT11423912;     ePteECByTViXTkBKT11423912 = ePteECByTViXTkBKT12461214;     ePteECByTViXTkBKT12461214 = ePteECByTViXTkBKT3380758;     ePteECByTViXTkBKT3380758 = ePteECByTViXTkBKT72056859;     ePteECByTViXTkBKT72056859 = ePteECByTViXTkBKT39968960;     ePteECByTViXTkBKT39968960 = ePteECByTViXTkBKT71525467;     ePteECByTViXTkBKT71525467 = ePteECByTViXTkBKT70069003;     ePteECByTViXTkBKT70069003 = ePteECByTViXTkBKT67123686;     ePteECByTViXTkBKT67123686 = ePteECByTViXTkBKT9433089;     ePteECByTViXTkBKT9433089 = ePteECByTViXTkBKT70245631;     ePteECByTViXTkBKT70245631 = ePteECByTViXTkBKT52088867;     ePteECByTViXTkBKT52088867 = ePteECByTViXTkBKT50869284;     ePteECByTViXTkBKT50869284 = ePteECByTViXTkBKT96512122;     ePteECByTViXTkBKT96512122 = ePteECByTViXTkBKT94316397;     ePteECByTViXTkBKT94316397 = ePteECByTViXTkBKT46064541;     ePteECByTViXTkBKT46064541 = ePteECByTViXTkBKT22988301;     ePteECByTViXTkBKT22988301 = ePteECByTViXTkBKT903399;     ePteECByTViXTkBKT903399 = ePteECByTViXTkBKT83947275;     ePteECByTViXTkBKT83947275 = ePteECByTViXTkBKT28171642;     ePteECByTViXTkBKT28171642 = ePteECByTViXTkBKT44410932;     ePteECByTViXTkBKT44410932 = ePteECByTViXTkBKT98688334;     ePteECByTViXTkBKT98688334 = ePteECByTViXTkBKT74795516;     ePteECByTViXTkBKT74795516 = ePteECByTViXTkBKT41560427;     ePteECByTViXTkBKT41560427 = ePteECByTViXTkBKT18652138;     ePteECByTViXTkBKT18652138 = ePteECByTViXTkBKT64762394;     ePteECByTViXTkBKT64762394 = ePteECByTViXTkBKT18168795;     ePteECByTViXTkBKT18168795 = ePteECByTViXTkBKT7968702;     ePteECByTViXTkBKT7968702 = ePteECByTViXTkBKT92131416;     ePteECByTViXTkBKT92131416 = ePteECByTViXTkBKT41387216;     ePteECByTViXTkBKT41387216 = ePteECByTViXTkBKT72827109;     ePteECByTViXTkBKT72827109 = ePteECByTViXTkBKT57974770;     ePteECByTViXTkBKT57974770 = ePteECByTViXTkBKT89055495;     ePteECByTViXTkBKT89055495 = ePteECByTViXTkBKT11937113;     ePteECByTViXTkBKT11937113 = ePteECByTViXTkBKT18802599;     ePteECByTViXTkBKT18802599 = ePteECByTViXTkBKT3109995;     ePteECByTViXTkBKT3109995 = ePteECByTViXTkBKT32318309;     ePteECByTViXTkBKT32318309 = ePteECByTViXTkBKT72944205;     ePteECByTViXTkBKT72944205 = ePteECByTViXTkBKT24762628;     ePteECByTViXTkBKT24762628 = ePteECByTViXTkBKT58806151;     ePteECByTViXTkBKT58806151 = ePteECByTViXTkBKT1949542;     ePteECByTViXTkBKT1949542 = ePteECByTViXTkBKT36668793;     ePteECByTViXTkBKT36668793 = ePteECByTViXTkBKT96101188;     ePteECByTViXTkBKT96101188 = ePteECByTViXTkBKT64656480;     ePteECByTViXTkBKT64656480 = ePteECByTViXTkBKT53510283;     ePteECByTViXTkBKT53510283 = ePteECByTViXTkBKT49955211;     ePteECByTViXTkBKT49955211 = ePteECByTViXTkBKT78347160;     ePteECByTViXTkBKT78347160 = ePteECByTViXTkBKT61802647;     ePteECByTViXTkBKT61802647 = ePteECByTViXTkBKT86283711;     ePteECByTViXTkBKT86283711 = ePteECByTViXTkBKT74216735;     ePteECByTViXTkBKT74216735 = ePteECByTViXTkBKT28944424;     ePteECByTViXTkBKT28944424 = ePteECByTViXTkBKT81094614;     ePteECByTViXTkBKT81094614 = ePteECByTViXTkBKT31445137;     ePteECByTViXTkBKT31445137 = ePteECByTViXTkBKT66978988;     ePteECByTViXTkBKT66978988 = ePteECByTViXTkBKT28801013;     ePteECByTViXTkBKT28801013 = ePteECByTViXTkBKT3056425;     ePteECByTViXTkBKT3056425 = ePteECByTViXTkBKT36628396;     ePteECByTViXTkBKT36628396 = ePteECByTViXTkBKT70900786;     ePteECByTViXTkBKT70900786 = ePteECByTViXTkBKT17059480;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cBsoqSlcspICBGCs51067409() {     double bKyTieRnuSDIJviEM23176587 = -179887065;    double bKyTieRnuSDIJviEM52223583 = -629360656;    double bKyTieRnuSDIJviEM6886247 = -911031106;    double bKyTieRnuSDIJviEM26388045 = -491624206;    double bKyTieRnuSDIJviEM66618375 = -663336419;    double bKyTieRnuSDIJviEM74139589 = -981395071;    double bKyTieRnuSDIJviEM62135616 = -736299199;    double bKyTieRnuSDIJviEM908620 = -51894438;    double bKyTieRnuSDIJviEM26555705 = -74665578;    double bKyTieRnuSDIJviEM90114743 = -900721591;    double bKyTieRnuSDIJviEM25426633 = -22632826;    double bKyTieRnuSDIJviEM52519258 = -441135774;    double bKyTieRnuSDIJviEM80187956 = -188682541;    double bKyTieRnuSDIJviEM59627241 = -981753519;    double bKyTieRnuSDIJviEM84311236 = -852913686;    double bKyTieRnuSDIJviEM53918086 = -696447178;    double bKyTieRnuSDIJviEM87155944 = -228102011;    double bKyTieRnuSDIJviEM51878275 = -580359118;    double bKyTieRnuSDIJviEM35197025 = -277310490;    double bKyTieRnuSDIJviEM64756290 = -145373521;    double bKyTieRnuSDIJviEM73702250 = -986935872;    double bKyTieRnuSDIJviEM7068382 = -152263529;    double bKyTieRnuSDIJviEM50124698 = -895949917;    double bKyTieRnuSDIJviEM28771897 = -602277356;    double bKyTieRnuSDIJviEM20024614 = -34661137;    double bKyTieRnuSDIJviEM21541163 = -681593824;    double bKyTieRnuSDIJviEM97234889 = -763791490;    double bKyTieRnuSDIJviEM86582134 = -56871512;    double bKyTieRnuSDIJviEM72870052 = -930556741;    double bKyTieRnuSDIJviEM31840511 = -574629293;    double bKyTieRnuSDIJviEM66332118 = 1176155;    double bKyTieRnuSDIJviEM21366537 = -536238905;    double bKyTieRnuSDIJviEM11755027 = -161315785;    double bKyTieRnuSDIJviEM24047008 = -75949017;    double bKyTieRnuSDIJviEM19789421 = -668361670;    double bKyTieRnuSDIJviEM49326021 = 78123674;    double bKyTieRnuSDIJviEM1543210 = -546570020;    double bKyTieRnuSDIJviEM86022937 = -349185482;    double bKyTieRnuSDIJviEM55608267 = -347880224;    double bKyTieRnuSDIJviEM2894351 = -699427314;    double bKyTieRnuSDIJviEM819854 = -756718396;    double bKyTieRnuSDIJviEM5362402 = -816508638;    double bKyTieRnuSDIJviEM79811737 = -124664350;    double bKyTieRnuSDIJviEM98831652 = -585778063;    double bKyTieRnuSDIJviEM24413679 = -836873802;    double bKyTieRnuSDIJviEM15319953 = 60634279;    double bKyTieRnuSDIJviEM89835434 = -453600004;    double bKyTieRnuSDIJviEM47925901 = -141991915;    double bKyTieRnuSDIJviEM81201196 = 68755285;    double bKyTieRnuSDIJviEM82980655 = 82333334;    double bKyTieRnuSDIJviEM2824365 = -393967901;    double bKyTieRnuSDIJviEM20242808 = -569078704;    double bKyTieRnuSDIJviEM18494089 = -786627525;    double bKyTieRnuSDIJviEM48057747 = 70912809;    double bKyTieRnuSDIJviEM77365158 = -789174718;    double bKyTieRnuSDIJviEM16108205 = 72376464;    double bKyTieRnuSDIJviEM2098885 = -733410740;    double bKyTieRnuSDIJviEM78114350 = -208753751;    double bKyTieRnuSDIJviEM6363432 = -356963069;    double bKyTieRnuSDIJviEM45077212 = -981742596;    double bKyTieRnuSDIJviEM76904699 = -117603582;    double bKyTieRnuSDIJviEM75553482 = -579427688;    double bKyTieRnuSDIJviEM28038567 = -121337698;    double bKyTieRnuSDIJviEM94715193 = -500036285;    double bKyTieRnuSDIJviEM23782626 = -801897746;    double bKyTieRnuSDIJviEM4060096 = -486393922;    double bKyTieRnuSDIJviEM40764231 = -179819989;    double bKyTieRnuSDIJviEM56140948 = -12733524;    double bKyTieRnuSDIJviEM39837820 = -213391850;    double bKyTieRnuSDIJviEM34985215 = -831037361;    double bKyTieRnuSDIJviEM52374876 = -49877158;    double bKyTieRnuSDIJviEM1133007 = -878916530;    double bKyTieRnuSDIJviEM96270008 = -132478894;    double bKyTieRnuSDIJviEM32302674 = -577883177;    double bKyTieRnuSDIJviEM63936436 = -388655125;    double bKyTieRnuSDIJviEM68339848 = -70427234;    double bKyTieRnuSDIJviEM27256644 = 72400820;    double bKyTieRnuSDIJviEM51293046 = -210171854;    double bKyTieRnuSDIJviEM4358218 = -765403554;    double bKyTieRnuSDIJviEM4704661 = 4704583;    double bKyTieRnuSDIJviEM31705729 = -127993820;    double bKyTieRnuSDIJviEM49308989 = -521799575;    double bKyTieRnuSDIJviEM5380938 = -25626798;    double bKyTieRnuSDIJviEM89889396 = -912890075;    double bKyTieRnuSDIJviEM29016147 = -80661393;    double bKyTieRnuSDIJviEM46089310 = -429745142;    double bKyTieRnuSDIJviEM2872448 = -749611380;    double bKyTieRnuSDIJviEM63697279 = -132228595;    double bKyTieRnuSDIJviEM46681850 = -286774300;    double bKyTieRnuSDIJviEM3681216 = -640738135;    double bKyTieRnuSDIJviEM47227137 = -188465587;    double bKyTieRnuSDIJviEM23428860 = -237816270;    double bKyTieRnuSDIJviEM79659506 = -992222413;    double bKyTieRnuSDIJviEM10531055 = -366137629;    double bKyTieRnuSDIJviEM25989651 = -481823732;    double bKyTieRnuSDIJviEM25266372 = -77290709;    double bKyTieRnuSDIJviEM77323834 = -595170941;    double bKyTieRnuSDIJviEM85096544 = -624628065;    double bKyTieRnuSDIJviEM75049026 = -783880318;    double bKyTieRnuSDIJviEM20353583 = -179887065;     bKyTieRnuSDIJviEM23176587 = bKyTieRnuSDIJviEM52223583;     bKyTieRnuSDIJviEM52223583 = bKyTieRnuSDIJviEM6886247;     bKyTieRnuSDIJviEM6886247 = bKyTieRnuSDIJviEM26388045;     bKyTieRnuSDIJviEM26388045 = bKyTieRnuSDIJviEM66618375;     bKyTieRnuSDIJviEM66618375 = bKyTieRnuSDIJviEM74139589;     bKyTieRnuSDIJviEM74139589 = bKyTieRnuSDIJviEM62135616;     bKyTieRnuSDIJviEM62135616 = bKyTieRnuSDIJviEM908620;     bKyTieRnuSDIJviEM908620 = bKyTieRnuSDIJviEM26555705;     bKyTieRnuSDIJviEM26555705 = bKyTieRnuSDIJviEM90114743;     bKyTieRnuSDIJviEM90114743 = bKyTieRnuSDIJviEM25426633;     bKyTieRnuSDIJviEM25426633 = bKyTieRnuSDIJviEM52519258;     bKyTieRnuSDIJviEM52519258 = bKyTieRnuSDIJviEM80187956;     bKyTieRnuSDIJviEM80187956 = bKyTieRnuSDIJviEM59627241;     bKyTieRnuSDIJviEM59627241 = bKyTieRnuSDIJviEM84311236;     bKyTieRnuSDIJviEM84311236 = bKyTieRnuSDIJviEM53918086;     bKyTieRnuSDIJviEM53918086 = bKyTieRnuSDIJviEM87155944;     bKyTieRnuSDIJviEM87155944 = bKyTieRnuSDIJviEM51878275;     bKyTieRnuSDIJviEM51878275 = bKyTieRnuSDIJviEM35197025;     bKyTieRnuSDIJviEM35197025 = bKyTieRnuSDIJviEM64756290;     bKyTieRnuSDIJviEM64756290 = bKyTieRnuSDIJviEM73702250;     bKyTieRnuSDIJviEM73702250 = bKyTieRnuSDIJviEM7068382;     bKyTieRnuSDIJviEM7068382 = bKyTieRnuSDIJviEM50124698;     bKyTieRnuSDIJviEM50124698 = bKyTieRnuSDIJviEM28771897;     bKyTieRnuSDIJviEM28771897 = bKyTieRnuSDIJviEM20024614;     bKyTieRnuSDIJviEM20024614 = bKyTieRnuSDIJviEM21541163;     bKyTieRnuSDIJviEM21541163 = bKyTieRnuSDIJviEM97234889;     bKyTieRnuSDIJviEM97234889 = bKyTieRnuSDIJviEM86582134;     bKyTieRnuSDIJviEM86582134 = bKyTieRnuSDIJviEM72870052;     bKyTieRnuSDIJviEM72870052 = bKyTieRnuSDIJviEM31840511;     bKyTieRnuSDIJviEM31840511 = bKyTieRnuSDIJviEM66332118;     bKyTieRnuSDIJviEM66332118 = bKyTieRnuSDIJviEM21366537;     bKyTieRnuSDIJviEM21366537 = bKyTieRnuSDIJviEM11755027;     bKyTieRnuSDIJviEM11755027 = bKyTieRnuSDIJviEM24047008;     bKyTieRnuSDIJviEM24047008 = bKyTieRnuSDIJviEM19789421;     bKyTieRnuSDIJviEM19789421 = bKyTieRnuSDIJviEM49326021;     bKyTieRnuSDIJviEM49326021 = bKyTieRnuSDIJviEM1543210;     bKyTieRnuSDIJviEM1543210 = bKyTieRnuSDIJviEM86022937;     bKyTieRnuSDIJviEM86022937 = bKyTieRnuSDIJviEM55608267;     bKyTieRnuSDIJviEM55608267 = bKyTieRnuSDIJviEM2894351;     bKyTieRnuSDIJviEM2894351 = bKyTieRnuSDIJviEM819854;     bKyTieRnuSDIJviEM819854 = bKyTieRnuSDIJviEM5362402;     bKyTieRnuSDIJviEM5362402 = bKyTieRnuSDIJviEM79811737;     bKyTieRnuSDIJviEM79811737 = bKyTieRnuSDIJviEM98831652;     bKyTieRnuSDIJviEM98831652 = bKyTieRnuSDIJviEM24413679;     bKyTieRnuSDIJviEM24413679 = bKyTieRnuSDIJviEM15319953;     bKyTieRnuSDIJviEM15319953 = bKyTieRnuSDIJviEM89835434;     bKyTieRnuSDIJviEM89835434 = bKyTieRnuSDIJviEM47925901;     bKyTieRnuSDIJviEM47925901 = bKyTieRnuSDIJviEM81201196;     bKyTieRnuSDIJviEM81201196 = bKyTieRnuSDIJviEM82980655;     bKyTieRnuSDIJviEM82980655 = bKyTieRnuSDIJviEM2824365;     bKyTieRnuSDIJviEM2824365 = bKyTieRnuSDIJviEM20242808;     bKyTieRnuSDIJviEM20242808 = bKyTieRnuSDIJviEM18494089;     bKyTieRnuSDIJviEM18494089 = bKyTieRnuSDIJviEM48057747;     bKyTieRnuSDIJviEM48057747 = bKyTieRnuSDIJviEM77365158;     bKyTieRnuSDIJviEM77365158 = bKyTieRnuSDIJviEM16108205;     bKyTieRnuSDIJviEM16108205 = bKyTieRnuSDIJviEM2098885;     bKyTieRnuSDIJviEM2098885 = bKyTieRnuSDIJviEM78114350;     bKyTieRnuSDIJviEM78114350 = bKyTieRnuSDIJviEM6363432;     bKyTieRnuSDIJviEM6363432 = bKyTieRnuSDIJviEM45077212;     bKyTieRnuSDIJviEM45077212 = bKyTieRnuSDIJviEM76904699;     bKyTieRnuSDIJviEM76904699 = bKyTieRnuSDIJviEM75553482;     bKyTieRnuSDIJviEM75553482 = bKyTieRnuSDIJviEM28038567;     bKyTieRnuSDIJviEM28038567 = bKyTieRnuSDIJviEM94715193;     bKyTieRnuSDIJviEM94715193 = bKyTieRnuSDIJviEM23782626;     bKyTieRnuSDIJviEM23782626 = bKyTieRnuSDIJviEM4060096;     bKyTieRnuSDIJviEM4060096 = bKyTieRnuSDIJviEM40764231;     bKyTieRnuSDIJviEM40764231 = bKyTieRnuSDIJviEM56140948;     bKyTieRnuSDIJviEM56140948 = bKyTieRnuSDIJviEM39837820;     bKyTieRnuSDIJviEM39837820 = bKyTieRnuSDIJviEM34985215;     bKyTieRnuSDIJviEM34985215 = bKyTieRnuSDIJviEM52374876;     bKyTieRnuSDIJviEM52374876 = bKyTieRnuSDIJviEM1133007;     bKyTieRnuSDIJviEM1133007 = bKyTieRnuSDIJviEM96270008;     bKyTieRnuSDIJviEM96270008 = bKyTieRnuSDIJviEM32302674;     bKyTieRnuSDIJviEM32302674 = bKyTieRnuSDIJviEM63936436;     bKyTieRnuSDIJviEM63936436 = bKyTieRnuSDIJviEM68339848;     bKyTieRnuSDIJviEM68339848 = bKyTieRnuSDIJviEM27256644;     bKyTieRnuSDIJviEM27256644 = bKyTieRnuSDIJviEM51293046;     bKyTieRnuSDIJviEM51293046 = bKyTieRnuSDIJviEM4358218;     bKyTieRnuSDIJviEM4358218 = bKyTieRnuSDIJviEM4704661;     bKyTieRnuSDIJviEM4704661 = bKyTieRnuSDIJviEM31705729;     bKyTieRnuSDIJviEM31705729 = bKyTieRnuSDIJviEM49308989;     bKyTieRnuSDIJviEM49308989 = bKyTieRnuSDIJviEM5380938;     bKyTieRnuSDIJviEM5380938 = bKyTieRnuSDIJviEM89889396;     bKyTieRnuSDIJviEM89889396 = bKyTieRnuSDIJviEM29016147;     bKyTieRnuSDIJviEM29016147 = bKyTieRnuSDIJviEM46089310;     bKyTieRnuSDIJviEM46089310 = bKyTieRnuSDIJviEM2872448;     bKyTieRnuSDIJviEM2872448 = bKyTieRnuSDIJviEM63697279;     bKyTieRnuSDIJviEM63697279 = bKyTieRnuSDIJviEM46681850;     bKyTieRnuSDIJviEM46681850 = bKyTieRnuSDIJviEM3681216;     bKyTieRnuSDIJviEM3681216 = bKyTieRnuSDIJviEM47227137;     bKyTieRnuSDIJviEM47227137 = bKyTieRnuSDIJviEM23428860;     bKyTieRnuSDIJviEM23428860 = bKyTieRnuSDIJviEM79659506;     bKyTieRnuSDIJviEM79659506 = bKyTieRnuSDIJviEM10531055;     bKyTieRnuSDIJviEM10531055 = bKyTieRnuSDIJviEM25989651;     bKyTieRnuSDIJviEM25989651 = bKyTieRnuSDIJviEM25266372;     bKyTieRnuSDIJviEM25266372 = bKyTieRnuSDIJviEM77323834;     bKyTieRnuSDIJviEM77323834 = bKyTieRnuSDIJviEM85096544;     bKyTieRnuSDIJviEM85096544 = bKyTieRnuSDIJviEM75049026;     bKyTieRnuSDIJviEM75049026 = bKyTieRnuSDIJviEM20353583;     bKyTieRnuSDIJviEM20353583 = bKyTieRnuSDIJviEM23176587;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void NqVuxxwPpopzkAdZ87712383() {     double vTpmslupExFXIZxHE45563704 = -914291028;    double vTpmslupExFXIZxHE36571083 = -577903711;    double vTpmslupExFXIZxHE35655526 = -50684950;    double vTpmslupExFXIZxHE41394734 = -498031738;    double vTpmslupExFXIZxHE64792629 = -154393421;    double vTpmslupExFXIZxHE89022706 = -842003449;    double vTpmslupExFXIZxHE54291355 = -558286505;    double vTpmslupExFXIZxHE76542478 = -522491082;    double vTpmslupExFXIZxHE23718836 = 41138383;    double vTpmslupExFXIZxHE81878328 = -643462924;    double vTpmslupExFXIZxHE62968492 = -19257227;    double vTpmslupExFXIZxHE8422207 = -691881068;    double vTpmslupExFXIZxHE47196137 = -807399861;    double vTpmslupExFXIZxHE2241993 = -409462276;    double vTpmslupExFXIZxHE21046514 = -480391399;    double vTpmslupExFXIZxHE98173998 = 21885280;    double vTpmslupExFXIZxHE36979296 = -505381831;    double vTpmslupExFXIZxHE57854691 = 67898191;    double vTpmslupExFXIZxHE76478655 = -412412616;    double vTpmslupExFXIZxHE43272573 = -398007974;    double vTpmslupExFXIZxHE74585357 = -978423180;    double vTpmslupExFXIZxHE70883599 = -657029843;    double vTpmslupExFXIZxHE53418021 = 27969491;    double vTpmslupExFXIZxHE55271683 = -66270213;    double vTpmslupExFXIZxHE55564860 = -756220262;    double vTpmslupExFXIZxHE9844158 = -25488047;    double vTpmslupExFXIZxHE44324310 = -521377983;    double vTpmslupExFXIZxHE92493584 = -945325966;    double vTpmslupExFXIZxHE79762612 = -174813860;    double vTpmslupExFXIZxHE40997691 = -494925160;    double vTpmslupExFXIZxHE10622923 = -80139997;    double vTpmslupExFXIZxHE78907727 = -88310259;    double vTpmslupExFXIZxHE92793697 = -690659280;    double vTpmslupExFXIZxHE83435695 = -400075867;    double vTpmslupExFXIZxHE96018785 = -732118092;    double vTpmslupExFXIZxHE91018890 = -237271702;    double vTpmslupExFXIZxHE94982922 = -279517972;    double vTpmslupExFXIZxHE39649039 = -216810954;    double vTpmslupExFXIZxHE3916398 = -748926256;    double vTpmslupExFXIZxHE23080065 = -835651343;    double vTpmslupExFXIZxHE29577225 = -159896062;    double vTpmslupExFXIZxHE93870792 = -574417329;    double vTpmslupExFXIZxHE85361929 = -12194934;    double vTpmslupExFXIZxHE98275771 = -583141712;    double vTpmslupExFXIZxHE92396945 = -646017279;    double vTpmslupExFXIZxHE23420235 = -121594817;    double vTpmslupExFXIZxHE2132832 = 67989743;    double vTpmslupExFXIZxHE92524575 = -998526906;    double vTpmslupExFXIZxHE75661101 = -980961413;    double vTpmslupExFXIZxHE80064401 = -59686925;    double vTpmslupExFXIZxHE11693581 = -749044178;    double vTpmslupExFXIZxHE94094065 = -999897986;    double vTpmslupExFXIZxHE96808492 = -458538755;    double vTpmslupExFXIZxHE8344293 = -204768000;    double vTpmslupExFXIZxHE13127026 = -929878833;    double vTpmslupExFXIZxHE74680104 = -157261186;    double vTpmslupExFXIZxHE83153062 = -505873202;    double vTpmslupExFXIZxHE80383842 = -984414738;    double vTpmslupExFXIZxHE85829873 = -741811476;    double vTpmslupExFXIZxHE54948472 = -28905374;    double vTpmslupExFXIZxHE44698396 = -220625467;    double vTpmslupExFXIZxHE61797771 = -612960540;    double vTpmslupExFXIZxHE96779865 = -247677223;    double vTpmslupExFXIZxHE82721145 = -463936457;    double vTpmslupExFXIZxHE71255405 = -463322927;    double vTpmslupExFXIZxHE84060764 = -930946968;    double vTpmslupExFXIZxHE15628510 = 98778212;    double vTpmslupExFXIZxHE63760441 = -307323994;    double vTpmslupExFXIZxHE6223208 = -677344185;    double vTpmslupExFXIZxHE30027623 = -143119697;    double vTpmslupExFXIZxHE3191077 = -698596749;    double vTpmslupExFXIZxHE97330257 = -188570878;    double vTpmslupExFXIZxHE53938293 = -183175553;    double vTpmslupExFXIZxHE53398591 = -576761273;    double vTpmslupExFXIZxHE13695349 = -138111913;    double vTpmslupExFXIZxHE80714564 = -304005852;    double vTpmslupExFXIZxHE85521670 = -544834909;    double vTpmslupExFXIZxHE55142250 = -388888797;    double vTpmslupExFXIZxHE62874738 = -420252934;    double vTpmslupExFXIZxHE32144626 = -534625446;    double vTpmslupExFXIZxHE7711327 = 6522209;    double vTpmslupExFXIZxHE51799734 = -522851077;    double vTpmslupExFXIZxHE16832483 = -964364553;    double vTpmslupExFXIZxHE99698211 = -15126935;    double vTpmslupExFXIZxHE29304110 = -745880982;    double vTpmslupExFXIZxHE16528858 = -80242012;    double vTpmslupExFXIZxHE82099235 = -629771505;    double vTpmslupExFXIZxHE84449404 = -385891281;    double vTpmslupExFXIZxHE70308669 = -470197035;    double vTpmslupExFXIZxHE21338682 = -474856907;    double vTpmslupExFXIZxHE7865829 = -731398500;    double vTpmslupExFXIZxHE14599081 = -295103234;    double vTpmslupExFXIZxHE53819165 = -474999478;    double vTpmslupExFXIZxHE48967926 = -620020883;    double vTpmslupExFXIZxHE78381668 = -515025876;    double vTpmslupExFXIZxHE67779454 = -546935523;    double vTpmslupExFXIZxHE97090927 = -226740107;    double vTpmslupExFXIZxHE2640785 = -548258477;    double vTpmslupExFXIZxHE27020366 = -19818786;    double vTpmslupExFXIZxHE8336181 = -914291028;     vTpmslupExFXIZxHE45563704 = vTpmslupExFXIZxHE36571083;     vTpmslupExFXIZxHE36571083 = vTpmslupExFXIZxHE35655526;     vTpmslupExFXIZxHE35655526 = vTpmslupExFXIZxHE41394734;     vTpmslupExFXIZxHE41394734 = vTpmslupExFXIZxHE64792629;     vTpmslupExFXIZxHE64792629 = vTpmslupExFXIZxHE89022706;     vTpmslupExFXIZxHE89022706 = vTpmslupExFXIZxHE54291355;     vTpmslupExFXIZxHE54291355 = vTpmslupExFXIZxHE76542478;     vTpmslupExFXIZxHE76542478 = vTpmslupExFXIZxHE23718836;     vTpmslupExFXIZxHE23718836 = vTpmslupExFXIZxHE81878328;     vTpmslupExFXIZxHE81878328 = vTpmslupExFXIZxHE62968492;     vTpmslupExFXIZxHE62968492 = vTpmslupExFXIZxHE8422207;     vTpmslupExFXIZxHE8422207 = vTpmslupExFXIZxHE47196137;     vTpmslupExFXIZxHE47196137 = vTpmslupExFXIZxHE2241993;     vTpmslupExFXIZxHE2241993 = vTpmslupExFXIZxHE21046514;     vTpmslupExFXIZxHE21046514 = vTpmslupExFXIZxHE98173998;     vTpmslupExFXIZxHE98173998 = vTpmslupExFXIZxHE36979296;     vTpmslupExFXIZxHE36979296 = vTpmslupExFXIZxHE57854691;     vTpmslupExFXIZxHE57854691 = vTpmslupExFXIZxHE76478655;     vTpmslupExFXIZxHE76478655 = vTpmslupExFXIZxHE43272573;     vTpmslupExFXIZxHE43272573 = vTpmslupExFXIZxHE74585357;     vTpmslupExFXIZxHE74585357 = vTpmslupExFXIZxHE70883599;     vTpmslupExFXIZxHE70883599 = vTpmslupExFXIZxHE53418021;     vTpmslupExFXIZxHE53418021 = vTpmslupExFXIZxHE55271683;     vTpmslupExFXIZxHE55271683 = vTpmslupExFXIZxHE55564860;     vTpmslupExFXIZxHE55564860 = vTpmslupExFXIZxHE9844158;     vTpmslupExFXIZxHE9844158 = vTpmslupExFXIZxHE44324310;     vTpmslupExFXIZxHE44324310 = vTpmslupExFXIZxHE92493584;     vTpmslupExFXIZxHE92493584 = vTpmslupExFXIZxHE79762612;     vTpmslupExFXIZxHE79762612 = vTpmslupExFXIZxHE40997691;     vTpmslupExFXIZxHE40997691 = vTpmslupExFXIZxHE10622923;     vTpmslupExFXIZxHE10622923 = vTpmslupExFXIZxHE78907727;     vTpmslupExFXIZxHE78907727 = vTpmslupExFXIZxHE92793697;     vTpmslupExFXIZxHE92793697 = vTpmslupExFXIZxHE83435695;     vTpmslupExFXIZxHE83435695 = vTpmslupExFXIZxHE96018785;     vTpmslupExFXIZxHE96018785 = vTpmslupExFXIZxHE91018890;     vTpmslupExFXIZxHE91018890 = vTpmslupExFXIZxHE94982922;     vTpmslupExFXIZxHE94982922 = vTpmslupExFXIZxHE39649039;     vTpmslupExFXIZxHE39649039 = vTpmslupExFXIZxHE3916398;     vTpmslupExFXIZxHE3916398 = vTpmslupExFXIZxHE23080065;     vTpmslupExFXIZxHE23080065 = vTpmslupExFXIZxHE29577225;     vTpmslupExFXIZxHE29577225 = vTpmslupExFXIZxHE93870792;     vTpmslupExFXIZxHE93870792 = vTpmslupExFXIZxHE85361929;     vTpmslupExFXIZxHE85361929 = vTpmslupExFXIZxHE98275771;     vTpmslupExFXIZxHE98275771 = vTpmslupExFXIZxHE92396945;     vTpmslupExFXIZxHE92396945 = vTpmslupExFXIZxHE23420235;     vTpmslupExFXIZxHE23420235 = vTpmslupExFXIZxHE2132832;     vTpmslupExFXIZxHE2132832 = vTpmslupExFXIZxHE92524575;     vTpmslupExFXIZxHE92524575 = vTpmslupExFXIZxHE75661101;     vTpmslupExFXIZxHE75661101 = vTpmslupExFXIZxHE80064401;     vTpmslupExFXIZxHE80064401 = vTpmslupExFXIZxHE11693581;     vTpmslupExFXIZxHE11693581 = vTpmslupExFXIZxHE94094065;     vTpmslupExFXIZxHE94094065 = vTpmslupExFXIZxHE96808492;     vTpmslupExFXIZxHE96808492 = vTpmslupExFXIZxHE8344293;     vTpmslupExFXIZxHE8344293 = vTpmslupExFXIZxHE13127026;     vTpmslupExFXIZxHE13127026 = vTpmslupExFXIZxHE74680104;     vTpmslupExFXIZxHE74680104 = vTpmslupExFXIZxHE83153062;     vTpmslupExFXIZxHE83153062 = vTpmslupExFXIZxHE80383842;     vTpmslupExFXIZxHE80383842 = vTpmslupExFXIZxHE85829873;     vTpmslupExFXIZxHE85829873 = vTpmslupExFXIZxHE54948472;     vTpmslupExFXIZxHE54948472 = vTpmslupExFXIZxHE44698396;     vTpmslupExFXIZxHE44698396 = vTpmslupExFXIZxHE61797771;     vTpmslupExFXIZxHE61797771 = vTpmslupExFXIZxHE96779865;     vTpmslupExFXIZxHE96779865 = vTpmslupExFXIZxHE82721145;     vTpmslupExFXIZxHE82721145 = vTpmslupExFXIZxHE71255405;     vTpmslupExFXIZxHE71255405 = vTpmslupExFXIZxHE84060764;     vTpmslupExFXIZxHE84060764 = vTpmslupExFXIZxHE15628510;     vTpmslupExFXIZxHE15628510 = vTpmslupExFXIZxHE63760441;     vTpmslupExFXIZxHE63760441 = vTpmslupExFXIZxHE6223208;     vTpmslupExFXIZxHE6223208 = vTpmslupExFXIZxHE30027623;     vTpmslupExFXIZxHE30027623 = vTpmslupExFXIZxHE3191077;     vTpmslupExFXIZxHE3191077 = vTpmslupExFXIZxHE97330257;     vTpmslupExFXIZxHE97330257 = vTpmslupExFXIZxHE53938293;     vTpmslupExFXIZxHE53938293 = vTpmslupExFXIZxHE53398591;     vTpmslupExFXIZxHE53398591 = vTpmslupExFXIZxHE13695349;     vTpmslupExFXIZxHE13695349 = vTpmslupExFXIZxHE80714564;     vTpmslupExFXIZxHE80714564 = vTpmslupExFXIZxHE85521670;     vTpmslupExFXIZxHE85521670 = vTpmslupExFXIZxHE55142250;     vTpmslupExFXIZxHE55142250 = vTpmslupExFXIZxHE62874738;     vTpmslupExFXIZxHE62874738 = vTpmslupExFXIZxHE32144626;     vTpmslupExFXIZxHE32144626 = vTpmslupExFXIZxHE7711327;     vTpmslupExFXIZxHE7711327 = vTpmslupExFXIZxHE51799734;     vTpmslupExFXIZxHE51799734 = vTpmslupExFXIZxHE16832483;     vTpmslupExFXIZxHE16832483 = vTpmslupExFXIZxHE99698211;     vTpmslupExFXIZxHE99698211 = vTpmslupExFXIZxHE29304110;     vTpmslupExFXIZxHE29304110 = vTpmslupExFXIZxHE16528858;     vTpmslupExFXIZxHE16528858 = vTpmslupExFXIZxHE82099235;     vTpmslupExFXIZxHE82099235 = vTpmslupExFXIZxHE84449404;     vTpmslupExFXIZxHE84449404 = vTpmslupExFXIZxHE70308669;     vTpmslupExFXIZxHE70308669 = vTpmslupExFXIZxHE21338682;     vTpmslupExFXIZxHE21338682 = vTpmslupExFXIZxHE7865829;     vTpmslupExFXIZxHE7865829 = vTpmslupExFXIZxHE14599081;     vTpmslupExFXIZxHE14599081 = vTpmslupExFXIZxHE53819165;     vTpmslupExFXIZxHE53819165 = vTpmslupExFXIZxHE48967926;     vTpmslupExFXIZxHE48967926 = vTpmslupExFXIZxHE78381668;     vTpmslupExFXIZxHE78381668 = vTpmslupExFXIZxHE67779454;     vTpmslupExFXIZxHE67779454 = vTpmslupExFXIZxHE97090927;     vTpmslupExFXIZxHE97090927 = vTpmslupExFXIZxHE2640785;     vTpmslupExFXIZxHE2640785 = vTpmslupExFXIZxHE27020366;     vTpmslupExFXIZxHE27020366 = vTpmslupExFXIZxHE8336181;     vTpmslupExFXIZxHE8336181 = vTpmslupExFXIZxHE45563704;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void abUqsKWTlNtypkiM55003983() {     double OphJHLDCxNiMzMHzp81022672 = -280567927;    double OphJHLDCxNiMzMHzp49843161 = -458455318;    double OphJHLDCxNiMzMHzp73042002 = -816352025;    double OphJHLDCxNiMzMHzp13618089 = -319182809;    double OphJHLDCxNiMzMHzp21077899 = -259992266;    double OphJHLDCxNiMzMHzp25245579 = -4351345;    double OphJHLDCxNiMzMHzp64948761 = -403012240;    double OphJHLDCxNiMzMHzp38674039 = -166725222;    double OphJHLDCxNiMzMHzp25236441 = -447193024;    double OphJHLDCxNiMzMHzp96781040 = -308149922;    double OphJHLDCxNiMzMHzp1150228 = -921264956;    double OphJHLDCxNiMzMHzp8990356 = -556001839;    double OphJHLDCxNiMzMHzp85430782 = -259412366;    double OphJHLDCxNiMzMHzp55382856 = -385825686;    double OphJHLDCxNiMzMHzp79241030 = -983816414;    double OphJHLDCxNiMzMHzp81715299 = -713926327;    double OphJHLDCxNiMzMHzp92815232 = 67643616;    double OphJHLDCxNiMzMHzp57614883 = -5529878;    double OphJHLDCxNiMzMHzp77028985 = -728976037;    double OphJHLDCxNiMzMHzp37109792 = -824569040;    double OphJHLDCxNiMzMHzp87672220 = -635697869;    double OphJHLDCxNiMzMHzp74305124 = -743666137;    double OphJHLDCxNiMzMHzp23943401 = -709662693;    double OphJHLDCxNiMzMHzp49064629 = -577094817;    double OphJHLDCxNiMzMHzp63582811 = -780205460;    double OphJHLDCxNiMzMHzp20542330 = -814064854;    double OphJHLDCxNiMzMHzp4744885 = -327662123;    double OphJHLDCxNiMzMHzp33222009 = -15098140;    double OphJHLDCxNiMzMHzp26430828 = -748908662;    double OphJHLDCxNiMzMHzp46531101 = -529432330;    double OphJHLDCxNiMzMHzp29206155 = -632120484;    double OphJHLDCxNiMzMHzp16730747 = -961281674;    double OphJHLDCxNiMzMHzp5011708 = -838255257;    double OphJHLDCxNiMzMHzp13184122 = -464769681;    double OphJHLDCxNiMzMHzp61827239 = -683218343;    double OphJHLDCxNiMzMHzp42579735 = -451837008;    double OphJHLDCxNiMzMHzp44394739 = -144214012;    double OphJHLDCxNiMzMHzp90864442 = -199466535;    double OphJHLDCxNiMzMHzp34306960 = -399802533;    double OphJHLDCxNiMzMHzp87351597 = -300913889;    double OphJHLDCxNiMzMHzp63028832 = 70905023;    double OphJHLDCxNiMzMHzp83235919 = -168962620;    double OphJHLDCxNiMzMHzp74745308 = -665375720;    double OphJHLDCxNiMzMHzp38407110 = -87474122;    double OphJHLDCxNiMzMHzp92413404 = -472528441;    double OphJHLDCxNiMzMHzp69381563 = -160518449;    double OphJHLDCxNiMzMHzp21477995 = -415996618;    double OphJHLDCxNiMzMHzp10250888 = -65053500;    double OphJHLDCxNiMzMHzp98553767 = -644698293;    double OphJHLDCxNiMzMHzp4593991 = -550713741;    double OphJHLDCxNiMzMHzp92464304 = -623320736;    double OphJHLDCxNiMzMHzp77579537 = -113550332;    double OphJHLDCxNiMzMHzp65429641 = -214584309;    double OphJHLDCxNiMzMHzp24336139 = -396276499;    double OphJHLDCxNiMzMHzp67331762 = -176368979;    double OphJHLDCxNiMzMHzp6717548 = -536901791;    double OphJHLDCxNiMzMHzp25899761 = -748792625;    double OphJHLDCxNiMzMHzp23977374 = -139257209;    double OphJHLDCxNiMzMHzp50035277 = -538977349;    double OphJHLDCxNiMzMHzp535570 = -445927412;    double OphJHLDCxNiMzMHzp20500695 = -676689222;    double OphJHLDCxNiMzMHzp31726752 = -287914100;    double OphJHLDCxNiMzMHzp12243212 = -417816561;    double OphJHLDCxNiMzMHzp78705339 = -917760694;    double OphJHLDCxNiMzMHzp67574886 = -676029438;    double OphJHLDCxNiMzMHzp84419481 = -959983283;    double OphJHLDCxNiMzMHzp3978648 = -717746582;    double OphJHLDCxNiMzMHzp72246660 = -794642685;    double OphJHLDCxNiMzMHzp93555617 = -702607343;    double OphJHLDCxNiMzMHzp36661296 = -431979406;    double OphJHLDCxNiMzMHzp37320561 = -469712316;    double OphJHLDCxNiMzMHzp1950791 = -732889850;    double OphJHLDCxNiMzMHzp23307923 = -605727346;    double OphJHLDCxNiMzMHzp89677388 = -328062148;    double OphJHLDCxNiMzMHzp74080959 = -795474064;    double OphJHLDCxNiMzMHzp4436301 = -366735250;    double OphJHLDCxNiMzMHzp99559816 = 21709583;    double OphJHLDCxNiMzMHzp85536291 = -522188572;    double OphJHLDCxNiMzMHzp56651225 = -4566377;    double OphJHLDCxNiMzMHzp94201248 = -519687012;    double OphJHLDCxNiMzMHzp99064334 = -298068237;    double OphJHLDCxNiMzMHzp94493996 = -162608624;    double OphJHLDCxNiMzMHzp34668242 = -370399847;    double OphJHLDCxNiMzMHzp21836837 = -98194922;    double OphJHLDCxNiMzMHzp54066797 = -906111595;    double OphJHLDCxNiMzMHzp51626617 = -418570153;    double OphJHLDCxNiMzMHzp51301105 = -646697365;    double OphJHLDCxNiMzMHzp80675569 = -341978759;    double OphJHLDCxNiMzMHzp45852360 = -188400702;    double OphJHLDCxNiMzMHzp55109691 = -46316552;    double OphJHLDCxNiMzMHzp16679974 = -703044384;    double OphJHLDCxNiMzMHzp20417366 = 95043196;    double OphJHLDCxNiMzMHzp40829165 = -660489186;    double OphJHLDCxNiMzMHzp33771391 = -953875121;    double OphJHLDCxNiMzMHzp66850903 = -624224668;    double OphJHLDCxNiMzMHzp31302080 = -641180877;    double OphJHLDCxNiMzMHzp70992708 = -751146060;    double OphJHLDCxNiMzMHzp96039968 = -747615026;    double OphJHLDCxNiMzMHzp70832224 = -411444685;    double OphJHLDCxNiMzMHzp7993923 = -280567927;     OphJHLDCxNiMzMHzp81022672 = OphJHLDCxNiMzMHzp49843161;     OphJHLDCxNiMzMHzp49843161 = OphJHLDCxNiMzMHzp73042002;     OphJHLDCxNiMzMHzp73042002 = OphJHLDCxNiMzMHzp13618089;     OphJHLDCxNiMzMHzp13618089 = OphJHLDCxNiMzMHzp21077899;     OphJHLDCxNiMzMHzp21077899 = OphJHLDCxNiMzMHzp25245579;     OphJHLDCxNiMzMHzp25245579 = OphJHLDCxNiMzMHzp64948761;     OphJHLDCxNiMzMHzp64948761 = OphJHLDCxNiMzMHzp38674039;     OphJHLDCxNiMzMHzp38674039 = OphJHLDCxNiMzMHzp25236441;     OphJHLDCxNiMzMHzp25236441 = OphJHLDCxNiMzMHzp96781040;     OphJHLDCxNiMzMHzp96781040 = OphJHLDCxNiMzMHzp1150228;     OphJHLDCxNiMzMHzp1150228 = OphJHLDCxNiMzMHzp8990356;     OphJHLDCxNiMzMHzp8990356 = OphJHLDCxNiMzMHzp85430782;     OphJHLDCxNiMzMHzp85430782 = OphJHLDCxNiMzMHzp55382856;     OphJHLDCxNiMzMHzp55382856 = OphJHLDCxNiMzMHzp79241030;     OphJHLDCxNiMzMHzp79241030 = OphJHLDCxNiMzMHzp81715299;     OphJHLDCxNiMzMHzp81715299 = OphJHLDCxNiMzMHzp92815232;     OphJHLDCxNiMzMHzp92815232 = OphJHLDCxNiMzMHzp57614883;     OphJHLDCxNiMzMHzp57614883 = OphJHLDCxNiMzMHzp77028985;     OphJHLDCxNiMzMHzp77028985 = OphJHLDCxNiMzMHzp37109792;     OphJHLDCxNiMzMHzp37109792 = OphJHLDCxNiMzMHzp87672220;     OphJHLDCxNiMzMHzp87672220 = OphJHLDCxNiMzMHzp74305124;     OphJHLDCxNiMzMHzp74305124 = OphJHLDCxNiMzMHzp23943401;     OphJHLDCxNiMzMHzp23943401 = OphJHLDCxNiMzMHzp49064629;     OphJHLDCxNiMzMHzp49064629 = OphJHLDCxNiMzMHzp63582811;     OphJHLDCxNiMzMHzp63582811 = OphJHLDCxNiMzMHzp20542330;     OphJHLDCxNiMzMHzp20542330 = OphJHLDCxNiMzMHzp4744885;     OphJHLDCxNiMzMHzp4744885 = OphJHLDCxNiMzMHzp33222009;     OphJHLDCxNiMzMHzp33222009 = OphJHLDCxNiMzMHzp26430828;     OphJHLDCxNiMzMHzp26430828 = OphJHLDCxNiMzMHzp46531101;     OphJHLDCxNiMzMHzp46531101 = OphJHLDCxNiMzMHzp29206155;     OphJHLDCxNiMzMHzp29206155 = OphJHLDCxNiMzMHzp16730747;     OphJHLDCxNiMzMHzp16730747 = OphJHLDCxNiMzMHzp5011708;     OphJHLDCxNiMzMHzp5011708 = OphJHLDCxNiMzMHzp13184122;     OphJHLDCxNiMzMHzp13184122 = OphJHLDCxNiMzMHzp61827239;     OphJHLDCxNiMzMHzp61827239 = OphJHLDCxNiMzMHzp42579735;     OphJHLDCxNiMzMHzp42579735 = OphJHLDCxNiMzMHzp44394739;     OphJHLDCxNiMzMHzp44394739 = OphJHLDCxNiMzMHzp90864442;     OphJHLDCxNiMzMHzp90864442 = OphJHLDCxNiMzMHzp34306960;     OphJHLDCxNiMzMHzp34306960 = OphJHLDCxNiMzMHzp87351597;     OphJHLDCxNiMzMHzp87351597 = OphJHLDCxNiMzMHzp63028832;     OphJHLDCxNiMzMHzp63028832 = OphJHLDCxNiMzMHzp83235919;     OphJHLDCxNiMzMHzp83235919 = OphJHLDCxNiMzMHzp74745308;     OphJHLDCxNiMzMHzp74745308 = OphJHLDCxNiMzMHzp38407110;     OphJHLDCxNiMzMHzp38407110 = OphJHLDCxNiMzMHzp92413404;     OphJHLDCxNiMzMHzp92413404 = OphJHLDCxNiMzMHzp69381563;     OphJHLDCxNiMzMHzp69381563 = OphJHLDCxNiMzMHzp21477995;     OphJHLDCxNiMzMHzp21477995 = OphJHLDCxNiMzMHzp10250888;     OphJHLDCxNiMzMHzp10250888 = OphJHLDCxNiMzMHzp98553767;     OphJHLDCxNiMzMHzp98553767 = OphJHLDCxNiMzMHzp4593991;     OphJHLDCxNiMzMHzp4593991 = OphJHLDCxNiMzMHzp92464304;     OphJHLDCxNiMzMHzp92464304 = OphJHLDCxNiMzMHzp77579537;     OphJHLDCxNiMzMHzp77579537 = OphJHLDCxNiMzMHzp65429641;     OphJHLDCxNiMzMHzp65429641 = OphJHLDCxNiMzMHzp24336139;     OphJHLDCxNiMzMHzp24336139 = OphJHLDCxNiMzMHzp67331762;     OphJHLDCxNiMzMHzp67331762 = OphJHLDCxNiMzMHzp6717548;     OphJHLDCxNiMzMHzp6717548 = OphJHLDCxNiMzMHzp25899761;     OphJHLDCxNiMzMHzp25899761 = OphJHLDCxNiMzMHzp23977374;     OphJHLDCxNiMzMHzp23977374 = OphJHLDCxNiMzMHzp50035277;     OphJHLDCxNiMzMHzp50035277 = OphJHLDCxNiMzMHzp535570;     OphJHLDCxNiMzMHzp535570 = OphJHLDCxNiMzMHzp20500695;     OphJHLDCxNiMzMHzp20500695 = OphJHLDCxNiMzMHzp31726752;     OphJHLDCxNiMzMHzp31726752 = OphJHLDCxNiMzMHzp12243212;     OphJHLDCxNiMzMHzp12243212 = OphJHLDCxNiMzMHzp78705339;     OphJHLDCxNiMzMHzp78705339 = OphJHLDCxNiMzMHzp67574886;     OphJHLDCxNiMzMHzp67574886 = OphJHLDCxNiMzMHzp84419481;     OphJHLDCxNiMzMHzp84419481 = OphJHLDCxNiMzMHzp3978648;     OphJHLDCxNiMzMHzp3978648 = OphJHLDCxNiMzMHzp72246660;     OphJHLDCxNiMzMHzp72246660 = OphJHLDCxNiMzMHzp93555617;     OphJHLDCxNiMzMHzp93555617 = OphJHLDCxNiMzMHzp36661296;     OphJHLDCxNiMzMHzp36661296 = OphJHLDCxNiMzMHzp37320561;     OphJHLDCxNiMzMHzp37320561 = OphJHLDCxNiMzMHzp1950791;     OphJHLDCxNiMzMHzp1950791 = OphJHLDCxNiMzMHzp23307923;     OphJHLDCxNiMzMHzp23307923 = OphJHLDCxNiMzMHzp89677388;     OphJHLDCxNiMzMHzp89677388 = OphJHLDCxNiMzMHzp74080959;     OphJHLDCxNiMzMHzp74080959 = OphJHLDCxNiMzMHzp4436301;     OphJHLDCxNiMzMHzp4436301 = OphJHLDCxNiMzMHzp99559816;     OphJHLDCxNiMzMHzp99559816 = OphJHLDCxNiMzMHzp85536291;     OphJHLDCxNiMzMHzp85536291 = OphJHLDCxNiMzMHzp56651225;     OphJHLDCxNiMzMHzp56651225 = OphJHLDCxNiMzMHzp94201248;     OphJHLDCxNiMzMHzp94201248 = OphJHLDCxNiMzMHzp99064334;     OphJHLDCxNiMzMHzp99064334 = OphJHLDCxNiMzMHzp94493996;     OphJHLDCxNiMzMHzp94493996 = OphJHLDCxNiMzMHzp34668242;     OphJHLDCxNiMzMHzp34668242 = OphJHLDCxNiMzMHzp21836837;     OphJHLDCxNiMzMHzp21836837 = OphJHLDCxNiMzMHzp54066797;     OphJHLDCxNiMzMHzp54066797 = OphJHLDCxNiMzMHzp51626617;     OphJHLDCxNiMzMHzp51626617 = OphJHLDCxNiMzMHzp51301105;     OphJHLDCxNiMzMHzp51301105 = OphJHLDCxNiMzMHzp80675569;     OphJHLDCxNiMzMHzp80675569 = OphJHLDCxNiMzMHzp45852360;     OphJHLDCxNiMzMHzp45852360 = OphJHLDCxNiMzMHzp55109691;     OphJHLDCxNiMzMHzp55109691 = OphJHLDCxNiMzMHzp16679974;     OphJHLDCxNiMzMHzp16679974 = OphJHLDCxNiMzMHzp20417366;     OphJHLDCxNiMzMHzp20417366 = OphJHLDCxNiMzMHzp40829165;     OphJHLDCxNiMzMHzp40829165 = OphJHLDCxNiMzMHzp33771391;     OphJHLDCxNiMzMHzp33771391 = OphJHLDCxNiMzMHzp66850903;     OphJHLDCxNiMzMHzp66850903 = OphJHLDCxNiMzMHzp31302080;     OphJHLDCxNiMzMHzp31302080 = OphJHLDCxNiMzMHzp70992708;     OphJHLDCxNiMzMHzp70992708 = OphJHLDCxNiMzMHzp96039968;     OphJHLDCxNiMzMHzp96039968 = OphJHLDCxNiMzMHzp70832224;     OphJHLDCxNiMzMHzp70832224 = OphJHLDCxNiMzMHzp7993923;     OphJHLDCxNiMzMHzp7993923 = OphJHLDCxNiMzMHzp81022672;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tssBuFIcVQkdXxSA70053050() {     double odULRLShwgWOdcWIW87139778 = -392469816;    double odULRLShwgWOdcWIW93216440 = -168055098;    double odULRLShwgWOdcWIW89084185 = -967184947;    double odULRLShwgWOdcWIW40692869 = -509931439;    double odULRLShwgWOdcWIW89973387 = -780642137;    double odULRLShwgWOdcWIW16662781 = -268847580;    double odULRLShwgWOdcWIW25437728 = -541977217;    double odULRLShwgWOdcWIW88433929 = -453599135;    double odULRLShwgWOdcWIW89878936 = -215225689;    double odULRLShwgWOdcWIW38010700 = -637125400;    double odULRLShwgWOdcWIW61260517 = -798702543;    double odULRLShwgWOdcWIW40813398 = 99591959;    double odULRLShwgWOdcWIW71639901 = -699303454;    double odULRLShwgWOdcWIW38526533 = -918064254;    double odULRLShwgWOdcWIW74983457 = -574278580;    double odULRLShwgWOdcWIW23220693 = -686925871;    double odULRLShwgWOdcWIW86651235 = -548901497;    double odULRLShwgWOdcWIW54668036 = -771052520;    double odULRLShwgWOdcWIW38858827 = -34745134;    double odULRLShwgWOdcWIW17659957 = -81471960;    double odULRLShwgWOdcWIW76225413 = -491185324;    double odULRLShwgWOdcWIW60826148 = -651595853;    double odULRLShwgWOdcWIW59534193 = -456180180;    double odULRLShwgWOdcWIW33057003 = -799399805;    double odULRLShwgWOdcWIW7282462 = -53401493;    double odULRLShwgWOdcWIW45264005 = -692720177;    double odULRLShwgWOdcWIW3204663 = -228324326;    double odULRLShwgWOdcWIW46329135 = -81027094;    double odULRLShwgWOdcWIW92563083 = -971291367;    double odULRLShwgWOdcWIW15146739 = -346903198;    double odULRLShwgWOdcWIW21448704 = -545441423;    double odULRLShwgWOdcWIW14341368 = -670728489;    double odULRLShwgWOdcWIW14722657 = -730868628;    double odULRLShwgWOdcWIW8014687 = -216311446;    double odULRLShwgWOdcWIW23301891 = -221951448;    double odULRLShwgWOdcWIW11305648 = -665863116;    double odULRLShwgWOdcWIW25656674 = -97849882;    double odULRLShwgWOdcWIW24954655 = 29027455;    double odULRLShwgWOdcWIW7917214 = -393726029;    double odULRLShwgWOdcWIW74853535 = -774353112;    double odULRLShwgWOdcWIW68698056 = 5631129;    double odULRLShwgWOdcWIW15386377 = -439104898;    double odULRLShwgWOdcWIW52812286 = -746180303;    double odULRLShwgWOdcWIW25814850 = -106817060;    double odULRLShwgWOdcWIW4365869 = -134426595;    double odULRLShwgWOdcWIW81320758 = -145734565;    double odULRLShwgWOdcWIW39256569 = -534772155;    double odULRLShwgWOdcWIW18207829 = -74949032;    double odULRLShwgWOdcWIW8229497 = -730435281;    double odULRLShwgWOdcWIW17505644 = -637724549;    double odULRLShwgWOdcWIW28164983 = -465614407;    double odULRLShwgWOdcWIW88389257 = 85723348;    double odULRLShwgWOdcWIW13678099 = -6373897;    double odULRLShwgWOdcWIW20305020 = -716746644;    double odULRLShwgWOdcWIW93827637 = -876900762;    double odULRLShwgWOdcWIW26313631 = -740873964;    double odULRLShwgWOdcWIW33682248 = -711874919;    double odULRLShwgWOdcWIW56027183 = -67785142;    double odULRLShwgWOdcWIW33410408 = -356529947;    double odULRLShwgWOdcWIW44709383 = 12078040;    double odULRLShwgWOdcWIW13458119 = 59476746;    double odULRLShwgWOdcWIW79108593 = -360950123;    double odULRLShwgWOdcWIW95870846 = -482307769;    double odULRLShwgWOdcWIW74732198 = -868322492;    double odULRLShwgWOdcWIW16561996 = 8316022;    double odULRLShwgWOdcWIW46919149 = -27974054;    double odULRLShwgWOdcWIW26090742 = -169539414;    double odULRLShwgWOdcWIW63625215 = -382992009;    double odULRLShwgWOdcWIW15224642 = -596112806;    double odULRLShwgWOdcWIW63677809 = -908415465;    double odULRLShwgWOdcWIW97564019 = -489075989;    double odULRLShwgWOdcWIW61696581 = -477928953;    double odULRLShwgWOdcWIW46750822 = -277326491;    double odULRLShwgWOdcWIW64005292 = -260392023;    double odULRLShwgWOdcWIW48961900 = 12896911;    double odULRLShwgWOdcWIW60839037 = 47919573;    double odULRLShwgWOdcWIW8013862 = -905415550;    double odULRLShwgWOdcWIW33719343 = -249363120;    double odULRLShwgWOdcWIW28691134 = -564973210;    double odULRLShwgWOdcWIW25961704 = -907666928;    double odULRLShwgWOdcWIW6007436 = -57948023;    double odULRLShwgWOdcWIW84996834 = -53375295;    double odULRLShwgWOdcWIW38099638 = -350591813;    double odULRLShwgWOdcWIW75057439 = -233566818;    double odULRLShwgWOdcWIW86981755 = -881288791;    double odULRLShwgWOdcWIW33059447 = -531164771;    double odULRLShwgWOdcWIW663270 = -564354593;    double odULRLShwgWOdcWIW94417637 = 85878016;    double odULRLShwgWOdcWIW14187050 = -339410685;    double odULRLShwgWOdcWIW96988260 = -481077485;    double odULRLShwgWOdcWIW77623399 = -953988198;    double odULRLShwgWOdcWIW69629490 = 69935260;    double odULRLShwgWOdcWIW91544246 = -614442598;    double odULRLShwgWOdcWIW63207831 = -305804070;    double odULRLShwgWOdcWIW61395416 = -733829859;    double odULRLShwgWOdcWIW89589463 = -633418748;    double odULRLShwgWOdcWIW19515530 = -956797129;    double odULRLShwgWOdcWIW78080088 = -877857812;    double odULRLShwgWOdcWIW9252855 = -15133083;    double odULRLShwgWOdcWIW57446719 = -392469816;     odULRLShwgWOdcWIW87139778 = odULRLShwgWOdcWIW93216440;     odULRLShwgWOdcWIW93216440 = odULRLShwgWOdcWIW89084185;     odULRLShwgWOdcWIW89084185 = odULRLShwgWOdcWIW40692869;     odULRLShwgWOdcWIW40692869 = odULRLShwgWOdcWIW89973387;     odULRLShwgWOdcWIW89973387 = odULRLShwgWOdcWIW16662781;     odULRLShwgWOdcWIW16662781 = odULRLShwgWOdcWIW25437728;     odULRLShwgWOdcWIW25437728 = odULRLShwgWOdcWIW88433929;     odULRLShwgWOdcWIW88433929 = odULRLShwgWOdcWIW89878936;     odULRLShwgWOdcWIW89878936 = odULRLShwgWOdcWIW38010700;     odULRLShwgWOdcWIW38010700 = odULRLShwgWOdcWIW61260517;     odULRLShwgWOdcWIW61260517 = odULRLShwgWOdcWIW40813398;     odULRLShwgWOdcWIW40813398 = odULRLShwgWOdcWIW71639901;     odULRLShwgWOdcWIW71639901 = odULRLShwgWOdcWIW38526533;     odULRLShwgWOdcWIW38526533 = odULRLShwgWOdcWIW74983457;     odULRLShwgWOdcWIW74983457 = odULRLShwgWOdcWIW23220693;     odULRLShwgWOdcWIW23220693 = odULRLShwgWOdcWIW86651235;     odULRLShwgWOdcWIW86651235 = odULRLShwgWOdcWIW54668036;     odULRLShwgWOdcWIW54668036 = odULRLShwgWOdcWIW38858827;     odULRLShwgWOdcWIW38858827 = odULRLShwgWOdcWIW17659957;     odULRLShwgWOdcWIW17659957 = odULRLShwgWOdcWIW76225413;     odULRLShwgWOdcWIW76225413 = odULRLShwgWOdcWIW60826148;     odULRLShwgWOdcWIW60826148 = odULRLShwgWOdcWIW59534193;     odULRLShwgWOdcWIW59534193 = odULRLShwgWOdcWIW33057003;     odULRLShwgWOdcWIW33057003 = odULRLShwgWOdcWIW7282462;     odULRLShwgWOdcWIW7282462 = odULRLShwgWOdcWIW45264005;     odULRLShwgWOdcWIW45264005 = odULRLShwgWOdcWIW3204663;     odULRLShwgWOdcWIW3204663 = odULRLShwgWOdcWIW46329135;     odULRLShwgWOdcWIW46329135 = odULRLShwgWOdcWIW92563083;     odULRLShwgWOdcWIW92563083 = odULRLShwgWOdcWIW15146739;     odULRLShwgWOdcWIW15146739 = odULRLShwgWOdcWIW21448704;     odULRLShwgWOdcWIW21448704 = odULRLShwgWOdcWIW14341368;     odULRLShwgWOdcWIW14341368 = odULRLShwgWOdcWIW14722657;     odULRLShwgWOdcWIW14722657 = odULRLShwgWOdcWIW8014687;     odULRLShwgWOdcWIW8014687 = odULRLShwgWOdcWIW23301891;     odULRLShwgWOdcWIW23301891 = odULRLShwgWOdcWIW11305648;     odULRLShwgWOdcWIW11305648 = odULRLShwgWOdcWIW25656674;     odULRLShwgWOdcWIW25656674 = odULRLShwgWOdcWIW24954655;     odULRLShwgWOdcWIW24954655 = odULRLShwgWOdcWIW7917214;     odULRLShwgWOdcWIW7917214 = odULRLShwgWOdcWIW74853535;     odULRLShwgWOdcWIW74853535 = odULRLShwgWOdcWIW68698056;     odULRLShwgWOdcWIW68698056 = odULRLShwgWOdcWIW15386377;     odULRLShwgWOdcWIW15386377 = odULRLShwgWOdcWIW52812286;     odULRLShwgWOdcWIW52812286 = odULRLShwgWOdcWIW25814850;     odULRLShwgWOdcWIW25814850 = odULRLShwgWOdcWIW4365869;     odULRLShwgWOdcWIW4365869 = odULRLShwgWOdcWIW81320758;     odULRLShwgWOdcWIW81320758 = odULRLShwgWOdcWIW39256569;     odULRLShwgWOdcWIW39256569 = odULRLShwgWOdcWIW18207829;     odULRLShwgWOdcWIW18207829 = odULRLShwgWOdcWIW8229497;     odULRLShwgWOdcWIW8229497 = odULRLShwgWOdcWIW17505644;     odULRLShwgWOdcWIW17505644 = odULRLShwgWOdcWIW28164983;     odULRLShwgWOdcWIW28164983 = odULRLShwgWOdcWIW88389257;     odULRLShwgWOdcWIW88389257 = odULRLShwgWOdcWIW13678099;     odULRLShwgWOdcWIW13678099 = odULRLShwgWOdcWIW20305020;     odULRLShwgWOdcWIW20305020 = odULRLShwgWOdcWIW93827637;     odULRLShwgWOdcWIW93827637 = odULRLShwgWOdcWIW26313631;     odULRLShwgWOdcWIW26313631 = odULRLShwgWOdcWIW33682248;     odULRLShwgWOdcWIW33682248 = odULRLShwgWOdcWIW56027183;     odULRLShwgWOdcWIW56027183 = odULRLShwgWOdcWIW33410408;     odULRLShwgWOdcWIW33410408 = odULRLShwgWOdcWIW44709383;     odULRLShwgWOdcWIW44709383 = odULRLShwgWOdcWIW13458119;     odULRLShwgWOdcWIW13458119 = odULRLShwgWOdcWIW79108593;     odULRLShwgWOdcWIW79108593 = odULRLShwgWOdcWIW95870846;     odULRLShwgWOdcWIW95870846 = odULRLShwgWOdcWIW74732198;     odULRLShwgWOdcWIW74732198 = odULRLShwgWOdcWIW16561996;     odULRLShwgWOdcWIW16561996 = odULRLShwgWOdcWIW46919149;     odULRLShwgWOdcWIW46919149 = odULRLShwgWOdcWIW26090742;     odULRLShwgWOdcWIW26090742 = odULRLShwgWOdcWIW63625215;     odULRLShwgWOdcWIW63625215 = odULRLShwgWOdcWIW15224642;     odULRLShwgWOdcWIW15224642 = odULRLShwgWOdcWIW63677809;     odULRLShwgWOdcWIW63677809 = odULRLShwgWOdcWIW97564019;     odULRLShwgWOdcWIW97564019 = odULRLShwgWOdcWIW61696581;     odULRLShwgWOdcWIW61696581 = odULRLShwgWOdcWIW46750822;     odULRLShwgWOdcWIW46750822 = odULRLShwgWOdcWIW64005292;     odULRLShwgWOdcWIW64005292 = odULRLShwgWOdcWIW48961900;     odULRLShwgWOdcWIW48961900 = odULRLShwgWOdcWIW60839037;     odULRLShwgWOdcWIW60839037 = odULRLShwgWOdcWIW8013862;     odULRLShwgWOdcWIW8013862 = odULRLShwgWOdcWIW33719343;     odULRLShwgWOdcWIW33719343 = odULRLShwgWOdcWIW28691134;     odULRLShwgWOdcWIW28691134 = odULRLShwgWOdcWIW25961704;     odULRLShwgWOdcWIW25961704 = odULRLShwgWOdcWIW6007436;     odULRLShwgWOdcWIW6007436 = odULRLShwgWOdcWIW84996834;     odULRLShwgWOdcWIW84996834 = odULRLShwgWOdcWIW38099638;     odULRLShwgWOdcWIW38099638 = odULRLShwgWOdcWIW75057439;     odULRLShwgWOdcWIW75057439 = odULRLShwgWOdcWIW86981755;     odULRLShwgWOdcWIW86981755 = odULRLShwgWOdcWIW33059447;     odULRLShwgWOdcWIW33059447 = odULRLShwgWOdcWIW663270;     odULRLShwgWOdcWIW663270 = odULRLShwgWOdcWIW94417637;     odULRLShwgWOdcWIW94417637 = odULRLShwgWOdcWIW14187050;     odULRLShwgWOdcWIW14187050 = odULRLShwgWOdcWIW96988260;     odULRLShwgWOdcWIW96988260 = odULRLShwgWOdcWIW77623399;     odULRLShwgWOdcWIW77623399 = odULRLShwgWOdcWIW69629490;     odULRLShwgWOdcWIW69629490 = odULRLShwgWOdcWIW91544246;     odULRLShwgWOdcWIW91544246 = odULRLShwgWOdcWIW63207831;     odULRLShwgWOdcWIW63207831 = odULRLShwgWOdcWIW61395416;     odULRLShwgWOdcWIW61395416 = odULRLShwgWOdcWIW89589463;     odULRLShwgWOdcWIW89589463 = odULRLShwgWOdcWIW19515530;     odULRLShwgWOdcWIW19515530 = odULRLShwgWOdcWIW78080088;     odULRLShwgWOdcWIW78080088 = odULRLShwgWOdcWIW9252855;     odULRLShwgWOdcWIW9252855 = odULRLShwgWOdcWIW57446719;     odULRLShwgWOdcWIW57446719 = odULRLShwgWOdcWIW87139778;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void iLAHkUUlVmomGiuo37344650() {     double PAXKGcBIItHRvTiRd22598748 = -858746716;    double PAXKGcBIItHRvTiRd6488520 = -48606705;    double PAXKGcBIItHRvTiRd26470663 = -632852022;    double PAXKGcBIItHRvTiRd12916224 = -331082511;    double PAXKGcBIItHRvTiRd46258656 = -886240983;    double PAXKGcBIItHRvTiRd52885653 = -531195475;    double PAXKGcBIItHRvTiRd36095134 = -386702951;    double PAXKGcBIItHRvTiRd50565491 = -97833276;    double PAXKGcBIItHRvTiRd91396541 = -703557096;    double PAXKGcBIItHRvTiRd52913412 = -301812398;    double PAXKGcBIItHRvTiRd99442252 = -600710272;    double PAXKGcBIItHRvTiRd41381546 = -864528812;    double PAXKGcBIItHRvTiRd9874547 = -151315959;    double PAXKGcBIItHRvTiRd91667396 = -894427663;    double PAXKGcBIItHRvTiRd33177973 = 22296405;    double PAXKGcBIItHRvTiRd6761994 = -322737478;    double PAXKGcBIItHRvTiRd42487171 = 24123951;    double PAXKGcBIItHRvTiRd54428227 = -844480589;    double PAXKGcBIItHRvTiRd39409157 = -351308555;    double PAXKGcBIItHRvTiRd11497175 = -508033026;    double PAXKGcBIItHRvTiRd89312276 = -148460013;    double PAXKGcBIItHRvTiRd64247673 = -738232147;    double PAXKGcBIItHRvTiRd30059573 = -93812364;    double PAXKGcBIItHRvTiRd26849948 = -210224409;    double PAXKGcBIItHRvTiRd15300413 = -77386692;    double PAXKGcBIItHRvTiRd55962176 = -381296984;    double PAXKGcBIItHRvTiRd63625236 = -34608467;    double PAXKGcBIItHRvTiRd87057559 = -250799268;    double PAXKGcBIItHRvTiRd39231298 = -445386169;    double PAXKGcBIItHRvTiRd20680149 = -381410368;    double PAXKGcBIItHRvTiRd40031936 = 2578090;    double PAXKGcBIItHRvTiRd52164386 = -443699904;    double PAXKGcBIItHRvTiRd26940667 = -878464605;    double PAXKGcBIItHRvTiRd37763113 = -281005260;    double PAXKGcBIItHRvTiRd89110344 = -173051699;    double PAXKGcBIItHRvTiRd62866491 = -880428422;    double PAXKGcBIItHRvTiRd75068490 = 37454078;    double PAXKGcBIItHRvTiRd76170058 = 46371874;    double PAXKGcBIItHRvTiRd38307775 = -44602306;    double PAXKGcBIItHRvTiRd39125067 = -239615658;    double PAXKGcBIItHRvTiRd2149664 = -863567785;    double PAXKGcBIItHRvTiRd4751504 = -33650189;    double PAXKGcBIItHRvTiRd42195664 = -299361089;    double PAXKGcBIItHRvTiRd65946188 = -711149470;    double PAXKGcBIItHRvTiRd4382328 = 39062244;    double PAXKGcBIItHRvTiRd27282087 = -184658198;    double PAXKGcBIItHRvTiRd58601733 = 81241484;    double PAXKGcBIItHRvTiRd35934140 = -241475625;    double PAXKGcBIItHRvTiRd31122163 = -394172162;    double PAXKGcBIItHRvTiRd42035233 = -28751365;    double PAXKGcBIItHRvTiRd8935707 = -339890964;    double PAXKGcBIItHRvTiRd71874729 = -127928999;    double PAXKGcBIItHRvTiRd82299246 = -862419451;    double PAXKGcBIItHRvTiRd36296866 = -908255144;    double PAXKGcBIItHRvTiRd48032373 = -123390907;    double PAXKGcBIItHRvTiRd58351074 = -20514570;    double PAXKGcBIItHRvTiRd76428947 = -954794341;    double PAXKGcBIItHRvTiRd99620714 = -322627613;    double PAXKGcBIItHRvTiRd97615811 = -153695820;    double PAXKGcBIItHRvTiRd90296480 = -404943999;    double PAXKGcBIItHRvTiRd89260416 = -396587009;    double PAXKGcBIItHRvTiRd49037574 = -35903683;    double PAXKGcBIItHRvTiRd11334194 = -652447107;    double PAXKGcBIItHRvTiRd70716392 = -222146728;    double PAXKGcBIItHRvTiRd12881477 = -204390489;    double PAXKGcBIItHRvTiRd47277866 = -57010369;    double PAXKGcBIItHRvTiRd14440879 = -986064208;    double PAXKGcBIItHRvTiRd72111433 = -870310700;    double PAXKGcBIItHRvTiRd2557052 = -621375964;    double PAXKGcBIItHRvTiRd70311482 = -97275174;    double PAXKGcBIItHRvTiRd31693504 = -260191556;    double PAXKGcBIItHRvTiRd66317113 = 77752076;    double PAXKGcBIItHRvTiRd16120453 = -699878284;    double PAXKGcBIItHRvTiRd284090 = -11692898;    double PAXKGcBIItHRvTiRd9347511 = -644465241;    double PAXKGcBIItHRvTiRd84560773 = -14809825;    double PAXKGcBIItHRvTiRd22052009 = -338871058;    double PAXKGcBIItHRvTiRd64113384 = -382662895;    double PAXKGcBIItHRvTiRd22467620 = -149286653;    double PAXKGcBIItHRvTiRd88018326 = -892728494;    double PAXKGcBIItHRvTiRd97360443 = -362538469;    double PAXKGcBIItHRvTiRd27691096 = -793132842;    double PAXKGcBIItHRvTiRd55935397 = -856627107;    double PAXKGcBIItHRvTiRd97196064 = -316634805;    double PAXKGcBIItHRvTiRd11744443 = 58480595;    double PAXKGcBIItHRvTiRd68157206 = -869492911;    double PAXKGcBIItHRvTiRd69865139 = -581280454;    double PAXKGcBIItHRvTiRd90643801 = -970209461;    double PAXKGcBIItHRvTiRd89730739 = -57614353;    double PAXKGcBIItHRvTiRd30759270 = -52537130;    double PAXKGcBIItHRvTiRd86437544 = -925634081;    double PAXKGcBIItHRvTiRd75447775 = -639918309;    double PAXKGcBIItHRvTiRd78554246 = -799932307;    double PAXKGcBIItHRvTiRd48011295 = -639658307;    double PAXKGcBIItHRvTiRd49864651 = -843028650;    double PAXKGcBIItHRvTiRd53112090 = -727664102;    double PAXKGcBIItHRvTiRd93417310 = -381203082;    double PAXKGcBIItHRvTiRd71479272 = 22785639;    double PAXKGcBIItHRvTiRd53064712 = -406758982;    double PAXKGcBIItHRvTiRd57104462 = -858746716;     PAXKGcBIItHRvTiRd22598748 = PAXKGcBIItHRvTiRd6488520;     PAXKGcBIItHRvTiRd6488520 = PAXKGcBIItHRvTiRd26470663;     PAXKGcBIItHRvTiRd26470663 = PAXKGcBIItHRvTiRd12916224;     PAXKGcBIItHRvTiRd12916224 = PAXKGcBIItHRvTiRd46258656;     PAXKGcBIItHRvTiRd46258656 = PAXKGcBIItHRvTiRd52885653;     PAXKGcBIItHRvTiRd52885653 = PAXKGcBIItHRvTiRd36095134;     PAXKGcBIItHRvTiRd36095134 = PAXKGcBIItHRvTiRd50565491;     PAXKGcBIItHRvTiRd50565491 = PAXKGcBIItHRvTiRd91396541;     PAXKGcBIItHRvTiRd91396541 = PAXKGcBIItHRvTiRd52913412;     PAXKGcBIItHRvTiRd52913412 = PAXKGcBIItHRvTiRd99442252;     PAXKGcBIItHRvTiRd99442252 = PAXKGcBIItHRvTiRd41381546;     PAXKGcBIItHRvTiRd41381546 = PAXKGcBIItHRvTiRd9874547;     PAXKGcBIItHRvTiRd9874547 = PAXKGcBIItHRvTiRd91667396;     PAXKGcBIItHRvTiRd91667396 = PAXKGcBIItHRvTiRd33177973;     PAXKGcBIItHRvTiRd33177973 = PAXKGcBIItHRvTiRd6761994;     PAXKGcBIItHRvTiRd6761994 = PAXKGcBIItHRvTiRd42487171;     PAXKGcBIItHRvTiRd42487171 = PAXKGcBIItHRvTiRd54428227;     PAXKGcBIItHRvTiRd54428227 = PAXKGcBIItHRvTiRd39409157;     PAXKGcBIItHRvTiRd39409157 = PAXKGcBIItHRvTiRd11497175;     PAXKGcBIItHRvTiRd11497175 = PAXKGcBIItHRvTiRd89312276;     PAXKGcBIItHRvTiRd89312276 = PAXKGcBIItHRvTiRd64247673;     PAXKGcBIItHRvTiRd64247673 = PAXKGcBIItHRvTiRd30059573;     PAXKGcBIItHRvTiRd30059573 = PAXKGcBIItHRvTiRd26849948;     PAXKGcBIItHRvTiRd26849948 = PAXKGcBIItHRvTiRd15300413;     PAXKGcBIItHRvTiRd15300413 = PAXKGcBIItHRvTiRd55962176;     PAXKGcBIItHRvTiRd55962176 = PAXKGcBIItHRvTiRd63625236;     PAXKGcBIItHRvTiRd63625236 = PAXKGcBIItHRvTiRd87057559;     PAXKGcBIItHRvTiRd87057559 = PAXKGcBIItHRvTiRd39231298;     PAXKGcBIItHRvTiRd39231298 = PAXKGcBIItHRvTiRd20680149;     PAXKGcBIItHRvTiRd20680149 = PAXKGcBIItHRvTiRd40031936;     PAXKGcBIItHRvTiRd40031936 = PAXKGcBIItHRvTiRd52164386;     PAXKGcBIItHRvTiRd52164386 = PAXKGcBIItHRvTiRd26940667;     PAXKGcBIItHRvTiRd26940667 = PAXKGcBIItHRvTiRd37763113;     PAXKGcBIItHRvTiRd37763113 = PAXKGcBIItHRvTiRd89110344;     PAXKGcBIItHRvTiRd89110344 = PAXKGcBIItHRvTiRd62866491;     PAXKGcBIItHRvTiRd62866491 = PAXKGcBIItHRvTiRd75068490;     PAXKGcBIItHRvTiRd75068490 = PAXKGcBIItHRvTiRd76170058;     PAXKGcBIItHRvTiRd76170058 = PAXKGcBIItHRvTiRd38307775;     PAXKGcBIItHRvTiRd38307775 = PAXKGcBIItHRvTiRd39125067;     PAXKGcBIItHRvTiRd39125067 = PAXKGcBIItHRvTiRd2149664;     PAXKGcBIItHRvTiRd2149664 = PAXKGcBIItHRvTiRd4751504;     PAXKGcBIItHRvTiRd4751504 = PAXKGcBIItHRvTiRd42195664;     PAXKGcBIItHRvTiRd42195664 = PAXKGcBIItHRvTiRd65946188;     PAXKGcBIItHRvTiRd65946188 = PAXKGcBIItHRvTiRd4382328;     PAXKGcBIItHRvTiRd4382328 = PAXKGcBIItHRvTiRd27282087;     PAXKGcBIItHRvTiRd27282087 = PAXKGcBIItHRvTiRd58601733;     PAXKGcBIItHRvTiRd58601733 = PAXKGcBIItHRvTiRd35934140;     PAXKGcBIItHRvTiRd35934140 = PAXKGcBIItHRvTiRd31122163;     PAXKGcBIItHRvTiRd31122163 = PAXKGcBIItHRvTiRd42035233;     PAXKGcBIItHRvTiRd42035233 = PAXKGcBIItHRvTiRd8935707;     PAXKGcBIItHRvTiRd8935707 = PAXKGcBIItHRvTiRd71874729;     PAXKGcBIItHRvTiRd71874729 = PAXKGcBIItHRvTiRd82299246;     PAXKGcBIItHRvTiRd82299246 = PAXKGcBIItHRvTiRd36296866;     PAXKGcBIItHRvTiRd36296866 = PAXKGcBIItHRvTiRd48032373;     PAXKGcBIItHRvTiRd48032373 = PAXKGcBIItHRvTiRd58351074;     PAXKGcBIItHRvTiRd58351074 = PAXKGcBIItHRvTiRd76428947;     PAXKGcBIItHRvTiRd76428947 = PAXKGcBIItHRvTiRd99620714;     PAXKGcBIItHRvTiRd99620714 = PAXKGcBIItHRvTiRd97615811;     PAXKGcBIItHRvTiRd97615811 = PAXKGcBIItHRvTiRd90296480;     PAXKGcBIItHRvTiRd90296480 = PAXKGcBIItHRvTiRd89260416;     PAXKGcBIItHRvTiRd89260416 = PAXKGcBIItHRvTiRd49037574;     PAXKGcBIItHRvTiRd49037574 = PAXKGcBIItHRvTiRd11334194;     PAXKGcBIItHRvTiRd11334194 = PAXKGcBIItHRvTiRd70716392;     PAXKGcBIItHRvTiRd70716392 = PAXKGcBIItHRvTiRd12881477;     PAXKGcBIItHRvTiRd12881477 = PAXKGcBIItHRvTiRd47277866;     PAXKGcBIItHRvTiRd47277866 = PAXKGcBIItHRvTiRd14440879;     PAXKGcBIItHRvTiRd14440879 = PAXKGcBIItHRvTiRd72111433;     PAXKGcBIItHRvTiRd72111433 = PAXKGcBIItHRvTiRd2557052;     PAXKGcBIItHRvTiRd2557052 = PAXKGcBIItHRvTiRd70311482;     PAXKGcBIItHRvTiRd70311482 = PAXKGcBIItHRvTiRd31693504;     PAXKGcBIItHRvTiRd31693504 = PAXKGcBIItHRvTiRd66317113;     PAXKGcBIItHRvTiRd66317113 = PAXKGcBIItHRvTiRd16120453;     PAXKGcBIItHRvTiRd16120453 = PAXKGcBIItHRvTiRd284090;     PAXKGcBIItHRvTiRd284090 = PAXKGcBIItHRvTiRd9347511;     PAXKGcBIItHRvTiRd9347511 = PAXKGcBIItHRvTiRd84560773;     PAXKGcBIItHRvTiRd84560773 = PAXKGcBIItHRvTiRd22052009;     PAXKGcBIItHRvTiRd22052009 = PAXKGcBIItHRvTiRd64113384;     PAXKGcBIItHRvTiRd64113384 = PAXKGcBIItHRvTiRd22467620;     PAXKGcBIItHRvTiRd22467620 = PAXKGcBIItHRvTiRd88018326;     PAXKGcBIItHRvTiRd88018326 = PAXKGcBIItHRvTiRd97360443;     PAXKGcBIItHRvTiRd97360443 = PAXKGcBIItHRvTiRd27691096;     PAXKGcBIItHRvTiRd27691096 = PAXKGcBIItHRvTiRd55935397;     PAXKGcBIItHRvTiRd55935397 = PAXKGcBIItHRvTiRd97196064;     PAXKGcBIItHRvTiRd97196064 = PAXKGcBIItHRvTiRd11744443;     PAXKGcBIItHRvTiRd11744443 = PAXKGcBIItHRvTiRd68157206;     PAXKGcBIItHRvTiRd68157206 = PAXKGcBIItHRvTiRd69865139;     PAXKGcBIItHRvTiRd69865139 = PAXKGcBIItHRvTiRd90643801;     PAXKGcBIItHRvTiRd90643801 = PAXKGcBIItHRvTiRd89730739;     PAXKGcBIItHRvTiRd89730739 = PAXKGcBIItHRvTiRd30759270;     PAXKGcBIItHRvTiRd30759270 = PAXKGcBIItHRvTiRd86437544;     PAXKGcBIItHRvTiRd86437544 = PAXKGcBIItHRvTiRd75447775;     PAXKGcBIItHRvTiRd75447775 = PAXKGcBIItHRvTiRd78554246;     PAXKGcBIItHRvTiRd78554246 = PAXKGcBIItHRvTiRd48011295;     PAXKGcBIItHRvTiRd48011295 = PAXKGcBIItHRvTiRd49864651;     PAXKGcBIItHRvTiRd49864651 = PAXKGcBIItHRvTiRd53112090;     PAXKGcBIItHRvTiRd53112090 = PAXKGcBIItHRvTiRd93417310;     PAXKGcBIItHRvTiRd93417310 = PAXKGcBIItHRvTiRd71479272;     PAXKGcBIItHRvTiRd71479272 = PAXKGcBIItHRvTiRd53064712;     PAXKGcBIItHRvTiRd53064712 = PAXKGcBIItHRvTiRd57104462;     PAXKGcBIItHRvTiRd57104462 = PAXKGcBIItHRvTiRd22598748;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cWOTgExSLReARHkl52393718() {     double DdINdUlnJMVMTCAlT28715854 = -970648605;    double DdINdUlnJMVMTCAlT49861799 = -858206486;    double DdINdUlnJMVMTCAlT42512846 = -783684943;    double DdINdUlnJMVMTCAlT39991005 = -521831141;    double DdINdUlnJMVMTCAlT15154145 = -306890854;    double DdINdUlnJMVMTCAlT44302855 = -795691710;    double DdINdUlnJMVMTCAlT96584099 = -525667928;    double DdINdUlnJMVMTCAlT325382 = -384707189;    double DdINdUlnJMVMTCAlT56039036 = -471589762;    double DdINdUlnJMVMTCAlT94143071 = -630787875;    double DdINdUlnJMVMTCAlT59552542 = -478147859;    double DdINdUlnJMVMTCAlT73204588 = -208935014;    double DdINdUlnJMVMTCAlT96083664 = -591207048;    double DdINdUlnJMVMTCAlT74811072 = -326666231;    double DdINdUlnJMVMTCAlT28920400 = -668165762;    double DdINdUlnJMVMTCAlT48267388 = -295737021;    double DdINdUlnJMVMTCAlT36323174 = -592421162;    double DdINdUlnJMVMTCAlT51481380 = -510003232;    double DdINdUlnJMVMTCAlT1238999 = -757077652;    double DdINdUlnJMVMTCAlT92047339 = -864935945;    double DdINdUlnJMVMTCAlT77865469 = -3947469;    double DdINdUlnJMVMTCAlT50768696 = -646161863;    double DdINdUlnJMVMTCAlT65650364 = -940329851;    double DdINdUlnJMVMTCAlT10842322 = -432529397;    double DdINdUlnJMVMTCAlT59000063 = -450582724;    double DdINdUlnJMVMTCAlT80683851 = -259952308;    double DdINdUlnJMVMTCAlT62085014 = 64729330;    double DdINdUlnJMVMTCAlT164685 = -316728222;    double DdINdUlnJMVMTCAlT5363554 = -667768874;    double DdINdUlnJMVMTCAlT89295785 = -198881236;    double DdINdUlnJMVMTCAlT32274485 = 89257152;    double DdINdUlnJMVMTCAlT49775008 = -153146719;    double DdINdUlnJMVMTCAlT36651616 = -771077975;    double DdINdUlnJMVMTCAlT32593677 = -32547025;    double DdINdUlnJMVMTCAlT50584997 = -811784804;    double DdINdUlnJMVMTCAlT31592405 = 5545471;    double DdINdUlnJMVMTCAlT56330425 = 83818208;    double DdINdUlnJMVMTCAlT10260271 = -825134135;    double DdINdUlnJMVMTCAlT11918029 = -38525803;    double DdINdUlnJMVMTCAlT26627005 = -713054881;    double DdINdUlnJMVMTCAlT7818888 = -928841679;    double DdINdUlnJMVMTCAlT36901960 = -303792467;    double DdINdUlnJMVMTCAlT20262643 = -380165673;    double DdINdUlnJMVMTCAlT53353929 = -730492409;    double DdINdUlnJMVMTCAlT16334793 = -722835911;    double DdINdUlnJMVMTCAlT39221282 = -169874314;    double DdINdUlnJMVMTCAlT76380307 = -37534053;    double DdINdUlnJMVMTCAlT43891081 = -251371158;    double DdINdUlnJMVMTCAlT40797892 = -479909150;    double DdINdUlnJMVMTCAlT54946885 = -115762173;    double DdINdUlnJMVMTCAlT44636385 = -182184636;    double DdINdUlnJMVMTCAlT82684449 = 71344681;    double DdINdUlnJMVMTCAlT30547704 = -654209038;    double DdINdUlnJMVMTCAlT32265746 = -128725289;    double DdINdUlnJMVMTCAlT74528248 = -823922690;    double DdINdUlnJMVMTCAlT77947157 = -224486742;    double DdINdUlnJMVMTCAlT84211434 = -917876635;    double DdINdUlnJMVMTCAlT31670525 = -251155546;    double DdINdUlnJMVMTCAlT80990942 = 28751583;    double DdINdUlnJMVMTCAlT34470293 = 53061453;    double DdINdUlnJMVMTCAlT82217840 = -760421040;    double DdINdUlnJMVMTCAlT96419414 = -108939707;    double DdINdUlnJMVMTCAlT94961828 = -716938316;    double DdINdUlnJMVMTCAlT66743251 = -172708527;    double DdINdUlnJMVMTCAlT61868586 = -620045028;    double DdINdUlnJMVMTCAlT9777535 = -225001140;    double DdINdUlnJMVMTCAlT36552973 = -437857039;    double DdINdUlnJMVMTCAlT63489988 = -458660024;    double DdINdUlnJMVMTCAlT24226076 = -514881427;    double DdINdUlnJMVMTCAlT97327995 = -573711233;    double DdINdUlnJMVMTCAlT91936963 = -279555230;    double DdINdUlnJMVMTCAlT26062904 = -767287027;    double DdINdUlnJMVMTCAlT39563352 = -371477429;    double DdINdUlnJMVMTCAlT74611994 = 55977228;    double DdINdUlnJMVMTCAlT84228451 = -936094266;    double DdINdUlnJMVMTCAlT40963509 = -700155002;    double DdINdUlnJMVMTCAlT30506054 = -165996191;    double DdINdUlnJMVMTCAlT12296436 = -109837443;    double DdINdUlnJMVMTCAlT94507528 = -709693487;    double DdINdUlnJMVMTCAlT19778781 = -180708411;    double DdINdUlnJMVMTCAlT4303545 = -122418255;    double DdINdUlnJMVMTCAlT18193934 = -683899513;    double DdINdUlnJMVMTCAlT59366793 = -836819073;    double DdINdUlnJMVMTCAlT50416668 = -452006701;    double DdINdUlnJMVMTCAlT44659401 = 83303399;    double DdINdUlnJMVMTCAlT49590036 = -982087529;    double DdINdUlnJMVMTCAlT19227304 = -498937682;    double DdINdUlnJMVMTCAlT4385870 = -542352687;    double DdINdUlnJMVMTCAlT58065429 = -208624335;    double DdINdUlnJMVMTCAlT72637839 = -487298063;    double DdINdUlnJMVMTCAlT47380970 = -76577895;    double DdINdUlnJMVMTCAlT24659900 = -665026246;    double DdINdUlnJMVMTCAlT29269329 = -753885719;    double DdINdUlnJMVMTCAlT77447735 = 8412744;    double DdINdUlnJMVMTCAlT44409164 = -952633841;    double DdINdUlnJMVMTCAlT11399473 = -719901973;    double DdINdUlnJMVMTCAlT41940132 = -586854152;    double DdINdUlnJMVMTCAlT53519392 = -107457147;    double DdINdUlnJMVMTCAlT91485342 = -10447381;    double DdINdUlnJMVMTCAlT6557259 = -970648605;     DdINdUlnJMVMTCAlT28715854 = DdINdUlnJMVMTCAlT49861799;     DdINdUlnJMVMTCAlT49861799 = DdINdUlnJMVMTCAlT42512846;     DdINdUlnJMVMTCAlT42512846 = DdINdUlnJMVMTCAlT39991005;     DdINdUlnJMVMTCAlT39991005 = DdINdUlnJMVMTCAlT15154145;     DdINdUlnJMVMTCAlT15154145 = DdINdUlnJMVMTCAlT44302855;     DdINdUlnJMVMTCAlT44302855 = DdINdUlnJMVMTCAlT96584099;     DdINdUlnJMVMTCAlT96584099 = DdINdUlnJMVMTCAlT325382;     DdINdUlnJMVMTCAlT325382 = DdINdUlnJMVMTCAlT56039036;     DdINdUlnJMVMTCAlT56039036 = DdINdUlnJMVMTCAlT94143071;     DdINdUlnJMVMTCAlT94143071 = DdINdUlnJMVMTCAlT59552542;     DdINdUlnJMVMTCAlT59552542 = DdINdUlnJMVMTCAlT73204588;     DdINdUlnJMVMTCAlT73204588 = DdINdUlnJMVMTCAlT96083664;     DdINdUlnJMVMTCAlT96083664 = DdINdUlnJMVMTCAlT74811072;     DdINdUlnJMVMTCAlT74811072 = DdINdUlnJMVMTCAlT28920400;     DdINdUlnJMVMTCAlT28920400 = DdINdUlnJMVMTCAlT48267388;     DdINdUlnJMVMTCAlT48267388 = DdINdUlnJMVMTCAlT36323174;     DdINdUlnJMVMTCAlT36323174 = DdINdUlnJMVMTCAlT51481380;     DdINdUlnJMVMTCAlT51481380 = DdINdUlnJMVMTCAlT1238999;     DdINdUlnJMVMTCAlT1238999 = DdINdUlnJMVMTCAlT92047339;     DdINdUlnJMVMTCAlT92047339 = DdINdUlnJMVMTCAlT77865469;     DdINdUlnJMVMTCAlT77865469 = DdINdUlnJMVMTCAlT50768696;     DdINdUlnJMVMTCAlT50768696 = DdINdUlnJMVMTCAlT65650364;     DdINdUlnJMVMTCAlT65650364 = DdINdUlnJMVMTCAlT10842322;     DdINdUlnJMVMTCAlT10842322 = DdINdUlnJMVMTCAlT59000063;     DdINdUlnJMVMTCAlT59000063 = DdINdUlnJMVMTCAlT80683851;     DdINdUlnJMVMTCAlT80683851 = DdINdUlnJMVMTCAlT62085014;     DdINdUlnJMVMTCAlT62085014 = DdINdUlnJMVMTCAlT164685;     DdINdUlnJMVMTCAlT164685 = DdINdUlnJMVMTCAlT5363554;     DdINdUlnJMVMTCAlT5363554 = DdINdUlnJMVMTCAlT89295785;     DdINdUlnJMVMTCAlT89295785 = DdINdUlnJMVMTCAlT32274485;     DdINdUlnJMVMTCAlT32274485 = DdINdUlnJMVMTCAlT49775008;     DdINdUlnJMVMTCAlT49775008 = DdINdUlnJMVMTCAlT36651616;     DdINdUlnJMVMTCAlT36651616 = DdINdUlnJMVMTCAlT32593677;     DdINdUlnJMVMTCAlT32593677 = DdINdUlnJMVMTCAlT50584997;     DdINdUlnJMVMTCAlT50584997 = DdINdUlnJMVMTCAlT31592405;     DdINdUlnJMVMTCAlT31592405 = DdINdUlnJMVMTCAlT56330425;     DdINdUlnJMVMTCAlT56330425 = DdINdUlnJMVMTCAlT10260271;     DdINdUlnJMVMTCAlT10260271 = DdINdUlnJMVMTCAlT11918029;     DdINdUlnJMVMTCAlT11918029 = DdINdUlnJMVMTCAlT26627005;     DdINdUlnJMVMTCAlT26627005 = DdINdUlnJMVMTCAlT7818888;     DdINdUlnJMVMTCAlT7818888 = DdINdUlnJMVMTCAlT36901960;     DdINdUlnJMVMTCAlT36901960 = DdINdUlnJMVMTCAlT20262643;     DdINdUlnJMVMTCAlT20262643 = DdINdUlnJMVMTCAlT53353929;     DdINdUlnJMVMTCAlT53353929 = DdINdUlnJMVMTCAlT16334793;     DdINdUlnJMVMTCAlT16334793 = DdINdUlnJMVMTCAlT39221282;     DdINdUlnJMVMTCAlT39221282 = DdINdUlnJMVMTCAlT76380307;     DdINdUlnJMVMTCAlT76380307 = DdINdUlnJMVMTCAlT43891081;     DdINdUlnJMVMTCAlT43891081 = DdINdUlnJMVMTCAlT40797892;     DdINdUlnJMVMTCAlT40797892 = DdINdUlnJMVMTCAlT54946885;     DdINdUlnJMVMTCAlT54946885 = DdINdUlnJMVMTCAlT44636385;     DdINdUlnJMVMTCAlT44636385 = DdINdUlnJMVMTCAlT82684449;     DdINdUlnJMVMTCAlT82684449 = DdINdUlnJMVMTCAlT30547704;     DdINdUlnJMVMTCAlT30547704 = DdINdUlnJMVMTCAlT32265746;     DdINdUlnJMVMTCAlT32265746 = DdINdUlnJMVMTCAlT74528248;     DdINdUlnJMVMTCAlT74528248 = DdINdUlnJMVMTCAlT77947157;     DdINdUlnJMVMTCAlT77947157 = DdINdUlnJMVMTCAlT84211434;     DdINdUlnJMVMTCAlT84211434 = DdINdUlnJMVMTCAlT31670525;     DdINdUlnJMVMTCAlT31670525 = DdINdUlnJMVMTCAlT80990942;     DdINdUlnJMVMTCAlT80990942 = DdINdUlnJMVMTCAlT34470293;     DdINdUlnJMVMTCAlT34470293 = DdINdUlnJMVMTCAlT82217840;     DdINdUlnJMVMTCAlT82217840 = DdINdUlnJMVMTCAlT96419414;     DdINdUlnJMVMTCAlT96419414 = DdINdUlnJMVMTCAlT94961828;     DdINdUlnJMVMTCAlT94961828 = DdINdUlnJMVMTCAlT66743251;     DdINdUlnJMVMTCAlT66743251 = DdINdUlnJMVMTCAlT61868586;     DdINdUlnJMVMTCAlT61868586 = DdINdUlnJMVMTCAlT9777535;     DdINdUlnJMVMTCAlT9777535 = DdINdUlnJMVMTCAlT36552973;     DdINdUlnJMVMTCAlT36552973 = DdINdUlnJMVMTCAlT63489988;     DdINdUlnJMVMTCAlT63489988 = DdINdUlnJMVMTCAlT24226076;     DdINdUlnJMVMTCAlT24226076 = DdINdUlnJMVMTCAlT97327995;     DdINdUlnJMVMTCAlT97327995 = DdINdUlnJMVMTCAlT91936963;     DdINdUlnJMVMTCAlT91936963 = DdINdUlnJMVMTCAlT26062904;     DdINdUlnJMVMTCAlT26062904 = DdINdUlnJMVMTCAlT39563352;     DdINdUlnJMVMTCAlT39563352 = DdINdUlnJMVMTCAlT74611994;     DdINdUlnJMVMTCAlT74611994 = DdINdUlnJMVMTCAlT84228451;     DdINdUlnJMVMTCAlT84228451 = DdINdUlnJMVMTCAlT40963509;     DdINdUlnJMVMTCAlT40963509 = DdINdUlnJMVMTCAlT30506054;     DdINdUlnJMVMTCAlT30506054 = DdINdUlnJMVMTCAlT12296436;     DdINdUlnJMVMTCAlT12296436 = DdINdUlnJMVMTCAlT94507528;     DdINdUlnJMVMTCAlT94507528 = DdINdUlnJMVMTCAlT19778781;     DdINdUlnJMVMTCAlT19778781 = DdINdUlnJMVMTCAlT4303545;     DdINdUlnJMVMTCAlT4303545 = DdINdUlnJMVMTCAlT18193934;     DdINdUlnJMVMTCAlT18193934 = DdINdUlnJMVMTCAlT59366793;     DdINdUlnJMVMTCAlT59366793 = DdINdUlnJMVMTCAlT50416668;     DdINdUlnJMVMTCAlT50416668 = DdINdUlnJMVMTCAlT44659401;     DdINdUlnJMVMTCAlT44659401 = DdINdUlnJMVMTCAlT49590036;     DdINdUlnJMVMTCAlT49590036 = DdINdUlnJMVMTCAlT19227304;     DdINdUlnJMVMTCAlT19227304 = DdINdUlnJMVMTCAlT4385870;     DdINdUlnJMVMTCAlT4385870 = DdINdUlnJMVMTCAlT58065429;     DdINdUlnJMVMTCAlT58065429 = DdINdUlnJMVMTCAlT72637839;     DdINdUlnJMVMTCAlT72637839 = DdINdUlnJMVMTCAlT47380970;     DdINdUlnJMVMTCAlT47380970 = DdINdUlnJMVMTCAlT24659900;     DdINdUlnJMVMTCAlT24659900 = DdINdUlnJMVMTCAlT29269329;     DdINdUlnJMVMTCAlT29269329 = DdINdUlnJMVMTCAlT77447735;     DdINdUlnJMVMTCAlT77447735 = DdINdUlnJMVMTCAlT44409164;     DdINdUlnJMVMTCAlT44409164 = DdINdUlnJMVMTCAlT11399473;     DdINdUlnJMVMTCAlT11399473 = DdINdUlnJMVMTCAlT41940132;     DdINdUlnJMVMTCAlT41940132 = DdINdUlnJMVMTCAlT53519392;     DdINdUlnJMVMTCAlT53519392 = DdINdUlnJMVMTCAlT91485342;     DdINdUlnJMVMTCAlT91485342 = DdINdUlnJMVMTCAlT6557259;     DdINdUlnJMVMTCAlT6557259 = DdINdUlnJMVMTCAlT28715854;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void BdXaSAsFUregqqhr19685317() {     double pNAHOxzZACphNlPkV64174822 = -336925505;    double pNAHOxzZACphNlPkV63133877 = -738758092;    double pNAHOxzZACphNlPkV79899323 = -449352018;    double pNAHOxzZACphNlPkV12214360 = -342982212;    double pNAHOxzZACphNlPkV71439414 = -412489699;    double pNAHOxzZACphNlPkV80525727 = 41960395;    double pNAHOxzZACphNlPkV7241506 = -370393662;    double pNAHOxzZACphNlPkV62456942 = -28941329;    double pNAHOxzZACphNlPkV57556642 = -959921169;    double pNAHOxzZACphNlPkV9045784 = -295474873;    double pNAHOxzZACphNlPkV97734277 = -280155588;    double pNAHOxzZACphNlPkV73772737 = -73055786;    double pNAHOxzZACphNlPkV34318310 = -43219553;    double pNAHOxzZACphNlPkV27951936 = -303029640;    double pNAHOxzZACphNlPkV87114916 = -71590777;    double pNAHOxzZACphNlPkV31808689 = 68451372;    double pNAHOxzZACphNlPkV92159110 = -19395715;    double pNAHOxzZACphNlPkV51241572 = -583431300;    double pNAHOxzZACphNlPkV1789329 = 26358926;    double pNAHOxzZACphNlPkV85884558 = -191497011;    double pNAHOxzZACphNlPkV90952332 = -761222158;    double pNAHOxzZACphNlPkV54190221 = -732798157;    double pNAHOxzZACphNlPkV36175744 = -577962035;    double pNAHOxzZACphNlPkV4635267 = -943354001;    double pNAHOxzZACphNlPkV67018014 = -474567923;    double pNAHOxzZACphNlPkV91382023 = 51470886;    double pNAHOxzZACphNlPkV22505589 = -841554811;    double pNAHOxzZACphNlPkV40893110 = -486500396;    double pNAHOxzZACphNlPkV52031768 = -141863676;    double pNAHOxzZACphNlPkV94829196 = -233388406;    double pNAHOxzZACphNlPkV50857717 = -462723335;    double pNAHOxzZACphNlPkV87598026 = 73881866;    double pNAHOxzZACphNlPkV48869627 = -918673953;    double pNAHOxzZACphNlPkV62342103 = -97240838;    double pNAHOxzZACphNlPkV16393451 = -762885055;    double pNAHOxzZACphNlPkV83153248 = -209019835;    double pNAHOxzZACphNlPkV5742242 = -880877832;    double pNAHOxzZACphNlPkV61475674 = -807789716;    double pNAHOxzZACphNlPkV42308590 = -789402079;    double pNAHOxzZACphNlPkV90898537 = -178317427;    double pNAHOxzZACphNlPkV41270496 = -698040593;    double pNAHOxzZACphNlPkV26267087 = -998337758;    double pNAHOxzZACphNlPkV9646021 = 66653541;    double pNAHOxzZACphNlPkV93485266 = -234824819;    double pNAHOxzZACphNlPkV16351252 = -549347072;    double pNAHOxzZACphNlPkV85182610 = -208797946;    double pNAHOxzZACphNlPkV95725471 = -521520414;    double pNAHOxzZACphNlPkV61617393 = -417897751;    double pNAHOxzZACphNlPkV63690558 = -143646030;    double pNAHOxzZACphNlPkV79476474 = -606788989;    double pNAHOxzZACphNlPkV25407109 = -56461193;    double pNAHOxzZACphNlPkV66169921 = -142307666;    double pNAHOxzZACphNlPkV99168852 = -410254592;    double pNAHOxzZACphNlPkV48257593 = -320233789;    double pNAHOxzZACphNlPkV28732985 = -70412836;    double pNAHOxzZACphNlPkV9984602 = -604127348;    double pNAHOxzZACphNlPkV26958133 = -60796057;    double pNAHOxzZACphNlPkV75264056 = -505998017;    double pNAHOxzZACphNlPkV45196346 = -868414290;    double pNAHOxzZACphNlPkV80057390 = -363960585;    double pNAHOxzZACphNlPkV58020138 = -116484795;    double pNAHOxzZACphNlPkV66348396 = -883893267;    double pNAHOxzZACphNlPkV10425175 = -887077654;    double pNAHOxzZACphNlPkV62727445 = -626532763;    double pNAHOxzZACphNlPkV58188067 = -832751539;    double pNAHOxzZACphNlPkV10136251 = -254037455;    double pNAHOxzZACphNlPkV24903111 = -154381833;    double pNAHOxzZACphNlPkV71976207 = -945978715;    double pNAHOxzZACphNlPkV11558486 = -540144585;    double pNAHOxzZACphNlPkV3961668 = -862570942;    double pNAHOxzZACphNlPkV26066448 = -50670797;    double pNAHOxzZACphNlPkV30683436 = -211605999;    double pNAHOxzZACphNlPkV8932982 = -794029222;    double pNAHOxzZACphNlPkV10890792 = -795323648;    double pNAHOxzZACphNlPkV44614063 = -493456418;    double pNAHOxzZACphNlPkV64685245 = -762884400;    double pNAHOxzZACphNlPkV44544200 = -699451699;    double pNAHOxzZACphNlPkV42690477 = -243137217;    double pNAHOxzZACphNlPkV88284015 = -294006929;    double pNAHOxzZACphNlPkV81835403 = -165769977;    double pNAHOxzZACphNlPkV95656552 = -427008701;    double pNAHOxzZACphNlPkV60888195 = -323657060;    double pNAHOxzZACphNlPkV77202552 = -242854367;    double pNAHOxzZACphNlPkV72555293 = -535074688;    double pNAHOxzZACphNlPkV69422088 = -76927214;    double pNAHOxzZACphNlPkV84687795 = -220415670;    double pNAHOxzZACphNlPkV88429174 = -515863542;    double pNAHOxzZACphNlPkV612034 = -498440164;    double pNAHOxzZACphNlPkV33609119 = 73171997;    double pNAHOxzZACphNlPkV6408850 = -58757708;    double pNAHOxzZACphNlPkV56195115 = -48223778;    double pNAHOxzZACphNlPkV30478185 = -274879815;    double pNAHOxzZACphNlPkV16279328 = -939375427;    double pNAHOxzZACphNlPkV62251199 = -325441494;    double pNAHOxzZACphNlPkV32878399 = 38167368;    double pNAHOxzZACphNlPkV74922099 = -814147327;    double pNAHOxzZACphNlPkV15841913 = -11260105;    double pNAHOxzZACphNlPkV46918576 = -306813697;    double pNAHOxzZACphNlPkV35297200 = -402073280;    double pNAHOxzZACphNlPkV6215001 = -336925505;     pNAHOxzZACphNlPkV64174822 = pNAHOxzZACphNlPkV63133877;     pNAHOxzZACphNlPkV63133877 = pNAHOxzZACphNlPkV79899323;     pNAHOxzZACphNlPkV79899323 = pNAHOxzZACphNlPkV12214360;     pNAHOxzZACphNlPkV12214360 = pNAHOxzZACphNlPkV71439414;     pNAHOxzZACphNlPkV71439414 = pNAHOxzZACphNlPkV80525727;     pNAHOxzZACphNlPkV80525727 = pNAHOxzZACphNlPkV7241506;     pNAHOxzZACphNlPkV7241506 = pNAHOxzZACphNlPkV62456942;     pNAHOxzZACphNlPkV62456942 = pNAHOxzZACphNlPkV57556642;     pNAHOxzZACphNlPkV57556642 = pNAHOxzZACphNlPkV9045784;     pNAHOxzZACphNlPkV9045784 = pNAHOxzZACphNlPkV97734277;     pNAHOxzZACphNlPkV97734277 = pNAHOxzZACphNlPkV73772737;     pNAHOxzZACphNlPkV73772737 = pNAHOxzZACphNlPkV34318310;     pNAHOxzZACphNlPkV34318310 = pNAHOxzZACphNlPkV27951936;     pNAHOxzZACphNlPkV27951936 = pNAHOxzZACphNlPkV87114916;     pNAHOxzZACphNlPkV87114916 = pNAHOxzZACphNlPkV31808689;     pNAHOxzZACphNlPkV31808689 = pNAHOxzZACphNlPkV92159110;     pNAHOxzZACphNlPkV92159110 = pNAHOxzZACphNlPkV51241572;     pNAHOxzZACphNlPkV51241572 = pNAHOxzZACphNlPkV1789329;     pNAHOxzZACphNlPkV1789329 = pNAHOxzZACphNlPkV85884558;     pNAHOxzZACphNlPkV85884558 = pNAHOxzZACphNlPkV90952332;     pNAHOxzZACphNlPkV90952332 = pNAHOxzZACphNlPkV54190221;     pNAHOxzZACphNlPkV54190221 = pNAHOxzZACphNlPkV36175744;     pNAHOxzZACphNlPkV36175744 = pNAHOxzZACphNlPkV4635267;     pNAHOxzZACphNlPkV4635267 = pNAHOxzZACphNlPkV67018014;     pNAHOxzZACphNlPkV67018014 = pNAHOxzZACphNlPkV91382023;     pNAHOxzZACphNlPkV91382023 = pNAHOxzZACphNlPkV22505589;     pNAHOxzZACphNlPkV22505589 = pNAHOxzZACphNlPkV40893110;     pNAHOxzZACphNlPkV40893110 = pNAHOxzZACphNlPkV52031768;     pNAHOxzZACphNlPkV52031768 = pNAHOxzZACphNlPkV94829196;     pNAHOxzZACphNlPkV94829196 = pNAHOxzZACphNlPkV50857717;     pNAHOxzZACphNlPkV50857717 = pNAHOxzZACphNlPkV87598026;     pNAHOxzZACphNlPkV87598026 = pNAHOxzZACphNlPkV48869627;     pNAHOxzZACphNlPkV48869627 = pNAHOxzZACphNlPkV62342103;     pNAHOxzZACphNlPkV62342103 = pNAHOxzZACphNlPkV16393451;     pNAHOxzZACphNlPkV16393451 = pNAHOxzZACphNlPkV83153248;     pNAHOxzZACphNlPkV83153248 = pNAHOxzZACphNlPkV5742242;     pNAHOxzZACphNlPkV5742242 = pNAHOxzZACphNlPkV61475674;     pNAHOxzZACphNlPkV61475674 = pNAHOxzZACphNlPkV42308590;     pNAHOxzZACphNlPkV42308590 = pNAHOxzZACphNlPkV90898537;     pNAHOxzZACphNlPkV90898537 = pNAHOxzZACphNlPkV41270496;     pNAHOxzZACphNlPkV41270496 = pNAHOxzZACphNlPkV26267087;     pNAHOxzZACphNlPkV26267087 = pNAHOxzZACphNlPkV9646021;     pNAHOxzZACphNlPkV9646021 = pNAHOxzZACphNlPkV93485266;     pNAHOxzZACphNlPkV93485266 = pNAHOxzZACphNlPkV16351252;     pNAHOxzZACphNlPkV16351252 = pNAHOxzZACphNlPkV85182610;     pNAHOxzZACphNlPkV85182610 = pNAHOxzZACphNlPkV95725471;     pNAHOxzZACphNlPkV95725471 = pNAHOxzZACphNlPkV61617393;     pNAHOxzZACphNlPkV61617393 = pNAHOxzZACphNlPkV63690558;     pNAHOxzZACphNlPkV63690558 = pNAHOxzZACphNlPkV79476474;     pNAHOxzZACphNlPkV79476474 = pNAHOxzZACphNlPkV25407109;     pNAHOxzZACphNlPkV25407109 = pNAHOxzZACphNlPkV66169921;     pNAHOxzZACphNlPkV66169921 = pNAHOxzZACphNlPkV99168852;     pNAHOxzZACphNlPkV99168852 = pNAHOxzZACphNlPkV48257593;     pNAHOxzZACphNlPkV48257593 = pNAHOxzZACphNlPkV28732985;     pNAHOxzZACphNlPkV28732985 = pNAHOxzZACphNlPkV9984602;     pNAHOxzZACphNlPkV9984602 = pNAHOxzZACphNlPkV26958133;     pNAHOxzZACphNlPkV26958133 = pNAHOxzZACphNlPkV75264056;     pNAHOxzZACphNlPkV75264056 = pNAHOxzZACphNlPkV45196346;     pNAHOxzZACphNlPkV45196346 = pNAHOxzZACphNlPkV80057390;     pNAHOxzZACphNlPkV80057390 = pNAHOxzZACphNlPkV58020138;     pNAHOxzZACphNlPkV58020138 = pNAHOxzZACphNlPkV66348396;     pNAHOxzZACphNlPkV66348396 = pNAHOxzZACphNlPkV10425175;     pNAHOxzZACphNlPkV10425175 = pNAHOxzZACphNlPkV62727445;     pNAHOxzZACphNlPkV62727445 = pNAHOxzZACphNlPkV58188067;     pNAHOxzZACphNlPkV58188067 = pNAHOxzZACphNlPkV10136251;     pNAHOxzZACphNlPkV10136251 = pNAHOxzZACphNlPkV24903111;     pNAHOxzZACphNlPkV24903111 = pNAHOxzZACphNlPkV71976207;     pNAHOxzZACphNlPkV71976207 = pNAHOxzZACphNlPkV11558486;     pNAHOxzZACphNlPkV11558486 = pNAHOxzZACphNlPkV3961668;     pNAHOxzZACphNlPkV3961668 = pNAHOxzZACphNlPkV26066448;     pNAHOxzZACphNlPkV26066448 = pNAHOxzZACphNlPkV30683436;     pNAHOxzZACphNlPkV30683436 = pNAHOxzZACphNlPkV8932982;     pNAHOxzZACphNlPkV8932982 = pNAHOxzZACphNlPkV10890792;     pNAHOxzZACphNlPkV10890792 = pNAHOxzZACphNlPkV44614063;     pNAHOxzZACphNlPkV44614063 = pNAHOxzZACphNlPkV64685245;     pNAHOxzZACphNlPkV64685245 = pNAHOxzZACphNlPkV44544200;     pNAHOxzZACphNlPkV44544200 = pNAHOxzZACphNlPkV42690477;     pNAHOxzZACphNlPkV42690477 = pNAHOxzZACphNlPkV88284015;     pNAHOxzZACphNlPkV88284015 = pNAHOxzZACphNlPkV81835403;     pNAHOxzZACphNlPkV81835403 = pNAHOxzZACphNlPkV95656552;     pNAHOxzZACphNlPkV95656552 = pNAHOxzZACphNlPkV60888195;     pNAHOxzZACphNlPkV60888195 = pNAHOxzZACphNlPkV77202552;     pNAHOxzZACphNlPkV77202552 = pNAHOxzZACphNlPkV72555293;     pNAHOxzZACphNlPkV72555293 = pNAHOxzZACphNlPkV69422088;     pNAHOxzZACphNlPkV69422088 = pNAHOxzZACphNlPkV84687795;     pNAHOxzZACphNlPkV84687795 = pNAHOxzZACphNlPkV88429174;     pNAHOxzZACphNlPkV88429174 = pNAHOxzZACphNlPkV612034;     pNAHOxzZACphNlPkV612034 = pNAHOxzZACphNlPkV33609119;     pNAHOxzZACphNlPkV33609119 = pNAHOxzZACphNlPkV6408850;     pNAHOxzZACphNlPkV6408850 = pNAHOxzZACphNlPkV56195115;     pNAHOxzZACphNlPkV56195115 = pNAHOxzZACphNlPkV30478185;     pNAHOxzZACphNlPkV30478185 = pNAHOxzZACphNlPkV16279328;     pNAHOxzZACphNlPkV16279328 = pNAHOxzZACphNlPkV62251199;     pNAHOxzZACphNlPkV62251199 = pNAHOxzZACphNlPkV32878399;     pNAHOxzZACphNlPkV32878399 = pNAHOxzZACphNlPkV74922099;     pNAHOxzZACphNlPkV74922099 = pNAHOxzZACphNlPkV15841913;     pNAHOxzZACphNlPkV15841913 = pNAHOxzZACphNlPkV46918576;     pNAHOxzZACphNlPkV46918576 = pNAHOxzZACphNlPkV35297200;     pNAHOxzZACphNlPkV35297200 = pNAHOxzZACphNlPkV6215001;     pNAHOxzZACphNlPkV6215001 = pNAHOxzZACphNlPkV64174822;}
// Junk Finished

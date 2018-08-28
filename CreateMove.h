#pragma once
#include "HookIncludes.h"
#include "Misc.h"
#include "RageBot.h"
#include "LegitBot.h"
#include "GrenadePrediction.h"
#include "LagComp.h"
#include "ESP.h"
#include "OverrideView.h"
#include "LocalInfo.h"

using create_move_t = void(__thiscall *)(IBaseClientDLL *, int, float, bool);
QAngle qLastTickAngles = QAngle(0.0f, 0.0f, 0.0f);
void __stdcall CHLCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
{
	static auto ofunc = hooks::client.get_original<create_move_t>(22);
	ofunc(g_CHLClient, sequence_number, input_sample_frametime, active);
	C_BaseEntity *local = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
	CInput::CUserCmd *cmd = g_Input->GetUserCmd(0, sequence_number);
	LocalInfo* localinfo1;
	CInput::CUserCmd * pCmd;

	if (!cmd)
		return;

	

	if (!cmd->command_number)
		return;

	CInput::CVerifiedUserCmd *verifiedCommands = *(CInput::CVerifiedUserCmd **)(reinterpret_cast< uint32_t >(g_Input) + 0xF0);
	CInput::CVerifiedUserCmd *verified = &verifiedCommands[sequence_number % 150];
	if (!verified)
		return;

	if (!local)
		return;
	QAngle oldAngle = cmd->viewangles;
	QAngle oldangles;
	float oldForward = cmd->forwardmove;
	float oldSideMove = cmd->sidemove;

	backtracking->legitBackTrack(cmd, local);

	legitbot::instance().OnCreateMove(cmd, local, bSendPacket);
	ragebot::instance().OnCreateMove(cmd, bSendPacket);
	misc::instance().OnCreateMove(cmd, local, bSendPacket);
	switch (g_Options.Misc.AutoStrafe)
	{
	case 1:
		misc::LegitAutoStrafe(cmd, local, cmd->viewangles);
		break;
	case 2:
		misc::RageAutoStrafe(cmd, local, cmd->viewangles);
		break;
	}

	CCSGrenadeHint::instance().Tick(cmd->buttons);
	if (!bSendPacket && g_Options.Visuals.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_REAL)  qLastTickAngles = cmd->viewangles;
	else if (bSendPacket && g_Options.Visuals.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_FAKE) qLastTickAngles = cmd->viewangles;


	if (g_Options.Misc.antiuntrusted)
		if (!sanitize_angles(cmd->viewangles))
			return;


	movementfix(oldAngle, cmd);

	if (g_Options.Misc.antiuntrusted)
		clampMovement(cmd);

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum();


	if (Globals::ySwitch) 
		Globals::FakeAngle = cmd->viewangles.y;
	else 
		Globals::RealAngle = cmd->viewangles.y;
	
}
#pragma warning(disable : 4409)
__declspec(naked) void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		PUSH	EBP
		MOV		EBP, ESP
		PUSH	EBX
		LEA		ECX, [ESP]
		PUSH	ECX
		PUSH	active
		PUSH	input_sample_frametime
		PUSH	sequence_number
		CALL	CHLCreateMove
		POP		EBX
		POP		EBP
		RETN	0xC
	}
}
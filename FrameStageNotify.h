#pragma once
#include "SkinChanger.h"
#include "HookIncludes.h"
#include "GloveChanger.h"
#include "LagComp.h"
#include "Resolver.h"
#include "Animfix.h"
typedef void(__stdcall *fsn_t)(ClientFrameStage_t);

void  __stdcall hkFrameStageNotify(ClientFrameStage_t curStage)
{
	C_BaseEntity* pEntity;
	static auto ofunc = hooks::client.get_original<fsn_t>(37);
	if (g_Engine->IsConnected() && g_Engine->IsInGame())
	{	
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
			Resolver::Instance().CallFSN();
			backtracking->Update(g_Globals->tickcount);
		}
		if (curStage == FRAME_RENDER_START)
		{
			AnimFix();
			auto pLocal = g_EntityList->GetClientEntity(g_Engine->GetLocalPlayer());
			auto dwDeadFlag = NetVarManager->GetOffset("DT_CSPlayer", "deadflag"); // int
			if (pLocal)
			{

				*(int*)((uintptr_t)pLocal + 0xA30) = g_Globals->framecount; //we'll skip occlusion checks now
				*(int*)((uintptr_t)pLocal + 0xA28) = 0; //clear occlusion flags

				if (pLocal->IsAlive() && g_Input->m_fCameraInThirdPerson) { *reinterpret_cast<Vector*>(reinterpret_cast<DWORD>(pLocal) + dwDeadFlag + 4) = qLastTickAngles; }
			}	
		}
		
	}
	ofunc(curStage);
}

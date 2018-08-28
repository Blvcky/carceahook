#pragma once

#include "CommonIncludes.h"
#include "SDK.h"
#include <iostream>
#include "Utilities.h"
#include "IViewRenderBeams.h"
#include "memalloc.h"
#include "CBaseClientState.h"
#include <d3d9.h>
#include "network.h"
#include <memory>

extern void InitialiseInterfaces();
extern IBaseClientDLL* g_CHLClient;
extern IVEngineClient* g_Engine;
extern IPanel* g_Panel;
extern C_BaseEntityList* g_EntityList;
extern ISurface* g_Surface;
extern IVDebugOverlay* g_DebugOverlay;
extern IClientMode* g_ClientMode;
extern CGlobalVarsBase* g_Globals;
extern IPrediction *g_Prediction;
extern CMaterialSystem* g_MaterialSystem;
extern CVRenderView* g_RenderView;
extern IVModelRender* g_ModelRender;
extern CModelInfo* g_ModelInfo;
extern IEngineTrace* g_EngineTrace;
extern IPhysicsSurfaceProps* g_PhysProps;
extern ICVar* g_CVar;
extern IVEffects* g_Dlight;
extern IMoveHelper* g_MoveHelper;
extern IGameMovement* g_GameMovement;
extern CInput* g_Input;
extern IGameEventManager2* g_EventManager;
extern C_CSPlayerResource* g_PlayerResource;
extern C_CSGameRules* g_GameRules;
extern IViewRender* g_ViewRender;
extern IGameConsole* g_GameConsole;
extern IMDLCache* g_MdlCache;
extern CHudChat* g_ChatElement;
extern CGlowObjectManager* g_GlowObjManager;
extern IViewRenderBeams* g_pViewRenderBeams;
extern IMemAlloc* g_pMemAlloc;
extern IDirect3DDevice9* g_D3DDevice9;
extern CNetworkStringTableContainer* g_ClientStringTableContainer;
extern CBaseClientState** g_ClientState;

typedef void* (__cdecl* CreateInterface_t)(const char*, int*);
typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);
inline CreateInterfaceFn get_module_factory(HMODULE module)
{
    return reinterpret_cast<CreateInterfaceFn>(GetProcAddress(module, "CreateInterface"));
}
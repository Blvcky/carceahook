

#pragma once
#include "MiscDefinitions.h"
#include "Vector.h"
#include "Utilities.h"
#include <map>




#define TEAM_CS_T 2
#define TEAM_CS_CT 3
#define BONE_USED_BY_HITBOX			0x00000100
#define BONE_USED_BY_ATTACHMENT      0x00000200    // bone (or child) is used by an attachment point
#define MAXSTUDIOBONES		128		// total bones actually used
#define TICK_INTERVAL			(g_Globals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( g_Globals->interval_per_tick *( t ) )
enum class TeamID : int
{
	TEAM_UNASSIGNED,
	TEAM_SPECTATOR,
	TEAM_TERRORIST,
	TEAM_COUNTER_TERRORIST,
};

class IClientRenderable;
class IClientNetworkable;
class IClientUnknown;
class IClientThinkable;
class C_BaseEntity;
class CSWeaponInfo;

typedef unsigned long CBaseHandle;

struct RecvProp;

class DVariant
{
public:
	union
	{
		float m_Float;
		long m_Int;
		char* m_pString;
		void* m_pData;
		float m_Vector[3];
	};
};

class CRecvProxyData
{
public:
	const RecvProp* m_pRecvProp;
	char _pad[4];//csgo ( for l4d keep it commented out :) )
	DVariant m_Value;
	int m_iElement;
	int m_ObjectID;
};

typedef void(*RecvVarProxyFn)(const CRecvProxyData* pData, void* pStruct, void* pOut);

struct RecvTable
{
	RecvProp* m_pProps;
	int m_nProps;
	void* m_pDecoder;
	char* m_pNetTableName;
	bool m_bInitialized;
	bool m_bInMainList;
};

struct RecvProp
{
	char* m_pVarName;
	int m_RecvType;
	int m_Flags;
	int m_StringBufferSize;
	bool m_bInsideArray;
	const void* m_pExtraData;
	RecvProp* m_pArrayProp;
	void* m_ArrayLengthProxy;
	void* m_ProxyFn;
	void* m_DataTableProxyFn;
	RecvTable* m_pDataTable;
	int m_Offset;
	int m_ElementStride;
	int m_nElements;
	const char* m_pParentArrayPropName;
};



typedef IClientNetworkable*	(*CreateClientClassFn)(int entnum, int serialNum);
typedef IClientNetworkable* (*CreateEventFn)();

class ClientClass
{
public:
	CreateClientClassFn		m_pCreateFn;
	CreateEventFn m_pCreateEventFn;
	char			*m_pNetworkName;
	RecvTable		*m_pRecvTable;
	ClientClass		*m_pNext;
	int			m_ClassID;
};

#include "memesets.h"
#include "NetVarManager.h"


struct CHudTexture
{
	char	szShortName[64];	//0x0000
	char	szTextureFile[64];	//0x0040
	bool	bRenderUsingFont;	//0x0080
	bool	bPrecached;			//0x0081
	char	cCharacterInFont;	//0x0082
	BYTE	pad_0x0083;			//0x0083
	int		hFont;				//0x0084
	int		iTextureId;			//0x0088
	float	afTexCoords[4];		//0x008C
	int		iPosX[4];			//0x009C
}; //Size=0x00AC
class CCStrike15ItemDefinition;
class CSWeaponInfo
{
public:
	char _0x0000[20];
	__int32 max_clip;			//0x0014 
	char _0x0018[12];
	__int32 max_reserved_ammo;	//0x0024 
	char _0x0028[96];
	char* hud_name;				//0x0088 
	char* weapon_name;			//0x008C 
	char _0x0090[60];
	__int32 type;				//0x00CC 
	__int32 price;				//0x00D0 
	__int32 reward;				//0x00D4 
	char _0x00D8[20];
	BYTE full_auto;				//0x00EC 
	char _0x00ED[3];
	__int32 damage;				//0x00F0 
	float armor_ratio;			//0x00F4 
	__int32 bullets;			//0x00F8 
	float penetration;			//0x00FC 
	char _0x0100[8];
	float range;				//0x0108 
	float range_modifier;		//0x010C 
	char _0x0110[16];
	BYTE silencer;				//0x0120 
	char _0x0121[15];
	float max_speed;			//0x0130 
	float max_speed_alt;		//0x0134 
	char _0x0138[76];
	__int32 recoil_seed;		//0x0184 
	char _0x0188[32];
};
class CCSWeaponInfo
{
public:
	float GetPen()
	{
		return *(float*)((DWORD)this + (DWORD)0x0f8);
	}

	int GetDamage()
	{
		return *(int*)((DWORD)this + (DWORD)0x0EC);
	}

	int GetMaxClip()
	{
		return *(int*)((DWORD)this + (DWORD)0x014);
	}

	float** GetRecoilTable()
	{
		return *(float***)((DWORD)this + (DWORD)0xA54);
	}

	float GetRange()
	{
		return *(float*)((DWORD)this + (DWORD)0x104);
	}

	float GetRangeModifier()
	{
		return *(float*)((DWORD)this + (DWORD)0x108);
	}

	float GetArmorRatio()
	{
		return *(float*)((DWORD)this + (DWORD)0x0F0);
	}

	float GetStandInacc()
	{
		return *(float*)((DWORD)this + (DWORD)0x9E0);
	}

	float GetJumpInacc()
	{
		return *(float*)((DWORD)this + (DWORD)0x9E8);
	}
};

enum class ClassID 
{
	CAK47 = 1,
	CBaseAnimating = 2,
	CBaseAnimatingOverlay = 3,
	CBaseAttributableItem = 4,
	CBaseButton = 5,
	CBaseCombatCharacter = 6,
	CBaseCombatWeapon = 7,
	CBaseCSGrenade = 8,
	CBaseCSGrenadeProjectile = 9,
	CBaseDoor = 10,
	CBaseEntity = 11,
	CBaseFlex = 12,
	CBaseGrenade = 13,
	CBaseParticleEntity = 14,
	CBasePlayer = 15,
	CBasePropDoor = 16,
	CBaseTeamObjectiveResource = 17,
	CBaseTempEntity = 18,
	CBaseToggle = 19,
	CBaseTrigger = 20,
	CBaseViewModel = 21,
	CBaseVPhysicsTrigger = 22,
	CBaseWeaponWorldModel = 23,
	CBeam = 24,
	CBeamSpotlight = 25,
	CBoneFollower = 26,
	CBreakableProp = 27,
	CBreakableSurface = 28,
	CC4 = 29,
	CCascadeLight = 30,
	CChicken = 31,
	CColorCorrection = 32,
	CColorCorrectionVolume = 33,
	CCSGameRulesProxy = 34,
	CCSPlayer = 35,
	CCSPlayerResource = 36,
	CCSRagdoll = 37,
	CCSTeam = 38,
	CDEagle = 39,
	CDecoyGrenade = 40,
	CDecoyProjectile = 41,
	CDynamicLight = 42,
	CDynamicProp = 43,
	CEconEntity = 44,
	CEconWearable = 45,
	CEmbers = 46,
	CEntityDissolve = 47,
	CEntityFlame = 48,
	CEntityFreezing = 49,
	CEntityParticleTrail = 50,
	CEnvAmbientLight = 51,
	CEnvDetailController = 52,
	CEnvDOFController = 53,
	CEnvParticleScript = 54,
	CEnvProjectedTexture = 55,
	CEnvQuadraticBeam = 56,
	CEnvScreenEffect = 57,
	CEnvScreenOverlay = 58,
	CEnvTonemapController = 59,
	CEnvWind = 60,
	CFEPlayerDecal = 61,
	CFireCrackerBlast = 62,
	CFireSmoke = 63,
	CFireTrail = 64,
	CFish = 65,
	CFlashbang = 66,
	CFogController = 67,
	CFootstepControl = 68,
	CFunc_Dust = 69,
	CFunc_LOD = 70,
	CFuncAreaPortalWindow = 71,
	CFuncBrush = 72,
	CFuncConveyor = 73,
	CFuncLadder = 74,
	CFuncMonitor = 75,
	CFuncMoveLinear = 76,
	CFuncOccluder = 77,
	CFuncReflectiveGlass = 78,
	CFuncRotating = 79,
	CFuncSmokeVolume = 80,
	CFuncTrackTrain = 81,
	CGameRulesProxy = 82,
	CHandleTest = 83,
	CHEGrenade = 84,
	CHostage = 85,
	CHostageCarriableProp = 86,
	CIncendiaryGrenade = 87,
	CInferno = 88,
	CInfoLadderDismount = 89,
	CInfoOverlayAccessor = 90,
	CItem_Healthshot = 91,
	CItemDogtags = 92,
	CKnife = 93,
	CKnifeGG = 94,
	CLightGlow = 95,
	CMaterialModifyControl = 96,
	CMolotovGrenade = 97,
	CMolotovProjectile = 98,
	CMovieDisplay = 99,
	CParticleFire = 100,
	CParticlePerformanceMonitor = 101,
	CParticleSystem = 102,
	CPhysBox = 103,
	CPhysBoxMultiplayer = 104,
	CPhysicsProp = 105,
	CPhysicsPropMultiplayer = 106,
	CPhysMagnet = 107,
	CPlantedC4 = 108,
	CPlasma = 109,
	CPlayerResource = 110,
	CPointCamera = 111,
	CPointCommentaryNode = 112,
	CPointWorldText = 113,
	CPoseController = 114,
	CPostProcessController = 115,
	CPrecipitation = 116,
	CPrecipitationBlocker = 117,
	CPredictedViewModel = 118,
	CProp_Hallucination = 119,
	CPropDoorRotating = 120,
	CPropJeep = 121,
	CPropVehicleDriveable = 122,
	CRagdollManager = 123,
	CRagdollProp = 124,
	CRagdollPropAttached = 125,
	CRopeKeyframe = 126,
	CSCAR17 = 127,
	CSceneEntity = 128,
	CSensorGrenade = 129,
	CSensorGrenadeProjectile = 130,
	CShadowControl = 131,
	CSlideshowDisplay = 132,
	CSmokeGrenade = 133,
	CSmokeGrenadeProjectile = 134,
	CSmokeStack = 135,
	CSpatialEntity = 136,
	CSpotlightEnd = 137,
	CSprite = 138,
	CSpriteOriented = 139,
	CSpriteTrail = 140,
	CStatueProp = 141,
	CSteamJet = 142,
	CSun = 143,
	CSunlightShadowControl = 144,
	CTeam = 145,
	CTeamplayRoundBasedRulesProxy = 146,
	CTEArmorRicochet = 147,
	CTEBaseBeam = 148,
	CTEBeamEntPoint = 149,
	CTEBeamEnts = 150,
	CTEBeamFollow = 151,
	CTEBeamLaser = 152,
	CTEBeamPoints = 153,
	CTEBeamRing = 154,
	CTEBeamRingPoint = 155,
	CTEBeamSpline = 156,
	CTEBloodSprite = 157,
	CTEBloodStream = 158,
	CTEBreakModel = 159,
	CTEBSPDecal = 160,
	CTEBubbles = 161,
	CTEBubbleTrail = 162,
	CTEClientProjectile = 163,
	CTEDecal = 164,
	CTEDust = 165,
	CTEDynamicLight = 166,
	CTEEffectDispatch = 167,
	CTEEnergySplash = 168,
	CTEExplosion = 169,
	CTEFireBullets = 170,
	CTEFizz = 171,
	CTEFootprintDecal = 172,
	CTEFoundryHelpers = 173,
	CTEGaussExplosion = 174,
	CTEGlowSprite = 175,
	CTEImpact = 176,
	CTEKillPlayerAttachments = 177,
	CTELargeFunnel = 178,
	CTEMetalSparks = 179,
	CTEMuzzleFlash = 180,
	CTEParticleSystem = 181,
	CTEPhysicsProp = 182,
	CTEPlantBomb = 183,
	CTEPlayerAnimEvent = 184,
	CTEPlayerDecal = 185,
	CTEProjectedDecal = 186,
	CTERadioIcon = 187,
	CTEShatterSurface = 188,
	CTEShowLine = 189,
	CTesla = 190,
	CTESmoke = 191,
	CTESparks = 192,
	CTESprite = 193,
	CTESpriteSpray = 194,
	CTest_ProxyToggle_Networkable = 194,
	CTestTraceline = 196,
	CTEWorldDecal = 197,
	CTriggerPlayerMovement = 198,
	CTriggerSoundOperator = 199,
	CVGuiScreen = 200,
	CVoteController = 201,
	CWaterBullet = 202,
	CWaterLODControl = 203,
	CWeaponAug = 204,
	CWeaponAWP = 205,
	CWeaponBaseItem = 206,
	CWeaponBizon = 207,
	CWeaponCSBase = 208,
	CWeaponCSBaseGun = 209,
	CWeaponCycler = 210,
	CWeaponElite = 211,
	CWeaponFamas = 212,
	CWeaponFiveSeven = 213,
	CWeaponG3SG1 = 214,
	CWeaponGalil = 215,
	CWeaponGalilAR = 216,
	CWeaponGlock = 217,
	CWeaponHKP2000 = 218,
	CWeaponM249 = 219,
	CWeaponM3 = 220,
	CWeaponM4A1 = 221,
	CWeaponMAC10 = 222,
	CWeaponMag7 = 223,
	CWeaponMP5Navy = 224,
	CWeaponMP7 = 225,
	CWeaponMP9 = 226,
	CWeaponNegev = 227,
	CWeaponNOVA = 228,
	CWeaponP228 = 229,
	CWeaponP250 = 230,
	CWeaponP90 = 231,
	CWeaponSawedoff = 232,
	CWeaponSCAR20 = 233,
	CWeaponScout = 234,
	CWeaponSG550 = 235,
	CWeaponSG552 = 236,
	CWeaponSG556 = 237,
	CWeaponSSG08 = 238,
	CWeaponTaser = 239,
	CWeaponTec9 = 240,
	CWeaponTMP = 241,
	CWeaponUMP45 = 242,
	CWeaponUSP = 243,
	CWeaponXM1014 = 244,
	CWorld = 245,
	DustTrail = 246,
	MovieExplosion = 247,
	ParticleSmokeGrenade = 248,
	RocketTrail = 249,
	SmokeTrail = 250,
	SporeExplosion = 251,
	SporeTrail = 252
};

enum CSGOHitboxID : int
{
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_BELLY,
	HITBOX_THORAX,
	HITBOX_LOWER_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	HITBOX_MAX
};
enum class ItemDefinitionIndexx : short
{
	ITEM_NONE = 0,
	WEAPON_DEAGLE = 1,
	WEAPON_DUALS = 2,
	WEAPON_FIVE7 = 3,
	WEAPON_GLOCK = 4,
	WEAPON_AK47 = 7,
	WEAPON_AUG = 8,
	WEAPON_AWP = 9,
	WEAPON_FAMAS = 10,
	WEAPON_G3SG1 = 11,
	WEAPON_GALIL = 13,
	WEAPON_M249 = 14,
	WEAPON_M4A1 = 16,
	WEAPON_MAC10 = 17,
	WEAPON_P90 = 19,
	WEAPON_UMP45 = 24,
	WEAPON_XM1014 = 25,
	WEAPON_BIZON = 26,
	WEAPON_MAG7 = 27,
	WEAPON_NEGEV = 28,
	WEAPON_SAWEDOFF = 29,
	WEAPON_TEC9 = 30,
	WEAPON_TASER = 31,
	WEAPON_P2000 = 32,
	WEAPON_MP7 = 33,
	WEAPON_MP9 = 34,
	WEAPON_NOVA = 35,
	WEAPON_P250 = 36,
	WEAPON_SCAR20 = 38,
	WEAPON_SG553 = 39,
	WEAPON_SCOUT = 40,
	WEAPON_KNIFE_T = 42,
	WEAPON_FLASH = 43,
	WEAPON_HE = 44,
	WEAPON_SMOKE = 45,
	WEAPON_MOLOTOV = 46,
	WEAPON_DECOY = 47,
	WEAPON_INC = 48,
	WEAPON_C4 = 49,
	WEAPON_KNIFE_CT = 59,
	WEAPON_M4A1S = 60,
	WEAPON_USPS = 61,
	WEAPON_CZ75 = 63,
	WEAPON_REVOLVER = 64,
	WEAPON_KNIFE_BAYONET = 500,
	WEAPON_KNIFE_FLIP = 505,
	WEAPON_KNIFE_GUT = 506,
	WEAPON_KNIFE_KARAMBIT = 507,
	WEAPON_KNIFE_M9BAYONET = 508,
	WEAPON_KNIFE_HUNTSMAN = 509,
	WEAPON_KNIFE_FALCHION = 512,
	WEAPON_KNIFE_BOWIE = 514,
	WEAPON_KNIFE_BUTTERFLY = 515,
	WEAPON_KNIFE_DAGGER = 516,
	GLOVE_STUDDED_BLOODHOUND = 5027,
	GLOVE_T_SIDE = 5028,
	GLOVE_CT_SIDE = 5029,
	GLOVE_SPORTY = 5030,
	GLOVE_SLICK = 5031,
	GLOVE_LEATHER_WRAP = 5032,
	GLOVE_MOTORCYCLE = 5033,
	GLOVE_SPECIALIST = 5034,
	GLOVE_HYDRA = 5035,
	WEAPON_MAX
};

struct DefItem_t
{
	DefItem_t(const char* displayName,
		const char* entityName,
		const char* entityModel,
		const char* killIcon = nullptr)
	{
		this->displayName = displayName;
		this->entityName = entityName;
		this->entityModel = entityModel;
		this->killIcon = killIcon;
	}

	const char* displayName = nullptr;
	const char* entityName = nullptr;
	const char* entityModel = nullptr;
	const char* killIcon = nullptr;
};

const std::map<ItemDefinitionIndexx, DefItem_t> ItemDefinitionIndexMap = {
	{ ItemDefinitionIndexx::ITEM_NONE,{ "<-Default->", "DEFAULT", "", "" } },
	{ ItemDefinitionIndexx::WEAPON_DEAGLE,{ "#SFUI_WPNHUD_DesertEagle", "WEAPON_DEAGLE", "models/weapons/v_pist_deagle.mdl", "deagle" } },
	{ ItemDefinitionIndexx::WEAPON_DUALS,{ "#SFUI_WPNHUD_Elites", "WEAPON_DUALS", "models/weapons/v_pist_elite.mdl", "elite" } },
	{ ItemDefinitionIndexx::WEAPON_FIVE7,{ "#SFUI_WPNHUD_FiveSeven", "WEAPON_FIVESEVEN", "models/weapons/v_pist_fiveseven.mdl", "fiveseven" } },
	{ ItemDefinitionIndexx::WEAPON_GLOCK,{ "#SFUI_WPNHUD_Glock18", "WEAPON_GLOCK", "models/weapons/v_pist_glock18.mdl", "glock" } },
	{ ItemDefinitionIndexx::WEAPON_AK47,{ "#SFUI_WPNHUD_AK47", "WEAPON_AK47", "models/weapons/v_rif_ak47.mdl", "ak47" } },
	{ ItemDefinitionIndexx::WEAPON_AUG,{ "#SFUI_WPNHUD_Aug", "WEAPON_AUG", "models/weapons/v_rif_aug.mdl", "aug" } },
	{ ItemDefinitionIndexx::WEAPON_AWP,{ "#SFUI_WPNHUD_AWP", "WEAPON_AWP", "models/weapons/v_snip_awp.mdl", "awp" } },
	{ ItemDefinitionIndexx::WEAPON_FAMAS,{ "#SFUI_WPNHUD_Famas", "WEAPON_FAMAS", "models/weapons/v_rif_famas.mdl", "famas" } },
	{ ItemDefinitionIndexx::WEAPON_G3SG1,{ "#SFUI_WPNHUD_G3SG1", "WEAPON_G3SG1", "models/weapons/v_snip_g3sg1.mdl", "g3sg1" } },
	{ ItemDefinitionIndexx::WEAPON_GALIL,{ "#SFUI_WPNHUD_GalilAR", "WEAPON_GALIL", "models/weapons/v_rif_galilar.mdl", "galilar" } },
	{ ItemDefinitionIndexx::WEAPON_M249,{ "#SFUI_WPNHUD_M249", "WEAPON_M249", "models/weapons/v_mach_m249para.mdl", "m249" } },
	{ ItemDefinitionIndexx::WEAPON_M4A1,{ "#SFUI_WPNHUD_M4A1", "WEAPON_M4A1", "models/weapons/v_rif_m4a1.mdl", "m4a1" } },
	{ ItemDefinitionIndexx::WEAPON_MAC10,{ "#SFUI_WPNHUD_MAC10", "WEAPON_MAC10", "models/weapons/v_smg_mac10.mdl", "mac10" } },
	{ ItemDefinitionIndexx::WEAPON_P90,{ "#SFUI_WPNHUD_P90", "WEAPON_P90", "models/weapons/v_smg_p90.mdl", "p90" } },
	{ ItemDefinitionIndexx::WEAPON_UMP45,{ "#SFUI_WPNHUD_UMP45", "WEAPON_UMP45", "models/weapons/v_smg_ump45.mdl", "ump45" } },
	{ ItemDefinitionIndexx::WEAPON_XM1014,{ "#SFUI_WPNHUD_xm1014", "WEAPON_XM1014", "models/weapons/v_shot_xm1014.mdl", "xm1014" } },
	{ ItemDefinitionIndexx::WEAPON_BIZON,{ "#SFUI_WPNHUD_Bizon", "WEAPON_BIZON", "models/weapons/v_smg_bizon.mdl", "bizon" } },
	{ ItemDefinitionIndexx::WEAPON_MAG7,{ "#SFUI_WPNHUD_Mag7", "WEAPON_MAG7", "models/weapons/v_shot_mag7.mdl", "mag7" } },
	{ ItemDefinitionIndexx::WEAPON_NEGEV,{ "#SFUI_WPNHUD_Negev", "WEAPON_NEGEV", "models/weapons/v_mach_negev.mdl", "negev" } },
	{ ItemDefinitionIndexx::WEAPON_SAWEDOFF,{ "#SFUI_WPNHUD_Sawedoff", "WEAPON_SAWEDOFF", "models/weapons/v_shot_sawedoff.mdl", "sawedoff" } },
	{ ItemDefinitionIndexx::WEAPON_TEC9,{ "#SFUI_WPNHUD_Tec9", "WEAPON_TEC9", "models/weapons/v_pist_tec9.mdl", "tec9" } },
	{ ItemDefinitionIndexx::WEAPON_TASER,{ "#SFUI_WPNHUD_Taser", "WEAPON_ZEUS", "models/weapons/v_eq_taser.mdl", "taser" } },
	{ ItemDefinitionIndexx::WEAPON_P2000,{ "#SFUI_WPNHUD_HKP2000", "WEAPON_P2000", "models/weapons/v_pist_hkp2000.mdl", "hkp2000" } },
	{ ItemDefinitionIndexx::WEAPON_MP7,{ "#SFUI_WPNHUD_MP7", "WEAPON_MP7", "models/weapons/v_smg_mp7.mdl", "mp7" } },
	{ ItemDefinitionIndexx::WEAPON_MP9,{ "#SFUI_WPNHUD_MP9", "WEAPON_MP9", "models/weapons/v_smg_mp9.mdl", "mp9" } },
	{ ItemDefinitionIndexx::WEAPON_NOVA,{ "#SFUI_WPNHUD_Nova", "WEAPON_NOVA", "models/weapons/v_shot_nova.mdl", "nova" } },
	{ ItemDefinitionIndexx::WEAPON_P250,{ "#SFUI_WPNHUD_P250", "WEAPON_P250", "models/weapons/v_pist_p250.mdl", "p250" } },
	{ ItemDefinitionIndexx::WEAPON_SCAR20,{ "#SFUI_WPNHUD_SCAR20", "WEAPON_SCAR20", "models/weapons/v_snip_scar20.mdl", "scar20" } },
	{ ItemDefinitionIndexx::WEAPON_SG553,{ "#SFUI_WPNHUD_SG556", "WEAPON_SG553", "models/weapons/v_rif_sg556.mdl", "sg556" } },
	{ ItemDefinitionIndexx::WEAPON_SCOUT,{ "#SFUI_WPNHUD_SSG08", "WEAPON_SCOUT", "models/weapons/v_snip_ssg08.mdl", "ssg08" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_CT,{ "#SFUI_WPNHUD_Knife", "WEAPON_KNIFE", "models/weapons/v_knife_default_ct.mdl", "knife_default_ct" } },
	{ ItemDefinitionIndexx::WEAPON_FLASH,{ "#SFUI_WPNHUD_FLASHBANG", "WEAPON_FLASHBANG", "models/weapons/v_eq_flashbang.mdl", "flashbang" } },
	{ ItemDefinitionIndexx::WEAPON_HE,{ "#SFUI_WPNHUD_HE_Grenade", "WEAPON_HEGRENADE", "models/weapons/v_eq_fraggrenade.mdl", "hegrenade" } },
	{ ItemDefinitionIndexx::WEAPON_SMOKE,{ "#SFUI_WPNHUD_Smoke_Grenade", "WEAPON_SMOKEGRENADE", "models/weapons/v_eq_smokegrenade.mdl", "smokegrenade" } },
	{ ItemDefinitionIndexx::WEAPON_MOLOTOV,{ "#SFUI_WPNHUD_MOLOTOV", "WEAPON_MOLOTOV", "models/weapons/v_eq_molotov.mdl", "inferno" } },
	{ ItemDefinitionIndexx::WEAPON_DECOY,{ "#SFUI_WPNHUD_DECOY", "WEAPON_DECOY", "models/weapons/v_eq_decoy.mdl", "decoy" } },
	{ ItemDefinitionIndexx::WEAPON_INC,{ "#SFUI_WPNHUD_IncGrenade", "WEAPON_INCGRENADE", "models/weapons/v_eq_incendiarygrenade.mdl", "inferno" } },
	{ ItemDefinitionIndexx::WEAPON_C4,{ "#SFUI_WPNHUD_C4", "WEAPON_C4", "models/weapons/v_ied.mdl" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_T,{ "#SFUI_WPNHUD_Knife", "WEAPON_KNIFE_T", "models/weapons/v_knife_default_t.mdl", "knife_t" } },
	{ ItemDefinitionIndexx::WEAPON_M4A1S,{ "#SFUI_WPNHUD_M4_SILENCER", "WEAPON_M4A1S", "models/weapons/v_rif_m4a1_s.mdl", "m4a1_silencer" } },
	{ ItemDefinitionIndexx::WEAPON_USPS,{ "#SFUI_WPNHUD_USP_SILENCER", "WEAPON_USP", "models/weapons/v_pist_223.mdl", "usp_silencer" } },
	{ ItemDefinitionIndexx::WEAPON_CZ75,{ "#SFUI_WPNHUD_CZ75", "WEAPON_CZ75", "models/weapons/v_pist_cz_75.mdl", "cz75a" } },
	{ ItemDefinitionIndexx::WEAPON_REVOLVER,{ "#SFUI_WPNHUD_REVOLVER", "WEAPON_REVOLVER", "models/weapons/v_pist_revolver.mdl", "revolver" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_BAYONET,{ "#SFUI_WPNHUD_KnifeBayonet", "WEAPON_KNIFE_BAYONET", "models/weapons/v_knife_bayonet.mdl", "bayonet" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_FLIP,{ "#SFUI_WPNHUD_KnifeFlip", "WEAPON_KNIFE_FLIP", "models/weapons/v_knife_flip.mdl", "knife_flip" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_GUT,{ "#SFUI_WPNHUD_KnifeGut", "WEAPON_KNIFE_GUT", "models/weapons/v_knife_gut.mdl", "knife_gut" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_KARAMBIT,{ "#SFUI_WPNHUD_KnifeKaram", "WEAPON_KNIFE_KARAMBIT", "models/weapons/v_knife_karam.mdl", "knife_karambit" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_M9BAYONET,{ "#SFUI_WPNHUD_KnifeM9", "WEAPON_KNIFE_M9BAYONET", "models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_HUNTSMAN,{ "#SFUI_WPNHUD_KnifeTactical", "WEAPON_KNIFE_HUNTSMAN", "models/weapons/v_knife_tactical.mdl", "knife_tactical" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_FALCHION,{ "#SFUI_WPNHUD_knife_falchion_advanced", "WEAPON_KNIFE_FALCHION", "models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_BOWIE,{ "#SFUI_WPNHUD_knife_survival_bowie", "WEAPON_KNIFE_BOWIE", "models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_BUTTERFLY,{ "#SFUI_WPNHUD_Knife_Butterfly", "WEAPON_KNIFE_BUTTERFLY", "models/weapons/v_knife_butterfly.mdl", "knife_butterfly" } },
	{ ItemDefinitionIndexx::WEAPON_KNIFE_DAGGER,{ "#SFUI_WPNHUD_knife_push", "WEAPON_KNIFE_DAGGER", "models/weapons/v_knife_push.mdl", "knife_push" } },
	{ ItemDefinitionIndexx::GLOVE_STUDDED_BLOODHOUND,{ "#CSGO_Wearable_t_studdedgloves", "studded_bloodhound_gloves", "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_T_SIDE,{ "#CSGO_Wearable_t_defaultgloves", "t_gloves", "models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_CT_SIDE,{ "#CSGO_Wearable_ct_defaultgloves", "ct_gloves", "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_SPORTY,{ "#CSGO_Wearable_v_sporty_glove", "sporty_gloves", "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_SLICK,{ "#CSGO_Wearable_v_slick_glove", "slick_gloves", "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_LEATHER_WRAP,{ "#CSGO_Wearable_v_leather_handwrap", "leather_handwraps", "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_MOTORCYCLE,{ "#CSGO_Wearable_v_motorcycle_glove", "motorcycle_gloves", "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl" } },
	{ ItemDefinitionIndexx::GLOVE_SPECIALIST,{ "#CSGO_Wearable_v_specialist_glove", "specialist_gloves", "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl" } }
};

class CBaseAttributableItem
{
public:

	short* ItemDefinitionIndex()
	{
		return (short*)((uintptr_t)this + 0x1EA); // https://www.unknowncheats.me/forum/2180327-post252.html
	}
	short GetItemDefinitionIndex()
	{
		return *(short*)((uintptr_t)this + 0x1EA); // https://www.unknowncheats.me/forum/2180327-post252.html
	}
	int* ItemIDHigh()
	{
		return (int*)((uintptr_t)this + 0x1F0);
	}
	int* ItemIDLow()
	{
		return (int*)((uintptr_t)this + 0x1F4);
	}

	int* FallbackPaintKit()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_nFallbackPaintKit);
	}

	int* FallbackSeed()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_nFallbackSeed);
	}

	float* FallbackWear()
	{
		return (float*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_flFallbackWear);
	}

	int* FallbackStatTrak()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_nFallbackStatTrak);
	}

	int* OwnerXuidLow()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_OriginalOwnerXuidLow);
	}
	int* OwnerXuidHigh()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_OriginalOwnerXuidHigh);
	}
	char* GetCustomName()
	{
		return (char*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_szCustomName);
	}
	int* GetEntityQuality() {
		// DT_BaseAttributableItem -> m_AttributeManager -> m_Item -> m_iEntityQuality
		return (int*)((DWORD)this + 0x2D70 + 0x40 + 0x1DC);
	}

};
class AnimLayer
{
public:
	char  pad_0000[20];
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void *m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class AttributeContainer
{
public:

	CBaseAttributableItem* m_Item()
	{
		return (CBaseAttributableItem*)((uintptr_t)this + 0x40);
	}
};

typedef CSWeaponInfo& (__thiscall* GetCSWpnDataFn)(void*);

class CBaseViewModel {
public:
	inline int GetModelIndex() {

		return *(int*)((DWORD)this + offsetz.DT_BaseViewModel.m_nModelIndex);
	}

	inline void SetModelIndex(int nModelIndex) {
		*(int*)((DWORD)this + offsetz.DT_BaseViewModel.m_nModelIndex) = nModelIndex;
	}

	inline DWORD GetOwner() {

		return *(PDWORD)((DWORD)this + offsetz.DT_BaseViewModel.m_hOwner);
	}

	inline DWORD GetWeapon() {

		return *(PDWORD)((DWORD)this + offsetz.DT_BaseViewModel.m_hWeapon);
	}
};

enum DataUpdateType_t
{
	DATA_UPDATE_CREATED = 0,
	DATA_UPDATE_DATATABLE_CHANGED,
};

class CBaseCombatWeapon : public CBaseAttributableItem
{
public:

	float m_flPostponeFireReadyTime()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_WeaponCSBase.m_flPostponeFireReadyTime);
	}


	HANDLE m_hWeaponWorldModel()
	{
		return *(HANDLE*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_hWeaponWorldModel);
	}
	int ammo()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_iClip1);
	}
	int ammo2()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_iReserve);
	}
	int* AccountID()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseAttributableItem.m_iAccountID);
	}
	float GetNextPrimaryAttack()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_flNextPrimaryAttack);
	}

	int GetOwnerHandle() //checkownerent
	{
	return *(int*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_hOwner);
	}

	Vector GetOrigin() // GetVecOrigin
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BaseEntity.m_vecOrigin);
	}


	int* ViewModelIndex()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_iViewModelIndex);
	}

	int* WorldModelIndex()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_iWorldModelIndex);
	}

	int* ModelIndex()
	{
		return (int*)((uintptr_t)this + offsetz.DT_BaseViewModel.m_nModelIndex);
	}

	//new
	void SetModelIndex(int model)
	{
		typedef void(__thiscall* oSetModelIndex)(PVOID, int);
		return call_vfunc< oSetModelIndex >(this, 75)(this, model);
	}

	AttributeContainer* m_AttributeManager()
	{
		return (AttributeContainer*)((uintptr_t)this + 0x2D70);
	}


	float GetInaccuracy() {
		typedef float(__thiscall* oInaccuracy)(PVOID);
		return call_vfunc< oInaccuracy >(this, 467)
			(this);
	}

	//getcone
	float GetSpread() {
		if (!this) return 0;
		typedef float(__thiscall* tFunc)(void*);
		return call_vfunc<tFunc>(this, 436)(this);
	}


	void UpdateAccuracyPenalty(CBaseCombatWeapon *pWeapon) {
		DWORD dwUpdateVMT = (*reinterpret_cast< PDWORD_PTR* >(pWeapon))[468];
		return ((void(__thiscall*)(CBaseCombatWeapon*)) dwUpdateVMT)(pWeapon);
	}


	CSWeaponInfo* GetCSWpnData()
	{
		if (!this)
			return nullptr;
		typedef CSWeaponInfo*(__thiscall* tGetCSWpnData)(void*);
		return call_vfunc<tGetCSWpnData>(this, 444)(this);
	}

	bool IsInReload()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_bInReload);
	}

	void Release()
	{
		PVOID pNetworkable = (PVOID)((DWORD)(this) + 0x8);
		typedef void(__thiscall* OriginalFn)(PVOID);
		return call_vfunc<OriginalFn>(pNetworkable, 1)(pNetworkable);
	}
	void SetDestroyedOnRecreateEntities(void)
	{
		PVOID pNetworkable = (PVOID)((DWORD)(this) + 0x8);
		typedef void(__thiscall* OriginalFn)(PVOID);
		return call_vfunc<OriginalFn>(pNetworkable, 13)(pNetworkable);
	}

	bool IsKnife()
	{
		short iWeaponID = this->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		return (iWeaponID == (short)ItemDefinitionIndexx::WEAPON_KNIFE_T || iWeaponID == (short)ItemDefinitionIndexx::WEAPON_KNIFE_CT
			|| iWeaponID == 59 || iWeaponID == 41
			|| iWeaponID == 500 || iWeaponID == 505 || iWeaponID == 506
			|| iWeaponID == 507 || iWeaponID == 508 || iWeaponID == 509
			|| iWeaponID == 515);
	}
	bool IsGrenade()
	{
		short iWeaponID = this->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		return (iWeaponID == (short)ItemDefinitionIndexx::WEAPON_HE
			|| (short)ItemDefinitionIndexx::WEAPON_INC
			|| (short)ItemDefinitionIndexx::WEAPON_DECOY
			|| (short)ItemDefinitionIndexx::WEAPON_SMOKE
			|| (short)ItemDefinitionIndexx::WEAPON_FLASH);
	}
	bool AWall()
	{
		short iWeaponID = this->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		return (iWeaponID ==
			(short)ItemDefinitionIndexx::WEAPON_AK47
			|| (short)ItemDefinitionIndexx::WEAPON_AUG
			|| (short)ItemDefinitionIndexx::WEAPON_REVOLVER
			|| (short)ItemDefinitionIndexx::WEAPON_BIZON
			|| (short)ItemDefinitionIndexx::WEAPON_CZ75
			|| (short)ItemDefinitionIndexx::WEAPON_DEAGLE
			|| (short)ItemDefinitionIndexx::WEAPON_DUALS
			|| (short)ItemDefinitionIndexx::WEAPON_FAMAS
			|| (short)ItemDefinitionIndexx::WEAPON_FIVE7
			|| (short)ItemDefinitionIndexx::WEAPON_GALIL
			|| (short)ItemDefinitionIndexx::WEAPON_GLOCK
			|| (short)ItemDefinitionIndexx::WEAPON_M249
			|| (short)ItemDefinitionIndexx::WEAPON_M4A1
			|| (short)ItemDefinitionIndexx::WEAPON_MAC10
			|| (short)ItemDefinitionIndexx::WEAPON_MAG7
			|| (short)ItemDefinitionIndexx::WEAPON_MP7
			|| (short)ItemDefinitionIndexx::WEAPON_MP9
			|| (short)ItemDefinitionIndexx::WEAPON_NOVA
			|| (short)ItemDefinitionIndexx::WEAPON_NEGEV
			|| (short)ItemDefinitionIndexx::WEAPON_P250
			|| (short)ItemDefinitionIndexx::WEAPON_P90
			|| (short)ItemDefinitionIndexx::WEAPON_SAWEDOFF
			|| (short)ItemDefinitionIndexx::WEAPON_SG553
			|| (short)ItemDefinitionIndexx::WEAPON_TEC9
			|| (short)ItemDefinitionIndexx::WEAPON_P2000
			|| (short)ItemDefinitionIndexx::WEAPON_USPS
			|| (short)ItemDefinitionIndexx::WEAPON_UMP45
			|| (short)ItemDefinitionIndexx::WEAPON_XM1014
			|| (short)ItemDefinitionIndexx::WEAPON_TASER
			|| (short)ItemDefinitionIndexx::WEAPON_M4A1S);
	}
	bool IsValidWeapon()
	{
		short iWeaponID = this->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		return (iWeaponID ==
			(short)ItemDefinitionIndexx::WEAPON_AK47
			|| (short)ItemDefinitionIndexx::WEAPON_AUG
			|| (short)ItemDefinitionIndexx::WEAPON_REVOLVER
			|| (short)ItemDefinitionIndexx::WEAPON_AWP
			|| (short)ItemDefinitionIndexx::WEAPON_BIZON
			|| (short)ItemDefinitionIndexx::WEAPON_CZ75
			|| (short)ItemDefinitionIndexx::WEAPON_DEAGLE
			|| (short)ItemDefinitionIndexx::WEAPON_DUALS
			|| (short)ItemDefinitionIndexx::WEAPON_FAMAS
			|| (short)ItemDefinitionIndexx::WEAPON_FIVE7
			|| (short)ItemDefinitionIndexx::WEAPON_G3SG1
			|| (short)ItemDefinitionIndexx::WEAPON_GALIL
			|| (short)ItemDefinitionIndexx::WEAPON_GLOCK
			|| (short)ItemDefinitionIndexx::WEAPON_SCOUT
			|| (short)ItemDefinitionIndexx::WEAPON_M249
			|| (short)ItemDefinitionIndexx::WEAPON_M4A1
			|| (short)ItemDefinitionIndexx::WEAPON_MAC10
			|| (short)ItemDefinitionIndexx::WEAPON_MAG7
			|| (short)ItemDefinitionIndexx::WEAPON_MP7
			|| (short)ItemDefinitionIndexx::WEAPON_MP9
			|| (short)ItemDefinitionIndexx::WEAPON_NOVA
			|| (short)ItemDefinitionIndexx::WEAPON_NEGEV
			|| (short)ItemDefinitionIndexx::WEAPON_P250
			|| (short)ItemDefinitionIndexx::WEAPON_P90
			|| (short)ItemDefinitionIndexx::WEAPON_SAWEDOFF
			|| (short)ItemDefinitionIndexx::WEAPON_SCAR20
			|| (short)ItemDefinitionIndexx::WEAPON_SG553
			|| (short)ItemDefinitionIndexx::WEAPON_TEC9
			|| (short)ItemDefinitionIndexx::WEAPON_P2000
			|| (short)ItemDefinitionIndexx::WEAPON_USPS
			|| (short)ItemDefinitionIndexx::WEAPON_UMP45
			|| (short)ItemDefinitionIndexx::WEAPON_XM1014
			|| (short)ItemDefinitionIndexx::WEAPON_TASER
			|| (short)ItemDefinitionIndexx::WEAPON_M4A1S);
	}
	char* GetGunName()
	{
		short WeaponId = this->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		if (this->IsKnife())
			return "Knife";
		switch (WeaponId)
		{
		case (short)ItemDefinitionIndexx::WEAPON_AK47:
			return "AK47";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_AUG:
			return "AUG";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_REVOLVER:
			return "Revolver";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_AWP:
			return "AWP";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_BIZON:
			return "Bizon";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_C4:
			return "Bomb";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_CZ75:
			return "CZ75";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_DEAGLE:
			return "Deagle";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_DECOY:
			return "Decoy";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_DUALS:
			return "Elites";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_FAMAS:
			return "Famas";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_FIVE7:
			return "FiveSeven";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_FLASH:
			return "Flash";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_G3SG1:
			return "G3SG1";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_GALIL:
			return "Galil";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_GLOCK:
			return "Glock";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_INC:
			return "Molotov";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MOLOTOV:
			return "Molotov";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SCOUT:
			return "Scout";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_HE:
			return "Grenade";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_M249:
			return "M249";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_M4A1:
			return "M4A1";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MAC10:
			return "MAC10";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MAG7:
			return "MAG7";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MP7:
			return "MP7";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MP9:
			return "MP9";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_NOVA:
			return "Nova";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_NEGEV:
			return "Negev";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_P250:
			return "P250";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_P90:
			return "P90";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SAWEDOFF:
			return "SawedOff";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SCAR20:
			return "SCAR20";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SMOKE:
			return "Smoke";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SG553:
			return "SG556";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_TEC9:
			return "TEC9";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_P2000:
			return "P2000";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_USPS:
			return "USP";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_UMP45:
			return "UMP";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_XM1014:
			return "XM1014";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_TASER:
			return "Zeus";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_M4A1S:
			return "M4A1-S";
		}
		return "";
	}

	char* GetGunIcon()
	{
		short WeaponId = this->m_AttributeManager()->m_Item()->GetItemDefinitionIndex();
		if (this->IsKnife())
			return "1";
		switch (WeaponId)
		{
		case (short)ItemDefinitionIndexx::WEAPON_AK47:
			return "W";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_AUG:
			return "U";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_REVOLVER:
			return "J";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_AWP:
			return "Z";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_BIZON:
			return "M";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_C4:
			return "o";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_CZ75:
			return "I";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_DEAGLE:
			return "A";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_DECOY:
			return "m";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_DUALS:
			return "B";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_FAMAS:
			return "R";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_FIVE7:
			return "C";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_FLASH:
			return "i";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_G3SG1:
			return "X";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_GALIL:
			return "Q";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_GLOCK:
			return "D";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_INC:
			return "n";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MOLOTOV:
			return "l";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SCOUT:
			return "a";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_HE:
			return "j";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_M249:
			return "g";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_M4A1:
			return "S";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MAC10:
			return "K";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MAG7:
			return "d";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MP7:
			return "N";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_MP9:
			return "O";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_NOVA:
			return "e";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_NEGEV:
			return "f";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_P250:
			return "F";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_P90:
			return "P";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SAWEDOFF:
			return "c";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SCAR20:
			return "Y";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SMOKE:
			return "k";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_SG553:
			return "V";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_TEC9:
			return "H";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_P2000:
			return "E";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_USPS:
			return "G";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_UMP45:
			return "L";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_XM1014:
			return "b";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_TASER:
			return "h";
			break;
		case (short)ItemDefinitionIndexx::WEAPON_M4A1S:
			return "T";
		}
		return "";
	}

};

class CGloves : public CBaseCombatWeapon {
public:

	void PreDataUpdate(int updateType)
	{
		PVOID pNetworkable = (PVOID)((DWORD)(this) + 0x8);
		typedef void(__thiscall* OriginalFn)(PVOID, int);
		return call_vfunc<OriginalFn>(pNetworkable, 6)(pNetworkable, updateType);
	}

	void SetGloveModelIndex(int modelIndex)
	{
		typedef void(__thiscall* OriginalFn)(PVOID, int);
		call_vfunc<OriginalFn>(this, 75)(this, modelIndex);
	}
};





class CCSGrenade
{
public:

	float GetThrowTime()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_BaseCSGrenade.m_fThrowTime);
	}
};

class CLocalPlayerExclusive
{
public:

	Vector GetViewPunchAngle()
	{
		return *(Vector*)((uintptr_t)this + 0x64);
	}
	Vector GetAimPunchAngle()
	{
		return *(Vector*)((uintptr_t)this + 0x70);
	}
	Vector GetAimPunchAngleVel()
	{
		return *(Vector*)((uintptr_t)this + 0x7C);
	}
};

class CollisionProperty
{
public:

	Vector GetMins()
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BaseEntity.m_vecMins);
	}
	Vector GetMaxs()
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BaseEntity.m_vecMaxs);
	}

	unsigned char GetSolidType()
	{
		return *(unsigned char*)((uintptr_t)this + offsetz.DT_BaseEntity.m_nSolidType);
	}

	unsigned short GetSolidFlags()
	{
		return *(unsigned short*)((uintptr_t)this + offsetz.DT_BaseEntity.m_usSolidFlags);
	}

	int GetSurroundsType()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BaseEntity.m_nSurroundType);
	}


	bool IsSolid()
	{
		return (GetSolidType() != SOLID_NONE) && ((GetSolidFlags() & FSOLID_NOT_SOLID) == 0);
	}
};

class IClientRenderable
{
public:
	//virtual void*					GetIClientUnknown() = 0;
	virtual Vector const&			GetRenderOrigin(void) = 0;
	virtual Vector const&			GetRenderAngles(void) = 0;
	virtual bool					ShouldDraw(void) = 0;
	virtual bool					IsTransparent(void) = 0;
	virtual bool					UsesPowerOfTwoFrameBufferTexture() = 0;
	virtual bool					UsesFullFrameBufferTexture() = 0;
	virtual void					GetShadowHandle() const = 0;
	virtual void*					RenderHandle() = 0;
	virtual const model_t*			GetModel() const = 0;
	virtual int						DrawModel(int flags) = 0;
	virtual int                     DrawModel2(int flags, const int /*RenderableInstance_t*/ &instance) = 0;
	virtual int						GetBody() = 0;
	virtual void					ComputeFxBlend() = 0;

	bool SetupBones(matrix3x4 *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
	{

		typedef bool(__thiscall* oSetupBones)(PVOID, matrix3x4*, int, int, float);
		return call_vfunc< oSetupBones>(this, 13)(this, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	}
};


class IClientNetworkable
{
public:
	virtual IClientUnknown*	GetIClientUnknown() = 0;
	virtual void			Release() = 0;
	virtual ClientClass*	GetClientClass() = 0;// FOR NETVARS FIND YOURSELF ClientClass* stuffs
	virtual void			NotifyShouldTransmit( /* ShouldTransmitState_t state*/) = 0;
	virtual void			OnPreDataChanged(DataUpdateType_t updateType) = 0;
	virtual void			OnDataChanged(DataUpdateType_t updateType) = 0;
	virtual void			PreDataUpdate(DataUpdateType_t updateType) = 0;
	virtual void			PostDataUpdate(DataUpdateType_t updateType) = 0;
	virtual void			unknown();
	virtual bool			IsDormant(void) = 0;
	virtual int				GetIndex(void) const = 0;
	virtual void			ReceiveMessage(int classID /*, bf_read &msg*/) = 0;
	virtual void*			GetDataTableBasePtr() = 0;
	virtual void			SetDestroyedOnRecreateEntities(void) = 0;
};
class ICollideable
{
public:
	virtual void pad0();
	virtual const Vector& OBBMins() const;
	virtual const Vector& OBBMaxs() const;
};

class IClientUnknown
{
public:
	ICollideable* GetCollideable()
	{
		return (ICollideable*)((DWORD)this + 0x318);
	}
	virtual IClientNetworkable*	GetClientNetworkable() = 0;
	virtual IClientRenderable*	GetClientRenderable() = 0;
	virtual C_BaseEntity*		GetIClientEntity() = 0;
	virtual C_BaseEntity*		GetBaseEntity() = 0;
	virtual IClientThinkable*	GetClientThinkable() = 0;
};


class IClientThinkable
{
public:
	virtual IClientUnknown*		GetIClientUnknown() = 0;
	virtual void				ClientThink() = 0;
	virtual void*				GetThinkHandle() = 0;
	virtual void				SetThinkHandle(void* hThink) = 0;
	virtual void				Release() = 0;
};
enum AntiAimThirdperson_t
{
	ANTIAIM_THIRDPERSON_REAL,
	ANTIAIM_THIRDPERSON_FAKE,
};
enum MoveType_t
{
	MOVETYPE_NONE = 0,
	MOVETYPE_ISOMETRIC,
	MOVETYPE_WALK,
	MOVETYPE_STEP,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_LADDER,
	MOVETYPE_OBSERVER,
	MOVETYPE_CUSTOM,
	MOVETYPE_LAST = MOVETYPE_CUSTOM,
	MOVETYPE_MAX_BITS = 4
};



enum class ObserverMode_t : int
{
	OBS_MODE_NONE = 0,
	OBS_MODE_DEATHCAM = 1,
	OBS_MODE_FREEZECAM = 2,
	OBS_MODE_FIXED = 3,
	OBS_MODE_IN_EYE = 4,
	OBS_MODE_CHASE = 5,
	OBS_MODE_ROAMING = 6
};

enum EntityShape_t : int
{
	SHAPE_CIRCLE,
	SHAPE_SQUARE,
	SHAPE_TRIANGLE,
	SHAPE_TRIANGLE_UPSIDEDOWN
};

class AnimationLayer
{
public:
	char  pad_0000[20];
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void *m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
};
class CBasePlayerAnimState
{
public:
	char pad[3];
	char bUnknown; //0x4
	char pad2[91];
	C_BaseEntity* pBaseEntity; //0x60
	CBaseCombatWeapon* pActiveWeapon; //0x64
	CBaseCombatWeapon* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float m_velocity; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
};
enum Activity
{
	ACT_RESET,
	ACT_IDLE,
	ACT_TRANSITION,
	ACT_COVER,
	ACT_COVER_MED,
	ACT_COVER_LOW,
	ACT_WALK,
	ACT_WALK_AIM,
	ACT_WALK_CROUCH,
	ACT_WALK_CROUCH_AIM,
	ACT_RUN,
	ACT_RUN_AIM,
	ACT_RUN_CROUCH,
	ACT_RUN_CROUCH_AIM,
	ACT_RUN_PROTECTED,
	ACT_SCRIPT_CUSTOM_MOVE,
	ACT_RANGE_ATTACK1,
	ACT_RANGE_ATTACK2,
	ACT_RANGE_ATTACK1_LOW,
	ACT_RANGE_ATTACK2_LOW,
	ACT_DIESIMPLE,
	ACT_DIEBACKWARD,
	ACT_DIEFORWARD,
	ACT_DIEVIOLENT,
	ACT_DIERAGDOLL,
	ACT_FLY,
	ACT_HOVER,
	ACT_GLIDE,
	ACT_SWIM,
	ACT_JUMP,
	ACT_HOP,
	ACT_LEAP,
	ACT_LAND,
	ACT_CLIMB_UP,
	ACT_CLIMB_DOWN,
	ACT_CLIMB_DISMOUNT,
	ACT_SHIPLADDER_UP,
	ACT_SHIPLADDER_DOWN,
	ACT_STRAFE_LEFT,
	ACT_STRAFE_RIGHT,
	ACT_ROLL_LEFT,
	ACT_ROLL_RIGHT,
	ACT_TURN_LEFT,
	ACT_TURN_RIGHT,
	ACT_CROUCH,
	ACT_CROUCHIDLE,
	ACT_STAND,
	ACT_USE,
	ACT_ALIEN_BURROW_IDLE,
	ACT_ALIEN_BURROW_OUT,
	ACT_SIGNAL1,
	ACT_SIGNAL2,
	ACT_SIGNAL3,
	ACT_SIGNAL_ADVANCE,
	ACT_SIGNAL_FORWARD,
	ACT_SIGNAL_GROUP,
	ACT_SIGNAL_HALT,
	ACT_SIGNAL_LEFT,
	ACT_SIGNAL_RIGHT,
	ACT_SIGNAL_TAKECOVER,
	ACT_LOOKBACK_RIGHT,
	ACT_LOOKBACK_LEFT,
	ACT_COWER,
	ACT_SMALL_FLINCH,
	ACT_BIG_FLINCH,
	ACT_MELEE_ATTACK1,
	ACT_MELEE_ATTACK2,
	ACT_RELOAD,
	ACT_RELOAD_START,
	ACT_RELOAD_FINISH,
	ACT_RELOAD_LOW,
	ACT_ARM,
	ACT_DISARM,
	ACT_DROP_WEAPON,
	ACT_DROP_WEAPON_SHOTGUN,
	ACT_PICKUP_GROUND,
	ACT_PICKUP_RACK,
	ACT_IDLE_ANGRY,
	ACT_IDLE_RELAXED,
	ACT_IDLE_STIMULATED,
	ACT_IDLE_AGITATED,
	ACT_IDLE_STEALTH,
	ACT_IDLE_HURT,
	ACT_WALK_RELAXED,
	ACT_WALK_STIMULATED,
	ACT_WALK_AGITATED,
	ACT_WALK_STEALTH,
	ACT_RUN_RELAXED,
	ACT_RUN_STIMULATED,
	ACT_RUN_AGITATED,
	ACT_RUN_STEALTH,
	ACT_IDLE_AIM_RELAXED,
	ACT_IDLE_AIM_STIMULATED,
	ACT_IDLE_AIM_AGITATED,
	ACT_IDLE_AIM_STEALTH,
	ACT_WALK_AIM_RELAXED,
	ACT_WALK_AIM_STIMULATED,
	ACT_WALK_AIM_AGITATED,
	ACT_WALK_AIM_STEALTH,
	ACT_RUN_AIM_RELAXED,
	ACT_RUN_AIM_STIMULATED,
	ACT_RUN_AIM_AGITATED,
	ACT_RUN_AIM_STEALTH,
	ACT_CROUCHIDLE_STIMULATED,
	ACT_CROUCHIDLE_AIM_STIMULATED,
	ACT_CROUCHIDLE_AGITATED,
	ACT_WALK_HURT,
	ACT_RUN_HURT,
	ACT_SPECIAL_ATTACK1,
	ACT_SPECIAL_ATTACK2,
	ACT_COMBAT_IDLE,
	ACT_WALK_SCARED,
	ACT_RUN_SCARED,
	ACT_VICTORY_DANCE,
	ACT_DIE_HEADSHOT,
	ACT_DIE_CHESTSHOT,
	ACT_DIE_GUTSHOT,
	ACT_DIE_BACKSHOT,
	ACT_FLINCH_HEAD,
	ACT_FLINCH_CHEST,
	ACT_FLINCH_STOMACH,
	ACT_FLINCH_LEFTARM,
	ACT_FLINCH_RIGHTARM,
	ACT_FLINCH_LEFTLEG,
	ACT_FLINCH_RIGHTLEG,
	ACT_FLINCH_PHYSICS,
	ACT_FLINCH_HEAD_BACK,
	ACT_FLINCH_HEAD_LEFT,
	ACT_FLINCH_HEAD_RIGHT,
	ACT_FLINCH_CHEST_BACK,
	ACT_FLINCH_STOMACH_BACK,
	ACT_FLINCH_CROUCH_FRONT,
	ACT_FLINCH_CROUCH_BACK,
	ACT_FLINCH_CROUCH_LEFT,
	ACT_FLINCH_CROUCH_RIGHT,
	ACT_IDLE_ON_FIRE,
	ACT_WALK_ON_FIRE,
	ACT_RUN_ON_FIRE,
	ACT_RAPPEL_LOOP,
	ACT_180_LEFT,
	ACT_180_RIGHT,
	ACT_90_LEFT,
	ACT_90_RIGHT,
	ACT_STEP_LEFT,
	ACT_STEP_RIGHT,
	ACT_STEP_BACK,
	ACT_STEP_FORE,
	ACT_GESTURE_RANGE_ATTACK1,
	ACT_GESTURE_RANGE_ATTACK2,
	ACT_GESTURE_MELEE_ATTACK1,
	ACT_GESTURE_MELEE_ATTACK2,
	ACT_GESTURE_RANGE_ATTACK1_LOW,
	ACT_GESTURE_RANGE_ATTACK2_LOW,
	ACT_MELEE_ATTACK_SWING_GESTURE,
	ACT_GESTURE_SMALL_FLINCH,
	ACT_GESTURE_BIG_FLINCH,
	ACT_GESTURE_FLINCH_BLAST,
	ACT_GESTURE_FLINCH_BLAST_SHOTGUN,
	ACT_GESTURE_FLINCH_BLAST_DAMAGED,
	ACT_GESTURE_FLINCH_BLAST_DAMAGED_SHOTGUN,
	ACT_GESTURE_FLINCH_HEAD,
	ACT_GESTURE_FLINCH_CHEST,
	ACT_GESTURE_FLINCH_STOMACH,
	ACT_GESTURE_FLINCH_LEFTARM,
	ACT_GESTURE_FLINCH_RIGHTARM,
	ACT_GESTURE_FLINCH_LEFTLEG,
	ACT_GESTURE_FLINCH_RIGHTLEG,
	ACT_GESTURE_TURN_LEFT,
	ACT_GESTURE_TURN_RIGHT,
	ACT_GESTURE_TURN_LEFT45,
	ACT_GESTURE_TURN_RIGHT45,
	ACT_GESTURE_TURN_LEFT90,
	ACT_GESTURE_TURN_RIGHT90,
	ACT_GESTURE_TURN_LEFT45_FLAT,
	ACT_GESTURE_TURN_RIGHT45_FLAT,
	ACT_GESTURE_TURN_LEFT90_FLAT,
	ACT_GESTURE_TURN_RIGHT90_FLAT,
	ACT_BARNACLE_HIT,
	ACT_BARNACLE_PULL,
	ACT_BARNACLE_CHOMP,
	ACT_BARNACLE_CHEW,
	ACT_DO_NOT_DISTURB,
	ACT_SPECIFIC_SEQUENCE,
	ACT_VM_DRAW,
	ACT_VM_HOLSTER,
	ACT_VM_IDLE,
	ACT_VM_FIDGET,
	ACT_VM_PULLBACK,
	ACT_VM_PULLBACK_HIGH,
	ACT_VM_PULLBACK_LOW,
	ACT_VM_THROW,
	ACT_VM_PULLPIN,
	ACT_VM_PRIMARYATTACK,
	ACT_VM_SECONDARYATTACK,
	ACT_VM_RELOAD,
	ACT_VM_DRYFIRE,
	ACT_VM_HITLEFT,
	ACT_VM_HITLEFT2,
	ACT_VM_HITRIGHT,
	ACT_VM_HITRIGHT2,
	ACT_VM_HITCENTER,
	ACT_VM_HITCENTER2,
	ACT_VM_MISSLEFT,
	ACT_VM_MISSLEFT2,
	ACT_VM_MISSRIGHT,
	ACT_VM_MISSRIGHT2,
	ACT_VM_MISSCENTER,
	ACT_VM_MISSCENTER2,
	ACT_VM_HAULBACK,
	ACT_VM_SWINGHARD,
	ACT_VM_SWINGMISS,
	ACT_VM_SWINGHIT,
	ACT_VM_IDLE_TO_LOWERED,
	ACT_VM_IDLE_LOWERED,
	ACT_VM_LOWERED_TO_IDLE,
	ACT_VM_RECOIL1,
	ACT_VM_RECOIL2,
	ACT_VM_RECOIL3,
	ACT_VM_PICKUP,
	ACT_VM_RELEASE,
	ACT_VM_ATTACH_SILENCER,
	ACT_VM_DETACH_SILENCER,
	ACT_VM_EMPTY_FIRE,
	ACT_VM_EMPTY_RELOAD,
	ACT_VM_EMPTY_DRAW,
	ACT_VM_EMPTY_IDLE,
	ACT_SLAM_STICKWALL_IDLE,
	ACT_SLAM_STICKWALL_ND_IDLE,
	ACT_SLAM_STICKWALL_ATTACH,
	ACT_SLAM_STICKWALL_ATTACH2,
	ACT_SLAM_STICKWALL_ND_ATTACH,
	ACT_SLAM_STICKWALL_ND_ATTACH2,
	ACT_SLAM_STICKWALL_DETONATE,
	ACT_SLAM_STICKWALL_DETONATOR_HOLSTER,
	ACT_SLAM_STICKWALL_DRAW,
	ACT_SLAM_STICKWALL_ND_DRAW,
	ACT_SLAM_STICKWALL_TO_THROW,
	ACT_SLAM_STICKWALL_TO_THROW_ND,
	ACT_SLAM_STICKWALL_TO_TRIPMINE_ND,
	ACT_SLAM_THROW_IDLE,
	ACT_SLAM_THROW_ND_IDLE,
	ACT_SLAM_THROW_THROW,
	ACT_SLAM_THROW_THROW2,
	ACT_SLAM_THROW_THROW_ND,
	ACT_SLAM_THROW_THROW_ND2,
	ACT_SLAM_THROW_DRAW,
	ACT_SLAM_THROW_ND_DRAW,
	ACT_SLAM_THROW_TO_STICKWALL,
	ACT_SLAM_THROW_TO_STICKWALL_ND,
	ACT_SLAM_THROW_DETONATE,
	ACT_SLAM_THROW_DETONATOR_HOLSTER,
	ACT_SLAM_THROW_TO_TRIPMINE_ND,
	ACT_SLAM_TRIPMINE_IDLE,
	ACT_SLAM_TRIPMINE_DRAW,
	ACT_SLAM_TRIPMINE_ATTACH,
	ACT_SLAM_TRIPMINE_ATTACH2,
	ACT_SLAM_TRIPMINE_TO_STICKWALL_ND,
	ACT_SLAM_TRIPMINE_TO_THROW_ND,
	ACT_SLAM_DETONATOR_IDLE,
	ACT_SLAM_DETONATOR_DRAW,
	ACT_SLAM_DETONATOR_DETONATE,
	ACT_SLAM_DETONATOR_HOLSTER,
	ACT_SLAM_DETONATOR_STICKWALL_DRAW,
	ACT_SLAM_DETONATOR_THROW_DRAW,
	ACT_SHOTGUN_RELOAD_START,
	ACT_SHOTGUN_RELOAD_FINISH,
	ACT_SHOTGUN_PUMP,
	ACT_SMG2_IDLE2,
	ACT_SMG2_FIRE2,
	ACT_SMG2_DRAW2,
	ACT_SMG2_RELOAD2,
	ACT_SMG2_DRYFIRE2,
	ACT_SMG2_TOAUTO,
	ACT_SMG2_TOBURST,
	ACT_PHYSCANNON_UPGRADE,
	ACT_RANGE_ATTACK_AR1,
	ACT_RANGE_ATTACK_AR2,
	ACT_RANGE_ATTACK_AR2_LOW,
	ACT_RANGE_ATTACK_AR2_GRENADE,
	ACT_RANGE_ATTACK_HMG1,
	ACT_RANGE_ATTACK_ML,
	ACT_RANGE_ATTACK_SMG1,
	ACT_RANGE_ATTACK_SMG1_LOW,
	ACT_RANGE_ATTACK_SMG2,
	ACT_RANGE_ATTACK_SHOTGUN,
	ACT_RANGE_ATTACK_SHOTGUN_LOW,
	ACT_RANGE_ATTACK_PISTOL,
	ACT_RANGE_ATTACK_PISTOL_LOW,
	ACT_RANGE_ATTACK_SLAM,
	ACT_RANGE_ATTACK_TRIPWIRE,
	ACT_RANGE_ATTACK_THROW,
	ACT_RANGE_ATTACK_SNIPER_RIFLE,
	ACT_RANGE_ATTACK_RPG,
	ACT_MELEE_ATTACK_SWING,
	ACT_RANGE_AIM_LOW,
	ACT_RANGE_AIM_SMG1_LOW,
	ACT_RANGE_AIM_PISTOL_LOW,
	ACT_RANGE_AIM_AR2_LOW,
	ACT_COVER_PISTOL_LOW,
	ACT_COVER_SMG1_LOW,
	ACT_GESTURE_RANGE_ATTACK_AR1,
	ACT_GESTURE_RANGE_ATTACK_AR2,
	ACT_GESTURE_RANGE_ATTACK_AR2_GRENADE,
	ACT_GESTURE_RANGE_ATTACK_HMG1,
	ACT_GESTURE_RANGE_ATTACK_ML,
	ACT_GESTURE_RANGE_ATTACK_SMG1,
	ACT_GESTURE_RANGE_ATTACK_SMG1_LOW,
	ACT_GESTURE_RANGE_ATTACK_SMG2,
	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,
	ACT_GESTURE_RANGE_ATTACK_PISTOL,
	ACT_GESTURE_RANGE_ATTACK_PISTOL_LOW,
	ACT_GESTURE_RANGE_ATTACK_SLAM,
	ACT_GESTURE_RANGE_ATTACK_TRIPWIRE,
	ACT_GESTURE_RANGE_ATTACK_THROW,
	ACT_GESTURE_RANGE_ATTACK_SNIPER_RIFLE,
	ACT_GESTURE_MELEE_ATTACK_SWING,
	ACT_IDLE_RIFLE,
	ACT_IDLE_SMG1,
	ACT_IDLE_ANGRY_SMG1,
	ACT_IDLE_PISTOL,
	ACT_IDLE_ANGRY_PISTOL,
	ACT_IDLE_ANGRY_SHOTGUN,
	ACT_IDLE_STEALTH_PISTOL,
	ACT_IDLE_PACKAGE,
	ACT_WALK_PACKAGE,
	ACT_IDLE_SUITCASE,
	ACT_WALK_SUITCASE,
	ACT_IDLE_SMG1_RELAXED,
	ACT_IDLE_SMG1_STIMULATED,
	ACT_WALK_RIFLE_RELAXED,
	ACT_RUN_RIFLE_RELAXED,
	ACT_WALK_RIFLE_STIMULATED,
	ACT_RUN_RIFLE_STIMULATED,
	ACT_IDLE_AIM_RIFLE_STIMULATED,
	ACT_WALK_AIM_RIFLE_STIMULATED,
	ACT_RUN_AIM_RIFLE_STIMULATED,
	ACT_IDLE_SHOTGUN_RELAXED,
	ACT_IDLE_SHOTGUN_STIMULATED,
	ACT_IDLE_SHOTGUN_AGITATED,
	ACT_WALK_ANGRY,
	ACT_POLICE_HARASS1,
	ACT_POLICE_HARASS2,
	ACT_IDLE_MANNEDGUN,
	ACT_IDLE_MELEE,
	ACT_IDLE_ANGRY_MELEE,
	ACT_IDLE_RPG_RELAXED,
	ACT_IDLE_RPG,
	ACT_IDLE_ANGRY_RPG,
	ACT_COVER_LOW_RPG,
	ACT_WALK_RPG,
	ACT_RUN_RPG,
	ACT_WALK_CROUCH_RPG,
	ACT_RUN_CROUCH_RPG,
	ACT_WALK_RPG_RELAXED,
	ACT_RUN_RPG_RELAXED,
	ACT_WALK_RIFLE,
	ACT_WALK_AIM_RIFLE,
	ACT_WALK_CROUCH_RIFLE,
	ACT_WALK_CROUCH_AIM_RIFLE,
	ACT_RUN_RIFLE,
	ACT_RUN_AIM_RIFLE,
	ACT_RUN_CROUCH_RIFLE,
	ACT_RUN_CROUCH_AIM_RIFLE,
	ACT_RUN_STEALTH_PISTOL,
	ACT_WALK_AIM_SHOTGUN,
	ACT_RUN_AIM_SHOTGUN,
	ACT_WALK_PISTOL,
	ACT_RUN_PISTOL,
	ACT_WALK_AIM_PISTOL,
	ACT_RUN_AIM_PISTOL,
	ACT_WALK_STEALTH_PISTOL,
	ACT_WALK_AIM_STEALTH_PISTOL,
	ACT_RUN_AIM_STEALTH_PISTOL,
	ACT_RELOAD_PISTOL,
	ACT_RELOAD_PISTOL_LOW,
	ACT_RELOAD_SMG1,
	ACT_RELOAD_SMG1_LOW,
	ACT_RELOAD_SHOTGUN,
	ACT_RELOAD_SHOTGUN_LOW,
	ACT_GESTURE_RELOAD,
	ACT_GESTURE_RELOAD_PISTOL,
	ACT_GESTURE_RELOAD_SMG1,
	ACT_GESTURE_RELOAD_SHOTGUN,
	ACT_BUSY_LEAN_LEFT,
	ACT_BUSY_LEAN_LEFT_ENTRY,
	ACT_BUSY_LEAN_LEFT_EXIT,
	ACT_BUSY_LEAN_BACK,
	ACT_BUSY_LEAN_BACK_ENTRY,
	ACT_BUSY_LEAN_BACK_EXIT,
	ACT_BUSY_SIT_GROUND,
	ACT_BUSY_SIT_GROUND_ENTRY,
	ACT_BUSY_SIT_GROUND_EXIT,
	ACT_BUSY_SIT_CHAIR,
	ACT_BUSY_SIT_CHAIR_ENTRY,
	ACT_BUSY_SIT_CHAIR_EXIT,
	ACT_BUSY_STAND,
	ACT_BUSY_QUEUE,
	ACT_DUCK_DODGE,
	ACT_DIE_BARNACLE_SWALLOW,
	ACT_GESTURE_BARNACLE_STRANGLE,
	ACT_PHYSCANNON_DETACH,
	ACT_PHYSCANNON_ANIMATE,
	ACT_PHYSCANNON_ANIMATE_PRE,
	ACT_PHYSCANNON_ANIMATE_POST,
	ACT_DIE_FRONTSIDE,
	ACT_DIE_RIGHTSIDE,
	ACT_DIE_BACKSIDE,
	ACT_DIE_LEFTSIDE,
	ACT_DIE_CROUCH_FRONTSIDE,
	ACT_DIE_CROUCH_RIGHTSIDE,
	ACT_DIE_CROUCH_BACKSIDE,
	ACT_DIE_CROUCH_LEFTSIDE,
	ACT_OPEN_DOOR,
	ACT_DI_ALYX_ZOMBIE_MELEE,
	ACT_DI_ALYX_ZOMBIE_TORSO_MELEE,
	ACT_DI_ALYX_HEADCRAB_MELEE,
	ACT_DI_ALYX_ANTLION,
	ACT_DI_ALYX_ZOMBIE_SHOTGUN64,
	ACT_DI_ALYX_ZOMBIE_SHOTGUN26,
	ACT_READINESS_RELAXED_TO_STIMULATED,
	ACT_READINESS_RELAXED_TO_STIMULATED_WALK,
	ACT_READINESS_AGITATED_TO_STIMULATED,
	ACT_READINESS_STIMULATED_TO_RELAXED,
	ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED,
	ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK,
	ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED,
	ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED,
	ACT_IDLE_CARRY,
	ACT_WALK_CARRY,
	ACT_STARTDYING,
	ACT_DYINGLOOP,
	ACT_DYINGTODEAD,
	ACT_RIDE_MANNED_GUN,
	ACT_VM_SPRINT_ENTER,
	ACT_VM_SPRINT_IDLE,
	ACT_VM_SPRINT_LEAVE,
	ACT_FIRE_START,
	ACT_FIRE_LOOP,
	ACT_FIRE_END,
	ACT_CROUCHING_GRENADEIDLE,
	ACT_CROUCHING_GRENADEREADY,
	ACT_CROUCHING_PRIMARYATTACK,
	ACT_OVERLAY_GRENADEIDLE,
	ACT_OVERLAY_GRENADEREADY,
	ACT_OVERLAY_PRIMARYATTACK,
	ACT_OVERLAY_SHIELD_UP,
	ACT_OVERLAY_SHIELD_DOWN,
	ACT_OVERLAY_SHIELD_UP_IDLE,
	ACT_OVERLAY_SHIELD_ATTACK,
	ACT_OVERLAY_SHIELD_KNOCKBACK,
	ACT_SHIELD_UP,
	ACT_SHIELD_DOWN,
	ACT_SHIELD_UP_IDLE,
	ACT_SHIELD_ATTACK,
	ACT_SHIELD_KNOCKBACK,
	ACT_CROUCHING_SHIELD_UP,
	ACT_CROUCHING_SHIELD_DOWN,
	ACT_CROUCHING_SHIELD_UP_IDLE,
	ACT_CROUCHING_SHIELD_ATTACK,
	ACT_CROUCHING_SHIELD_KNOCKBACK,
	ACT_TURNRIGHT45,
	ACT_TURNLEFT45,
	ACT_TURN,
	ACT_OBJ_ASSEMBLING,
	ACT_OBJ_DISMANTLING,
	ACT_OBJ_STARTUP,
	ACT_OBJ_RUNNING,
	ACT_OBJ_IDLE,
	ACT_OBJ_PLACING,
	ACT_OBJ_DETERIORATING,
	ACT_OBJ_UPGRADING,
	ACT_DEPLOY,
	ACT_DEPLOY_IDLE,
	ACT_UNDEPLOY,
	ACT_CROSSBOW_DRAW_UNLOADED,
	ACT_GAUSS_SPINUP,
	ACT_GAUSS_SPINCYCLE,
	ACT_VM_PRIMARYATTACK_SILENCED,
	ACT_VM_RELOAD_SILENCED,
	ACT_VM_DRYFIRE_SILENCED,
	ACT_VM_IDLE_SILENCED,
	ACT_VM_DRAW_SILENCED,
	ACT_VM_IDLE_EMPTY_LEFT,
	ACT_VM_DRYFIRE_LEFT,
	ACT_VM_IS_DRAW,
	ACT_VM_IS_HOLSTER,
	ACT_VM_IS_IDLE,
	ACT_VM_IS_PRIMARYATTACK,
	ACT_PLAYER_IDLE_FIRE,
	ACT_PLAYER_CROUCH_FIRE,
	ACT_PLAYER_CROUCH_WALK_FIRE,
	ACT_PLAYER_WALK_FIRE,
	ACT_PLAYER_RUN_FIRE,
	ACT_IDLETORUN,
	ACT_RUNTOIDLE,
	ACT_VM_DRAW_DEPLOYED,
	ACT_HL2MP_IDLE_MELEE,
	ACT_HL2MP_RUN_MELEE,
	ACT_HL2MP_IDLE_CROUCH_MELEE,
	ACT_HL2MP_WALK_CROUCH_MELEE,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,
	ACT_HL2MP_GESTURE_RELOAD_MELEE,
	ACT_HL2MP_JUMP_MELEE,
	ACT_VM_FIZZLE,
	ACT_MP_STAND_IDLE,
	ACT_MP_CROUCH_IDLE,
	ACT_MP_CROUCH_DEPLOYED_IDLE,
	ACT_MP_CROUCH_DEPLOYED,
	ACT_MP_DEPLOYED_IDLE,
	ACT_MP_RUN,
	ACT_MP_WALK,
	ACT_MP_AIRWALK,
	ACT_MP_CROUCHWALK,
	ACT_MP_SPRINT,
	ACT_MP_JUMP,
	ACT_MP_JUMP_START,
	ACT_MP_JUMP_FLOAT,
	ACT_MP_JUMP_LAND,
	ACT_MP_JUMP_IMPACT_N,
	ACT_MP_JUMP_IMPACT_E,
	ACT_MP_JUMP_IMPACT_W,
	ACT_MP_JUMP_IMPACT_S,
	ACT_MP_JUMP_IMPACT_TOP,
	ACT_MP_DOUBLEJUMP,
	ACT_MP_SWIM,
	ACT_MP_DEPLOYED,
	ACT_MP_SWIM_DEPLOYED,
	ACT_MP_VCD,
	ACT_MP_ATTACK_STAND_PRIMARYFIRE,
	ACT_MP_ATTACK_STAND_PRIMARYFIRE_DEPLOYED,
	ACT_MP_ATTACK_STAND_SECONDARYFIRE,
	ACT_MP_ATTACK_STAND_GRENADE,
	ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,
	ACT_MP_ATTACK_CROUCH_PRIMARYFIRE_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_SECONDARYFIRE,
	ACT_MP_ATTACK_CROUCH_GRENADE,
	ACT_MP_ATTACK_SWIM_PRIMARYFIRE,
	ACT_MP_ATTACK_SWIM_SECONDARYFIRE,
	ACT_MP_ATTACK_SWIM_GRENADE,
	ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE,
	ACT_MP_ATTACK_AIRWALK_SECONDARYFIRE,
	ACT_MP_ATTACK_AIRWALK_GRENADE,
	ACT_MP_RELOAD_STAND,
	ACT_MP_RELOAD_STAND_LOOP,
	ACT_MP_RELOAD_STAND_END,
	ACT_MP_RELOAD_CROUCH,
	ACT_MP_RELOAD_CROUCH_LOOP,
	ACT_MP_RELOAD_CROUCH_END,
	ACT_MP_RELOAD_SWIM,
	ACT_MP_RELOAD_SWIM_LOOP,
	ACT_MP_RELOAD_SWIM_END,
	ACT_MP_RELOAD_AIRWALK,
	ACT_MP_RELOAD_AIRWALK_LOOP,
	ACT_MP_RELOAD_AIRWALK_END,
	ACT_MP_ATTACK_STAND_PREFIRE,
	ACT_MP_ATTACK_STAND_POSTFIRE,
	ACT_MP_ATTACK_STAND_STARTFIRE,
	ACT_MP_ATTACK_CROUCH_PREFIRE,
	ACT_MP_ATTACK_CROUCH_POSTFIRE,
	ACT_MP_ATTACK_SWIM_PREFIRE,
	ACT_MP_ATTACK_SWIM_POSTFIRE,
	ACT_MP_STAND_PRIMARY,
	ACT_MP_CROUCH_PRIMARY,
	ACT_MP_RUN_PRIMARY,
	ACT_MP_WALK_PRIMARY,
	ACT_MP_AIRWALK_PRIMARY,
	ACT_MP_CROUCHWALK_PRIMARY,
	ACT_MP_JUMP_PRIMARY,
	ACT_MP_JUMP_START_PRIMARY,
	ACT_MP_JUMP_FLOAT_PRIMARY,
	ACT_MP_JUMP_LAND_PRIMARY,
	ACT_MP_SWIM_PRIMARY,
	ACT_MP_DEPLOYED_PRIMARY,
	ACT_MP_SWIM_DEPLOYED_PRIMARY,
	ACT_MP_ATTACK_STAND_PRIMARY,
	ACT_MP_ATTACK_STAND_PRIMARY_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_PRIMARY,
	ACT_MP_ATTACK_CROUCH_PRIMARY_DEPLOYED,
	ACT_MP_ATTACK_SWIM_PRIMARY,
	ACT_MP_ATTACK_AIRWALK_PRIMARY,
	ACT_MP_RELOAD_STAND_PRIMARY,
	ACT_MP_RELOAD_STAND_PRIMARY_LOOP,
	ACT_MP_RELOAD_STAND_PRIMARY_END,
	ACT_MP_RELOAD_CROUCH_PRIMARY,
	ACT_MP_RELOAD_CROUCH_PRIMARY_LOOP,
	ACT_MP_RELOAD_CROUCH_PRIMARY_END,
	ACT_MP_RELOAD_SWIM_PRIMARY,
	ACT_MP_RELOAD_SWIM_PRIMARY_LOOP,
	ACT_MP_RELOAD_SWIM_PRIMARY_END,
	ACT_MP_RELOAD_AIRWALK_PRIMARY,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_LOOP,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_END,
	ACT_MP_ATTACK_STAND_GRENADE_PRIMARY,
	ACT_MP_ATTACK_CROUCH_GRENADE_PRIMARY,
	ACT_MP_ATTACK_SWIM_GRENADE_PRIMARY,
	ACT_MP_ATTACK_AIRWALK_GRENADE_PRIMARY,
	ACT_MP_STAND_SECONDARY,
	ACT_MP_CROUCH_SECONDARY,
	ACT_MP_RUN_SECONDARY,
	ACT_MP_WALK_SECONDARY,
	ACT_MP_AIRWALK_SECONDARY,
	ACT_MP_CROUCHWALK_SECONDARY,
	ACT_MP_JUMP_SECONDARY,
	ACT_MP_JUMP_START_SECONDARY,
	ACT_MP_JUMP_FLOAT_SECONDARY,
	ACT_MP_JUMP_LAND_SECONDARY,
	ACT_MP_SWIM_SECONDARY,
	ACT_MP_ATTACK_STAND_SECONDARY,
	ACT_MP_ATTACK_CROUCH_SECONDARY,
	ACT_MP_ATTACK_SWIM_SECONDARY,
	ACT_MP_ATTACK_AIRWALK_SECONDARY,
	ACT_MP_RELOAD_STAND_SECONDARY,
	ACT_MP_RELOAD_STAND_SECONDARY_LOOP,
	ACT_MP_RELOAD_STAND_SECONDARY_END,
	ACT_MP_RELOAD_CROUCH_SECONDARY,
	ACT_MP_RELOAD_CROUCH_SECONDARY_LOOP,
	ACT_MP_RELOAD_CROUCH_SECONDARY_END,
	ACT_MP_RELOAD_SWIM_SECONDARY,
	ACT_MP_RELOAD_SWIM_SECONDARY_LOOP,
	ACT_MP_RELOAD_SWIM_SECONDARY_END,
	ACT_MP_RELOAD_AIRWALK_SECONDARY,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_LOOP,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_END,
	ACT_MP_ATTACK_STAND_GRENADE_SECONDARY,
	ACT_MP_ATTACK_CROUCH_GRENADE_SECONDARY,
	ACT_MP_ATTACK_SWIM_GRENADE_SECONDARY,
	ACT_MP_ATTACK_AIRWALK_GRENADE_SECONDARY,
	ACT_MP_STAND_MELEE,
	ACT_MP_CROUCH_MELEE,
	ACT_MP_RUN_MELEE,
	ACT_MP_WALK_MELEE,
	ACT_MP_AIRWALK_MELEE,
	ACT_MP_CROUCHWALK_MELEE,
	ACT_MP_JUMP_MELEE,
	ACT_MP_JUMP_START_MELEE,
	ACT_MP_JUMP_FLOAT_MELEE,
	ACT_MP_JUMP_LAND_MELEE,
	ACT_MP_SWIM_MELEE,
	ACT_MP_ATTACK_STAND_MELEE,
	ACT_MP_ATTACK_STAND_MELEE_SECONDARY,
	ACT_MP_ATTACK_CROUCH_MELEE,
	ACT_MP_ATTACK_CROUCH_MELEE_SECONDARY,
	ACT_MP_ATTACK_SWIM_MELEE,
	ACT_MP_ATTACK_AIRWALK_MELEE,
	ACT_MP_ATTACK_STAND_GRENADE_MELEE,
	ACT_MP_ATTACK_CROUCH_GRENADE_MELEE,
	ACT_MP_ATTACK_SWIM_GRENADE_MELEE,
	ACT_MP_ATTACK_AIRWALK_GRENADE_MELEE,
	ACT_MP_STAND_ITEM1,
	ACT_MP_CROUCH_ITEM1,
	ACT_MP_RUN_ITEM1,
	ACT_MP_WALK_ITEM1,
	ACT_MP_AIRWALK_ITEM1,
	ACT_MP_CROUCHWALK_ITEM1,
	ACT_MP_JUMP_ITEM1,
	ACT_MP_JUMP_START_ITEM1,
	ACT_MP_JUMP_FLOAT_ITEM1,
	ACT_MP_JUMP_LAND_ITEM1,
	ACT_MP_SWIM_ITEM1,
	ACT_MP_ATTACK_STAND_ITEM1,
	ACT_MP_ATTACK_STAND_ITEM1_SECONDARY,
	ACT_MP_ATTACK_CROUCH_ITEM1,
	ACT_MP_ATTACK_CROUCH_ITEM1_SECONDARY,
	ACT_MP_ATTACK_SWIM_ITEM1,
	ACT_MP_ATTACK_AIRWALK_ITEM1,
	ACT_MP_STAND_ITEM2,
	ACT_MP_CROUCH_ITEM2,
	ACT_MP_RUN_ITEM2,
	ACT_MP_WALK_ITEM2,
	ACT_MP_AIRWALK_ITEM2,
	ACT_MP_CROUCHWALK_ITEM2,
	ACT_MP_JUMP_ITEM2,
	ACT_MP_JUMP_START_ITEM2,
	ACT_MP_JUMP_FLOAT_ITEM2,
	ACT_MP_JUMP_LAND_ITEM2,
	ACT_MP_SWIM_ITEM2,
	ACT_MP_ATTACK_STAND_ITEM2,
	ACT_MP_ATTACK_STAND_ITEM2_SECONDARY,
	ACT_MP_ATTACK_CROUCH_ITEM2,
	ACT_MP_ATTACK_CROUCH_ITEM2_SECONDARY,
	ACT_MP_ATTACK_SWIM_ITEM2,
	ACT_MP_ATTACK_AIRWALK_ITEM2,
	ACT_MP_GESTURE_FLINCH,
	ACT_MP_GESTURE_FLINCH_PRIMARY,
	ACT_MP_GESTURE_FLINCH_SECONDARY,
	ACT_MP_GESTURE_FLINCH_MELEE,
	ACT_MP_GESTURE_FLINCH_ITEM1,
	ACT_MP_GESTURE_FLINCH_ITEM2,
	ACT_MP_GESTURE_FLINCH_HEAD,
	ACT_MP_GESTURE_FLINCH_CHEST,
	ACT_MP_GESTURE_FLINCH_STOMACH,
	ACT_MP_GESTURE_FLINCH_LEFTARM,
	ACT_MP_GESTURE_FLINCH_RIGHTARM,
	ACT_MP_GESTURE_FLINCH_LEFTLEG,
	ACT_MP_GESTURE_FLINCH_RIGHTLEG,
	ACT_MP_GRENADE1_DRAW,
	ACT_MP_GRENADE1_IDLE,
	ACT_MP_GRENADE1_ATTACK,
	ACT_MP_GRENADE2_DRAW,
	ACT_MP_GRENADE2_IDLE,
	ACT_MP_GRENADE2_ATTACK,
	ACT_MP_PRIMARY_GRENADE1_DRAW,
	ACT_MP_PRIMARY_GRENADE1_IDLE,
	ACT_MP_PRIMARY_GRENADE1_ATTACK,
	ACT_MP_PRIMARY_GRENADE2_DRAW,
	ACT_MP_PRIMARY_GRENADE2_IDLE,
	ACT_MP_PRIMARY_GRENADE2_ATTACK,
	ACT_MP_SECONDARY_GRENADE1_DRAW,
	ACT_MP_SECONDARY_GRENADE1_IDLE,
	ACT_MP_SECONDARY_GRENADE1_ATTACK,
	ACT_MP_SECONDARY_GRENADE2_DRAW,
	ACT_MP_SECONDARY_GRENADE2_IDLE,
	ACT_MP_SECONDARY_GRENADE2_ATTACK,
	ACT_MP_MELEE_GRENADE1_DRAW,
	ACT_MP_MELEE_GRENADE1_IDLE,
	ACT_MP_MELEE_GRENADE1_ATTACK,
	ACT_MP_MELEE_GRENADE2_DRAW,
	ACT_MP_MELEE_GRENADE2_IDLE,
	ACT_MP_MELEE_GRENADE2_ATTACK,
	ACT_MP_ITEM1_GRENADE1_DRAW,
	ACT_MP_ITEM1_GRENADE1_IDLE,
	ACT_MP_ITEM1_GRENADE1_ATTACK,
	ACT_MP_ITEM1_GRENADE2_DRAW,
	ACT_MP_ITEM1_GRENADE2_IDLE,
	ACT_MP_ITEM1_GRENADE2_ATTACK,
	ACT_MP_ITEM2_GRENADE1_DRAW,
	ACT_MP_ITEM2_GRENADE1_IDLE,
	ACT_MP_ITEM2_GRENADE1_ATTACK,
	ACT_MP_ITEM2_GRENADE2_DRAW,
	ACT_MP_ITEM2_GRENADE2_IDLE,
	ACT_MP_ITEM2_GRENADE2_ATTACK,
	ACT_MP_STAND_BUILDING,
	ACT_MP_CROUCH_BUILDING,
	ACT_MP_RUN_BUILDING,
	ACT_MP_WALK_BUILDING,
	ACT_MP_AIRWALK_BUILDING,
	ACT_MP_CROUCHWALK_BUILDING,
	ACT_MP_JUMP_BUILDING,
	ACT_MP_JUMP_START_BUILDING,
	ACT_MP_JUMP_FLOAT_BUILDING,
	ACT_MP_JUMP_LAND_BUILDING,
	ACT_MP_SWIM_BUILDING,
	ACT_MP_ATTACK_STAND_BUILDING,
	ACT_MP_ATTACK_CROUCH_BUILDING,
	ACT_MP_ATTACK_SWIM_BUILDING,
	ACT_MP_ATTACK_AIRWALK_BUILDING,
	ACT_MP_ATTACK_STAND_GRENADE_BUILDING,
	ACT_MP_ATTACK_CROUCH_GRENADE_BUILDING,
	ACT_MP_ATTACK_SWIM_GRENADE_BUILDING,
	ACT_MP_ATTACK_AIRWALK_GRENADE_BUILDING,
	ACT_MP_STAND_PDA,
	ACT_MP_CROUCH_PDA,
	ACT_MP_RUN_PDA,
	ACT_MP_WALK_PDA,
	ACT_MP_AIRWALK_PDA,
	ACT_MP_CROUCHWALK_PDA,
	ACT_MP_JUMP_PDA,
	ACT_MP_JUMP_START_PDA,
	ACT_MP_JUMP_FLOAT_PDA,
	ACT_MP_JUMP_LAND_PDA,
	ACT_MP_SWIM_PDA,
	ACT_MP_ATTACK_STAND_PDA,
	ACT_MP_ATTACK_SWIM_PDA,
	ACT_MP_GESTURE_VC_HANDMOUTH,
	ACT_MP_GESTURE_VC_FINGERPOINT,
	ACT_MP_GESTURE_VC_FISTPUMP,
	ACT_MP_GESTURE_VC_THUMBSUP,
	ACT_MP_GESTURE_VC_NODYES,
	ACT_MP_GESTURE_VC_NODNO,
	ACT_MP_GESTURE_VC_HANDMOUTH_PRIMARY,
	ACT_MP_GESTURE_VC_FINGERPOINT_PRIMARY,
	ACT_MP_GESTURE_VC_FISTPUMP_PRIMARY,
	ACT_MP_GESTURE_VC_THUMBSUP_PRIMARY,
	ACT_MP_GESTURE_VC_NODYES_PRIMARY,
	ACT_MP_GESTURE_VC_NODNO_PRIMARY,
	ACT_MP_GESTURE_VC_HANDMOUTH_SECONDARY,
	ACT_MP_GESTURE_VC_FINGERPOINT_SECONDARY,
	ACT_MP_GESTURE_VC_FISTPUMP_SECONDARY,
	ACT_MP_GESTURE_VC_THUMBSUP_SECONDARY,
	ACT_MP_GESTURE_VC_NODYES_SECONDARY,
	ACT_MP_GESTURE_VC_NODNO_SECONDARY,
	ACT_MP_GESTURE_VC_HANDMOUTH_MELEE,
	ACT_MP_GESTURE_VC_FINGERPOINT_MELEE,
	ACT_MP_GESTURE_VC_FISTPUMP_MELEE,
	ACT_MP_GESTURE_VC_THUMBSUP_MELEE,
	ACT_MP_GESTURE_VC_NODYES_MELEE,
	ACT_MP_GESTURE_VC_NODNO_MELEE,
	ACT_MP_GESTURE_VC_HANDMOUTH_ITEM1,
	ACT_MP_GESTURE_VC_FINGERPOINT_ITEM1,
	ACT_MP_GESTURE_VC_FISTPUMP_ITEM1,
	ACT_MP_GESTURE_VC_THUMBSUP_ITEM1,
	ACT_MP_GESTURE_VC_NODYES_ITEM1,
	ACT_MP_GESTURE_VC_NODNO_ITEM1,
	ACT_MP_GESTURE_VC_HANDMOUTH_ITEM2,
	ACT_MP_GESTURE_VC_FINGERPOINT_ITEM2,
	ACT_MP_GESTURE_VC_FISTPUMP_ITEM2,
	ACT_MP_GESTURE_VC_THUMBSUP_ITEM2,
	ACT_MP_GESTURE_VC_NODYES_ITEM2,
	ACT_MP_GESTURE_VC_NODNO_ITEM2,
	ACT_MP_GESTURE_VC_HANDMOUTH_BUILDING,
	ACT_MP_GESTURE_VC_FINGERPOINT_BUILDING,
	ACT_MP_GESTURE_VC_FISTPUMP_BUILDING,
	ACT_MP_GESTURE_VC_THUMBSUP_BUILDING,
	ACT_MP_GESTURE_VC_NODYES_BUILDING,
	ACT_MP_GESTURE_VC_NODNO_BUILDING,
	ACT_MP_GESTURE_VC_HANDMOUTH_PDA,
	ACT_MP_GESTURE_VC_FINGERPOINT_PDA,
	ACT_MP_GESTURE_VC_FISTPUMP_PDA,
	ACT_MP_GESTURE_VC_THUMBSUP_PDA,
	ACT_MP_GESTURE_VC_NODYES_PDA,
	ACT_MP_GESTURE_VC_NODNO_PDA,
	ACT_VM_UNUSABLE,
	ACT_VM_UNUSABLE_TO_USABLE,
	ACT_VM_USABLE_TO_UNUSABLE,
	ACT_PRIMARY_VM_DRAW,
	ACT_PRIMARY_VM_HOLSTER,
	ACT_PRIMARY_VM_IDLE,
	ACT_PRIMARY_VM_PULLBACK,
	ACT_PRIMARY_VM_PRIMARYATTACK,
	ACT_PRIMARY_VM_SECONDARYATTACK,
	ACT_PRIMARY_VM_RELOAD,
	ACT_PRIMARY_VM_DRYFIRE,
	ACT_PRIMARY_VM_IDLE_TO_LOWERED,
	ACT_PRIMARY_VM_IDLE_LOWERED,
	ACT_PRIMARY_VM_LOWERED_TO_IDLE,
	ACT_SECONDARY_VM_DRAW,
	ACT_SECONDARY_VM_HOLSTER,
	ACT_SECONDARY_VM_IDLE,
	ACT_SECONDARY_VM_PULLBACK,
	ACT_SECONDARY_VM_PRIMARYATTACK,
	ACT_SECONDARY_VM_SECONDARYATTACK,
	ACT_SECONDARY_VM_RELOAD,
	ACT_SECONDARY_VM_DRYFIRE,
	ACT_SECONDARY_VM_IDLE_TO_LOWERED,
	ACT_SECONDARY_VM_IDLE_LOWERED,
	ACT_SECONDARY_VM_LOWERED_TO_IDLE,
	ACT_MELEE_VM_DRAW,
	ACT_MELEE_VM_HOLSTER,
	ACT_MELEE_VM_IDLE,
	ACT_MELEE_VM_PULLBACK,
	ACT_MELEE_VM_PRIMARYATTACK,
	ACT_MELEE_VM_SECONDARYATTACK,
	ACT_MELEE_VM_RELOAD,
	ACT_MELEE_VM_DRYFIRE,
	ACT_MELEE_VM_IDLE_TO_LOWERED,
	ACT_MELEE_VM_IDLE_LOWERED,
	ACT_MELEE_VM_LOWERED_TO_IDLE,
	ACT_PDA_VM_DRAW,
	ACT_PDA_VM_HOLSTER,
	ACT_PDA_VM_IDLE,
	ACT_PDA_VM_PULLBACK,
	ACT_PDA_VM_PRIMARYATTACK,
	ACT_PDA_VM_SECONDARYATTACK,
	ACT_PDA_VM_RELOAD,
	ACT_PDA_VM_DRYFIRE,
	ACT_PDA_VM_IDLE_TO_LOWERED,
	ACT_PDA_VM_IDLE_LOWERED,
	ACT_PDA_VM_LOWERED_TO_IDLE,
	ACT_ITEM1_VM_DRAW,
	ACT_ITEM1_VM_HOLSTER,
	ACT_ITEM1_VM_IDLE,
	ACT_ITEM1_VM_PULLBACK,
	ACT_ITEM1_VM_PRIMARYATTACK,
	ACT_ITEM1_VM_SECONDARYATTACK,
	ACT_ITEM1_VM_RELOAD,
	ACT_ITEM1_VM_DRYFIRE,
	ACT_ITEM1_VM_IDLE_TO_LOWERED,
	ACT_ITEM1_VM_IDLE_LOWERED,
	ACT_ITEM1_VM_LOWERED_TO_IDLE,
	ACT_ITEM2_VM_DRAW,
	ACT_ITEM2_VM_HOLSTER,
	ACT_ITEM2_VM_IDLE,
	ACT_ITEM2_VM_PULLBACK,
	ACT_ITEM2_VM_PRIMARYATTACK,
	ACT_ITEM2_VM_SECONDARYATTACK,
	ACT_ITEM2_VM_RELOAD,
	ACT_ITEM2_VM_DRYFIRE,
	ACT_ITEM2_VM_IDLE_TO_LOWERED,
	ACT_ITEM2_VM_IDLE_LOWERED,
	ACT_ITEM2_VM_LOWERED_TO_IDLE,
	ACT_RELOAD_SUCCEED,
	ACT_RELOAD_FAIL,
	ACT_WALK_AIM_AUTOGUN,
	ACT_RUN_AIM_AUTOGUN,
	ACT_IDLE_AUTOGUN,
	ACT_IDLE_AIM_AUTOGUN,
	ACT_RELOAD_AUTOGUN,
	ACT_CROUCH_IDLE_AUTOGUN,
	ACT_RANGE_ATTACK_AUTOGUN,
	ACT_JUMP_AUTOGUN,
	ACT_IDLE_AIM_PISTOL,
	ACT_WALK_AIM_DUAL,
	ACT_RUN_AIM_DUAL,
	ACT_IDLE_DUAL,
	ACT_IDLE_AIM_DUAL,
	ACT_RELOAD_DUAL,
	ACT_CROUCH_IDLE_DUAL,
	ACT_RANGE_ATTACK_DUAL,
	ACT_JUMP_DUAL,
	ACT_IDLE_SHOTGUN,
	ACT_IDLE_AIM_SHOTGUN,
	ACT_CROUCH_IDLE_SHOTGUN,
	ACT_JUMP_SHOTGUN,
	ACT_IDLE_AIM_RIFLE,
	ACT_RELOAD_RIFLE,
	ACT_CROUCH_IDLE_RIFLE,
	ACT_RANGE_ATTACK_RIFLE,
	ACT_JUMP_RIFLE,
	ACT_SLEEP,
	ACT_WAKE,
	ACT_FLICK_LEFT,
	ACT_FLICK_LEFT_MIDDLE,
	ACT_FLICK_RIGHT_MIDDLE,
	ACT_FLICK_RIGHT,
	ACT_SPINAROUND,
	ACT_PREP_TO_FIRE,
	ACT_FIRE,
	ACT_FIRE_RECOVER,
	ACT_SPRAY,
	ACT_PREP_EXPLODE,
	ACT_EXPLODE,
	ACT_DOTA_IDLE,
	ACT_DOTA_RUN,
	ACT_DOTA_ATTACK,
	ACT_DOTA_ATTACK_EVENT,
	ACT_DOTA_DIE,
	ACT_DOTA_FLINCH,
	ACT_DOTA_DISABLED,
	ACT_DOTA_CAST_ABILITY_1,
	ACT_DOTA_CAST_ABILITY_2,
	ACT_DOTA_CAST_ABILITY_3,
	ACT_DOTA_CAST_ABILITY_4,
	ACT_DOTA_OVERRIDE_ABILITY_1,
	ACT_DOTA_OVERRIDE_ABILITY_2,
	ACT_DOTA_OVERRIDE_ABILITY_3,
	ACT_DOTA_OVERRIDE_ABILITY_4,
	ACT_DOTA_CHANNEL_ABILITY_1,
	ACT_DOTA_CHANNEL_ABILITY_2,
	ACT_DOTA_CHANNEL_ABILITY_3,
	ACT_DOTA_CHANNEL_ABILITY_4,
	ACT_DOTA_CHANNEL_END_ABILITY_1,
	ACT_DOTA_CHANNEL_END_ABILITY_2,
	ACT_DOTA_CHANNEL_END_ABILITY_3,
	ACT_DOTA_CHANNEL_END_ABILITY_4,
	ACT_MP_RUN_SPEEDPAINT,
	ACT_MP_LONG_FALL,
	ACT_MP_TRACTORBEAM_FLOAT,
	ACT_MP_DEATH_CRUSH,
	ACT_MP_RUN_SPEEDPAINT_PRIMARY,
	ACT_MP_DROWNING_PRIMARY,
	ACT_MP_LONG_FALL_PRIMARY,
	ACT_MP_TRACTORBEAM_FLOAT_PRIMARY,
	ACT_MP_DEATH_CRUSH_PRIMARY,
	ACT_DIE_STAND,
	ACT_DIE_STAND_HEADSHOT,
	ACT_DIE_CROUCH,
	ACT_DIE_CROUCH_HEADSHOT,
	ACT_CSGO_NULL,
	ACT_CSGO_DEFUSE,
	ACT_CSGO_DEFUSE_WITH_KIT,
	ACT_CSGO_FLASHBANG_REACTION,
	ACT_CSGO_FIRE_PRIMARY,
	ACT_CSGO_FIRE_PRIMARY_OPT_1,
	ACT_CSGO_FIRE_PRIMARY_OPT_2,
	ACT_CSGO_FIRE_SECONDARY,
	ACT_CSGO_FIRE_SECONDARY_OPT_1,
	ACT_CSGO_FIRE_SECONDARY_OPT_2,
	ACT_CSGO_RELOAD,
	ACT_CSGO_RELOAD_START,
	ACT_CSGO_RELOAD_LOOP,
	ACT_CSGO_RELOAD_END,
	ACT_CSGO_OPERATE,
	ACT_CSGO_DEPLOY,
	ACT_CSGO_CATCH,
	ACT_CSGO_SILENCER_DETACH,
	ACT_CSGO_SILENCER_ATTACH,
	ACT_CSGO_TWITCH,
	ACT_CSGO_TWITCH_BUYZONE,
	ACT_CSGO_PLANT_BOMB,
	ACT_CSGO_IDLE_TURN_BALANCEADJUST,
	ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING,
	ACT_CSGO_ALIVE_LOOP,
	ACT_CSGO_FLINCH,
	ACT_CSGO_FLINCH_HEAD,
	ACT_CSGO_FLINCH_MOLOTOV,
	ACT_CSGO_JUMP,
	ACT_CSGO_FALL,
	ACT_CSGO_CLIMB_LADDER,
	ACT_CSGO_LAND_LIGHT,
	ACT_CSGO_LAND_HEAVY,
	ACT_CSGO_EXIT_LADDER_TOP,
	ACT_CSGO_EXIT_LADDER_BOTTOM,
};
class CBaseAnimState
{
public:
	char pad[3];
	char bUnknown; //0x4
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
};
class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual void Release(void) = 0;
	virtual const Vector &GetAbsOrigin(void) const = 0;
	virtual const QAngle &GetAbsAngles(void) const = 0;
	int GetSequenceActivity(int sequence);
};

class __declspec (novtable)C_BaseEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
public:
	virtual void			Release(void) = 0;
	virtual void			blahblahpad(void) = 0;
	virtual Vector&	GetAbsOrigin(void) const = 0;
	virtual const Vector&	GetAbsAngles(void) const = 0;
	int GetSequenceActivity(int sequence);

	Vector GetAbsOrigin3() {
		__asm {
			MOV ECX, this
			MOV EAX, DWORD PTR DS : [ECX]
			CALL DWORD PTR DS : [EAX + 0x28]
		}
	}
	
	CUtlVector<AnimationLayer> GetAnimationOverlay() {
		return (CUtlVector<AnimationLayer>)((DWORD)this + 0x2970);
	}
	int GetNumAnimOverlays()
	{
		return *(int*)((DWORD)this + 0x297C);
	}
	CLocalPlayerExclusive* localPlayerExclusive()
	{
		return (CLocalPlayerExclusive*)((uintptr_t)this + 0x2FAC);
	}
	CollisionProperty* collisionProperty()
	{
		return (CollisionProperty*)((uintptr_t)this + 0x318);
	}
	void SetModelIndex(int model)
	{
		typedef void(__thiscall* oSetModelIndex)(PVOID, int);
		return call_vfunc< oSetModelIndex >(this, 75)(this, model);
	}
	float GetSimulationTime()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_BaseEntity.m_flSimulationTime);
	}
	std::array<float, 24> &m_flPoseParameter()
	{
		static int _m_flPoseParameter = NetVarManager->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget");
		return *(std::array<float, 24>*)((uintptr_t)this + _m_flPoseParameter);
	}
	QAngle GetEyeAnglesUrMom()
	{
		return *reinterpret_cast<QAngle*>((DWORD)this + (DWORD)0x528C);
	}
	float GetAnimTime()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_BaseEntity.m_flAnimTime);
	}

	HANDLE m_hViewModel()
	{
		return *(HANDLE*)((uintptr_t)this + offsetz.DT_BasePlayer.m_hViewModel);
	}

	const matrix3x4_t& m_rgflCoordinateFrame()
	{
		static auto _m_rgflCoordinateFrame = NetVarManager->GetOffset("CBaseEntity", "m_CollisionGroup") - 0x30;
		return *(matrix3x4_t*)((uintptr_t)this + _m_rgflCoordinateFrame);
	}

	int m_iShotsFired()
	{
		static int meme = NetVarManager->GetOffset("DT_CSPlayer", "m_iShotsFired");
		return *(int*)((uintptr_t)this + meme);
	}
	bool IsSpotted()
	{
		return (bool*)((DWORD)this + offsetz.DT_BaseEntity.m_bSpotted);
	}
	bool* IsSpottedPtr()
	{
		return (bool*)((DWORD)this + offsetz.DT_BaseEntity.m_bSpotted);
	}
	float GetNextAttack()
	{
		return *(float*)((DWORD)this + 0x2D60);
	}
	Vector GetAbsOrigin2()
	{
		return *(Vector*)((DWORD)this + 0x84 + 0x1C);
	}
	CBaseAnimState* GetAnimState()
	{
		return *reinterpret_cast<CBaseAnimState**>(uintptr_t(this) + 0x3874);
	}
	int GetHitboxSet()
	{
		return *(int*)((DWORD)this + offsetz.DT_BaseAnimating.m_nHitboxSet);
	}
	void UpdateClientSideAnimation()
	{
		typedef void(__thiscall* original)(void*);
		call_vfunc<original>(this, 218)(this);
	}
	void SetAbsAngles(Vector angles)
	{
		using Fn = void(__thiscall*)(C_BaseEntity*, const Vector &angles);
		static Fn func;

		if (!func)
			func = (Fn)(U::FindPattern("client.dll", (BYTE*)"\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x64\x53\x56\x57\x8B\xF1\xE8", "xxxxxxxxxxxxxxx"));

		func(this, angles);
	}
	bool hasDefuser()
	{
		return *(bool*)((DWORD)this + offsetz.DT_CSPlayer.m_bHasDefuser);
	}
	bool m_bHasHeavyArmor()
	{
		return *(bool*)((DWORD)this + offsetz.DT_CSPlayer.m_bHasHeavyArmor);
	}
	int GetFlags()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BasePlayer.m_fFlags);
	}
	Vector GetVecOrigin()
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BaseEntity.m_vecOrigin);
	}
	int GetOwnerHandle()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BaseEntity.m_hOwner);
	}

	Vector GetRotation()
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BaseEntity.m_angRotation);
	}

	int GetTeamNum()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BaseEntity.m_iTeamNum);
	}

	int GetMaxHealth()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BasePlayer.m_iMaxHealth);
	}


	int GetHealth()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BasePlayer.m_iHealth);
	}

	unsigned char GetLifeState()
	{
		return *(unsigned char*)((uintptr_t)this + offsetz.DT_BasePlayer.m_lifeState);
	}

	char* GetCallout()
	{
		return (char*)((uintptr_t)this + offsetz.DT_BasePlayer.m_szLastPlaceName);
	}

	HANDLE GetActiveWeaponHandle()
	{
		return *(HANDLE*)((uintptr_t)this + offsetz.DT_BaseCombatCharacter.m_hActiveWeapon);
	}


	CBaseHandle* m_hMyWeapons()
	{
		return (CBaseHandle*)((uintptr_t)this + offsetz.DT_BaseCombatCharacter.m_hMyWeapons);
	}

	int GetTickBase()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_BasePlayer.m_nTickBase);
	}

	Vector GetViewOffset()
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BasePlayer.m_vecViewOffset);
	}

	Vector GetVelocity()
	{
		return *(Vector*)((uintptr_t)this + offsetz.DT_BasePlayer.m_vecVelocity);
	}

	bool HasGunGameImmunity()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSPlayer.m_bGunGameImmunity);
	}

	bool IsDefusing()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSPlayer.m_bIsDefusing);
	}

	int ArmorValue()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayer.m_ArmorValue);
	}

	int iAccount()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayer.m_iAccount);
	}

	char* GetArmorName()
	{
		if (ArmorValue() > 0)
		{
			if (HasHelmet())
				return "K+H";
			else
				return "K";
		}
		else
			return "";
	}

	char* GetArmorIcon()
	{
		if (ArmorValue() > 0)
		{
			if (HasHelmet())
				return "q";
			else
				return "p";
		}
		else
			return "";
	}

	bool HasHelmet()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSPlayer.m_bHasHelmet);
	}

	bool IsScoped()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSPlayer.m_bIsScoped);
	}

	float GetFlashDuration() {
		return *(float*)((DWORD)this + offsetz.DT_CSPlayer.m_flFlashDuration);
	}

	bool IsFlashed()
	{
		return GetFlashDuration() > 0;
	}

	int GetMoney()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayer.m_iAccount);
	}

	HANDLE GetObserverTargetHandle()
	{
		return *(HANDLE*)((uintptr_t)this + offsetz.DT_BasePlayer.m_hObserverTarget);
	}

	void* GetObserverTarget()
	{
		return (void*)((uintptr_t)this + offsetz.DT_BasePlayer.m_hObserverTarget);
	}

	ObserverMode_t* GetObserverMode()
	{
		return (ObserverMode_t*)((uintptr_t)this + offsetz.DT_BasePlayer.m_iObserverMode);
	}

	CBaseHandle* GetWearables()
	{
		return reinterpret_cast<CBaseHandle*>(this + offsetz.DT_BaseCombatCharacter.m_hMyWearables);
	}

	int GetMoveType() {
		return *(int*)((DWORD)this + offsetz.DT_BaseEntity.m_MoveType);
	}

	float GetLowerBodyYaw() {
		return *(float*)((DWORD)this + offsetz.DT_CSPlayer.m_flLowerBodyYawTarget);
	}
	AnimationLayer* GetAnimOverlays()
	{
		// to find offset: use 9/12/17 dll
		// sig: 55 8B EC 51 53 8B 5D 08 33 C0
		return *(AnimationLayer**)((DWORD)this + 10608);
	}
	bool& client_side_animation()
	{
		return *reinterpret_cast<bool*>(uintptr_t(this) + offsetz.DT_BaseAnimating.m_bClientSideAnimation);
	}

	Vector get_ragdoll_pos()
	{
		return *reinterpret_cast<Vector*>(uintptr_t(this) + offsetz.DT_Ragdoll.m_ragPos);
	}
	QAngle* GetEyeAngles()
	{
		return (QAngle*)((uintptr_t)this + offsetz.DT_CSPlayer.m_angEyeAngles[0]);
	}
	void SetEyeAngles(Vector angles)
	{
		*reinterpret_cast<Vector*>(uintptr_t(this) + offsetz.DT_CSPlayer.m_angEyeAngles) = angles;
	}
	IClientRenderable* GetRenderable()
	{
		return reinterpret_cast<IClientRenderable*>((DWORD)this + 0x4);
	}
	bool IsAlive()
	{
		return (GetLifeState() == LIFE_ALIVE && GetHealth() > 0);
	}
	AnimationLayer& GetAnimOverlay(int Index)
	{
		return (*(AnimationLayer**)((DWORD)this + 0x2970))[Index];
	}
	bool IsWeapon()
	{
		return call_vfunc<bool(__thiscall*)(C_BaseEntity*)>(this, 160)(this);
	}
	float* GetLowerBodyYawTarget() /*m_flLowerBodyYawTarget*/
	{
		return (float*)((DWORD)this + NetVarManager->GetOffset("DT_CSPlayer", "m_flLowerBodyYawTarget"));
	}
	bool IsPlayer()
	{
		typedef bool(__thiscall *IsPlayer_t)(PVOID);
		return ((IsPlayer_t)(*(PDWORD)(*(PDWORD)(this) + 0x260)))(this);
	}
	CBasePlayerAnimState* GetBasePlayerAnimState()
	{
		static int basePlayerAnimStateOffset = 0x3884;
		return *(CBasePlayerAnimState**)((DWORD)this + basePlayerAnimStateOffset);
	}
	Vector GetBonePos(int i)
	{
		matrix3x4 boneMatrix[128];
		if (this->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, static_cast<float>(GetTickCount64())))
		{
			return Vector(boneMatrix[i][0][3], boneMatrix[i][1][3], boneMatrix[i][2][3]);
		}
		return Vector(0, 0, 0);
	}

	Vector GetHeadPos()
	{
		return this->GetBonePos(8);
	}
	Vector GetEyePosition() {
		Vector Origin = *(Vector*)((DWORD)this + 0x00000134);
		Vector View = *(Vector*)((DWORD)this + 0x00000104);
		return(Origin + View);
	}

	int draw_model(int flags, uint8_t alpha) {
		using fn = int(__thiscall*)(void*, int, uint8_t);
		return call_vfunc< fn >(GetRenderable(), 9)(GetRenderable(), flags, alpha);
	}
	void SetAngle2(Vector wantedang) {
		typedef void(__thiscall* oSetAngle)(void*, const Vector &);
		static oSetAngle _SetAngle = (oSetAngle)((uintptr_t)U::FindSig("client_panorama.dll", "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
		_SetAngle(this, wantedang);
	}

};

class CCSBomb : public C_BaseEntity
{
public:

	HANDLE GetOwnerHandle()
	{
		return *(HANDLE*)((uintptr_t)this + offsetz.DT_BaseCombatWeapon.m_hOwner);
	}

	float GetC4BlowTime()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_PlantedC4.m_flC4Blow);
	}

	float GetC4DefuseCountDown()
	{
		return *(float*)((uintptr_t)this + offsetz.DT_PlantedC4.m_flDefuseCountDown);
	}

	int GetBombDefuser()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlantedC4.m_hBombDefuser);
	}

	bool IsBombDefused()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_PlantedC4.m_bBombDefused);
	}
};


class CPlayerResource {};

class C_CSPlayerResource : public CPlayerResource
{
public:
	int GetPing(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iPing + index * 4);
	}

	int GetKills(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iKills + index * 4);
	}

	int GetAssists(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iAssists + index * 4);
	}

	int GetDeaths(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iDeaths + index * 4);
	}

	bool GetConnected(int index)
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_PlayerResource.m_bConnected + index);
	}

	TeamID GetTeam(int index)
	{
		return *(TeamID*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iTeam + index * 4);
	}

	int GetPendingTeam(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iPendingTeam + index * 4);
	}

	bool GetAlive(int index)
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_PlayerResource.m_bAlive + index);
	}

	int GetHealth(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_PlayerResource.m_iHealth + index * 4);
	}

	int GetPlayerC4()
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iPlayerC4);
	}

	int GetMVPs(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iMVPs + index * 4);
	}

	int GetArmor(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iArmor + index * 4);
	}

	int GetScore(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iScore + index * 4);
	}

	int* GetCompetitiveRanking(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iCompetitiveRanking + index * 4);
	}

	int* GetCompetitiveWins(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iCompetitiveWins + index * 4);
	}

	int GetCompTeammateColor(int index)
	{
		return *(int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_iCompTeammateColor + index * 4);
	}

	const char* GetClan(int index)
	{
		return (const char*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_szClan + index * 16);
	}

	int* GetActiveCoinRank(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_nActiveCoinRank + index * 4);
	}

	int* GetMusicID(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_nMusicID + index * 4);
	}

	int* GetPersonaDataPublicCommendsLeader(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_nPersonaDataPublicCommendsLeader + index * 4);
	}

	int* GetPersonaDataPublicCommendsTeacher(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_nPersonaDataPublicCommendsTeacher + index * 4);
	}

	int* GetPersonaDataPublicCommendsFriendly(int index)
	{
		return (int*)((uintptr_t)this + offsetz.DT_CSPlayerResource.m_nPersonaDataPublicCommendsFriendly + index * 4);
	}
};

class C_CSGameRules
{
public:
	bool IsValveDS()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSGameRulesProxy.m_bIsValveDS);
	}

	bool IsBombDropped()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSGameRulesProxy.m_bBombDropped);
	}

	bool IsBombPlanted()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSGameRulesProxy.m_bBombPlanted);
	}

	bool IsFreezeTime()
	{
		return *(bool*)((uintptr_t)this + offsetz.DT_CSGameRulesProxy.m_bFreezePeriod);
	}
};

class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

class IGameConsole : public IBaseInterface
{
public:
	// activates the console, makes it visible and brings it to the foreground
	virtual void Activate() = 0;

	virtual void Initialize() = 0;

	// hides the console
	virtual void Hide() = 0;

	// clears the console
	virtual void Clear() = 0;

	// return true if the console has focus
	virtual bool IsConsoleVisible() = 0;

	virtual void SetParent(int parent) = 0;
};

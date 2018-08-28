#pragma once
#include <set>
#include <map>
#include <unordered_map>


struct skinInfo
{
	int seed = -1;
	int paintkit;
	std::string tagName;
};


//Color Feature for Color Changer
struct ColorP
{
public:
	const char* Name;
	float* Ccolor;

	ColorP(const char* name, float* color)
	{
		this->Name = name;
		this->Ccolor = color;
	}
};

struct Variables
{
	Variables()
	{

	}

	struct Ragebot_s
	{
		bool	MainSwitch;
		bool 	Enabled;
		bool 	AutoFire;
		float 	FOV;
		bool 	Silent;
		bool	AutoPistol;
		int		KeyPress;
		bool	AimStep;
		bool	BreakLBY;
		float	LBYDelta;

		

		int flip_aa;

		bool ayywarecrasher = false;

		bool	EnabledAntiAim;
		int		SubAATabs;
		int		Pitch;
		int		YawTrue;
		int		YawFake;
		int		YawTrueMove;
		float	PitchAdder;
		float	YawTrueAdder;
		float	YawFakeAdder;
		float	YawFakeMove;
		bool	AtTarget;
		bool	KnifeAA;
		// Pitch  PitchAdder  YawTrue  YawFakeAdder  YawFake  YawFakeAdder

		//walking prebuilt aa
		bool	walk_PreAAs;
		bool	walk_LBYBreak;
		int		walk_Pitch;
		float	walk_PitchAdder;
		int		walk_YawTrue;
		float	walk_YawTrueAdder;
		int		walk_YawFake;
		float	walk_YawFakeAdder;

		//standing prebuilt aa's 
		bool	stand_PreAAs;
		bool	stand_LBYBreak;
		int		stand_Pitch;
		float	stand_PitchAdder;
		int		stand_YawTrue;
		float	stand_YawTrueAdder;
		int		stand_YawFake;
		float	stand_YawFakeAdder;

		//air prebuilt aa's 
		bool	air_PreAAs;
		bool	air_LBYBreak;
		int		air_Pitch;
		float	air_PitchAdder;
		int		air_YawTrue;
		float	air_YawTrueAdder;
		int		air_YawFake;
		float	air_YawFakeAdder;

		bool stand_allowflip;
		bool fwalk_allowflip;
		bool crouch_allowflip;
		bool walk_allowflip;
		bool run_allowflip;

		int flipkey;

		int error_type;  //aa warning variable

		bool	FriendlyFire;
		int		Hitbox;
		int		Hitscan;
		float	Pointscale;
		bool	Multipoint;
		float	Multipoints;

		bool	AntiRecoil;
		bool	AutoWall;
		int 	AutoStop;
		bool	AutoCrouch;
		bool	AutoScope;
		float	MinimumDamageSniper;
		float	MinimumDamageRifle;
		float	MinimumDamagePistol;
		float	MinimumDamageHeavy;
		float	MinimumDamageSmg;
		float	MinimumDamageRevolver;
		bool	Hitchance;
		float	HitchanceSniper;
		float	HitchancePistol;
		float	HitchanceRifle;
		float	HitchanceHeavy;
		float	HitchanceSmgs;
		float	HitchanceRevolver;
		bool	Resolver;
		bool	airlbyplus119;
		bool	playerlist;
		float	bruteAfterX;
		int manualkey;

		bool	AA_onWalk;
		bool	AA_onStand;
		bool	AA_onAir;

		bool fakewalk;
		int fakewalkkey;

		bool BAIMIfLethal;
		bool Backtrack;
	} Ragebot;

	

	struct
	{
		bool Enable;
		bool AutoPistol;
		bool EnableLegitAA;
		float AAAngle;
		bool ManualAA;
		int Up = 1;
		int Down = 1;
		int Left = 1;
		int Right = 1;

		int MainKey = 1;
		float MainSmooth = 1;
		float Mainfov;
		float main_random_smooth;
		float main_recoil_min;
		float main_recoil_max;
		float main_randomized_angle;
		float mainrcs;


		int PistolKey = 1;
		float Pistolfov;
		float PistolSmooth = 1;;
		float pistol_random_smooth;
		float pistol_recoil_min;
		float pistol_recoil_max;
		float pistol_randomized_angle;
		float pistolrcs;


		int SniperKey = 1;
		float Sniperfov;
		float SniperSmooth = 1;
		float sniper_random_smooth;
		float sniper_recoil_min;
		float sniper_recoil_max;
		float sniper_randomized_angle;
		float sniperrcs;

		int smg_Key = 1;
		float smg_fov;
		float smg_Smooth = 1;
		float smg_random_smooth;
		float smg_recoil_min;
		float smg_recoil_max;
		float smg_randomized_angle;
		float smgrcs;

		int heavy_wp_Key = 1;
		float heavy_wp_fov;
		float heavy_wp_Smooth;
		float heavy_wp_random_smooth;
		float heavy_wp_recoil_min;
		float heavy_wp_recoil_max;
		float heavy_wp_randomized_angle;
		float heavyrcs;
		int riflehitbox;
		int pistolhitbox;
		int sniperhitbox;
		int smghitbox;
		int heavyhitbox;

		bool ResolverLegit;

		struct
		{
			bool Enabled;
			float Delay;
			int Key = 6;
			float hitchance;
			struct
			{
				bool Head;
				bool Arms;
				bool Chest;
				bool Stomach;
				bool Legs;
			} Filter;

		} Triggerbot;


	}LegitBot;

	struct
	{
		bool backtrackenable;
		int BacktrackType;
		int  backtrackticks = 1;


	}Backtrack;





	struct
	{
		bool Enabled;
		bool StreamProof;
		
		bool LBYTimer;
		bool TeamESP;
		bool Box;
		int BoxType;
		int BoxHotkey;
		bool fill;
		float esp_fill_amount;
		float spread_crosshair_amount = 110;
		int healthtype;
		bool health;
		bool armor;
		int Armor2;

		bool Money;
		bool Name;
		bool skeletonenbl;
		int skeletonopts;
		int Weapon;
		bool AimLine;
		bool angleLines;
		bool angleLinesName;
		bool barrel;
		int barrelL;
		bool DrawAwall;
		bool LBYIndicator;
		bool RageDraw;
		bool C4World;
		bool resolveMode;
		int DroppedGunsType;
		bool DroppedGuns;
		bool noscopeborder;

		bool AmmoBox;


         //Info Enemy
		bool BombCarrier;
		bool Flashed;
		bool Distance;
		bool Scoped;
		bool IsHasDefuser;
		bool IsDefusing;

		//GRENADE
		int Grenades;
		bool GrenadePrediction;
		bool GrenadeBox;

		bool EdgyTPThing;

		//CHAMS
		bool Chams;
		bool Teamchams;
		int champlayeralpha = 100;
		bool XQZ;

		int Hands;
		int HandsAlpha = 100;
		int chamswphands;
		int chamswphandsalpha = 100;
		bool FakeAngleChams;

		//CROSSHAIR
		bool RecoilCrosshair;
		int RecoilCrosshair2;
		bool SpreadCrosshair;
		bool SniperCrosshair;
		int SniperCrosshairType;
		bool AWallCrosshair;

	

		//REMOVERS
		bool NoVisualRecoil;
		bool NoFlash;
		int Smoke;
		bool NoPostProcess;


		//VIEWMODEL CHANGERS
		bool viewmodelChanger_enabled;
		bool FOVChanger_enabled;
		float FOVChanger;
		float viewmodelChanger = 68;

		//WORLD
		bool nightMode;
		bool snowmode;
		bool lsdmode;
		bool chromemode;
		float wallalpha = 1.f;
		float propalpha = 1.f;
		float modelalpha = 1.f;
		bool AsusProps;
		bool minecraftmode;
		bool ambientlight;
		int SkyboxChanger;
		bool OffscreenIndicator;
		int OffscreenIndicatorSize = 7;
		bool Hostage;
		bool HostageBox;
		bool Chicken;
		bool ChickenBox;
		bool WeaponBox;

		//ThirdPerson
		int TPKey;
		bool Enabletp;
		bool GrenadeCheck;
		int ThirdpersonMode;
		int antiaim_thirdperson_angle;

		//GLOW
		bool GlowEnable;
		bool GlowPlayerEnable;
		bool GlowEnemy;
		bool GlowTeam;
		float EnemyAlpha = 255;
		float TeamAlpha = 255;
		bool GlowWeaponsEnable;
		bool GlowC4Enable;
		float C4GlowAlpha = 255;
		float WeaponsGlowAlpha = 255;

		//OTHERS
	    bool DamageIndicator;
		bool bulletshow;
		float flTracersDuration;
		float flTracersWidth;
		int   Beamtype;
		bool Hitbox;
		float HitboxDuration;
		
		bool ManualAAIndicator;
		bool ScopedBlurRemoval;
	} Visuals;
	struct
	{
		bool antiuntrusted = true;
		bool syncclantag;
		bool Bhop;
		int spammer;
		bool spammeron;
		int AutoStrafe;
		bool SpecList;
		bool Watermark = true;
		bool Hitmarker;
		int Hitsound = 0;
		bool afkbot;
		bool inventoryalwayson;
		bool eventlogs;
		bool eventlogs2;
		bool radaringame;
		bool radarwindow;
		int radrsize = 200;
		float radralpha = 1.f;
		float radrzoom = 2.f;
		bool mediamode;
		bool BuyBot;
		bool BuyBotZeus;
		int BuyBotWeap;
		int BuyBotGrenade;
		bool BuyBotDefuse;
		int BuyBotKevlarHelmet;
		bool FakePing;
		int FakePingKey;
		int FakePingType;
		int FakePingAmmnt;
		bool models;
		int knifemodel;
	} Misc;
	
	struct
	{
		bool Opened = false;
		bool mainopened = false;
		int 	Key;
		int		ConfigFile = 0;
		int		Theme = 0;
		short currentWeapon;
		int MenuTab = 1;
		int AimBotTab = 1;
		int RageTab = 1;
		int VisualsTab = 1;
		int LegitTab = 1;
		int MiscTab = 1;
	} Menu;

	struct
	{//need more
		float RedColor[3] = { 1.f, 0, 0 };
		float GreenColor[3] = { 0, 1.f, 0 };
		float BlueColor[3] = { 0, 0, 1.f };

		bool ColorSkybox;
		int SkyColor;
		float TeamESP[3] = { 0, 1.f, 0 };
		float EnemyESP[3] = { 1.f, 1.f, 1.f };
		float EnemyChamsVis[3] = { 1.f, 0, 1.f };
		float EnemyChamsNVis[3] = { 1.f, 0, 1.f };
		float TeamChamsVis[3] = { 0, 1.f, 0 };
		float TeamChamsNVis[3] = { 0, 1.f, 0 };
		float hitmarker_color[3] = { 1.f, 1.f, 1.f };
		float backtrackdots_color[3] = { 1.f, 1.f, 1.f };
		float dlight_color[3] = { 1.f, 1.f, 1.f };
		float color_grenadeprediction[3] = { 1.f, 1.f, 1.f };// line
		float color_grenadeprediction_circle[3] = { 1.f, 0.f, 0.f };// circle
		float GrenadeCollision[3] = { 1.f, 0.f, 0.f };// box
		float color_skeleton[3] = { 1.f, 1.f, 1.f };
		float color_recoil[3] = { 1.f, 1.f, 1.f };
		float color_spread[3] = { 1.f, 1.f, 1.f };
		float color_sniper[3] = { 1.f, 1.f, 1.f };
		float EnemyGlow[3] = { 1.f, 1.f, 1.f };
		float TeamGlow[3] = { 1.f, 1.f, 1.f };
		float OtherGlow[3] = { 1.f, 1.f, 1.f };
		float HandsColor[3] = { 1.f, 1.f, 1.f };
		float AimLineColor[3] = { 1.f, 1.f, 1.f };
		float BulletTraceColor[3] = { 1.f, 1.f, 1.f };
		float fill_color_enemy[3] = { 0.f, 0.f, 0.0f};
		float fill_color_team[3] = { 0.f, 0.f, 0.f};
		float glow_weapon[3] = { 1.f, 1.f, 1.f };
		float glow_c4[3] = { 1.f, 1.f, 1.f };
    	float damageindicator[3] = { 1.f,1.f,1.f };
		float WeaponColor[3] = { 1.f, 1.f, 1.f };
		float droppedguns [3] = { 1.f,1.f,1.f };
		float ScopeLine[3] = { 0.f,0.f,0.f };
		float flTracers[3] = { 0.f, 0.f, 1.f };
		float ambientlightcolor[3] = { 1.f, 1.f, 1.f };
		float Offscreen[3] = { 1.f, 0.f, 0.f };
		float hitbox[3] = { 1.f, 1.f, 1.f };
		float FakeAngleChams[3] = { 1.f, 1.f, 1.f };
		float ManualIndicator[3] = { 1.f, 0.f, 1.f };
	}Colors;
};

extern Variables g_Options;
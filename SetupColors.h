#pragma once

#include "Global.h"

//Colors
void SetupColors()
{
	//ESP

	Global::ColorsForPicker1.push_back(ColorP("Team - Box", g_Options.Colors.TeamESP));
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Box", g_Options.Colors.EnemyESP));
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Fill-Box", g_Options.Colors.fill_color_enemy));
	Global::ColorsForPicker1.push_back(ColorP("Team - Fill-Box", g_Options.Colors.fill_color_team));


	//CHAMS
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Visible Chams", g_Options.Colors.EnemyChamsVis));
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Invisible Chams", g_Options.Colors.EnemyChamsNVis));
	Global::ColorsForPicker1.push_back(ColorP("Team - Visible Chams", g_Options.Colors.TeamChamsVis));
	Global::ColorsForPicker1.push_back(ColorP("Team - Invisible Chams", g_Options.Colors.TeamChamsNVis));
	Global::ColorsForPicker1.push_back(ColorP("Local - Fake Angle Chams", g_Options.Colors.FakeAngleChams));
	//GLOW
	Global::ColorsForPicker1.push_back(ColorP("Enemy - Glow", g_Options.Colors.EnemyGlow));
	Global::ColorsForPicker1.push_back(ColorP("Team - Glow", g_Options.Colors.TeamGlow));

	Global::ColorsForPicker1.push_back(ColorP("Enemy - Hitbox", g_Options.Colors.hitbox));

	Global::ColorsForPicker1.push_back(ColorP("Local - Hands", g_Options.Colors.HandsColor));
	Global::ColorsForPicker1.push_back(ColorP("Local - Weapons", g_Options.Colors.WeaponColor));

	Global::ColorsForPicker1.push_back(ColorP("World - Dropped Weapons", g_Options.Colors.droppedguns));

	Global::ColorsForPicker1.push_back(ColorP("Hitmarker", g_Options.Colors.hitmarker_color));
	Global::ColorsForPicker1.push_back(ColorP("Dynamic Light", g_Options.Colors.dlight_color));
	Global::ColorsForPicker1.push_back(ColorP("Skeleton", g_Options.Colors.color_skeleton));
	Global::ColorsForPicker1.push_back(ColorP("BackTrack History", g_Options.Colors.backtrackdots_color));
	Global::ColorsForPicker1.push_back(ColorP("Sniper Crosshair", g_Options.Colors.color_sniper));
	Global::ColorsForPicker1.push_back(ColorP("Recoil Crosshair", g_Options.Colors.color_recoil));
	Global::ColorsForPicker1.push_back(ColorP("Spread Crosshair", g_Options.Colors.color_spread));
	Global::ColorsForPicker1.push_back(ColorP("Grenade-Prediction Line", g_Options.Colors.color_grenadeprediction));
	Global::ColorsForPicker1.push_back(ColorP("Grenade-Prediction Circle", g_Options.Colors.color_grenadeprediction_circle));
	Global::ColorsForPicker1.push_back(ColorP("Grenade-Prediction Box", g_Options.Colors.GrenadeCollision));
	Global::ColorsForPicker1.push_back(ColorP("Offscreen Indicator", g_Options.Colors.Offscreen));
	Global::ColorsForPicker1.push_back(ColorP("Aim Direction", g_Options.Colors.BulletTraceColor));
	Global::ColorsForPicker1.push_back(ColorP("AimLine", g_Options.Colors.AimLineColor));
	Global::ColorsForPicker1.push_back(ColorP("No-Scope Line", g_Options.Colors.ScopeLine));
	Global::ColorsForPicker1.push_back(ColorP("Damage Indicator", g_Options.Colors.damageindicator));
	Global::ColorsForPicker1.push_back(ColorP("Bullet Tracers", g_Options.Colors.flTracers));
	Global::ColorsForPicker1.push_back(ColorP("AmbientLight", g_Options.Colors.ambientlightcolor));
	Global::ColorsForPicker1.push_back(ColorP("Manual AA Indicator", g_Options.Colors.ManualIndicator));
}
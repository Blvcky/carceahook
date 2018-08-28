#pragma once
#include "SDK.h"

struct lbyRecords
{
	int tick_count;
	float lby;
	Vector HeadPosition;
};
struct backtrackData
{
	float simtime;
	Vector HitboxPos;
};

class BackTrack
{
	int latest_tick;
	void UpdateRecord(int i);
public:

	lbyRecords records[64];
	bool IsTickValid(int tick);
	bool RunLBYBackTrack(int i, CInput::CUserCmd* cmd, Vector& aimPoint);
	void Update(int tick_count);
	void legitBackTrack(CInput::CUserCmd* cmd, C_BaseEntity* pLocal);
	void RageBackTrack(CInput::CUserCmd* cmd, C_BaseEntity* pLocal);
};

extern backtrackData headPositions[64][12];

extern BackTrack* backtracking;
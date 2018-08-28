#include "Render.h"
#include "SDK.h"

struct DamageIndicator_t {
	int iDamage;
	bool bInitialized;
	float flEraseTime;
	float flLastUpdate;
	C_BaseEntity * Player;
	Vector Position;
};

class DamageIndicators {
public:
	std::vector<DamageIndicator_t> data;
	void paint();
};
extern DamageIndicators damage_indicators;

#pragma once
#include <cstdint>

class CBaseClientState
{
public:
	void ForceFullUpdate()
	{
		*(uint32_t*)((uintptr_t)this + 0x174) = -1;
	};
};

#include <array>
#include <string>
#include <deque>
#include <algorithm>
#include "Entities.h"
#include "CommonIncludes.h"
#include "Entities.h"
#include "Vector.h"
#include <map>
#include "Interfaces.h"
#include "Hooks.h"


class Resolver
{
public:
	bool didhit;
	void CallFSN();
	void ResolveYaw(C_BaseEntity* pEntity);
	static Resolver Instance()
	{
		static Resolver instance;
		return instance;
	}
};
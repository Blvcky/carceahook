#pragma once
#include <chrono>

namespace NameChanger
{
	extern int changes;
	std::string GetName();
	void SetName(const char* name);
};



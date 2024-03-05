#pragma once
#include "../valve_sdk/csgostructs.hpp"

struct CUserCmd;

namespace Triggerbot
{
	void run(CUserCmd*) noexcept;
	float serverTime(CUserCmd*) noexcept;
	bool IsLineGoesThroughSmoke(Vector, Vector);
}
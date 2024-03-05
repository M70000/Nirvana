/*#include "../valve_sdk/csgostructs.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../helpers/math.hpp"
#include "../options.hpp"
#include "Autoblock.h"

void BlockBot::AutoBlock(CUserCmd* cmd)
{
	if (!g_Options.block_bot)
		return;

	if (!GetAsyncKeyState(g_Options.blockbot_key))
		return;

	C_BasePlayer* localplayer = g_LocalPlayer;
	float bestdist = 200.f;
	int index = -1;

	for (int i = 1; i < 64; i++)
	{
		C_BasePlayer* entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!entity)
			continue;

		if (!entity->IsAlive() || entity->IsDormant() || entity == localplayer)
			continue;

		float dist = localplayer->m_angAbsOrigin().DistTo(entity->m_angAbsOrigin());

		if (dist < bestdist)
		{
			bestdist = dist;
			index = i;
		}
	}

	if (index == -1)
		return;

	C_BasePlayer* target = (C_BasePlayer*)g_EntityList->GetClientEntity(index);

	if (!target)
		return;

	if (target)
	{

		QAngle angles = Math::CalcAngle(localplayer->m_angAbsOrigin(), target->m_angAbsOrigin());

		QAngle shit;

		g_EngineClient->GetViewAngles(&shit);

		angles.yaw -= shit.yaw;
		Math::FixAngles(angles);

		if (angles.yaw < 0.20f)
			cmd->sidemove = 450.f;
		else if (angles.yaw > 0.20f)
			cmd->sidemove = -450.f;
	}
}

BlockBot g_BlockBot;*/
#include "legitresolver.h"

LegitResolver legitresolver;

void LegitResolver::Createmove(C_BasePlayer* pEntity)
{

	auto local = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));

	for (int x = 1; x < g_GlobalVars->maxClients; x++)
	{

		pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == local
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

	}
}

void LegitResolver::FrameStageNotify(int stage)
{

	auto local = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));


	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < g_GlobalVars->maxClients; i++)
		{

			auto pEntity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));

			if (!pEntity
				|| pEntity == local
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;

			LegitResolver::Resolve(pEntity);
		}
	}
}

void LegitResolver::Resolve(C_BasePlayer* pEntity)
{
	auto local = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
	if (!local || !local->IsAlive() || !pEntity || pEntity->IsDormant()) return;
	auto idx = pEntity->EntIndex();
	auto new_yaw = pEntity->m_flLowerBodyYawTarget();
	auto new_pitch = FLT_MAX;

	static auto old_pitch = FLT_MAX;

	auto state = pEntity->GetPlayerAnimState();
	auto yaw = pEntity->m_angEyeAngles().yaw;
	auto entity_lby_update = pEntity->Animating();
	auto resolve = yaw;
	auto lby = pEntity->m_flLowerBodyYawTarget();

	if (g_Options.legit_resolver)
	{
		//auto anim_layer3 = player->GetAnimationLayer(3);
		//auto breaker_desync_anim_detection = anim_layer3 && anim_layer3->flWeight == 0.0f &&;// i don't remember exact values rn, let me check later

		if (lby > 25)
		{
			resolve = yaw + 60.f;
		}

		if (lby < 25)

		{
			resolve = yaw - 60.f;
		}
	}
}
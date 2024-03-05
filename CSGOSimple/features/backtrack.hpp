#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include <map>
#include <deque>

struct backtrack_data {
	backtrack_data()
	{
		HeadPosition.Init();
		simTime = -1.f;
		MatrixBuilt = false;
	}
	backtrack_data(C_BasePlayer* Player)
	{
		HeadPosition = Player->GetHitboxPos(HITBOX_HEAD);
		simTime = Player->m_flSimulationTime();
		MatrixBuilt = false;
		if (Player->SetupBones(boneMatrix, 128, BONE_USED_BY_ANYTHING, g_GlobalVars->curtime))
			MatrixBuilt = true;
	}

	Vector HeadPosition;
	bool MatrixBuilt;
	float simTime;
	Vector hitboxPos;
	matrix3x4_t boneMatrix[128];
};

struct IncomingSequence
{
	int inreliablestate;
	int sequencenr;
	float servertime;
};

static std::deque< IncomingSequence > sequences;

class Backtrack {
public:
	void OnMove(CUserCmd *pCmd);
	void AddLatencyToNetwork(INetChannel*, float) noexcept;
	void UpdateIncomingSequences(bool reset = false) noexcept;

	float correct_time = 0.0f;
	float latency			 = 0.0f;
	float lerp_time		 = 0.0f;	
	std::map<int, std::deque<backtrack_data>> data;

	/*constexpr auto getExtraTicks() noexcept
	{
		auto network = g_EngineClient->GetNetChannelInfo();
		if (!network)
			return 0.f;

		return std::clamp(network->GetLatency(1) - network->GetLatency(0), 0.f, 0.2f);
	}*/
};
extern Backtrack g_Backtrack;
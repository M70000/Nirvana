#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
class Aimbot {
public:
	void OnMove(CUserCmd *pCmd);
	bool IsEnabled(CUserCmd *pCmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);
	legitbot_settings settings;

	bool IsRcs();
	float GetSmooth();
	float GetFov();

	void LegitAA_Packets(CUserCmd* cmd, bool& send_packet);
	void Update_LBY_Breaker();
	void LegitAA_Run(CUserCmd* cmd, bool& send_packet);
	void FakeBackwards(CUserCmd* cmd);
	void FakeBackIndicator();

	bool break_lby = false;
	float next_update = 0;
	bool desync_side = false;
private:
	void RCS(QAngle &angle, C_BasePlayer* target, bool should_run);
	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool IsNotSilent(float fov);
	float serverTime(CUserCmd* cmd) noexcept;
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int &bestBone);
	bool silent_enabled = false;
	QAngle CurrentPunch = { 0,0,0 };
	QAngle RCSLastPunch = { 0,0,0 };
	bool is_delayed = false;
	int kill_delay_time;
	bool kill_delay = false;
	C_BasePlayer* target = NULL;
};
extern Aimbot g_Aimbot;

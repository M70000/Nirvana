#include "bhop.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../helpers/math.hpp"
#include "../options.hpp"
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <regex>
#include "../helpers/input.hpp"
#include "../dirent.h"
#include "../render.hpp"

namespace
{
#include "engine_prediction.hpp"
}

#define D3DX_PI    ((FLOAT)  3.141592654f)

/*
void BunnyHop::CopyBOTMovement(CUserCmd* cmd)
{
	if (!g_Options.misc_copybot)
		return;

	for (size_t i = 0; i < g_EngineClient->GetMaxClients(); i++) {
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player)
			continue;

		if (!player->IsAlive() || player->IsDormant() || player == g_LocalPlayer)
			continue;

		if (cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT)
			return;

		int head_height = player->GetPlayerAnimState()->m_fDuckAmount > 0 ? 46 : 64;
		auto is_directly_above = g_LocalPlayer->m_vecOrigin().DistTo(Vector(player->m_vecOrigin().x, player->m_vecOrigin().y, player->m_vecOrigin().z + head_height)) < 32.f;
		auto is_above = g_LocalPlayer->m_vecOrigin().z > (player->m_vecOrigin().z + head_height);
		if (is_above && is_directly_above)
		{
			auto velocity = player->m_vecVelocity();
			auto speed = velocity.Length();
			auto direction = velocity.Angle();

			direction.y = cmd->viewangles.yaw - direction.y;
			Vector forward;

			QAngle q_dir = QAngle(direction.x, direction.y, direction.z);
			Math::AngleVectors(q_dir, forward);

			cmd->forwardmove = forward.x * speed;
			cmd->sidemove = forward.y * speed;
		}
	}
}*/

void BunnyHop::Autoalign(CUserCmd* cmd) {
	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive()) return;

	// Movement checks
	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP) return;

	Vector wishdir;
	bool done = false;

	if ((cmd->buttons & IN_FORWARD) && !(cmd->buttons & IN_BACK) && !(cmd->buttons & IN_MOVELEFT) && !(cmd->buttons & IN_MOVERIGHT)) {
		wishdir = { std::cos(DEG2RAD2(cmd->viewangles.yaw)) * 17.f, std::sin(DEG2RAD2(cmd->viewangles.yaw)) * 17.f, 0.f };
		done = true;
	}
	if ((cmd->buttons & IN_BACK) && !(cmd->buttons & IN_FORWARD) && !(cmd->buttons & IN_MOVELEFT) && !(cmd->buttons & IN_MOVERIGHT)) {
		wishdir = { std::cos(DEG2RAD2(cmd->viewangles.yaw + 180.f)) * 17.f, std::sin(DEG2RAD2(cmd->viewangles.yaw + 180.f)) * 17.f, 0.f };
		done = true;
	}
	if ((cmd->buttons & IN_MOVELEFT) && !(cmd->buttons & IN_BACK) && !(cmd->buttons & IN_FORWARD) && !(cmd->buttons & IN_MOVERIGHT)) {
		wishdir = { std::cos(DEG2RAD2(cmd->viewangles.yaw + 90.f)) * 17.f, std::sin(DEG2RAD2(cmd->viewangles.yaw + 90.f)) * 17.f, 0.f };
		done = true;
	}
	if ((cmd->buttons & IN_MOVERIGHT) && !(cmd->buttons & IN_BACK) && !(cmd->buttons & IN_MOVELEFT) && !(cmd->buttons & IN_FORWARD)) {
		wishdir = { std::cos(DEG2RAD2(cmd->viewangles.yaw - 90.f)) * 17.f, std::sin(DEG2RAD2(cmd->viewangles.yaw - 90.f)) * 17.f, 0.f };
		done = true;
	}

	if (!done)
		return;

	const auto startPos = g_LocalPlayer->m_angAbsOrigin();
	const auto endPos = startPos + wishdir;

	trace_t trace;
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;
	Ray_t ray;
	ray.Init(startPos, endPos);
	g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);
	if ((trace.fraction < 1.f) && (trace.plane.normal.z == 0.f)) {
		Vector Angles{ trace.plane.normal.x * -16.005f, trace.plane.normal.y * -16.005f, 0.f };
		const auto endPos2 = startPos + Angles;
		trace_t trace2;
		Ray_t ray2;
		ray.Init(startPos, endPos2);
		g_EngineTrace->TraceRay(ray2, MASK_PLAYERSOLID, &filter, &trace2);
		if (trace.plane != trace2.plane) {
			Vector to_wall = Angles.toAngle();
			float mVel = hypotf(g_LocalPlayer->m_vecVelocity().x, g_LocalPlayer->m_vecVelocity().y);
			float ideal = RAD2DEG(atanf(28.33f / mVel));
			Vector dvelo = g_LocalPlayer->m_vecVelocity();
			dvelo.z = 0.f;
			Vector velo_angle = dvelo.toAngle();
			Vector delta = velo_angle - to_wall;
			delta.Normalized();
			if (delta.y >= 0.f)
				to_wall.y += ideal;
			else
				to_wall.y -= ideal;
			float rotation = DEG2RAD2(to_wall.y - cmd->viewangles.yaw);
			float cos_rot = cos(rotation);
			float sin_rot = sin(rotation);

			float forwardmove = cos_rot * 450.f;
			float sidemove = -sin_rot * 450.f;
			cmd->forwardmove = forwardmove;
			cmd->sidemove = sidemove;
		}
	}
}

void BunnyHop::FastStop(CUserCmd* cmd)
{
	if (!g_Options.misc_faststop)
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || !(g_LocalPlayer->m_fFlags() & 1) || cmd->buttons & IN_JUMP)
		return;

	if (cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))
		return;

	const auto velocity = g_LocalPlayer->m_vecVelocity();
	const auto speed = velocity.Length2D();
	if (speed < 15.0f)
		return;

	Vector direction = velocity.toAngle();
	direction.y = cmd->viewangles.yaw - direction.y;

	const auto negatedDirection = Vector::fromAngle(direction) * -speed;
	cmd->forwardmove = negatedDirection.x;
	cmd->sidemove = negatedDirection.y;

}

// Edgebug variables
bool edgebugging, ducking = false;
int tick, eb_counter = 0;
float last_set = 0;
Vector eb_pos;

// Edgebug detected
bool BunnyHop::EdgebugDetected()
{
	return edgebugging;
}

// Edgebug lock
void BunnyHop::EdgebugLock(CUserCmd* cmd) {
	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive()) return;

	// Is in edgebug
	if (edgebugging) 
	{
		
	//	EnginePrediction::restore(g_Prediction->Split->nCommandsPredicted - 1);
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND || tick < cmd->tick_count) {
			//eb_counter++;
			if (g_Options.misc_ebugdetector)
			{
				g_ChatElement->ChatPrintf(0, 0, std::string("").
					append("\x01"). //Default Color
					append("[").
					append("\x0B"). //Blue
					append("Nirvana").
					append("\x01"). //Default Color
					append("]").
					append(" \x0C"). //Dark Blue 
					append("edgebugged assisted").c_str());

				// Play sound
				g_VGuiSurface->PlaySound_("survival\\money_collect_05.wav");

				// Spark effect
				if (g_Options.esp_detectionsparks) g_LocalEffects->Sparks(g_LocalPlayer->m_vecOrigin(), 4, 4);
				// Boost effect
				if (g_Options.esp_detectionhealthboost) g_LocalPlayer->health_boost_time() = g_GlobalVars->curtime + 0.5f;
				// Beam effect
				if (g_Options.esp_detectionbeam)  Render::Get().DrawBeamCircle(Color(255, 255, 255), 3, eb_pos);
			}
			// Release movement
			last_set = g_GlobalVars->curtime + 3.0f;
			edgebugging = false;
			return;
		}

		// No movement for you
		cmd->sidemove = 0.f;
		cmd->forwardmove = 0.f;
		cmd->upmove = 0.f;
		cmd->mousedx = 0.f;

		// Lock movement
		if (ducking)
		{
			cmd->buttons |= IN_DUCK; // Force duck
		}
		else 
		{
			cmd->buttons &= ~IN_DUCK; // Force stand
		}
		return;
	}
}

void BunnyHop::EdgebugRun(CUserCmd* cmd)
{
	// Keybind check
	if (!g_Options.misc_ebug && !GetAsyncKeyState(g_Options.misc_ebug_key)) return;

	// Return if we are edgebugging
	if (edgebugging) return;

	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive()) return;

	// Check for edgebug
	if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP) || g_LocalPlayer->m_vecVelocity().z >= 0) return;

	for (int s = 0; s < 2; s++) {
		// Break if found in first scenario
		if (edgebugging) break;

		//Restore to first tick
		//EnginePrediction::restore(g_Prediction->Split->nCommandsPredicted - 1); //THIS IS COMMENTED BECAUSE IT'S MAKING THE GAME CRASH ON THE PATTERN SCAN, IT COULD BE THE CAUSE

		int flags = g_LocalPlayer->m_fFlags();
		Vector velocity = g_LocalPlayer->m_vecVelocity();

		for (int i = 0; i < g_Options.misc_ebugalignticks; i++) {
			CUserCmd* scenario = new CUserCmd(*cmd);

			// Send movement for scenario
			scenario->sidemove = 0.f;
			scenario->forwardmove = 0.f;
			scenario->upmove = 0.f;
			scenario->mousedx = 0.f;

			if (s == 0) {
				scenario->buttons |= IN_DUCK;
			}
			else {
				scenario->buttons &= ~IN_DUCK;
			}

			// Start the prediction
			EnginePrediction::Begin(scenario);
		//	prediction::start(scenario);

			if (floor(velocity.z) < -7 && floor(g_LocalPlayer->m_vecVelocity().z) == -7 && !(flags & FL_ONGROUND)) {
				// Edgebug detected
				tick = cmd->tick_count + i;
				ducking = (s == 0);
				eb_pos = g_LocalPlayer->m_vecOrigin();
				edgebugging = true;
				break;
			}
			else if ((flags & FL_ONGROUND) || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER) break;

			// Replace old edgebug detection variables
			velocity = g_LocalPlayer->m_vecVelocity();
			flags = g_LocalPlayer->m_fFlags();
		}
		EnginePrediction::End();
	}
}

void BunnyHop::EdgebugDraw() {
	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive()) return;

	// Edgebug detection indicator
	Vector pos;
	if (Math::WorldToScreen(eb_pos, pos) && last_set > g_GlobalVars->curtime) {
		Render::Get().RenderCircleFilled(pos.x, pos.y, 4, 4, Color(g_Options.color_detection_beam));
	}
}

/*void BunnyHop::EdgebugDetection(CUserCmd* ucmd, float unpredicted_velocity)
{
	static auto& option_eb_detect = g_Options.misc_ebugdetector;
	static auto& option_jumpbug = g_Options.misc_jumpbug;
	static auto& option_edgebug = g_Options.misc_ebug;
	static auto& option_jumpbug_key = g_Options.misc_jumpbug_bind;
	static auto& option_edgebug_key = g_Options.misc_ebug_key;

	if (!option_eb_detect)
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	float cooldown = 0.f;

	//  Default Color (White): \x01
	//  teamcolour(will be purple if message from server): \x03
	//	red: \x07
	//	lightred: \x0F
	//	darkred: \x02
	//	bluegray: \x0A
	//	blue: \x0B
	//	darkblue: \x0C
	//	purple: \x03
	//	orchid: \x0E
	//	yellow: \x09
	//	gold: \x10
	//	lightgreen: \x05
	//	green: \x04
	//	lime: \x06
	//	gray: \x08
	//	gray2: \x0D

	auto predicted_velocity = std::floor(g_LocalPlayer->m_vecVelocity().z);
	bool did_edgebug;
	if (unpredicted_velocity >= -7 || predicted_velocity != -7 || g_LocalPlayer->m_fFlags() & 1)
	{
		did_edgebug = false;
	}
	else
	{
		if ((!(GetAsyncKeyState(option_edgebug_key)) && (!(GetAsyncKeyState(option_jumpbug_key))) && g_GlobalVars->realtime > cooldown))
		{
			g_ChatElement->ChatPrintf(0, 0, std::string("").
				append("\x01"). //Default Color
				append("[").
				append("\x0B"). //Blue
				append("Nirvana").
				append("\x01"). //Default Color
				append("]").
				append(" \x01"). //Default Color
				append("|").
				append(" \x0E"). //Orchid 
				append("edgebugged").c_str());

			cooldown = g_GlobalVars->realtime + 0.5;
			did_edgebug = true;
		}

		if (option_edgebug && GetAsyncKeyState(option_edgebug_key) && g_GlobalVars->realtime > cooldown)
		{
			g_ChatElement->ChatPrintf(0, 0, std::string("").
				append("\x01"). //Default Color
				append("[").
				append("\x0B"). //Blue
				append("Nirvana").
				append("\x01"). //Default Color
				append("]").
				append(" \x01"). //Default Color
				append("|").
				append(" \x0E"). //Orchid
				append("edgebugged").
				append(" \x01"). //Default Color
				append("-").
				append(" \x0E"). //Orchid
				append("assisted").c_str());

			cooldown = g_GlobalVars->realtime + 0.5;
			did_edgebug = true;
		}

		if ((GetAsyncKeyState(option_jumpbug_key) && !(GetAsyncKeyState(option_edgebug_key)) && g_GlobalVars->realtime > cooldown))
		{
			g_ChatElement->ChatPrintf(0, 0, std::string("").
				append("\x01"). //Default Color
				append("[").
				append("\x0B"). //Blue
				append("Nirvana").
				append("\x01"). //Default Color
				append("]").
				append(" \x01"). //Default Color
				append("|").
				append(" \x0E"). //Orchid 
				append("edgebugged").
				append(" \x01"). //Default Color
				append("-").
				append(" \x0E"). //Orchid
				append("jumpbugged").c_str());

			cooldown = g_GlobalVars->realtime + 0.5;
			did_edgebug = true;
		}
	}
}*/

/*void BunnyHop::EBUG(CUserCmd* pCmd)
{
	if (!g_Options.misc_ebug)
		return;

	//	auto local = reinterpret_cast<C_BaseEntity*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	float max_radias = D3DX_PI * 2;
	float step = max_radias / 128;
	float xThick = 23;
	bool did_jump;
	bool unduck = false;

	if (g_Options.misc_ebug && GetAsyncKeyState(g_Options.misc_ebug_key) && (g_LocalPlayer->m_fFlags() & (1 << 0)))
	{
	//	g_Options.misc_bhop = false;
		int screenWidth, screenHeight;
		g_EngineClient->GetScreenSize(screenWidth, screenHeight);
		if (unduck)
		{
			did_jump = false;
			pCmd->buttons &= ~IN_DUCK; // duck
		//	pCmd->buttons |= IN_JUMP; // jump
			unduck = false;
		}
		Vector pos = g_LocalPlayer->m_vecOrigin();
		for (float a = 0.f; a < max_radias; a += step) {
			Vector pt;
			pt.x = (xThick * cos(a)) + pos.x;
			pt.y = (xThick * sin(a)) + pos.y;
			pt.z = pos.z;


			Vector pt2 = pt;
			pt2.z -= 6;

			trace_t fag;

			Ray_t ray;
			ray.Init(pt, pt2);

			CTraceFilter flt;
			flt.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

			if (fag.fraction != 1.f && fag.fraction != 0.f)
			{
				did_jump = true;
				pCmd->buttons |= IN_DUCK; // duck
			//	pCmd->buttons &= ~IN_JUMP; // jump
				unduck = true;
			}
		}
		for (float a = 0.f; a < max_radias; a += step) {
			Vector pt;
			pt.x = ((xThick - 2.f) * cos(a)) + pos.x;
			pt.y = ((xThick - 2.f) * sin(a)) + pos.y;
			pt.z = pos.z;

			Vector pt2 = pt;
			pt2.z -= 6;

			trace_t fag;

			Ray_t ray;
			ray.Init(pt, pt2);

			CTraceFilter flt;
			flt.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

			if (fag.fraction != 1.f && fag.fraction != 0.f) {
				did_jump = true;
				pCmd->buttons |= IN_DUCK; // duck
			//	pCmd->buttons &= ~IN_JUMP; // jump
				unduck = true;
			}
		}
		for (float a = 0.f; a < max_radias; a += step) {
			Vector pt;
			pt.x = ((xThick - 20.f) * cos(a)) + pos.x;
			pt.y = ((xThick - 20.f) * sin(a)) + pos.y;
			pt.z = pos.z;

			Vector pt2 = pt;
			pt2.z -= 6;

			trace_t fag;

			Ray_t ray;
			ray.Init(pt, pt2);

			CTraceFilter flt;
			flt.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

			if (fag.fraction != 1.f && fag.fraction != 0.f) {
				did_jump = true;
				pCmd->buttons |= IN_DUCK; // duck
			//	pCmd->buttons &= ~IN_JUMP; // jump
				unduck = true;
			}
		}
	}

	//else { g_Options.misc_bhop = true; }
}*/

float DistanceBetweenCross(float X, float Y)
{
	float ydist = (Y - GetSystemMetrics(SM_CYSCREEN) / 2);
	float xdist = (X - GetSystemMetrics(SM_CXSCREEN) / 2);
	float Hypotenuse = sqrt(pow(ydist, 2) + pow(xdist, 2));
	return Hypotenuse;
}

namespace MovementRecorder2
{
	struct UserCmd
	{
		QAngle            viewAngles;
		float            forwardmove;
		float            sidemove;
		float            upmove;
		int                buttons;
		Vector          position;
	};

	Vector startVec;
	std::vector<UserCmd> CmdTmp;
	UserCmd tempcmd;
	bool f2 = true;
	bool f1 = true;
	bool isPlayingback = false;

	void MovementR(CUserCmd* cmd)
	{
		if (GetKeyState(g_Options.record_movement_key))
		{
			if (f2)
			{
				CmdTmp = {};
				startVec = g_LocalPlayer->m_angAbsOrigin();//Set where recording starts
			}
			tempcmd.position = g_LocalPlayer->m_angAbsOrigin();
			tempcmd.buttons = cmd->buttons;
			tempcmd.forwardmove = cmd->forwardmove;
			tempcmd.sidemove = cmd->sidemove;
			tempcmd.upmove = cmd->upmove;
			tempcmd.viewAngles = cmd->viewangles;
			CmdTmp.push_back(tempcmd);
			f2 = false;
		}
		else
		{
			f2 = true;
		}
	}

	bool MovementP(CUserCmd* cmd)
	{
		bool ret = false;
		int i = 0;
		if (GetKeyState(g_Options.play_movement_key))
		{
			ret = true;
			if (!isPlayingback)
			{
				float dist = Math::VectorDistance(g_LocalPlayer->m_angAbsOrigin(), startVec);
				if (dist < 1)
				{
					isPlayingback = true; //start playback
				}
				else
				{
					Vector finalVec;
					Math::VectorSubtract(startVec, g_LocalPlayer->m_angAbsOrigin(), finalVec);
					Math::VectorAngles(finalVec, cmd->viewangles);
					cmd->forwardmove = dist;
				}
			}
			else
			{
				//playback
				if (i >= CmdTmp.size())
				{
					i = 0;//reset
					isPlayingback = false;
				}
				else
				{

					cmd->buttons = CmdTmp[i].buttons;
					cmd->forwardmove = CmdTmp[i].forwardmove;
					cmd->sidemove = CmdTmp[i].sidemove;
					cmd->upmove = CmdTmp[i].upmove;
					cmd->viewangles = CmdTmp[i].viewAngles;

					if (!g_Options.silent_movementrecord)
					{
						g_EngineClient->SetViewAngles(&CmdTmp[i].viewAngles);
					}

					i++;
				}
			}
		}

		else
		{
			i = 0;//reset
			isPlayingback = false;//stop playback
		}

		return ret;
	}

	void MovementD()
	{

		if (!g_Options.enable_movement_recorder)
			return;

		if (!g_EngineClient->IsInGame() && !g_EngineClient->IsConnected())
			return;

		auto iDist = Color(0, 255, 0);

		if (g_LocalPlayer->m_vecOrigin().DistTo(startVec) <= 4000)
		{
			Render::Get().RenderCircle3D(startVec, 90.f, 20.f, iDist);
			Render::Get().RenderCircle3D(startVec, 90.f, 105.f, iDist);
		}
		if (GetKeyState(g_Options.record_movement_key))
		{
			int w, h;
			g_EngineClient->GetScreenSize(w, h);
			Render::Get().RenderText("RECORDING", ImVec2(w - 120, 700), 25.f, Color(255, 0, 0), false, true);
		}
		if (GetKeyState(g_Options.play_movement_key))
		{
			int w, h;
			g_EngineClient->GetScreenSize(w, h);
			Render::Get().RenderText("RE-PLAYING", ImVec2(w - 130, 716), 25.f, Color(0, 255, 0), false, true);
		}

		g_Options.rainbow += 0.0005f;
		if (g_Options.rainbow > 1.f)
			g_Options.rainbow = 0.f;

		//Drawing the path
		for (int i = 0; i < CmdTmp.size(); i++)
		{
			auto tracer = CmdTmp.at(i);
			auto to_world = Vector(CmdTmp.size(), CmdTmp.size(), CmdTmp.size());
			Vector last_pos;
			Vector last_w2s;
			if (g_LocalPlayer->m_angAbsOrigin().DistTo(tracer.position) <= 120)
				if (Math::WorldToScreen(tracer.position, to_world) && Math::WorldToScreen(last_pos, last_w2s))
					Render::Get().RenderLine(to_world.x, to_world.y, last_w2s.x, last_w2s.y, Color::FromHSB(g_Options.rainbow, 1.f, 1.f), 2.f);
			last_pos = tracer.position;
		}
	}
}

namespace movementRecorder {

	struct custom_cmd
	{
		QAngle			viewAngles;
		Vector			pos;
		float			forwardmove;
		float			sidemove;
		float			upmove;
		int				buttons;
	};

	std::vector<custom_cmd> CmdFinal;
	custom_cmd tempCmd;

	bool recordBool;
	bool playBool;
	bool crossDistBool;
	bool AimToFirstRecord;
	bool recordAfterDonePlaying;

	QAngle viewPos;
	Vector lastPos;
	bool tempBool = true;
	bool isPlayingback = false;
	int i = 0;

	void writeVec(std::ostream& os, const std::vector<custom_cmd>& cmd)
	{
		typename std::vector<custom_cmd>::size_type size = cmd.size();
		os.write((char*)&size, sizeof(size));
		os.write((char*)&cmd[0], cmd.size() * sizeof(custom_cmd));
	}

	void readVec(std::istream& is, std::vector<custom_cmd>& cmd)
	{
		typename std::vector<custom_cmd>::size_type size = 0;
		is.read((char*)&size, sizeof(size));
		cmd.resize(size);
		is.read((char*)&cmd[0], cmd.size() * sizeof(custom_cmd));
	}

	void saveToFile()
	{
		const char* mapname = g_EngineClient->GetMapGroupName();

		std::string ayyy(mapname);

		std::string result = "movementrecorder/";

		std::regex re("\/(?!.*\/)(.*)");

		std::smatch match;

		if (std::regex_search(ayyy, match, re) && !CmdFinal.empty())
		{
			result += match.str(1);
			result += ".bin";
			std::ofstream out(result, std::ios::out | std::ios::binary);
			writeVec(out, CmdFinal);
			out.close();
		}
	}

	void readFromFile()
	{
		const char* mapname = g_EngineClient->GetMapGroupName();

		CmdFinal = {};

		DIR* dpdf;
		struct dirent* epdf;
		struct stat file_stats;

		std::string tempname = "movementrecorder/";

		dpdf = opendir("C:/Nirvana/movementrecorder/");

		if (dpdf != NULL) {
			while (epdf = readdir(dpdf)) {
				if (*epdf->d_name == '.')
					continue;


				std::string temp = (std::string)epdf->d_name;
				temp = temp.substr(0, temp.size() - 4);

				if (strstr(mapname, temp.c_str()))
				{
					tempname += temp + ".bin";
					std::ifstream in(tempname.c_str(), std::ios::out | std::ios::binary);
					readVec(in, CmdFinal);
					in.close();
					break;
				}
			}
		}
		closedir(dpdf);

	}

	void MovementRecord(CUserCmd* cmd)
	{
		if (!g_Options.enable_movement_recorder)
			return;

		if (GetKeyState(g_Options.record_movement_key))
		{
			g_Options.recordBool = !g_Options.recordBool;

			custom_cmd tempCmd = {};

			tempCmd.buttons = cmd->buttons;
			tempCmd.forwardmove = cmd->forwardmove;
			tempCmd.sidemove = cmd->sidemove;
			tempCmd.upmove = cmd->upmove;
			tempCmd.viewAngles = cmd->viewangles;
			tempCmd.pos = g_LocalPlayer->m_vecOrigin();

			if (tempBool)
			{
				CmdFinal = {};
			}
			CmdFinal.push_back(tempCmd);
			tempBool = false;
		}
		else
		{
			tempBool = true;
		}
	}
	QAngle wishAngle;

	void MovementPlay(CUserCmd* cmd)
	{
		if (!g_Options.enable_movement_recorder)
			return;

		if (CmdFinal.size() <= 0)
			return;

		if (GetKeyState(g_Options.play_movement_key))
		{
			g_Options.playBool = !g_Options.playBool;

			if (!isPlayingback) {
				auto dist = Math::VectorDistance(g_LocalPlayer->m_vecOrigin(), CmdFinal[0].pos);

				if (dist <= 1.f) {
					isPlayingback = true;

					auto viewPos = Vector(CmdFinal.size(), CmdFinal.size(), CmdFinal.size());
					Vector temp2D;

					if (Math::WorldToScreen(viewPos, temp2D))
					{
						Render::Get().RenderLine(viewPos.x, viewPos.y, temp2D.x, temp2D.y, Color(255, 255, 255));
					}
				}
				else {
					if (!crossDistBool) {
						auto viewPos = Vector(CmdFinal[0].pos[0], CmdFinal[0].pos[1], CmdFinal[0].pos[2] + 64.f);

						Vector temp2D;

						float crossDist = 0;

						if (Math::WorldToScreen(viewPos, temp2D)) {
							crossDist = DistanceBetweenCross(temp2D[0], temp2D[1]);
						}
						else {
							crossDist = 10000;
						}

						if (crossDist <= 1.f) {
							crossDistBool = true;
						}
					}
					else {

						//move to start point
						auto finalVec = CmdFinal[0].pos - g_LocalPlayer->m_vecOrigin();

						Math::VectorAngles(finalVec, wishAngle);

						g_EngineClient->SetViewAngles(&wishAngle);

						cmd->viewangles = wishAngle;
						cmd->forwardmove = dist;

					}



				}
			}
			else
			{
				if (i >= CmdFinal.size()) {
					i = 0;//reset
					isPlayingback = false;
					crossDistBool = false;
					AimToFirstRecord = true;
					if (!recordAfterDonePlaying)
						g_Options.playBool = false;
					else {
						g_Options.playBool = false;
						g_Options.recordBool = true;
						tempBool = false;
					}
				}
				else
				{

					if (AimToFirstRecord) {
						Vector aimVec;
						Math::AngleVectors(CmdFinal[0].viewAngles, aimVec);
						Vector curVec;
						Math::AngleVectors(cmd->viewangles, curVec);

						auto delta = aimVec - curVec;

						const auto smoothed = curVec + delta / 64.f;

						QAngle aimAng;
						Math::VectorAngles(smoothed, aimAng);

						Math::Normalize3(aimAng);
						Math::ClampAngles(aimAng);

						g_EngineClient->SetViewAngles(&aimAng);

						cmd->viewangles = aimAng;

						auto deltadist = Math::VectorLength(delta);


						if (deltadist <= 0.0001f) {
							AimToFirstRecord = false;
						}

					}
					else
					{
						cmd->buttons = CmdFinal[i].buttons;
						cmd->forwardmove = CmdFinal[i].forwardmove;
						cmd->sidemove = CmdFinal[i].sidemove;
						cmd->upmove = CmdFinal[i].upmove;
						cmd->viewangles = CmdFinal[i].viewAngles;

						if (!g_Options.silent_movementrecord)
						{
							g_EngineClient->SetViewAngles(&CmdFinal[i].viewAngles);
						}

						i++;

					}

				}
			}
		}
		else
		{
			i = 0;//reset
			isPlayingback = false;//stop playback
			crossDistBool = false;
			AimToFirstRecord = true;

		}
	}

	void MovementDrawing()
	{
		if (!g_Options.enable_movement_recorder)
			return;

		if (!g_EngineClient->IsInGame() && !g_EngineClient->IsConnected())
			return;

		if (GetKeyState(g_Options.record_movement_key))
		{
			int w, h;
			g_EngineClient->GetScreenSize(w, h);
			Render::Get().RenderText("RECORDING", ImVec2(w - 120, 700), 25.f, Color(255, 0, 0), false, true, g_pSmallestPixelFont);
		}
		if (GetKeyState(g_Options.play_movement_key))
		{
			int w, h;
			g_EngineClient->GetScreenSize(w, h);
			Render::Get().RenderText("RE-PLAYING", ImVec2(w - 130, 716), 25.f, Color(0, 255, 0), false, true, g_pSmallestPixelFont);
		}
	}
}

inline float anglemod(float a)
{
	a = (360.f / 65536) * ((int)(a * (65536.f / 360.0f)) & 65535);
	return a;
}

float ApproachAngle(float target, float value, float speed)
{
	target = anglemod(target);
	value = anglemod(value);

	float delta = target - value;

	// Speed is assumed to be positive
	if (speed < 0)
		speed = -speed;

	if (delta < -180)
		delta += 360;
	else if (delta > 180)
		delta -= 360;

	if (delta > speed)
		value += speed;
	else if (delta < -speed)
		value -= speed;
	else
		value = target;

	return value;
}

/*
void BunnyHop::fakestuff(CUserCmd* cmd)
{
	QAngle wish_angle = cmd->viewangles;
	if (g_Options.misc_fakebackwards)
	{
		float varzea = cmd->viewangles.yaw;
		float varzea2 = cmd->viewangles.yaw;

		cmd->viewangles.yaw -= ApproachAngle(varzea + 180, varzea2, g_Options.fakeback_speed);
		Math::MovementFix(cmd, wish_angle, cmd->viewangles);
		cmd->viewangles.Normalize();
	}
}*/

/*Vector startVec;
std::vector<CUserCmd> CmdTmp;//recording
bool f2 = true;
bool f1 = true;
void BunnyHop::MovementRecord(C_BaseEntity* LocalEntity, CUserCmd* cmd)
{
	if (!g_Options.enable_movement_recorder)
		return;

	C_BasePlayer* LocalPlayer = (C_BasePlayer*)LocalEntity;
	if (GetAsyncKeyState(g_Options.record_movement_key))
	{
		CUserCmd* tmpcmd = cmd;
		if (f2)
		{
			CmdTmp = {};
			startVec = g_LocalPlayer->m_angAbsOrigin();//Set where recording starts
		}
		if (!g_LocalPlayer->IsAlive())
		{
			auto Observer = (C_BaseEntity*)g_LocalPlayer->m_hObserverTarget();
			C_BasePlayer* pPlayer = (C_BasePlayer*)Observer;
			tmpcmd = pPlayer->m_pCurrentCommand();
			if (f2)
				startVec = Observer->m_angAbsOrigin();
		}
		CmdTmp.push_back(*tmpcmd);
		f2 = false;
	}
	else
	{
		f2 = true;
	}
}
bool isPlayingback = false;
int i = 0;
bool BunnyHop::MovementPlay(C_BaseEntity* LocalEntity, CUserCmd* cmd)
{

	bool ret = false;

	if (g_Options.enable_movement_recorder)
	{
		if (GetAsyncKeyState(g_Options.play_movement_key))
		{
			ret = true;
			if (!isPlayingback)
			{
				float dist = Math::VectorDistance(LocalEntity->m_angAbsOrigin(), startVec);
				if (dist < 1)
					isPlayingback = true; //start playback
				else
				{
					//move to start point
					Vector finalVec;
					Math::VectorSubtract(startVec, LocalEntity->m_angAbsOrigin(), finalVec);
					Math::VectorAngles(finalVec, cmd->viewangles);
					cmd->forwardmove = dist;
				}
			}
			else
			{
				//playback
				if (i >= CmdTmp.size())
				{
					i = 0;//reset
					isPlayingback = false;
				}
				else
				{
					cmd->buttons = CmdTmp[i].buttons;
					cmd->forwardmove = CmdTmp[i].forwardmove;
					cmd->sidemove = CmdTmp[i].sidemove;
					cmd->upmove = CmdTmp[i].upmove;
					cmd->viewangles = CmdTmp[i].viewangles;
					i++;
				}
			}
		}
		else
		{
			i = 0;//reset
			isPlayingback = false;//stop playback
		}
	}

	return ret;
}*/

void BunnyHop::SlideWalk(CUserCmd* cmd)
{
	if (g_Options.misc_slide_walk)
	{
		if (g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
			return;

		if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER)
			return;

		if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
			return;

		if (cmd->buttons & IN_FORWARD)
		{
			cmd->forwardmove = 450;
			cmd->buttons &= ~IN_FORWARD;
			cmd->buttons |= IN_BACK;
		}
		else if (cmd->buttons & IN_BACK)
		{
			cmd->forwardmove = -450;
			cmd->buttons &= ~IN_BACK;
			cmd->buttons |= IN_FORWARD;
		}

		if (cmd->buttons & IN_MOVELEFT)
		{
			cmd->sidemove = -450;
			cmd->buttons &= ~IN_MOVELEFT;
			cmd->buttons |= IN_MOVERIGHT;
		}
		else if (cmd->buttons & IN_MOVERIGHT)
		{
			cmd->sidemove = 450;
			cmd->buttons &= ~IN_MOVERIGHT;
			cmd->buttons |= IN_MOVELEFT;
		}
	}
}

bool IsPlayerBehind(C_BasePlayer* player)
{
	Vector toTarget = (g_LocalPlayer->m_vecOrigin() - player->m_vecOrigin()).Normalized();
	Vector playerViewAngles;
	Math::AngleVectors(player->m_angEyeAngles(), playerViewAngles);
	if (toTarget.Dot(playerViewAngles) > -0.5f)
		return false;
	else
		return true;
}

void BunnyHop::Knifebot(CUserCmd* pCmd)
{
	if (g_Options.misc_knifebot)
	{
		C_BaseCombatWeapon* pLocalWeapon = g_LocalPlayer->m_hActiveWeapon();
		if (pLocalWeapon && pLocalWeapon->GetCSWeaponData())
		{
			if (g_LocalPlayer->m_hActiveWeapon()->GetCSWeaponData()->WeaponType == WEAPONTYPE_KNIFE)
			{

				for (int EntIndex = 1; EntIndex < g_EngineClient->GetMaxClients(); EntIndex++)
				{
					C_BasePlayer* Entity = (C_BasePlayer*)g_EntityList->GetClientEntity(EntIndex);

					if (!Entity)
						continue;

					if (!Entity->IsPlayer())
						continue;

					if (!Entity->IsAlive())
						continue;

					if (Entity == g_LocalPlayer)
						continue;

					float Distance = g_LocalPlayer->m_vecOrigin().DistTo(Entity->m_vecOrigin());

					if (Distance > 68)
						continue;

					if (Entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
						continue;

					Vector OrignWorld = Entity->m_vecOrigin();
					Vector OrignScreen;

					if (!Math::WorldToScreen(OrignWorld, OrignScreen))
						continue;

					static int cur_attack = 0;

					if (Distance > 60.f || Entity->m_iHealth() < 33)
						pCmd->buttons |= IN_ATTACK;
					else
					{
						if ((g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick) - pLocalWeapon->m_flNextPrimaryAttack() > 0)
						{
							if (IsPlayerBehind(Entity))
							{
								pCmd->buttons |= IN_ATTACK2;
							}
							else
							{
								if (Entity->m_ArmorValue() > 0)
								{
									if (Entity->m_iHealth() > 61)
									{
										pCmd->buttons |= IN_ATTACK;
									}
									else
									{
										pCmd->buttons |= IN_ATTACK2;
									}
								}
								else
								{
									if (Entity->m_iHealth() < 33)
									{
										pCmd->buttons |= IN_ATTACK;
									}
									else
									{
										pCmd->buttons |= IN_ATTACK2;
									}
								}
							}
						}

					}
				}
			}
		}
	}
}

void BunnyHop::JumpBug(CUserCmd* pCmd)
{
	if (!g_Options.misc_jumpbug)
		return;

	//	auto local = reinterpret_cast<C_BaseEntity*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	float max_radias = D3DX_PI * 2;
	float step = max_radias / 128;
	float xThick = 23;
	bool did_jump;
	bool unduck = false;

	if (g_Options.misc_jumpbug && GetAsyncKeyState(g_Options.misc_jumpbug_bind) && (g_LocalPlayer->m_fFlags() & (1 << 0)))
	{
		g_Options.misc_bhop = false;
		int screenWidth, screenHeight;
		g_EngineClient->GetScreenSize(screenWidth, screenHeight);
		if (unduck)
		{
			did_jump = false;
			pCmd->buttons &= ~IN_DUCK; // duck
			pCmd->buttons |= IN_JUMP; // jump
			unduck = false;
		}
		Vector pos = g_LocalPlayer->m_vecOrigin();
		for (float a = 0.f; a < max_radias; a += step) {
			Vector pt;
			pt.x = (xThick * cos(a)) + pos.x;
			pt.y = (xThick * sin(a)) + pos.y;
			pt.z = pos.z;


			Vector pt2 = pt;
			pt2.z -= 6;

			trace_t fag;

			Ray_t ray;
			ray.Init(pt, pt2);

			CTraceFilter flt;
			flt.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

			if (fag.fraction != 1.f && fag.fraction != 0.f)
			{
				did_jump = true;
				pCmd->buttons |= IN_DUCK; // duck
				pCmd->buttons &= ~IN_JUMP; // jump
				unduck = true;
			}
		}
		for (float a = 0.f; a < max_radias; a += step) {
			Vector pt;
			pt.x = ((xThick - 2.f) * cos(a)) + pos.x;
			pt.y = ((xThick - 2.f) * sin(a)) + pos.y;
			pt.z = pos.z;

			Vector pt2 = pt;
			pt2.z -= 6;

			trace_t fag;

			Ray_t ray;
			ray.Init(pt, pt2);

			CTraceFilter flt;
			flt.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

			if (fag.fraction != 1.f && fag.fraction != 0.f) {
				did_jump = true;
				pCmd->buttons |= IN_DUCK; // duck
				pCmd->buttons &= ~IN_JUMP; // jump
				unduck = true;
			}
		}
		for (float a = 0.f; a < max_radias; a += step) {
			Vector pt;
			pt.x = ((xThick - 20.f) * cos(a)) + pos.x;
			pt.y = ((xThick - 20.f) * sin(a)) + pos.y;
			pt.z = pos.z;

			Vector pt2 = pt;
			pt2.z -= 6;

			trace_t fag;

			Ray_t ray;
			ray.Init(pt, pt2);

			CTraceFilter flt;
			flt.pSkip = g_LocalPlayer;
			g_EngineTrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

			if (fag.fraction != 1.f && fag.fraction != 0.f) {
				did_jump = true;
				pCmd->buttons |= IN_DUCK; // duck
				pCmd->buttons &= ~IN_JUMP; // jump
				unduck = true;
			}
		}
	}



	else { g_Options.misc_bhop = true; }

}

void BunnyHop::OnCreateMove(CUserCmd* cmd) {
	static bool jumped_last_tick = false;
	static bool should_fake_jump = false;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (!jumped_last_tick && should_fake_jump) {
		should_fake_jump = false;
		cmd->buttons |= IN_JUMP;
	}
	else if (cmd->buttons & IN_JUMP) {
		if (g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
			jumped_last_tick = true;
			should_fake_jump = true;
		}
		else {
			cmd->buttons &= ~IN_JUMP;
			jumped_last_tick = false;
		}
	}
	else {
		jumped_last_tick = false;
		should_fake_jump = false;
	}
}

static vec_t normalize_yaw(vec_t ang)
{
	while (ang < -180.0f)
		ang += 360.0f;
	while (ang > 180.0f)
		ang -= 360.0f;
	return ang;
}

float get_delta(float hspeed, float maxspeed, float airaccelerate)
{
	auto term = (30.0 - (airaccelerate * maxspeed / 66.0)) / hspeed;

	if (term < 1.0f && term > -1.0f) {
		return acos(term);
	}

	return 0.f;
}

void BunnyHop::AutoStrafe(CUserCmd* cmd, QAngle va)
{
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_Options.misc_autostrafe)
		return;


	if (!g_LocalPlayer->IsAlive())
		return;

	if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
		return;

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	if (g_LocalPlayer->m_vecVelocity().Length2D() < 0.5)
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->m_nMoveType() != MOVETYPE_WALK)
		return;

	if (g_InputSystem->IsButtonDown(ButtonCode_t::KEY_A) ||
		g_InputSystem->IsButtonDown(ButtonCode_t::KEY_D) ||
		g_InputSystem->IsButtonDown(ButtonCode_t::KEY_S) ||
		g_InputSystem->IsButtonDown(ButtonCode_t::KEY_W))
		return;

	/*static auto side = 1.0f;
	side = -side;

	auto velocity = g_LocalPlayer->m_vecVelocity();
	velocity.z = 0.0f;

	QAngle wish_angle = cmd->viewangles;

	auto speed = velocity.Length2D();
	auto ideal_strafe = std::clamp(RAD2DEG(atan(15.f / speed)), 0.0f, 90.0f);

	if (cmd->forwardmove > 0.0f)
		cmd->forwardmove = 0.0f;

	static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");

	static float old_yaw = 0.f;
	auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.0f);
	auto abs_angle_delta = abs(yaw_delta);
	old_yaw = wish_angle.yaw;

	if (abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f) {
		QAngle velocity_direction;
		Math::VectorAngles(velocity, velocity_direction);
		auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
		auto retrack = std::clamp(RAD2DEG(atan(30.0f / speed)), 0.0f, 90.0f) * g_Options.retrack_speed;
		if (velocity_delta <= retrack || speed <= 15.0f) {
			if (-(retrack) <= velocity_delta || speed <= 15.0f) {
				wish_angle.yaw += side * ideal_strafe;
				cmd->sidemove = cl_sidespeed->GetFloat() * side;
			}
			else {
				wish_angle.yaw = velocity_direction.yaw - retrack;
				cmd->sidemove = cl_sidespeed->GetFloat();
			}
		}
		else {
			wish_angle.yaw = velocity_direction.yaw + retrack;
			cmd->sidemove = -cl_sidespeed->GetFloat();
		}

		Math::MovementFix(cmd, wish_angle, cmd->viewangles);
	}
	else if (yaw_delta > 0.0f) {
		cmd->sidemove = -cl_sidespeed->GetFloat();
	}
	else if (yaw_delta < 0.0f) {
		cmd->sidemove = cl_sidespeed->GetFloat();
	}
	*/
	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
	{
		float speed = g_LocalPlayer->m_vecVelocity().Length2D();
		Vector velocity = g_LocalPlayer->m_vecVelocity();
		QAngle wish_angle = cmd->viewangles;
		QAngle velocity_direction;
		float yawVelocity = RAD2DEG(atan2(velocity.y, velocity.x));
		float velocityDelta = normalize_yaw(cmd->viewangles.yaw - yawVelocity);
		static float sideSpeed = g_CVar->FindVar("cl_sidespeed")->GetFloat();
		QAngle viewAngles = va;

		if (fabsf(cmd->mousedx > 2)) {

			cmd->sidemove = (cmd->mousedx < 0.f) ? -sideSpeed : sideSpeed;
			return;
		}

		if (cmd->buttons & IN_BACK)
			return; //cmd->viewangles.yaw -= 180.f;
		else if (cmd->buttons & IN_MOVELEFT)
			return; //cmd->viewangles.yaw -= 90.f;
		else if (cmd->buttons & IN_MOVERIGHT)
			return; //cmd->viewangles.yaw += 90.f;

		if (!speed > 0.5f || speed == NAN || speed == INFINITE) {

			cmd->forwardmove = 450.f;
			return;
		}

		cmd->forwardmove = std::clamp(5850.f / speed, -450.f, 450.f);

		if ((cmd->forwardmove < -450.f || cmd->forwardmove > 450.f))
			cmd->forwardmove = 0.f;

		cmd->sidemove = (velocityDelta > 0.0f) ? -sideSpeed : sideSpeed;
		Math::MovementFix(cmd, wish_angle, cmd->viewangles);
		viewAngles.Normalize();
	}
}































































































































































































































































































































































































































































































































































































































































































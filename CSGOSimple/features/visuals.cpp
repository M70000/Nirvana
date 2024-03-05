#include <algorithm>
#include "visuals.hpp"

#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "damageindicator.hpp"
#include "autowall.hpp"
#include "grenade_prediction.hpp"
#include "backtrack.hpp"
#include "aimbot.hpp"
#include "hitsounds.h"
#include "local_info.h"
#include "bhop.hpp"
#include <iomanip>

bomb_planted_event bomb_planted_listener;
round_end_event round_end_listener;
//vote_cast_event vote_cast_listener;

float StoredCurtimePlayer[1024];
float flPlayerAlpha[1024];

int get_player_alpha(int alpha, int index)
{
	int i = alpha - flPlayerAlpha[index];
	i = std::clamp(i, 0, 255);
	return i;
}

RECT GetBBox(C_BaseEntity* ent) {
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals() {
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}


bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos) {
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}

//--------------------------------------------------------------------------------
void Visuals::Render() 
{
}
//--------------------------------------------------------------------------------
bool Visuals::Player::Begin(C_BasePlayer* pl) {
	if (pl->IsDormant() || !pl->IsAlive())
		return false;

	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_HEAD || HITBOX_PELVIS || HITBOX_UPPER_CHEST || HITBOX_CHEST || HITBOX_STOMACH || HITBOX_RIGHT_FOREARM || HITBOX_LEFT_FOREARM);
	
	Vector eVecTarget;
	Vector pVecTarget = g_LocalPlayer->GetEyePos();

	if (g_Options.esp_smokecheck && IsLineGoesThroughSmoke(pVecTarget, eVecTarget))
		return false;

	if (!ctx.is_enemy)
		return false;
	if (g_Options.esp_visiblecheck && !ctx.is_visible)
		return false;

	ctx.clr = ctx.is_enemy ?
		(ctx.is_visible ? Color(g_Options.color_esp_enemy_visible) : Color(g_Options.color_esp_enemy_occluded)) :
		(ctx.is_visible ? Color(g_Options.color_esp_ally_visible) : Color(g_Options.color_esp_ally_occluded));

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 6;

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
	auto w = h / 2.f;

	ctx.bbox.left = static_cast<long>(ctx.feet_pos.x - w * 0.45f);
	ctx.bbox.right = static_cast<long>(ctx.bbox.left + w);
	ctx.bbox.bottom = static_cast<long>(ctx.feet_pos.y);
	ctx.bbox.top = static_cast<long>(ctx.head_pos.y);

	ctx.pinfo = &pl->GetPlayerInfo();

	//ctx.is_nirvana_user = (std::find(info_lp::nirvana_users.begin(), info_lp::nirvana_users.end(), ctx.pinfo->steamID64) != info_lp::nirvana_users.end());

	return true;
}

void Visuals::Player::DrawNirvanaUser()
{
	if (!ctx.pl || !ctx.pl->IsAlive())
		return;

	if (!ctx.pinfo)
		return;

	if (ctx.is_nirvana_user)
	{
		if (g_Options.use_tahomafont)
		{
			Render::Get().RenderText("Nirvana User", ctx.bbox.right + 5, ctx.bbox.top + ctx.right_text_size * 10, 14.f, Color(238, 232, 170, 255), false, true, g_pTahomaFont);
			ctx.right_text_size += 1;
		}
		else
		{
			Render::Get().RenderText("Nirvana User", ctx.bbox.right + 5, ctx.bbox.top + ctx.right_text_size * 10, 14.f, Color(238, 232, 170, 255), false, true, g_pSmallestPixelFont);
			ctx.right_text_size += 1;
		}
	}
}

int killcounter_kills = 0;
int killcounter_time = 0;
//float killcounter_time = 0.0f;

void Visuals::KillCounterEvent(IGameEvent* event)
{
	if (!event)
		return;

	if (!g_LocalPlayer)
		return;

	if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
		return;

	if (!g_Options.kill_counter)
		return;

	auto attacker = g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")));

	if (!attacker)
		return;

	if (attacker == g_LocalPlayer)
	{
		killcounter_kills++;
		killcounter_time = 255;
	}
}

void Visuals::KillCounter()
{
	if (!g_Options.kill_counter)
		return;

	if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
		return;

	int screen_sizex, screen_sizey, cW, cH;
	g_EngineClient->GetScreenSize(screen_sizex, screen_sizey);

	int globalAlpha = 0;

	float endTime = globalAlpha + 1.1f;
	float t = 1.0f - (endTime - g_GlobalVars->curtime) / (endTime - globalAlpha);

	cW = screen_sizex / 2;
	cH = screen_sizey / 2;

	screen_sizey = (screen_sizey / 2) - (t * 10.f);

	if (killcounter_time > 0)
	{
		int alpha = killcounter_time;
		switch (killcounter_kills)
		{
		case 1:
			Render::Get().RenderText("FIRST BLOOD", screen_sizex / 2, screen_sizey, 30.f, Color(255, 0, 0, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		case 2:
			Render::Get().RenderText("DOUBLE KILL", screen_sizex / 2, screen_sizey, 30.f, Color(0, 0, 255, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		case 3:
			Render::Get().RenderText("TRIPLE KILL", screen_sizex / 2, screen_sizey, 30.f, Color(0, 255, 0, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		case 4:
			Render::Get().RenderText("MULTI KILL", screen_sizex / 2, screen_sizey, 30.f, Color(155, 200, 0, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		case 5:
			Render::Get().RenderText("ULTRA KILL", screen_sizex / 2, screen_sizey, 30.f, Color(200, 0, 167, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		case 6:
			Render::Get().RenderText("KILLING SPREE", screen_sizex / 2, screen_sizey, 30.f, Color(23, 34, 255, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		default:
			Render::Get().RenderText("HUMILIATION", screen_sizex / 2, screen_sizey, 30.f, Color(23, 34, 255, alpha), true, true, g_pSmallestPixelFont);
			killcounter_time -= 2;
			break;
		}
	}
}

/*
void Visuals::KillCounterEvent(IGameEvent* event)
{
	if (!event)
		return;

	if (!g_EngineClient->IsInGame())
		return;

	if (!g_EngineClient->IsConnected())
		return;

	if (!g_Options.kill_counter)
		return;

	if (!g_LocalPlayer)
		return;

	auto attacker = g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")));

	if (!attacker)
		return;

	if (attacker == g_LocalPlayer)
	{
		killcounter_kills++;
		killcounter_time = 2.0f;
	}

	if (strstr(event->GetName(), "round_start") || strstr(event->GetName(), "round_end"))
	{
		killcounter_kills = 0;
	}
}*/
/*
void Visuals::KillCounter()
{
	if (!g_EngineClient->IsInGame())
		return;

	if (!g_EngineClient->IsConnected())
		return;

	if (!g_Options.kill_counter)
		return;

	if (!g_LocalPlayer)
		return;

	int globalAlpha = 0;

	int screen_sizex, screen_sizey, cW, cH;
	g_EngineClient->GetScreenSize(screen_sizex, screen_sizey);

	cW = screen_sizex / 2;
	cH = screen_sizey / 2;

	float animationAlpha = 0.0;
	int alpha = 0;
	float animation_speed_value = 0.6;
	float step = 255 / 0.6 * g_GlobalVars->frametime;

	if (killcounter_time + 0.4 > g_GlobalVars->realtime)
	{
		alpha = 255;
	}
	else
		alpha = alpha - step;

	float animate = g_GlobalVars->curtime;

	float endTime = globalAlpha + 1.1f;
	float t = 1.0f - (endTime - g_GlobalVars->curtime) / (endTime - globalAlpha);


	float animationStep = animation_speed_value * g_GlobalVars->frametime;

	if (killcounter_time + 0.1 > g_GlobalVars->realtime)
	{
		animationAlpha -= 80;
	}
	else
		animationAlpha = animationAlpha - animationStep;

	int customY, customX;

	screen_sizey = (screen_sizey / 2) - (t * 10.f);

	if (alpha > 0)
	{
		switch (killcounter_kills)
		{
		case 1:
			Render::Get().RenderText("FIRST BLOOD", screen_sizex / 2, screen_sizey, 24.f, Color(255, 0, 0, alpha), true, true, g_pTahomaFont);
		    PlaySoundA(cracksound, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		case 2:
			Render::Get().RenderText("DOUBLE KILL", screen_sizex / 2, screen_sizey, 24.f, Color(0, 0, 255, alpha), true, true, g_pTahomaFont);
			PlaySoundA(gnome, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		case 3:
			Render::Get().RenderText("TRIPLE KILL", screen_sizex / 2, screen_sizey, 24.f, Color(0, 255, 0, alpha), true, true, g_pTahomaFont);
			PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		case 4:
			Render::Get().RenderText("MULTI KILL", screen_sizex / 2, screen_sizey, 24.f, Color(155, 200, 0, alpha), true, true, g_pTahomaFont);
			PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		case 5:
			Render::Get().RenderText("ULTRA KILL", screen_sizex / 2, screen_sizey, 24.f, Color(200, 0, 167, alpha), true, true, g_pTahomaFont);
			PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		case 6:
			Render::Get().RenderText("KILLING SPREE", screen_sizex / 2, screen_sizey, 24.f, Color(23, 34, 255, alpha), true, true, g_pTahomaFont);
			PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		default:
			Render::Get().RenderText("HUMILIATION", screen_sizex / 2, screen_sizey, 24.f, Color(23, 34, 255, alpha), true, true, g_pTahomaFont);
			PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break;
			break;
		}

	}
}*/

/*void draw_text(int x, int y, unsigned long font, std::string string, bool text_centered, color colour) {
	const auto converted_text = std::wstring(string.begin(), string.end());

	int width, height;
	g_VGuiSurface->GetTextSize(font, converted_text.c_str(), width, height);

	g_VGuiSurface->DrawSetTextColor(colour.r, colour.g, colour.b, colour.a);
	g_VGuiSurface->DrawSetTextFont(font);
	if (text_centered)
		g_VGuiSurface->DrawSetTextPos(x - (width / 2), y);
	else
		g_VGuiSurface->DrawSetTextPos(x, y);
	Render::Get().RenderText(converted_text.c_str(), wcslen(converted_text.c_str()));
}*/

/*void Visuals::GrenadeEsp(C_BaseEntity* entity)
{
	Vector pos, pos3D;
	pos3D = entity->m_angAbsOrigin();
	if (!Math::WorldToScreen(pos3D, pos))
		return;

	auto local_player = g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());
	if (!local_player) return;

	auto cClass = entity->GetClientClass();

	std::string model_name = g_MdlInfo->GetModelName(entity->GetModel());

	if (!model_name.empty())
	{
		if (cClass->m_ClassID == (int)ClassId_CBaseCSGrenadeProjectile)
		{
			if (model_name.find("fraggrenade") != std::string::npos) 
			{
				if (g_Options.use_tahomafont)
				{
					Render::Get().RenderText("HE", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont);
				}
				else
				{
					Render::Get().RenderText("HE", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
				}
			}
			else if (model_name.find("bang") != std::string::npos)
			{
				if (g_Options.use_tahomafont)
				{
					Render::Get().RenderText("Flash", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont);
				}
				else
				{
					Render::Get().RenderText("Flash", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
				}
			}

		}
		if (cClass->m_ClassID == (int)ClassId_CDecoyProjectile) 
		{

			if (model_name.find("decoy") != std::string::npos)
			{
				if (g_Options.use_tahomafont)
				{
					Render::Get().RenderText("Decoy", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont);
				}
				else
				{
					Render::Get().RenderText("Decoy", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
				}
			}
		}
		if (cClass->m_ClassID == (int)ClassId_CMolotovProjectile) 
		{

			if (model_name.find("molotov") != std::string::npos) 
			{
				if (g_Options.use_tahomafont)
				{
					Render::Get().RenderText("Molotov", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont);
				}
				else
				{
					Render::Get().RenderText("Molotov", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
				}
			}
			else if (model_name.find("incendiary") != std::string::npos) 
			{
				if (g_Options.use_tahomafont)
				{
					Render::Get().RenderText("Incendiary", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont);
				}
				else
				{
					Render::Get().RenderText("Incendiary", pos.x, pos.y, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
				}
			}
		}
	}
}*/

void Visuals::MovementIndicators() noexcept
{
	if (!g_Options.movement_indicators)
		return;

	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;

	int w, h;
	int centerW, centerh, topH;
	g_EngineClient->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;

	if (g_Options.misc_edgejump && GetAsyncKeyState(g_Options.misc_edgejump_bind))
	{
		Render::Get().RenderText("EJ", (centerW), (centerh + 275), 20.f, Color(255, 255, 255), true, true, g_pVelocityFont);

		if (g_Options.misc_jumpbug && GetAsyncKeyState(g_Options.misc_jumpbug_bind))
		{
			Render::Get().RenderText("JB |", (centerW - 30), (centerh + 275), 20.f, Color(255, 255, 255), true, true, g_pVelocityFont);
		} 
		else {}

		if (g_Options.misc_ebug && GetAsyncKeyState(g_Options.misc_ebug_key))
		{
			Render::Get().RenderText("| EB", (centerW + 30), (centerh + 275), 20.f, Color(255, 255, 255), true, true, g_pVelocityFont);
		} else {}
	}
	else
	{
		if (g_Options.misc_jumpbug && GetAsyncKeyState(g_Options.misc_jumpbug_bind))
		{
			Render::Get().RenderText("JB", (centerW), (centerh + 275), 20.f, Color(255, 255, 255), true, true, g_pVelocityFont);

			if (g_Options.misc_ebug && GetAsyncKeyState(g_Options.misc_ebug_key))
			{
				Render::Get().RenderText("| EB", (centerW + 30), (centerh + 275), 20.f, Color(255, 255, 255), true, true, g_pVelocityFont);
			}
		} 
		else
		{
			if (g_Options.misc_ebug && GetAsyncKeyState(g_Options.misc_ebug_key))
			{
				Render::Get().RenderText("EB", (centerW), (centerh + 275), 20.f, Color(255, 255, 255), true, true, g_pVelocityFont);
			}
		}
	}
}

#define FLAG_MACRO std::pair<std::string, Color>
#define FLAG(string, color) vecFlags.push_back(FLAG_MACRO(string, color)) 

void Visuals::Player::DrawFlags()
{
	if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;
	std::vector<FLAG_MACRO> vecFlags;

	int alpha = ctx.clr.a();
	if (g_Options.esp_flags_armor)
		if (ctx.pl->m_ArmorValue() > 0)
			FLAG(ctx.pl->GetArmorName(), Color(255, 255, 255, alpha));

	if (g_Options.esp_flags_scoped)
		if (ctx.pl->m_bIsScoped())
			FLAG("zoom", Color(52, 165, 207, alpha));

	if (g_Options.esp_flags_money)
		if (ctx.pl->m_iAccount())
			FLAG(std::string("$").append(std::to_string(ctx.pl->m_iAccount())), Color(0, 98, 51, alpha));

	if (g_Options.esp_flags_defusing)
		if (ctx.pl->m_bIsDefusing())
			FLAG("Defusing", Color(200, 0, 0, alpha));

	if (g_Options.esp_flags_c4)
		if (ctx.pl->HasC4())
			FLAG("C4", Color(200, 0, 0, alpha));

	if (g_Options.esp_flags_hostage)
		if (ctx.pl->m_hCarriedHostage() || ctx.pl->m_bIsGrabbingHostage())
			FLAG("Hostage", ctx.pl->m_bIsGrabbingHostage() ? Color(200, 0, 0, alpha) : Color(52, 165, 207, alpha));

	if (g_Options.esp_flags_kit)
		if (ctx.pl->m_bHasDefuser())
			FLAG("Kit", Color(52, 165, 207, alpha));

	int offset = 0; //smh, have to think about a better way just because of this lmao
	for (auto Text : vecFlags)
	{
		if (g_Options.use_tahomafont)
		{
			Render::Get().RenderText(Text.first, ctx.bbox.right + 4 + ((ctx.pl->m_ArmorValue() > 0 && g_Options.esp_player_armour) ? 5 : 0), ctx.bbox.top - 2 + offset, 14.f, Text.second, false, true, g_pTahomaFont);
		}
		else
		{
			Render::Get().RenderText(Text.first, ctx.bbox.right + 4 + ((ctx.pl->m_ArmorValue() > 0 && g_Options.esp_player_armour) ? 5 : 0), ctx.bbox.top - 2 + offset, 14.f, Text.second, false, true, g_pSmallestPixelFont);
		}
		offset += 12;
	}
}

static auto degrees_to_radians(float deg) -> float { return deg * (M_PI / 180.f); }

void Visuals::LegitAA_Draw() 
{
	if (!g_Options.enable_legitaa)
		return;

	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	auto anim_state = g_LocalPlayer->GetPlayerAnimState();

	if (!anim_state)
		return;

	auto delta = fabsf(g_LocalPlayer->GetDesyncDelta());

	int width, height;
	g_EngineClient->GetScreenSize(width, height);

	if (g_Options.use_tahomafont)
	{
		Render::Get().RenderText(g_Aimbot.break_lby ? "Broken" : "Safe", ImVec2(39, (height / 2) + 15), 13.f, g_Aimbot.break_lby ? Color::Red : Color(187, 217, 0), true, true, g_pTahomaFont);
		Render::Get().RenderText(g_Aimbot.desync_side ? "Real: Left" : "Real: Right", ImVec2(39, (height / 2) + 25), 13.f, Color::White, true, true, g_pTahomaFont);
	}
	else
	{
		Render::Get().RenderText(g_Aimbot.break_lby ? "Broken" : "Safe", ImVec2(39, (height / 2) + 15), 13.f, g_Aimbot.break_lby ? Color::Red : Color(187, 217, 0), true, true, g_pSmallestPixelFont);
		Render::Get().RenderText(g_Aimbot.desync_side ? "Real: Left" : "Real: Right", ImVec2(39, (height / 2) + 25), 13.f, Color::White, true, true, g_pSmallestPixelFont);
	}

	const auto origin = g_LocalPlayer->m_vecOrigin();
	Vector screen1, screen2;

	if (g_Options.use_tahomafont)
	{
		Render::Get().RenderText(std::to_string(g_LocalPlayer->m_angEyeAngles().yaw), ImVec2(width + 5, height + 5), 12.f, Color::White, false, true, g_pTahomaFont);
	}
	else
	{
		Render::Get().RenderText(std::to_string(g_LocalPlayer->m_angEyeAngles().yaw), ImVec2(width + 5, height + 5), 12.f, Color::White, false, true, g_pSmallestPixelFont);
	}

	auto get_rotated_position = [](Vector start, const float rotation, const float distance) -> Vector {
		const auto rad = degrees_to_radians(rotation);
		start.x += cosf(rad) * distance;
		start.y += sinf(rad) * distance;

		return start;
	};

	if (!Math::WorldToScreen(origin, screen1))
		return;

	if (Math::WorldToScreen(get_rotated_position(origin, g_LocalPlayer->m_angEyeAngles().yaw, 50.f), screen2))
	{
		Render::Get().RenderLine(screen1.x, screen1.y, screen2.x, screen2.y, Color::White);
		if (g_Options.use_tahomafont)
		{
			Render::Get().RenderText("Real", ImVec2(screen2.x, screen2.y), 14.f, Color(135, 206, 235), true, true, g_pTahomaFont);
		}
		else
		{
			Render::Get().RenderText("Real", ImVec2(screen2.x, screen2.y), 14.f, Color(135, 206, 235), true, true, g_pSmallestPixelFont);
		}
	}

	if (Math::WorldToScreen(get_rotated_position(origin, g_LocalPlayer->m_flLowerBodyYawTarget(), 50.f), screen2))
	{
		Render::Get().RenderLine(screen1.x, screen1.y, screen2.x, screen2.y, Color::White);
		if (g_Options.use_tahomafont)
		{
			Render::Get().RenderText("LBY", ImVec2(screen2.x, screen2.y), 14.f, Color(187, 217, 0), true, true, g_pTahomaFont);
		}
		else
		{
			Render::Get().RenderText("LBY", ImVec2(screen2.x, screen2.y), 14.f, Color(187, 217, 0), true, true, g_pSmallestPixelFont);
		}
	}

	if (Math::WorldToScreen(get_rotated_position(origin, anim_state->m_flGoalFeetYaw, 50.f), screen2))
	{
		Render::Get().RenderLine(screen1.x, screen1.y, screen2.x, screen2.y, Color::White);
		if (g_Options.use_tahomafont)
		{
			Render::Get().RenderText("Fake", ImVec2(screen2.x, screen2.y), 14.f, Color::Red, true, true, g_pTahomaFont);
		}
		else
		{
			Render::Get().RenderText("Fake", ImVec2(screen2.x, screen2.y), 14.f, Color::Red, true, true, g_pSmallestPixelFont);
		}
	}
}

void Visuals::MovementRecorderInd() noexcept
{
	if (!g_Options.enable_movement_recorder)
		return;

	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;

	int w, h;
	int centerW, centerh, topH;
	g_EngineClient->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;

	if (GetKeyState(g_Options.record_movement_key))
	{
		Render::Get().RenderText("Recording", (centerW), (centerh - 300), 22.f, Color(150, 0, 255), true, true, g_pVelocityFont);
	}
	else
	{

	}

	if (GetKeyState(g_Options.play_movement_key))
	{
		Render::Get().RenderText("Playing", (centerW), (centerh - 280), 22.f, Color(150, 0, 255), true, true, g_pVelocityFont);
	}
	else
	{

	}
}

void sin_cos(float r, float* s, float* c) {
	*s = sin(r);
	*c = cos(r);
}

void MotionVectors(Vector& angles, Vector* forward, Vector* right, Vector* up) {
	float sp, sy, sr, cp, cy, cr;
	sin_cos(DEG2RAD(angles[0]), &sp, &cp);
	sin_cos(DEG2RAD(angles[1]), &sy, &cy);
	sin_cos(DEG2RAD(angles[2]), &sr, &cr);

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}
	if (right) {
		right->x = -1 * sr * sp * cy + -1 * cr * -sy;
		right->y = -1 * sr * sp * sy + -1 * cr * cy;
		right->z = -1 * sr * cp;
	}
	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

static float motionblurvalues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
void Visuals::motionblur_render(CViewSetup* setup) {
	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer) return;

	float fallingmin = 10.f;
	float fallingmax = 10.f;
	float rotationintensity = 0.15f;
	float fallingintensity = 1.0f;
	float rollintensity = 0.3f;

	static motionblurhistory history;
	if (setup) {
		float timeelapsed = g_GlobalVars->realtime - history.lasttimeupdate;
		float currentpitch = setup->angles.x;
		while (currentpitch > 180.0f)
			currentpitch -= 360.0f;
		while (currentpitch < -180.0f)
			currentpitch += 360.0f;

		float currentyaw = setup->angles.y;
		while (currentyaw > 180.0f)
			currentyaw -= 360.0f;
		while (currentyaw < -180.0f)
			currentyaw += 360.0f;

		Vector currentforward;
		Vector currentside;
		Vector currentup;
		MotionVectors(setup->angles, &currentforward, &currentside, &currentup);

		Vector currentposition = setup->origin;
		Vector positionchange = history.previouspositon - currentposition;
		if (positionchange.Length() > 30.0f && timeelapsed >= 0.5f) {
			motionblurvalues[0] = 0.0f;
			motionblurvalues[1] = 0.0f;
			motionblurvalues[2] = 0.0f;
			motionblurvalues[3] = 0.0f;
		}
		else if (timeelapsed > (1.0f / 15.0f)) {
			motionblurvalues[0] = 0.0f;
			motionblurvalues[1] = 0.0f;
			motionblurvalues[2] = 0.0f;
			motionblurvalues[3] = 0.0f;
		}
		else if (positionchange.Length() > 50.0f) {
			history.nomotionbluruntil = g_GlobalVars->realtime + 1.0f;
		}
		else {
			float horizontalfov = setup->fov;
			float verticalfov = setup->aspectratio <= 0.0f ? setup->fov : setup->fov / setup->aspectratio;
			float viewmotion = currentforward.Dot(positionchange);

			float sidemotion = currentside.Dot(positionchange);
			float yawdifforiginal = history.previousyaw - currentyaw;
			if (((history.previousyaw - currentyaw > 180.0f) || (history.previousyaw - currentyaw < -180.0f)) && ((history.previousyaw + currentyaw > -180.0f) && (history.previousyaw + currentyaw < 180.0f)))
				yawdifforiginal = history.previousyaw + currentyaw;

			float yawdiffadjusted = yawdifforiginal + (sidemotion / 3.0f);
			if (yawdifforiginal < 0.0f)
				yawdiffadjusted = std::clamp(yawdiffadjusted, yawdifforiginal, 0.0f);
			else
				yawdiffadjusted = std::clamp(yawdiffadjusted, 0.0f, yawdifforiginal);

			float undampenedyaw = yawdiffadjusted / horizontalfov;
			motionblurvalues[0] = undampenedyaw * (1.0f - (fabsf(currentpitch) / 90.0f));

			float pitchcompensate = 1.0f - ((1.0f - fabsf(currentforward[2])) * (1.0f - fabsf(currentforward[2])));
			float pitchdifforiginal = history.previouspitch - currentpitch;
			float pitchdiffadjusted = pitchdifforiginal;
			if (currentpitch > 0.0f)
				pitchdiffadjusted = pitchdifforiginal - ((viewmotion / 2.0f) * pitchcompensate);
			else
				pitchdiffadjusted = pitchdifforiginal + ((viewmotion / 2.0f) * pitchcompensate);

			if (pitchdifforiginal < 0.0f)
				pitchdiffadjusted = std::clamp(pitchdiffadjusted, pitchdifforiginal, 0.0f);
			else
				pitchdiffadjusted = std::clamp(pitchdiffadjusted, 0.0f, pitchdifforiginal);

			motionblurvalues[1] = pitchdiffadjusted / verticalfov;
			motionblurvalues[3] = undampenedyaw;
			motionblurvalues[3] *= (fabs(currentpitch) / 90.0f) * (fabs(currentpitch) / 90.0f) * (fabs(currentpitch) / 90.0f);

			if (timeelapsed > 0.0f)
				motionblurvalues[2] /= timeelapsed * 30.0f;
			else
				motionblurvalues[2] = 0.0f;

			motionblurvalues[2] = std::clamp((fabsf(motionblurvalues[2]) - fallingmin) / (fallingmax - fallingmin), 0.0f, 1.0f) * (motionblurvalues[2] >= 0.0f ? 1.0f : -1.0f);
			motionblurvalues[2] /= 30.f;
			motionblurvalues[0] *= rotationintensity * g_Options.esp_motionblur_strength;
			motionblurvalues[1] *= rotationintensity * g_Options.esp_motionblur_strength;
			motionblurvalues[2] *= fallingintensity * g_Options.esp_motionblur_strength;
			motionblurvalues[3] *= rollintensity * g_Options.esp_motionblur_strength;

		}

		if (g_GlobalVars->realtime < history.nomotionbluruntil) {
			motionblurvalues[0] = 0.0f;
			motionblurvalues[1] = 0.0f;
			motionblurvalues[3] = 0.0f;
		}
		else {
			history.nomotionbluruntil = 0.0f;
		}

		history.previouspositon = currentposition;
		history.previouspitch = currentpitch;
		history.previousyaw = currentyaw;
		history.lasttimeupdate = g_GlobalVars->realtime;
		return;
	}
}

void draw_screen_effect(IMaterial* material) {
	// Get screen size
	int w, h;
	g_EngineClient->GetScreenSize(w, h);

	static auto fn = Utils::PatternScan(GetModuleHandleA("client.dll"), ("55 8B EC 83 E4 ? 83 EC ? 53 56 57 8D 44 24 ? 89 4C 24 ?"));
	_asm {
		push h
		push w
		push 0
		xor edx, edx
		mov ecx, material
		call fn
		add esp, 12
	}
}

void Visuals::motionblur_run() {
	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer) return;

	auto material = g_MatSystem->FindMaterial("dev/motion_blur", "RenderTargets", false);
	//auto material = g_MatSystem->FindMaterial("dev/motion_blur", TEXTURE_GROUP_RENDER_TARGET);
	auto motionblurinternal = material->FindVar("$MotionBlurInternal", nullptr, false);
	auto motionblurviewport = material->FindVar("$MotionBlurViewportInternal", nullptr, false);

	motionblurinternal->SetVectorComponent(motionblurvalues[0], 0);
	motionblurinternal->SetVectorComponent(motionblurvalues[1], 1);
	motionblurinternal->SetVectorComponent(motionblurvalues[2], 2);
	motionblurinternal->SetVectorComponent(motionblurvalues[3], 3);

	motionblurviewport->SetVectorComponent(0.0f, 0);
	motionblurviewport->SetVectorComponent(0.0f, 1);
	motionblurviewport->SetVectorComponent(1.0f, 2);
	motionblurviewport->SetVectorComponent(1.0f, 3);

	draw_screen_effect(material);
}

void Visuals::RCSCrosshair()
{
	if (g_Options.esp_rcscrosshair)
	{
		if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
			return;

		if (!g_LocalPlayer)
			return;

		if (!g_LocalPlayer->IsAlive())
			return;

		QAngle ViewAngles;
		g_EngineClient->GetViewAngles1337(ViewAngles);
		ViewAngles += (g_LocalPlayer->m_aimPunchAngle()) * 2.f;

		Vector fowardVec;
		Math::AngleVectors(ViewAngles, fowardVec);
		fowardVec *= 10000;

		Vector start = g_LocalPlayer->GetEyePos();
		Vector end = start + fowardVec, endScreen;

		auto& active_wpn = g_LocalPlayer->m_hActiveWeapon();

		if (!active_wpn)
			return;

		static auto index = active_wpn->m_iItemDefinitionIndex();

		if (Math::WorldToScreen(end, endScreen))
		{
			if (g_LocalPlayer->m_iShotsFired() >= 2)
			{
				Render::Get().RenderLine(endScreen.x - 6, endScreen.y, endScreen.x + 6, endScreen.y, Color(g_Options.color_rcs_crosshair));
				Render::Get().RenderLine(endScreen.x, endScreen.y - 6, endScreen.x, endScreen.y + 6, Color(g_Options.color_rcs_crosshair));
			}
		}
	}
}

void Visuals::RenderSniperCrosshair()
{
	if (g_Options.misc_snipercrosshair)
	{
		if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
			return;

		if (!g_LocalPlayer)
			return;

		if (g_LocalPlayer->m_bIsScoped())
			return;
	
		if (!g_LocalPlayer->IsAlive())
			return;

		auto& active_wpn = g_LocalPlayer->m_hActiveWeapon();

		if (!active_wpn)
			return;

		static auto index = active_wpn->m_iItemDefinitionIndex();

		int w, h;
		int centerW, centerh, topH;
		g_EngineClient->GetScreenSize(w, h);
		centerW = w / 2;
		centerh = h / 2;

		if (active_wpn->m_iItemDefinitionIndex() == WEAPON_AWP || active_wpn->m_iItemDefinitionIndex() == WEAPON_SSG08 || active_wpn->m_iItemDefinitionIndex() == WEAPON_SCAR20 || active_wpn->m_iItemDefinitionIndex() == WEAPON_G3SG1)
		{
			Render::Get().RenderLine((centerW - 6), (centerh), (centerW + 6), (centerh), Color(g_Options.color_rcs_crosshair));
			Render::Get().RenderLine((centerW), (centerh - 6), (centerW), (centerh + 6), Color(g_Options.color_rcs_crosshair));
		}
	}
}

void NewDrawBeam(Color clr, int width, int life, int flag, Vector start, Vector end) {
	BeamInfo_t beam_info;
	beam_info.m_nType = TE_BEAMPOINTS;
	beam_info.m_pszModelName = "sprites/purplelaser1.vmt";
	beam_info.m_nModelIndex = g_MdlInfo->GetModelIndex("sprites/purplelaser1.vmt");
	beam_info.m_flHaloScale = 0;
	beam_info.m_flLife = life;
	beam_info.m_flWidth = width;
	beam_info.m_flEndWidth = width;
	beam_info.m_flFadeLength = 0;
	beam_info.m_flAmplitude = 0;
	beam_info.m_flRed = clr.r();
	beam_info.m_flGreen = clr.g();
	beam_info.m_flBlue = clr.b();
	beam_info.m_flBrightness = clr.a();
	beam_info.m_flSpeed = 0;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0;
	beam_info.m_nSegments = 2;
	beam_info.m_nFlags = flag;
	beam_info.m_vecStart = start;
	beam_info.m_vecEnd = end;

	auto beam = g_RenderBeams->CreateBeamPoints(beam_info);
	if (beam) g_RenderBeams->DrawBeam(beam);
}

void DrawBeamPaw(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/glow01.vmt";
	beamInfo.m_pszHaloName = "sprites/glow01.vmt";
	beamInfo.m_flHaloScale = 3.0;
	beamInfo.m_flWidth = 4.5f;
	beamInfo.m_flEndWidth = 4.5f;
	beamInfo.m_flFadeLength = 0.5f;
	beamInfo.m_flAmplitude = 0;
	beamInfo.m_flBrightness = 255.f;
	beamInfo.m_flSpeed = 0.0f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 1;
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 2;
	beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
	Beam_t* myBeam = g_RenderBeams->CreateBeamPoints(beamInfo);
	if (myBeam)
		g_RenderBeams->DrawBeam(myBeam);
}

void Visuals::FootTrails()
{
	// Sanity checks
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive()) return;

	if (!g_Options.enable_foottrail)
		return;

	static float rainbow;
	rainbow += 0.001f;
	if (rainbow > 1.f)
		rainbow = 0.f;

	auto rainbow_col = Color::FromHSB(rainbow, 1, 1);

	// Render trail beam
	static Vector lastpos;
	if (g_Options.foot_trailcolor)
	{
		NewDrawBeam(Color(g_Options.color_foot_trail), 2, 3, FBEAM_ONLYNOISEONCE, g_LocalPlayer->m_vecOrigin(), lastpos);
	}
	else
	{
		NewDrawBeam(Color(rainbow_col), 2, 3, FBEAM_ONLYNOISEONCE, g_LocalPlayer->m_vecOrigin(), lastpos);
	}
	lastpos = g_LocalPlayer->m_vecOrigin();

	/*if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
	{
		static float rainbow;
		rainbow += 0.001f;
		if (rainbow > 1.f)
			rainbow = 0.f;

		auto rainbow_col = Color::FromHSB(rainbow, 1, 1);
		auto local_pos = g_LocalPlayer->m_vecOrigin();
		if (g_Options.foot_trailcolor)
		{
			DrawBeamPaw(local_pos, Vector(local_pos.x, local_pos.y + 10, local_pos.z), Color(g_Options.color_foot_trail));
		}
		else
		{
			DrawBeamPaw(local_pos, Vector(local_pos.x, local_pos.y + 10, local_pos.z), rainbow_col);
		}
	}*/
}

void Visuals::NullingIndicator() noexcept
{
	if (!g_Options.misc_nullindicator)
		return;

	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;
	if (!g_LocalPlayer->IsAlive())
		return;

	int w, h;
	int centerW, centerh, topH;
	g_EngineClient->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;

	if (GetAsyncKeyState(0x41) && GetAsyncKeyState(0x44) && !GetAsyncKeyState(0x57) && !GetAsyncKeyState(0x53) || GetAsyncKeyState(0x57) && GetAsyncKeyState(0x53) && !GetAsyncKeyState(0x41) && !GetAsyncKeyState(0x44) || GetAsyncKeyState(0x57) && GetAsyncKeyState(0x53) && GetAsyncKeyState(0x41) && GetAsyncKeyState(0x44))
	{
		Render::Get().RenderText("NULLING", 60, (centerh + 50), 24.f, Color(200, 5, 15), true, true, g_pVelocityFont);
	}
}

void Visuals::keystrokes() noexcept
{
	if (!g_Options.misc_keystrokes)
		return;

	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;
	if (!g_LocalPlayer->IsAlive())
		return;

	int w, h;
	int centerW, centerh, topH;
	g_EngineClient->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;

	g_VGuiSurface->FilledRect(10, 200 + 200, 30, 35, Color(0, 0, 0, 155));
	g_VGuiSurface->FilledRect(10 + 32, 200 + 200, 30, 35, Color(0, 0, 0, 155));
	g_VGuiSurface->FilledRect(10 + 32 + 32, 200 + 200, 30, 35, Color(0, 0, 0, 155));
	g_VGuiSurface->FilledRect(10 + 32, 200 - 37 + 200, 30, 35, Color(0, 0, 0, 155));
	g_VGuiSurface->FilledRect(10, 100 + 160 + 177, 95, 35, Color(0, 0, 0, 155));


	if (GetAsyncKeyState(0x41)) {
		g_VGuiSurface->FilledRect(10, 200 + 200, 30, 35, Color(0, 0, 0, 155));
	}

	if (GetAsyncKeyState(0x53)) {
		g_VGuiSurface->FilledRect(10 + 32, 200 + 200, 30, 35, Color(0, 0, 0, 155));
	}

	if (GetAsyncKeyState(0x44)) {
		g_VGuiSurface->FilledRect(10 + 32 + 32, 200 + 200, 30, 35, Color(0, 0, 0, 155));
	}

	if (GetAsyncKeyState(0x57)) {
		g_VGuiSurface->FilledRect(10 + 32, 200 - 37 + 200, 30, 35, Color(0, 0, 0, 155));
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		g_VGuiSurface->FilledRect(10, 100 + 160 + 177, 95, 35, Color(0, 0, 0, 155));
	}

	if (GetAsyncKeyState(0x41))
	{
		Render::Get().RenderText("A", 17, 205 + 200, 22.f, Color(0, 0, 255), false, true, g_pVelocityFont);
		//	g_VGuiSurface->DrawT(18, 205 + 200, Color(0, 0, 255), g_pVelocityFont, false, "a");

	}
	else
	{
		Render::Get().RenderText("A", 17, 205 + 200, 22.f, Color(255, 255, 255), false, true, g_pVelocityFont);
		//	g_VGuiSurface->DrawT(18, 205 + 200, Color(255, 255, 255, 255), g::key_font, false, "a");

	}

	if (GetAsyncKeyState(0x53)) {
		Render::Get().RenderText("S", 17 + 34, 205 + 200, 22.f, Color(0, 0, 255), false, true, g_pVelocityFont);
		//	g_VGuiSurface->DrawT(17 + 35, 205 + 200, Color(0, 0, 255), g::key_font, false, "s");

	}
	else
	{
		Render::Get().RenderText("S", 17 + 34, 205 + 200, 22.f, Color(255, 255, 255), false, true, g_pVelocityFont);
		//g_VGuiSurface->DrawT(17 + 35, 205 + 200, Color(255, 255, 255, 255), g::key_font, false, "s");
	}

	if (GetAsyncKeyState(0x44)) {

		Render::Get().RenderText("D", 17 + 34 + 32, 205 + 200, 22.f, Color(0, 0, 255), false, true, g_pVelocityFont);
		//g_VGuiSurface->DrawT(17 + 35 + 32, 205 + 200, Color(0, 0, 255), g::key_font, false, "d");

	}
	else
	{
		Render::Get().RenderText("D", 17 + 34 + 32, 205 + 200, 22.f, Color(255, 255, 255), false, true, g_pVelocityFont);
		//g_VGuiSurface->DrawT(17 + 35 + 32, 205 + 200, Color(255, 255, 255, 255), g::key_font, false, "d");
	}

	if (GetAsyncKeyState(0x57)) {
		Render::Get().RenderText("W", 17 + 30, 205 - 35 + 200, 22.f, Color(0, 0, 255), false, true, g_pVelocityFont);
		//g_VGuiSurface->DrawT(18 + 31, 205 - 35 + 200, Color(0, 0, 255), g::key_font, false, "w");

	}
	else
	{
		Render::Get().RenderText("W", 17 + 30, 205 - 35 + 200, 22.f, Color(255, 255, 255), false, true, g_pVelocityFont);
		//	g_VGuiSurface->DrawT(18 + 31, 205 - 35 + 200, Color(255, 255, 255, 255), g::key_font, false, "w");
	}

	if (GetAsyncKeyState(VK_SPACE))
	{
		Render::Get().RenderText("SPACE", 26, 385 - 120 + 177, 22.f, Color(0, 0, 255), false, true, g_pVelocityFont);
		//g_VGuiSurface->DrawT(30, 385 - 120 + 177, Color(0, 0, 255), g::key_font, false, "space");

	}
	else
	{
		Render::Get().RenderText("SPACE", 26, 385 - 120 + 177, 22.f, Color(255, 255, 255), false, true, g_pVelocityFont);
		//g_VGuiSurface->DrawT(30, 385 - 120 + 177, Color(255, 255, 255, 255), g::key_font, false, "space");
	}

}

bool lastvelsaved = false; //saver
int lastjump, lastvel, lasttick = 0; // last vel holder

void Visuals::VelocityDraw() noexcept
{
	if (!g_Options.misc_velocitydraw)
		return;

	//auto local_player = reinterpret_cast<C_BaseEntity*>(interfaces::entity_list->get_client_entity(interfaces::engine->get_local_player()));
	if (!g_EngineClient->IsConnected())
		return;
	if (!g_EngineClient->IsInGame())
		return;
	if (!g_LocalPlayer->IsAlive())
		return;



	char jew[64];
	float lspeed = g_LocalPlayer->m_vecVelocity().Length2D();
	int w, h;
	int centerW, centerh, topH;

	g_EngineClient->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;

	g_Options.rainbow += 0.0005f;
	if (g_Options.rainbow > 1.f)
		g_Options.rainbow = 0.f;

	sprintf_s(jew, "%.f", lspeed);

	if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
	{
		if (lastvelsaved)
		{
			lastvelsaved = false;
		}

		if (g_Options.rainbow_velocitydraw)
		{
			Render::Get().RenderText(jew, (centerW), (centerh + 300), 34.f, Color::FromHSB(g_Options.rainbow, 1.f, 1.f), true, true, g_pVelocityFont);
		}
		else
		{
			if (lspeed <= 269)
			{
				Render::Get().RenderText(jew, (centerW), (centerh + 300), 34.f, Color(50, 215, 116), true, true, g_pVelocityFont);
			}
			else if (lspeed >= 270 && lspeed <= 289)
			{
				Render::Get().RenderText(jew, (centerW), (centerh + 300), 34.f, Color(254, 202, 3), true, true, g_pVelocityFont);
			}
			else if (lspeed >= 290)
			{
				Render::Get().RenderText(jew, (centerW), (centerh + 300), 34.f, Color(255, 0, 255), true, true, g_pVelocityFont);
			}
		}
	}
	else
	{
		if (!lastvelsaved)
		{
			lastjump = lspeed;
			lastvelsaved = true;
		}

		if (g_Options.rainbow_velocitydraw)
		{
			if (lspeed == 0)
			{
				Render::Get().RenderText(jew, (centerW), (centerh + 300), 34.f, Color::FromHSB(g_Options.rainbow, 1.f, 1.f), true, true, g_pVelocityFont);
			}
			else
			{
				Render::Get().RenderText(jew + std::string(" (") + std::to_string(lastjump) + std::string(")"), (centerW), (centerh + 300), 34.f, Color::FromHSB(g_Options.rainbow, 1.f, 1.f), true, true, g_pVelocityFont);
			}
		}
		else
		{
			if (lspeed == 0)
			{
				Render::Get().RenderText(jew, (centerW), (centerh + 300), 34.f, Color(50, 215, 116), true, true, g_pVelocityFont);
			}
			else if (lspeed <= 269)
			{
				Render::Get().RenderText(jew + std::string(" (") + std::to_string(lastjump) + std::string(")"), (centerW), (centerh + 300), 34.f, Color(50, 215, 116), true, true, g_pVelocityFont);
			}
			else if (lspeed >= 270 && lspeed <= 289)
			{
				Render::Get().RenderText(jew + std::string(" (") + std::to_string(lastjump) + std::string(")"), (centerW), (centerh + 300), 34.f, Color(254, 202, 3), true, true, g_pVelocityFont);
			}
			else if (lspeed >= 290)
			{
				Render::Get().RenderText(jew + std::string(" (") + std::to_string(lastjump) + std::string(")"), (centerW), (centerh + 300), 34.f, Color(255, 0, 255), true, true, g_pVelocityFont);
			}
		}
	}
}

void Visuals::EdgeBugDetector()
{
	if (!g_Options.misc_ebugdetector)
		return;

	const auto velocity = g_LocalPlayer->m_vecVelocity();
	const auto speed = velocity.Length2D();
	bool edgebugged = false;
	const auto predicted_velocity = velocity.z * g_GlobalVars->interval_per_tick;

	if (velocity.z >= -7 || predicted_velocity != -7 || g_LocalPlayer->m_fFlags() & FL_ONGROUND || g_LocalPlayer->IsAlive())
	{
		edgebugged = false;
	}
	else
	{
		edgebugged = true;
		int w, h;
		g_EngineClient->GetScreenSize(w, h);
		Render::Get().RenderText("EDGEBUGGED", ImVec2(w + 100, 0), 25.f, Color(0, 100, 250), false, true);
		/*if (eb_counter)
			++edgebugs;*/
	}
}

void Visuals::VelocityGraph() noexcept
{
	if (!g_Options.misc_velocitygraph)
		return;

	if (!g_LocalPlayer)
		return;

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer->IsAlive())
		return;

	static std::vector<float> velData(120, 0);

	Vector vecVelocity = g_LocalPlayer->m_vecVelocity();
	float currentVelocity = sqrt(vecVelocity.x * vecVelocity.x + vecVelocity.y * vecVelocity.y);

	velData.erase(velData.begin());
	velData.push_back(currentVelocity);

	float vel = g_LocalPlayer->m_vecVelocity().Length2D();
	char velwstr[64];
	sprintf_s(velwstr, "%.f", vel);

	int width, height;
	int centerW, centerh, topH;

	g_EngineClient->GetScreenSize(width, height);
	centerW = width / 2;
	centerh = height / 2;

	for (auto i = 0; i < velData.size() - 1; i++)
	{
		int cur = velData.at(i);
		int next = velData.at(i + 1);

		Render::Get().RenderLine(width / 2 + (velData.size() * 5 / 2) - (i - 1) * 5.f, height / 2 - (std::clamp(cur, 0, 450) * .2f) + 275, width / 2 + (velData.size() * 5 / 2) - i * 5.f, height / 2 - (std::clamp(next, 0, 450) * .2f) + 275, Color(255, 255, 255));
	}
}
//--------------------------------------------------------------------------------
bool bombisready = false;

void Visuals::Listener()
{
	g_GameEvents->AddListener(&bomb_planted_listener, "bomb_planted", false);
	g_GameEvents->AddListener(&round_end_listener, "round_end", false);
	//g_GameEvents->AddListener(&vote_cast_listener, "vote_cast", false);
}

void Visuals::RemoveListener()
{
	g_GameEvents->RemoveListener(&bomb_planted_listener);
	g_GameEvents->RemoveListener(&round_end_listener);
//	g_GameEvents->RemoveListener(&vote_cast_listener);
}

/*void vote_cast_event::FireGameEvent(IGameEvent* pEvent)
{
	if (!strcmp(pEvent->GetName(), "vote_cast") && g_Options.misc_voterevealer)
	{
		int vote = pEvent->GetInt("vote_option");
		int id = pEvent->GetInt("entityid");

		//	C_BaseHudChat* chat = g_ClientMode->m_pChatElement;

		if (g_ChatElement)
		{
			player_info_t player;
			g_EngineClient->GetPlayerInfo(id, &player);

			if (player.szName) {
				g_ChatElement->ChatPrintf(0, 0, std::string("").
					append(" \x06"). //Yellow
					append("[Nirvana]").
					append(" \x01"). //Default Color
					append("Voted").
					append((vote == 0 ? std::string(" \x04").append("YES") : std::string(" \x02").append("NO"))).
					append("\x01"). //Default Color
					append(": [").
					append("\x0A"). //Light Blue
					append(player.szName).
					append("\x01"). //Default Color
					append("]").c_str());
			}
		}
	}
}*/

void round_end_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (strstr(p_event->GetName(), "round_end")) {
		bombisready = false;
	}
}

void bomb_planted_event::FireGameEvent(IGameEvent* p_event)
{
	/* if we are connected */
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_LocalPlayer)
		return;

	/* return if not event */
	if (!p_event)
		return;

	if (strstr(p_event->GetName(), "bomb_planted")) {
		bombisready = true;
	}
}
void Visuals::FireGameEvent(IGameEvent* event) 
{
	if (!strcmp(event->GetName(), "player_hurt"))
	{
		auto victim = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		int attacker = event->GetInt("attacker");
		auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(victim));
		if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
		{
			if (!strcmp(event->GetName(), "player_death"))
				return;

			hitmarker_event(event);
		}
	}

	if (!strcmp(event->GetName(), "player_death"))
	{
		auto victim = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		int attacker = event->GetInt("attacker");
		auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(victim));
		if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
		{
			killsound_event(event);
			KillCounterEvent(event);
		}
	}

	/*if (strcmp(event->GetName(), "round_start"))
	{
		killcounter_kills = 0;
	}*/
}
//--------------------------------------------------------------------------------
int Visuals::GetEventDebugID(void) {
	return EVENT_DEBUG_ID_INIT;
}
//--------------------------------------------------------------------------------
void Visuals::DrawLine(float x1, float y1, float x2, float y2, Color color, float size) {
	g_VGuiSurface->DrawSetColor(color);

	if (size == 1.f)
		g_VGuiSurface->DrawLine(x1, y1, x2, y2);
	else
		g_VGuiSurface->DrawFilledRect(x1 - (size / 2.f), y1 - (size / 2.f), x2 + (size / 2.f), y2 + (size / 2.f));
}

void Visuals::DrawBox(float x1, float y1, float x2, float y2, Color clr, float size) {
	DrawLine(x1, y1, x2, y1, clr, size);
	DrawLine(x1, y2, x2, y2, clr, size);
	DrawLine(x1, y1, x1, y2, clr, size);
	DrawLine(x2, y1, x2, y2 + 1, clr, size);
}
//--------------------------------------------------------------------------------
void Visuals::DrawBoxEdges(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size) {
	if (fabs(x1 - x2) < (edge_size * 2)) {
		//x2 = x1 + fabs(x1 - x2);
		edge_size = fabs(x1 - x2) / 4.f;
	}

	DrawLine(x1, y1, x1, y1 + edge_size + (0.5f * edge_size), clr, size);
	DrawLine(x2, y1, x2, y1 + edge_size + (0.5f * edge_size), clr, size);
	DrawLine(x1, y2, x1, y2 - edge_size - (0.5f * edge_size), clr, size);
	DrawLine(x2, y2, x2, y2 - edge_size - (0.5f * edge_size), clr, size);
	DrawLine(x1, y1, x1 + edge_size, y1, clr, size);
	DrawLine(x2, y1, x2 - edge_size, y1, clr, size);
	DrawLine(x1, y2, x1 + edge_size, y2, clr, size);
	DrawLine(x2, y2, x2 - edge_size, y2, clr, size);
}
//--------------------------------------------------------------------------------
void Visuals::RenderSounds() {
	UpdateSounds();

	auto Add3DCircle = [](const Vector& position, Color color, float radius) {
		float precision = 24.0f;
		if (radius >= 60.0f)
			precision = 48.0f;
		else if (radius >= 30.0f)
			precision = 36.0f;

		const float step = DirectX::XM_2PI / precision;

		for (float a = 0.f; a < DirectX::XM_2PI; a += step) {
			Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
			Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

			Vector start2d, end2d;
			if (!Math::WorldToScreen(start, start2d) || !Math::WorldToScreen(end, end2d))
				return;

			Render::Get().RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
		}
	};


	for (auto& [entIndex, sound] : m_Sounds) {
		if (sound.empty())
			continue;

		for (auto& info : sound) {
			if (info.soundTime + g_Options.esp_sounds_time < g_GlobalVars->realtime)
				info.alpha -= g_GlobalVars->frametime;

			if (info.alpha <= 0.0f)
				continue;

			float deltaTime = g_GlobalVars->realtime - info.soundTime;

			auto factor = deltaTime / g_Options.esp_sounds_time;
			if (factor > 1.0f)
				factor = 1.0f;

			float radius = g_Options.esp_sounds_radius * factor;
			Color color = Color(g_Options.color_esp_sounds[0], g_Options.color_esp_sounds[1], g_Options.color_esp_sounds[2],
				info.alpha);

			Add3DCircle(info.soundPos, color, radius);
		}

		while (!sound.empty()) {
			auto& back = sound.back();
			if (back.alpha <= 0.0f)
				sound.pop_back();
			else
				break;
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::UpdateSounds() {
	CUtlVector< SndInfo_t > sounds;
	g_EngineSound->GetActiveSounds(sounds);
	if (sounds.Count() < 1)
		return;

	Vector eye_pos = g_LocalPlayer->GetEyePos();
	for (int i = 0; i < sounds.Count(); ++i) {
		SndInfo_t& sound = sounds.Element(i);
		if (sound.m_nSoundSource < 1)
			continue;

		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(sound.m_nSoundSource);
		if (!player)
			continue;

		if (player->m_hOwnerEntity().IsValid() && player->IsWeapon()) {
			player = (C_BasePlayer*)player->m_hOwnerEntity().Get();
		}

		if (!player->IsPlayer() || !player->IsAlive())
			continue;

		if ((C_BasePlayer*)g_LocalPlayer == player || (g_LocalPlayer->m_iTeamNum() == player->m_iTeamNum()))
			continue;

		auto& player_sound = m_Sounds[player->EntIndex()];
		if (player_sound.size() > 0) {
			bool should_break = false;
			for (const auto& snd : player_sound) {
				if (snd.guid == sound.m_nGuid) {
					should_break = true;
					break;
				}
			}

			if (should_break)
				continue;
		}

		SoundInfo_t& snd = player_sound.emplace_back();
		snd.guid = sound.m_nGuid;
		snd.soundPos = *sound.m_pOrigin;
		snd.soundTime = g_GlobalVars->realtime;
		snd.alpha = 1.0f;
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderSkeletBacktrack(C_BasePlayer* player)
{
	studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(ctx.pl->GetModel());
	if (studioHdr)
	{
		if (g_Backtrack.data[player->EntIndex()].size() > 0)
		{
			backtrack_data record = g_Backtrack.data[player->EntIndex()].back();


			for (int i = 0; i < studioHdr->numbones; i++)
			{
				mstudiobone_t* bone = studioHdr->GetBone(i);
				if (!bone || !(bone->flags & BONE_USED_BY_HITBOX) || bone->parent == -1)
					continue;

				Vector bonePos1;
				if (!Math::WorldToScreen(Vector(record.boneMatrix[i][0][3], record.boneMatrix[i][1][3], record.boneMatrix[i][2][3]), bonePos1))
					continue;

				Vector bonePos2;
				if (!Math::WorldToScreen(Vector(record.boneMatrix[bone->parent][0][3], record.boneMatrix[bone->parent][1][3], record.boneMatrix[bone->parent][2][3]), bonePos2))
					continue;

				auto color = Color{};

				color = Color(
					int(g_Options.color_skeleton_backtrack[0] * 255),
					int(g_Options.color_skeleton_backtrack[1] * 255),
					int(g_Options.color_skeleton_backtrack[2] * 255),
					int(255));

				Render::Get().RenderLine((int)bonePos1.x, (int)bonePos1.y, (int)bonePos2.x, (int)bonePos2.y, Color(color), 1.0f);
				//Visuals::Get().DrawLine((int)bonePos1.x, (int)bonePos1.y, (int)bonePos2.x, (int)bonePos2.y, Color(color));

			}


		}
	}
}

void Visuals::Player::RenderSkeleton(C_BaseEntity* ent) {
	auto model = ent->GetModel();
	if (ent && !model)
		return;

	studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel(model);

	if (pStudioModel) {
		static matrix3x4_t pBoneToWorldOut[128];

		if (ent->SetupBones(pBoneToWorldOut, 128, 256, g_GlobalVars->curtime)) {
			for (int i = 0; i < pStudioModel->numbones; i++) {
				mstudiobone_t* pBone = pStudioModel->GetBone(i);
				if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
					continue;

				Vector vBonePos1;
				if (!Math::WorldToScreen(Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]), vBonePos1))
					continue;

				Vector vBonePos2;
				if (!Math::WorldToScreen(Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]), vBonePos2))
					continue;

				auto color = Color{};

				color = Color(
					int(g_Options.color_skeleton[0] * 255),
					int(g_Options.color_skeleton[1] * 255),
					int(g_Options.color_skeleton[2] * 255),
					int(255));

				Render::Get().RenderLine((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, Color(color), 1.0f);
			}
		}
	}
}

/*void Visuals::sniperCrosshair() noexcept
{
	static auto showSpread = g_CVar->FindVar("weapon_debug_spread_show");
	showSpread->SetValue(g_LocalPlayer && !g_LocalPlayer->m_bIsScoped ? 3 : 0);
}*/

void Visuals::sniperCrosshair() noexcept
{
/*	if (!g_LocalPlayer)
		return;

	static auto weapon_debug_spread_show = g_CVar->FindVar("weapon_debug_spread_show");

	if (g_LocalPlayer && g_LocalPlayer->m_iHealth() > 0) {
		weapon_debug_spread_show->SetValue(g_LocalPlayer->m_bIsScoped() || !g_Options.misc_snipercrosshair ? 0 : 3);
	}*/
}

void Visuals::Player::RenderBacktrack() {
	Vector localEyePos = g_LocalPlayer->GetEyePos();
	QAngle angles;
	int tick_count = -1;
	float best_fov = 180.0f;
	for (auto& node : g_Backtrack.data) {
		auto& cur_data = node.second;
		if (cur_data.empty())
			return;

		for (auto& bd : cur_data) {
			float deltaTime = g_Backtrack.correct_time - (g_GlobalVars->curtime - bd.simTime);
			if (std::fabsf(deltaTime) > g_Aimbot.settings.backtrack_time / 1000.f)
				continue;

			Vector w2s;
			if (!Math::WorldToScreen(bd.hitboxPos, w2s))
				continue;

			Render::Get().RenderBoxFilled(w2s.x - 1.0f, w2s.y - 1.0f, w2s.x + 1.0f, w2s.y + 1.0f, Color::White);
		}
	}

}

/*
Vector Extrplt(C_BasePlayer* pTarget, Vector point, float value) {
	point += pTarget->m_vecVelocity() * (g_GlobalVars->interval_per_tick * (float)value);

	return point;
}

void Visuals::Player::DrawForwardtrack(C_BasePlayer* pPlayer) {
	INetChannelInfo* nci = g_EngineClient->GetNetChannelInfo();
	auto ping = nci->GetLatency(FLOW_OUTGOING);
	if (ping > 0.199f) // check to not go outside of backtrack window (+200/-200ms)
	{
		ping = 0.199f - g_GlobalVars->interval_per_tick;
	}

	Vector head = pPlayer->GetBonePos((int)HITBOX_HEAD);
	Vector forwardTrack = Extrplt(pPlayer, head, TIME_TO_TICKS(ping));

	Vector output = Vector(0, 0, 0);
	g_DebugOverlay->ScreenPosition(
		forwardTrack,
		output);// headprints2D[shit]
	if (!output.IsZero())
		Render::Get().RenderCircleFilled(output.x, output.y, 10, 2, Color(0, 0, 255));
	//Draw::Rectangle(forwardTrack.x - 2, forwardTrack.y - 2, forwardTrack.x + 2, forwardTrack.y + 2, Color(0, 255, 0));
}*/

void Visuals::Player::RenderBox()
{
	switch (g_Options.esp_player_boxes_type) 
	{
	case 0:
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.clr, 1, 0);
		break;
	case 1:
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.clr, 1, 1);
		break;
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName() {
	player_info_t info = ctx.pl->GetPlayerInfo();

	float off = 7;
	int boxx = ctx.bbox.left - off;
	int boxy = ctx.bbox.top;

	if (g_Options.use_tahomafont)
	{
		auto sz = g_pTahomaFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);

		Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y - 2, 14.f, Color(255, 255, 255), false, true, g_pTahomaFont);
	}
	else
	{
		auto sz = g_pSmallestPixelFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);

		Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y - 2, 14.f, Color(255, 255, 255), false, true, g_pSmallestPixelFont);
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::GrenadeKillAlert() {
	auto hp = ctx.pl->m_iHealth();
	auto alerttext = "Grenade Kill";

	if (g_Options.use_tahomafont)
	{
		auto sz = g_pTahomaFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, alerttext);

		if (hp == 1)
		{
			Render::Get().RenderText("Grenade Kill", ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y - 12, 14.f, Color(62, 161, 255), false, true, g_pTahomaFont); //or Color(255, 237, 131)
		}
	}
	else
	{
		auto sz = g_pSmallestPixelFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, alerttext);

		if (hp == 1)
		{
			Render::Get().RenderText("Grenade Kill", ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y - 12, 14.f, Color(62, 161, 255), false, true, g_pSmallestPixelFont); //or Color(255, 237, 131)
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth() {
	auto  hp = ctx.pl->m_iHealth();
	float box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 7;

	auto health = std::to_string(hp);
	auto textSizeSmallest = g_pSmallestPixelFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, health.c_str());
	auto textSizeTahoma = g_pTahomaFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, health.c_str());

	int height = (box_h * hp) / 100;

	int green = int(hp * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	int healthLock = hp;

	if (healthLock > 100)
		healthLock = 100;

	float healthPerc = healthLock / 100.f;

	Render::Get().RenderBox(x, y - 1, x + w, y + h + 2, Color::Black, 1.f, true);
	Render::Get().RenderBox(x + 1, y, x + w - 1, y + height + 1, Color(red, green, 0, 255), 1.f, true);

	if (healthLock < 100)
	{
		if (g_Options.use_tahomafont)
		{
			Render::Get().RenderText(health, x - textSizeTahoma.x / 2 - 4, y + ((box_h * hp) / 100.f) + 1, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont); //Animation from the bottom to the top
            //Render::Get().RenderText(health, x - textSizeTahoma.x / 2 - 4, y + (h * (1.f - healthPerc)) + 1, 14.f, Color(255, 255, 255), true, true, g_pTahomaFont); //Animation from the top to bottom
		}
		else
		{
			Render::Get().RenderText(health, x - textSizeSmallest.x / 2 - 4, y + ((box_h * hp) / 100.f) + 1, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont); //Animation from the bottom to the top
            //Render::Get().RenderText(health, x - textSizeSmallest.x / 2 - 4, y + (h * (1.f - healthPerc)) + 1, 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont); //Animation from the top to bottom
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderArmour() {
	auto armour = ctx.pl->m_ArmorValue();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float offtopic = 4;

	int height = (((box_h * armour) / 100));

	int x = ctx.bbox.right + 4;
	int y = ctx.bbox.top;
	/*int x = esp_ctx.bbox.left + offtopic;
	int y = esp_ctx.bbox.bottom;*/
	
	int w = 4;
	int h = box_h;
	
	Render::Get().RenderBox(x, y - 1, x + w, y + h + 2, Color::Black, 1.f, true);
	Render::Get().RenderBox(x + 1, y, x + w - 1, y + height + 1, Color(62, 161, 255, 255), 1.f, true);
}
//--------------------------------------------------------------------------------

void Visuals::Player::RenderWeaponName() {
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;

	auto text = weapon_data->szHudName + 7;
//	auto iconfont = g_pAstriumFont->CalcTextSizeA(18.f, FLT_MAX, 0.0f, weapon->WeaponIconDefinition().c_str());

	if (g_Options.use_tahomafont)
	{
		auto size = g_pTahomaFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, text);
		Render::Get().RenderText(text, ctx.feet_pos.x, ctx.feet_pos.y, 14.f, Color::White, true, true, g_pTahomaFont);
	}
	else
	{
		auto size = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, text);
		Render::Get().RenderText(text, ctx.feet_pos.x, ctx.feet_pos.y, 14.f, Color::White, true, true, g_pSmallestPixelFont);
	}
}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent) {
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

//	Render::Get().RenderBox(bbox, Color(g_Options.color_esp_weapons));

	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);
	if (!name)
		return;

	auto iconfont = g_pAstriumFont->CalcTextSizeA(18.f, FLT_MAX, 0.0f, ent->WeaponIconDefinition().c_str());
	int w = bbox.right - bbox.left;

	switch (g_Options.esp_dropped_weapons_type)
	{
	case 0:
		if (g_Options.use_tahomafont)
		{
			auto sz = g_pTahomaFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);

			Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(g_Options.color_esp_weapons), false, true, g_pTahomaFont);
		}
		else
		{
			auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);

			Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(g_Options.color_esp_weapons), false, true, g_pSmallestPixelFont);
		}
		break;
	case 1:
		Render::Get().RenderText(ent->WeaponIconDefinition().c_str(), ImVec2((bbox.left + w * 0.5f) - iconfont.x * 0.5f, bbox.bottom + 1), 18.f, Color(g_Options.color_esp_weapons), false, true, g_pAstriumFont);
		break;
	}
}
//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent) {
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, Color::White);

	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color::Yellow, true, true);
}
//--------------------------------------------------------------------------------

C_BasePlayer* get_bomb_player()
{
	/*int maxEntityIndex = g_EntityList->GetHighestEntityIndex();
	for (int i = 0; i < maxEntityIndex; i++)*/
	for (int i = 1; i < g_EntityList->GetMaxEntities(); i++)
	{
		auto* entity = C_BasePlayer::GetPlayerByIndex(i);
		if (!entity || entity->IsPlayer() || entity->IsDormant() || entity == g_LocalPlayer)
			continue;

		if (entity->IsPlantedC4())
			return entity;
	}

	return nullptr;
}

C_PlantedC4* get_bomb()
{
	C_BaseEntity* entity;
	/*int maxEntityIndex = g_EntityList->GetHighestEntityIndex();
	for (int i = 0; i < maxEntityIndex; i++)*/
	for (auto i = 1; i < g_EntityList->GetMaxEntities(); ++i)
	{
		entity = C_BaseEntity::GetEntityByIndex(i);
		if (entity && !entity->IsDormant() && entity->IsPlantedC4())
			return reinterpret_cast<C_PlantedC4*>(entity);
	}

	return nullptr;
}

float get_defuse_time(C_PlantedC4* bomb)
{
	static float defuse_time = -1;

	if (!bomb)
		return 0;

	if (!bomb->m_hBombDefuser())
		defuse_time = -1;

	else if (defuse_time == -1)
		defuse_time = g_GlobalVars->curtime + bomb->m_flDefuseLength();

	if (defuse_time > -1 && bomb->m_hBombDefuser())
		return defuse_time - g_GlobalVars->curtime;

	return 0;
}

void draw_window_c4(C_BaseEntity* ent)
{
	if (!ent)
		return;

	float bombTimer = ent->m_flC4Blow() - g_GlobalVars->curtime;

	int x;
	int y;

	g_EngineClient->GetScreenSize(x, y);

	int windowX = 0;
	int windowY = y - 430;

	int windowSizeX = 86;
	static int windowSizeY = 15; // 3 item - 35 | 2 item - 25 | 1 item - 15
	const auto bomb = get_bomb();
	if (!bomb)
		return;

	if (bombTimer < 0)
		return;

	bool bomb_defusing_with_kits = false;

	Render::Get().RenderBoxFilled(windowX, windowY, windowX + windowSizeX, windowY + windowSizeY, Color(0, 0, 0, 100));
	Render::Get().RenderBoxFilled(windowX, windowY, windowX + 2, windowY + windowSizeY, Color(100, 100, 255));

	Render::Get().RenderText("Bomb: ", ImVec2(windowX + 4, windowY + 2), 11, Color(255, 255, 255, 255), false, false, g_pSmallestPixelFont);
	ImVec2 textSize_bomb = g_pDefaultFont->CalcTextSizeA(11, FLT_MAX, 0.0f, "bomb: ");
	std::string bomb_timer_text;
	//Math::sprintfs(bomb_timer_text, "%4.3f", bombTimer);

	char buff[228];

	snprintf(buff, sizeof(buff), "%4.3f", bombTimer);

	bomb_timer_text = buff;

	Render::Get().RenderText(bombTimer >= 0 ? bomb_timer_text : "0", ImVec2(windowX + 4 + textSize_bomb.x, windowY + 2), 11, Color(255, 255, 255, 255), false, false, g_pSmallestPixelFont);

	ImVec2 textSize_defuse = g_pDefaultFont->CalcTextSizeA(11, FLT_MAX, 0.0f, "defuse: ");
	float time = get_defuse_time(bomb);

	std::string defuse_timer_text;
	//Math::sprintfs(defuse_timer_text, "%4.3f", time);
	char buff_2[228];

	snprintf(buff_2, sizeof(buff_2), "%4.3f", time);

	defuse_timer_text = buff_2;

	const auto bomb_1 = get_bomb_player();

	if (!bomb_1)
		return;

	float flArmor = g_LocalPlayer->m_ArmorValue();
	float flDistance = g_LocalPlayer->GetEyePos().DistTo(bomb_1->m_angAbsOrigin());

	float a = 450.7f;
	float b = 75.68f;
	float c = 789.2f;
	float d = ((flDistance - b) / c);
	float flDamage = a * exp(-d * d);

	float flDmg = flDamage;

	if (flArmor > 0)
	{
		float flNew = flDmg * 0.5f;
		float flArmor = (flDmg - flNew) * 0.5f;

		if (flArmor > static_cast<float>(flArmor))
		{
			flArmor = static_cast<float>(flArmor) * (1.f / 0.5f);
			flNew = flDmg - flArmor;
		}

		flDamage = flNew;
	}

	std::string damage;
	char buff_3[228];

	snprintf(buff_3, sizeof(buff_3), "%i", (int)flDamage);

	damage = buff_3;

	ImVec2 textSize_damage = g_pDefaultFont->CalcTextSizeA(11, FLT_MAX, 0.0f, "Damage: ");

	if (get_defuse_time(bomb) > 0)
	{
		Render::Get().RenderText("Defuse: ", ImVec2(windowX + 4, windowY + 12), 11, Color(255, 255, 255, 255), false, false, g_pSmallestPixelFont);
		if (bombTimer < time)
		{
			Render::Get().RenderText(defuse_timer_text, ImVec2(windowX + 4 + textSize_defuse.x, windowY + 12), 11, Color(255, 50, 50, 255), false, false, g_pSmallestPixelFont);
		}
		else if (bombTimer > time)
		{
			Render::Get().RenderText(defuse_timer_text, ImVec2(windowX + 4 + textSize_defuse.x, windowY + 12), 11, Color(50, 255, 50, 255), false, false, g_pSmallestPixelFont);
		}
		if (flDamage > 1 && bombTimer >= 0)
		{
			Render::Get().RenderText("Damage: ", ImVec2(windowX + 4, windowY + 22), 11, Color(255, 255, 255, 255), false, 0, g_pSmallestPixelFont);
			Render::Get().RenderText(flDamage < 100 ? damage : "FATAL", ImVec2(windowX + 4 + textSize_damage.x, windowY + 22), 11, Color(255, 255, 255, 255), false, false, g_pSmallestPixelFont);
			windowSizeY = 35;
		}
		else
		{
			windowSizeY = 25;
		}

		float box_w = (float)fabs(0 - windowSizeX);

		float max_time;

		float width;

		if (bomb_defusing_with_kits)
		{
			width = (((box_w * time) / 5.f));
		}
		else
		{
			width = (((box_w * time) / 10.f));
		}

		Render::Get().RenderBoxFilled(windowX, windowY + windowSizeY + 2, windowX + (int)width, windowY + windowSizeY + 4, Color(50, 50, 255, 255));

	}
	else if (get_defuse_time(bomb) <= 0)
	{
		if (flDamage > 1 && bombTimer >= 0)
		{
			Render::Get().RenderText("damage: ", ImVec2(windowX + 4, windowY + 12), 11, Color(255, 255, 255, 255), false, false, g_pSmallestPixelFont);
			Render::Get().RenderText(flDamage < 100 ? damage : "you dead", ImVec2(windowX + 4 + textSize_damage.x, windowY + 12), 11, Color(255, 255, 255, 255), false, false, g_pSmallestPixelFont);
			windowSizeY = 25;
		}
		else
		{
			windowSizeY = 15;
		}
	}

	float box_w = (float)fabs(0 - windowSizeX);

	auto width = (((box_w * bombTimer) / 40.f));
	Render::Get().RenderBoxFilled(windowX, windowY + windowSizeY, windowX + (int)width, windowY + windowSizeY + 2, Color(255, 100, 100, 255));

}

RECT GetViewport()
{
	RECT viewport = { 0, 0, 0, 0 };
	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	viewport.right = w; viewport.bottom = h;

	return viewport;
}

float GetArmourHealth(float flDamage, int ArmorValue)
{
	float flCurDamage = flDamage;

	if (flCurDamage == 0.0f || ArmorValue == 0)
		return flCurDamage;

	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	float flNew = flCurDamage * flArmorRatio;
	float flArmor = (flCurDamage - flNew) * flArmorBonus;

	if (flArmor > ArmorValue)
	{
		flArmor = ArmorValue * (1.0f / flArmorBonus);
		flNew = flCurDamage - flArmor;
	}

	return flNew;
}

std::string string_format(const std::string fmt_str, ...) {
	int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
	std::unique_ptr<char[]> formatted;
	va_list ap;
	while (1) {
		formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
		strcpy(&formatted[0], fmt_str.c_str());
		va_start(ap, fmt_str);
		final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
		va_end(ap);
		if (final_n < 0 || final_n >= n)
			n += abs(final_n - n + 1);
		else
			break;
	}
	return std::string(formatted.get());
}

namespace GrenadeColors
{
	Color FLASH = Color(36, 218, 218); //Cyan
	Color FRAG = Color(218, 36, 36); //Red
	Color DECOY = Color(235, 235, 235); //White
	Color SMOKE = Color(218, 36, 127); //Magenta
	Color MOLLY = Color(218, 127, 36); //Orange
}

NadeInfo_t Visuals::GetNadeInfo(C_BaseEntity* nade)
{
	const model_t* nadeModel = nade->GetModel();

	Color nadeColor = Color(255, 255, 255, 255);
	std::string nadeName = "";

	if (!nadeModel || !nadeModel->szName)
	{
		return { nadeColor, nadeName };
	}

	if (strstr(nadeModel->szName, "flashbang"))
	{
		nadeName = "i";
		nadeColor = GrenadeColors::FLASH;
	}
	else if (strstr(nadeModel->szName, "frag"))
	{
		nadeName = "j";
		nadeColor = GrenadeColors::FRAG;
	}
	else if (strstr(nadeModel->szName, "smoke"))
	{
		nadeName = "k";
		nadeColor = GrenadeColors::SMOKE;
	}
	else if (strstr(nadeModel->szName, "decoy"))
	{
		nadeName = "m";
		nadeColor = GrenadeColors::DECOY;
	}
	else if (strstr(nadeModel->szName, "molotov"))
	{
		nadeName = "l";
		nadeColor = GrenadeColors::MOLLY;
	}
	else if (strstr(nadeModel->szName, "incendiary"))
	{
		nadeName = "n";
		nadeColor = GrenadeColors::MOLLY;
	}

	return { nadeColor, nadeName };
}

void Visuals::DrawThrowable(C_BaseEntity* throwable, ClientClass* client)
{
	if (!g_Options.esp_throwable)
		return;

	auto bbox = GetBBox(throwable);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	const model_t* nadeModel = throwable->GetModel();

	if (!nadeModel)
		return;

	studiohdr_t* hdr = g_MdlInfo->GetStudiomodel(nadeModel);

	if (!hdr)
		return;

	if (!strstr(hdr->szName, "thrown") && !strstr(hdr->szName, "dropped"))
		return;

	NadeInfo_t ni = GetNadeInfo(throwable);

	int w = bbox.right - bbox.left;

	//POINT txtSize = Render::GetTextSize(Fonts::smallESP, ni.name.c_str());
	auto txtSize = g_pAstriumFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, ni.name.c_str());

	Render::Get().RenderBoxByType(bbox.left, bbox.top, bbox.right, bbox.bottom, ni.color, 1, 1);
	Render::Get().RenderText(ni.name.c_str(), ImVec2((bbox.left + w * 0.5f) - txtSize.x * 0.5f, bbox.bottom + 1), 14.f, ni.color, true, true, g_pAstriumFont);
	//Render::Text(x + ((w - txtSize.x) / 2), y - txtSize.y - 4, ni.color, Fonts::smallESP, ni.name.c_str());
}

void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	float C4Blow = ent->m_flC4Blow();
	float ExplodeTimeRemaining = C4Blow - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);
	char timer[64]; sprintf_s(timer, "Exploding: %.1f", ExplodeTimeRemaining);

	auto name = (ExplodeTimeRemaining < 0.f) ? "Bomb" : timer;
	auto sz = g_pTahomaFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	auto sz1 = g_pSmallestPixelFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	auto sz2 = g_pAstriumFont->CalcTextSizeA(16.f, FLT_MAX, 0.0f, "o");
	int w = bbox.right - bbox.left;
	char tmp[64];

	Render::Get().RenderText("o", ImVec2((bbox.left + w * 0.5f) - sz2.x * 0.5f, bbox.top + 3), 16.f, Color(255, 255, 255), true, true, g_pAstriumFont);
	if (g_Options.use_tahomafont)
	{
		Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255), true, true, g_pTahomaFont);
	}
	else
	{
		Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz1.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
	}
}

void Visuals::C4Info(C_BaseEntity* ent)
{
	int width, height;
	int centerW, centerh, topH;
	g_EngineClient->GetScreenSize(width, height);
	centerW = width / 2;
	centerh = height / 2;

	if (ent->m_hBombDefuser())
	{
		float defusetime = ent->m_flDefuseCountDown();
		float DefuseTimeRemaining = defusetime - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);
		char TimeToDefuse[64]; sprintf_s(TimeToDefuse, "Defusing: %.1f", DefuseTimeRemaining);

	//	auto name = "Defusing: " + timer;
		if (DefuseTimeRemaining >= 0.0f)
		{
			Render::Get().RenderText(TimeToDefuse, (centerW), (centerh + 360), 20.f, Color(36, 218, 218), true, true, g_pVelocityFont);
		}
	}

	if (ent->m_nBombSite() == 0)
	{
		Render::Get().RenderText("Bombsite A", (centerW), (centerh + 380), 20.f, Color(254, 202, 3), true, true, g_pVelocityFont);
	}
	else
	{
		Render::Get().RenderText("Bombsite B", (centerW), (centerh + 380), 20.f, Color(254, 202, 3), true, true, g_pVelocityFont);
	}
}

void Visuals::MolotovTimer(C_BaseEntity* entity)
{
	if (!g_Options.esp_molotovtimer)
		return;

	auto inferno = reinterpret_cast<C_InfernoData*>(entity);
	auto origin = inferno->m_angAbsOrigin();

	Vector screen_origin;

	if (!Math::WorldToScreen(origin, screen_origin))
		return;


	auto spawn_time = inferno->GetSpawnTime();
	auto factor = (spawn_time + C_InfernoData::GetExpiryTime() - g_GlobalVars->curtime) / C_InfernoData::GetExpiryTime();

	static auto alpha = 2.0f;



	static auto size = Vector2D(35.0f, 5.0f);
	Render::Get().RenderCircleFilled(screen_origin.x, screen_origin.y - size.y * 0.5f, 60, 20, Color(15, 15, 15, 187));

	Render::Get().RenderBoxFilled(screen_origin.x - size.x * 0.5f, screen_origin.y - size.y * 0.5f - 1.0f, size.x, size.y, Color(37, 37, 37, 160));
	Render::Get().RenderBoxFilled(screen_origin.x - size.x * 0.5f + 2.0f, screen_origin.y - size.y * 0.5f, (size.x - 4.0f) * factor, size.y - 2.0f, Color(g_Options.color_molotov_timer_col));
	Render::Get().Draw3DFilledCircle(origin, 140.0f, g_Options.esp_molotovtimer ? Color(247, 174, 49, (int)(alpha * 24.0f)) : Color(247, 174, 49, (int)(alpha * 24.0f)));
	Render::Get().RenderBox(screen_origin.x - size.x * 0.5f, screen_origin.y - size.y * 0.5f, size.x, size.y, Color(7, 7, 7, 160));
	Render::Get().RenderText("FIRE ", screen_origin.x, screen_origin.y - size.y * 0.5f + 12.0f, 11, Color(g_Options.color_molotov_timer_col), true, false, g_pSmallestPixelFont);
	Render::Get().RenderText("l ", screen_origin.x + 1.0f, screen_origin.y - size.y * 0.5f - 9.0f, 11, Color(g_Options.color_molotov_timer_col), true, false, g_pSmallestPixelFont);
}

/*#define TORAD(x) ((x) * 0.01745329252)

Vector TransformVec(Vector src, Vector ang, float d) {
	Vector newPos;
	newPos.x = src.x + (cosf(TORAD(ang.y)) * d);
	newPos.y = src.y + (sinf(TORAD(ang.y)) * d);
	newPos.z = src.z + (tanf(TORAD(ang.x)) * d);
	return newPos;
}

void HeadlineESP(C_BasePlayer* player)
{
	Vector head3D = player->GetBonePos(8);
	Vector entAngles = player->GetEyePos();

	entAngles.x = -player->GetEyePos().x;

	Vector endPoint = TransformVec(head3D, entAngles, 60);
	Vector endPoint2D, head2D;

	if (!Math::WorldToScreen(head3D, head2D))
		return;

	if (!Math::WorldToScreen(endPoint, endPoint2D))
		return;

	Render::Get().RenderLine(head2D.x, head2D.y, endPoint2D.x, endPoint2D.y, Color::White, 1.0f);

}*/

//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t* itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t* hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";

	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;

	//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255), true, true, g_pSmallestPixelFont);
}
//--------------------------------------------------------------------------------
void Visuals::BombTimer(C_BaseEntity* ent) {

	if (ent->GetClientClass()->m_ClassID == ClassId::ClassId_CPlantedC4) {
		int x, y;
		g_EngineClient->GetScreenSize(x, y);
		float flblow = ent->m_flC4Blow();
		float ExplodeTimeRemaining = flblow - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);
		float fldefuse = ent->m_flDefuseCountDown();
		float DefuseTimeRemaining = fldefuse - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);
		char TimeToExplode[64]; sprintf_s(TimeToExplode, "Explode in: %.1f", ExplodeTimeRemaining);
		char TimeToDefuse[64]; sprintf_s(TimeToDefuse, "Defuse in: %.1f", DefuseTimeRemaining);
		if (ExplodeTimeRemaining > 0 && !ent->m_bBombDefused()) {
			float fraction = ExplodeTimeRemaining / ent->m_flTimerLength();
			int onscreenwidth = fraction * (x / 3);

			// Background 
			g_VGuiSurface->DrawSetColor(178, 34, 34, 255);
			g_VGuiSurface->DrawFilledRect(x / 3, y / 6, x / 3 * 2, y / 6 + 6);
			// Filled Color 
			g_VGuiSurface->DrawSetColor(47, 154, 26, 255);
			g_VGuiSurface->DrawFilledRect(x / 3, y / 6, x / 3 + onscreenwidth, y / 6 + 6);
			// Outline
			g_VGuiSurface->DrawSetColor(0, 0, 0, 255);
			g_VGuiSurface->DrawOutlinedRect(x / 3, y / 6, x / 3 * 2, y / 6 + 6);
			Render::Get().RenderText(TimeToExplode, ImVec2(x / 3 * 1.5, y / 6 - 12), 12, Color(255, 255, 255), true);
		}
		C_BasePlayer* Defuser = (C_BasePlayer*)C_BasePlayer::get_entity_from_handle(ent->m_hBombDefuser());
		if (Defuser) {
			float fraction = DefuseTimeRemaining / ent->m_flTimerLength();
			int onscreenwidth = fraction * (x / 3);

			// Background 
			g_VGuiSurface->DrawSetColor(30, 30, 30, 255);
			g_VGuiSurface->DrawFilledRect(x / 3, y / 6 + 20, x / 3 * 2, y / 6 + 26);
			// Filled Color 
			g_VGuiSurface->DrawSetColor(100, 255, 255, 255);
			g_VGuiSurface->DrawFilledRect(x / 3, y / 6 + 20, x / 3 + onscreenwidth, y / 6 + 26);
			// Outline
			g_VGuiSurface->DrawSetColor(0, 0, 0, 255);
			g_VGuiSurface->DrawOutlinedRect(x / 3, y / 6 + 20, x / 3 * 2, y / 6 + 26);
			Render::Get().RenderText(TimeToDefuse, ImVec2(x / 3 * 1.5, y / 6 + 8), 12, Color(255, 255, 255), true);
		}
	}
}
void Visuals::ThirdPerson()
{
	if (!g_LocalPlayer)
		return;

	if (g_Options.misc_thirdperson && g_LocalPlayer->IsAlive())
	{
		if (!g_Input->m_fCameraInThirdPerson)
			g_Input->m_fCameraInThirdPerson = true;

		float dist = g_Options.misc_thirdperson_dist;

		QAngle* view = g_LocalPlayer->GetVAngles();
		trace_t tr;
		Ray_t ray;

		Vector desiredCamOffset = Vector(cos(DEG2RAD(view->yaw)) * dist, sin(DEG2RAD(view->yaw)) * dist, sin(DEG2RAD(-view->pitch)) * dist);

		ray.Init(g_LocalPlayer->GetEyePos(), (g_LocalPlayer->GetEyePos() - desiredCamOffset));
		CTraceFilter traceFilter;
		traceFilter.pSkip = g_LocalPlayer;
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &traceFilter, &tr);

		Vector diff = g_LocalPlayer->GetEyePos() - tr.endpos;

		float distance2D = sqrt(abs(diff.x * diff.x) + abs(diff.y * diff.y));

		bool horOK = distance2D > (dist - 2.0f);
		bool vertOK = (abs(diff.z) - abs(desiredCamOffset.z) < 3.0f);

		float cameraDistance;

		if (horOK && vertOK)
			cameraDistance = dist;
		else
		{
			if (vertOK)
				cameraDistance = distance2D * 0.95f;
			else
				cameraDistance = abs(diff.z) * 0.95f;
		}
		g_Input->m_fCameraInThirdPerson = true;
		g_Input->m_vecCameraOffset.z = cameraDistance;
	}
	else
		g_Input->m_fCameraInThirdPerson = false;
}

auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr)
{
	Vector forward;
	Math::AngleVectors(QAngle(0.0f, angle, 0.0f), forward);
	float AngleLinesLength = 30.0f;

	Vector w2sReal;
	if (Math::WorldToScreen(origin + forward * AngleLinesLength, w2sReal)) {
		Render::Get().RenderLine(w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, Color::White, 1.0f);
		Render::Get().RenderBoxFilled(w2sReal.x - 5.0f, w2sReal.y - 5.0f, w2sReal.x + 5.0f, w2sReal.y + 5.0f, Color::White);
		Render::Get().RenderText(text, w2sReal.x, w2sReal.y - 5.0f, 14.0f, clr, true, true, g_pSmallestPixelFont);
	}
};

//--------------------------------------------------------------------------------
void Visuals::AddToDrawList() {
	if (g_Options.esp_grenade_prediction)
		GrenadePrediction::draw();

	if (g_Options.esp_sounds)
		RenderSounds();

	extern float real_angle;
	extern float view_angle;

	if (g_Options.esp_angle_lines) {
		Vector w2sOrigin;
		if (Math::WorldToScreen(g_LocalPlayer->m_vecOrigin(), w2sOrigin)) {
			drawAngleLine(g_LocalPlayer->m_vecOrigin(), w2sOrigin, view_angle, "viewangles", Color(0.937f, 0.713f, 0.094f, 1.0f));
			drawAngleLine(g_LocalPlayer->m_vecOrigin(), w2sOrigin, g_LocalPlayer->m_flLowerBodyYawTarget(), "lby", Color(0.0f, 0.0f, 1.0f, 1.0f));
			drawAngleLine(g_LocalPlayer->m_vecOrigin(), w2sOrigin, real_angle, "real", Color(0.0f, 1.0f, 0.0f, 1.0f));
		}
	}

	if (g_Options.esp_hitmarker)
		DrawHitmarker();

	if (g_Options.movement_recorder_type == 0)
	{
		movementRecorder::MovementDrawing();
	}

	RenderSniperCrosshair();
	RCSCrosshair();

	MovementIndicators();

	keystrokes();

	LegitAA_Draw();

	NullingIndicator();

	g_Aimbot.FakeBackIndicator();

	VelocityDraw();
	VelocityGraph();

	//for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) //INSANE FPS DROP
	if (g_Options.esp_enabled) 
	{
		/*int maxEntityIndex = g_EntityList->GetHighestEntityIndex();
		for (int i = 0; i < maxEntityIndex; i++)*/

		for (int i = 1; i < g_EntityList->GetMaxEntities(); i++)
		{
			auto entity = C_BaseEntity::GetEntityByIndex(i);

			if (!entity)
				continue;

			if (entity == g_LocalPlayer)
				continue;

			ClientClass* pClass = entity->GetClientClass();
			auto Playerzs = C_BasePlayer::GetPlayerByIndex(i);

			if (!pClass)
				continue;

			if (pClass->m_ClassID == ClassId::ClassId_CCSPlayer)
			{
				auto player = Player();
				if (player.Begin((C_BasePlayer*)entity))
				{
					if (g_Options.esp_player_skeleton)	 player.RenderSkeleton(entity);
					if (g_Options.esp_player_boxes)     player.RenderBox();
					if (g_Options.esp_player_weapons)   player.RenderWeaponName();
					if (g_Options.esp_player_names)     player.RenderName();
					if (g_Options.esp_grenadekillalert)     player.GrenadeKillAlert();
					if (g_Options.esp_player_health)    player.RenderHealth();
					if (g_Options.esp_player_armour)    player.RenderArmour();
					if (g_Options.esp_backtrack)		player.RenderBacktrack();
					if (g_Options.esp_backtrackskeleton)	player.RenderSkeletBacktrack(Playerzs);
					if (g_Options.esp_flags) player.DrawFlags();

				}
			}
			else
			{
				if (g_Options.esp_dropped_weapons && entity->IsWeapon())
					RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));

				if (g_Options.esp_damageindicator)
					damage_indicators.paint();

				if (entity->IsPlantedC4() && g_Options.esp_bombtimer)
				{
					RenderPlantedC4(entity);
					C4Info(entity);
				}

				if (strstr(pClass->m_pNetworkName, "Projectile"))
					DrawThrowable(entity, pClass);

				if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
					RenderDefuseKit(entity);
			}

			if (g_Options.misc_engineradar)
				entity->m_bSpotted() = true;
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::RenderSpectatorList() {
	int specs = 0;
	std::string spect = "";

	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) 
	{
		int localIndex = g_EngineClient->GetLocalPlayer();
		C_BasePlayer* pLocalEntity = C_BasePlayer::GetPlayerByIndex(localIndex);
		if (pLocalEntity) {
			for (int i = 1; i < g_EngineClient->GetMaxClients(); i++) 
			{
				C_BasePlayer* pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
				if (!pBaseEntity)										     continue;
				if (pBaseEntity->m_iHealth() > 0)							 continue;
				if (pBaseEntity == pLocalEntity)							 continue;
				if (pBaseEntity->IsDormant())								 continue;
				if (!pLocalEntity->IsAlive()) {
					if (pBaseEntity->m_hObserverTarget() != pLocalEntity->m_hObserverTarget())
						continue;
				}
				else 
				{
					if (pBaseEntity->m_hObserverTarget() != pLocalEntity)
						continue;
				}

				player_info_t pInfo;
				g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
				if (pInfo.ishltv) continue;

				spect += pInfo.szName;
				spect += "\n";
				specs++;
			}
		}
	}
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
	if (ImGui::Begin("Spectators List", nullptr, ImVec2(0, 0), 0.4F, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
	{
		//Render::Get().RenderLine(200, 10, 0, 0, Color::FromHSB(g_Options.rainbow, 1.f, 1.f), 3.f);
		if (specs > 0) spect += "\n";

		ImVec2 size = ImGui::CalcTextSize(spect.c_str());
		ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
		ImGui::Text(spect.c_str());
	}
	ImGui::End();
	ImGui::PopStyleVar();
}
//--------------------------------------------------------------------------------
void Visuals::DrawFOV() {
	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pWeapon)
		return;

	auto settings = g_Options.legitbot_items[pWeapon->m_Item().m_iItemDefinitionIndex()];

	if (settings.enabled) {

		float fov = static_cast<float>(g_LocalPlayer->GetFOV());

		int w, h;
		g_EngineClient->GetScreenSize(w, h);

		Vector2D screenSize = Vector2D(w, h);
		Vector2D center = screenSize * 0.5f;

		float ratio = screenSize.x / screenSize.y;
		float screenFov = atanf((ratio) * (0.75f) * tan(DEG2RAD(fov * 0.5f)));

		float radiusFOV = tanf(DEG2RAD(g_Aimbot.GetFov())) / tanf(screenFov) * center.x;

		Render::Get().RenderCircleFilled(center.x, center.y, radiusFOV, 32, Color(0, 0, 0, 50));
		Render::Get().RenderCircle(center.x, center.y, radiusFOV, 32, Color(0, 0, 0, 100));

		if (settings.silent) {
			float silentRadiusFOV = tanf(DEG2RAD(settings.silent_fov)) / tanf(screenFov) * center.x;
			Render::Get().RenderCircleFilled(center.x, center.y, silentRadiusFOV, 32, Color(80, 174, 238, 50)/*Color(255, 25, 10, 50)*/);
			Render::Get().RenderCircle(center.x, center.y, silentRadiusFOV, 32, Color(80, 174, 238, 100)/*Color(255, 25, 10, 100)*/);
		}
	}
}

int hitmarker_time = 0;

void Visuals::killsound_event(IGameEvent* event)
{
	if (!event)
		return;

	if (!g_LocalPlayer)
		return;

	auto attacker = g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")));

	if (!attacker)
		return;

	if (attacker == g_LocalPlayer)
	{
	//	hitmarker_time = 255;

		switch (g_Options.esp_killsound)
		{
		case 0: break;
		case 1: PlaySoundA(ara_ara, NULL, SND_ASYNC | SND_MEMORY); break; //Ara-Ara Sound
		case 2: PlaySoundA(archivesound, NULL, SND_ASYNC | SND_MEMORY); break; //Archive Sound
		case 3: g_VGuiSurface->PlaySound_("buttons\\arena_switch_press_02.wav"); break; //gamesense
		case 4: PlaySoundA(bameware, NULL, SND_ASYNC | SND_MEMORY); break; //Bameware
		case 5: PlaySoundA(bass_hit, NULL, SND_ASYNC | SND_MEMORY); break; //Bass-Hit
		case 6: PlaySoundA(coinssound, NULL, SND_ASYNC | SND_MEMORY); break; //Coin Sound
		case 7: PlaySoundA(cracksound, NULL, SND_ASYNC | SND_MEMORY); break; //Crack Sound
		case 8: PlaySoundA(gnome, NULL, SND_ASYNC | SND_MEMORY); break; //Gnome
		case 9: PlaySoundA(jotaro, NULL, SND_ASYNC | SND_MEMORY); break; //Jotaro Sound
		case 10: PlaySoundA(mario_coins, NULL, SND_ASYNC | SND_MEMORY); break; //Mario Coins
		case 11: PlaySoundA(mgs_sound, NULL, SND_ASYNC | SND_MEMORY); break; //MGS Sound
		case 12: PlaySoundA(minecrafthit, NULL, SND_ASYNC | SND_MEMORY); break; //Minecraft Hit
		case 13: PlaySoundA(penguware, NULL, SND_ASYNC | SND_MEMORY); break; //Penguware
		case 14: PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break; //Point Blank
		case 15: PlaySoundA(rifk7, NULL, SND_ASYNC | SND_MEMORY); break; //rifk-7
		case 16: PlaySoundA(woahh_sound, NULL, SND_ASYNC | SND_MEMORY); break; //Mario Woahh
		case 17: PlaySoundA(zing, NULL, SND_ASYNC | SND_MEMORY); break; //Zing
		case 18: PlaySoundA(zing_trio, NULL, SND_ASYNC | SND_MEMORY); break; //Zing-Trio

		}
	}
}

void Visuals::hitmarker_event(IGameEvent* event)
{
	if (!event)
		return;

	if (!g_LocalPlayer)
		return;

	auto attacker = g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("attacker")));

	if (!attacker)
		return;

	if (attacker == g_LocalPlayer)
	{
		hitmarker_time = 255;

		switch (g_Options.esp_hitsound)
		{
		case 0: break;
		case 1: PlaySoundA(ara_ara, NULL, SND_ASYNC | SND_MEMORY); break; //Ara-Ara Sound
		case 2: PlaySoundA(archivesound, NULL, SND_ASYNC | SND_MEMORY); break; //Archive Sound
		case 3: g_VGuiSurface->PlaySound_("buttons\\arena_switch_press_02.wav"); break; //gamesense
		case 4: PlaySoundA(bameware, NULL, SND_ASYNC | SND_MEMORY); break; //Bameware
		case 5: PlaySoundA(bass_hit, NULL, SND_ASYNC | SND_MEMORY); break; //Bass-Hit
		case 6: PlaySoundA(coinssound, NULL, SND_ASYNC | SND_MEMORY); break; //Coin Sound
		case 7: PlaySoundA(cracksound, NULL, SND_ASYNC | SND_MEMORY); break; //Crack Sound
		case 8: PlaySoundA(gnome, NULL, SND_ASYNC | SND_MEMORY); break; //Gnome
		case 9: PlaySoundA(jotaro, NULL, SND_ASYNC | SND_MEMORY); break; //Jotaro Sound
		case 10: PlaySoundA(mario_coins, NULL, SND_ASYNC | SND_MEMORY); break; //Mario Coins
		case 11: PlaySoundA(mgs_sound, NULL, SND_ASYNC | SND_MEMORY); break; //MGS Sound
		case 12: PlaySoundA(minecrafthit, NULL, SND_ASYNC | SND_MEMORY); break; //Minecraft Hit
		case 13: PlaySoundA(penguware, NULL, SND_ASYNC | SND_MEMORY); break; //Penguware
		case 14: PlaySoundA(point_blank, NULL, SND_ASYNC | SND_MEMORY); break; //Point Blank
		case 15: PlaySoundA(rifk7, NULL, SND_ASYNC | SND_MEMORY); break; //rifk-7
		case 16: PlaySoundA(woahh_sound, NULL, SND_ASYNC | SND_MEMORY); break; //Mario Woahh
		case 17: PlaySoundA(zing, NULL, SND_ASYNC | SND_MEMORY); break; //Zing
		case 18: PlaySoundA(zing_trio, NULL, SND_ASYNC | SND_MEMORY); break; //Zing-Trio

		}
	}
}

void Visuals::DrawHitmarker()
{

	if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
		return;

	int width, height;
	g_EngineClient->GetScreenSize(width, height);
	int width_mid = width / 2;
	int height_mid = height / 2;

	if (hitmarker_time > 0) {
		int alpha = hitmarker_time;

		Render::Get().RenderLine(width_mid + 6, height_mid + 6, width_mid + 3, height_mid + 3, Color(255, 255, 255, alpha));
		Render::Get().RenderLine(width_mid - 6, height_mid + 6, width_mid - 3, height_mid + 3, Color(255, 255, 255, alpha));
		Render::Get().RenderLine(width_mid + 6, height_mid - 6, width_mid + 3, height_mid - 3, Color(255, 255, 255, alpha));
		Render::Get().RenderLine(width_mid - 6, height_mid - 6, width_mid - 3, height_mid - 3, Color(255, 255, 255, alpha));

		hitmarker_time -= 2;
	}
}

















































































































































































































































































































































































































































































































































































































































































































































































































































































































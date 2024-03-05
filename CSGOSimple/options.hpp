#pragma once

#include <set>
#include <string>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <limits>
#include "kit_parser.hpp"
#include "item_definitions.hpp"
#include "valve_sdk/Misc/Color.hpp"
#define FLOAT_TO_CLR(clr) clr[0] * 255.0f, clr[1] * 255.0f, clr[2] * 255.0f, clr[3] * 255.0f

extern std::map<int, const char*> weapon_names;

class Sticker_t
{
public:
	int stickers_id = 0;
	int sticker_vector_index = 0;
};

struct item_setting
{
	char name[32] = "Default";
	bool enabled = false;
	Sticker_t stickers[5];
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 1;
	bool enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	int stat_trak = 0;
	float wear = 0.0f;
	char custom_name[32] = "";
};

struct legitbot_settings {
	bool enabled = true;
	bool deathmatch = false;
	bool autopistol = false;
	bool smoke_check = false;
	bool flash_check = false;
	bool jump_check = false;
	bool autowall = false;
	bool silent = false;
	bool autofire = false;
	bool on_key = true;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool autostop = false;
	float backtrack_time = 0.f;
	bool only_in_zoom = true;
	int autofire_key = 1;
	int key = 1;
	int aim_type = 1;
	int priority = 0;
	int fov_type = 0;
	int smooth_type = 0;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	float silent_fov = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 0;
	int min_damage = 1;
};

#define OPTION(type, var, val) type var = val

class Options
{
public:

	// AIMBOT

	std::map<int, legitbot_settings> legitbot_items = { };

	// LEGIT AA
	OPTION(bool, enable_legitaa, false);
	OPTION(bool, legit_resolver, false);
	OPTION(bool, legitaa_indicators, false);
	OPTION(int, legitaa_key, 0);
	OPTION(int, legitaa_packets, 0);

	// TRIGGERBOT

	OPTION(bool, trigger_ignoresmoke, false);
	OPTION(bool, trigger_autostop, false);
	OPTION(bool, trigger_autoscope, false);
	OPTION(float, trigger_hitchance, 0.f);
	OPTION(bool, trigger_delay, false);
	OPTION(int, trigger_delay_time, 0);
	OPTION(float, trigger_burst_time, 0.f);
	OPTION(bool, trigger_ignoreflash, false);
	OPTION(bool, trigger_scopecheck, false);
	OPTION(int, trigger_minDamage, 0);
	OPTION(int, trigger_hitgroup, 0);
	OPTION(bool, trigger_killshot, false);

	OPTION(bool, trigger_enable, false);
	OPTION(int, trigger_key, 0);

	// ESP

	OPTION(bool, esp_enabled, false);
	OPTION(bool, movement_indicators, false);
	OPTION(bool, esp_visiblecheck, false);
	OPTION(bool, esp_smokecheck, false);
	OPTION(bool, esp_enemies_only, false);
	OPTION(bool, esp_damageindicator, false);
	OPTION(bool, esp_damageindremainhp, false);
	OPTION(bool, esp_backtrackskeleton, false);
	OPTION(bool, esp_player_skeleton, false);
	OPTION(bool, esp_player_boxes, false);
	OPTION(int, esp_player_boxes_type, 0);
	OPTION(bool, esp_player_names, false);
	OPTION(bool, esp_grenadekillalert, false);
	OPTION(bool, esp_throwable, false);
	OPTION(bool, esp_player_health, false);
	OPTION(bool, esp_player_armour, false);
	OPTION(bool, esp_player_weapons, false);
	OPTION(bool, esp_dropped_weapons, false);
//	OPTION(int, esp_weapon_type, 0);
	OPTION(int, esp_dropped_weapons_type, 0);
	OPTION(bool, esp_defuse_kit, false);
	OPTION(bool, esp_planted_c4, false);
	OPTION(bool, esp_molotovtimer, false);
	OPTION(bool, esp_bombtimer, false);
	OPTION(bool, esp_bomblogs, false);
	OPTION(bool, esp_items, false);
	OPTION(bool, esp_rcscrosshair, false);
	OPTION(bool, esp_grenade_prediction, false);
	OPTION(bool, esp_backtrack, false);
	OPTION(bool, esp_motionblur, false);
	OPTION(bool, esp_draw_nrvusers, false);
	OPTION(bool, esp_sounds, false);
	OPTION(float, esp_motionblur_strength, 1.f);
	OPTION(float, esp_sounds_time, 1.0f);
	OPTION(float, esp_sounds_radius, 1.0f);
	OPTION(bool, esp_angle_lines, false);
	OPTION(bool, esp_choke_indicator, false);
	OPTION(bool, esp_hitmarker, false);
	OPTION(bool, esp_detectionsparks, false);
	OPTION(bool, esp_detectionhealthboost, false);
	OPTION(bool, esp_detectionbeam, false);
	OPTION(int, esp_hitsound, 0);
	OPTION(int, esp_killsound, 0);

	OPTION(bool, esp_flags, false);
	OPTION(bool, esp_flags_armor, false);
	OPTION(bool, esp_flags_kit, false);
	OPTION(bool, esp_flags_hostage, false);
	OPTION(bool, esp_flags_scoped, false);
	OPTION(bool, esp_flags_money, false);
	OPTION(bool, esp_flags_c4, false);
	OPTION(bool, esp_flags_defusing, false);

	// GLOW

	OPTION(bool, glow_enabled, false);
	OPTION(bool, glow_enemies_only, false);
	OPTION(bool, glow_players, false);
	OPTION(bool, glow_chickens, false);
	OPTION(bool, glow_c4_carrier, false);
	OPTION(bool, glow_planted_c4, false);
	OPTION(bool, glow_defuse_kits, false);
	OPTION(bool, glow_weapons, false);

	// CHAMS

	OPTION(bool, chams_player_enabled, false);
	OPTION(bool, chams_player_enemies_only, false);
	OPTION(bool, chams_player_wireframe, false);
	OPTION(bool, chams_player_flat, false);
	OPTION(bool, chams_player_ignorez, false);
	OPTION(bool, chams_player_glass, false);
	OPTION(bool, chams_disable_occulusion, false);
	OPTION(bool, toggle_chams_history, false);
	OPTION(int, chams_backtrack, 0);
	OPTION(int, chams_backtrack_type, 0);
	OPTION(bool, chams_desync, false);

	// OTHER

	OPTION(bool, other_nightmode, false);
	OPTION(float, other_nightmode_size, 0.05f);
	OPTION(float, other_mat_ambient_light_r, 0.0f);
	OPTION(float, other_mat_ambient_light_g, 0.0f);
	OPTION(float, other_mat_ambient_light_b, 0.0f);
	OPTION(bool, other_mat_ambient_light_rainbow, false);
	OPTION(bool, enable_world_color_modulation, false);
	OPTION(bool, enable_foottrail, false);
	OPTION(bool, foot_trailcolor, false);
	OPTION(bool, other_drawfov, false);
	OPTION(bool, other_no_smoke, false);
	OPTION(bool, other_single_thread_bones, false);
	OPTION(bool, other_no_3dsky, false);
	OPTION(bool, other_no_flash, false);
	OPTION(bool, other_no_fog, false);
	OPTION(bool, other_no_hands, false);
	OPTION(bool, other_no_shadows, false);
	OPTION(bool, enable_movement_recorder, false);
	OPTION(bool, playBool, false);
	OPTION(bool, recordBool, false);
	OPTION(bool, silent_movementrecord, false);
	OPTION(int, record_movement_key, 0);
	OPTION(int, play_movement_key, 0);
	OPTION(bool, use_tahomafont, false);
	OPTION(bool, misc_fakebackwards, false);
	OPTION(bool, misc_fakebackchooseangle, false);
	OPTION(int, fakebackwardskey, 0);
	OPTION(int, fakeback_angle, 0);
	OPTION(float, fakeback_speed, 0.0f);

	// SKINCHANGER

	struct {
		std::map<int, item_setting> m_items;
		std::unordered_map<std::string, std::string> m_icon_overrides;
		auto get_icon_override(const std::string original) const -> const char*
		{
			return m_icon_overrides.count(original) ? m_icon_overrides.at(original).data() : nullptr;
		}
	} skins;
	OPTION(int, skins_knifemodel, 0);
	OPTION(bool, enable_agentskins, false);
	OPTION(int, skins_agentmodelct, 0);
	OPTION(int, skins_agentmodeltr, 0);

	// PROFILE CHANGER
	OPTION(bool, enable_profilechanger, false);
	OPTION(int, profile_rankid, 0);
	OPTION(int, profile_friendly, -1);
	OPTION(int, profile_teaching, -1);
	OPTION(int, profile_leader, -1);
	OPTION(int, profile_wins, -1);
	OPTION(int, profile_level, -1);
	OPTION(int, profile_exp, -1);

	// MISC

	OPTION(float, rainbow, 0.f);
//	OPTION(bool, misc_backdrop, true);
	OPTION(bool, anti_untrusted, true);
	OPTION(bool, reveal_money, false);
	OPTION(bool, unlock_inventory, false);
	OPTION(bool, discord_rpc, false);
	OPTION(bool, kill_counter, false);
	OPTION(bool, misc_fastduck, false);
//	OPTION(bool, block_bot, false);
//	OPTION(int, blockbot_key, 0);
	OPTION(bool, no_panoramablur, false);
	OPTION(bool, enable_skybox_changer, false);
	OPTION(int, skybox_combo, 0);
	OPTION(bool, no_postprocess, false);
	OPTION(bool, no_blood, false);
	OPTION(bool, misc_autoaccept, true);
	OPTION(bool, misc_aspectratiotoggle, false);
	OPTION(float, misc_aspectratio, 0.f);
	OPTION(bool, misc_owrevealer, false);
	OPTION(bool, misc_antivotekick, false);
	OPTION(bool, misc_voterevealer, false);
	OPTION(bool, misc_slide_walk, false);
	OPTION(bool, misc_jumpbug, false);
	OPTION(int, misc_jumpbug_bind, 0);
	OPTION(bool, misc_ebug, false);
	OPTION(bool, misc_autoalign, false);
	OPTION(bool, misc_ebugdetector, false);
	OPTION(int, misc_ebugalignticks, 32);
	OPTION(float, misc_ebuglockfactor, 0.5f);
	OPTION(int, misc_ebug_key, 0);
	OPTION(int, misc_ebug_sound, 0);
	OPTION(bool, misc_bhop, false);
	OPTION(float, retrack_speed, 1.0f);
	OPTION(bool, misc_svpure, false);
	OPTION(bool, misc_faststop, false);
	OPTION(bool, misc_snipercrosshair, false);
	OPTION(bool, misc_keystrokes, false);
	OPTION(bool, misc_nullindicator, false);
	OPTION(bool, misc_faceitfakechat, false);
	OPTION(bool, rainbow_velocitydraw, false);
	OPTION(bool, misc_velocitydraw, false);
	OPTION(bool, misc_velocitygraph, false);
	OPTION(bool, misc_autostrafe, false);
	OPTION(bool, misc_edgejump, false);
	OPTION(int, misc_edgejump_bind, 0);
	OPTION(bool, misc_spectatorlist, false);
	OPTION(bool, misc_sonaresp, false);
	OPTION(bool, misc_copybot, false);
	OPTION(bool, misc_knifebot, false);
	OPTION(bool, misc_grenade_circle, false);
	OPTION(bool, misc_engineradar, false);
	OPTION(bool, misc_radar, false);
	OPTION(bool, misc_radar_enemyonly, false);
	OPTION(bool, misc_thirdperson, false);
	OPTION(int, misc_thirdperson_bind, 0);
	OPTION(bool, misc_showranks, true);
	OPTION(bool, misc_clantag, false);
	OPTION(bool, misc_watermark, true);
	OPTION(int, misc_desync, 0);
	OPTION(int, misc_desync_bind, 0);
	OPTION(float, misc_thirdperson_dist, 50.f);
	OPTION(bool, misc_changefov, false);
	OPTION(int, misc_override_fov, 90);
	OPTION(int, misc_viewmodel_fov, 68);
	OPTION(int, misc_slowwalk_bind, 0);
	OPTION(float, misc_slowwalk_speed, 15.0f);
	OPTION(bool, fakelatency, false);
	OPTION(float, fakelatency_amouth, 0.f);
	OPTION(bool, fakelag_enabled, false);
	OPTION(bool, fakelag_standing, false);
	OPTION(bool, fakelag_moving, false);
	OPTION(bool, fakelag_unducking, false);
	OPTION(int, fakelag_mode, 0);
	OPTION(int, fakelag_factor, 0);
	OPTION(int, movement_recorder_type, 0);

	// COLORS

	float color_beam_smoke[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_beam_molotov[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_beam_decoy[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_detection_beam[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float color_esp_ally_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_ally_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_defuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_c4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_item[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_foot_trail[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_grenade_prediction[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_sounds[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float color_glow_ally[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_enemy[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_chickens[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_c4_carrier[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_planted_c4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_defuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	float color_chams_player_ally_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_chams_player_ally_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_chams_player_enemy_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_chams_player_enemy_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_molotov_timer_col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_skeleton_backtrack[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_skeleton[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_chams_backtrack[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_damage_indicator[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_rcs_crosshair[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_world_modulation[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

static char* tabs[] = { "Legit", "Visuals", "Misc", "Skins", "Config" };
static char* Esp_tabs[] = { "ESP", "Other" };
static char* Box_type[] = { "Normal", "Cornered" };
static char* Dropped_type[] = { "Text", "Icon" };
static char* Weapon_type[] = { "Text", "Icon" };
extern Options g_Options;
extern bool   g_Unload;
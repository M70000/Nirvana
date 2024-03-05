#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "config.hpp"
#include "features/visuals.hpp"
#include "features/menu_soundeffects.h"
#include "ui.hpp"
#include "features/bhop.hpp"
#include "security.hpp"
#include "features/moverecorder.h"
#include "helpers/proto/protobuff.hpp"
#include "DiscordRPC.hpp"
#include <thread>
#include "features/skins.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#define FCVAR_HIDDEN			(1<<4)	// Hidden. Doesn't appear in find or 
#define FCVAR_UNREGISTERED		(1<<0)	// If this is set, don't add to linked list, etc.
#define FCVAR_DEVELOPMENTONLY	(1<<1)	// Hidden in released products. Flag is removed 

std::string imData;
std::string link = "";
IDirect3DTexture9* skinImage = nullptr;

void downloadImage()
{
	imData = Preview::DownloadBytes(link.c_str());
	D3DXCreateTextureFromFileInMemoryEx(g_D3DDevice9, imData.data(), imData.length(), 512, 384, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &skinImage);
}

void EnableHiddenCVars()
{
	auto p = **reinterpret_cast<ConCommandBase***>(g_CVar + 0x34);
	for (auto c = p->m_pNext; c != nullptr; c = c->m_pNext)
	{
		ConCommandBase* cmd = c;
		cmd->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
		cmd->m_nFlags &= ~FCVAR_HIDDEN;
	}
}

using KeyBindPair_t = std::pair< int, const char* >;
std::vector< KeyBindPair_t > ButtonNames{
	{ 0, ("None") },
{ 1, ("Left Mouse") },
{ 2, ("Right Mouse") },
{ 3, ("Scroll Lock") },
{ 4, ("Middle Mouse") },
{ 5, ("X1 Mouse") },
{ 6, ("X2 Mouse") },
{ 8, ("Backspace") },
{ 9, ("Tab") },
{ 12, ("Num 5") },
{ 13, ("Enter") },
{ 16, ("Shift") },
{ 17, ("Ctrl") },
{ 18, ("Alt") },
{ 20, ("Caps Lock") },
{ 27, ("Esc") },
{ 32, ("Space") },
{ 33, ("Num 9") },
{ 34, ("Num 3") },
{ 35, ("Num 1") },
{ 36, ("Num 7") },
{ 37, ("Num 4") },
{ 38, ("Num 8") },
{ 39, ("Num 6") },
{ 40, ("Num 2") },
{ 44, ("Sys Req") },
{ 45, ("Num 0") },
{ 46, ("Num Del") },
{ 48, ("0") },
{ 49, ("1") },
{ 50, ("2") },
{ 51, ("3") },
{ 52, ("4") },
{ 53, ("5") },
{ 54, ("6") },
{ 55, ("7") },
{ 56, ("8") },
{ 57, ("9") },
{ 65, ("A") },
{ 66, ("B") },
{ 67, ("C") },
{ 68, ("D") },
{ 69, ("E") },
{ 70, ("F") },
{ 71, ("G") },
{ 72, ("H") },
{ 73, ("I") },
{ 74, ("J") },
{ 75, ("K") },
{ 76, ("L") },
{ 77, ("M") },
{ 78, ("N") },
{ 79, ("O") },
{ 80, ("P") },
{ 81, ("Q") },
{ 82, ("R") },
{ 83, ("S") },
{ 84, ("T") },
{ 85, ("U") },
{ 86, ("V") },
{ 87, ("W") },
{ 88, ("X") },
{ 89, ("Y") },
{ 90, ("Z") },
{ 96, ("Num 0") },
{ 97, ("Num 1") },
{ 98, ("Num 2") },
{ 99, ("Num 3") },
{ 100, ("Num 4") },
{ 101, ("Num 5") },
{ 102, ("Num 6") },
{ 103, ("Num 7") },
{ 104, ("Num 8") },
{ 105, ("Num 9") },
{ 106, ("Num *") },
{ 107, ("Num +") },
{ 109, ("Num -") },
{ 110, ("Num Del") },
{ 111, ("/") },
{ 112, ("F1") },
{ 113, ("F2") },
{ 114, ("F3") },
{ 115, ("F4") },
{ 116, ("F5") },
{ 117, ("F6") },
{ 118, ("F7") },
{ 119, ("F8") },
{ 120, ("F9") },
{ 121, ("F10") },
{ 122, ("F11") },
{ 123, ("F12") },
{ 144, ("Pause") },
{ 145, ("Scroll Lock") },
{ 161, ("Right Shift") },
{ 186, (";") },
{ 187, ("=") },
{ 188, (",") },
{ 189, ("-") },
{ 190, (".") },
{ 191, ("/") },
{ 192, ("`") },
{ 219, ("[") },
{ 220, ("\\") },
{ 221, ("]") },
{ 222, ("'") },
{ 226, ("\\") },
};

bool KeyBind(const char* name, int& keyBind) {
	auto result = std::find_if(ButtonNames.begin(), ButtonNames.end(), [&](const KeyBindPair_t& a) { return a.first == keyBind; });
	if (result == ButtonNames.end()) {
		result = ButtonNames.begin();
		keyBind = 0;
	}

	int key = result - ButtonNames.begin();
	auto comboRes = ImGui::Combo(
		name, &key, [](void* data, int32_t idx, const char** out_text) {
		*out_text = ButtonNames[idx].second;
		return true;
	},
		nullptr, ButtonNames.size());

	keyBind = ButtonNames[key].first;
	return comboRes;
}

#pragma region RenderTabs
template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h)
{
	bool values[N] = { false };
	values[activetab] = true;
	for (auto i = 0; i < N; ++i) {
		if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) activetab = i;
		if (i < N - 1) ImGui::SameLine();
	}
}
#pragma endregion
#pragma region Legit
static int weapon_index = 7;
static int weapon_vector_index = 0;
struct WeaponName_t {
	constexpr WeaponName_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};

std::vector< WeaponName_t> WeaponNames =
{
{ 7, "AK-47" },
{ 8, "AUG" },
{ 9, "AWP" },
{ 63, "CZ75 Auto" },
{ 1, "Desert Eagle" },
{ 2, "Dual Berettas" },
{ 10, "FAMAS" },
{ 3, "Five-SeveN" },
{ 11, "G3SG1" },
{ 13, "Galil AR" },
{ 4, "Glock-18" },
{ 14, "M249" },
{ 60, "M4A1-S" },
{ 16, "M4A4" },
{ 17, "MAC-10" },
{ 27, "MAG-7" },
{ 33, "MP7" },
{ WEAPON_MP5, "MP5" },
{ 34, "MP9" },
{ 28, "Negev" },
{ 35, "Nova" },
{ 32, "P2000" },
{ 36, "P250" },
{ 19, "P90" },
{ 26, "PP-Bizon" },
{ 64, "R8 Revolver" },
{ 29, "Sawed-Off" },
{ 38, "SCAR-20" },
{ 40, "SSG 08" },
{ 39, "SG 553" },
{ 30, "Tec-9" },
{ 24, "UMP-45" },
{ 61, "USP-S" },
{ 25, "XM1014" },
};

const char* MoveRecType[] =
{
	"Basic",
	"Beta"
};

const char* SkyboxSelector[] =
{
	"Aztec",
	"Baggage",
	"Cloudy",
	"Day",
	"Day HD",
	"Daylight 1",
	"Daylight 2",
	"Daylight 3",
	"Daylight 4",
	"Dusty Sky",
	"Embassy",
	"Italy",
	"Jungle",
	"Lunacy",
	"Night 1",
	"Night 2",
	"Night Flat",
	"Nuke",
	"Office",
	"Tibet",
	"Vertigo",
	"Vertigo HD",
	"Vertigo Blue HD",
	"Vietnam"
};

const char* LJBindType[] =
{
	"Forward",
	"Back",
	"Left",
	"Right"
};

const char* HitmarkSound[] =
{
	"None",
	"Ara-Ara",
	"Archive",
	"Arena-Switch",
	"Bameware",
	"Bass-Hit",
	"Coins",
	"Crack",
	"Gnome",
	"Jotaro",
	"Mario Coins",
	"MGS",
	"Minecraft",
	"Penguware",
	"Point Blank",
	"rifk-7",
	"Woahh",
	"Zing",
	"Zing-Trio"
};


void RenderCurrentWeaponButton(float width) {
	ImGui::SameLine();
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		ImGui::Button("Current", ImVec2(width, 0.0f));
		return;
	}

	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsWeapon()) {
		ImGui::Button("Current", ImVec2(width, 0.0f));
		return;
	}

	if (ImGui::Button("Current", ImVec2(width, 0.0f))) {
		int wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
		auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a) {
			return a.definition_index == wpn_idx;
			});
		if (wpn_it != WeaponNames.end()) {
			weapon_index = wpn_idx;
			weapon_vector_index = std::abs(std::distance(WeaponNames.begin(), wpn_it));
		}
	}
}

void RenderLegitTab() {
	ImGui::Columns(3, NULL, false);

	float item_width = ImGui::GetColumnWidth() * 0.5f - ImGui::GetStyle().ItemSpacing.x;
	ImGui::PushItemWidth(item_width);
	if (ImGui::Combo(
		"##weapon_aimbot", &weapon_vector_index,
		[](void* data, int32_t idx, const char** out_text) {
			auto vec = reinterpret_cast<std::vector< WeaponName_t >*>(data);
			*out_text = vec->at(idx).name;
			return true;
		}, (void*)(&WeaponNames), WeaponNames.size())) {
		weapon_index = WeaponNames[weapon_vector_index].definition_index;
	}

	ImGui::PopItemWidth();

	RenderCurrentWeaponButton(item_width - ImGui::GetStyle().ItemSpacing.x);

	auto settings = &g_Options.legitbot_items[weapon_index];
	ImGui::BeginChild("##aimbot.general", ImVec2(0, 0), true);
	{
		ImGui::Text("General");
		ImGui::Separator();
		ImGui::PushItemWidth(-1);
		ImGui::Checkbox("Toggle", &settings->enabled);
		ImGui::Checkbox("Friendly fire", &settings->deathmatch);

		if (weapon_index == WEAPON_P250 ||
			weapon_index == WEAPON_USP_SILENCER ||
			weapon_index == WEAPON_GLOCK ||
			weapon_index == WEAPON_FIVESEVEN ||
			weapon_index == WEAPON_TEC9 ||
			weapon_index == WEAPON_DEAGLE ||
			weapon_index == WEAPON_ELITE ||
			weapon_index == WEAPON_HKP2000) {
			ImGui::Checkbox("Auto Pistol", &settings->autopistol);
		}

		ImGui::Checkbox("Smoke check", &settings->smoke_check);
		ImGui::Checkbox("Flash check ", &settings->flash_check);
		ImGui::Checkbox("Jump check", &settings->jump_check);
		ImGui::Checkbox("Silent", &settings->silent);

		ImGui::Checkbox("On Key", &settings->on_key);
		if (settings->on_key)
		{
			ImGui::Hotkey("##aimbot key", &settings->key);
		}
		if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08 ||
			weapon_index == WEAPON_AUG || weapon_index == WEAPON_SG556) {
			ImGui::Checkbox("Zoom Check", &settings->only_in_zoom);
		}


		ImGui::Separator();

		ImGui::Text("Backtrack");
		ImGui::SliderFloat("##aimbot_backtrack_time", &settings->backtrack_time, 0.f, 200.f, "%.0f ms");

		ImGui::Separator();

		ImGui::Text("Legit AA");
		ImGui::Checkbox("Toggle##legitaa_key", &g_Options.enable_legitaa);
		ImGui::SliderInt("Choked Packets", &g_Options.legitaa_packets, 1, 14, "%.f");
		ImGui::Hotkey("##legitaa key", &g_Options.legitaa_key);
		ImGui::Checkbox("Resolver##legitaa_resolver", &g_Options.legit_resolver);

		ImGui::Separator();

		ImGui::Text("Fake Backwards");
		ImGui::Checkbox("Toggle##fakebackwards_toggle", &g_Options.misc_fakebackwards);
		ImGui::Checkbox("Choose Angle##fakeangle_select", &g_Options.misc_fakebackchooseangle);
		if (g_Options.misc_fakebackchooseangle)
		{
			ImGui::SliderInt("##fakeback_angle", &g_Options.fakeback_angle, -180, 180, "%.f");
		}
		ImGui::SliderFloat("##fakeback_speed", &g_Options.fakeback_speed, 1.f, 20.f, "Speed: %.f");
		ImGui::Hotkey("##fakeback key", &g_Options.fakebackwardskey);

		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("##aimbot.misc", ImVec2(0, 0), true);
	{
		ImGui::Text("Misc");
		ImGui::Separator();
		ImGui::PushItemWidth(-1);

		static char* priorities[] = {
			"Fov",
			"Health",
			"Damage",
			"Distance"
		};

		static char* aim_types[] = {
			"Hitbox",
			"Nearest"
		};

		static char* smooth_types[] = {
			"Static",
			"Dynamic"
		};

		static char* fov_types[] = {
			"Static",
			"Dynamic"
		};

		static char* hitbox_list[] = {
			"Head",
			"Neck",
			"Pelvis",
			"Stomach",
			"Lower chest",
			"Chest",
			"Upper chest",
		};

		ImGui::Text("Aim Type:");
		ImGui::Combo("##aimbot.aim_type", &settings->aim_type, aim_types, IM_ARRAYSIZE(aim_types));

		if (settings->aim_type == 0) {
			ImGui::Text("Hitbox:");

			ImGui::Combo("##aimbot.hitbox", &settings->hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
		}

		ImGui::Text("Priority:");
		ImGui::Combo("##aimbot.priority", &settings->priority, priorities, IM_ARRAYSIZE(priorities));
		ImGui::Text("Fov Type:");
		ImGui::Combo("##aimbot.fov_type", &settings->fov_type, fov_types, IM_ARRAYSIZE(fov_types));
		ImGui::Text("Smooth Type:");
		ImGui::Combo("##aimbot.smooth_type", &settings->smooth_type, smooth_types, IM_ARRAYSIZE(smooth_types));
		ImGui::SliderFloat("##aimbot.fov", &settings->fov, 0, 30, "Fov: %.1f");

		if (settings->silent) {
			ImGui::SliderFloat("##aimbot.silent_fov", &settings->silent_fov, 0, 20, "Silent Fov: %.1f");
		}

		ImGui::SliderFloat("##aimbot.smooth", &settings->smooth, 1, 20, "Smooth: %.1f");

		if (!settings->silent) {
			ImGui::SliderInt("##aimbot.shot_delay", &settings->shot_delay, 0, 100, "Shot Delay: %.0f");
		}

		ImGui::SliderInt("##aimbot.kill_delay", &settings->kill_delay, 0, 1000, "Kill Delay: %.0f");

		/*if (settings->autowall) {
			ImGui::SliderInt("##aimbot.min_damage", &settings->min_damage, 1, 100, "Min Damage: %.0f");
		}*/

		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("##aimbot.other", ImVec2(0, 0), true);
	{
		ImGui::Text("Other");
		ImGui::Separator();
		ImGui::PushItemWidth(-1);
		ImGui::Text("RCS");
		ImGui::Checkbox("Toggle##aimbot.rcs", &settings->rcs);
		ImGui::Text("RCS Type:");
		static char* rcs_types[] = {
			"Standalone",
			"Aimbot"
		};
		ImGui::Combo("##aimbot.rcs_type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));
		ImGui::SliderInt("##aimbot.rcs_x", &settings->rcs_x, 0, 100, "RCS X: %.0f");
		ImGui::SliderInt("##aimbot.rcs_y", &settings->rcs_y, 0, 100, "RCS Y: %.0f");
		ImGui::SliderInt("##aimbot.rcs_start", &settings->rcs_start, 0, 30, "RCS Start: %.0f");
		ImGui::PopItemWidth();

		ImGui::Separator();
		ImGui::Text("Triggerbot");
		ImGui::Checkbox("Toggle##444", &g_Options.trigger_enable);
		ImGui::Hotkey("##trigger_key", &g_Options.trigger_key);
		ImGui::Checkbox("Ignore Smoke", &g_Options.trigger_ignoresmoke);
		ImGui::Checkbox("Ignore Flash", &g_Options.trigger_ignoreflash);
		ImGui::Checkbox("Killshot", &g_Options.trigger_killshot);
		ImGui::SliderInt("##trigger.delay.time", &g_Options.trigger_delay_time, 1, 250, "Delay: %.f ms");
		ImGui::SliderInt("##trigger.mindmg", &g_Options.trigger_minDamage, 1, 100, "Mindmg: %.f");
		ImGui::SliderFloat("##trigger.bursttime", &g_Options.trigger_burst_time, 0.0f, 0.5f, "Burst: %.3f s");
		ImGui::Combo("Hitbox", &g_Options.trigger_hitgroup, "Nearest\0Head\0Chest\0Stomach\0Left arm\0Right arm\0Left leg\0Right leg\0");
	}
	ImGui::EndChild();
	ImGui::Columns(1, NULL, false);
}
#pragma endregion

#pragma region Visuals
void RenderVisualsTab() {
	static int ESP_Tab = 0;

	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	{
		render_tabs(Esp_tabs, ESP_Tab, group_w / _countof(Esp_tabs), 25.0f);
	}
	ImGui::PopStyleVar();
	ImGui::BeginGroupBox("##body_content");
	{
		if (ESP_Tab == 0)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::BeginChild("Player ESP##visuals.playeresp", ImVec2(273, 305), true);
				{
					ImGui::Text("Player ESP");
					ImGui::Separator();
					ImGui::Checkbox("Toggle", &g_Options.esp_enabled);
					ImGui::Checkbox("Visible Check", &g_Options.esp_visiblecheck);
					ImGui::SameLine();
					ImGui::ColorEdit4("##visible_esp_color", g_Options.color_esp_enemy_visible, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Smoke Check", &g_Options.esp_smokecheck);
					ImGui::Checkbox("Box", &g_Options.esp_player_boxes);
					ImGui::SameLine();
					ImGui::ColorEdit4("Occluded", g_Options.color_esp_enemy_occluded, ImGuiColorEditFlags_NoInputs);
					if (g_Options.esp_player_boxes)
					{
						ImGui::Combo("##box_type", &g_Options.esp_player_boxes_type, Box_type, IM_ARRAYSIZE(Box_type), 2);
					}
					ImGui::Checkbox("Name", &g_Options.esp_player_names);
					ImGui::Checkbox("Health", &g_Options.esp_player_health);
					//	ImGui::Checkbox("Armor", &g_Options.esp_player_armour);
					ImGui::Checkbox("Skeleton Backtrack", &g_Options.esp_backtrackskeleton);
					ImGui::SameLine();
					ImGui::ColorEdit4("##skeleton_backtrack_color", g_Options.color_skeleton_backtrack, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Skeleton", &g_Options.esp_player_skeleton);
					ImGui::SameLine();
					ImGui::ColorEdit4("##skeleton_color", g_Options.color_skeleton, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
					ImGui::Checkbox("Flags", &g_Options.esp_flags);
					if (g_Options.esp_flags)
					{
						ImGui::PushItemWidth(100);
						if (ImGui::BeginCombo("", "..."))
						{
							ImGui::Selectable(("Armor"), &g_Options.esp_flags_armor, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Defusing"), &g_Options.esp_flags_defusing, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Defuse Kit"), &g_Options.esp_flags_kit, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Grenade Kill"), &g_Options.esp_grenadekillalert, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Hostage Carrier"), &g_Options.esp_flags_hostage, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Scoped"), &g_Options.esp_flags_scoped, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Money"), &g_Options.esp_flags_money, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Bomb"), &g_Options.esp_flags_c4, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);

							ImGui::EndCombo();
						}
						ImGui::PopItemWidth();
					}
				}
				ImGui::EndChild();

				ImGui::BeginChild("Glow##visuals.glow", ImVec2(273, 264), true);
				{
					ImGui::Text("Glow");
					ImGui::Separator();
					ImGui::Checkbox("Toggle", &g_Options.glow_enabled);
					ImGui::Checkbox("Player", &g_Options.glow_players);
					ImGui::SameLine();
					ImGui::ColorEdit4("##enemy_glow_color", g_Options.color_glow_enemy, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("C4 Carrier", &g_Options.glow_c4_carrier);
					ImGui::SameLine();
					ImGui::ColorEdit4("##c4_carrier_glow_color", g_Options.color_glow_c4_carrier, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Weapons", &g_Options.glow_weapons);
					ImGui::SameLine();
					ImGui::ColorEdit4("##weapons_glow_color", g_Options.color_glow_weapons, ImGuiColorEditFlags_NoInputs);
				}
				ImGui::EndChild();

				ImGui::NextColumn();

				ImGui::BeginChild("Other ESP##visuals.otheresp", ImVec2(270, 305), true);
				{
					ImGui::Text("Other ESP");
					ImGui::Separator();
					ImGui::Checkbox("Dropped Weapons", &g_Options.esp_dropped_weapons);
					ImGui::SameLine();
					ImGui::ColorEdit4("##dropped_weapons_color", g_Options.color_esp_weapons, ImGuiColorEditFlags_NoInputs);
					if (g_Options.esp_dropped_weapons)
					{
						ImGui::PushItemWidth(100);
						ImGui::Combo("##dropped_weapon_type", &g_Options.esp_dropped_weapons_type, Dropped_type, IM_ARRAYSIZE(Dropped_type), 2);
						ImGui::PopItemWidth();
					}
					ImGui::Checkbox("Projectiles", &g_Options.esp_throwable);
					ImGui::Checkbox("Bomb Timer", &g_Options.esp_bombtimer);
					ImGui::Checkbox("Bomb Logs", &g_Options.esp_bomblogs);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Displays Bomb Logs in the Chat");
					ImGui::Checkbox("Tahoma ESP", &g_Options.use_tahomafont);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Changes the ESP font to Tahoma");
				}
				ImGui::EndChild();

				ImGui::BeginChild("Chams##visuals.models", ImVec2(270, 264), true);
				{
					ImGui::Text("Chams");
					ImGui::Separator();
					ImGui::Checkbox("Toggle", &g_Options.chams_player_enabled);
					ImGui::SameLine();
					ImGui::ColorEdit4("##visible_chams_color", g_Options.color_chams_player_enemy_visible, ImGuiColorEditFlags_NoInputs);
					ImGui::Checkbox("Occluded", &g_Options.chams_player_ignorez);
					ImGui::SameLine();
					ImGui::ColorEdit4("##occluded_chams_color", g_Options.color_chams_player_enemy_occluded, ImGuiColorEditFlags_NoInputs);
					ImGui::PushItemWidth(100);
					if (ImGui::BeginCombo("", "Materials"))
					{
						ImGui::Selectable(("Flat"), &g_Options.chams_player_flat, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Wireframe"), &g_Options.chams_player_wireframe, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Glass"), &g_Options.chams_player_glass, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Disable Occulusion"), &g_Options.chams_disable_occulusion, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();
					ImGui::Checkbox("History", &g_Options.toggle_chams_history);
					ImGui::SameLine();
					ImGui::ColorEdit4("##history_chams_color", g_Options.color_chams_backtrack, ImGuiColorEditFlags_NoInputs);
					if (g_Options.toggle_chams_history)
					{
						ImGui::Combo("##backtrack_chams_style", &g_Options.chams_backtrack, std::vector<std::string>{ "Last tick", "All ticks" });
						ImGui::Combo("##backtrack_chams_type", &g_Options.chams_backtrack_type, std::vector<std::string>{ "Normal", "Flat" });
					}
				}
				ImGui::EndChild();
				ImGui::Columns(1, nullptr, false);
			}
			ImGui::PopStyleVar();
		}
		else if (ESP_Tab == 1) 
{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
			{
				ImGui::Columns(2, nullptr, false);
				ImGui::BeginChild("Other Visuals##othervisuals.other", ImVec2(273, 573), true);
				{
					ImGui::Text("Other Visuals");
					ImGui::Separator();
					ImGui::Checkbox("Aimbot FOV", &g_Options.other_drawfov);
					ImGui::Checkbox("Engine Radar", &g_Options.misc_engineradar);
					ImGui::Checkbox("Spectators", &g_Options.misc_spectatorlist);
					ImGui::Checkbox("Sonar ESP", &g_Options.misc_sonaresp);
					ImGui::Checkbox("History Dots", &g_Options.esp_backtrack);
					ImGui::Checkbox("Motion Blur", &g_Options.esp_motionblur);
					if (g_Options.esp_motionblur)
					{
						ImGui::SliderFloat("##motionblur_strength", &g_Options.esp_motionblur_strength, 1.f, 10.f, "%.f");
					}
					ImGui::Checkbox("Sniper Crosshair", &g_Options.misc_snipercrosshair);
					if (g_Options.misc_snipercrosshair)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##snipercrosshaircolor", g_Options.color_rcs_crosshair, ImGuiColorEditFlags_NoInputs);
					}
					ImGui::Checkbox("Recoil Crosshair", &g_Options.esp_rcscrosshair);
					if (g_Options.esp_rcscrosshair)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##rcscrosshaircolor", g_Options.color_rcs_crosshair, ImGuiColorEditFlags_NoInputs);
					}
					ImGui::Checkbox("Grenade Trajectory", &g_Options.esp_grenade_prediction);
					if (g_Options.esp_grenade_prediction)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##grenadepredcolor", g_Options.color_grenade_prediction, ImGuiColorEditFlags_NoInputs);
					}
					ImGui::Checkbox("Damage Indicator", &g_Options.esp_damageindicator);
					if (g_Options.esp_damageindicator)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##damage_indicator_color", g_Options.color_damage_indicator, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Show Remain HP", &g_Options.esp_damageindremainhp);
					}
					ImGui::Checkbox("Hitmarker", &g_Options.esp_hitmarker);
					if (g_Options.esp_hitmarker)
					{
						ImGui::PushItemWidth(100);
						ImGui::Combo("Hitsound", &g_Options.esp_hitsound, HitmarkSound, ARRAYSIZE(HitmarkSound));
						ImGui::Combo("Killsound", &g_Options.esp_killsound, HitmarkSound, ARRAYSIZE(HitmarkSound));
						ImGui::PopItemWidth();
					}
					ImGui::Checkbox("Grenade Circle", &g_Options.misc_grenade_circle);
					if (g_Options.misc_grenade_circle)
					{
						ImGui::ColorEdit4("Decoy##grenade_circle_decoy_color", g_Options.color_beam_decoy, ImGuiColorEditFlags_NoInputs);
						ImGui::ColorEdit4("Incendiary##grenade_circle_molotov_color", g_Options.color_beam_molotov, ImGuiColorEditFlags_NoInputs);
						ImGui::ColorEdit4("Smoke##grenade_circle_smoke_color", g_Options.color_beam_smoke, ImGuiColorEditFlags_NoInputs);
					}
				}
				ImGui::EndChild();

				ImGui::NextColumn();

				ImGui::BeginChild("Movement Visuals##othervisuals.movement", ImVec2(270, 305), true);
				{
					ImGui::Text("Movement Visuals");
					ImGui::Separator();
					ImGui::Checkbox("Keystrokes", &g_Options.misc_keystrokes);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Not fully Stream Proof");
					ImGui::Checkbox("Nulling Indicator", &g_Options.misc_nullindicator);
					ImGui::Checkbox("Foot Trail", &g_Options.enable_foottrail);
					if (g_Options.enable_foottrail)
					{
						ImGui::SameLine();
						ImGui::Checkbox("Solid Color", &g_Options.foot_trailcolor);
						ImGui::SameLine();
						ImGui::ColorEdit4("##foottrail_color", g_Options.color_foot_trail, ImGuiColorEditFlags_NoInputs);
					}
					ImGui::Checkbox("Draw Velocity", &g_Options.misc_velocitydraw);
					if (g_Options.misc_velocitydraw)
					{
						ImGui::SameLine();
						ImGui::Checkbox("Rainbow", &g_Options.rainbow_velocitydraw);
					}
					ImGui::Checkbox("Velocity Graph", &g_Options.misc_velocitygraph);

					ImGui::PushItemWidth(100);
					ImGui::Checkbox("Edgebug Detector", &g_Options.misc_ebugdetector);
					if (g_Options.misc_ebugdetector)
					{
						if (ImGui::BeginCombo("", "..."))
						{
							ImGui::Selectable(("Beam"), &g_Options.esp_detectionbeam, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Health Boost"), &g_Options.esp_detectionhealthboost, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
							ImGui::Selectable(("Sparks"), &g_Options.esp_detectionsparks, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);

							ImGui::EndCombo();
						}
					}
					ImGui::PopItemWidth();

				}
				ImGui::EndChild();

				ImGui::BeginChild("World##othervisuals.world", ImVec2(270, 264), true);
				{
					ImGui::Text("World");
					ImGui::Separator();
					ImGui::Checkbox("Nightmode", &g_Options.other_nightmode);
					if (g_Options.other_nightmode)
						ImGui::SliderFloat("Amount", &g_Options.other_nightmode_size, 0.05f, 1.0f, "%.2f");

					ImGui::Checkbox("Skybox Changer", &g_Options.enable_skybox_changer);
					if (g_Options.enable_skybox_changer)
					{
						ImGui::Combo("##skybox_combo", &g_Options.skybox_combo, SkyboxSelector, IM_ARRAYSIZE(SkyboxSelector));
					}

					ImGui::Checkbox("World Color", &g_Options.enable_world_color_modulation);
					if (g_Options.enable_world_color_modulation)
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("##world_color_modul", g_Options.color_world_modulation, ImGuiColorEditFlags_NoInputs);
						ImGui::Checkbox("Rainbow", &g_Options.other_mat_ambient_light_rainbow);
					}

					ImGui::PushItemWidth(100);
					if (ImGui::BeginCombo("", "Removals"))
					{
						ImGui::Selectable(("Blood"), &g_Options.no_blood, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Flashbang"), &g_Options.other_no_flash, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Fog"), &g_Options.other_no_fog, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Panorama Blur"), &g_Options.no_panoramablur, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Post-Processing"), &g_Options.no_postprocess, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Shadows"), &g_Options.other_no_shadows, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Sleeves"), &g_Options.other_no_hands, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Smoke"), &g_Options.other_no_smoke, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("Single Thread Bones"), &g_Options.other_single_thread_bones, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
						ImGui::Selectable(("3D Sky"), &g_Options.other_no_3dsky, ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);

						ImGui::EndCombo();
					}
					ImGui::PopItemWidth();
				}
				ImGui::EndChild();

				ImGui::Columns(1, nullptr, false);
			}
			ImGui::PopStyleVar();
		}
	}
	ImGui::EndGroupBox();
}
#pragma endregion

static std::string GetLocalName()
{
	player_info_t LocalPlayer;
	g_EngineClient->GetPlayerInfo(g_EngineClient->GetLocalPlayer(), &LocalPlayer);
	return std::string(LocalPlayer.szName);
}

bool canreset = false;

#pragma region Misc
void RenderMiscTab() {
	auto& style = ImGui::GetStyle();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
	{
		ImGui::Columns(2, nullptr, false);
		ImGui::BeginChild("General##misc.general", ImVec2(281, 305), true);
		{
			ImGui::Checkbox("Anti Untrusted", &g_Options.anti_untrusted);
			ImGui::Checkbox("Auto Strafe", &g_Options.misc_autostrafe);
			/*if (g_Options.misc_autostrafe)
			{
				ImGui::SliderFloat("##retrack_speed", &g_Options.retrack_speed, 1.f, 15.f, "Retrack: %.f");
			}*/
			ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
			ImGui::Checkbox("Fast Stop", &g_Options.misc_faststop);
			ImGui::Checkbox("Knifebot", &g_Options.misc_knifebot);
			//ImGui::Checkbox("Reveal Money", &g_Options.reveal_money);
			ImGui::Checkbox("Slide Walk", &g_Options.misc_slide_walk);
			ImGui::Checkbox("Vote Revealer", &g_Options.misc_voterevealer);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Displays the Vote Info in the Chat");
			//ImGui::Checkbox("Unlock Inventory", &g_Options.unlock_inventory);
			ImGui::Checkbox("Watermark", &g_Options.misc_watermark);
		}
		ImGui::EndChild();

		ImGui::BeginChild("Movement##misc.movement", ImVec2(281, 312), true);
		{
			ImGui::PushItemWidth(100);

			ImGui::Checkbox("Show Indicators", &g_Options.movement_indicators);

			ImGui::Checkbox("Jump Bug", &g_Options.misc_jumpbug);
			if (g_Options.misc_jumpbug)
			{
				ImGui::Hotkey("##jump bug key", &g_Options.misc_jumpbug_bind);
			}

			ImGui::Checkbox("Edge Bug", &g_Options.misc_ebug);
			if (g_Options.misc_ebug)
			{
				ImGui::SameLine();
				{
					ImGui::Checkbox("Auto Align", &g_Options.misc_autoalign);
				}
				ImGui::SliderInt("Assist Ticks", &g_Options.misc_ebugalignticks, 1, 64, "%d");
				ImGui::SliderFloat("Assist Lock Factor", &g_Options.misc_ebuglockfactor, 0.1f, 1.f, "%.1f");
				ImGui::Hotkey("##edge bug key", &g_Options.misc_ebug_key);
			}

			ImGui::Checkbox("Edge Jump", &g_Options.misc_edgejump);
			if (g_Options.misc_edgejump)
			{
				ImGui::Hotkey("##edge jump key", &g_Options.misc_edgejump_bind);
			}

			ImGui::Text("Movement Recorder");
			ImGui::Combo("##movement_recorder_type", &g_Options.movement_recorder_type, MoveRecType, ARRAYSIZE(MoveRecType));

			if (g_Options.movement_recorder_type == 0)
			{
				ImGui::Checkbox("Enable", &g_Options.enable_movement_recorder);
				if (g_Options.enable_movement_recorder)
				{
					ImGui::Text("Record");
					ImGui::Hotkey("##record_movement_key", &g_Options.record_movement_key);
					ImGui::Text("Play");
					ImGui::Hotkey("##play_movement_key", &g_Options.play_movement_key);
					ImGui::Checkbox("Silent", &g_Options.silent_movementrecord);

				}
			}

			if (g_Options.movement_recorder_type == 1)
			{
				if (r.recording_active()) {
					if (ImGui::Button("Stop Recording"))
						r.stop_recording();
				}
				else if (r.rerecording_active()) {
					if (ImGui::Button("Save Re-recording"))
						r.stop_rerecording(true);

					if (ImGui::Button("Clear Re-recording"))
						r.stop_rerecording(false);
				}
				else {
					if (ImGui::Button("Start Recording"))
						r.start_recording();

					if (!r.get_active_recording().empty()) {
						if (ImGui::Button("Clear Run Frames"))
							r.get_active_recording().clear();

						if (p.playback_active() && ImGui::Button("Stop Playback"))
							p.stop_playback();

						if (!p.playback_active() && ImGui::Button("Start Playback"))
							p.start_playback(r.get_active_recording());

						if (!p.playback_active())
						{
							ImGui::Checkbox("Silent", &g_Options.silent_movementrecord);
						}
					}
				}

			}
		}
		ImGui::EndChild();

		ImGui::NextColumn();
		ImGui::PopItemWidth();
		
		ImGui::BeginChild("Other##misc.other", ImVec2(277, 305), true);
		{
			ImGui::Checkbox("Auto Accept", &g_Options.misc_autoaccept);
			ImGui::Checkbox("Bypass sv_pure", &g_Options.misc_svpure);
			ImGui::Checkbox("Clantag", &g_Options.misc_clantag);
			ImGui::Checkbox("Discord RPC", &g_Options.discord_rpc);
			if (g_Options.discord_rpc)
			{
				DCRPC.RPCStart();
				DCRPC.UpdateRPC();
				canreset = true;
			}
			else if (canreset == true)
			{
				Discord_ClearPresence();
				Discord_Shutdown();
				bool canreset = false;
			}
			ImGui::Checkbox("Faceit Damage Print", &g_Options.misc_faceitfakechat);
			//ImGui::Checkbox("Overwatch Revealer", &g_Options.misc_owrevealer);
			ImGui::Checkbox("Rank Reveal", &g_Options.misc_showranks);
			ImGui::Checkbox("Aspect Ratio", &g_Options.misc_aspectratiotoggle);
			if (g_Options.misc_aspectratiotoggle)
			{
				ImGui::SliderFloat("##aspectratio.amount", &g_Options.misc_aspectratio, 0.f, 10.f, "%.1f");
			}
			ImGui::Checkbox("Enable FOV Changer", &g_Options.misc_changefov);
			if (g_Options.misc_changefov)
			{
				ImGui::PushItemWidth(100);
				{
					ImGui::SliderInt("Override FOV", &g_Options.misc_override_fov, 90, 150);
					ImGui::SliderInt("Viewmodel FOV", &g_Options.misc_viewmodel_fov, 68, 120);
				}
			}

			if (ImGui::Button("Unload"))
				g_Unload = true;
		}
		ImGui::EndChild();

		ImGui::BeginChild("Exploits##misc.exploits", ImVec2(277, 312), true);
		{
			ImGui::Checkbox("Third Person", &g_Options.misc_thirdperson);
			if (g_Options.misc_thirdperson)
			{
				ImGui::SliderFloat("Distance", &g_Options.misc_thirdperson_dist, 25.f, 250.f, "%.0f");
				ImGui::Hotkey("##third person key", &g_Options.misc_thirdperson_bind);
			}

			std::string fakeban = " \x01\x0B\x07 " + GetLocalName() + " foi banido(a) permanentemente dos servidores oficiais do CS:GO. \x01 ";

			if (ImGui::Button("Clear Name"))
				Utils::SetName("\n\xAD\xAD\xAD");

			if (ImGui::Button("Fake Ban"))
				Utils::SetName(fakeban.c_str());

			static char choice_name[128];
			ImGui::PushItemWidth(100);
			ImGui::InputText("", choice_name, IM_ARRAYSIZE(choice_name), ImGuiInputTextFlags_EnterReturnsTrue);
			if (ImGui::Button("Set Name"))
				Utils::SetName(choice_name);
		}
		ImGui::EndChild();
		ImGui::Columns(1, nullptr, false);
	}
	ImGui::PopStyleVar();
}
#pragma endregion
#pragma region Skins
void RenderSkinsTab() {
	if (k_skins.size() == 0) {
		initialize_kits();
	}
	auto& entries = g_Options.skins.m_items;
	static auto definition_vector_index = 0;
	ImGui::Columns(2, nullptr, false);
	{
		ImGui::BeginChild("##skins.list", ImVec2(0, 0), true);
		{
			ImGui::PushItemWidth(-1);
			const auto wnd = ImGui::GetCurrentWindowRead();
			float height = (wnd->Pos.y + wnd->Size.y) - wnd->DC.CursorPos.y - 18.0f - ImGui::GetStyle().WindowPadding.y - ImGui::GetStyle().FramePadding.y * 2.0f;

			ImGui::ListBoxHeader("##config", ImVec2(0.f, height));
			{
				for (size_t w = 0; w < k_weapon_names.size(); w++) {
					if (ImGui::Selectable(k_weapon_names[w].name, definition_vector_index == w)) {
						definition_vector_index = w;
					}
				}
			}
			ImGui::ListBoxFooter();

			if (ImGui::Button("Update"))
				g_ClientState->ForceFullUpdate();

			ImGui::PopItemWidth();
		}
		ImGui::EndChild();
	}
	ImGui::NextColumn();
	{
		ImGui::BeginChild("##skins.options", ImVec2(277, 252), true);
		{
			auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
			selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
			selected_entry.definition_vector_index = definition_vector_index;

			static std::string weapon = "";
			static std::string kit = "";
			static auto paintKit = 0;

			ImGui::Text("Skin Changer");
			ImGui::Separator();

			ImGui::Checkbox("Toggle", &selected_entry.enabled);
			ImGui::InputInt("Seed", &selected_entry.seed);
			ImGui::InputInt("StatTrak", &selected_entry.stat_trak);
			ImGui::SliderFloat("Wear", &selected_entry.wear, FLT_MIN, 1.f, "%.4f", 5);
			ImGui::InputText("NameTag##skin_tag", selected_entry.custom_name, 32);

			if (selected_entry.definition_index != GLOVE_T_SIDE) {
				ImGui::Combo("Paint Kit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text) {
					*out_text = k_skins[idx].name.c_str();
					return true;
					}, nullptr, k_skins.size(), 20);
				selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
			}
			else {
				ImGui::Combo("Paint Kit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text) {
					*out_text = k_gloves[idx].name.c_str();
					return true;
					}, nullptr, k_gloves.size(), 20);
				selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
			}
			if (selected_entry.definition_index == WEAPON_KNIFE)
			{
				selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;
				selected_entry.definition_override_vector_index = g_Options.skins_knifemodel;
				ImGui::Combo("Knife", &g_Options.skins_knifemodel, [](void* data, int idx, const char** out_text)
					{
						*out_text = k_knife_names.at(idx).name;
						return true;
					}, nullptr, k_knife_names.size(), 20);
			}
			else if (selected_entry.definition_index == GLOVE_T_SIDE) {
				ImGui::Combo("Glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
					*out_text = k_glove_names.at(idx).name;
					return true;
					}, nullptr, k_glove_names.size(), 10);
				selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
			}
			else {
				static auto unused_value = 0;
				selected_entry.definition_override_vector_index = 0;

				const char* Slot[] =
				{
					"Slot 1",
					"Slot 2",
					"Slot 3",
					"Slot 4",
					"Slot 5 - R8 and G3SG1 only"
				};
				ImGui::Checkbox("Sticker Changer", &selected_entry.enabled_stickers);

				if (selected_entry.definition_index == WEAPON_REVOLVER || selected_entry.definition_index == WEAPON_G3SG1)
					ImGui::Combo("Slot", &selected_entry.stickers_place, Slot, IM_ARRAYSIZE(Slot));
				else
					ImGui::Combo("Slot", &selected_entry.stickers_place, Slot, IM_ARRAYSIZE(Slot) - 1);

				ImGui::Combo("Sticker", &selected_entry.stickers[selected_entry.stickers_place].sticker_vector_index, [](void* data, int idx, const char** out_text) {
					*out_text = k_stickers[idx].name.c_str();
					return true;
					}, nullptr, k_stickers.size(), 20);
				selected_entry.stickers[selected_entry.stickers_place].stickers_id = k_stickers[selected_entry.stickers[selected_entry.stickers_place].sticker_vector_index].id;
			}
		}
		ImGui::EndChild();

		ImGui::BeginChild("##skins.agent", ImVec2(277, 110), true);
		{
			static char* AgentModelsTR[] =
			{
				"Default",
				"Getaway Sally", //The Professionals
				"Number K", //The Professionals
				"Little Kev", //The Professionals
				"Safecracker Voltzmann", //The Professionals
				"Bloody Darryl The Strapped", //The Professionals
				"Sir Bloody Loudmouth Darryl", //The Professionals
				"Sir Bloody Darryl Royale", //The Professionals
				"Sir Bloody Skullhead Darryl", //The Professionals
				"Sir Bloody Silent Darryl", //The Professionals
				"Sir Bloody Miami Darryl", //The Professionals
				"Street Soldier", //Phoenix
				"Soldier", //Phoenix
				"Slingshot", //Phoenix
				"Enforcer", //Phoenix
				"Mr. Muhlik", //Elite Crew
				"Prof. Shahmat", //Elite Crew
				"Osiris", //Elite Crew
				"Ground Rebel", //Elite Crew
				"The Elite Mr. Muhlik", //Elite Crew
				"Trapper", //Guerrilla Warfare
				"Trapper Aggressor", //Guerrilla Warfare
				"Vypa Sista of the Revolution", //Guerrilla Warfare
				"Col. Mangos Dabisi", //Guerrilla Warfare
				//"Arno The Overgrown", //Guerrilla Warfare
				"'Medium Rare' Crasswater", //Guerrilla Warfare
				"Crasswater The Forgotten", //Guerrilla Warfare
				"Elite Trapper Solman", //Guerrilla Warfare
				"'The Doctor' Romanov", //Sabre
				"Blackwolf", //Sabre
				"Maximus", //Sabre
				"Dragomir", //Sabre
				"Rezan The Ready", //Sabre
				"Rezan The Redshirt", //Sabre
				"Dragomir The Footsoldier" //Sabre Footsoldier
			};

			static char* AgentModelsCT[] =
			{
				"Default",
				"Cmdr. Davida 'Goggles' Fernandez", //SEAL Frogman
				"Cmdr. Frank 'Wet Sox' Baroud", //SEAL Frogman
				"Lieutenant Rex Krikey", //SEAL Frogman
				"Michael Syfers", //FBI Sniper
				"Operator", //FBI SWAT
				"Special Agent Ava", //FBI
				"Markus Delrow", //FBI HRT
				"Sous-Lieutenant Medic", //Gendarmerie Nationale
				"Chem-Haz Capitaine", //Gendarmerie Nationale
				"Chef d'Escadron Rouchard", //Gendarmerie Nationale
				"Aspirant", //Gendarmerie Nationale
				"Officer Jacques Beltram", //Gendarmerie Nationale
				"D Squadron Officer", //NZSAS
				"B Squadron Officer", //SAS
				"Seal Team 6 Soldier", //NSWC SEAL
				"Buckshot", //NSWC SEAL
				"Lt. Commander Ricksaw", //NSWC SEAL
				"'Blueberries' Buckshot", //NSWC SEAL
				"3rd Commando Company", //KSK
				"'Two Times' McCoy - Cavalry", //TACP Cavalry
				"'Two Times' McCoy - USAF", //USAF TACP
				"Primeiro Tenente", //Brazilian 1st Battalion
				"Cmdr. Mae 'Dead Cold' Jamison", //SWAT
				"1st Lieutenant Farlow", //SWAT
				"John 'Van Healen' Kask", //SWAT
				"Bio-Haz Specialist", //SWAT
				"Sergeant Bombson", //SWAT
				"Chem-Haz Specialist", //SWAT
				"Lieutenant 'Tree Hugger' Farlow" //SWAT

			};

			ImGui::Text("Agent Changer");
			ImGui::Separator();

			ImGui::Checkbox("Toggle", &g_Options.enable_agentskins);
			if (g_Options.enable_agentskins)
			{
				ImGui::Combo("CT", &g_Options.skins_agentmodelct, AgentModelsCT, ARRAYSIZE(AgentModelsCT));
				ImGui::Combo("TR", &g_Options.skins_agentmodeltr, AgentModelsTR, ARRAYSIZE(AgentModelsTR));
			}
		}
		ImGui::EndChild();
		/*ImGui::BeginChild("##profile.changer", ImVec2(277, 250), true);
		{
			ImGui::Text("Profile Changer");
			ImGui::Separator();

			const char* ranklist[] =
			{
				"None",
				"Silver I",
				"Silver II",
				"Silver III",
				"Silver IV",
				"Silver Elite",
				"Silver Elite Master",

				"Gold Nova I",
				"Gold Nova II",
				"Gold Nova III",
				"Gold Nova Master",

				"Master Guardian I",
				"Master Guardian II",
				"Master Guardian Elite",

				"Distinguished Master Guardian",
				"Legendary Eagle",
				"Legendary Eagle Master",
				"Supreme Master First Class",
				"Global Elite"

			};

			ImGui::Checkbox("Toggle", &g_Options.enable_profilechanger);

			ImGui::Combo("Rank", &g_Options.profile_rankid, ranklist, IM_ARRAYSIZE(ranklist));
			ImGui::InputInt("Wins", &g_Options.profile_wins);
			ImGui::SliderInt("Level", &g_Options.profile_level, -1, 40);
			ImGui::SliderInt("XP", &g_Options.profile_exp, -1, 5000);
			ImGui::InputInt("Teaching", &g_Options.profile_teaching);
			ImGui::InputInt("Friendly", &g_Options.profile_friendly);
			ImGui::InputInt("Leader", &g_Options.profile_leader);

			if (ImGui::Button("Apply"))
			{
				Protobuf::SendClientHello();
				Protobuf::SendMatchmakingClient2GCHello();
			}
		}*/

	}
	ImGui::Columns(1, nullptr, false);
}
#pragma endregion

#pragma region Options
void RenderOptionsTab()
{
	static std::vector<std::string> configs;

	static auto load_configs = []() {
		std::vector<std::string> items = {};

		std::string path("C:\\Nirvana");
		if (!fs::is_directory(path))
			fs::create_directories(path);

		for (auto& p : fs::directory_iterator(path))
		{
			if (!strstr(p.path().string().c_str(), "other"))
				items.push_back(p.path().string().substr(path.length() + 1));
		}

		return items;
	};

	static auto is_configs_loaded = false;
	if (!is_configs_loaded) {
		is_configs_loaded = true;
		configs = load_configs();
	}

	static std::string current_config;

	static char config_name[32];

	ImGui::Columns(1, nullptr, false);
	{
		ImGui::BeginChild("##config.list", ImVec2(0, 0), true);
		{
			ImGui::InputText("##config_name", config_name, sizeof(config_name));

			ImGui::ListBoxHeader("##configs");
			{
				for (auto& config : configs) {
					if (ImGui::Selectable(config.c_str(), config == current_config)) {
						current_config = config;
					}
				}
			}
			ImGui::ListBoxFooter();

			if (ImGui::Button("Create")) {
				current_config = std::string(config_name);

				Config->save(current_config + ".json");
				is_configs_loaded = false;
				memset(config_name, 0, 32);
			}

			ImGui::SameLine();

			if (!current_config.empty()) {

				if (ImGui::Button("Load"))
				{
					Config->load(current_config);
				}

				ImGui::SameLine();

				if (ImGui::Button("Save"))
				{
					Config->save(current_config);
				}

				ImGui::SameLine();

				if (ImGui::Button("Delete") && fs::remove("C:\\Nirvana\\" + current_config))
				{
					current_config.clear();
					is_configs_loaded = false;
				}

				ImGui::SameLine();
			}

			if (ImGui::Button("Refresh"))
				is_configs_loaded = false;
		}
		ImGui::EndChild();
	}
}
#pragma endregion
#pragma region Radar
void Menu::DrawRadar() {
	auto visible = Color::Red;
	auto Tvisible = Color::White;

	ImGuiStyle& style = ImGui::GetStyle();
	ImVec2 oldPadding = style.WindowPadding;
	float oldAlpha = style.Colors[ImGuiCol_WindowBg].w;
	style.WindowPadding = ImVec2(0, 0);

	//style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	//style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
	if (ImGui::Begin("Radar", nullptr, ImVec2(150, 170), 0.7F, ImGuiWindowFlags_NoCollapse))
		//if (ImGui::Begin("Radar", &_visible, ImVec2(200, 200)), ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)
	{
		ImVec2 siz = ImGui::GetWindowSize();
		ImVec2 pos = ImGui::GetWindowPos();

		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();
		windowDrawList->AddLine(ImVec2(pos.x + (siz.x / 2), pos.y + 0), ImVec2(pos.x + (siz.x / 2), pos.y + siz.y), ImColor(64, 72, 95), 1.5f);
		windowDrawList->AddLine(ImVec2(pos.x + 0, pos.y + (siz.y / 2)), ImVec2(pos.x + siz.x, pos.y + (siz.y / 2)), ImColor(64, 72, 95), 1.5f);
		static auto GetU32 = [](Color _color) {
			return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
				+ (_color[0] & 0xff);
		};

		static auto RotatePoint = [](Vector EntityPos, Vector LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom) {
			float r_1, r_2;
			float x_1, y_1;

			r_1 = -(EntityPos.y - LocalPlayerPos.y);
			r_2 = EntityPos.x - LocalPlayerPos.x;
			float Yaw = angle - 90.0f;

			float yawToRadian = DEG2RAD(Yaw);
			x_1 = r_2 * cosf(yawToRadian) - r_1 * sin(yawToRadian) / 20.0f;
			y_1 = r_2 * sinf(yawToRadian) + r_1 * cos(yawToRadian) / 20.0f;

			x_1 *= zoom;
			y_1 *= zoom;

			int sizX = sizeX / 2;
			int sizY = sizeY / 2;

			x_1 += sizX;
			y_1 += sizY;

			if (x_1 < 5)
				x_1 = 5;

			if (x_1 > sizeX - 5)
				x_1 = sizeX - 5;

			if (y_1 < 5)
				y_1 = 5;

			if (y_1 > sizeY - 5)
				y_1 = sizeY - 5;

			x_1 += posX;
			y_1 += posY;

			return Vector2D(x_1, y_1);
		};

		if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
			if (g_LocalPlayer) {
				Vector LocalPos = g_LocalPlayer->GetEyePos();
				QAngle ang;
				g_EngineClient->GetViewAngles(&ang);

				for (int i = 1; i <= g_GlobalVars->maxClients; ++i) {
					auto player = C_BasePlayer::GetPlayerByIndex(i);
					if (!player || player->IsDormant() || !player->IsAlive())
						continue;

					bool bIsEnemy = g_LocalPlayer->m_iTeamNum() != player->m_iTeamNum();

					if (g_Options.misc_radar_enemyonly && !bIsEnemy)
						continue;

					bool viewCheck = false;
					Vector2D EntityPos = RotatePoint(Vector(player->m_vecOrigin().x, player->m_vecOrigin().y, 0), LocalPos, pos.x, pos.y, siz.x, siz.y, ang.yaw, 0.5f);

					//ImU32 clr = (bIsEnemy ? (isVisibled ? Color::LightGreen() : Color::Blue()) : Color::White()).GetU32();
					ImU32 clr = GetU32(bIsEnemy ? (visible) : Tvisible);

					int s = 4;
					windowDrawList->AddCircleFilled(ImVec2(EntityPos.x, EntityPos.y), s, clr);
				}
			}
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
	style.WindowPadding = oldPadding;
	style.Colors[ImGuiCol_WindowBg].w = oldAlpha;
}
#pragma endregion

void Menu::Initialize() {
	CreateStyle();

	_visible = false;
}

void Menu::Shutdown() {
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost() {
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset() {
	ImGui_ImplDX9_CreateDeviceObjects();
}

bool FileExists(const TCHAR *fileName)
{
	DWORD fileAttr;
	fileAttr = GetFileAttributes(fileName);
	if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
		return false;
	return true;
}
void RenderEmptyTab()
{
	
		ImGui::Text("Is empty <3");
		//ImGui::Text("%.2f x, %.2f y", ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
	
}

void Menu::Render() {
	ImGui::GetIO().MouseDrawCursor = _visible;

	if (g_Options.misc_spectatorlist)
		Visuals::Get().RenderSpectatorList();

	if (g_Options.misc_radar)
		DrawRadar();

	if (!_visible)
		return;

	static int selected_tab = 0;

	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	float x = w * 0.5f - 300.0f;
	float y = h * 0.5f - 200.0f;

	ImGui::SetNextWindowPos(ImVec2{ x, y }, ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2{ 600, 700 }, ImGuiSetCond_Once);

#ifdef _DEBUG
	ImGui::ShowDemoWindow();
#endif

	if (ImGui::Begin("Nirvana", &_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar)) {

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		{
			ImGui::Image(brand_img, ImVec2(142.0f, 40.0f)); ImGui::SameLine();
			ImGui::Dummy(ImVec2(_style.WindowPadding.x, 0.0f)); ImGui::SameLine();
			auto TabsW = (ImGui::GetCurrentWindow()->Size.x - _style.WindowPadding.x * 2.0f - 150.0f) / _countof(tabs);
			render_tabs(tabs, selected_tab, TabsW, 40.0f);
		}
		ImGui::PopStyleVar();

		ImGui::BeginGroupBox("##empty");
		{
			if (selected_tab == 0)		RenderLegitTab();
		//	else if (selected_tab == 1)	RenderTriggerTab();
			else if (selected_tab == 1)	RenderVisualsTab();
			else if (selected_tab == 2)	RenderMiscTab();
			else if (selected_tab == 3)	RenderSkinsTab();
			else if (selected_tab == 4)	RenderOptionsTab();
		}
		ImGui::EndGroupBox();

		ImGui::End();
	}
}

bool firstopening = true;

void Menu::Toggle() 
{
	_visible = !_visible;
	if (_visible)
	{
		if (firstopening)
		{
			PlaySoundA(rezero_sound_wav, NULL, SND_ASYNC | SND_MEMORY);
			firstopening = false;
		}
	}
	else
	{
	}
}

void Menu::CreateStyle() {
	ImGui::StyleColorsM0ne0N(); // why nut? xd

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX); // this for print hex, without - rgba
	ImGui::GetStyle() = _style;
}






































































































































































































































































































































































































































































































































































































































































































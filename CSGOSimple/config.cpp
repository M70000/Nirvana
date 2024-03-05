#include "config.hpp"
#include "options.hpp"
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "menu.hpp"
#include "helpers/input.hpp"

CConfig* Config = new CConfig();

/*void CConfig::SetupValue( int& value, int def, std::string category, std::string name ) { value = def; ints.push_back( new ConfigValue< int >( category, name, &value, def ) ); }
void CConfig::SetupValue( char* value, char* def, std::string category, std::string name ) { value = def; chars.push_back( new ConfigValue< char >( category, name, value, *def ) ); }
void CConfig::SetupValue( float& value, float def, std::string category, std::string name ) { value = def; floats.push_back( new ConfigValue< float >( category, name, &value, def ) ); }
void CConfig::SetupValue( bool& value, bool def, std::string category, std::string name ) { value = def; bools.push_back( new ConfigValue< bool >( category, name, &value, def ) ); }

void CConfig::SetupSkinz(int value[5], std::string category, std::string name)
{
	SetupValue(value[0], value[0], category, name + "_1");
	SetupValue(value[1], value[1], category, name + "_2");
	SetupValue(value[2], value[2], category, name + "_3");
	SetupValue(value[3], value[3], category, name + "_4");
	SetupValue(value[4], value[4], category, name + "_5");
}
void CConfig::SetupColor( float value[4], std::string name ) {
	SetupValue( value[0], value[0], ( "colors" ), name + "_r" );
	SetupValue( value[1], value[1], ( "colors" ), name + "_g" );
	SetupValue( value[2], value[2], ( "colors" ), name + "_b" );
	SetupValue( value[3], value[3], ( "colors" ), name + "_a" );
}*/

void CConfig::save(std::string name)
{
	config::save(name, "", true, []()
		{
			Json::Value config;

			//LEGIT

			Json::Value aimbot_items;
			for (auto data : g_Options.legitbot_items)
			{
				Json::Value act;
				const auto aimbot_data = data.second;
				act["toggle"] = aimbot_data.enabled;
				act["deathmatch"] = aimbot_data.deathmatch;
				act["autopistol"] = aimbot_data.autopistol;
				act["autowall"] = aimbot_data.autowall;
				act["autofire"] = aimbot_data.autofire;
				act["autofire.key"] = aimbot_data.autofire_key;
				act["autostop"] = aimbot_data.autostop;
				act["backtrack.time"] = aimbot_data.backtrack_time;
				act["aim.type"] = aimbot_data.aim_type;
				act["aim.priority"] = aimbot_data.priority;
				act["aim.fov.amouth"] = aimbot_data.fov;
				act["aim.smooth.amouth"] = aimbot_data.smooth;
				act["aim.fov.type"] = aimbot_data.fov_type;
				act["aim.smooth.type"] = aimbot_data.smooth_type;
				act["aim.hitbox"] = aimbot_data.hitbox;
				act["aim.silent"] = aimbot_data.silent;
				act["aim.silent.fov"] = aimbot_data.silent;
				act["aim.delay.shot"] = aimbot_data.shot_delay;
				act["aim.delay.kill"] = aimbot_data.kill_delay;
				act["aim.mindamage"] = aimbot_data.min_damage;
				act["check.jump"] = aimbot_data.jump_check;
				act["check.smoke"] = aimbot_data.smoke_check;
				act["check.flash"] = aimbot_data.flash_check;
				act["check.zoom"] = aimbot_data.only_in_zoom;
				act["on_key"] = aimbot_data.on_key;
				act["key"] = aimbot_data.key;
				act["rcs"] = aimbot_data.rcs;
				act["rcs.x"] = aimbot_data.rcs_x;
				act["rcs.y"] = aimbot_data.rcs_y;
				act["rcs.type"] = aimbot_data.rcs_type;
				act["rcs.fov.toggle"] = aimbot_data.rcs_fov_enabled;
				act["rcs.smooth.toggle"] = aimbot_data.rcs_smooth_enabled;
				act["rcs.fov.amouth"] = aimbot_data.rcs_fov;
				act["rcs.smooth.amouth"] = aimbot_data.rcs_smooth;
				act["rcs.start"] = aimbot_data.rcs_start;

				aimbot_items[data.first] = act;
			}

			config["legitbot"] = aimbot_items;

			//TRIGGER

			config["trigger.toggle"] = g_Options.trigger_enable;
			config["trigger.keybind"] = g_Options.trigger_key;
			config["trigger.delay"] = g_Options.trigger_delay;
			config["trigger.delay.time"] = g_Options.trigger_delay_time;
			config["trigger.hitchance"] = g_Options.trigger_hitchance;
			config["trigger.ignore.smoke"] = g_Options.trigger_ignoresmoke;
			config["trigger.ignore.flashbang"] = g_Options.trigger_ignoreflash;
			config["trigger.check.scope"] = g_Options.trigger_scopecheck;

			//ESP

			config["esp.toggle"] = g_Options.esp_enabled;
			config["esp.visiblecheck"] = g_Options.esp_visiblecheck;
			config["esp.smokecheck"] = g_Options.esp_smokecheck;
			config["esp.enemiesonly"] = g_Options.esp_enemies_only;

			config["esp.indicator.damage"] = g_Options.esp_damageindicator;
			config["esp.indicator.damage.hp"] = g_Options.esp_damageindremainhp;
			config["esp.indicator.movement"] = g_Options.movement_indicators;
			config["esp.indicator.choke"] = g_Options.esp_choke_indicator;

			config["esp.player.skeleton.backtrack"] = g_Options.esp_backtrackskeleton;
			config["esp.player.skeleton"] = g_Options.esp_player_skeleton;
			config["esp.player.boxes"] = g_Options.esp_player_boxes;
			config["esp.player.boxes.type"] = g_Options.esp_player_boxes_type;
			config["esp.player.names"] = g_Options.esp_player_names;
			config["esp.player.health"] = g_Options.esp_player_health;
			config["esp.player.armour"] = g_Options.esp_player_armour;
			config["esp.player.weapons"] = g_Options.esp_player_weapons;
			config["esp.player.backtrack"] = g_Options.esp_backtrack;
			config["esp.player.sounds"] = g_Options.esp_sounds;
			config["esp.player.sounds.time"] = g_Options.esp_sounds_time;
			config["esp.player.sounds.radius"] = g_Options.esp_sounds_radius;
			config["esp.player.anglelines"] = g_Options.esp_angle_lines;

			config["esp.hitmarker"] = g_Options.esp_hitmarker;
			config["esp.detectionsparks"] = g_Options.esp_detectionsparks;
			config["esp.detectionhealthboost"] = g_Options.esp_detectionhealthboost;
			config["esp.detectionbeam"] = g_Options.esp_detectionbeam;
			config["esp.hitsound"] = g_Options.esp_hitsound;
			config["esp.killsound"] = g_Options.esp_killsound;

			config["esp.player.flags"] = g_Options.esp_flags;
			config["esp.player.flags.armor"] = g_Options.esp_flags_armor;
			config["esp.player.flags.kit"] = g_Options.esp_flags_kit;
			config["esp.player.flags.hostage"] = g_Options.esp_flags_hostage;
			config["esp.player.flags.scoped"] = g_Options.esp_flags_scoped;
			config["esp.player.flags.money"] = g_Options.esp_flags_money;
			config["esp.player.flags.c4"] = g_Options.esp_flags_c4;
			config["esp.player.flags.defusing"] = g_Options.esp_flags_defusing;
			config["esp.player.flags.grenadekill"] = g_Options.esp_grenadekillalert;
			config["esp.player.flags.nirvanauser"] = g_Options.esp_draw_nrvusers;

			config["esp.world.items"] = g_Options.esp_items;
			config["esp.world.grenade.throwable"] = g_Options.esp_throwable;
			config["esp.world.weapons.dropped"] = g_Options.esp_player_weapons;
			config["esp.world.defusekit"] = g_Options.esp_defuse_kit;
			config["esp.world.molotov.timer"] = g_Options.esp_molotovtimer;
			config["esp.world.bomb.planted"] = g_Options.esp_planted_c4;
			config["esp.world.bomb.timer"] = g_Options.esp_bombtimer;
			config["esp.world.bomb.logs"] = g_Options.esp_bomblogs;
			config["esp.world.grenade.prediction"] = g_Options.esp_grenade_prediction;
			config["esp.world.motionblur"] = g_Options.esp_motionblur;
			config["esp.world.motionblurstrength"] = g_Options.esp_motionblur_strength;

			//GLOW

			config["glow.toggle"] = g_Options.glow_enabled;
			config["glow.enemiesonly"] = g_Options.glow_enemies_only;
			config["glow.players"] = g_Options.glow_players;
			config["glow.chickens"] = g_Options.glow_chickens;
			config["glow.bomb.carrier"] = g_Options.glow_c4_carrier;
			config["glow.bomb.planted"] = g_Options.glow_planted_c4;
			config["glow.defusekit"] = g_Options.glow_defuse_kits;
			config["glow.weapons"] = g_Options.glow_weapons;

			//CHAMS

			config["chams.player.toggle"] = g_Options.chams_player_enabled;
			config["chams.player.enemies_only"] = g_Options.chams_player_enemies_only;
			config["chams.player.wireframe"] = g_Options.chams_player_wireframe;
			config["chams.player.flat"] = g_Options.chams_player_flat;
			config["chams.player.ignorez"] = g_Options.chams_player_ignorez;
			config["chams.player.glass"] = g_Options.chams_player_glass;
			config["chams.player.occulusion"] = g_Options.chams_disable_occulusion;
			config["chams.player.history"] = g_Options.toggle_chams_history;
			config["chams.player.backtrack"] = g_Options.chams_backtrack;
			config["chams.player.backtrack.type"] = g_Options.chams_backtrack_type;
			config["chams.player.desync"] = g_Options.chams_desync;

			//OTHER

			config["other.world.nightmode"] = g_Options.other_nightmode;
			config["other.world.nightmode.size"] = g_Options.other_nightmode_size;
			config["other.world.ambient.light.r"] = g_Options.other_mat_ambient_light_r;
			config["other.world.ambient.light.g"] = g_Options.other_mat_ambient_light_g;
			config["other.world.ambient.light.b"] = g_Options.other_mat_ambient_light_b;
			config["other.world.ambient.light.rainbow"] = g_Options.other_mat_ambient_light_rainbow;
			config["other.world.color.modulation"] = g_Options.enable_world_color_modulation;
			config["other.world.foottrail"] = g_Options.enable_foottrail;
			config["other.world.foottrail.color"] = g_Options.foot_trailcolor;

			config["other.moverec.toggle"] = g_Options.enable_movement_recorder;
			config["other.moverec.playbool"] = g_Options.playBool;
			config["other.moverec.recordbool"] = g_Options.recordBool;
			config["other.moverec.type"] = g_Options.movement_recorder_type;
			config["other.moverec.silent"] = g_Options.silent_movementrecord;
			config["other.moverec.key.play"] = g_Options.play_movement_key;
			config["other.moverec.key.record"] = g_Options.record_movement_key;

			config["other.remove.smoke"] = g_Options.other_no_smoke;
			config["other.remove.3dsky"] = g_Options.other_no_3dsky;
			config["other.remove.single_thread_bones"] = g_Options.other_single_thread_bones;
			config["other.remove.flashbang"] = g_Options.other_no_flash;
			config["other.remove.fog"] = g_Options.other_no_fog;
			config["other.remove.hands"] = g_Options.other_no_hands;
			config["other.remove.shadows"] = g_Options.other_no_shadows;

			config["other.fakeback.toggle"] = g_Options.misc_fakebackwards;
			config["other.fakeback.chooseangle"] = g_Options.misc_fakebackchooseangle;
			config["other.fakeback.angle"] = g_Options.fakeback_angle;
			config["other.fakeback.speed"] = g_Options.fakeback_speed;
			config["other.fakeback.key"] = g_Options.fakebackwardskey;

			config["other.draw.tahoma"] = g_Options.use_tahomafont;
			config["other.draw.fov"] = g_Options.other_drawfov;
			config["other.draw.rcs"] = g_Options.esp_rcscrosshair;

			//MISC

			config["misc.anti.untrusted"] = g_Options.anti_untrusted;
			config["misc.anti.votekick"] = g_Options.misc_antivotekick;
			config["misc.autoaccept"] = g_Options.misc_autoaccept;
			config["misc.killcounter"] = g_Options.kill_counter;
			config["misc.knifebot"] = g_Options.misc_knifebot;
			config["misc.fastduck"] = g_Options.misc_fastduck;
			config["menu.rainbow"] = g_Options.rainbow;
			config["misc.discord.rpc"] = g_Options.discord_rpc;
			config["misc.unlock.inventory"] = g_Options.unlock_inventory;

			config["misc.aspectratio.toggle"] = g_Options.misc_aspectratiotoggle;
			config["misc.aspectratio.size"] = g_Options.misc_aspectratio;

			config["misc.movement.bunnyhop"] = g_Options.misc_bhop;
			config["misc.movement.jumpbug"] = g_Options.misc_jumpbug;
			config["misc.movement.jumpbug.key"] = g_Options.misc_jumpbug_bind;
			config["misc.movement.edgejump"] = g_Options.misc_edgejump;
			config["misc.movement.edgejump.key"] = g_Options.misc_edgejump_bind;
			config["misc.movement.autoalign"] = g_Options.misc_autoalign;
			config["misc.movement.edgebug"] = g_Options.misc_ebug;
			config["misc.movement.ebugalignticks"] = g_Options.misc_ebugalignticks;
			config["misc.movement.ebuglockfactor"] = g_Options.misc_ebuglockfactor;
			config["misc.movement.edgebug.key"] = g_Options.misc_ebug_key;
			config["misc.movement.edgebug.sound"] = g_Options.misc_ebug_sound;
			config["misc.movement.edgebug.detector"] = g_Options.misc_ebugdetector;
			config["misc.movement.faststop"] = g_Options.misc_faststop;
			config["misc.movement.autostrafe"] = g_Options.misc_autostrafe;
			config["misc.movement.autostrafe.retrack"] = g_Options.retrack_speed;
			config["misc.movement.slidewalk"] = g_Options.misc_slide_walk;
			config["misc.movement.slowwalk.speed"] = g_Options.misc_slowwalk_speed;
			config["misc.movement.slowwalk.key"] = g_Options.misc_slowwalk_bind;

			config["misc.keystrokes"] = g_Options.misc_keystrokes;
			config["misc.spectators"] = g_Options.misc_spectatorlist;
			config["misc.sonar"] = g_Options.misc_sonaresp;
			config["misc.copybot"] = g_Options.misc_copybot;
			config["misc.thirdperson"] = g_Options.misc_thirdperson;
			config["misc.thirdperson.key"] = g_Options.misc_thirdperson_bind;
			config["misc.thirdperson.dist"] = g_Options.misc_thirdperson_dist;
			config["misc.snipercrosshair"] = g_Options.misc_snipercrosshair;

			config["misc.draw.ranks"] = g_Options.misc_showranks;
			config["misc.draw.watermark"] = g_Options.misc_watermark;
			config["misc.draw.clantag"] = g_Options.misc_clantag;
			config["misc.draw.velocity"] = g_Options.misc_velocitydraw;
			config["misc.draw.velocity.graph"] = g_Options.misc_velocitygraph;
			config["misc.draw.velocity.rainbow"] = g_Options.rainbow_velocitydraw;
			config["misc.draw.grenade.circle"] = g_Options.misc_grenade_circle;

			config["misc.fakelag.toggle"] = g_Options.fakelag_enabled;
			config["misc.fakelag.standing"] = g_Options.fakelag_standing;
			config["misc.fakelag.moving"] = g_Options.fakelag_moving;
			config["misc.fakelag.unducking"] = g_Options.fakelag_unducking;
			config["misc.fakelag.factor"] = g_Options.fakelag_factor;
			config["misc.fakelag.mode"] = g_Options.fakelag_mode;

			config["misc.radar.engine"] = g_Options.misc_engineradar;
			config["misc.radar.toggle"] = g_Options.misc_radar;
			config["misc.radar.enemiesonly"] = g_Options.misc_radar_enemyonly;

			config["misc.revealer.vote"] = g_Options.misc_voterevealer;
			config["misc.revealer.money"] = g_Options.reveal_money;
			config["misc.revealer.overwatch"] = g_Options.misc_owrevealer;

			config["misc.remove.panorama.blur"] = g_Options.no_panoramablur;
			config["misc.remove.postprocess"] = g_Options.no_postprocess;
			config["misc.remove.blood"] = g_Options.no_blood;

			config["misc.fov.toggle"] = g_Options.misc_changefov;
			config["misc.fov.override"] = g_Options.misc_override_fov;
			config["misc.fov.viewmodel"] = g_Options.misc_viewmodel_fov;

			config["misc.desync"] = g_Options.misc_desync;
			config["misc.desync.key"] = g_Options.misc_desync_bind;

			config["menu.skybox.toggle"] = g_Options.enable_skybox_changer;
			config["menu.skybox.list"] = g_Options.skybox_combo;

			config["misc.indicator.nulling"] = g_Options.misc_nullindicator;

			config["misc.faceit.fakechat"] = g_Options.misc_faceitfakechat;

			config["misc.server.pure"] = g_Options.misc_svpure;

			//SKINS

			Json::Value items;
			for (auto m_item : g_Options.skins.m_items)
			{
				Json::Value item;

				item["enabled"] = m_item.second.enabled;
				item["definition_vector_index"] = m_item.second.definition_vector_index;
				item["definition_index"] = m_item.second.definition_index;
				item["definition_override_index"] = m_item.second.definition_override_index;
				item["definition_override_vector_index"] = m_item.second.definition_override_vector_index;
				item["paint_kit_index"] = m_item.second.paint_kit_index;
				item["paint_kit_vector_index"] = m_item.second.paint_kit_vector_index;
				item["seed"] = m_item.second.seed;
				item["stat_track.enabled"] = m_item.second.stat_trak; //item["stat_track.counter"] = m_item.second.stat_trak.counter;
				item["wear"] = m_item.second.wear;

				item["stickerstoggle"] = m_item.second.enabled_stickers;
				item["stickerid[1]"] = m_item.second.stickers[0].stickers_id;
				item["stickerid[2]"] = m_item.second.stickers[1].stickers_id;
				item["stickerid[3]"] = m_item.second.stickers[2].stickers_id;
				item["stickerid[4]"] = m_item.second.stickers[3].stickers_id;
				item["stickerid[5]"] = m_item.second.stickers[4].stickers_id;
				item["sticker_vector_index[1]"] = m_item.second.stickers[0].sticker_vector_index;
				item["sticker_vector_index[2]"] = m_item.second.stickers[1].sticker_vector_index;
				item["sticker_vector_index[3]"] = m_item.second.stickers[2].sticker_vector_index;
				item["sticker_vector_index[4]"] = m_item.second.stickers[3].sticker_vector_index;
				item["sticker_vector_index[5]"] = m_item.second.stickers[4].sticker_vector_index;

				items.append(item);
			}

			config["skins"] = items;

			config["skinchanger.knifemodel"] = g_Options.skins_knifemodel;
			config["skinchanger.agents.toggle"] = g_Options.enable_agentskins;
			config["skinchanger.agents.ct"] = g_Options.skins_agentmodelct;
			config["skinchanger.agents.tr"] = g_Options.skins_agentmodeltr;

			//COLOR

			Option::Save(config["color.beam.smoke"], g_Options.color_beam_smoke);
			Option::Save(config["color.beam.molotov"], g_Options.color_beam_molotov);
			Option::Save(config["color.beam.decoy"], g_Options.color_beam_decoy);
			Option::Save(config["color.beam.detection"], g_Options.color_detection_beam);

			Option::Save(config["color.esp.ally.visible"], g_Options.color_esp_ally_visible);
			Option::Save(config["color.esp.ally.occluded"], g_Options.color_esp_ally_occluded);
			Option::Save(config["color.esp.enemy.visible"], g_Options.color_esp_enemy_visible);
			Option::Save(config["color.esp.enemy.occluded"], g_Options.color_esp_enemy_occluded);
			Option::Save(config["color.esp.weapons"], g_Options.color_esp_weapons);
			Option::Save(config["color.esp.defuse"], g_Options.color_esp_defuse);
			Option::Save(config["color.esp.c4"], g_Options.color_esp_c4);
			Option::Save(config["color.esp.item"], g_Options.color_esp_item);
			Option::Save(config["color.esp.sounds"], g_Options.color_esp_sounds);
			Option::Save(config["color.esp.foottrail"], g_Options.color_foot_trail);
			Option::Save(config["color.esp.grenade.prediction"], g_Options.color_grenade_prediction);
			Option::Save(config["color.esp.skeleton"], g_Options.color_skeleton);
			Option::Save(config["color.esp.skeleton.backtrack"], g_Options.color_skeleton_backtrack);

			Option::Save(config["color.glow.ally"], g_Options.color_glow_ally);
			Option::Save(config["color.glow.enemy"], g_Options.color_glow_enemy);
			Option::Save(config["color.glow.chickens"], g_Options.color_glow_chickens);
			Option::Save(config["color.glow.defuse"], g_Options.color_glow_defuse);
			Option::Save(config["color.glow.weapons"], g_Options.color_glow_weapons);
			Option::Save(config["color.glow.c4.planted"], g_Options.color_glow_planted_c4);
			Option::Save(config["color.glow.c4.carrier"], g_Options.color_glow_c4_carrier);

			Option::Save(config["color.chams.ally.visible"], g_Options.color_chams_player_ally_visible);
			Option::Save(config["color.chams.ally.occluded"], g_Options.color_chams_player_ally_occluded);
			Option::Save(config["color.chams.enemy.visible"], g_Options.color_chams_player_enemy_visible);
			Option::Save(config["color.chams.enemy.occluded"], g_Options.color_chams_player_enemy_occluded);
			Option::Save(config["color.chams.backtrack"], g_Options.color_chams_backtrack);

			Option::Save(config["color.world.modulation"], g_Options.color_world_modulation);
			Option::Save(config["color.world.molotov.timer"], g_Options.color_molotov_timer_col);

			Option::Save(config["color.indicator.damage"], g_Options.color_damage_indicator);

			Option::Save(config["color.crosshair.rcs"], g_Options.color_rcs_crosshair);

			return config;
		});
}

void CConfig::load(std::string name)
{
	config::load(name, "", true, [](Json::Value root)
		{

			g_ClientState->ForceFullUpdate();

			//LEGIT

			g_Options.legitbot_items.clear();
			Json::Value aimbot_items = root["legitbot"];
			for (Json::Value::iterator it = aimbot_items.begin(); it != aimbot_items.end(); ++it)
			{
				Json::Value settings = *it;

				legitbot_settings aimbot_data;
				Option::Load(settings["toggle"], aimbot_data.enabled);
				Option::Load(settings["deathmatch"], aimbot_data.deathmatch);
				Option::Load(settings["autopistol"], aimbot_data.autopistol);
				Option::Load(settings["autowall"], aimbot_data.autowall);
				Option::Load(settings["autofire"], aimbot_data.autofire);
				Option::Load(settings["autofire.key"], aimbot_data.autofire_key);
				Option::Load(settings["autostop"], aimbot_data.autostop);
				Option::Load(settings["backtrack.time"], aimbot_data.backtrack_time);
				Option::Load(settings["aim.type"], aimbot_data.aim_type);
				Option::Load(settings["aim.priority"], aimbot_data.priority);
				Option::Load(settings["aim.fov.amouth"], aimbot_data.fov);
				Option::Load(settings["aim.smooth.amouth"], aimbot_data.smooth);
				Option::Load(settings["aim.fov.type"], aimbot_data.fov_type);
				Option::Load(settings["aim.smooth.type"], aimbot_data.smooth_type);
				Option::Load(settings["aim.hitbox"], aimbot_data.hitbox);
				Option::Load(settings["aim.silent"], aimbot_data.silent);
				Option::Load(settings["aim.silent.fov"], aimbot_data.silent);
				Option::Load(settings["aim.delay.shot"], aimbot_data.shot_delay);
				Option::Load(settings["aim.delay.kill"], aimbot_data.kill_delay);
				Option::Load(settings["aim.mindamage"], aimbot_data.min_damage);
				Option::Load(settings["check.jump"], aimbot_data.jump_check);
				Option::Load(settings["check.smoke"], aimbot_data.smoke_check);
				Option::Load(settings["check.flash"], aimbot_data.flash_check);
				Option::Load(settings["check.zoom"], aimbot_data.only_in_zoom);
				Option::Load(settings["on_key"], aimbot_data.on_key);
				Option::Load(settings["key"], aimbot_data.key);
				Option::Load(settings["rcs"], aimbot_data.rcs);
				Option::Load(settings["rcs.x"], aimbot_data.rcs_x);
				Option::Load(settings["rcs.y"], aimbot_data.rcs_y);
				Option::Load(settings["rcs.type"], aimbot_data.rcs_type);
				Option::Load(settings["rcs.fov.toggle"], aimbot_data.rcs_fov_enabled);
				Option::Load(settings["rcs.smooth.toggle"], aimbot_data.rcs_smooth_enabled);
				Option::Load(settings["rcs.fov.amouth"], aimbot_data.rcs_fov);
				Option::Load(settings["rcs.smooth.amouth"], aimbot_data.rcs_smooth);
				Option::Load(settings["rcs.start"], aimbot_data.rcs_start);

				g_Options.legitbot_items[it.key().asInt()] = aimbot_data;
			}

			//TRIGGER

			Option::Load(root["trigger.toggle"], g_Options.trigger_enable);
			Option::Load(root["trigger.keybind"], g_Options.trigger_key);
			Option::Load(root["trigger.delay"], g_Options.trigger_delay);
			Option::Load(root["trigger.delay.time"], g_Options.trigger_delay_time);
			Option::Load(root["trigger.hitchance"], g_Options.trigger_hitchance);
			Option::Load(root["trigger.ignore.smoke"], g_Options.trigger_ignoresmoke);
			Option::Load(root["trigger.ignore.flashbang"], g_Options.trigger_ignoreflash);
			Option::Load(root["trigger.check.scope"], g_Options.trigger_scopecheck);

			//ESP

			Option::Load(root["esp.toggle"], g_Options.esp_enabled);
			Option::Load(root["esp.visiblecheck"], g_Options.esp_visiblecheck);
			Option::Load(root["esp.smokecheck"], g_Options.esp_smokecheck);
			Option::Load(root["esp.enemiesonly"], g_Options.esp_enemies_only);

			Option::Load(root["esp.indicator.damage"], g_Options.esp_damageindicator);
			Option::Load(root["esp.indicator.damage.hp"], g_Options.esp_damageindremainhp);
			Option::Load(root["esp.indicator.movement"], g_Options.movement_indicators);
			Option::Load(root["esp.indicator.choke"], g_Options.esp_choke_indicator);

			Option::Load(root["esp.player.skeleton.backtrack"], g_Options.esp_backtrackskeleton);
			Option::Load(root["esp.player.skeleton"], g_Options.esp_player_skeleton);
			Option::Load(root["esp.player.boxes"], g_Options.esp_player_boxes);
			Option::Load(root["esp.player.boxes.type"], g_Options.esp_player_boxes_type);
			Option::Load(root["esp.player.names"], g_Options.esp_player_names);
			Option::Load(root["esp.player.health"], g_Options.esp_player_health);
			Option::Load(root["esp.player.armour"], g_Options.esp_player_armour);
			Option::Load(root["esp.player.weapons"], g_Options.esp_player_weapons);
			Option::Load(root["esp.player.backtrack"], g_Options.esp_backtrack);
			Option::Load(root["esp.player.sounds"], g_Options.esp_sounds);
			Option::Load(root["esp.player.sounds.time"], g_Options.esp_sounds_time);
			Option::Load(root["esp.player.sounds.radius"], g_Options.esp_sounds_radius);
			Option::Load(root["esp.player.anglelines"], g_Options.esp_angle_lines);

			Option::Load(root["esp.hitmarker"], g_Options.esp_hitmarker);
			Option::Load(root["esp.detectionsparks"], g_Options.esp_detectionsparks);
			Option::Load(root["esp.detectionhealthboost"], g_Options.esp_detectionhealthboost);
			Option::Load(root["esp.detectionbeam"], g_Options.esp_detectionbeam);
			Option::Load(root["esp.hitsound"], g_Options.esp_hitsound);
			Option::Load(root["esp.killsound"], g_Options.esp_killsound);

			Option::Load(root["esp.player.flags"], g_Options.esp_flags);
			Option::Load(root["esp.player.flags.armor"], g_Options.esp_flags_armor);
			Option::Load(root["esp.player.flags.kit"], g_Options.esp_flags_kit);
			Option::Load(root["esp.player.flags.hostage"], g_Options.esp_flags_hostage);
			Option::Load(root["esp.player.flags.scoped"], g_Options.esp_flags_scoped);
			Option::Load(root["esp.player.flags.money"], g_Options.esp_flags_money);
			Option::Load(root["esp.player.flags.c4"], g_Options.esp_flags_c4);
			Option::Load(root["esp.player.flags.defusing"], g_Options.esp_flags_defusing);
			Option::Load(root["esp.player.flags.grenadekill"], g_Options.esp_grenadekillalert);
			Option::Load(root["esp.player.flags.nirvanauser"], g_Options.esp_draw_nrvusers);

			Option::Load(root["esp.world.items"], g_Options.esp_items);
			Option::Load(root["esp.world.grenade.throwable"], g_Options.esp_throwable);
			Option::Load(root["esp.world.weapons.dropped"], g_Options.esp_player_weapons);
			Option::Load(root["esp.world.defusekit"], g_Options.esp_defuse_kit);
			Option::Load(root["esp.world.molotov.timer"], g_Options.esp_molotovtimer);
			Option::Load(root["esp.world.bomb.planted"], g_Options.esp_planted_c4);
			Option::Load(root["esp.world.bomb.timer"], g_Options.esp_bombtimer);
			Option::Load(root["esp.world.bomb.logs"], g_Options.esp_bomblogs);
			Option::Load(root["esp.world.grenade.prediction"], g_Options.esp_grenade_prediction);
			Option::Load(root["esp.world.motionblur"], g_Options.esp_motionblur);
			Option::Load(root["esp.world.motionblurstrength"], g_Options.esp_motionblur_strength);

			//GLOW

			Option::Load(root["glow.toggle"], g_Options.glow_enabled);
			Option::Load(root["glow.enemiesonly"], g_Options.glow_enemies_only);
			Option::Load(root["glow.players"], g_Options.glow_players);
			Option::Load(root["glow.chickens"], g_Options.glow_chickens);
			Option::Load(root["glow.bomb.carrier"], g_Options.glow_c4_carrier);
			Option::Load(root["glow.bomb.planted"], g_Options.glow_planted_c4);
			Option::Load(root["glow.defusekit"], g_Options.glow_defuse_kits);
			Option::Load(root["glow.weapons"], g_Options.glow_weapons);

			//CHAMS

			Option::Load(root["chams.player.toggle"], g_Options.chams_player_enabled);
			Option::Load(root["chams.player.enemies_only"], g_Options.chams_player_enemies_only);
			Option::Load(root["chams.player.wireframe"], g_Options.chams_player_wireframe);
			Option::Load(root["chams.player.flat"], g_Options.chams_player_flat);
			Option::Load(root["chams.player.ignorez"], g_Options.chams_player_ignorez);
			Option::Load(root["chams.player.glass"], g_Options.chams_player_glass);
			Option::Load(root["chams.player.occulusion"], g_Options.chams_disable_occulusion);
			Option::Load(root["chams.player.history"], g_Options.toggle_chams_history);
			Option::Load(root["chams.player.backtrack"], g_Options.chams_backtrack);
			Option::Load(root["chams.player.backtrack.type"], g_Options.chams_backtrack_type);
			Option::Load(root["chams.player.desync"], g_Options.chams_desync);

			//OTHER

			Option::Load(root["other.world.nightmode"], g_Options.other_nightmode);
			Option::Load(root["other.world.nightmode.size"], g_Options.other_nightmode_size);
			Option::Load(root["other.world.ambient.light.r"], g_Options.other_mat_ambient_light_r);
			Option::Load(root["other.world.ambient.light.g"], g_Options.other_mat_ambient_light_g);
			Option::Load(root["other.world.ambient.light.b"], g_Options.other_mat_ambient_light_b);
			Option::Load(root["other.world.ambient.light.rainbow"], g_Options.other_mat_ambient_light_rainbow);
			Option::Load(root["other.world.color.modulation"], g_Options.enable_world_color_modulation);
			Option::Load(root["other.world.foottrail"], g_Options.enable_foottrail);
			Option::Load(root["other.world.foottrail.color"], g_Options.foot_trailcolor);

			Option::Load(root["other.moverec.toggle"], g_Options.enable_movement_recorder);
			Option::Load(root["other.moverec.playbool"], g_Options.playBool);
			Option::Load(root["other.moverec.recordbool"], g_Options.recordBool);
			Option::Load(root["other.moverec.type"], g_Options.movement_recorder_type);
			Option::Load(root["other.moverec.silent"], g_Options.silent_movementrecord);
			Option::Load(root["other.moverec.key.play"], g_Options.play_movement_key);
			Option::Load(root["other.moverec.key.record"], g_Options.record_movement_key);

			Option::Load(root["other.remove.smoke"], g_Options.other_no_smoke);
			Option::Load(root["other.remove.3dsky"], g_Options.other_no_3dsky);
			Option::Load(root["other.remove.single_thread_bones"], g_Options.other_single_thread_bones);
			Option::Load(root["other.remove.flashbang"], g_Options.other_no_flash);
			Option::Load(root["other.remove.fog"], g_Options.other_no_fog);
			Option::Load(root["other.remove.hands"], g_Options.other_no_hands);
			Option::Load(root["other.remove.shadows"], g_Options.other_no_shadows);

			Option::Load(root["other.fakeback.toggle"], g_Options.misc_fakebackwards);
			Option::Load(root["other.fakeback.chooseangle"], g_Options.misc_fakebackchooseangle);
			Option::Load(root["other.fakeback.angle"], g_Options.fakeback_angle);
			Option::Load(root["other.fakeback.speed"], g_Options.fakeback_speed);
			Option::Load(root["other.fakeback.key"], g_Options.fakebackwardskey);

			Option::Load(root["other.draw.tahoma"], g_Options.use_tahomafont);
			Option::Load(root["other.draw.fov"], g_Options.other_drawfov);
			Option::Load(root["other.draw.rcs"], g_Options.esp_rcscrosshair);

			//MISC

			Option::Load(root["misc.anti.untrusted"], g_Options.anti_untrusted, true);
			Option::Load(root["misc.anti.votekick"], g_Options.misc_antivotekick);
			Option::Load(root["misc.autoaccept"], g_Options.misc_autoaccept);
			Option::Load(root["misc.killcounter"], g_Options.kill_counter);
			Option::Load(root["misc.knifebot"], g_Options.misc_knifebot);
			Option::Load(root["misc.fastduck"], g_Options.misc_fastduck);
			Option::Load(root["menu.rainbow"], g_Options.rainbow);
			Option::Load(root["misc.discord.rpc"], g_Options.discord_rpc);
			Option::Load(root["misc.unlock.inventory"], g_Options.unlock_inventory);

			Option::Load(root["misc.aspectratio.toggle"], g_Options.misc_aspectratiotoggle);
			Option::Load(root["misc.aspectratio.size"], g_Options.misc_aspectratio);

			Option::Load(root["misc.movement.bunnyhop"], g_Options.misc_bhop);
			Option::Load(root["misc.movement.jumpbug"], g_Options.misc_jumpbug);
			Option::Load(root["misc.movement.jumpbug.key"], g_Options.misc_jumpbug_bind);
			Option::Load(root["misc.movement.edgejump"], g_Options.misc_edgejump);
			Option::Load(root["misc.movement.edgejump.key"], g_Options.misc_edgejump_bind);
			Option::Load(root["misc.movement.ebugalignticks"], g_Options.misc_ebugalignticks);
			Option::Load(root["misc.movement.ebuglockfactor"], g_Options.misc_ebuglockfactor);
			Option::Load(root["misc.movement.autoalign"], g_Options.misc_autoalign);
			Option::Load(root["misc.movement.edgebug"], g_Options.misc_ebug);
			Option::Load(root["misc.movement.edgebug.key"], g_Options.misc_ebug_key);
			Option::Load(root["misc.movement.edgebug.sound"], g_Options.misc_ebug_sound);
			Option::Load(root["misc.movement.edgebug.detector"], g_Options.misc_ebugdetector);
			Option::Load(root["misc.movement.faststop"], g_Options.misc_faststop);
			Option::Load(root["misc.movement.autostrafe"], g_Options.misc_autostrafe);
			Option::Load(root["misc.movement.autostrafe.retrack"], g_Options.retrack_speed);
			Option::Load(root["misc.movement.slidewalk"], g_Options.misc_slide_walk);
			Option::Load(root["misc.movement.slowwalk.speed"], g_Options.misc_slowwalk_speed);
			Option::Load(root["misc.movement.slowwalk.key"], g_Options.misc_slowwalk_bind);

			Option::Load(root["misc.keystrokes"], g_Options.misc_keystrokes);
			Option::Load(root["misc.spectators"], g_Options.misc_spectatorlist);
			Option::Load(root["misc.sonar"], g_Options.misc_sonaresp);
			Option::Load(root["misc.copybot"], g_Options.misc_copybot);
			Option::Load(root["misc.thirdperson"], g_Options.misc_thirdperson);
			Option::Load(root["misc.thirdperson.key"], g_Options.misc_thirdperson_bind);
			Option::Load(root["misc.thirdperson.dist"], g_Options.misc_thirdperson_dist);
			Option::Load(root["misc.snipercrosshair"], g_Options.misc_snipercrosshair);

			Option::Load(root["misc.draw.ranks"], g_Options.misc_showranks);
			Option::Load(root["misc.draw.watermark"], g_Options.misc_watermark, true);
			Option::Load(root["misc.draw.clantag"], g_Options.misc_clantag);
			Option::Load(root["misc.draw.velocity"], g_Options.misc_velocitydraw);
			Option::Load(root["misc.draw.velocity.graph"], g_Options.misc_velocitygraph);
			Option::Load(root["misc.draw.velocity.rainbow"], g_Options.rainbow_velocitydraw);
			Option::Load(root["misc.draw.grenade.circle"], g_Options.misc_grenade_circle);

			Option::Load(root["misc.fakelag.toggle"], g_Options.fakelag_enabled);
			Option::Load(root["misc.fakelag.standing"], g_Options.fakelag_standing);
			Option::Load(root["misc.fakelag.moving"], g_Options.fakelag_moving);
			Option::Load(root["misc.fakelag.unducking"], g_Options.fakelag_unducking);
			Option::Load(root["misc.fakelag.factor"], g_Options.fakelag_factor);
			Option::Load(root["misc.fakelag.mode"], g_Options.fakelag_mode);

			Option::Load(root["misc.radar.engine"], g_Options.misc_engineradar);
			Option::Load(root["misc.radar.toggle"], g_Options.misc_radar);
			Option::Load(root["misc.radar.enemiesonly"], g_Options.misc_radar_enemyonly);

			Option::Load(root["misc.revealer.vote"], g_Options.misc_voterevealer);
			Option::Load(root["misc.revealer.money"], g_Options.reveal_money);
			Option::Load(root["misc.revealer.overwatch"], g_Options.misc_owrevealer);

			Option::Load(root["misc.remove.panorama.blur"], g_Options.no_panoramablur);
			Option::Load(root["misc.remove.postprocess"], g_Options.no_postprocess);
			Option::Load(root["misc.remove.blood"], g_Options.no_blood);

			Option::Load(root["misc.fov.toggle"], g_Options.misc_changefov);
			Option::Load(root["misc.fov.override"], g_Options.misc_override_fov);
			Option::Load(root["misc.fov.viewmodel"], g_Options.misc_viewmodel_fov);

			Option::Load(root["misc.desync"], g_Options.misc_desync);
			Option::Load(root["misc.desync.key"], g_Options.misc_desync_bind);

			Option::Load(root["menu.skybox.toggle"], g_Options.enable_skybox_changer);
			Option::Load(root["menu.skybox.list"], g_Options.skybox_combo);

			Option::Load(root["misc.indicator.nulling"], g_Options.misc_nullindicator);

			Option::Load(root["misc.faceit.fakechat"], g_Options.misc_faceitfakechat);

			Option::Load(root["misc.server.pure"], g_Options.misc_svpure);

			//COLOR

			Option::Load(root["color.beam.smoke"], g_Options.color_beam_smoke);
			Option::Load(root["color.beam.molotov"], g_Options.color_beam_molotov);
			Option::Load(root["color.beam.decoy"], g_Options.color_beam_decoy);
			Option::Load(root["color.beam.detection"], g_Options.color_detection_beam);

			Option::Load(root["color.esp.ally.visible"], g_Options.color_esp_ally_visible);
			Option::Load(root["color.esp.ally.occluded"], g_Options.color_esp_ally_occluded);
			Option::Load(root["color.esp.enemy.visible"], g_Options.color_esp_enemy_visible);
			Option::Load(root["color.esp.enemy.occluded"], g_Options.color_esp_enemy_occluded);
			Option::Load(root["color.esp.weapons"], g_Options.color_esp_weapons);
			Option::Load(root["color.esp.defuse"], g_Options.color_esp_defuse);
			Option::Load(root["color.esp.c4"], g_Options.color_esp_c4);
			Option::Load(root["color.esp.item"], g_Options.color_esp_item);
			Option::Load(root["color.esp.sounds"], g_Options.color_esp_sounds);
			Option::Load(root["color.esp.foottrail"], g_Options.color_foot_trail);
			Option::Load(root["color.esp.grenade.prediction"], g_Options.color_grenade_prediction);
			Option::Load(root["color.esp.skeleton"], g_Options.color_skeleton);
			Option::Load(root["color.esp.skeleton.backtrack"], g_Options.color_skeleton_backtrack);

			Option::Load(root["color.glow.ally"], g_Options.color_glow_ally);
			Option::Load(root["color.glow.enemy"], g_Options.color_glow_enemy);
			Option::Load(root["color.glow.chickens"], g_Options.color_glow_chickens);
			Option::Load(root["color.glow.defuse"], g_Options.color_glow_defuse);
			Option::Load(root["color.glow.weapons"], g_Options.color_glow_weapons);
			Option::Load(root["color.glow.c4.planted"], g_Options.color_glow_planted_c4);
			Option::Load(root["color.glow.c4.carrier"], g_Options.color_glow_c4_carrier);

			Option::Load(root["color.chams.ally.visible"], g_Options.color_chams_player_ally_visible);
			Option::Load(root["color.chams.ally.occluded"], g_Options.color_chams_player_ally_occluded);
			Option::Load(root["color.chams.enemy.visible"], g_Options.color_chams_player_enemy_visible);
			Option::Load(root["color.chams.enemy.occluded"], g_Options.color_chams_player_enemy_occluded);
			Option::Load(root["color.chams.backtrack"], g_Options.color_chams_backtrack);

			Option::Load(root["color.world.modulation"], g_Options.color_world_modulation);
			Option::Load(root["color.world.molotov.timer"], g_Options.color_molotov_timer_col);

			Option::Load(root["color.indicator.damage"], g_Options.color_damage_indicator);

			Option::Load(root["color.crosshair.rcs"], g_Options.color_rcs_crosshair);

			Json::Value items = root["skins"];
			if (items.isNull())
				return;

			for (auto item : items)
			{
				auto skin_data = &g_Options.skins.m_items[item["definition_index"].asInt()];

				Option::Load(item["enabled"], skin_data->enabled);

				Option::Load(item["definition_vector_index"], skin_data->definition_vector_index);

				Option::Load(item["definition_index"], skin_data->definition_index);
				Option::Load(item["definition_override_index"], skin_data->definition_override_index);
				Option::Load(item["definition_override_vector_index"], skin_data->definition_override_vector_index);
				Option::Load(item["paint_kit_index"], skin_data->paint_kit_index);
				Option::Load(item["paint_kit_vector_index"], skin_data->paint_kit_vector_index);
				Option::Load(item["wear"], skin_data->wear);
				Option::Load(item["stattrack.enabled"], skin_data->stat_trak);
				Option::Load(item["seed"], skin_data->seed);
				if (root.isMember("skins.nametag"))
					strcpy_s(skin_data->custom_name, sizeof(skin_data->custom_name), root["skins.nametag"].asCString());

				//&g_Options.skins.m_items[key.definition_index].stickers[selected_entry.stickers_place].stickers_id

				Option::Load(item["stickerstoggle"], skin_data->enabled_stickers);
				Option::Load(item["stickerid[1]"], skin_data->stickers[0].stickers_id);
				Option::Load(item["stickerid[2]"], skin_data->stickers[1].stickers_id);
				Option::Load(item["stickerid[3]"], skin_data->stickers[2].stickers_id);
				Option::Load(item["stickerid[4]"], skin_data->stickers[3].stickers_id);
				Option::Load(item["stickerid[5]"], skin_data->stickers[4].stickers_id);
				Option::Load(item["sticker_vector_index[1]"], skin_data->stickers[0].sticker_vector_index);
				Option::Load(item["sticker_vector_index[2]"], skin_data->stickers[1].sticker_vector_index);
				Option::Load(item["sticker_vector_index[3]"], skin_data->stickers[2].sticker_vector_index);
				Option::Load(item["sticker_vector_index[4]"], skin_data->stickers[3].sticker_vector_index);
				Option::Load(item["sticker_vector_index[5]"], skin_data->stickers[4].sticker_vector_index);
			}

			Option::Load(root["skinchanger.knifemodel"], g_Options.skins_knifemodel);
			Option::Load(root["skinchanger.agents.toggle"],  g_Options.enable_agentskins);
			Option::Load(root["skinchanger.agents.ct"], g_Options.skins_agentmodelct);
			Option::Load(root["skinchanger.agents.tr"], g_Options.skins_agentmodeltr);

	});

}

/*void CConfig::SetupLegit( ) {
	for( auto&[key, val] : weapon_names ) {
		SetupValue( g_Options.legitbot_items[key].enabled, false, ( val ), ( "enabled" ) );
		SetupValue( g_Options.legitbot_items[key].deathmatch, false, ( val ), ( "deathmatch" ) );
		SetupValue( g_Options.legitbot_items[key].autopistol, false, ( val ), ( "autopistol" ) );
		SetupValue( g_Options.legitbot_items[key].smoke_check, false, ( val ), ( "smoke_check" ) );
		SetupValue( g_Options.legitbot_items[key].flash_check, false, ( val ), ( "flash_check" ) );
		SetupValue( g_Options.legitbot_items[key].jump_check, false, ( val ), ( "jump_check" ) );
		SetupValue( g_Options.legitbot_items[key].autowall, false, ( val ), ( "autowall" ) );
		SetupValue( g_Options.legitbot_items[key].silent, false, ( val ), ( "silent" ) );
		SetupValue( g_Options.legitbot_items[key].autofire, false, (val), ("autofire") );
		SetupValue( g_Options.legitbot_items[key].on_key, true, (val), ("on_key") );
		SetupValue( g_Options.legitbot_items[key].rcs, false, ( val ), ( "rcs" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_fov_enabled, false, ( val ), ( "rcs_fov_enabled" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_smooth_enabled, false, ( val ), ( "rcs_smooth_enabled" ) );
		SetupValue( g_Options.legitbot_items[key].autostop, false, ( val ), ( "autostop" ) );
		SetupValue( g_Options.legitbot_items[key].backtrack_time, 0.f, ( val ), ( "backtrack_time" ) );
		SetupValue( g_Options.legitbot_items[key].only_in_zoom, false, ( val ), ( "only_in_zoom" ) );
		SetupValue( g_Options.legitbot_items[key].autofire_key, 1, (val), ("autofire_key") );
		SetupValue( g_Options.legitbot_items[key].key, 1, (val), ("key") );
		SetupValue( g_Options.legitbot_items[key].aim_type, 1, ( val ), ( "aim_type" ) );
		SetupValue( g_Options.legitbot_items[key].priority, 0, ( val ), ( "priority" ) );
		SetupValue( g_Options.legitbot_items[key].fov_type, 0, ( val ), ( "fov_type" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_type, 0, ( val ), ( "rcs_type" ) );
		SetupValue( g_Options.legitbot_items[key].smooth_type, 0, ( val ), ( "smooth_type" ) );
		SetupValue( g_Options.legitbot_items[key].hitbox, 1, ( val ), ( "hitbox" ) );
		SetupValue( g_Options.legitbot_items[key].fov, 0.f, ( val ), ( "fov" ) );
		SetupValue( g_Options.legitbot_items[key].silent_fov, 0.f, ( val ), ( "silent_fov" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_fov, 0.f, ( val ), ( "rcs_fov" ) );
		SetupValue( g_Options.legitbot_items[key].smooth, 1, ( val ), ( "smooth" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_smooth, 1, ( val ), ( "rcs_smooth" ) );
		SetupValue( g_Options.legitbot_items[key].shot_delay, 0, ( val ), ( "shot_delay" ) );
		SetupValue( g_Options.legitbot_items[key].kill_delay, 0, ( val ), ( "kill_delay" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_x, 100, ( val ), ( "rcs_x" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_y, 100, ( val ), ( "rcs_y" ) );
		SetupValue( g_Options.legitbot_items[key].rcs_start, 1, ( val ), ( "rcs_start" ) );
		SetupValue( g_Options.legitbot_items[key].min_damage, 1, ( val ), ( "min_damage" ) );
	}

	SetupValue(g_Options.trigger_enable, false, ( "trigger" ), ("trigger_enable"));
	SetupValue(g_Options.trigger_key, 0, ("trigger"), ("trigger_key"));
	SetupValue(g_Options.trigger_checksmoke, false, ("trigger"), ("trigger_smoke"));
	SetupValue(g_Options.trigger_checkflash, false, ("trigger"), ("trigger_flash"));
	SetupValue(g_Options.trigger_hitchance, 0.f, ("trigger"), ("trigger_hitchance"));
	SetupValue(g_Options.trigger_delay, 0.f, ("trigger"), ("trigger_delay"));
	SetupValue(g_Options.trigger_head, false, ("trigger"), ("trigger_head"));
	SetupValue(g_Options.trigger_chest, false, ("trigger"), ("trigger_chest"));
	SetupValue(g_Options.trigger_stomach, false, ("trigger"), ("trigger_stomach"));
	SetupValue(g_Options.trigger_leftarm, false, ("trigger"), ("trigger_la"));
	SetupValue(g_Options.trigger_rightarm, false, ("trigger"), ("trigger_ra"));
	SetupValue(g_Options.trigger_leftleg, false, ("trigger"), ("trigger_ll"));
	SetupValue(g_Options.trigger_rightleg, false, ("trigger"), ("trigger_rl"));
}

void CConfig::SetupVisuals( ) {
	SetupValue( g_Options.esp_enabled, false, ( "esp" ), ( "enabled" ) );
	SetupValue( g_Options.movement_indicators, false, ( "esp" ), ( "movement_indicators" ) );
	SetupValue( g_Options.esp_visiblecheck, false, ( "esp" ), ( "visiblecheck" ) );
	SetupValue( g_Options.esp_smokecheck, false, ( "esp" ), ( "smokecheck" ) );
	SetupValue( g_Options.esp_backtrackskeleton, false, ( "esp" ), ( "backtrackskeleton" ) );
	SetupValue( g_Options.esp_damageindicator, false, ( "esp" ), ( "damageindicator" ) );
	SetupValue( g_Options.esp_damageindremainhp, false, ( "esp" ), ( "damageindremainhp" ) );
	SetupValue( g_Options.esp_flags, false, ( "esp" ), ( "flags" ) );
	SetupValue( g_Options.esp_enemies_only, false, ( "esp" ), ( "enemies_only" ) );
	SetupValue( g_Options.esp_player_skeleton, false, ( "esp" ), ( "player_skeleton" ) );
	SetupValue( g_Options.esp_player_boxes, false, ( "esp" ), ( "player_boxes" ) );
	SetupValue( g_Options.esp_player_boxes_type, 0, ( "esp" ), ( "player_boxes_type" ) );
	SetupValue( g_Options.esp_dropped_weapons_type, 0, ( "esp" ), ( "dropped_weapons_type" ) );
//	SetupValue( g_Options.esp_weapon_type, 0, ( "esp" ), ( "weapon_type" ) );
	SetupValue( g_Options.esp_player_names, false, ( "esp" ), ( "player_names" ) );
	SetupValue( g_Options.esp_grenadekillalert, false, ( "esp" ), ( "grenadekillalert" ) );
	SetupValue( g_Options.esp_throwable, false, ( "esp" ), ( "throwable" ) );
	SetupValue( g_Options.esp_player_health, false, ( "esp" ), ( "player_health" ) );
	SetupValue( g_Options.esp_player_armour, false, ( "esp" ), ( "player_armour" ) );
	SetupValue( g_Options.esp_player_weapons, false, ( "esp" ), ( "player_weapons" ) );
	SetupValue( g_Options.esp_dropped_weapons, false, ( "esp" ), ( "dropped_weapons" ) );
	SetupValue( g_Options.esp_defuse_kit, false, ( "esp" ), ( "defuse_kit" ) );
	SetupValue( g_Options.esp_planted_c4, false, ( "esp" ), ( "planted_c4" ) );
	SetupValue( g_Options.esp_bombtimer, false, ( "esp" ), ( "bombtimer" ) );
	SetupValue( g_Options.esp_bomblogs, false, ( "esp" ), ( "bomblogs" ) );
	SetupValue( g_Options.esp_molotovtimer, false, ( "esp" ), ( "molotovtimer" ) );
	SetupValue( g_Options.esp_items, false, ( "esp" ), ( "items" ) );
	SetupValue( g_Options.esp_grenade_prediction, false, ( "esp" ), ( "grenade_prediction" ) );
	SetupValue( g_Options.esp_backtrack, false, ( "esp" ), ( "backtrack" ) );
	SetupValue( g_Options.esp_sounds, false, ( "esp" ), ( "sounds" ) );
	SetupValue( g_Options.esp_sounds_time, 0.5f, ( "esp" ), ( "sounds_time" ) );
	SetupValue( g_Options.esp_sounds_radius, 15.0f, ( "esp" ), ( "sounds_radius" ) );
	SetupValue( g_Options.esp_choke_indicator, false, ( "esp" ), ( "choke_indicator" ) );
	SetupValue( g_Options.esp_angle_lines, false, ( "esp" ), ( "angle_lines" ) );
	SetupValue( g_Options.esp_hitmarker, false, ( "esp" ), ( "hitmarker" ) );
	SetupValue( g_Options.esp_hitsound, 0, ( "esp" ), ( "hitsound" ) );
	SetupValue( g_Options.esp_killsound, 0, ( "esp" ), ( "killsound" ) );

	SetupValue(g_Options.esp_flags_armor, false, ("esp"), ("flags_armor"));
	SetupValue(g_Options.esp_flags_c4, false, ("esp"), ("flags_c4"));
	SetupValue(g_Options.esp_flags_kit, false, ("esp"), ("flags_kit"));
	SetupValue(g_Options.esp_flags_defusing, false, ("esp"), ("flags_defusing"));
	SetupValue(g_Options.esp_flags_scoped, false, ("esp"), ("flags_scoped"));
	SetupValue(g_Options.esp_flags_money, false, ("esp"), ("flags_money"));
	SetupValue(g_Options.esp_flags_hostage, false, ("esp"), ("flags_hostage"));

	SetupValue( g_Options.glow_enabled, false, ( "glow" ), ( "enabled" ) );
	SetupValue( g_Options.glow_enemies_only, false, ( "glow" ), ( "enemies_only" ) );
	SetupValue( g_Options.glow_players, false, ( "glow" ), ( "players" ) );
	SetupValue( g_Options.glow_chickens, false, ( "glow" ), ( "chickens" ) );
	SetupValue( g_Options.glow_c4_carrier, false, ( "glow" ), ( "c4_carrier" ) );
	SetupValue( g_Options.glow_planted_c4, false, ( "glow" ), ( "planted_c4" ) );
	SetupValue( g_Options.glow_defuse_kits, false, ( "glow" ), ( "defuse_kits" ) );
	SetupValue( g_Options.glow_weapons, false, ( "glow" ), ( "weapons" ) );

	SetupValue( g_Options.chams_player_enabled, false, ( "chams" ), ( "player_enabled" ) );
	SetupValue( g_Options.chams_player_enemies_only, false, ( "chams" ), ( "player_enemies_only" ) );
	SetupValue( g_Options.chams_player_wireframe, false, ( "chams" ), ( "player_wireframe" ) );
	SetupValue( g_Options.chams_player_ignorez, false, ( "chams" ), ( "player_ignorez" ) );
	SetupValue( g_Options.chams_player_glass, false, ( "chams" ), ( "player_glass" ) );
	SetupValue( g_Options.chams_disable_occulusion, false, ( "chams" ), ( "disable_occulusion" ) );
	SetupValue( g_Options.toggle_chams_history, false, ( "chams" ), ( "toggle_chams_history" ) );
	SetupValue(g_Options.chams_backtrack_type, 0, ("chams"), ("chams_backtrack_type"));
	SetupValue( g_Options.chams_backtrack, 0, ( "chams" ), ( "backtrack" ) );
	SetupValue( g_Options.chams_desync, false, ( "chams" ), ( "desync" ) );

	SetupValue(g_Options.other_nightmode, false, ("other"), ("nightmode"));
	SetupValue(g_Options.other_nightmode_size, 0.2f, ("other"), ("nightmode_size"));
	SetupValue(g_Options.other_mat_ambient_light_r, false, ("other"), ("mat_ambient_light_r"));
	SetupValue(g_Options.other_mat_ambient_light_g, false, ("other"), ("mat_ambient_light_g"));
	SetupValue(g_Options.other_mat_ambient_light_b, false, ("other"), ("mat_ambient_light_b"));
	SetupValue(g_Options.other_mat_ambient_light_rainbow, false, ("other"), ("mat_ambient_light_rainbow"));
	SetupValue(g_Options.enable_world_color_modulation, false, ("other"), ("world_color_modulation"));

	SetupValue( g_Options.other_drawfov, false, ( "other" ), ( "drawfov" ) );
	SetupValue( g_Options.other_no_hands, false, ( "other" ), ( "no_hands" ) );
	SetupValue( g_Options.other_no_smoke, false, ( "other" ), ( "no_smoke" ) );
	SetupValue( g_Options.other_no_flash, false, ( "other" ), ( "no_flash" ) );
	SetupValue( g_Options.use_tahomafont, false, ( "other" ), ( "use_tahomafont" ) );
	SetupValue( g_Options.misc_fakebackwards, false, ( "other" ), ( "fakebackwards" ) );
	SetupValue( g_Options.misc_fakebackchooseangle, false, ( "other" ), ( "fakebackchooseangle" ) );
	SetupValue(g_Options.fakeback_speed, 1.f, ("other"), ("fakebackspeed"));
	SetupValue(g_Options.fakebackwardskey, 0, ("other"), ("fakebackwardskey"));
	SetupValue(g_Options.fakeback_angle, 0, ("other"), ("fakeback_angle"));
	SetupValue( g_Options.playBool, false, ( "other" ), ( "playBool" ) );
	SetupValue( g_Options.recordBool, false, ( "other" ), ( "recordBool" ) );
	SetupValue( g_Options.silent_movementrecord, false, ( "other" ), ( "silent_movementrecord" ) );
	SetupValue( g_Options.enable_movement_recorder, false, ( "other" ), ( "enable_movement_recorder" ) );
	SetupValue( g_Options.record_movement_key, 0, ( "other" ), ( "record_movement_key" ) );
	SetupValue( g_Options.play_movement_key, 0, ( "other" ), ( "play_movement_key" ) );
}

void CConfig::SetupMisc( )
{
	SetupValue(g_Options.enable_legitaa, false, ("misc"), ("enable_legitaa"));
	SetupValue(g_Options.legit_resolver, false, ("misc"), ("legit_resolver"));
	SetupValue(g_Options.legitaa_indicators, false, ("misc"), ("legitaa_indicators"));
	SetupValue(g_Options.legitaa_key, 0, ("misc"), ("legitaa_key"));
	SetupValue(g_Options.legitaa_packets, 0, ("misc"), ("legitaa_packets"));

	SetupValue( g_Options.rainbow, false, ( "misc" ), ( "rainbow" ) );
	SetupValue( g_Options.misc_sonaresp, false, ( "misc" ), ( "sonaresp" ) );
	SetupValue( g_Options.kill_counter, false, ( "misc" ), ( "kill_counter" ) );
	SetupValue( g_Options.misc_fastduck, false, ( "misc" ), ( "fastduck" ) );
//	SetupValue( g_Options.block_bot, false, ( "misc" ), ( "block_bot" ) );
//	SetupValue( g_Options.blockbot_key, 0, ( "misc" ), ( "blockbot_key" ) );
	SetupValue( g_Options.misc_changefov, false, ( "misc" ), ( "changefov" ) );
	SetupValue( g_Options.misc_copybot, false, ( "misc" ), ( "copybot" ) );
	SetupValue( g_Options.misc_grenade_circle, false, ( "misc" ), ( "grenade_circle" ) );
	SetupValue( g_Options.anti_untrusted, true, ( "misc" ), ( "anti_untrusted" ) );
	SetupValue( g_Options.reveal_money, true, ( "misc" ), ( "reveal_money" ) );
	SetupValue( g_Options.unlock_inventory, true, ( "misc" ), ( "unlock_inventory" ) );
	SetupValue( g_Options.discord_rpc, false, ( "misc" ), ( "discord_rpc" ) );
	SetupValue( g_Options.misc_autoaccept, true, ( "misc" ), ( "auto_accept" ) );
	SetupValue( g_Options.misc_svpure, false, ( "misc" ), ( "sv_pure" ) );
	SetupValue( g_Options.misc_owrevealer, false, ( "misc" ), ( "owrevealer" ) );
	SetupValue( g_Options.no_panoramablur, false, ( "misc" ), ( "no_panoramablur" ) );
	SetupValue( g_Options.enable_skybox_changer, false, ( "misc" ), ( "enable_skybox_changer" ) );
	SetupValue( g_Options.skybox_combo, 0, ( "misc" ), ( "skybox_combo" ) );
	SetupValue( g_Options.no_postprocess, false, ( "misc" ), ( "no_postprocess" ) );
	SetupValue( g_Options.no_blood, false, ( "misc" ), ( "no_blood" ) );
	SetupValue( g_Options.misc_slide_walk, false, ( "misc" ), ( "slide_walk" ) );
	SetupValue( g_Options.misc_ebug, false, ( "misc" ), ( "ebug" ) );
	SetupValue( g_Options.misc_ebugdetector, false, ( "misc" ), ( "ebugdetector" ) );
	SetupValue( g_Options.misc_ebug_sound, 0, ( "misc" ), ( "ebug_sound" ) );
	SetupValue( g_Options.misc_ebug_key, 0, ("misc"), ("ebug_key"));
	SetupValue( g_Options.misc_jumpbug, false, ( "misc" ), ( "jumpbug" ) );
	SetupValue( g_Options.misc_jumpbug_bind, 0, ("misc"), ("jumpbug_bind"));
	SetupValue( g_Options.misc_bhop, false, ( "misc" ), ( "bhop" ) );
	SetupValue( g_Options.misc_faststop, false, ( "misc" ), ( "misc_faststop" ) );
	SetupValue( g_Options.misc_ljbind, false, ( "misc" ), ( "ljbind" ) );
	SetupValue( g_Options.misc_ljbindtype, 0, ( "misc" ), ( "ljbindtype" ) );
	SetupValue( g_Options.misc_voterevealer, false, ( "misc" ), ( "voterevealer" ) );
	SetupValue( g_Options.misc_antivotekick, false, ( "misc" ), ( "antivotekick" ) );
	SetupValue( g_Options.misc_snipercrosshair, false, ( "misc" ), ( "snipercrosshair" ) );
	SetupValue( g_Options.misc_keystrokes, false, ( "misc" ), ( "keystrokes" ) );
	SetupValue( g_Options.misc_nullindicator, false, ( "misc" ), ( "nullindicator" ) );
	SetupValue( g_Options.enable_foottrail, false, ( "misc" ), ( "enable_foottrail" ) );
	SetupValue( g_Options.foot_trailcolor, false, ( "misc" ), ( "enable_foottrailcolor" ) );
	SetupValue( g_Options.rainbow_velocitydraw, false, ( "misc" ), ( "rainbow_velocitydraw" ) );
	SetupValue( g_Options.misc_velocitydraw, false, ( "misc" ), ( "velocitydraw" ) );
	SetupValue( g_Options.misc_velocitygraph, false, ( "misc" ), ( "velocitygraph" ) );
	SetupValue( g_Options.misc_autostrafe, false, ( "misc" ), ( "autostrafe" ) );
	SetupValue( g_Options.misc_edgejump, false, ( "misc" ), ( "edgejump" ) );
	SetupValue( g_Options.misc_edgejump_bind, 0, ( "misc" ), ( "edgejump_bind" ) );
	SetupValue( g_Options.misc_spectatorlist, false, ( "misc" ), ( "spectatorlist" ) );
	SetupValue( g_Options.misc_engineradar, false, ( "misc" ), ( "engineradar" ) );
	SetupValue( g_Options.misc_radar, false, ( "misc" ), ( "radar" ) );
	SetupValue( g_Options.misc_radar_enemyonly, false, ( "misc" ), ( "radar_enemyonly" ) );
	SetupValue( g_Options.misc_thirdperson, false, ( "misc" ), ( "thirdperson" ) );
	SetupValue( g_Options.misc_thirdperson_bind, 0, ( "misc" ), ( "thirdperson_bind" ) );
	SetupValue( g_Options.misc_showranks, false, ( "misc" ), ( "showranks" ) );
	SetupValue( g_Options.misc_clantag, false, ("misc"), ("clantag"));
	SetupValue( g_Options.misc_watermark, true, ( "misc" ), ( "watermark" ) );
	SetupValue( g_Options.misc_desync, 0, ( "misc" ), ( "desync" ) );
	SetupValue( g_Options.misc_desync_bind, 0, ( "misc" ), ( "desync_bind" ) );
	SetupValue( g_Options.misc_thirdperson_dist, 50.f, ( "misc" ), ( "thirdperson_dist" ) );
	SetupValue( g_Options.misc_override_fov, 90, ( "misc" ), ( "override_fov" ) );
	SetupValue( g_Options.misc_viewmodel_fov, 68, ( "misc" ), ( "viewmodel_fov" ) );
	SetupValue(g_Options.movement_recorder_type, 0, ("misc"), ("movement_recorder_type"));
	SetupValue(g_Options.misc_aspectratiotoggle, false, ("misc"), ("aspect_ratiotoggle"));
	SetupValue(g_Options.misc_aspectratio, 0.f, ("misc"), ("aspect_ratio"));

	SetupValue( g_Options.misc_slowwalk_bind, false, ( "misc" ), ( "slowwalk_bind" ) );
	SetupValue( g_Options.misc_slowwalk_speed, 1.0f, ( "misc" ), ( "slowwalk_speed" ) );

	SetupValue( g_Options.fakelag_enabled, false, ( "fakelag" ), ( "enabled" ) );
	SetupValue( g_Options.fakelag_standing, false, ( "fakelag" ), ( "standing" ) );
	SetupValue( g_Options.fakelag_moving, false, ( "fakelag" ), ( "moving" ) );
	SetupValue( g_Options.fakelag_unducking, false, ( "fakelag" ), ( "unducking" ) );
	SetupValue( g_Options.fakelag_mode, 0, ( "fakelag" ), ( "mode" ) );
	SetupValue( g_Options.fakelag_factor, 0, ( "fakelag" ), ( "factor" ) );
}

void CConfig::SetupSkins() {
	//for (auto&[key, val] : k_weapon_names) {
	for (auto& key : k_weapon_names)
	{
		auto& entries = g_Options.skins.m_items;
		static auto definition_vector_index = 0;

		auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
		selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
		selected_entry.definition_vector_index = definition_vector_index;

		SetupValue(g_Options.skins.m_items[key.definition_index].name, "Default",( key.name ), ("name"));
		SetupValue(g_Options.skins.m_items[key.definition_index].enabled, 0, (key.name), ("enabled"));
		SetupValue(g_Options.skins.m_items[key.definition_index].definition_vector_index, 0, ( key.name ), ("definition_vector_index"));
		SetupValue(g_Options.skins.m_items[key.definition_index].definition_index, 1, (key.name), ("definition_index"));
		SetupValue(g_Options.skins.m_items[key.definition_index].paint_kit_vector_index, 0, (key.name), ("paint_kit_vector_index"));
		SetupValue(g_Options.skins.m_items[key.definition_index].paint_kit_index, 0, (key.name), ("paint_kit_index"));
		SetupValue(g_Options.skins.m_items[key.definition_index].definition_override_vector_index, 0, (key.name), ("definition_override_vector_index"));
		SetupValue(g_Options.skins.m_items[key.definition_index].definition_override_index, 0, (key.name), ("definition_override_index"));
		SetupValue(g_Options.skins.m_items[key.definition_index].seed, 0, (key.name), ("seed"));
		SetupValue(g_Options.skins.m_items[key.definition_index].stat_trak, 0, (key.name), ("stat_trak"));
		SetupValue(g_Options.skins.m_items[key.definition_index].wear, 0.0f, (key.name), ("wear"));

		SetupValue(g_Options.skins.m_items[key.definition_index].enabled_stickers, false, (key.name), ("enable_stickers"));
		//SetupValue(g_Options.skins.m_items[key.definition_index].stickers_place, 0, (key.name), ("stickers_place"));

		SetupSkinz(&g_Options.skins.m_items[key.definition_index].stickers[selected_entry.stickers_place].stickers_id, (key.name), ("stickers_id"));
		SetupSkinz(&g_Options.skins.m_items[key.definition_index].stickers[selected_entry.stickers_place].sticker_vector_index, (key.name), ("sticker_vector_index"));

		SetupValue(g_Options.skins.m_items[key.definition_index].custom_name, "", (key.name), ("custom_name"));
	}

	SetupValue(g_Options.skins_knifemodel, 0, ("misc"), ("knifemodel"));
	SetupValue(g_Options.enable_agentskins, false, ("misc"), ("enable_agentskins"));
	SetupValue(g_Options.skins_agentmodelct, 0, ("misc"), ("skins_agentmodelct"));
	SetupValue(g_Options.skins_agentmodeltr, 0, ("misc"), ("skins_agentmodeltr"));

	SetupValue(g_Options.enable_profilechanger, false, ("misc"), ("enable_profilechanger"));
	SetupValue(g_Options.profile_rankid, 0, ("misc"), ("profile_rankid"));
	SetupValue(g_Options.profile_wins, -1, ("misc"), ("profile_wins"));
	SetupValue(g_Options.profile_level, -1, ("misc"), ("profile_level"));
	SetupValue(g_Options.profile_exp, -1, ("misc"), ("profile_exp"));
	SetupValue(g_Options.profile_teaching, -1, ("misc"), ("profile_teaching"));
	SetupValue(g_Options.profile_friendly, -1, ("misc"), ("profile_friendly"));
	SetupValue(g_Options.profile_leader, -1, ("misc"), ("profile_leader"));
}

void CConfig::SetupColors( ) {
	SetupColor( g_Options.color_esp_ally_visible, "color_esp_ally_visible" );
	SetupColor( g_Options.color_esp_enemy_visible, "color_esp_enemy_visible" );
	SetupColor( g_Options.color_esp_ally_occluded, "color_esp_ally_occluded" );
	SetupColor( g_Options.color_esp_enemy_occluded, "color_esp_enemy_occluded" );
	SetupColor( g_Options.color_esp_enemy_occluded, "color_esp_enemy_occluded" );
	SetupColor( g_Options.color_esp_weapons, "color_esp_weapons" );
	SetupColor( g_Options.color_esp_defuse, "color_esp_defuse" );
	SetupColor( g_Options.color_esp_c4, "color_esp_c4" );
	SetupColor( g_Options.color_esp_item, "color_esp_item" );
	SetupColor( g_Options.color_glow_ally, "color_glow_ally" );
	SetupColor( g_Options.color_glow_enemy, "color_glow_enemy" );
	SetupColor( g_Options.color_glow_chickens, "color_glow_chickens" );
	SetupColor( g_Options.color_glow_c4_carrier, "color_glow_c4_carrier" );
	SetupColor( g_Options.color_glow_planted_c4, "color_glow_planted_c4" );
	SetupColor( g_Options.color_glow_defuse, "color_glow_defuse" );
	SetupColor( g_Options.color_glow_weapons, "color_glow_weapons" );
	SetupColor( g_Options.color_chams_player_ally_visible, "color_chams_player_ally_visible" );
	SetupColor( g_Options.color_chams_player_ally_occluded, "color_chams_player_ally_occluded" );
	SetupColor( g_Options.color_chams_player_enemy_visible, "color_chams_player_enemy_visible" );
	SetupColor( g_Options.color_chams_player_enemy_occluded, "color_chams_player_enemy_occluded" );
	SetupColor( g_Options.color_foot_trail, "color_foot_trail" );
	SetupColor( g_Options.color_chams_backtrack, "color_chams_backtrack" );
	SetupColor( g_Options.color_grenade_prediction, "color_grenade_prediction" );
	SetupColor( g_Options.color_skeleton_backtrack, "color_skeleton_backtrack" );
	SetupColor( g_Options.color_damage_indicator, "color_damage_indicator" );
	SetupColor( g_Options.color_esp_sounds, "color_esp_sounds" );
	SetupColor(g_Options.color_beam_molotov, "color_beam_molotov");
	SetupColor(g_Options.color_beam_decoy, "color_beam_decoy");
	SetupColor(g_Options.color_beam_smoke, "color_beam_smoke");
	SetupColor(g_Options.color_world_modulation, "color_world_modulation");
}

void CConfig::Setup( ) {
	CConfig::SetupLegit( );
	CConfig::SetupVisuals( );
	CConfig::SetupMisc( );
	CConfig::SetupSkins();
	CConfig::SetupColors( );
}

void CConfig::Save( const std::string& name ) {
	if( name.empty( ) )
		return;

	CreateDirectoryA( u8"C:\\Nirvana\\", NULL );
	std::string file = u8"C:\\Nirvana\\" + name;

	for (auto value : ints) {
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	}

	for( auto value : floats ) WritePrivateProfileStringA( value->category.c_str( ), value->name.c_str( ), std::to_string( *value->value ).c_str( ), file.c_str( ) );
	for( auto value : bools ) WritePrivateProfileStringA( value->category.c_str( ), value->name.c_str( ), *value->value ? "true" : "false", file.c_str( ) );
}

void CConfig::Load( const std::string& name ) {
	if( name.empty( ) )
		return;

	g_ClientState->ForceFullUpdate();

	CreateDirectoryA( u8"C:\\Nirvana\\", NULL );
	std::string file = u8"C:\\Nirvana\\" + name;

	char value_l[32] = { '\0' };
	for( auto value : ints ) {
		GetPrivateProfileStringA( value->category.c_str( ), value->name.c_str( ), "0", value_l, 32, file.c_str( ) ); *value->value = atoi( value_l );
	}

	for( auto value : floats ) {
		GetPrivateProfileStringA( value->category.c_str( ), value->name.c_str( ), "0.0f", value_l, 32, file.c_str( ) ); *value->value = atof( value_l );
	}

	for( auto value : bools ) {
		GetPrivateProfileStringA( value->category.c_str( ), value->name.c_str( ), "false", value_l, 32, file.c_str( ) ); *value->value = !strcmp( value_l, "true" );
	}
}*/
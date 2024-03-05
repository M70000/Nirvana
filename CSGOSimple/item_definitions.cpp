#include "item_definitions.hpp"

const std::map<size_t, weapon_info> k_weapon_info =
{
	{WEAPON_KNIFE,{"models/weapons/v_knife_default_ct.mdl", "knife_default_ct"}},
	{WEAPON_KNIFE_T,{"models/weapons/v_knife_default_t.mdl", "knife_t"}},
	{WEAPON_KNIFE_BAYONET, {"models/weapons/v_knife_bayonet.mdl", "bayonet"}},
	{WEAPON_KNIFE_FLIP, {"models/weapons/v_knife_flip.mdl", "knife_flip"}},
	{WEAPON_KNIFE_GUT, {"models/weapons/v_knife_gut.mdl", "knife_gut"}},
	{WEAPON_KNIFE_KARAMBIT, {"models/weapons/v_knife_karam.mdl", "knife_karambit"}},
	{WEAPON_KNIFE_M9_BAYONET, {"models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet"}},
	{WEAPON_KNIFE_TACTICAL, {"models/weapons/v_knife_tactical.mdl", "knife_tactical"}},
	{WEAPON_KNIFE_FALCHION, {"models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion"}},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, {"models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie"}},
	{WEAPON_KNIFE_BUTTERFLY, {"models/weapons/v_knife_butterfly.mdl", "knife_butterfly"}},
	{WEAPON_KNIFE_PUSH, {"models/weapons/v_knife_push.mdl", "knife_push"}},
	{WEAPON_KNIFE_URSUS,{"models/weapons/v_knife_ursus.mdl", "knife_ursus"}},
	{WEAPON_KNIFE_GYPSY_JACKKNIFE,{"models/weapons/v_knife_gypsy_jackknife.mdl", "knife_gypsy_jackknife"}},
	{WEAPON_KNIFE_STILETTO,{"models/weapons/v_knife_stiletto.mdl", "knife_stiletto"}},
	{WEAPON_KNIFE_WIDOWMAKER,{"models/weapons/v_knife_widowmaker.mdl", "knife_widowmaker"}},
	{WEAPON_KNIFE_CSS,{"models/weapons/v_knife_css.mdl", "knife_css"}},
	{WEAPON_KNIFE_SKELETON,{"models/weapons/v_knife_skeleton.mdl", "knife_skeleton"}},
	{WEAPON_KNIFE_OUTDOOR,{"models/weapons/v_knife_outdoor.mdl", "knife_outdoor"}},
	{WEAPON_KNIFE_CANIS,{"models/weapons/v_knife_canis.mdl", "knife_canis"}},
	{WEAPON_KNIFE_CORD,{"models/weapons/v_knife_cord.mdl", "knife_cord"}},
	{GLOVE_STUDDED_BLOODHOUND,{"models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"}},
	{GLOVE_T_SIDE,{"models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl"}},
	{GLOVE_CT_SIDE,{"models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl"}},
	{GLOVE_SPORTY,{"models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"}},
	{GLOVE_SLICK,{"models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"}},
	{GLOVE_LEATHER_WRAP,{"models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"}},
	{GLOVE_MOTORCYCLE,{"models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"}},
	{GLOVE_SPECIALIST,{"models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"}},
	{GLOVE_HYDRA,{"models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl"}},
	{GLOVE_BROKEN_FANG,{"models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_brokenfang.mdl"}}
};

const std::vector<weapon_name> k_knife_names =
{
	{0, "Default"},
	{WEAPON_KNIFE_BAYONET, "Bayonet"},
	{WEAPON_KNIFE_M9_BAYONET, "M9 Bayonet"},
	{WEAPON_KNIFE_KARAMBIT, "Karambit"},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, "Bowie Knife"},
	{WEAPON_KNIFE_BUTTERFLY, "Butterfly"},
	{WEAPON_KNIFE_FALCHION, "Falchion"},
	{WEAPON_KNIFE_FLIP, "Flip"},
	{WEAPON_KNIFE_GUT, "Gut Knife"},
	{WEAPON_KNIFE_TACTICAL, "Huntsman"},
	{WEAPON_KNIFE_PUSH, "Shadow Daggers"},
	{WEAPON_KNIFE_GYPSY_JACKKNIFE, "Navaja"},
	{WEAPON_KNIFE_STILETTO, "Stiletto"},
	{WEAPON_KNIFE_WIDOWMAKER, "Talon"},
	{WEAPON_KNIFE_URSUS, "Ursus"},
	{WEAPON_KNIFE_CSS, "Classic Knife"},
	{WEAPON_KNIFE_SKELETON, "Skeleton"},
	{WEAPON_KNIFE_OUTDOOR, "Nomad"},
	{WEAPON_KNIFE_CANIS, "Survival"},
	{WEAPON_KNIFE_CORD, "Paracord"}
};

const std::vector<weapon_name> k_glove_names =
{
	{0, "Default"},
	{GLOVE_STUDDED_BLOODHOUND, "Bloodhound"},
	{GLOVE_T_SIDE, "Default (Terrorists)"},
	{GLOVE_CT_SIDE, "Default (Counter-Terrorists)"},
	{GLOVE_SPORTY, "Sporty"},
	{GLOVE_SLICK, "Slick"},
	{GLOVE_LEATHER_WRAP, "Handwraps"},
	{GLOVE_MOTORCYCLE, "Motorcycle"},
	{GLOVE_SPECIALIST, "Specialist"},
	{GLOVE_HYDRA, "Hydra"},
	{GLOVE_BROKEN_FANG, "Broken Fang"}
};

const std::vector<weapon_name> k_weapon_names =
{
	{WEAPON_KNIFE, "Knife"},
	{GLOVE_T_SIDE, "Glove"},
	{7, "AK-47"},
	{8, "AUG"},
	{9, "AWP"},
	{63, "CZ75 Auto"},
	{1, "Desert Eagle"},
	{2, "Dual Berettas"},
	{10, "FAMAS"},
	{3, "Five-SeveN"},
	{11, "G3SG1"},
	{13, "Galil AR"},
	{4, "Glock-18"},
	{14, "M249"},
	{60, "M4A1-S"},
	{16, "M4A4"},
	{17, "MAC-10"},
	{27, "MAG-7"},
	{23, "MP5-SD"},
	{33, "MP7"},
	{34, "MP9"},
	{28, "Negev"},
	{35, "Nova"},
	{32, "P2000"},
	{36, "P250"},
	{19, "P90"},
	{26, "PP-Bizon"},
	{64, "R8 Revolver"},
	{29, "Sawed-Off"},
	{38, "SCAR-20"},
	{40, "SSG 08"},
	{39, "SG 553"},
	{30, "Tec-9"},
	{24, "UMP-45"},
	{61, "USP-S"},
	{25, "XM1014"},
};





























































































































































































































































































































































































































































































































































































































































































































































































































































































































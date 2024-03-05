#include "skins.hpp"
#include <WinInet.h>

#pragma comment(lib, "Wininet")

short itemDefinitionIndex;

enum class EStickerAttributeType
{
	Index,
	Wear,
	Scale,
	Rotation
};

static uint16_t s_iwoff = 0;

static void* o_float_fn;

static float __fastcall hooked_float_fn(void* thisptr, void*, int slot, EStickerAttributeType attribute, float fl)
{
	auto item = reinterpret_cast<C_BaseAttributableItem*>(uintptr_t(thisptr) - s_iwoff);

	switch (attribute)
	{
	case EStickerAttributeType::Wear:
		return FLT_MIN;
	case EStickerAttributeType::Scale:
		return 1.f;
	case EStickerAttributeType::Rotation:
		return 0.f;
	}

	return reinterpret_cast<decltype(hooked_float_fn)*>(o_float_fn)(thisptr, nullptr, slot, attribute, fl);
}

static void* o_uint_fn;

static unsigned int __fastcall hooked_uint_fn(void* thisptr, void*, int slot, EStickerAttributeType attribute, unsigned fl)
{
	auto item = reinterpret_cast<C_BaseAttributableItem*>(uintptr_t(thisptr) - s_iwoff);

	return g_Options.skins.m_items[itemDefinitionIndex].stickers[slot].stickers_id;

	//return reinterpret_cast<decltype(hooked_uint_fn)*>(o_uint_fn)(thisptr, nullptr, slot, attribute, fl);
}

void ApplyStickerHooks(C_BaseAttributableItem* item)
{
	if (!s_iwoff)
	{
		//s_iwoff = NetVarManager::Get().GetOffset(FnvHash("CEconEntity->m_Item")) + 0xC;
		s_iwoff = NetvarSys::Get().GetOffset("DT_EconEntity", "m_Item") + 0xC;
	}

	void**& iw_vt = *reinterpret_cast<void***>(uintptr_t(item) + s_iwoff);

	static void** iw_hook_vt = nullptr;

	if (!iw_hook_vt)
	{
		size_t len = 0;

		while (iw_vt[len])
			++len;

		iw_hook_vt = new void* [len];
		memcpy(iw_hook_vt, iw_vt, len * sizeof(void*));
		o_float_fn = iw_hook_vt[4];
		iw_hook_vt[4] = &hooked_float_fn;
		o_uint_fn = iw_hook_vt[5];
		iw_hook_vt[5] = &hooked_uint_fn;
	}

	iw_vt = iw_hook_vt;
}

static auto erase_override_if_exists_by_index( const int definition_index ) -> void {
	if( k_weapon_info.count( definition_index ) ) {
		auto& icon_override_map = g_Options.skins.m_icon_overrides;
		const auto& original_item = k_weapon_info.at( definition_index );
		if( original_item.icon && icon_override_map.count( original_item.icon ) )
			icon_override_map.erase( icon_override_map.at( original_item.icon ) ); // Remove the leftover override
	}
}
static auto apply_config_on_attributable_item( C_BaseAttributableItem* item, const item_setting* config,
	const unsigned xuid_low ) -> void {
	if( !config->enabled ) {
		return;
	}
	item->m_Item( ).m_iItemIDHigh( ) = -1;
	item->m_Item( ).m_iAccountID( ) = xuid_low;
	if( config->custom_name[0] )
		strcpy_s( item->m_Item( ).m_iCustomName( ), config->custom_name );
	if( config->paint_kit_index )
		item->m_nFallbackPaintKit( ) = config->paint_kit_index;
	if( config->seed )
		item->m_nFallbackSeed( ) = config->seed;
	if( config->stat_trak ) {
		item->m_nFallbackStatTrak( ) = config->stat_trak;
		item->m_Item( ).m_iEntityQuality( ) = 9;
	} else {
		item->m_Item( ).m_iEntityQuality( ) = is_knife( config->definition_index ) ? 3 : 0;
	}
	item->m_flFallbackWear( ) = config->wear;
	auto& definition_index = item->m_Item( ).m_iItemDefinitionIndex( );
	auto& icon_override_map = g_Options.skins.m_icon_overrides;
	if( config->definition_override_index && config->definition_override_index != definition_index && k_weapon_info.count( config->definition_override_index ) ) {
		const auto old_definition_index = definition_index;
		definition_index = config->definition_override_index;
		const auto& replacement_item = k_weapon_info.at( config->definition_override_index );
		item->m_nModelIndex( ) = g_MdlInfo->GetModelIndex( replacement_item.model );
		item->SetModelIndex( g_MdlInfo->GetModelIndex( replacement_item.model ) );
		item->GetClientNetworkable( )->PreDataUpdate( 0 );
		if( old_definition_index && k_weapon_info.count( old_definition_index ) ) {
			const auto& original_item = k_weapon_info.at( old_definition_index );
			if( original_item.icon && replacement_item.icon ) {
				icon_override_map[original_item.icon] = replacement_item.icon;
			}
		}
	} 
	else 
	{
		erase_override_if_exists_by_index( definition_index );
	}

	if (is_knife(config->definition_index) || !config->enabled_stickers)
		return;

	itemDefinitionIndex = item->m_Item().m_iItemDefinitionIndex();

	ApplyStickerHooks(item);

}
static auto get_wearable_create_fn( ) -> CreateClientClassFn {
	auto clazz = g_CHLClient->GetAllClasses( );
	// Please, if you gonna paste it into a cheat use classids here. I use names because they
	// won't change in the foreseeable future and i dont need high speed, but chances are
	// you already have classids, so use them instead, they are faster.
	while( strcmp( clazz->m_pNetworkName, "CEconWearable" ) )
		clazz = clazz->m_pNext;
	return clazz->m_pCreateFn;
}

std::add_pointer_t<const char** __FASTCALL(const char* playerModelName)> getPlayerViewmodelArmConfigForPlayerModel;

void Skins::AgentChanger(ClientFrameStage_t stage) noexcept
{
	/*if (stage != ClientFrameStage_t::FRAME_START && stage != ClientFrameStage_t::FRAME_RENDER_END)
		return;*/

	if (!g_Options.enable_agentskins)
		return;

	static int originalIdx = 0;

	if (!g_LocalPlayer) {
		originalIdx = 0;
		return;
	}

	constexpr auto getModel = [](int team) constexpr noexcept -> const char* 
	{
		constexpr std::array modelsTR
		{
			"models/player/custom_player/legacy/tm_professional_varj.mdl", // Getaway Sally | The Professionals
			"models/player/custom_player/legacy/tm_professional_vari.mdl", // Number K | The Professionals
			"models/player/custom_player/legacy/tm_professional_varh.mdl", // Little Kev | The Professionals
			"models/player/custom_player/legacy/tm_professional_varg.mdl", // Safecracker Voltzmann | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf5.mdl", // Bloody Darryl The Strapped | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf4.mdl", // Sir Bloody Loudmouth Darryl | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf3.mdl", // Sir Bloody Darryl Royale | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf2.mdl", // Sir Bloody Skullhead Darryl | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf1.mdl", // Sir Bloody Silent Darryl | The Professionals
			"models/player/custom_player/legacy/tm_professional_varf.mdl", // Sir Bloody Miami Darryl | The Professionals
			"models/player/custom_player/legacy/tm_phoenix_varianti.mdl", // Street Soldier | Phoenix
			"models/player/custom_player/legacy/tm_phoenix_varianth.mdl", // Soldier | Phoenix
			"models/player/custom_player/legacy/tm_phoenix_variantg.mdl", // Slingshot | Phoenix
			"models/player/custom_player/legacy/tm_phoenix_variantf.mdl", // Enforcer | Phoenix
			"models/player/custom_player/legacy/tm_leet_variantj.mdl", // Mr. Muhlik | Elite Crew
			"models/player/custom_player/legacy/tm_leet_varianti.mdl", // Prof. Shahmat | Elite Crew
			"models/player/custom_player/legacy/tm_leet_varianth.mdl", // Osiris | Elite Crew
			"models/player/custom_player/legacy/tm_leet_variantg.mdl", // Ground Rebel | Elite Crew
			"models/player/custom_player/legacy/tm_leet_variantf.mdl", // The Elite Mr. Muhlik | Elite Crew
			"models/player/custom_player/legacy/tm_jungle_raider_variantf2.mdl", // Trapper | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantf.mdl", // Trapper Aggressor | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variante.mdl", // Vypa Sista of the Revolution | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantd.mdl", // Col. Mangos Dabisi | Guerrilla Warfare
			//"models/player/custom_player/legacy/tm_jungle_raider_variantc.mdl", // Arno The Overgrown | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantb2.mdl", // 'Medium Rare' Crasswater | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_variantb.mdl", // Crasswater The Forgotten | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_jungle_raider_varianta.mdl", // Elite Trapper Solman | Guerrilla Warfare
			"models/player/custom_player/legacy/tm_balkan_varianth.mdl", // 'The Doctor' Romanov | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantj.mdl", // Blackwolf | Sabre
			"models/player/custom_player/legacy/tm_balkan_varianti.mdl", // Maximus | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantf.mdl", // Dragomir | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantg.mdl", // Rezan The Ready | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantk.mdl", // Rezan the Redshirt | Sabre
			"models/player/custom_player/legacy/tm_balkan_variantl.mdl" // Dragomir | Sabre Footsoldier
		};

		constexpr std::array modelsCT
		{
			"models/player/custom_player/legacy/ctm_diver_varianta.mdl", // Cmdr. Davida 'Goggles' Fernandez | SEAL Frogman
			"models/player/custom_player/legacy/ctm_diver_variantb.mdl", // Cmdr. Frank 'Wet Sox' Baroud | SEAL Frogman
			"models/player/custom_player/legacy/ctm_diver_variantc.mdl", // Lieutenant Rex Krikey | SEAL Frogman
			"models/player/custom_player/legacy/ctm_fbi_varianth.mdl", // Michael Syfers | FBI Sniper
			"models/player/custom_player/legacy/ctm_fbi_variantf.mdl", // Operator | FBI SWAT
			"models/player/custom_player/legacy/ctm_fbi_variantb.mdl", // Special Agent Ava | FBI
			"models/player/custom_player/legacy/ctm_fbi_variantg.mdl", // Markus Delrow | FBI HRT
			"models/player/custom_player/legacy/ctm_gendarmerie_varianta.mdl", // Sous-Lieutenant Medic | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variantb.mdl", // Chem-Haz Capitaine | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variantc.mdl", // Chef d'Escadron Rouchard | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variantd.mdl", // Aspirant | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_gendarmerie_variante.mdl", // Officer Jacques Beltram | Gendarmerie Nationale
			"models/player/custom_player/legacy/ctm_sas_variantg.mdl", // D Squadron Officer | NZSAS
			"models/player/custom_player/legacy/ctm_sas_variantf.mdl", // B Squadron Officer | SAS
			"models/player/custom_player/legacy/ctm_st6_variante.mdl", // Seal Team 6 Soldier | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_variantg.mdl", // Buckshot | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_varianti.mdl", // Lt. Commander Ricksaw | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_variantj.mdl", // 'Blueberries' Buckshot | NSWC SEAL
			"models/player/custom_player/legacy/ctm_st6_variantk.mdl", // 3rd Commando Company | KSK
			"models/player/custom_player/legacy/ctm_st6_variantl.mdl", // 'Two Times' McCoy | TACP Cavalry
			"models/player/custom_player/legacy/ctm_st6_variantm.mdl", // 'Two Times' McCoy | USAF TACP
			"models/player/custom_player/legacy/ctm_st6_variantn.mdl", // Primeiro Tenente | Brazilian 1st Battalion
			"models/player/custom_player/legacy/ctm_swat_variante.mdl", // Cmdr. Mae 'Dead Cold' Jamison | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantf.mdl", // 1st Lieutenant Farlow | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantg.mdl", // John 'Van Healen' Kask | SWAT
			"models/player/custom_player/legacy/ctm_swat_varianth.mdl", // Bio-Haz Specialist | SWAT
			"models/player/custom_player/legacy/ctm_swat_varianti.mdl", // Sergeant Bombson | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantj.mdl", // Chem-Haz Specialist | SWAT
			"models/player/custom_player/legacy/ctm_swat_variantk.mdl" // Lieutenant 'Tree Hugger' Farlow | SWAT
		};

		switch (team) {
		case 2: return static_cast<std::size_t>(g_Options.skins_agentmodeltr - 1) < modelsTR.size() ? modelsTR[g_Options.skins_agentmodeltr - 1] : nullptr;
		case 3: return static_cast<std::size_t>(g_Options.skins_agentmodelct - 1) < modelsCT.size() ? modelsCT[g_Options.skins_agentmodelct - 1] : nullptr;
		default: return nullptr;
		}
	};

	if (const auto model = getModel(g_LocalPlayer->m_iTeamNum())) {
		if (stage == ClientFrameStage_t::FRAME_RENDER_START) {
			originalIdx = g_LocalPlayer->m_nModelIndex();
			if (const auto modelprecache = networkStringTableContainer->findTable("modelprecache")) {
				modelprecache->addString(false, model);
				const auto viewmodelArmConfig = getPlayerViewmodelArmConfigForPlayerModel(model);
				modelprecache->addString(false, viewmodelArmConfig[2]);
				modelprecache->addString(false, viewmodelArmConfig[3]);
			}
		}

		const auto idx = stage == ClientFrameStage_t::FRAME_RENDER_END && originalIdx ? originalIdx : g_MdlInfo->GetModelIndex(model);

		g_LocalPlayer->SetModelIndex(idx);

		if (const auto ragdoll = g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->ragdoll()))
			ragdoll->SetModelIndex(idx);
	}
}

void Skins::OnFrameStageNotify( bool frame_end ) {
	const auto local_index = g_EngineClient->GetLocalPlayer( );
	const auto local = static_cast< C_BasePlayer* >( g_EntityList->GetClientEntity( local_index ) );
	if( !local )
		return;

	player_info_t player_info;
	if( !g_EngineClient->GetPlayerInfo( local_index, &player_info ) )
		return;

	if( frame_end ) {
		const auto wearables = local->m_hMyWearables( );
		const auto glove_config = &g_Options.skins.m_items[GLOVE_T_SIDE];

		static auto glove_handle = CBaseHandle( 0 );
		auto glove = reinterpret_cast< C_BaseAttributableItem* >( g_EntityList->GetClientEntityFromHandle( wearables[0] ) );
		if( !glove ) {
			const auto our_glove = reinterpret_cast< C_BaseAttributableItem* >( g_EntityList->GetClientEntityFromHandle( glove_handle ) );
			if( our_glove ) // Our glove still exists
			{
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}

		if( !local->IsAlive( ) ) {
			if( glove ) {
				glove->GetClientNetworkable( )->SetDestroyedOnRecreateEntities( );
				glove->GetClientNetworkable( )->Release( );
			}
			return;
		}

		if( glove_config && glove_config->definition_override_index ) {
			if( !glove ) {
				static auto create_wearable_fn = get_wearable_create_fn( );
				const auto entry = g_EntityList->GetHighestEntityIndex( ) + 1;
				const auto serial = rand( ) % 0x1000;

				//glove = static_cast<C_BaseAttributableItem*>(create_wearable_fn(entry, serial));
				create_wearable_fn( entry, serial );
				glove = reinterpret_cast< C_BaseAttributableItem* >( g_EntityList->GetClientEntity( entry ) );

				assert( glove );
				{
					static auto set_abs_origin_addr = Utils::PatternScan( GetModuleHandle( L"client.dll" ), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8" );
					const auto set_abs_origin_fn = reinterpret_cast< void( __thiscall* )( void*, const std::array<float, 3>& ) >( set_abs_origin_addr );
					static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
					set_abs_origin_fn( glove, new_pos );
				}

				const auto wearable_handle = reinterpret_cast< CBaseHandle* >( &wearables[0] );
				*wearable_handle = entry | serial << 16;
				glove_handle = wearables[0];
			}

			// Thanks, Beakers
			glove->SetGloveModelIndex( -1 );
			apply_config_on_attributable_item( glove, glove_config, player_info.xuid_low );
		}
	} else {
		auto weapons = local->m_hMyWeapons( );
		for( int i = 0; weapons[i].IsValid( ); i++ ) {
			C_BaseAttributableItem *weapon = ( C_BaseAttributableItem* )g_EntityList->GetClientEntityFromHandle( weapons[i] );
			if( !weapon )
				continue;
			auto& definition_index = weapon->m_Item( ).m_iItemDefinitionIndex( );
			const auto active_conf = &g_Options.skins.m_items[is_knife( definition_index ) ? WEAPON_KNIFE : definition_index];
			apply_config_on_attributable_item( weapon, active_conf, player_info.xuid_low );
		}
		const auto view_model_handle = local->m_hViewModel( );
		if( !view_model_handle.IsValid( ) )
			return;

		const auto view_model = static_cast< C_BaseViewModel* >( g_EntityList->GetClientEntityFromHandle( view_model_handle ) );
		if( !view_model )
			return;

		const auto view_model_weapon_handle = view_model->m_hWeapon( );
		if( !view_model_weapon_handle.IsValid( ) )
			return;

		const auto view_model_weapon = static_cast< C_BaseCombatWeapon* >( g_EntityList->GetClientEntityFromHandle( view_model_weapon_handle ) );
		if( !view_model_weapon )
			return;

		if( k_weapon_info.count( view_model_weapon->m_Item( ).m_iItemDefinitionIndex( ) ) ) {
			const auto override_model = k_weapon_info.at( view_model_weapon->m_Item( ).m_iItemDefinitionIndex( ) ).model;
			auto override_model_index = g_MdlInfo->GetModelIndex( override_model );
			view_model->m_nModelIndex( ) = override_model_index;
			auto world_model_handle = view_model_weapon->m_hWeaponWorldModel( );
			if( !world_model_handle.IsValid( ) )
				return;
			const auto world_model = static_cast< C_BaseWeaponWorldModel* >( g_EntityList->GetClientEntityFromHandle( world_model_handle ) );
			if( !world_model )
				return;
			world_model->m_nModelIndex( ) = override_model_index + 1;
		}
	}
}

namespace Preview
{
	std::string FindUrl(std::string paintName, std::string weapon)
	{
		std::ifstream inFile;
		inFile.open(".\\csgo\\scripts\\items\\items_game_cdn.txt");
		std::string line;
		std::string search = paintName.append("=");
		while (std::getline(inFile, line))
		{
			if (line.find(search, 0) != std::string::npos)
			{
				if (line.find(weapon, 0) != std::string::npos)
				{
					return line.substr(line.find(search)).erase(0, search.length());
				}
			}
		}
		return "nf";
	}

	std::string DownloadBytes(const char* const szUrl)
	{
		HINTERNET hOpen = NULL;
		HINTERNET hFile = NULL;
		char* lpBuffer = NULL;
		DWORD dwBytesRead = 0;
		char* data = 0;
		DWORD dataSize = 0;

		hOpen = InternetOpenA("Nirvana", NULL, NULL, NULL, NULL);
		if (!hOpen) return "";

		hFile = InternetOpenUrlA(hOpen, szUrl, NULL, NULL, INTERNET_FLAG_RELOAD, NULL);

		if (!hFile) {
			InternetCloseHandle(hOpen);
			return "";
		}

		std::string output;
		do {
			char buffer[2000];
			InternetReadFile(hFile, (LPVOID)buffer, _countof(buffer), &dwBytesRead);
			output.append(buffer, dwBytesRead);
		} while (dwBytesRead);

		InternetCloseHandle(hFile);
		InternetCloseHandle(hOpen);

		return output;
	}
}





























































































































































































































































































































































































































































































































































































































































































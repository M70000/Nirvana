#pragma once

#include "sdk.hpp"
#include <array>
#include "../helpers/utils.hpp"

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
    }

#define PNETVAR(type, name, table, netvar)                           \
    type* name##() const {                                          \
        static int _##name = NetvarSys::Get().GetOffset(table, netvar);     \
        return (type*)((uintptr_t)this + _##name);                 \
    }


#define NETVAR_EX( type, name, table, netvar, extra )                         \
  type& name##() const {                                                      \
    static int _##name = NetvarSys::Get().GetOffset( table, netvar ) + extra; \
    return *(type*)( (uintptr_t)this + _##name );                             \
  }

#define OFFSET(type, name, offset)\
type &name##() const\
{\
        return *(type*)(uintptr_t(this) + offset);\
}

#define NETPROP(name, table, netvar) static RecvProp* name() \
{ \
	static auto prop_ptr = NetvarSys::Get().GetNetvarProp(table,netvar); \
	return prop_ptr; \
}

struct datamap_t;
class AnimationLayer;
class CBasePlayerAnimState;
class CCSGOPlayerAnimState;
class C_BaseEntity;

enum CSWeaponType
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

class C_BaseEntity;

// Created with ReClass.NET by KN4CK3R
class CHudTexture
{
public:
	char szShortName[64];    //0x0000
	char szTextureFile[64];  //0x0040
	bool bRenderUsingFont;   //0x0080
	bool bPrecached;         //0x0081
	int8_t cCharacterInFont; //0x0082
	uint8_t pad_0083[1];     //0x0083
	uint32_t hFont;          //0x0084
	int32_t iTextureId;      //0x0088
	float afTexCoords[4];    //0x008C
	uint8_t pad_009C[16];    //0x009C
};

class C_EconItemView
{
private:
	using str_32 = char[32];
public:
	NETVAR( int32_t, m_bInitialized, "DT_BaseAttributableItem", "m_bInitialized" );
	NETVAR( int16_t, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex" );
	NETVAR( int32_t, m_iEntityLevel, "DT_BaseAttributableItem", "m_iEntityLevel" );
	NETVAR( int32_t, m_iAccountID, "DT_BaseAttributableItem", "m_iAccountID" );
	NETVAR( int32_t, m_iItemIDLow, "DT_BaseAttributableItem", "m_iItemIDLow" );
	NETVAR( int32_t, m_iItemIDHigh, "DT_BaseAttributableItem", "m_iItemIDHigh" );
	NETVAR( int32_t, m_iEntityQuality, "DT_BaseAttributableItem", "m_iEntityQuality" );
	NETVAR( str_32, m_iCustomName, "DT_BaseAttributableItem", "m_szCustomName" );
};

class C_BaseEntity : public IClientEntity
{
public:
	datamap_t * GetDataDescMap( ) {
		typedef datamap_t*( __thiscall *o_GetPredDescMap )( void* );
		return CallVFunction<o_GetPredDescMap>( this, 15 )( this );
	}

	datamap_t *GetPredDescMap( ) {
		typedef datamap_t*( __thiscall *o_GetPredDescMap )( void* );
		return CallVFunction<o_GetPredDescMap>( this, 17 )( this );
	}
	static __forceinline C_BaseEntity* GetEntityByIndex( int index ) {
		return static_cast< C_BaseEntity* >( g_EntityList->GetClientEntity( index ) );
	}

	Vector GetBonePositiion(int bone) {
		matrix3x4_t bone_matrices[128];
		if (SetupBones(bone_matrices, 128, 256, 0.0f))
			return Vector{ bone_matrices[bone][0][3], bone_matrices[bone][1][3], bone_matrices[bone][2][3] };
		else
			return Vector{ };
	}

	Vector GetHeadPos()
	{
		return this->GetBonePositiion(6);
	}

	static __forceinline C_BaseEntity* get_entity_from_handle( CBaseHandle h ) {
		return static_cast< C_BaseEntity* >( g_EntityList->GetClientEntityFromHandle( h ) );
	}

	NETVAR( int32_t, m_nModelIndex, "DT_BaseEntity", "m_nModelIndex" );
	NETVAR( int32_t, m_iTeamNum, "DT_BaseEntity", "m_iTeamNum" );
	NETVAR(Vector, m_angAbsOrigin, "DT_BaseEntity", "m_angAbsOrigin");
	NETVAR( Vector, m_vecOrigin, "DT_BaseEntity", "m_vecOrigin" );
	NETVAR( Vector, m_vecAngles, "DT_BaseEntity", "m_vecAngles" );
	NETVAR( bool, m_bShouldGlow, "DT_DynamicProp", "m_bShouldGlow" );
	NETVAR( CHandle<C_BasePlayer>, m_hOwnerEntity, "DT_BaseEntity", "m_hOwnerEntity" );
	NETVAR( bool, m_bSpotted, "DT_BaseEntity", "m_bSpotted" );
	NETVAR( bool, m_bSpottedByMask, "DT_BaseEntity", "m_bSpottedByMask" );
	NETVAR( float_t, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow" );
	NETVAR(CHandle<C_BasePlayer>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser");
	NETVAR( bool, m_bStartedArming, "DT_WeaponC4", "m_bStartedArming");
	NETVAR(int32_t, m_nBombSite, "DT_PlantedC4", "m_nBombSite");
	NETVAR(float_t, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown");
	NETVAR(bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused");
	NETVAR(float_t, m_flTimerLength, "DT_PlantedC4", "m_flTimerLength");
	NETVAR(bool, ragdoll, "DT_CSPlayer", "m_hRagdoll");
	NETVAR(bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity");
	NETVAR(bool, hasHelmet, "DT_CSPlayer", "m_bHasHelmet");
	NETVAR(int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue");
	NETVAR(int32_t, m_iHealth, "DT_BasePlayer", "m_iHealth");

	void SetModelIndex(const int index)
	{
		return CallVFunction<void(__thiscall*)(C_BaseEntity*, int)>(this, 75)(this, index);
	}

	const matrix3x4_t& m_rgflCoordinateFrame( ) {
		static auto _m_rgflCoordinateFrame = NetvarSys::Get( ).GetOffset( "DT_BaseEntity", "m_CollisionGroup" ) - 0x30;
		return *( matrix3x4_t* )( ( uintptr_t )this + _m_rgflCoordinateFrame );
	}

	bool IsPlayer( );
	bool IsLoot( );
	bool IsWeapon( );
	bool IsPlantedC4( );
	bool IsDefuseKit( );
	//bool isSpotted();
};

class C_PlantedC4
{
public:
	NETVAR( bool, m_bBombTicking, "DT_PlantedC4", "m_bBombTicking" );
	NETVAR( bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused" );
	NETVAR( float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow" );
	NETVAR( float, m_flTimerLength, "DT_PlantedC4", "m_flTimerLength" );
	NETVAR(int32_t, m_nBombSite, "DT_PlantedC4", "m_nBombSite");
	NETVAR( float, m_flDefuseLength, "DT_PlantedC4", "m_flDefuseLength" );
	NETVAR( float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown" );
	NETVAR( CHandle<C_BasePlayer>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser" );
};

struct C_InfernoData : public C_BaseEntity
{
	OFFSET(float, GetSpawnTime, 0x20);

	static float GetExpiryTime()
	{
		return 7.03125f;
	}
};

class C_BaseAttributableItem : public C_BaseEntity
{
public:
	NETVAR( uint64_t, m_OriginalOwnerXuid, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	NETVAR( int32_t, m_OriginalOwnerXuidLow, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	NETVAR( int32_t, m_OriginalOwnerXuidHigh, "DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh" );
	NETVAR( int32_t, m_nFallbackStatTrak, "DT_BaseAttributableItem", "m_nFallbackStatTrak" );
	NETVAR( int32_t, m_nFallbackPaintKit, "DT_BaseAttributableItem", "m_nFallbackPaintKit" );
	NETVAR( int32_t, m_nFallbackSeed, "DT_BaseAttributableItem", "m_nFallbackSeed" );
	NETVAR( float_t, m_flFallbackWear, "DT_BaseAttributableItem", "m_flFallbackWear" );

	NETVAR( C_EconItemView, m_Item2, "DT_BaseAttributableItem", "m_Item" );

	short m_iItemDefinitionIndex()
	{
		if (!this)
			return 0;
		return *reinterpret_cast<short*>(uintptr_t(this) + /*0x2FAA*/0x2FBA);
	}

	C_EconItemView& m_Item( ) {
		// Cheating. It should be this + m_Item netvar but then the netvars inside C_EconItemView wont work properly.
		// A real fix for this requires a rewrite of the netvar manager
		return *( C_EconItemView* )this;
	}
	void SetGloveModelIndex( int modelIndex );

	void SetModelIndex(const int index)
	{
		return CallVFunction<void(__thiscall*)(C_BaseEntity*, int)>(this, 75)(this, index);
	}

};

class C_BaseWeaponWorldModel : public C_BaseEntity
{
public:
	NETVAR( int32_t, m_nModelIndex, "DT_BaseWeaponWorldModel", "m_nModelIndex" );
};

class CBoneAccessor
{

public:

	inline matrix3x4_t* GetBoneArrayForWrite()
	{
		return m_pBones;
	}

	inline void SetBoneArrayForWrite(matrix3x4_t* bone_array)
	{
		m_pBones = bone_array;
	}

	alignas(16) matrix3x4_t* m_pBones;
	int32_t m_ReadableBones; // Which bones can be read.
	int32_t m_WritableBones; // Which bones can be written.
};

class C_BaseCombatWeapon : public C_BaseAttributableItem
{
public:
	NETVAR( float_t, m_flNextPrimaryAttack, "DT_BaseCombatWeapon", "m_flNextPrimaryAttack" );
	NETVAR( float_t, m_flNextSecondaryAttack, "DT_BaseCombatWeapon", "m_flNextSecondaryAttack" );
	NETVAR( int32_t, m_iClip1, "DT_BaseCombatWeapon", "m_iClip1" );
	NETVAR( int32_t, m_iClip2, "DT_BaseCombatWeapon", "m_iClip2" );
	NETVAR( float_t, m_flRecoilIndex, "DT_WeaponCSBase", "m_flRecoilIndex" );
	NETVAR( int32_t, m_iViewModelIndex, "DT_BaseCombatWeapon", "m_iViewModelIndex" );
	NETVAR( int32_t, m_iWorldModelIndex, "DT_BaseCombatWeapon", "m_iWorldModelIndex" );
	NETVAR( int32_t, m_iWorldDroppedModelIndex, "DT_BaseCombatWeapon", "m_iWorldDroppedModelIndex" );
	NETVAR( bool, m_bPinPulled, "DT_BaseCSGrenade", "m_bPinPulled" );
	NETVAR( float_t, m_fThrowTime, "DT_BaseCSGrenade", "m_fThrowTime" );
	NETVAR( float_t, m_flPostponeFireReadyTime, "DT_BaseCombatWeapon", "m_flPostponeFireReadyTime" );
	NETVAR( CHandle<C_BaseWeaponWorldModel>, m_hWeaponWorldModel, "DT_BaseCombatWeapon", "m_hWeaponWorldModel" );

	auto isFullAuto() noexcept
	{
		const auto weaponData = GetCSWeaponData();
		if (weaponData)
			return weaponData->bFullAuto;
		return false;
	}

	NETVAR( int32_t, m_weaponMode, "DT_WeaponCSBase", "m_weaponMode" );
	NETVAR( int32_t, m_zoomLevel, "DT_WeaponCSBaseGun", "m_zoomLevel" );

	std::string WeaponIconDefinition() {
		if (!this)
			return ("");
		int id = this->m_iItemDefinitionIndex();
		switch (id) {
		case WEAPON_KNIFE_T:
			return ("[");
		case WEAPON_DEAGLE:
			return ("A");
		case WEAPON_AUG:
			return ("U");
		case WEAPON_G3SG1:
			return ("X");
		case WEAPON_MAC10:
			return ("K");
		case WEAPON_P90:
			return ("P");
		case WEAPON_SSG08:
			return ("a");
		case WEAPON_SCAR20:
			return ("Y");
		case WEAPON_UMP45:
			return ("L");
		case WEAPON_ELITE:
			return ("B");
		case WEAPON_FAMAS:
			return ("R");
		case WEAPON_FIVESEVEN:
			return ("C");
		case WEAPON_GALILAR:
			return ("Q");
		case WEAPON_M4A1_SILENCER:
			return ("T");
		case WEAPON_M4A1:
			return ("S");
		case WEAPON_P250:
			return ("F");
		case WEAPON_M249:
			return ("g");
		case WEAPON_XM1014:
			return ("b");
		case WEAPON_GLOCK:
			return ("D");
		case WEAPON_USP_SILENCER:
			return ("G");
		case WEAPON_HKP2000:
			return ("E");
		case WEAPON_AK47:
			return ("W");
		case WEAPON_AWP:
			return ("Z");
		case WEAPON_BIZON:
			return ("M");
		case WEAPON_MAG7:
			return ("d");
		case WEAPON_NEGEV:
			return ("f");
		case WEAPON_SAWEDOFF:
			return ("c");
		case WEAPON_TEC9:
			return ("H");
		case WEAPON_TASER:
			return ("h");
		case WEAPON_NOVA:
			return ("e");
		case WEAPON_CZ75A:
			return ("I");
		case WEAPON_SG556:
			return ("V");
		case WEAPON_REVOLVER:
			return ("J");
		case WEAPON_MP7:
			return ("N");
		case WEAPON_MP9:
			return ("O");
		case WEAPON_MP5:  //same icon as ump
			return ("L");
		case WEAPON_C4:
			return ("o");
		case WEAPON_FRAG_GRENADE:
			return ("j");
		case WEAPON_SMOKEGRENADE:
			return ("k");
		case WEAPON_MOLOTOV:
			return ("l");
		case WEAPON_INCGRENADE:
			return ("n");
		case WEAPON_FLASHBANG:
			return ("i");
		case WEAPON_DECOY:
			return ("m");
		default:
			return ("]");
		}
		return ("");
	}

	CCSWeaponInfo* GetCSWeaponData( );
	bool HasBullets( );
	bool CanFire( );
	bool IsGrenade( );
	bool IsKnife( );
	bool IsZeus( );
	bool IsReloading( );
	bool IsRifle( );
	bool IsPistol( );
	bool IsSniper( );
	bool IsGun( );
	float GetInaccuracy( );
	float GetSpread( );
	void UpdateAccuracyPenalty( );
	CUtlVector<IRefCounted*>& m_CustomMaterials( );
	bool* m_bCustomMaterialInitialized( );

};

#define member_func_args(...) (this, __VA_ARGS__ ); }
#define vfunc(index, func, sig) auto func { return reinterpret_cast<sig>((*(uint32_t**)this)[index]) member_func_args
class IKContext
{
public:
	void init(studiohdr_t* hdr, Vector& angles, Vector& origin, float curtime, int framecount, int boneMask);
	void update_targets(Vector* pos, quaternion* q, matrix3x4_t* bone_array, byte* computed);
	void solve_dependencies(Vector* pos, quaternion* q, matrix3x4_t* bone_array, byte* computed);

	void clear_targets()
	{
		/*
		08.10.2019
		if ( (unsigned __int8)sub_101C9820(v53) || (*(_DWORD *)(v7 + 236) >> 3) & 1 )
		{
			  v56 = 0;
			  if ( *(_DWORD *)(v105 + 4080) > 0 )
			  {
				   v57 = (_DWORD *)(v105 + 208);
				   do
				   {
					   *v57 = -9999;
						v57 += 85;
						++v56;
					}
					while ( v56 < *(_DWORD *)(v105 + 4080) );
				  }
				}
		}
		*/
		auto v56 = 0;
		if (*(int*)((DWORD)this + 4080) > 0)
		{
			auto v57 = (int*)((DWORD)this + 208);
			do
			{
				*v57 = -9999;
				v57 += 85;
				++v56;
			} while (v56 < *(int*)((DWORD)this + 4080));
		}
	}
};

class C_BasePlayer : public C_BaseEntity
{
public:
	static __forceinline C_BasePlayer* GetPlayerByUserId( int id ) {
		return static_cast< C_BasePlayer* >( GetEntityByIndex( g_EngineClient->GetPlayerForUserID( id ) ) );
	}
	static __forceinline C_BasePlayer* GetPlayerByIndex( int i ) {
		return static_cast< C_BasePlayer* >( GetEntityByIndex( i ) );
	}
	void* Animating() {
		return reinterpret_cast<void*>(uintptr_t(this) + 0x4);
	}

	NETVAR( bool, waitForNoAttack, "DT_CSPlayer", "m_bWaitForNoAttack" );
	NETVAR( bool, m_bHasDefuser, "DT_CSPlayer", "m_bHasDefuser" );
	NETVAR( bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity" );
	NETVAR( int32_t, m_iShotsFired, "DT_CSPlayer", "m_iShotsFired" );
	NETVAR( QAngle, m_angEyeAngles, "DT_CSPlayer", "m_angEyeAngles[0]" );
	NETVAR( int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue" );
	NETVAR( bool, m_bHasHeavyArmor, "DT_CSPlayer", "m_bHasHeavyArmor" );
	NETVAR( bool, m_bHasHelmet, "DT_CSPlayer", "m_bHasHelmet" );
	NETVAR( bool, m_bIsScoped, "DT_CSPlayer", "m_bIsScoped" );;
	NETVAR( float, m_flLowerBodyYawTarget, "DT_CSPlayer", "m_flLowerBodyYawTarget" );
	NETVAR( int32_t, m_iHealth, "DT_BasePlayer", "m_iHealth" );
	NETVAR( int32_t, m_lifeState, "DT_BasePlayer", "m_lifeState" );
	NETVAR( int32_t, m_fFlags, "DT_BasePlayer", "m_fFlags" );
	NETVAR( float, health_boost_time, "DT_CSPlayer", "m_flHealthShotBoostExpirationTime");
	NETVAR( int32_t, m_nTickBase, "DT_BasePlayer", "m_nTickBase" );
	NETVAR( Vector, m_vecViewOffset, "DT_BasePlayer", "m_vecViewOffset[0]" );
	NETVAR( QAngle, m_viewPunchAngle, "DT_BasePlayer", "m_viewPunchAngle" );
	NETVAR( QAngle, m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle" );
	NETVAR( Vector, m_aimPunchAngle1337, "DT_BasePlayer", "m_aimPunchAngle" );
	NETVAR( CHandle<C_BaseViewModel>, m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]" );
	NETVAR( Vector, m_vecVelocity, "DT_BasePlayer", "m_vecVelocity[0]" );
	NETVAR( float, m_flMaxspeed, "DT_BasePlayer", "m_flMaxspeed" );
	NETVAR( CHandle<C_BasePlayer>, m_hObserverTarget, "DT_BasePlayer", "m_hObserverTarget" );
	NETVAR(CHandle<C_BasePlayer>, m_hGroundEntity, "DT_BasePlayer", "m_hGroundEntity");
	NETVAR( float, m_flFlashMaxAlpha, "DT_CSPlayer", "m_flFlashMaxAlpha" );
	NETVAR( int32_t, m_nHitboxSet, "DT_BaseAnimating", "m_nHitboxSet" );
	NETVAR( CHandle<C_BaseCombatWeapon>, m_hActiveWeapon, "DT_BaseCombatCharacter", "m_hActiveWeapon" );
	NETVAR( int32_t, m_iAccount, "DT_CSPlayer", "m_iAccount" );
	NETVAR( float, m_flFlashDuration, "DT_CSPlayer", "m_flFlashDuration" );
	NETVAR( float, m_flSimulationTime, "DT_BaseEntity", "m_flSimulationTime" );
	NETVAR( float, m_flCycle, "DT_ServerAnimationData", "m_flCycle" );
	NETVAR( int, m_nSequence, "DT_BaseViewModel", "m_nSequence" );
	PNETVAR( char, m_szLastPlaceName, "DT_BasePlayer", "m_szLastPlaceName" );
	NETPROP( m_flLowerBodyYawTargetProp, "DT_CSPlayer", "m_flLowerBodyYawTarget" );
	NETVAR( int32_t, m_iFOV, "DT_BasePlayer", "m_iFOV" );
	NETVAR( int32_t, m_iDefaultFOV, "DT_BasePlayer", "m_iDefaultFOV" );
	/*NETVAR( int, m_plKills, "DT_CSPlayer", "m_iMatchStats_Kills_Total")
	NETVAR( int, m_plDeaths, "DT_CSPlayer", "m_iMatchStats_Deaths_Total ")*/

	NETVAR(bool, m_bIsDefusing, "DT_CSPlayer", "m_bIsDefusing");
	NETVAR(bool, m_bIsGrabbingHostage, "DT_CSPlayer", "m_bIsGrabbingHostage");
	NETVAR(CHandle<C_BasePlayer>, m_hCarriedHostage, "DT_CSPlayer", "m_hCarriedHostage");

	NETVAR(int, m_nSurvivalTeam, "DT_CSPlayer", "m_nSurvivalTeam");

	char* GetArmorName()
	{
		if (this->m_ArmorValue() > 0)
		{
			if (this->m_bHasHelmet())
				return "hk";
			else
				return "k";
		}
		return "";
	}

	int C_BasePlayer::GetFOV( ) {
		if( m_iFOV( ) != 0 )
			return m_iFOV( );
		return m_iDefaultFOV( );
	}

	//NETVAR(int, m_iAccount, "DT_CSPlayer", "m_iAccount");

	bool InDangerzone()
	{
		static auto game_type = g_CVar->FindVar("game_type");
		return game_type->GetInt() == 6;
	}

	bool IsEnemy()
	{
		if (InDangerzone())
		{
			return this->m_nSurvivalTeam() != g_LocalPlayer->m_nSurvivalTeam() || g_LocalPlayer->m_nSurvivalTeam() == -1;
		}
		else
		{
			return this->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
		}
	}

	void C_BasePlayer::SetAbsAngles(const QAngle& angles)
	{
		using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle &angles);
		static SetAbsAnglesFn SetAbsAngles = (SetAbsAnglesFn)Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");

		SetAbsAngles(this, angles);
	}

	void SetAbsOriginal(Vector origin)
	{
		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector &origin);
		static SetAbsOriginFn SetAbsOrigin;
		if (!SetAbsOrigin)
		{
			SetAbsOrigin = (SetAbsOriginFn)((DWORD)Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
		}
		SetAbsOrigin(this, origin);
	}

	NETVAR( QAngle, m_angAbsAngles, "DT_BaseEntity", "m_angAbsAngles" );
	NETVAR( Vector, m_angAbsOrigin, "DT_BaseEntity", "m_angAbsOrigin" );
	NETVAR( float, m_flDuckSpeed, "DT_BaseEntity", "m_flDuckSpeed" );
	NETVAR( float, m_flDuckAmount, "DT_BasePlayer", "m_flDuckAmount" );
	NETVAR(float_t, m_flNextAttack, "DT_BaseCombatCharacter", "m_flNextAttack");
	std::array<float, 24> m_flPoseParameter( ) const {
		static int _m_flPoseParameter = NetvarSys::Get( ).GetOffset( "DT_BaseAnimating", "m_flPoseParameter" );
		return *( std::array<float, 24>* )( ( uintptr_t )this + _m_flPoseParameter );
	}


	PNETVAR( CHandle<C_BaseCombatWeapon>, m_hMyWeapons, "DT_BaseCombatCharacter", "m_hMyWeapons" );
	PNETVAR( CHandle<C_BaseAttributableItem>, m_hMyWearables, "DT_BaseCombatCharacter", "m_hMyWearables" );

	CUserCmd*& m_pCurrentCommand( );

	void InvalidateBoneCache( );
	int GetNumAnimOverlays( );
	AnimationLayer *GetAnimOverlays( );
	AnimationLayer *GetAnimOverlay( int i );
	float GetDesyncDelta();
	int GetSequenceActivity( int sequence );
	CCSGOPlayerAnimState *GetPlayerAnimState( );

	static void UpdateAnimationState( CCSGOPlayerAnimState *state, QAngle angle );
	static void ResetAnimationState( CCSGOPlayerAnimState *state );
	void CreateAnimationState( CCSGOPlayerAnimState *state );

	float_t &m_surfaceFriction( ) {
		static unsigned int _m_surfaceFriction = Utils::FindInDataMap( GetPredDescMap( ), "m_surfaceFriction" );
		return *( float_t* )( ( uintptr_t )this + _m_surfaceFriction );
	}
	Vector &m_vecBaseVelocity( ) {
		static unsigned int _m_vecBaseVelocity = Utils::FindInDataMap( GetPredDescMap( ), "m_vecBaseVelocity" );
		return *( Vector* )( ( uintptr_t )this + _m_vecBaseVelocity );
	}

	float_t &m_flMaxspeed( ) {
		static unsigned int _m_flMaxspeed = Utils::FindInDataMap( GetPredDescMap( ), "m_flMaxspeed" );
		return *( float_t* )( ( uintptr_t )this + _m_flMaxspeed );
	}


	Vector        GetEyePos( );
	player_info_t GetPlayerInfo( );
	bool          IsAlive( );
	bool		  IsFlashed( );
	bool          HasC4( );
	Vector        GetHitboxPos( int hitbox_id );

	void			PrecaceOptimizedHitboxes();
	bool GetOptimizedHitboxPos(int hitbox, Vector& output);

	mstudiobbox_t * GetHitbox( int hitbox_id );
	bool          GetHitboxPos( int hitbox, Vector &output );
	Vector        GetBonePos( int bone );

	bool          CanSeePlayer( C_BasePlayer* player, int hitbox );
	bool          CanSeePlayer( C_BasePlayer* player, const Vector& pos );
	void UpdateClientSideAnimation( );

	int m_nMoveType( );

	QAngle * GetVAngles( );

	float_t m_flSpawnTime( );

	bool IsNotTarget( );

};

class C_BaseViewModel : public C_BaseEntity
{
public:
	NETVAR( int32_t, m_nModelIndex, "DT_BaseViewModel", "m_nModelIndex" );
	NETVAR( int32_t, m_nViewModelIndex, "DT_BaseViewModel", "m_nViewModelIndex" );
	NETVAR( CHandle<C_BaseCombatWeapon>, m_hWeapon, "DT_BaseViewModel", "m_hWeapon" );
	NETVAR( CHandle<C_BasePlayer>, m_hOwner, "DT_BaseViewModel", "m_hOwner" );
	NETPROP( m_nSequence, "DT_BaseViewModel", "m_nSequence" );
	void SendViewModelMatchingSequence( int sequence );
};

class AnimationLayer
{
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void *m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class CCSGOPlayerAnimState
{
public:
	void* pThis;
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float& m_flAbsRotation() {
		return *(float*)((uintptr_t)this + 0x80);
	}
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];

	Vector get_vec_velocity()
	{
		// Only on ground velocity
		return *reinterpret_cast<Vector*> (reinterpret_cast<uintptr_t> (this) + 0xC8);
	}
}; //Size=0x344

class c_player_resource
{
public:
	NETPROP(GetTeamProp, "CPlayerResource", "m_iTeam");
	NETVAR(int, GetPing, "CPlayerResource", "m_iPing");
};

class DT_CSPlayerResource : public c_player_resource
{
public:
	PNETVAR( int32_t, m_nActiveCoinRank, "DT_CSPlayerResource", "m_nActiveCoinRank" );
	PNETVAR( int32_t, m_nMusicID, "DT_CSPlayerResource", "m_nMusicID" );
	PNETVAR( int32_t, m_nPersonaDataPublicLevel, "DT_CSPlayerResource", "m_nPersonaDataPublicLevel" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsLeader, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsLeader" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsTeacher, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsTeacher" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsFriendly, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsFriendly" );
	PNETVAR( int32_t, m_iCompetitiveRanking, "DT_CSPlayerResource", "m_iCompetitiveRanking" );
	PNETVAR( int32_t, m_iCompetitiveWins, "DT_CSPlayerResource", "m_iCompetitiveWins" );
	PNETVAR( int32_t, m_iPlayerVIP, "DT_CSPlayerResource", "m_iPlayerVIP" );
	PNETVAR( int32_t, m_iMVPs, "DT_CSPlayerResource", "m_iMVPs" );
	PNETVAR( int32_t, m_iScore, "DT_CSPlayerResource", "m_iScore" );
};

class CGameRules {
public:
	NETVAR( int, m_iMatchStats_PlayersAlive_T, "DT_CSGameRulesProxy", "m_iMatchStats_PlayersAlive_T" );
	NETVAR( int, m_iMatchStats_PlayersAlive_CT, "DT_CSGameRulesProxy", "m_iMatchStats_PlayersAlive_CT" );
	NETVAR( int, m_iRoundTime, "DT_CSGameRulesProxy", "m_iRoundTime" );
	NETVAR( bool, m_bFreezePeriod, "DT_CSGameRulesProxy", "m_bBombDropped" );
	NETVAR( bool, m_bIsValveDS, "DT_CSGameRulesProxy", "m_bIsValveDS" );
	NETVAR( bool, m_bBombDropped, "DT_CSGameRulesProxy", "m_bBombDropped" );
	NETVAR( bool, m_bBombPlanted, "DT_CSGameRulesProxy", "m_bBombPlanted" );
	NETVAR( float, m_flSurvivalStartTime, "DT_CSGameRulesProxy", "m_flSurvivalStartTime" );
}; extern CGameRules* g_GameRules;
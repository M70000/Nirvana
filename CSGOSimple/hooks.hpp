#pragma once

#include "valve_sdk/csgostructs.hpp"
#include "minhook/minhook.h"
#include "INetMessage.h"
#include <d3d9.h>

extern int32_t nTickBaseShift;
extern int32_t nSinceUse;

namespace index
{
  constexpr auto EmitSound1 = 5;
  constexpr auto EmitSound2 = 6;
  constexpr auto EndScene = 42;
  constexpr auto Reset = 16;
  constexpr auto PaintTraverse = 41;
  constexpr auto CreateMove = 22;
  constexpr auto PlaySound = 82;
  constexpr auto FrameStageNotify = 37;
  constexpr auto DrawModelExecute = 29;
  constexpr auto DoPostScreenSpaceEffects = 44;
  constexpr auto SvCheatsGetBool = 13;
  constexpr auto OverrideView = 18;
  constexpr auto FireEvent = 9;
  constexpr auto LockCursor = 67;
  constexpr auto ClientModeCreateMove = 24;
  constexpr auto OverrideMouseInput = 23;
  constexpr auto ListLeavesInBox = 6;
  constexpr auto SuppressLists = 16;
  constexpr auto PostEntityPacketReceived = 20 / 4;
  constexpr auto ReadPacketEntities = 64;
  constexpr auto AspectRatio = 101;
  constexpr auto SendMsg = 0;
  constexpr auto RetrieveMsg = 2;
}

namespace Hooks
{
  void Initialize( );
  void Shutdown( );

  struct file_system_017
  {
	  static const int index = 128;
	  using fn = int(__fastcall*)(void*, void*);
	  static int __fastcall hooked(void* ecx, void* edx);

	  inline static fn original;
	  inline static void* setup;
  };

  struct end_scene
  {
	  static const int index = 42;
	  using fn = long(__stdcall*)(IDirect3DDevice9*);
	  static long __stdcall hooked(IDirect3DDevice9*);

	  inline static fn original;
	  inline static void* setup;
  };

  struct create_move
  {
	  static const int index = 24;
	  using fn = bool(__thiscall*)(IClientMode*, float, CUserCmd*);
	  static bool __stdcall hooked(float input_sample_frametime, CUserCmd* cmd);
	 // static void __stdcall proxy(int sequence_number, float input_sample_frametime, bool active);

	  inline static fn original;
	  inline static void* setup;
  };

  struct reset
  {
	  static const int index = 16;
	  using fn = long(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	  static long __stdcall hooked(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

	  inline static fn original;
	  inline static void* setup;
  };

  struct suppress_list
  {
	  static const int index = 16;
	  using fn = bool(__thiscall*)(void*, int, bool);
	  static void __stdcall hooked(int a2, bool a3);

	  inline static fn original;
	  inline static void* setup;
  };

  struct paint_traverse
  {
	  static const int index = 41;
	  using fn = void(__thiscall*)(IPanel*, vgui::VPANEL, bool, bool);
	  static void __stdcall hooked(vgui::VPANEL, bool forceRepaint, bool allowForce);

	  inline static fn original;
	  inline static void* setup;
  };

  struct list_leaves_in_box
  {
	  static const int index = 6;
	  using fn = int(__thiscall*)(void*, const Vector&, const Vector&, unsigned short*, int);
	  static int __fastcall hooked(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax);

	  inline static fn original;
	  inline static void* setup;
  };

  // RETADRR FIX START
  struct retaddrClientDLL
  {
	  static const int index = 1;
	  using fn = char(__fastcall*)(void*, void*, const char*);
	  static char __fastcall hooked(void* thisPointer, void* edx, const char* moduleName);

	  inline static fn original;
	  inline static void* setup;
  };

  struct retaddrEngineDLL
  {
	  static const int index = 1;
	  using fn = char(__fastcall*)(void*, void*, const char*);
	  static char __fastcall hooked(void* thisPointer, void* edx, const char* moduleName);

	  inline static fn original;
	  inline static void* setup;
  };

  struct retaddrStudioRenderDLL
  {
	  static const int index = 1;
	  using fn = char(__fastcall*)(void*, void*, const char*);
	  static char __fastcall hooked(void* thisPointer, void* edx, const char* moduleName);

	  inline static fn original;
	  inline static void* setup;
  };

  struct retaddrMaterialSystemDLL
  {
	  static const int index = 1;
	  using fn = char(__fastcall*)(void*, void*, const char*);
	  static char __fastcall hooked(void* thisPointer, void* edx, const char* moduleName);

	  inline static fn original;
	  inline static void* setup;
  };
  // RETADRR FIX END

  struct fire_event
  {
	  static const int index = 9;
	  using fn = bool(__thiscall*)(IGameEventManager2*, IGameEvent*);
	  static bool __stdcall hooked(IGameEvent* pEvent);

	  inline static fn original;
	  inline static void* setup;
  };

  struct override_view
  {
	  static const int index = 18;
	  using fn = void(__thiscall*)(IClientMode*, CViewSetup*);
	  static void __stdcall hooked(CViewSetup*);

	  inline static fn original;
	  inline static void* setup;
  };

  struct override_mouse_input
  {
	  static const int index = 23;
	  using fn = void(__fastcall*)(void*, int, float*, float*);
	  static void __fastcall hooked(void* thisptr, int edx, float* x, float* y);

	  inline static fn original;
	  inline static void* setup;
  };

  struct emit_sound
  {
	  static const int index = 5;
	  using fn = void(__thiscall*)(void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, int, float, int, int, const Vector*, const Vector*, void*, bool, float, int, int);
	  static void __stdcall hooked(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);

	  inline static fn original;
	  inline static void* setup;
  };

  struct post_entity_packet_received
  {
	  static const int index = 20/4;
	  using fn = void(__thiscall*)(void*);
	  static void __fastcall hooked(void* pPrediction, void* pEDX);

	  inline static fn original;
	  inline static void* setup;
  };

  struct read_packet_entities
  {
	  static const int index = 64;
	  using fn = void(__thiscall*)(void*, void*);
	  static void __fastcall hooked(void* pClientState, void* pEDX, void* pEntInfo);

	  inline static fn original;
	  inline static void* setup;
  };

  struct sv_cheats
  {
	  static const int index = 13;
	  using fn = bool(__thiscall*)(PVOID);
	  static bool __fastcall hooked(PVOID pConVar, void* edx);

	  inline static fn original;
	  inline static void* setup;
  };

  struct draw_model_execute
  {
	  static const int index = 29;
	  using fn = void(__thiscall*)(void*, void*, DrawModelInfo_t*, matrix3x4_t*, float*, float*, Vector&, int);
	  static void __fastcall hooked(void* pEcx, void* pEdx, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags);

	  inline static fn original;
	  inline static void* setup;
  };

  struct post_screen_effects
  {
	  static const int index = 44;
	  using fn = int(__thiscall*)(void*, int);
	  static int __stdcall hooked(int value);

	  inline static fn original;
	  inline static void* setup;
  };

  struct is_connected
  {
	  static const int index = 27;
	  using fn = bool(__thiscall*)(IVEngineClient*);
	  static bool __fastcall hooked();

	  inline static fn original;
	  inline static void* setup;
  };

  struct frame_stage_notify
  {
	  static const int index = 37;
	  using fn = void(__thiscall*)(IBaseClientDLL*, ClientFrameStage_t);
	  static void __stdcall hooked(ClientFrameStage_t stage);

	  inline static fn original;
	  inline static void* setup;
  };

  struct retrieve_message
  {
	  static const int index = 2;
	  using fn = EGCResult(__thiscall*)(void*, void*, uint32_t*, void*, uint32_t, uint32_t*);
	  static EGCResult __fastcall hooked(void* _this, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);

	  inline static fn original;
	  inline static void* setup;
  };

  struct send_message
  {
	  static const int index = 0;
	  using fn = EGCResult(__thiscall*)(void*, void*, uint32_t, const void*, uint32_t);
	  static EGCResult __fastcall hooked(void* _this, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData);

	  inline static fn original;
	  inline static void* setup;
  };

  struct send_net_message
  {
	  static const int index = 40;
	  using fn = bool(__fastcall*)(INetChannel*, void*, INetMessage&, bool, bool);
	  static bool __fastcall hooked(INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice);

	  inline static fn original;
	  inline static void* setup;
  };

  struct send_datagram
  {
	  static const int index = 46;
	  using fn = int(__thiscall*)(INetChannel* netchan, bf_write* datagram);
	  static int __fastcall hooked(INetChannel* thisptr, int edx, bf_write* datagram);

	  inline static fn original;
	  inline static void* setup;
  };

  struct sequence
  {
	  using fn = void(__thiscall*)(const CRecvProxyData* data, void* entity, void* output);
	  static void hooked(const CRecvProxyData* data, void* entity, void* output);
  };

  struct check_file_crc_server
  {
	  using fn = void(__thiscall*)(void*, void*);
	  static void __fastcall hooked(void*, void*);

	  inline static fn original;
	  inline static void* setup;
  };

  struct is_depth_of_field 
  {
	  using fn = bool(__thiscall*)();
	  static bool __stdcall hooked();

	  inline static fn original;
	  inline static void* setup;
  };

  struct lock_cursor
  {
	  static const int index = 67;
	  using fn = void(__thiscall*)(ISurface*);
	  static void __stdcall hooked();

	  inline static fn original;
	  inline static void* setup;
  };

  /*struct is_playing_demo
  {
	  static const int index = 82;
	  using fn = bool(__thiscall*)();
	  static bool __stdcall hooked();

	  inline static fn original;
	  inline static void* setup;
  };*/

  /*using zSendNetMsg = bool(__fastcall*)(INetChannel*, void*, INetMessage&, bool, bool);
  using vFileSystem017 = int(__fastcall*)(void*, void*);
  using EndScene = long( __stdcall * )( IDirect3DDevice9* );
  using Reset = long( __stdcall * )( IDirect3DDevice9*, D3DPRESENT_PARAMETERS* );
  using CreateMove = void( __thiscall* )( IBaseClientDLL*, int, float, bool );
  using CreateMoveClientMode = bool( __thiscall* )( IClientMode*, float, CUserCmd* );
  using PaintTraverse = void( __thiscall* )( IPanel*, vgui::VPANEL, bool, bool );
  using EmitSound1 = void( __thiscall* )( void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, int, float, int, int, const Vector*, const Vector*, void*, bool, float, int, int );

  using FrameStageNotify = void( __thiscall* )( IBaseClientDLL*, ClientFrameStage_t );
  using PlaySound = void( __thiscall* )( ISurface*, const char* name );
  using LockCursor_t = void( __thiscall* )( ISurface* );
  using DrawModelExecute = void( __thiscall* )( void*, void*, DrawModelInfo_t*, matrix3x4_t*, float*, float*, Vector&, int );
  using FireEvent = bool( __thiscall* )( IGameEventManager2*, IGameEvent* pEvent );
  using DoPostScreenEffects = int( __thiscall* )( IClientMode*, int );
  using OverrideView = void( __thiscall* )( IClientMode*, CViewSetup* );
  using PostEntityPacketReceived = void( __thiscall* )( void* );
  using ReadPacketEntities = void( __thiscall* )( void*, void* );

  using SuppressLists = bool( __thiscall* )( void*, int, bool );*/

}

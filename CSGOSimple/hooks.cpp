#include "hooks.hpp"
#include <intrin.h>  

#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "features/visuals.hpp"
#include "features/glow.hpp"
#include "features/aimbot.hpp"
#include "features/triggerbot.h"
#include "features/nosmoke.hpp"
#include "features/backtrack.hpp"
#include "features/engine_prediction.hpp"
#include "features/grenade_prediction.hpp"
#include "features/skins.hpp"
#include "features/vote_listener.h"
#include "features/sonar_esp.h"
#include "features/grenade_circle.h"
#include "features/moverecorder.h"
#include "features/Autoblock.h"
#include "features/damageindicator.hpp"
#include "features/legitresolver.h"
#include "helpers/proto/protobuff.hpp"
#pragma intrinsic(_ReturnAddress)  

void i_can_fix_you_daddy()
{
	const char* modules[]{ "client.dll", "engine.dll", "server.dll", "studiorender.dll", "materialsystem.dll", "shaderapidx9.dll", "vstdlib.dll", "vguimatsurface.dll" };
	long long long_long = 0x69690004C201B0;
	for (auto test : modules)
		WriteProcessMemory(GetCurrentProcess(), (LPVOID)Utils::PatternScan(GetModuleHandleA(test), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &long_long, 7, 0);
}

float side = 1.0f;

float real_angle = 0.0f;
float view_angle = 0.0f;
int32_t nTickBaseShift = 0;
int32_t nSinceUse = 0;

static auto sonar = SonarStuff();
void ApplyStickers(C_BaseCombatWeapon* pItem);

void ResolverLegit(C_BasePlayer* pl) {
	if (!pl->GetPlayerAnimState() || pl->m_vecVelocity().Length() > 0 || !g_Options.legit_resolver)
		return;

	if (fabs(std::remainderf(pl->m_flLowerBodyYawTarget(), 360.f) - std::remainderf(pl->m_angEyeAngles().yaw, 360.f)) < 60)
		return;

	auto lbyfix = Math::ClampYaw(std::remainderf(pl->m_flLowerBodyYawTarget() + 180, 360.f));
	pl->GetPlayerAnimState()->m_flGoalFeetYaw = lbyfix;
}

namespace Hooks
{
	recv_prop_hook* sequence_hook;

  void Initialize( )
  {
	  sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), sequence::hooked);
	  end_scene::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_D3DDevice9, Hooks::end_scene::index));
	  create_move::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_ClientMode, Hooks::create_move::index));
	  reset::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_D3DDevice9, Hooks::reset::index));
	  paint_traverse::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_VGuiPanel, Hooks::paint_traverse::index));
	  override_view::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_ClientMode, Hooks::override_view::index));
	  fire_event::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_GameEvents, Hooks::fire_event::index));
	  emit_sound::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_EngineSound, Hooks::emit_sound::index));
	  is_connected::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_ClientMode, Hooks::is_connected::index));;
	  post_screen_effects::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_ClientMode, Hooks::post_screen_effects::index));
	  frame_stage_notify::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_CHLClient, Hooks::frame_stage_notify::index));
	  draw_model_execute::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_StudioRender, Hooks::draw_model_execute::index));
	  check_file_crc_server::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC 81 EC ? ? ? ? 53 8B D9 89 5D F8 80"));
	  file_system_017::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_FileSystem, Hooks::file_system_017::index));
	  //is_playing_demo::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_ClientMode, Hooks::is_playing_demo::index));
	  suppress_list::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_SpatialPartition, Hooks::suppress_list::index));
	  lock_cursor::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_VGuiSurface, Hooks::lock_cursor::index));
	  override_mouse_input::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_ClientMode, Hooks::override_mouse_input::index));
	 // send_datagram::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_EngineClient, Hooks::send_datagram::index));
	//  send_message::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_SteamGameCoordinator, Hooks::send_message::index));
	//  retrieve_message::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_SteamGameCoordinator, Hooks::retrieve_message::index));
	  is_depth_of_field::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("client.dll"), "8B 0D ? ? ? ? 56 8B 01 FF 50 ? 8B F0 85 F6 75 ?"));
	  list_leaves_in_box::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_EngineClient->GetBSPTreeQuery(), Hooks::list_leaves_in_box::index));
	  sv_cheats::setup = reinterpret_cast<void*>(Utils::GetVirtual(g_CVar->FindVar("sv_cheats"), Hooks::sv_cheats::index));
	  read_packet_entities::setup = reinterpret_cast<void*>(Utils::GetVirtual((void*)((uintptr_t)g_ClientState + 0x8), Hooks::read_packet_entities::index));

	  retaddrClientDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"));
	  retaddrEngineDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"));
	  retaddrStudioRenderDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("studiorender.dll"), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"));
	  retaddrMaterialSystemDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("materialsystem.dll"), "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"));

	  /*retaddrClientDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 34 56 C7"));
	  retaddrEngineDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC 56 8B F1 33 C0 57"));
	  retaddrStudioRenderDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("studiorender.dll"), "55 8B EC 56 8B F1 33"));
	  retaddrMaterialSystemDLL::setup = reinterpret_cast<void*>(Utils::PatternScan(GetModuleHandleA("materialsystem.dll"), "55 8B EC 56 8B F1 33 C0"));*/

	  if (MH_Initialize() != MH_OK) {
		  MessageBoxA(NULL, "Failed to initialize Minhook.", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(end_scene::setup, &Hooks::end_scene::hooked, reinterpret_cast<void**>(&end_scene::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - DirectX: End Scene", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(create_move::setup, &Hooks::create_move::hooked, reinterpret_cast<void**>(&create_move::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Create Move", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(is_depth_of_field::setup, &Hooks::is_depth_of_field::hooked, reinterpret_cast<void**>(&is_depth_of_field::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Is Depth Of Field", MB_OK, MB_ICONERROR);
	  }

	  /*if (MH_CreateHook(create_move::setup, &Hooks::create_move::proxy, reinterpret_cast<void**>(&create_move::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Create Move", MB_OK, MB_ICONERROR);
	  }*/

	  if (MH_CreateHook(reset::setup, &Hooks::reset::hooked, reinterpret_cast<void**>(&reset::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - DirectX: Reset", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(paint_traverse::setup, &Hooks::paint_traverse::hooked, reinterpret_cast<void**>(&paint_traverse::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Paint Traverse", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(override_view::setup, &Hooks::override_view::hooked, reinterpret_cast<void**>(&override_view::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Override View", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(override_mouse_input::setup, &Hooks::override_mouse_input::hooked, reinterpret_cast<void**>(&override_mouse_input::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Override Mouse Input", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(fire_event::setup, &Hooks::fire_event::hooked, reinterpret_cast<void**>(&fire_event::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Fire Game Event", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(emit_sound::setup, &Hooks::emit_sound::hooked, reinterpret_cast<void**>(&emit_sound::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Emit Sound", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(is_connected::setup, &Hooks::is_connected::hooked, reinterpret_cast<void**>(&is_connected::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Is Connected", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(post_screen_effects::setup, &Hooks::post_screen_effects::hooked, reinterpret_cast<void**>(&post_screen_effects::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Post Screen Effects", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(frame_stage_notify::setup, &Hooks::frame_stage_notify::hooked, reinterpret_cast<void**>(&frame_stage_notify::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Frame Stage Notify", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(draw_model_execute::setup, &Hooks::draw_model_execute::hooked, reinterpret_cast<void**>(&draw_model_execute::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Draw Model Execute", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(check_file_crc_server::setup, &Hooks::check_file_crc_server::hooked, reinterpret_cast<void**>(&check_file_crc_server::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated pattern - Check File CRC Server", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(file_system_017::setup, &Hooks::file_system_017::hooked, reinterpret_cast<void**>(&file_system_017::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - File System", MB_OK, MB_ICONERROR);
	  }

	  /*if (MH_CreateHook(is_playing_demo::setup, &Hooks::is_playing_demo::hooked, reinterpret_cast<void**>(&is_playing_demo::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Is Playing Demo", MB_OK, MB_ICONERROR);
	  }*/

	  if (MH_CreateHook(suppress_list::setup, &Hooks::suppress_list::hooked, reinterpret_cast<void**>(&suppress_list::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Suppress List", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(lock_cursor::setup, &Hooks::lock_cursor::hooked, reinterpret_cast<void**>(&lock_cursor::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Lock Cursor", MB_OK, MB_ICONERROR);
	  }

	  /*if (MH_CreateHook(send_message::setup, &Hooks::send_message::hooked, reinterpret_cast<void**>(&send_message::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Send Message", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(retrieve_message::setup, &Hooks::retrieve_message::hooked, reinterpret_cast<void**>(&retrieve_message::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Retrieve Message", MB_OK, MB_ICONERROR);
	  }*/

	  if (MH_CreateHook(list_leaves_in_box::setup, &Hooks::list_leaves_in_box::hooked, reinterpret_cast<void**>(&list_leaves_in_box::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - List Leaves In Box", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(sv_cheats::setup, &Hooks::sv_cheats::hooked, reinterpret_cast<void**>(&sv_cheats::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - sv_cheats", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(read_packet_entities::setup, &Hooks::read_packet_entities::hooked, reinterpret_cast<void**>(&read_packet_entities::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Read Packet Entities", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(retaddrClientDLL::setup, &Hooks::retaddrClientDLL::hooked, reinterpret_cast<void**>(&retaddrClientDLL::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Return Address Client DLL", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(retaddrEngineDLL::setup, &Hooks::retaddrEngineDLL::hooked, reinterpret_cast<void**>(&retaddrEngineDLL::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Return Address Engine DLL", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(retaddrStudioRenderDLL::setup, &Hooks::retaddrStudioRenderDLL::hooked, reinterpret_cast<void**>(&retaddrStudioRenderDLL::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Return Address Studio Render DLL", MB_OK, MB_ICONERROR);
	  }

	  if (MH_CreateHook(retaddrMaterialSystemDLL::setup, &Hooks::retaddrMaterialSystemDLL::hooked, reinterpret_cast<void**>(&retaddrMaterialSystemDLL::original)) != MH_OK) {
		  MessageBoxA(NULL, "Outdated index - Return Address Material System DLL", MB_OK, MB_ICONERROR);
	  }

	  if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		  MessageBoxA(NULL, "Failed to enable hooks.", MB_OK, MB_ICONERROR);
	  }

	  i_can_fix_you_daddy();

	 //Listeners
	 g_GameEvents->AddListener( &Visuals::Get( ), "player_hurt", false );
	 g_GameEvents->AddListener( &Visuals::Get( ), "vote_cast", true);
	 g_GameEvents->AddListener( &Visuals::Get( ), "bomb_beginplant", true );
	 g_GameEvents->AddListener( &Visuals::Get( ), "bomb_abortplant", true );
	 g_GameEvents->AddListener( &Visuals::Get( ), "bomb_begindefuse", true );
	 g_GameEvents->AddListener( &Visuals::Get( ), "bomb_abortdefuse", true );
	 g_GameEvents->AddListener( &Visuals::Get( ), "round_end", true );
	 g_GameEvents->AddListener( &Visuals::Get( ), "show_freezepanel", true );
	 damage_indicators.Listener();
	 GrenadeDetonateEvent::Get().RegisterSelf();

  }
  //--------------------------------------------------------------------------------
  void Shutdown( )
  {
	  MH_Uninitialize();
	  MH_DisableHook(MH_ALL_HOOKS);

	  g_GameEvents->RemoveListener(&Visuals::Get());
	  damage_indicators.RemoveListener();
	  sequence_hook->~recv_prop_hook();
	  GrenadeDetonateEvent::Get().UnregisterSelf();
	  Glow::Get( ).Shutdown( );
  }
  //--------------------------------------------------------------------------------
  int __fastcall file_system_017::hooked(void* ecx, void* edx)
  {
	  return original(ecx, edx);
  }

  char __fastcall retaddrClientDLL::hooked(void* thisPointer, void* edx, const char* moduleName)
  {
	  return original(thisPointer, edx, moduleName);
  }
  char __fastcall retaddrEngineDLL::hooked(void* thisPointer, void* edx, const char* moduleName)
  {
	  return original(thisPointer, edx, moduleName);
  }
  char __fastcall retaddrStudioRenderDLL::hooked(void* thisPointer, void* edx, const char* moduleName)
  {
	  return original(thisPointer, edx, moduleName);
  }
  char __fastcall retaddrMaterialSystemDLL::hooked(void* thisPointer, void* edx, const char* moduleName)
  {
	  return original(thisPointer, edx, moduleName);
  }

  /*bool __stdcall is_playing_demo::hooked()
  {
	  if (*static_cast<uintptr_t*>(_ReturnAddress()) == 0x0975C084 // client.dll : 84 C0 75 09 38 05
		  && **reinterpret_cast<uintptr_t**>(uintptr_t(_AddressOfReturnAddress()) + 4) == 0x0C75C084) { // client.dll : 84 C0 75 0C 5B
		  return true;
	  }
	  return original;
  }*/
  void __fastcall check_file_crc_server::hooked(void* ecx, void* edx)
  {
	  return;
  }
  bool __fastcall is_connected::hooked()
  {
	 /* static void* force_inventory_open = Utils::PatternScan(("client.dll"), "75 04 B0 01 5F") - 2;

	  if (_ReturnAddress() == force_inventory_open && g_Options.unlock_inventory)
		  return false;*/

	  return original;
  }
  int __fastcall send_datagram::hooked(INetChannel* thisptr, int edx, bf_write* datagram)
  {
	  auto network = g_EngineClient->GetNetChannelInfo();
	  ConVar* sv_maxunlag = g_CVar->FindVar("sv_maxunlag");

	 /*if (!g_Options.fakelatency || datagram || !g_EngineClient->IsInGame())
	  {
		  return original(thisptr, datagram);
	  }*/

	  int instate = thisptr->m_nInReliableState;
	  int insequencenr = thisptr->m_nInSequenceNr;

	 //float delta = std::max(0.f, std::clamp(g_Options.fakelatency_amouth / 1000.f, 0.f, sv_maxunlag->GetFloat()) - network->GetLatency(0));
	  
	  //g_Backtrack.AddLatencyToNetwork(thisptr, delta);

	  int result = original(thisptr, datagram);

	  thisptr->m_nInReliableState = instate;
	  thisptr->m_nInSequenceNr = insequencenr;

	  return result;
  }
  bool __fastcall send_net_message::hooked(INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice)
  {
	  if (msg.GetType() == 14 && g_Options.misc_svpure) // Return and don't send messsage if its FileCRCCheck
		  return false;

	  if (msg.GetGroup() == 9) // Fix lag when transmitting voice and fakelagging
		  bVoice = true;

	  return original(pNetChan, edx, msg, bForceReliable, bVoice);
  }
  /*EGCResult __fastcall retrieve_message::hooked(void* _this, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
  {
	  auto status = original(_this, 0, punMsgType, pubDest, cubDest, pcubMsgSize);

	  if (status == k_EGCResultOK)
	  {
		  void* thisPtr = nullptr;
		  __asm mov thisPtr, ebx;
		  auto oldEBP = *reinterpret_cast<void**>((uint32_t)_AddressOfReturnAddress() - 4);

		  uint32_t messageType = *punMsgType & 0x7FFFFFFF;
		  Protobuf::ReceiveMessage(thisPtr, oldEBP, messageType, pubDest, cubDest, pcubMsgSize);
	  }

	  return status;
  }
  EGCResult __fastcall send_message::hooked(void* _this, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)
  {
	  Protobuf::PreSendMessage(unMsgType, const_cast<void*>(pubData), cubData);

	  return original(_this, 0, unMsgType, pubData, cubData);
  }*/
  //--------------------------------------------------------------------------------
  long __stdcall end_scene::hooked( IDirect3DDevice9* pDevice ) {
	  auto oEndScene = original;

	 static uintptr_t gameoverlay_return_address = 0;
	 if( !gameoverlay_return_address ) {
		MEMORY_BASIC_INFORMATION info;
		VirtualQuery( _ReturnAddress( ), &info, sizeof( MEMORY_BASIC_INFORMATION ) );

		char mod[MAX_PATH];
		GetModuleFileNameA( ( HMODULE )info.AllocationBase, mod, MAX_PATH );

		if( strstr( mod, "gameoverlay" ) )
		  gameoverlay_return_address = ( uintptr_t )( _ReturnAddress( ) );
	 }

	 if( gameoverlay_return_address != ( uintptr_t )( _ReturnAddress( ) ) )
		return oEndScene( pDevice );

	 IDirect3DStateBlock9* stateBlock = NULL;
	 pDevice->CreateStateBlock(D3DSBT_ALL, &stateBlock);
	 stateBlock->Capture();

	 IDirect3DVertexDeclaration9* vertDec;
	 IDirect3DVertexShader9* vertShader;
	 pDevice->GetVertexDeclaration( &vertDec );
	 pDevice->GetVertexShader( &vertShader );

	 pDevice->SetVertexDeclaration( nullptr );
	 pDevice->SetVertexShader( nullptr );

	 static auto r_modelAmbientMin = g_CVar->FindVar( "r_modelAmbientMin" );
	 static auto mat_force_tonemap_scale = g_CVar->FindVar( "mat_force_tonemap_scale" );
	 static auto mat_postprocess_enable = g_CVar->FindVar( "mat_postprocess_enable" );
	 static auto aspect_ratio = g_CVar->FindVar("r_aspectratio");

	 static auto violence_ablood = g_CVar->FindVar( "violence_ablood" );
	 static auto violence_hblood = g_CVar->FindVar( "violence_hblood" );

	 static auto mat_ambient_light_r = g_CVar->FindVar( "mat_ambient_light_r" );
	 static auto mat_ambient_light_g = g_CVar->FindVar( "mat_ambient_light_g" );
	 static auto mat_ambient_light_b = g_CVar->FindVar( "mat_ambient_light_b" );

	 static auto panoramablur = g_CVar->FindVar("@panorama_disable_blur");
	 static auto sv_skyname = g_CVar->FindVar("sv_skyname");
	 static auto r_3dsky = g_CVar->FindVar("r_3dsky");
	 static auto shadows = g_CVar->FindVar("cl_csm_enabled");
	 static auto thread_bones = g_CVar->FindVar("cl_threaded_bone_setup");

	 int w, h;
	 g_EngineClient->GetScreenSize(w, h);

	 if (!g_Unload)
	 {
		 r_modelAmbientMin->SetValue(g_Options.other_nightmode ? 1.f : 0.0f);
		 mat_force_tonemap_scale->SetValue(g_Options.other_nightmode ? g_Options.other_nightmode_size : 1.0f);
		 //	mat_postprocess_enable->SetValue( g_Options.other_nightmode ? 1 : 0 );
		 float aspect = (w / h);
		 float ratio = (g_Options.misc_aspectratio / 2);

		 if (g_Options.misc_aspectratiotoggle)
		 {
			 if (g_Options.misc_aspectratio > 0 || g_Options.misc_aspectratio < 1)
				 aspect_ratio->SetValue(ratio);
			 else
				 aspect_ratio->SetValue(0);

		 }
		 else
		 {
			 aspect_ratio->SetValue(0);
		 }

		 if (g_Options.other_no_3dsky)
		 {
			 r_3dsky->SetValue(0);
		 }
		 else
		 {
			 r_3dsky->SetValue(1);
		 }

		 if (g_Options.other_no_shadows)
		 {
			 shadows->SetValue(0);
		 }
		 else
		 {
			 shadows->SetValue(1);
		 }

		 if (g_Options.other_single_thread_bones)
		 {
			 thread_bones->SetValue(1);
		 }
		 else
		 {
			 thread_bones->SetValue(0);
		 }

		if (g_Options.no_blood)
		{
			violence_ablood->SetValue(0);
			violence_hblood->SetValue(0);
		}
		else
		{
			violence_ablood->SetValue(1);
			violence_hblood->SetValue(1);
		}

		if (g_Options.no_postprocess)
		{
			mat_postprocess_enable->SetValue(0);
		}
		else
		{
			mat_postprocess_enable->SetValue(1);
		}

		if (g_Options.no_panoramablur)
		{
			panoramablur->SetValue(1);
		}
		else
		{
			panoramablur->SetValue(0);
		}

		if (g_Options.enable_skybox_changer)
		{
			switch (g_Options.skybox_combo)
			{
			case 0: //Aztec
				sv_skyname->SetValue("sky_hr_aztec");
				break;
			case 1: //Baggage
				sv_skyname->SetValue("cs_baggage_skybox_");
				break;
			case 2: //Cloudy
				sv_skyname->SetValue("sky_csgo_cloudy01");
				break;
			case 3: //Day
				sv_skyname->SetValue("sky_day02_05");
				break;
			case 4: //Day HD
				sv_skyname->SetValue("sky_day02_05_hdr");
				break;
			case 5: //Daylight 1
				sv_skyname->SetValue("sky_cs15_daylight01_hdr");
				break;
			case 6: //Daylight 2
				sv_skyname->SetValue("sky_cs15_daylight02_hdr");
				break;
			case 7: //Daylight 3
				sv_skyname->SetValue("sky_cs15_daylight03_hdr");
				break;
			case 8: //Daylight 4
				sv_skyname->SetValue("sky_cs15_daylight04_hdr");
				break;
			case 9: //Dusty Sky
				sv_skyname->SetValue("sky_dust");
				break;
			case 10: //Embassy
				sv_skyname->SetValue("embassy");
				break;
			case 11: //Italy
				sv_skyname->SetValue("italy");
				break;
			case 12: //Jungle
				sv_skyname->SetValue("jungle");
				break;
			case 13: //Lunacy
				sv_skyname->SetValue("sky_lunacy");
				break;
			case 14: //Night 1
				sv_skyname->SetValue("sky_csgo_night02");
				break;
			case 15: //Night 2
				sv_skyname->SetValue("sky_csgo_night02b");
				break;
			case 16: //Night Flat
				sv_skyname->SetValue("sky_csgo_night_flat");
				break;
			case 17: //Nuke
				sv_skyname->SetValue("nukeblank");
				break;
			case 18: //Office
				sv_skyname->SetValue("office");
				break;
			case 19: //Tibet
				sv_skyname->SetValue("cs_tibet");
				break;
			case 20: //Vertigo
				sv_skyname->SetValue("vertigo");
				break;
			case 21: //Vertigo HD
				sv_skyname->SetValue("vertigo_hdr");
				break;
			case 22: //Vertigo Blue HD
				sv_skyname->SetValue("vertigoblue_hdr");
				break;
			case 23: //Vietnam
				sv_skyname->SetValue("vietnam");
				break;
			default:
				break;
			}
		}

		if (g_Options.enable_world_color_modulation)
		{
			if (g_Options.other_mat_ambient_light_rainbow)
			{

				static float rainbow; rainbow += 0.005f; if (rainbow > 1.f) rainbow = 0.f;
				auto rainbow_col = Color::FromHSB(rainbow, 1.0f, 1.0f);

				mat_ambient_light_r->SetValue(rainbow_col.r() / 255.0f);
				mat_ambient_light_g->SetValue(rainbow_col.g() / 255.0f);
				mat_ambient_light_b->SetValue(rainbow_col.b() / 255.0f);
			}
			else
			{
				mat_ambient_light_r->SetValue(g_Options.color_world_modulation[0]);
				mat_ambient_light_g->SetValue(g_Options.color_world_modulation[1]);
				mat_ambient_light_b->SetValue(g_Options.color_world_modulation[2]);
				/*  mat_ambient_light_r->SetValue( g_Options.other_mat_ambient_light_r );
				  mat_ambient_light_g->SetValue( g_Options.other_mat_ambient_light_g );
				  mat_ambient_light_b->SetValue( g_Options.other_mat_ambient_light_b );*/
			}
		}
		else
		{
			mat_ambient_light_r->SetValue(0.f / 255.0f);
			mat_ambient_light_g->SetValue(0.f / 255.0f);
			mat_ambient_light_b->SetValue(0.f / 255.0f);
		}

	 } else {
		if( r_modelAmbientMin->GetFloat( ) != 0.0f )
		  r_modelAmbientMin->SetValue( 0.0f );

		if( mat_force_tonemap_scale->GetFloat( ) != 1.0f )
		  mat_force_tonemap_scale->SetValue( 1.0f );

		if( mat_postprocess_enable->GetInt( ) != 0 )
		  mat_postprocess_enable->SetValue( 0 );

		if (panoramablur->GetBool() != false)
			panoramablur->SetValue(false);

		if( mat_ambient_light_r->GetFloat( ) != 0.0f )	mat_ambient_light_r->SetValue( 0.0f );
		if( mat_ambient_light_g->GetFloat( ) != 0.0f )	mat_ambient_light_g->SetValue( 0.0f );
		if( mat_ambient_light_b->GetFloat( ) != 0.0f )	mat_ambient_light_b->SetValue( 0.0f );
	 }

	 DWORD colorwrite, srgbwrite;
	 pDevice->GetRenderState( D3DRS_COLORWRITEENABLE, &colorwrite );
	 pDevice->GetRenderState( D3DRS_SRGBWRITEENABLE, &srgbwrite );

	 pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
	 pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, false ); // removes the source engine color correction

	 pDevice->SetSamplerState( NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
	 pDevice->SetSamplerState( NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );
	 pDevice->SetSamplerState( NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP );
	 pDevice->SetSamplerState( NULL, D3DSAMP_SRGBTEXTURE, NULL );

	 static auto misc_viewmodel_fov = g_CVar->FindVar( "viewmodel_fov" );

	 if (g_Options.misc_changefov)
	 {
		 misc_viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		 misc_viewmodel_fov->SetValue(g_Options.misc_viewmodel_fov);
	 }

	 ImGui_ImplDX9_NewFrame( );
	 ImGui_ImplWin32_NewFrame( );
	 ImGui::NewFrame( );
	 auto esp_drawlist = Render::Get( ).RenderScene( );
	 Menu::Get( ).Render( );
	ImGui::Render( esp_drawlist );

	// Visuals::Get().KillCounter();
	 ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );

	 pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, colorwrite );
	 pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, srgbwrite );
	 pDevice->SetVertexDeclaration( vertDec );
	 pDevice->SetVertexShader( vertShader );

	 stateBlock->Apply();
	 stateBlock->Release();

	 return oEndScene( pDevice );
  }
  //--------------------------------------------------------------------------------
  long __stdcall reset::hooked( IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters ) {

	 Menu::Get( ).OnDeviceLost( );

	 auto hr = original( device, pPresentationParameters );

	 if( hr >= 0 )
		Menu::Get( ).OnDeviceReset( );

	 return hr;
  }
  //--------------------------------------------------------------------------------
  float AngleDiff( float destAngle, float srcAngle ) {
	 float delta;

	 delta = fmodf( destAngle - srcAngle, 360.0f );
	 if( destAngle > srcAngle ) {
		if( delta >= 180 )
		  delta -= 360;
	 } else {
		if( delta <= -180 )
		  delta += 360;
	 }
	 return delta;
  }

  static int max_choke_ticks = 14;
  static CCSGOPlayerAnimState g_AnimState;
  bool __stdcall create_move::hooked( float input_sample_frametime, CUserCmd* cmd )
  {
	 bool* send_packet = reinterpret_cast< bool* >( reinterpret_cast< uintptr_t >( _AddressOfReturnAddress( ) ) + 0x14 );

	 auto oCreateMove = original;
	 bool result = oCreateMove( g_ClientMode, input_sample_frametime, cmd );

	 if( !cmd || !cmd->command_number || g_Unload )
		return result;

	 static float SpawnTime = 0.0f;
	 if( g_LocalPlayer->m_flSpawnTime( ) != SpawnTime ) 
	 {
		g_AnimState.pBaseEntity = g_LocalPlayer;
		g_LocalPlayer->ResetAnimationState( &g_AnimState );
		SpawnTime = g_LocalPlayer->m_flSpawnTime( );
	 }

	 for (int x = 1; x < g_GlobalVars->maxClients; x++)
	 {
		 auto entity = reinterpret_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(x));
		 legitresolver.Createmove(entity);
	 }

	 NoSmoke::Get( ).OnFrameStageNotify( );

	 if (g_Options.esp_enabled && g_Options.misc_grenade_circle)
		 GrenadeDetonateEvent::Get().Paint();

	 if (g_Options.misc_fastduck)
	 {
		 cmd->buttons |= IN_BULLRUSH;
	 }

	 if (g_LocalPlayer->IsAlive())
	 {
		 sonar.ExecSonar();
	 }

	 static void* oldPointer = nullptr;

	 auto network = g_EngineClient->GetNetChannelInfo();
	 if (oldPointer != network && network && g_EngineClient->IsInGame())
	 {
		 oldPointer = network;
		 send_net_message::setup = reinterpret_cast<void*>(Utils::GetVirtual(network, Hooks::send_net_message::index));
		 send_datagram::setup = reinterpret_cast<void*>(Utils::GetVirtual(network, Hooks::send_datagram::index));

		 g_Backtrack.UpdateIncomingSequences(true);
		 if (MH_CreateHook(send_net_message::setup, &Hooks::send_net_message::hooked, reinterpret_cast<void**>(&send_net_message::original)) != MH_OK)
		 {
			 //MessageBoxA(NULL, "Outdated index - Send Net Message", MB_OK, MB_ICONERROR);
		 }

		 if (MH_CreateHook(send_datagram::setup, &Hooks::send_datagram::hooked, reinterpret_cast<void**>(&send_datagram::original)) != MH_OK)
		 {
			 //MessageBoxA(NULL, "Outdated index - Send Datagram", MB_OK, MB_ICONERROR);
		 }
	 }
	 g_Backtrack.UpdateIncomingSequences();

	 Visuals::Get().FootTrails();

	 if (g_Options.misc_ebugdetector && g_Options.misc_ebug)
	 {
		 BunnyHop::Get().EdgebugDraw();
	 }

	 if( Menu::Get( ).IsVisible( ) )
		cmd->buttons &= ~( IN_ATTACK | IN_ATTACK2 );

	 if( g_Options.misc_bhop )
		BunnyHop::Get( ).OnCreateMove( cmd );

	// BunnyHop::Get().EdgeBugDetector(cmd);

	 if (g_Options.misc_faststop)
		 BunnyHop::Get().FastStop(cmd);

	 Visuals::Get().sniperCrosshair();

	 if (g_Options.anti_untrusted)
	 {
		 Math::ClampAngles(cmd->viewangles);
	 }

	 if( g_Options.misc_autostrafe )
		BunnyHop::Get( ).AutoStrafe( cmd, cmd->viewangles );

	 if( g_Options.other_no_flash )
		g_LocalPlayer->m_flFlashDuration( ) = 0.f;

	 static float LastChangeTime = 0.f;
	 bool restore = false;
	 if( g_Options.misc_clantag && g_LocalPlayer )
	 {
		if( g_GlobalVars->realtime - LastChangeTime >= 0.5f )
		{
		  static std::string text = "$ Nirvana ";

		  LastChangeTime = g_GlobalVars->realtime;

		  std::string temp = text;
		  text.erase( 0, 1 );
		  text += temp[0];

		  Utils::SetClantag( text.data( ) );
		  restore = true;
		}
	 } 
	 else if( restore ) 
	 {
		restore = false;
		Utils::SetClantag( "" );
	 }

	 int old_flags = g_LocalPlayer->m_fFlags( );

	 static float next_lby = 0.0f;

	 BunnyHop::Get().SlideWalk(cmd);

	 uintptr_t* frame_pointer;
	 __asm mov frame_pointer, ebp;
	// bool& send_packetz = *reinterpret_cast<bool*>(*frame_pointer - 0x1C);

	 //send_packetz = true;
	 bool send_packetz = true;

	 if (g_Options.misc_autoalign)
	 {
		 BunnyHop::Get().Autoalign(cmd);
	 }

	 EnginePrediction::Begin( cmd );
	 {
		 if (g_Options.misc_edgejump && g_Options.misc_edgejump_bind && GetAsyncKeyState(g_Options.misc_edgejump_bind) && (old_flags & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		 {
			 cmd->buttons |= IN_JUMP;
		 }

		g_Aimbot.OnMove( cmd );
		//Triggerbot::run(cmd); crashing don't know why
		g_Backtrack.OnMove( cmd );

		g_Aimbot.LegitAA_Packets(cmd, send_packetz);
		g_Aimbot.Update_LBY_Breaker();
		g_Aimbot.LegitAA_Run(cmd, send_packetz);
		g_Aimbot.FakeBackwards(cmd);

		BunnyHop::Get().Knifebot(cmd);

		BunnyHop::Get().JumpBug(cmd);
		//BunnyHop::Get().EBUG(cmd);

		const float unpredicted_velocity = std::floor(g_LocalPlayer->m_vecVelocity().z);
		//BunnyHop::Get().EdgebugDetection(cmd, unpredicted_velocity);
		
		if (g_Options.movement_recorder_type == 0)
		{
			movementRecorder::MovementRecord(cmd);
			movementRecorder::MovementPlay(cmd);
		}
		else
		{
			if (cmd->buttons != 0 && p.playback_active())
			{
				r.start_rerecording(p.get_current_frame());
				p.stop_playback();
			}

			r.start_recording(cmd);
			p.start_playback(cmd);
		}

		if( g_Options.esp_grenade_prediction )
		  GrenadePrediction::trace( cmd );

		Math::FixAngles( cmd->viewangles );
		cmd->viewangles.yaw = std::remainderf( cmd->viewangles.yaw, 360.0f );

		// from aimware dump
		static ConVar* m_yaw = m_yaw = g_CVar->FindVar( "m_yaw" );
		static ConVar* m_pitch = m_pitch = g_CVar->FindVar( "m_pitch" );
		static ConVar* sensitivity = sensitivity = g_CVar->FindVar( "sensitivity" );

		static QAngle m_angOldViewangles = g_ClientState->viewangles;

		float delta_x = std::remainderf( cmd->viewangles.pitch - m_angOldViewangles.pitch, 360.0f );
		float delta_y = std::remainderf( cmd->viewangles.yaw - m_angOldViewangles.yaw, 360.0f );

		if( delta_x != 0.0f ) {
		  float mouse_y = -( ( delta_x / m_pitch->GetFloat( ) ) / sensitivity->GetFloat( ) );
		  short mousedy;
		  if( mouse_y <= 32767.0f ) {
			 if( mouse_y >= -32768.0f ) {
				if( mouse_y >= 1.0f || mouse_y < 0.0f ) {
				  if( mouse_y <= -1.0f || mouse_y > 0.0f )
					 mousedy = static_cast< short >( mouse_y );
				  else
					 mousedy = -1;
				} else {
				  mousedy = 1;
				}
			 } else {
				mousedy = 0x8000u;
			 }
		  } else {
			 mousedy = 0x7FFF;
		  }

		  cmd->mousedy = mousedy;
		}

		if( delta_y != 0.0f ) {
		  float mouse_x = -( ( delta_y / m_yaw->GetFloat( ) ) / sensitivity->GetFloat( ) );
		  short mousedx;
		  if( mouse_x <= 32767.0f ) {
			 if( mouse_x >= -32768.0f ) {
				if( mouse_x >= 1.0f || mouse_x < 0.0f ) {
				  if( mouse_x <= -1.0f || mouse_x > 0.0f )
					 mousedx = static_cast< short >( mouse_x );
				  else
					 mousedx = -1;
				} else {
				  mousedx = 1;
				}
			 } else {
				mousedx = 0x8000u;
			 }
		  } else {
			 mousedx = 0x7FFF;
		  }

		  cmd->mousedx = mousedx;
		}

		auto anim_state = g_LocalPlayer->GetPlayerAnimState( );
		if( anim_state ) {
		  CCSGOPlayerAnimState anim_state_backup = *anim_state;
		  *anim_state = g_AnimState;
		  *g_LocalPlayer->GetVAngles( ) = cmd->viewangles;
		  g_LocalPlayer->UpdateClientSideAnimation( );

		  if( anim_state->speed_2d > 0.1f || std::fabsf( anim_state->flUpVelocity ) ) {
			 next_lby = g_GlobalVars->curtime + 0.22f;
		  } else if( g_GlobalVars->curtime > next_lby ) {
			 if( std::fabsf( AngleDiff( anim_state->m_flGoalFeetYaw, anim_state->m_flEyeYaw ) ) > 35.0f ) {
				next_lby = g_GlobalVars->curtime + 1.1f;
			 }
		  }

		  g_AnimState = *anim_state;
		  *anim_state = anim_state_backup;
		}

		if( *send_packet )
		{
		  real_angle = g_AnimState.m_flGoalFeetYaw;
		  view_angle = g_AnimState.m_flEyeYaw;
		}
	 }
	 EnginePrediction::End( );

	 auto old_viewangles = cmd->viewangles;
	 auto old_forwardmove = cmd->forwardmove;
	 auto old_sidemove = cmd->sidemove;

	 if (g_Options.misc_ebug && GetAsyncKeyState(g_Options.misc_ebug_key))
	 {
		 BunnyHop::Get().EdgebugRun(cmd);
	 }

	 Math::CorrectMovement(old_viewangles, cmd, old_forwardmove, old_sidemove);

	 if (g_Options.misc_ebug && GetAsyncKeyState(g_Options.misc_ebug_key))
	 {
		 BunnyHop::Get().EdgebugLock(cmd);
	 }
	 return false;
  }
  //--------------------------------------------------------------------------------
  void __fastcall override_mouse_input::hooked(void* thisptr, int edx, float* x, float* y) 
  {
	  if (BunnyHop::Get().EdgebugDetected())
	  {
		  *x *= g_Options.misc_ebuglockfactor;
		  *y *= g_Options.misc_ebuglockfactor;
	  }

	  original(thisptr, edx, x, y);
  }
  void __stdcall paint_traverse::hooked( vgui::VPANEL panel, bool forceRepaint, bool allowForce ) {
	 static auto panelId = vgui::VPANEL{ 0 };
	 static auto oPaintTraverse = original;

	 oPaintTraverse( g_VGuiPanel, panel, forceRepaint, allowForce );

	 if( !panelId ) 
	 {
		const auto panelName = g_VGuiPanel->GetName( panel );
		if( !strcmp( panelName, "FocusOverlayPanel" ) ) 
		{
		  panelId = panel;
		}
	 }
	 else if( panelId == panel ) 
	 {
		static bool bSkip = false;
		bSkip = !bSkip;

		if( bSkip )
		  return;

		//FakePrime();

		/*if (g_EngineClient->IsInGame() && g_Options.esp_enabled)
			Visuals::Get().AddToDrawList();*/

		if( g_LocalPlayer && InputSys::Get( ).IsKeyDown( VK_TAB ) && g_Options.misc_showranks )
		  Utils::RankRevealAll( );

		Render::Get( ).BeginScene( );
	 }
  }
  //--------------------------------------------------------------------------------
  void __stdcall emit_sound::hooked( IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk ) {

	 if( !strcmp( pSoundEntry, "UIPanorama.popup_accept_match_beep" ) ) {
		static auto fnAccept = reinterpret_cast< bool( __stdcall* )( const char* ) >( Utils::PatternScan( GetModuleHandleA( "client.dll" ), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12" ) );

		if( g_Options.misc_autoaccept && fnAccept ) {

		  fnAccept( "" );

		  //This will flash the CSGO window on the taskbar
		  //so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
		  FLASHWINFO fi;
		  fi.cbSize = sizeof( FLASHWINFO );
		  fi.hwnd = InputSys::Get( ).GetMainWindow( );
		  fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
		  fi.uCount = 0;
		  fi.dwTimeout = 0;
		  FlashWindowEx( &fi );
		}
	 }

	 //fix prediction sound bug
	 if (g_Options.misc_ebug && !GetAsyncKeyState(g_Options.misc_ebug_key)) return original(g_EngineSound, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

  }
  //--------------------------------------------------------------------------------
  int __stdcall post_screen_effects::hooked( int a1 )
  {
	 if( g_LocalPlayer && g_Options.glow_enabled )
		Glow::Get( ).Run( );

	 return original( g_ClientMode, a1 );
  }
  //--------------------------------------------------------------------------------
  void __stdcall override_view::hooked(CViewSetup* vsView)
  {

	 if( g_EngineClient->IsInGame( ) && vsView ) {
		if( g_Options.misc_thirdperson_bind && InputSys::Get( ).WasKeyPressed( g_Options.misc_thirdperson_bind ) )
		  g_Options.misc_thirdperson = !g_Options.misc_thirdperson;

		Visuals::Get( ).ThirdPerson( );
	 }

	 if(g_Options.misc_changefov && g_LocalPlayer && g_EngineClient->IsInGame( ) && !g_LocalPlayer->m_bIsScoped( ) ) {
		vsView->fov = g_Options.misc_override_fov;
	 }

	 if (g_Options.esp_motionblur) Visuals::Get().motionblur_render(vsView);

	 original( g_ClientMode, vsView );
  }
  //--------------------------------------------------------------------------------
  void __stdcall lock_cursor::hooked( ) {

	 if( Menu::Get( ).IsVisible( ) ) {
		g_VGuiSurface->UnlockCursor( );
		g_InputSystem->ResetInputState( );
		return;
	 }
	 original( g_VGuiSurface );
  }
  //--------------------------------------------------------------------------------
  bool __stdcall is_depth_of_field::hooked() {
	  if (g_Options.esp_motionblur) Visuals::Get().motionblur_run();
	  return false;
  }
  //--------------------------------------------------------------------------------
  void __fastcall draw_model_execute::hooked( void* pEcx, void* pEdx, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags ) {
	 bool forced_mat = !g_MdlRender->IsForcedMaterialOverride( );
	 if( forced_mat )
		Chams::Get( ).OnDrawModelExecute( pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags );

	 original( pEcx, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags );

	 if( forced_mat )
		g_MdlRender->ForcedMaterialOverride( nullptr );
  }
  //--------------------------------------------------------------------------------
  auto dwCAM_Think = Utils::PatternScan( GetModuleHandleW( L"client.dll" ), "85 C0 75 30 38 86" );
  bool __fastcall sv_cheats::hooked( PVOID pConVar, void* edx ) {
	 static auto ofunc = original;
	 if( !ofunc )
		return false;

	 if( reinterpret_cast< DWORD >( _ReturnAddress( ) ) == reinterpret_cast< DWORD >( dwCAM_Think ) )
		return true;
	 return ofunc(pConVar);
  }
  //--------------------------------------------------------------------------------
  bool __stdcall fire_event::hooked( IGameEvent* pEvent ) {
	  static auto oFireEvent = original;
	 if( !strcmp( pEvent->GetName( ), "player_death" ) && g_EngineClient->GetPlayerForUserID( pEvent->GetInt( "attacker" ) ) == g_EngineClient->GetLocalPlayer( ) ) 
	 {
		auto& weapon = g_LocalPlayer->m_hActiveWeapon( );
		if( weapon && weapon->IsWeapon( ) ) {
		  auto& skin_data = g_Options.skins.m_items[weapon->m_Item( ).m_iItemDefinitionIndex( )];
		  if( skin_data.enabled && skin_data.stat_trak ) {
			 skin_data.stat_trak++;
			 weapon->m_nFallbackStatTrak( ) = skin_data.stat_trak;
			 weapon->GetClientNetworkable( )->PostDataUpdate( 0 );
			 weapon->GetClientNetworkable( )->OnDataChanged( 0 );
		  }
		}

		if (strstr(pEvent->GetString("weapon"), "knife"))
		{
			auto& entries = g_Options.skins.m_items;
			static auto definition_vector_index = 0;
			auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
			switch (g_Options.skins_knifemodel)
			{
			case 0:
				pEvent->SetString("weapon", "knife");
				break;
			case 1:
				pEvent->SetString("weapon", "bayonet");
				break;
			case 2:
				pEvent->SetString("weapon", "knife_m9_bayonet");
				break;
			case 3:
				pEvent->SetString("weapon", "knife_karambit");
				break;
			case 4:
				pEvent->SetString("weapon", "knife_survival_bowie");
				break;
			case 5:
				pEvent->SetString("weapon", "knife_butterfly");
				break;
			case 6:
				pEvent->SetString("weapon", "knife_falchion");
				break;
			case 7:
				pEvent->SetString("weapon", "knife_flip");
				break;
			case 8:
				pEvent->SetString("weapon", "knife_gut");
				break;
			case 9:
				pEvent->SetString("weapon", "knife_tactical");
				break;
			case 10:
				pEvent->SetString("weapon", "knife_push");
				break;
			case 11:
				pEvent->SetString("weapon", "knife_gypsy_jackknife");
				break;
			case 12:
				pEvent->SetString("weapon", "knife_stiletto");
				break;
			case 13:
				pEvent->SetString("weapon", "knife_widowmaker");
				break;
			case 14:
				pEvent->SetString("weapon", "knife_ursus");
				break;
			case 15:
				pEvent->SetString("weapon", "knife_css");
				break;
			case 16:
				pEvent->SetString("weapon", "knife_skeleton");
				break;
			case 17:
				pEvent->SetString("weapon", "knife_outdoor");
				break;
			case 18:
				pEvent->SetString("weapon", "knife_canis");
				break;
			case 19:
				pEvent->SetString("weapon", "knife_cord");
				break;
			}
		}
	 }

	 if (!strcmp(pEvent->GetName(), "vote_cast") && g_Options.misc_voterevealer)
	 {
     /*Default Color (White): \x01
	 teamcolour(will be purple if message from server): \x03
	 red: \x07
	 lightred: \x0F
	 darkred: \x02
	 bluegray: \x0A
	 blue: \x0B
	 darkblue: \x0C
	 purple: \x03
	 orchid: \x0E
	 yellow: \x09
	 gold: \x10
	 lightgreen: \x05
	 green: \x04
	 lime: \x06
	 gray: \x08
	 gray2: \x0D */

		 int vote = pEvent->GetInt("vote_option");
		 int id = pEvent->GetInt("entityid");

		 //	C_BaseHudChat* chat = g_ClientMode->m_pChatElement;

		 player_info_t player;
		 g_EngineClient->GetPlayerInfo(id, &player);

		 if (player.szName) {
			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append("\x01"). //Default Color
				 append("[").
				 append("\x0B"). //Blue
				 append("Nirvana").
				 append("\x01"). //Default Color
				 append("]").
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

	 if (!strcmp(pEvent->GetName(), "show_freezepanel") && g_Options.misc_faceitfakechat)
	 {
		 int dmg_to = pEvent->GetInt("damage_given");
		 int hits_to = pEvent->GetInt("hits_given");
		 int dmg_taken = pEvent->GetInt("damage_taken");
		 int hits_taken = pEvent->GetInt("hits_taken");
	 }

	/* if (!strcmp(pEvent->GetName(), "round_end") && g_Options.misc_faceitfakechat)
	 {
		 C_BasePlayer* enemy = (C_BasePlayer*)(g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(pEvent->GetInt("userid"))));

		 auto hp = enemy->m_iHealth();
		 auto health = std::to_string(hp);

		 int id = pEvent->GetInt("entityid");

		 player_info_t player;
		 g_EngineClient->GetPlayerInfo(id, &player);

		 player_info_t playert1;
		 g_EngineClient->GetPlayerInfo(1, &playert1);

		 player_info_t playert2;
		 g_EngineClient->GetPlayerInfo(5, &playert2);

		 char pname[64];
		 g_LocalPlayer->GetClientClass();

		 g_ChatElement->ChatPrintf(0, 0, std::string("").
			 append(" \x09"). //Yellow - FIRST LINE START
			 append("[FACEIT^]").
			 append(" \x09"). //Yellow
			 append("team_").
			 append("\x09"). //Yellow
			 append(playert1.szName).
			 append(" \x09"). //Yellow
			 append("[").
			 append("\x09"). //Yellow
			 append("PLACAR - PLACAR").
			 append("\x09"). //Yellow
			 append("]").
			 append(" \x09"). //Yellow
			 append("team_").
			 append("\x09"). //Yellow
			 append(playert2.szName).c_str());

		 if (player.szName)
		 {
			 auto dmg_to_player = std::to_string(dmg_to);
			 auto hits_to_player = std::to_string(hits_to);
			 auto dmg_taken_player = std::to_string(dmg_taken);
			 auto hits_taken_player = std::to_string(hits_taken);

			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append(" \x09"). //Yellow - SECOND LINE START
				 append("[FACEIT^]").
				 append(" \x09").
				 append("To:").
				 append(" \x09").
				 append("[").
				 append(dmg_to_player).
				 append(" \x09").
				 append("/").
				 append(" \x09").
				 append(hits_to_player + " hits").
				 append("]").
				 append(" \x09").
				 append("From:").
				 append(" \x09").
				 append("[").
				 append(dmg_taken_player).
				 append(" \x09").
				 append("/").
				 append(" \x09").
				 append(hits_taken_player + " hits").
				 append("]").
				 append(" \x09").
				 append("-").
				 append(" \x09").
				 append(player.szName).
				 append(" \x09").
				 append("(").
				 append(health.c_str()).
				 append(" \x09").
				 append("hp").
				 append(")").c_str());
		 }

	 }*/

	 if (!strcmp(pEvent->GetName(), "bomb_beginplant") && g_Options.esp_bomblogs)
	 {
		 int id = pEvent->GetInt("userid");
		 int bombsite = pEvent->GetInt("site");

		 player_info_t player;
		 g_EngineClient->GetPlayerInfo(id, &player);

		 if (bombsite == 0)
		 {
			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append("\x01"). //Default Color
				 append("[").
				 append("\x0B"). //Blue
				 append("Nirvana").
				 append("\x01"). //Default Color
				 append("]").
				 append(" \x01"). //Default Color
				 append("The").
				 append(" \x02"). //Red
				 append("Bomb").
				 append(" \x01"). //Default Color
				 append("is being").
				 append(" \x0A"). //Light Blue
				 append("Planted").
				 append(" \x01"). //Default Color
				 append("at the").
				 append(" \x0C"). //Blue
				 append("Bombsite A").
				 append("\x01"). //Default Color
				 append(".").c_str());
		 }
		 else if (bombsite == 1)
		 {
			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append("\x01"). //Default Color
				 append("[").
				 append("\x0B"). //Blue
				 append("Nirvana").
				 append("\x01"). //Default Color
				 append("]").
				 append(" \x01"). //Default Color
				 append("The").
				 append(" \x02"). //Red
				 append("Bomb").
				 append(" \x01"). //Default Color
				 append("is being").
				 append(" \x0A"). //Light Blue
				 append("Planted").
				 append(" \x01"). //Default Color
				 append("at the").
				 append(" \x0C"). //Blue
				 append("Bombsite B").
				 append("\x01"). //Default Color
				 append(".").c_str());
		 }
		 else
		 {
			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append("\x01"). //Default Color
				 append("[").
				 append("\x0B"). //Blue
				 append("Nirvana").
				 append("\x01"). //Default Color
				 append("]").
				 append(" \x01"). //Default Color
				 append("The").
				 append(" \x02"). //Red
				 append("Bomb").
				 append(" \x01"). //Default Color
				 append("is being").
				 append(" \x0A"). //Light Blue
				 append("Planted").
				 append("\x01"). //Default Color
				 append(".").c_str());
		 }
	 }

	 if (!strcmp(pEvent->GetName(), "bomb_abortplant") && g_Options.esp_bomblogs)
	 {
		 int id = pEvent->GetInt("userid");
		 int bombsite = pEvent->GetInt("site");

		 player_info_t player;
		 g_EngineClient->GetPlayerInfo(id, &player);

		 g_ChatElement->ChatPrintf(0, 0, std::string("").
			 append("\x01"). //Default Color
			 append("[").
			 append("\x0B"). //Blue
			 append("Nirvana").
			 append("\x01"). //Default Color
			 append("]").
			 append(" \x01"). //Default Color
			 append("The").
			 append(" \x02"). //Red
			 append("Bomb").
			 append(" \x0A"). //Light Blue
			 append("Planting").
			 append(" \x01"). //Default Color
			 append("has been").
			 append(" \x04"). //Green
			 append("Aborted").
			 append("\x01"). //Default Color
			 append(".").c_str());
	 }

	 if (!strcmp(pEvent->GetName(), "bomb_begindefuse") && g_Options.esp_bomblogs)
	 {
		 int id = pEvent->GetInt("userid");
		 bool haskit = pEvent->GetBool("haskit");

		 player_info_t player;
		 g_EngineClient->GetPlayerInfo(id, &player);

		 if (haskit)
		 {
			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append("\x01"). //Default Color
				 append("[").
				 append("\x0B"). //Blue
				 append("Nirvana").
				 append("\x01"). //Default Color
				 append("]").
				 append(" \x01"). //Default Color
				 append("The").
				 append(" \x02"). //Red
				 append("Bomb").
				 append(" \x01"). //Default Color
				 append("is being").
				 append(" \x0A"). //Light Blue
				 append("Defused").
				 append(" \x04"). //Green
				 append("WITH").
				 append(" \x01"). //Default Color
				 append("a").
				 append(" \x0C"). //Dark Blue
				 append("Defuse Kit").
				 append("\x01"). //Default Color
				 append(".").c_str());
		 }
		 else
		 {
			 g_ChatElement->ChatPrintf(0, 0, std::string("").
				 append("\x01"). //Default Color
				 append("[").
				 append("\x0B"). //Blue
				 append("Nirvana").
				 append("\x01"). //Default Color
				 append("]").
				 append(" \x01"). //Default Color
				 append("The").
				 append(" \x02"). //Red
				 append("Bomb").
				 append(" \x01"). //Default Color
				 append("is being").
				 append(" \x0A"). //Light Blue
				 append("Defused").
				 append(" \x02"). //Red
				 append("WITHOUT").
				 append(" \x01"). //Default Color
				 append("a").
				 append(" \x0C"). //Blue
				 append("Defuse Kit").
				 append("\x01"). //Default Color
				 append(".").c_str());
		 }
	 }

	 if (!strcmp(pEvent->GetName(), "bomb_abortdefuse") && g_Options.esp_bomblogs)
	 {
		 int id = pEvent->GetInt("userid");

		 player_info_t player;
		 g_EngineClient->GetPlayerInfo(id, &player);

		 g_ChatElement->ChatPrintf(0, 0, std::string("").
			 append("\x01"). //Default Color
			 append("[").
			 append("\x0B"). //Blue
			 append("Nirvana").
			 append("\x01"). //Default Color
			 append("]").
			 append(" \x01"). //Default Color
			 append("The").
			 append(" \x02"). //Red
			 append("Bomb").
			 append(" \x0A"). //Light Blue
			 append("Defusing").
			 append(" \x01"). //Default Color
			 append("has been").
			 append(" \x04"). //Green
			 append("Aborted").
			 append("\x01"). //Default Color
			 append(".").c_str());
	 }

	// vote_cast_event::Get().FireGameEvent(pEvent);
	 Visuals::Get().FireGameEvent(pEvent);
	// CallVoteEvent::Get().FireGameEvent(pEvent);

	 return oFireEvent( g_GameEvents, pEvent );
  }
  //--------------------------------------------------------------------------------
  static auto random_sequence(const int low, const int high) -> int 
  {
	  return rand() % (high - low + 1) + low;
  }

  static auto fix_animation(const char* model, const int sequence) -> int {
	  enum ESequence
	  {
		  SEQUENCE_DEFAULT_DRAW = 0,
		  SEQUENCE_DEFAULT_IDLE1 = 1,
		  SEQUENCE_DEFAULT_IDLE2 = 2,
		  SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
		  SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
		  SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
		  SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
		  SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
		  SEQUENCE_DEFAULT_LOOKAT01 = 12,
		  SEQUENCE_BUTTERFLY_DRAW = 0,
		  SEQUENCE_BUTTERFLY_DRAW2 = 1,
		  SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
		  SEQUENCE_BUTTERFLY_LOOKAT03 = 15,
		  SEQUENCE_FALCHION_IDLE1 = 1,
		  SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
		  SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
		  SEQUENCE_FALCHION_LOOKAT01 = 12,
		  SEQUENCE_FALCHION_LOOKAT02 = 13,
		  SEQUENCE_DAGGERS_IDLE1 = 1,
		  SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
		  SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
		  SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
		  SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,
		  SEQUENCE_BOWIE_IDLE1 = 1,
		  SEQUENCE_CSS_LOOKAT01 = 12,
		  SEQUENCE_CSS_LOOKAT02 = 15,
		  SEQUENCE_TALON_LOOKAT1 = 14,
		  SEQUENCE_TALON_LOOKAT2 = 15
	  };
	  if (strstr(model, "models/weapons/v_knife_butterfly.mdl")) {
		  switch (sequence) {
		  case SEQUENCE_DEFAULT_DRAW:
			  return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
		  default:
			  return sequence + 1;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_falchion_advanced.mdl")) {
		  switch (sequence) {
		  case SEQUENCE_DEFAULT_IDLE2:
			  return SEQUENCE_FALCHION_IDLE1;
		  case SEQUENCE_DEFAULT_HEAVY_MISS1:
			  return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
		  case SEQUENCE_DEFAULT_DRAW:
		  case SEQUENCE_DEFAULT_IDLE1:
			  return sequence;
		  default:
			  return sequence - 1;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_push.mdl")) {
		  switch (sequence) {
		  case SEQUENCE_DEFAULT_IDLE2:
			  return SEQUENCE_DAGGERS_IDLE1;
		  case SEQUENCE_DEFAULT_LIGHT_MISS1:
		  case SEQUENCE_DEFAULT_LIGHT_MISS2:
			  return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
		  case SEQUENCE_DEFAULT_HEAVY_MISS1:
			  return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
		  case SEQUENCE_DEFAULT_HEAVY_HIT1:
		  case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return sequence + 3;
		  case SEQUENCE_DEFAULT_DRAW:
		  case SEQUENCE_DEFAULT_IDLE1:
			  return sequence;
		  default:
			  return sequence + 2;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_ursus.mdl"))
	  {
		  switch (sequence) {
		  case SEQUENCE_DEFAULT_DRAW:
			  return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		  default:
			  return sequence + 1;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_css.mdl"))
	  {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(15, 15);
		  default:
			  return sequence;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_stiletto.mdl"))
	  {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(12, 13);
		  default:
			  return sequence;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_widowmaker.mdl"))
	  {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(14, 15);
		  default:
			  return sequence;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_skeleton.mdl"))
	  {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_DRAW:
			  return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		  default:
			  return sequence + 1;
		  }

	  }
	  else if (strstr(model, "models/weapons/v_knife_outdoor.mdl"))
	  {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_DRAW:
			  return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		  default:
			  return sequence + 1;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_canis.mdl")) {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_DRAW:
			  return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		  default:
			  return sequence + 1;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_cord.mdl")) {
		  switch (sequence)
		  {
		  case SEQUENCE_DEFAULT_DRAW:
			  return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		  case SEQUENCE_DEFAULT_LOOKAT01:
			  return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		  default:
			  return sequence + 1;
		  }
	  }
	  else if (strstr(model, "models/weapons/v_knife_survival_bowie.mdl"))
	  {
		  switch (sequence) {
		  case SEQUENCE_DEFAULT_DRAW:
		  case SEQUENCE_DEFAULT_IDLE1:
			  return sequence;
		  case SEQUENCE_DEFAULT_IDLE2:
			  return SEQUENCE_BOWIE_IDLE1;
		  default:
			  return sequence - 1;
		  }
	  }
	  else {
		  return sequence;
	  }
  }

  void sequence::hooked( const CRecvProxyData* pData, void* entity, void* output ) 
  {
	 static auto original_fn = sequence_hook->get_original_function( );
	 const auto local = static_cast< C_BasePlayer* >( g_EntityList->GetClientEntity( g_EngineClient->GetLocalPlayer( ) ) );
	 if( local && local->IsAlive( ) ) 
	 {
		const auto proxy_data = const_cast< CRecvProxyData* >( pData );
		const auto view_model = static_cast< C_BaseViewModel* >( entity );
		if( view_model && view_model->m_hOwner( ) && view_model->m_hOwner( ).IsValid( ) ) 
		{
		  const auto owner = static_cast< C_BasePlayer* >( g_EntityList->GetClientEntityFromHandle( view_model->m_hOwner( ) ) );
		  if( owner == g_EntityList->GetClientEntity( g_EngineClient->GetLocalPlayer( ) ) ) 
		  {
			 const auto view_model_weapon_handle = view_model->m_hWeapon( );
			 if( view_model_weapon_handle.IsValid( ) ) 
			 {
				const auto view_model_weapon = static_cast< C_BaseAttributableItem* >( g_EntityList->GetClientEntityFromHandle( view_model_weapon_handle ) );
				if( view_model_weapon ) 
				{
				  if( k_weapon_info.count( view_model_weapon->m_Item( ).m_iItemDefinitionIndex( ) ) ) 
				  {
					 auto original_sequence = proxy_data->m_Value.m_Int;
					 const auto override_model = k_weapon_info.at( view_model_weapon->m_Item( ).m_iItemDefinitionIndex( ) ).model;
					 proxy_data->m_Value.m_Int = fix_animation( override_model, proxy_data->m_Value.m_Int );
				  }
				}
			 }
		  }
		}
	 }
	 original_fn( pData, entity, output );
  }
  //--------------------------------------------------------------------------------
  struct RenderableInfo_t {
	 IClientRenderable* m_pRenderable;
	 void* m_pAlphaProperty;
	 int m_EnumCount;
	 int m_nRenderFrame;
	 unsigned short m_FirstShadow;
	 unsigned short m_LeafList;
	 short m_Area;
	 uint16_t m_Flags;   // 0x0016
	 uint16_t m_Flags2; // 0x0018
	 Vector m_vecBloatedAbsMins;
	 Vector m_vecBloatedAbsMaxs;
	 Vector m_vecAbsMins;
	 Vector m_vecAbsMaxs;
	 int pad;
  };

#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )
  int __fastcall list_leaves_in_box::hooked( void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax ) {
	 static auto ofunc = original;

	 // occulusion getting updated on player movement/angle change,
	 // in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
	 // check for return in CClientLeafSystem::InsertIntoTree
	 if( !g_Options.chams_player_enabled || !g_Options.chams_disable_occulusion || *( uint32_t* )_ReturnAddress( ) != 0x8B087D8B )
		return ofunc( bsp, mins, maxs, pList, listMax );

	// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
	 auto info = *( RenderableInfo_t** )( ( uintptr_t )_AddressOfReturnAddress( ) + 0x14 );
	 if( !info || !info->m_pRenderable )
		return ofunc( bsp, mins, maxs, pList, listMax );

	// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
	 auto base_entity = info->m_pRenderable->GetIClientUnknown( )->GetBaseEntity( );
	 if( !base_entity || !base_entity->IsPlayer( ) )
		return ofunc( bsp, mins, maxs, pList, listMax );

	// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
	 static const Vector map_min = Vector( MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT );
	 static const Vector map_max = Vector( MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT );
	 auto count = ofunc( bsp, map_min, map_max, pList, listMax );
	 return count;
  }
  //--------------------------------------------------------------------------------
  void __stdcall frame_stage_notify::hooked(ClientFrameStage_t stage)
  {
	  if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	  {
		  legitresolver.FrameStageNotify(stage);
		  Skins::OnFrameStageNotify(false);

		  if (g_EngineClient->IsInGame())
		  {
			  Skins::AgentChanger(stage);
		  }
	  }
	  if (stage == FRAME_NET_UPDATE_END)
	  {
		  Skins::OnFrameStageNotify(true);
	  }

	  original(g_CHLClient, stage);
  }
  void __stdcall suppress_list::hooked( int a2, bool a3) {
	  static auto ofunc = original;

	 static auto OnRenderStart_Return = Utils::PatternScan( GetModuleHandleA( "client.dll" ), "FF 50 40 8B 1D ? ? ? ?" ) + 0x3;
	 static auto FrameNetUpdateEnd_Return = Utils::PatternScan( GetModuleHandleA( "client.dll" ), "5F 5E 5D C2 04 00 83 3D ? ? ? ? ?" );

	 if( g_LocalPlayer && g_LocalPlayer->IsAlive( ) ) {
		if( _ReturnAddress( ) == OnRenderStart_Return )
		{
		  if (g_LocalPlayer->GetPlayerAnimState())
			  g_LocalPlayer->SetAbsAngles(QAngle(0.0f, g_LocalPlayer->GetPlayerAnimState()->m_flGoalFeetYaw, 0.0f));
		  auto state = g_LocalPlayer->GetPlayerAnimState();
		  if (state)
			  if (g_LocalPlayer->m_vecVelocity().Length2D() < 3.f)
				  *reinterpret_cast<float*> (reinterpret_cast<uintptr_t> (state) + 292) = 0.f;

		}
		else if( _ReturnAddress( ) == FrameNetUpdateEnd_Return )
		{
		  for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
		  {
			  auto ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
			  if (ent && ent->IsAlive() && !ent->IsDormant() && ent != g_LocalPlayer)
			  {
				  //ResolverLegit(ent);
				  auto VarMap = reinterpret_cast<uintptr_t>(ent) + 36;
				  auto VarMapSize = *reinterpret_cast<int*>(VarMap + 20);

				  for (auto index = 0; index < VarMapSize; index++)
					  *reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(VarMap) + index * 12) = 0;
			  }
		  }
		}
	 }

	 ofunc( g_SpatialPartition, a2, a3 );
  }
  void __fastcall post_entity_packet_received::hooked( void * pPrediction, void * pEDX ) {
	 //FRAME_NET_UPDATE_POSTDATAUPDATE_END
	 static auto FramePostDataUpdateEnd = Utils::PatternScan( GetModuleHandleA( "client.dll" ), "FF 50 14 5F C6 05 ? ? ? ? ?" ) + 0x3;

	 //if( _ReturnAddress( ) == FramePostDataUpdateEnd )

	 original( pPrediction );
  }
  void __fastcall read_packet_entities::hooked( void * pClientState, void * pEDX, void * pEntInfo ) {

	 original( pClientState, pEntInfo );
  }
  //--------------------------------------------------------------------------------
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
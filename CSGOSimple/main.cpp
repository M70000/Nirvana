#define NOMINMAX
#include <Windows.h>

#include "config.hpp"
#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"

#include "hooks.hpp"
#include "menu.hpp"
#include "security.hpp"
#include "options.hpp"
#include "render.hpp"
#include "AntiLeak.h"
#include "DiscordRPC.hpp"

bool ErasePEHeader(HMODULE hModule)
{
	hModule = GetModuleHandle(NULL);
	if ((DWORD)hModule == 0) return 0;
	DWORD IMAGE_NT_HEADER = *(int*)((DWORD)hModule + 60);
	for (int i = 0; i < 0x108; i++)
		*(BYTE*)(IMAGE_NT_HEADER + i) = 0;
	for (int i = 0; i < 120; i++)
		*(BYTE*)((DWORD)hModule + i) = 0;
	return 1;
}

void CAntiLeak::ErasePE()
{
	char* pBaseAddr = (char*)GetModuleHandle(NULL);
	DWORD dwOldProtect = 0;
	VirtualProtect(pBaseAddr, 4096, PAGE_READWRITE, &dwOldProtect);
	ZeroMemory(pBaseAddr, 4096);
	VirtualProtect(pBaseAddr, 4096, dwOldProtect, &dwOldProtect);
}

DWORD WINAPI OnDllAttach( LPVOID base ) 
{
	if (Utils::WaitForModules(10000, { L"client.dll", L"engine.dll", L"shaderapidx9.dll" }) == WAIT_TIMEOUT) {
		return FALSE;
	}

	try {
		/*LPVOID ntOpenFile = GetProcAddress(LoadLibraryW(L"ntdll"), "NtOpenFile");
if (ntOpenFile) {
	char originalBytes[5];
	memcpy(originalBytes, ntOpenFile, 5);
	WriteProcessMemory("csgo", ntOpenFile, originalBytes, 5, NULL);
}*/
		//Config->Setup();
		Utils::ConsolePrint("Initializing...\n");

		Interfaces::Initialize();
		Interfaces::Dump();

		NetvarSys::Get().Initialize();
		InputSys::Get().Initialize();
		Render::Get().Initialize();
		Menu::Get().Initialize();

		Hooks::Initialize();

		// Menu Toggle
		InputSys::Get().RegisterHotkey(VK_INSERT, [base]() {
			Menu::Get().Toggle();
		});

		Utils::ConsolePrint("Finished.\n");

		while (!g_Unload)
			Sleep(1000);

		FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
		AntiLeak->ErasePE();

	}
	catch (const std::exception& ex) {
		Utils::ConsolePrint("An error occured during initialization:\n");
		Utils::ConsolePrint("%s\n", ex.what());
		Utils::ConsolePrint("Press any key to exit.\n");
		Utils::ConsoleReadKey();
		Utils::DetachConsole();

		FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
		AntiLeak->ErasePE();
	}

	// unreachable
	//return TRUE;
}

BOOL WINAPI OnDllDetach( ) {
#ifdef _DEBUG
	Utils::DetachConsole( );
#endif

	Hooks::Shutdown( );

	Menu::Get( ).Shutdown( );

	return TRUE;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDll,_In_ DWORD fdwReason, _In_opt_ LPVOID lpvReserved) 
{
	switch( fdwReason ) 
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls( hinstDll );
		CreateThread( nullptr, 0, OnDllAttach, hinstDll, 0, nullptr );

		return TRUE;

	case DLL_PROCESS_DETACH:
		if( lpvReserved == nullptr )
			return OnDllDetach( );

		return TRUE;

	default:
		return TRUE;
	}
}





































































































































































































































































































































































































































































































































































































































































































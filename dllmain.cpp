// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include "mem.h"
#include "proc.h"
#include <vector>

// Game offsets and vars0
struct gameOffsets {
	DWORD localPlayer = 0xD28B1C;
	DWORD velocity = 0x114;
	DWORD forceJump = 0x51DFEAC;
	DWORD flag = 0x104;
}Offsets;

struct gameVars {
	DWORD LocalPlayerPTR;
	DWORD ModuleBase;
	BYTE flag;
}Vars;

struct hackToggles {
	bool bhopToggle = false;
}Toggles;

struct vector {
	float x, y, z;
};

bool isPlayerMoving() {
	// Get velocity
	vector VelocityVector = *(vector*)(Vars.LocalPlayerPTR + Offsets.velocity);
	int Velocity = (int)VelocityVector.x + VelocityVector.y + VelocityVector.z;

	// If player is moving return true
	if (Velocity != 0)
		return true;
	else    // Return false otherwise
		return false;
}

// Main hack loop
void hackLoop() {
	while (true) {
		// Toggle hack options when key pressed
		// Bhop Toggle
		if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
			Toggles.bhopToggle = !Toggles.bhopToggle;

			if (Toggles.bhopToggle == 1)
				std::cout << "[+] Toggled bhop: True" << std::endl;
			else if (Toggles.bhopToggle == 0)
				std::cout << "[+] Toggled bhop: False" << std::endl;
		}
		// Exit Toggle
		if (GetAsyncKeyState(VK_NUMPAD0) & 1) {
			std::cout << "[*] Stopping loop..." << std::endl;
			break;
		}

		// Find Local Player PTR if anything changes
		Vars.LocalPlayerPTR = *(DWORD*)(Vars.ModuleBase + Offsets.localPlayer);

		// Get flags
		Vars.flag = *(BYTE*)(Vars.LocalPlayerPTR + Offsets.flag);

		// If bhop is toggled
		if (Toggles.bhopToggle) {
			// If Velocity is 0
			if (isPlayerMoving()) {
				// If Space is pressed and player is jumping
				if (GetAsyncKeyState(VK_SPACE) && Vars.flag & (1 << 0)) {
					// Force jump
					*(DWORD*)(Vars.ModuleBase + Offsets.forceJump) = 6;
				}
			}
		}

		//Sleep(1);
	}
}

// Main hack thread
DWORD WINAPI HackThread(HMODULE hModule) {
	// Init Console
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "[+] Created Console" << std::endl;

	std::cout << "[*] Finding Module Address..." << std::endl;
	// Get module base address
	while (!Vars.ModuleBase) {
		Vars.ModuleBase = (uintptr_t)GetModuleHandle((LPCSTR)"client_panorama.dll");
		Sleep(500);
	}
	std::cout << "[+] Found module address (client_panorama.dll) -> 0x" << std::hex << Vars.ModuleBase << std::endl;

	// Get local player address
	std::cout << "[*] Finding Local Player Address..." << std::endl;
	if (Vars.LocalPlayerPTR == NULL)
		while (Vars.LocalPlayerPTR == NULL)
			Vars.LocalPlayerPTR = *(DWORD*)(Vars.ModuleBase + Offsets.localPlayer);
	std::cout << "[+] Found LocalPlayer PTR -> 0x" << std::hex << Vars.LocalPlayerPTR << std::endl;

	// Init main hack loop()
	std::cout << "[*] Starting main hack loop" << std::endl;
	hackLoop();

	// Clean & Eject
	std::cout << "[*] Cleaning up..." << std::endl;
	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// Create hack thread
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
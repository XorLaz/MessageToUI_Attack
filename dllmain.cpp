#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <cstdio>
#include "MinHook/include/MinHook.h"
#include "RecvHook.h"


// 2026 7.11 增加了一些安全性检查和准确性  


static DWORD WINAPI InitThread(LPVOID) {
    // AllocConsole();
     //FILE* dummy = nullptr;
     //freopen_s(&dummy, "CONOUT$", "w", stdout);
     //freopen_s(&dummy, "CONOUT$", "w", stderr);
     //SetConsoleOutputCP(CP_UTF8);
     std::printf("=== recv anti-spam filter loaded ===\n");

     HMODULE ws2 = GetModuleHandleA("ws2_32.dll");
     if (!ws2) { std::printf("[AntiSpam] ws2_32.dll not loaded\n"); return 1; }

     FARPROC recvAddr = GetProcAddress(ws2, "recv");
     if (!recvAddr) { std::printf("[AntiSpam] recv not found\n"); return 1; }

     if (MH_Initialize() != MH_OK) {
          std::printf("[AntiSpam] MH_Initialize failed\n"); return 1;
     }
     if (MH_CreateHook(reinterpret_cast<LPVOID>(recvAddr), &HookedRecv,
          reinterpret_cast<void**>(&g_originalRecv)) != MH_OK) {
          std::printf("[AntiSpam] MH_CreateHook failed\n"); return 1;
     }
     if (MH_EnableHook(reinterpret_cast<LPVOID>(recvAddr)) != MH_OK) {
          std::printf("[AntiSpam] MH_EnableHook failed\n"); return 1;
     }
     std::printf("[AntiSpam] recv hooked OK\n");
     return 0;
}

static void Shutdown() {
     MH_DisableHook(MH_ALL_HOOKS);
     MH_Uninitialize();
}



BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
     switch (reason) {
     case DLL_PROCESS_ATTACH:
          DisableThreadLibraryCalls(hModule);
          CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
          break;
     case DLL_PROCESS_DETACH:
          if (lpReserved == nullptr) Shutdown();
          break;
     }
     return TRUE;
}
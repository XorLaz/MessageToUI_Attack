#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include "MinHook/include/MinHook.h"
#include "Bypass.h"

//  By XorLaz(小懒仔)  QQ 2499464524
 // 2025 .11.30

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {

         AllocConsole();
         freopen(("CON"), "w", stdout);  // 重定向标准输出
         freopen(("CON"), "w", stderr);  // 重定向标准错误输出
         freopen(("CON"), "r", stdin);   // 重定向标准输入
         SetConsoleOutputCP(CP_UTF8);

	     HMODULE hModule = GetModuleHandleA("ws2_32.dll");
         //printf("hModule %X\n", (int)hModule);
         FARPROC recv_addr = GetProcAddress(hModule, "recv");
         //printf("recv_addr %X\n", (int)recv_addr);

		 printf("       Anti Spam messages Begin !!! \n\n\n");

         MH_Initialize();
         MH_CreateHook(recv_addr, HookedRecv, (void**)&OriginalRecv);
         MH_EnableHook(recv_addr);

    }

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


//  By XorLaz(小懒仔)  QQ 2499464524

 // 2025 .11.30

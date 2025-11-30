#pragma once
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

int WINAPI HookedRecv(SOCKET s, char* buf, int len, int flags);
//  By XorLaz(小懒仔)  QQ 2499464524
 // 2025 .11.30
typedef int (WINAPI* pRecv)(SOCKET, char*, int, int);
extern pRecv OriginalRecv;
#pragma once
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

using RecvFn = int (WINAPI*)(SOCKET, char*, int, int);


extern RecvFn g_originalRecv;

int WINAPI HookedRecv(SOCKET s, char* buf, int len, int flags);
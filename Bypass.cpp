#include "Bypass.h"
#include <WinSock2.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

//  By XorLaz(小懒仔)  QQ 2499464524
 // 2025 .11.30

typedef int (WINAPI* pRecv)(SOCKET, char*, int, int);
pRecv OriginalRecv;


bool IsUtf8Chinese(const unsigned char* p) {
	return (p[0] >= 0xE4 && p[0] <= 0xE9) &&
		(p[1] >= 0x80 && p[1] <= 0xBF) &&
		(p[2] >= 0x80 && p[2] <= 0xBF);
}

bool ContainsChinese(const char* data, int len) {
	for (int i = 0; i < len - 2; i++) {
		unsigned char c = data[i];

		// 中文都是 3 字节，leading byte = E4~E9
		if (c >= 0xE4 && c <= 0xE9) {
			if (i + 2 < len && IsUtf8Chinese((unsigned char*)&data[i])) {
				return true;
			}
			i += 2; // 跳过剩余 continuation bytes
		}
	}
	return false;
}





DWORD lastChatTime = 0;
std::string lastMessage = "";
int WINAPI HookedRecv(SOCKET s, char* buf, int len, int flags)
{
	int ret = OriginalRecv(s, buf, len, flags);

	if (ret == -1)   // 排除一下错误信息
		return ret;

	std::string str(buf, ret);

	if (lastMessage == str) {
		std::cout << "Duplicate information detected" << std::endl;
		return 0;
	}


	int starCount = 0;
	for (char c : str) {
		if (c == '*') starCount++;
	}

	// 如果星号超过40个，屏蔽消息
	if (starCount > 40) {

		std::cout << "Spam" << starCount << "*" << std::endl;
		return 0;
	}



	// 判定为聊天消息  可能性较高
	if (ContainsChinese(buf, 30)) {

		DWORD now = GetTickCount();


		// 0.5 秒内重复消息   这里是判断刷屏   时长是500毫秒 也可以改小一些 
		if (now - lastChatTime < 500)
		{
			std::cout << "Spam Detected (duplicate message within 0.2s)\n";
			return 0; 
		}


		lastChatTime = now;


	}



	//printf("buf:%X  length:%d\n", buf, ret);
	//std::cout << "text: " << str << "\n\n\n" << std::endl;


	lastMessage = str;

	return ret;

}


//  By XorLaz(小懒仔)  QQ 2499464524

 // 2025 .11.30

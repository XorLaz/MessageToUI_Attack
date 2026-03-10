#include "Bypass.h"
#include <WinSock2.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

//  By XorLaz(小懒仔)  QQ 2499464524
 // 2026 .3.10

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

bool HasRepeatingChars(const std::string& str, int threshold = 15) {
	int consecutiveCount = 1;

	for (size_t i = 1; i < str.length(); i++) {
		char current = str[i];
		char previous = str[i - 1];

		// 只检测英文字母 a-z A-Z
		if ((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z')) {
			if ((previous >= 'a' && previous <= 'z') || (previous >= 'A' && previous <= 'Z')) {
				// 都是字母，检查是否相同
				if (current == previous) {
					consecutiveCount++;

					if (consecutiveCount >= threshold) {
						return true;
					}
				}
				else {
					consecutiveCount = 1;
				}
			}
			else {
				consecutiveCount = 1;
			}
		}
		else {
			consecutiveCount = 1;
		}
	}

	return false;
}



DWORD lastChatTime = 0;
std::string lastMessage = "";
int lastLedn;
std::string lastPlayerName = "";
int WINAPI HookedRecv(SOCKET s, char* buf, int len, int flags)
{
    int ret = OriginalRecv(s, buf, len, flags);

    if (ret == -1)   // 排除一下错误信息
        return ret;

    std::string str(buf + 0x1A, len);

    if(str.find("hhhhhhh") != std::string::npos || str.find("HHHHHHH") != std::string::npos)
    {
        std::cout << "Going viral: hhhhh\n";
        return -1;
    }


	    if (HasRepeatingChars(str, 15)) {
		      std::cout << "Going viral: Others\n";
		       return -1;
     	}

    // 判定为聊天消息  可能性较高
    if (ContainsChinese(buf + 0xC, 14)) {
        std::string PlayerName(buf + 0xC, 14);
        DWORD now = GetTickCount();

        // 0.05 秒内重复消息   这里是判断刷屏   时长是0.05毫秒 也可以改小一些 
        if (now - lastChatTime < 50 && lastPlayerName == PlayerName)
        {
            std::cout << "Spam Detected (duplicate message within 0.05s)\n";
            return -1;
        }


        lastChatTime = now;
        lastPlayerName = PlayerName;

    }



    //这里是判断上次消息和现在消息是否相同  如果相同就屏蔽掉 但是感觉没啥必要了
    if (lastMessage == str && lastLedn == len && ContainsChinese(buf + 0xC, 14)) {
        std::cout << "Duplicate information detected" << std::endl;
        return -1;
    }



    //printf("buf:%X  length:%d\n", buf, ret);
    //std::cout << "text: " << str << "\n\n\n" << std::endl;

    lastLedn = len;
    lastMessage = str;

    return ret;

}

//  By XorLaz(小懒仔)  QQ 2499464524
 // 2026 .3.10



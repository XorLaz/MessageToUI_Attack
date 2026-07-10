#include "RecvHook.h"
#include "Protocol.h"
#include "SpamFilter.h"
#include <windows.h>
#include <cstdio>
#include <cstring>

RecvFn g_originalRecv = nullptr;

static spam::Filter g_filter; 

static const char* ReasonName(spam::Reason r) {
     switch (r) {
     case spam::Reason::RateLimited:    return "rate-limited";
     case spam::Reason::DuplicateFlood: return "duplicate flood";
     case spam::Reason::RepeatedChars:  return "repeated chars";
     case spam::Reason::LowEntropy:     return "low entropy";
     case spam::Reason::TooLong:        return "too long";
     default:                           return "none";
     }
}

static void NeutralizeMessage(const protocol::ChatFrame& f) {
     if (f.messageData && f.messageLen > 0) {
          std::memset(f.messageData, ' ', static_cast<size_t>(f.messageLen));
     }
}

int WINAPI HookedRecv(SOCKET s, char* buf, int len, int flags) {
     int ret = g_originalRecv(s, buf, len, flags);

     // 切勿处理：错误 SOCKET_ERROR    正常关闭 (0) 或任何长度不足以构成聊天帧的数据。
     if (ret <= 0) return ret;

     auto frame = protocol::ParseChat(buf, ret); // 经过边界检查；可能为空
     if (!frame) return ret;                        // 未聊天 -> 未触碰

     spam::Reason why = g_filter.Check(frame->sender, frame->message,
          GetTickCount64());
     if (why != spam::Reason::None) {
          NeutralizeMessage(*frame);
          std::printf("[AntiSpam] blocked (%s)\n", ReasonName(why));
     }

     // 返回实际的字节数
     return ret;
}
#include "Protocol.h"

namespace protocol {

     // 这里我他妈用的是启发式替代方法  此固定字段是否包含一个 3 字节的 UTF-8 CJK 字符？
     static bool FieldLooksLikeText(const char* p, int len) {
          for (int i = 0; i + 2 < len; ++i) {
               unsigned char c0 = static_cast<unsigned char>(p[i]);
               unsigned char c1 = static_cast<unsigned char>(p[i + 1]);
               unsigned char c2 = static_cast<unsigned char>(p[i + 2]);
               if (c0 >= 0xE4 && c0 <= 0xE9 &&
                    c1 >= 0x80 && c1 <= 0xBF &&
                    c2 >= 0x80 && c2 <= 0xBF) {
                    return true;
               }
          }
          return false;
     }

     bool IsChatPacket(const char* buf, int received) {
          // 必须至少包含头部、全名字段以及一个消息字节。
          if (received <= kMessageOffset) return false;
          return FieldLooksLikeText(buf + kSenderNameOffset, kSenderNameLen);
     }

     std::optional<ChatFrame> ParseChat(char* buf, int received) {
          if (!IsChatPacket(buf, received)) return std::nullopt;

          // 在第一个 NUL 字符处截断名称，以确保同一玩家始终对应相同的键值。
          int nameLen = 0;
          while (nameLen < kSenderNameLen &&
               buf[kSenderNameOffset + nameLen] != '\0') {
               ++nameLen;
          }

          ChatFrame f;
          f.sender = std::string_view(buf + kSenderNameOffset, nameLen);
          f.messageData = buf + kMessageOffset;
          f.messageLen = received - kMessageOffset;   // 假设每次接收一帧
          f.message = std::string_view(f.messageData, f.messageLen);
          return f;
     }

}
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <mutex>

namespace spam {

     enum class Reason {
          None,
          RateLimited,     // 在时间窗口内，该发送者发送的消息过多
          DuplicateFlood,  // 相同文本重复发送过快
          RepeatedChars,   // 出现长串重复字符，例如 "hhhhhhhh"
          LowEntropy,      // 长消息中不同字符种类过少，例如 "abababab"
          TooLong,         // 超过了硬性长度限制
     };

     struct Config {
          uint64_t windowMs = 1000; // 滑动窗口时长
          int      maxPerWindow = 5;    // 每个窗口内单个发送者的最大消息数
          uint64_t duplicateMs = 800;  // 在此时间间隔内重复发送相同文本即视为滥发（flood）
          int      maxByteRun = 10;   // 允许的相同字节连续出现的最大次数
          int      maxMessageBytes = 256;  // 聊天内容长度的硬性上限
          double   minDistinctRatio = 0.18; // 长消息中“不同字节数/总长度”的最低比例阈值
          int      longMsgThreshold = 16;   // 仅当消息长度超过此值时进行熵（多样性）检查
          uint64_t idleEvictMs = 60000;// 空闲超过此时间后移除发送者记录
          size_t   maxSenders = 4096; // 跟踪发送者的数量上限（防止内存耗尽攻击）
     };

     // 线程安全。单个实例管理所有套接字/线程。
     class Filter {
     public:
          explicit Filter(Config cfg = {}) : cfg_(cfg) {}

          // sender 和 message 是指向接收缓冲区的原始字节视图。 
          // `now` 应传入 GetTickCount64() 的值。返回 Reason::None 表示允许通过。 
          Reason Check(std::string_view sender, std::string_view message, uint64_t now);

          const Config& config() const { return cfg_; }

     private:
          struct Record {
               std::deque<uint64_t> stamps;      // 仍在时间窗口内的消息时间戳
               uint64_t lastSeen = 0;            // 最后一次观察到的时间
               uint64_t lastMsgTime = 0;         // 最后一条消息的时间戳
               size_t   lastMsgHash = 0;         // 最后一条消息的哈希值
          };

          Reason CheckContent(std::string_view msg) const; // 无状态启发式检查
          void   EvictStale(uint64_t now);                 // 限制内存使用

          Config     cfg_;
          std::mutex mtx_;
          std::unordered_map<std::string, Record> senders_;
          uint64_t   lastEvict_ = 0;
     };

}
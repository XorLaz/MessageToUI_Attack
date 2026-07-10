#include "SpamFilter.h"

namespace spam {

     static size_t HashBytes(std::string_view s) {
          size_t h = 1469598103934665603ull;            // FNV-1a 64
          for (unsigned char c : s) { h ^= c; h *= 1099511628211ull; }
          return h;
     }

     // 最长的相同字节连续出现。捕获“hhhh”、“aaaa”、“。。。。”（相同的
     // 多字节字符也会重复相同的字节）。
     static int LongestByteRun(std::string_view s) {
          int best = 0, run = 0;
          unsigned char prev = 0;
          bool have = false;
          for (unsigned char c : s) {
               if (have && c == prev) { ++run; }
               else { run = 1; prev = c; have = true; }
               if (run > best) best = run;
          }
          return best;
     }

     // 不同傻逼字节数 / 总长度。长消息中该值较低 == “abababab” 这种傻逼垃圾内容。
     static double DistinctRatio(std::string_view s) {
          if (s.empty()) return 1.0;
          bool seen[256] = {};
          int distinct = 0;
          for (unsigned char c : s) {
               if (!seen[c]) { seen[c] = true; ++distinct; }
          }
          return static_cast<double>(distinct) / static_cast<double>(s.size());
     }

     Reason Filter::CheckContent(std::string_view msg) const {
          if (static_cast<int>(msg.size()) > cfg_.maxMessageBytes) return Reason::TooLong;
          if (LongestByteRun(msg) >= cfg_.maxByteRun)              return Reason::RepeatedChars;
          if (static_cast<int>(msg.size()) >= cfg_.longMsgThreshold &&
               DistinctRatio(msg) < cfg_.minDistinctRatio)          return Reason::LowEntropy;
          return Reason::None;
     }


     void Filter::EvictStale(uint64_t now) {
          if (now - lastEvict_ < 5000 && senders_.size() < cfg_.maxSenders) return;
          lastEvict_ = now;

          for (auto it = senders_.begin(); it != senders_.end(); ) {
               if (now - it->second.lastSeen > cfg_.idleEvictMs) it = senders_.erase(it);
               else ++it;
          }
          // 在名称欺骗攻击引发的洪泛下，map仍可能无限制增长；
          // 清理操作开销很低，且仅在发生此类恶意攻击时才会执行。
          if (senders_.size() > cfg_.maxSenders) senders_.clear();
     }


     Reason Filter::Check(std::string_view sender, std::string_view message, uint64_t now) {
          std::lock_guard<std::mutex> lk(mtx_);
          EvictStale(now);

          // 1 优先进行无状态内容检查：利用伪造名称发送的乱码洪泛攻击将在此处被拦截，
          //    且无需为发送者分配任何记录。
          if (Reason r = CheckContent(message); r != Reason::None) return r;

          Record& rec = senders_[std::string(sender)];
          rec.lastSeen = now;

          // 2 重复内容泛滥：duplicateMs 中重复出现相同的文本。
          size_t h = HashBytes(message);
          if (h == rec.lastMsgHash && (now - rec.lastMsgTime) < cfg_.duplicateMs) {
               rec.lastMsgTime = now;
               return Reason::DuplicateFlood;
          }
          rec.lastMsgHash = h;
          rec.lastMsgTime = now;

          // 3 滑动窗口速率限制：每个发送者在每个窗口内允许发送 N 条消息。
          auto& q = rec.stamps;
          while (!q.empty() && now - q.front() > cfg_.windowMs) q.pop_front();
          q.push_back(now);
          if (static_cast<int>(q.size()) > cfg_.maxPerWindow) return Reason::RateLimited;

          return Reason::None;
     }

}
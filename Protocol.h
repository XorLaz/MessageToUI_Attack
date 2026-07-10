#pragma once
#include <cstddef>
#include <string_view>
#include <optional>



// 我他妈实际收到的 


//   +--------------------+ 0x00
//     头部 (操作码等)   
//   +--------------------+ 0x0C  kSenderNameOffset
//    发送者名称 (14字节)
//   +--------------------+ 0x1A  kMessageOffset  (== 0x0C + 14)
//    消息文本 …        



namespace protocol {

	inline constexpr int kSenderNameOffset = 0x0C;
	inline constexpr int kSenderNameLen = 14;
	inline constexpr int kMessageOffset = kSenderNameOffset + kSenderNameLen; // 0x1A

	// 指向接收缓冲区的视图，已完成解析并经过边界检查。
	// 其中的 string_view 指向调用者的缓冲区，仅在该缓冲区的生命周期内有效；
	// messageData 则是指向同一缓冲区的可写指针。
	struct ChatFrame {
		std::string_view sender;          // 发送者名称字节，在第一个 NUL 处截断
		std::string_view message;   // 消息体字节
		char* messageData = nullptr;
		int              messageLen = 0;
	};

	// 针对这是否为聊天数据包的一个傻逼的保守检测。仅在received缓冲区范围内读取数据。 
	// 注意下方的正文检查是沿用自原项目的启发式逻辑
	// 即检查 name 字段中是否存在 UTF-8 编码的中日韩字符序列
	// 正确的做法是检测数据包实际的操作码（opcode）或头部信息——
	// 一旦确定了这些信息，就应将 FieldLooksLikeText()函数 替换为相应的检测逻辑。 
	// 若将非聊天数据包误判为聊天数据包并对其进行“中和”处理，会导致游戏状态损坏，
	// 因此务必保持检测逻辑的严格性。
	bool IsChatPacket(const char* buf, int received);

	// 仅当缓冲区经证实足以容纳所有待读取字段时，才返回 ChatFrame。 
	// 绝不会读取超过 received 标记的位置。
	std::optional<ChatFrame> ParseChat(char* buf, int received);

}
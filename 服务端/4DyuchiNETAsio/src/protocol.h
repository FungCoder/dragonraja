// =====================================================================
// protocol.h
// 长度前缀分帧协议
// =====================================================================
//
// 帧格式(按连接角色区分):
//
//   role=server (4DyuchiNET 服务间协议):
//     [4 bytes: DWORD, little-endian, length N][N bytes: payload]
//     length N 不含自身 4 字节; N = 0 合法(仅触发 OnRecv,不携带数据)
//
//   role=user (原版 2001 客户端协议, 见 Client network.cpp HandleReading):
//     [2 bytes: WORD, little-endian, length N][N bytes: payload]
//     payload = t_header(5B: type2+size2+crc1) + body; N = 5 + body
//     N < 5 视为协议错误(合法 t_packet 至少含 5 字节头)
//
// 最大 N:
//   role=user  : 8192  (与 DESC_NETWORK.dwUserMaxTransferSize 对应)
//   role=server: 65000 (与 DESC_NETWORK.dwServerMaxTransferSize 对应)
// =====================================================================

#pragma once

#ifndef _PROTOCOL_H_INCLUDED
#define _PROTOCOL_H_INCLUDED

#include <cstdint>
#include <cstring>

namespace asionet {

constexpr std::size_t kHeaderSize      = sizeof(std::uint32_t);  // server 帧头 4B
constexpr std::size_t kUserHeaderSize  = sizeof(std::uint16_t);  // user 帧头 2B
constexpr std::size_t kMaxHeaderSize   = kHeaderSize;            // 两者取大,读缓冲按此分配
constexpr std::size_t kMinUserPayload  = 5;                      // t_header 尺寸

// 根据 role 返回帧头字节数
inline std::size_t header_size_for(bool is_server_role) {
    return is_server_role ? kHeaderSize : kUserHeaderSize;
}

// 根据 role 返回最大负载长度
inline std::size_t max_payload_for(bool is_server_role) {
    return is_server_role ? 65000u : 8192u;
}

// 编码长度头到 4 字节缓冲区 (小端,x86/x64 原生字节序)
inline void encode_header(char* buf, std::uint32_t length) {
    std::memcpy(buf, &length, sizeof(length));
}

// 解码长度头(已确认缓冲区有 4 字节)
inline std::uint32_t decode_header(const char* buf) {
    std::uint32_t length = 0;
    std::memcpy(&length, buf, sizeof(length));
    return length;
}

// 编码/解码 user 帧的 2 字节长度头(小端 WORD)
inline void encode_user_header(char* buf, std::uint16_t length) {
    std::memcpy(buf, &length, sizeof(length));
}

inline std::uint16_t decode_user_header(const char* buf) {
    std::uint16_t length = 0;
    std::memcpy(&length, buf, sizeof(length));
    return length;
}

} // namespace asionet

#endif // _PROTOCOL_H_INCLUDED

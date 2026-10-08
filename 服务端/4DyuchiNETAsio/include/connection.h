// =====================================================================
// connection.h
// CConnection - 单 TCP 连接(由 C4DyuchiNETAsio 持有)
// =====================================================================
//
// 角色:
//   - user   : 由 StartServerWithUserSide 接受的连接(玩家客户端)
//   - server : 由 StartServerWithServerSide 接受的连接 / 主动 ConnectTo 建立的连接
//
// 线程模型:
//   - m_strand 串行化同一连接的所有 async handler
//   - 用户回调由 *外部* 的 callback_strand 派发(在 do_start_read 内部 post)
//
// 生命周期:
//   - 通过 std::shared_ptr 持有
//   - m_socket.open() 后即"已存在",关闭后等待所有 handler 完成再析构
//   - sockaddr_in 是稳定成员,GetUserAddress()/GetServerAddress() 直接返回 &m_address
//     (调用方 usertable.cpp:106 缓存该指针,因此 m_address 不能 move)
// =====================================================================

#pragma once

#ifndef _CONNECTION_H_INCLUDED
#define _CONNECTION_H_INCLUDED

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include "../src/protocol.h"
#include "inetwork.h"

namespace asionet {

// 前向声明
class CConnectionRegistry;
class C4DyuchiNETAsio;

enum class ConnectionRole : std::uint8_t
{
    User   = 0,
    Server = 1,
};

// 一帧待发送的数据
struct OutboundPacket
{
    std::vector<char> data;       // 帧头+负载(完整可发的字节流)
};

class CConnection : public std::enable_shared_from_this<CConnection>
{
public:
    CConnection(boost::asio::ip::tcp::socket socket,
                ConnectionRole role,
                DWORD connection_index,
                CConnectionRegistry& registry,
                C4DyuchiNETAsio& owner);
    ~CConnection();

    CConnection(const CConnection&) = delete;
    CConnection& operator=(const CConnection&) = delete;

    // 启动读循环。必须在 socket 已 accepted/connected 之后调用。
    void Start();

    // 在 strand 内关闭 socket (force-close 路径)。
    void ForceClose();

    // 设置/读取 user info 指针(Registry 锁外访问,本类内仅存指针)
    void SetUserInfo(void* p) { m_user_info.store(p, std::memory_order_release); }
    void* GetUserInfo() const { return m_user_info.load(std::memory_order_acquire); }

    // 由 send_to_xxx 路径调用:在 strand 内 push 一帧并触发 write 循环
    void PostSend(const char* msg, std::size_t length);

    ConnectionRole role() const { return m_role; }
    DWORD connection_index() const { return m_connection_index; }
    void set_connection_index(DWORD idx) { m_connection_index = idx; }
    bool is_server_role() const { return m_role == ConnectionRole::Server; }

    // 稳定指针 —— usertable.cpp:106 缓存
    sockaddr_in* address_ptr() { return &m_address; }
    const sockaddr_in* address_ptr() const { return &m_address; }

    // 在 strand 内填充 m_address;Accept 路径在 accept handler 中调用
    void set_address(const sockaddr_in& addr) { m_address = addr; }

private:
    // 读循环 (在 strand 内)
    void do_read_header();
    void do_read_payload(std::uint32_t payload_length);
    void handle_read_error(const boost::system::error_code& ec);

    // 写循环 (在 strand 内)
    void do_write_one();
    void handle_write(const boost::system::error_code& ec, std::size_t bytes_written);

    // 关闭后通知 owner 触发 OnDisconnectXxx (在 callback_strand 内)
    void notify_disconnect_and_remove();

    boost::asio::ip::tcp::socket      m_socket;
    boost::asio::strand<boost::asio::io_context::executor_type> m_strand;
    ConnectionRole                    m_role;
    DWORD                             m_connection_index;
    CConnectionRegistry&             m_registry;
    C4DyuchiNETAsio&                  m_owner;

    // 稳定地址成员 —— 永不在生命周期内 move/reallocate
    sockaddr_in                       m_address;

    // 读状态
    char                              m_header_buf[kMaxHeaderSize];
    std::vector<char>                 m_payload_buf;

    // 写状态
    std::deque<OutboundPacket>        m_outbound;
    bool                              m_writing_in_flight;

    // 是否已发起过 OnDisconnect 通知(防止重复)
    bool                              m_disconnect_notified;

    // user/server info (本类不关心类型)
    std::atomic<void*>                m_user_info;
};

} // namespace asionet

#endif // _CONNECTION_H_INCLUDED

// =====================================================================
// connection.cpp
// CConnection - 单 TCP 连接实现
// =====================================================================

#include "../include/connection.h"
#include "../include/connection_registry.h"
#include "4DyuchiNETAsioImpl.h"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace asionet {

using boost::asio::ip::tcp;

CConnection::CConnection(tcp::socket socket,
                         ConnectionRole role,
                         DWORD connection_index,
                         CConnectionRegistry& registry,
                         C4DyuchiNETAsio& owner)
    : m_socket(std::move(socket))
    , m_strand(boost::asio::make_strand(owner.io().get_executor()))
    , m_role(role)
    , m_connection_index(connection_index)
    , m_registry(registry)
    , m_owner(owner)
    , m_writing_in_flight(false)
    , m_disconnect_notified(false)
    , m_user_info(nullptr)
{
    std::memset(&m_address, 0, sizeof(m_address));
}

CConnection::~CConnection()
{
    // socket 应在 ForceClose / handle_*_error 中已 close
    boost::system::error_code ec;
    if (m_socket.is_open()) {
        m_socket.close(ec);
    }
}

void CConnection::Start()
{
    // 在 strand 内启动 read 循环
    boost::asio::post(m_strand, [self = shared_from_this()]() {
        self->do_read_header();
    });
}

void CConnection::ForceClose()
{
    // 调用方(post 到 io_context 后)进入 strand
    boost::asio::post(m_strand, [self = shared_from_this()]() {
        boost::system::error_code ec;
        if (self->m_socket.is_open()) {
            self->m_socket.close(ec);
        }
        // 关闭后通知(若尚未通知)
        if (!self->m_disconnect_notified) {
            self->notify_disconnect_and_remove();
        }
    });
}

void CConnection::PostSend(const char* msg, std::size_t length)
{
    // 按角色编码帧头:
    //   user   : [2B WORD length][payload] —— 原版客户端协议
    //   server : [4B DWORD length][payload] —— 4DyuchiNET 服务间协议
    OutboundPacket pkt;
    const std::size_t prefix = header_size_for(is_server_role());
    pkt.data.resize(prefix + length);
    if (is_server_role()) {
        encode_header(pkt.data.data(), static_cast<std::uint32_t>(length));
    } else {
        encode_user_header(pkt.data.data(), static_cast<std::uint16_t>(length));
    }
    if (length > 0) {
        std::memcpy(pkt.data.data() + prefix, msg, length);
    }

    auto self = shared_from_this();
    boost::asio::post(m_strand, [self, p = std::move(pkt)]() mutable {
        self->m_outbound.push_back(std::move(p));
        if (!self->m_writing_in_flight) {
            self->do_write_one();
        }
    });
}

void CConnection::do_read_header()
{
    auto self = shared_from_this();
    const std::size_t prefix = header_size_for(is_server_role());
    // 用 async_read 替代 async_read_some,内部循环直到读够帧头
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_header_buf, prefix),
        boost::asio::bind_executor(m_strand,
            [this, self, prefix](const boost::system::error_code& ec, std::size_t /*bytes_read*/) {
                if (ec) {
                    handle_read_error(ec);
                    return;
                }
                std::uint32_t length = 0;
                if (prefix == kUserHeaderSize) {
                    length = decode_user_header(m_header_buf);
                    // 合法 t_packet 至少 5 字节(t_header); 不足视为协议错误
                    if (length < kMinUserPayload) {
                        boost::system::error_code ig;
                        m_socket.close(ig);
                        handle_read_error(boost::asio::error::operation_aborted);
                        return;
                    }
                } else {
                    length = decode_header(m_header_buf);
                }
                const std::size_t max_len = max_payload_for(is_server_role());
                if (length > max_len) {
                    // 超限 -> 关闭 + 通知
                    boost::system::error_code ig;
                    m_socket.close(ig);
                    handle_read_error(boost::asio::error::operation_aborted);
                    return;
                }
                m_payload_buf.assign(length, 0);
                if (length == 0) {
                    // 0 长 payload:直接触发 OnRecv (按角色)
                    if (is_server_role()) {
                        m_owner.InvokeOnRecvFromServer(m_connection_index, m_payload_buf.data(), 0);
                    } else {
                        m_owner.InvokeOnRecvFromUser(m_connection_index, m_payload_buf.data(), 0);
                    }
                    do_read_header();
                    return;
                }
                do_read_payload(length);
            }));
}

void CConnection::do_read_payload(std::uint32_t payload_length)
{
    auto self = shared_from_this();
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_payload_buf.data(), payload_length),
        boost::asio::bind_executor(m_strand,
            [this, self, payload_length](const boost::system::error_code& ec, std::size_t) {
                if (ec) {
                    handle_read_error(ec);
                    return;
                }
                if (is_server_role()) {
                    m_owner.InvokeOnRecvFromServer(m_connection_index, m_payload_buf.data(), payload_length);
                } else {
                    m_owner.InvokeOnRecvFromUser(m_connection_index, m_payload_buf.data(), payload_length);
                }
                do_read_header();
            }));
}

void CConnection::handle_read_error(const boost::system::error_code& ec)
{
    // 静默关闭 socket + 通知一次
    boost::system::error_code ig;
    if (m_socket.is_open()) {
        m_socket.close(ig);
    }
    if (!m_disconnect_notified) {
        notify_disconnect_and_remove();
    }
}

void CConnection::do_write_one()
{
    if (m_outbound.empty()) {
        m_writing_in_flight = false;
        return;
    }
    m_writing_in_flight = true;
    auto& front = m_outbound.front();
    auto self = shared_from_this();

    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(front.data.data(), front.data.size()),
        boost::asio::bind_executor(m_strand,
            [this, self](const boost::system::error_code& ec, std::size_t bytes_written) {
                handle_write(ec, bytes_written);
            }));
}

void CConnection::handle_write(const boost::system::error_code& ec, std::size_t /*bytes_written*/)
{
    if (ec) {
        // 写失败 -> 关闭 + 通知
        boost::system::error_code ig;
        if (m_socket.is_open()) {
            m_socket.close(ig);
        }
        if (!m_disconnect_notified) {
            notify_disconnect_and_remove();
        }
        // 不重置 m_writing_in_flight;后续 m_outbound 会被析构
        return;
    }
    if (!m_outbound.empty()) {
        m_outbound.pop_front();
    }
    if (!m_outbound.empty()) {
        do_write_one();
    } else {
        m_writing_in_flight = false;
    }
}

void CConnection::notify_disconnect_and_remove()
{
    m_disconnect_notified = true;

    // 移到 callback_strand 内派发 OnDisconnectXxx (直接调用 m_desc->OnDisconnectXxx,
    // 避免再通过 Invoke* 方法二次 post)
    const DWORD idx = m_connection_index;
    const bool is_user = !is_server_role();
    C4DyuchiNETAsio& owner = m_owner;

    boost::asio::post(owner.callback_strand(),
        [&owner, idx, is_user, connection = shared_from_this()]() {
            // owner.m_desc 由 callback_strand 串行访问,这里只在 strand 内读
            if (is_user) {
                if (owner.HasDesc() && owner.Desc().OnDisconnectUser) {
                    owner.Desc().OnDisconnectUser(idx);
                }
            } else {
                if (owner.HasDesc() && owner.Desc().OnDisconnectServer) {
                    owner.Desc().OnDisconnectServer(idx);
                }
            }
            // 回调仍需通过连接索引读取 UserInfo / ServerInfo，先通知业务层，
            // 再移除注册表。保留 shared_ptr 直到回调结束，防止提前析构。
            connection->m_registry.Unregister(idx);
        });

    // 注册表删除在上述串行回调末尾执行。
}

} // namespace asionet

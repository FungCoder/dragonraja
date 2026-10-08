// =====================================================================
// 4DyuchiNETAsioImpl.cpp
// C4DyuchiNETAsio 主类实现
// =====================================================================

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "4DyuchiNETAsioImpl.h"
#include "network_guid.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace asionet {

using boost::asio::ip::tcp;
using boost::asio::ip::address_v4;
using boost::asio::ip::make_address;

// 将 boost::asio::ip::tcp::endpoint 转换为 sockaddr_in (仅 IPv4)
// Asio 1.83 不再提供 to_sockaddr_in 扩展,需要手写
static sockaddr_in endpoint_to_sockaddr_in(const tcp::endpoint& ep)
{
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<u_short>(ep.port()));
    if (ep.address().is_v4()) {
        const auto bytes = ep.address().to_v4().to_bytes();
        std::memcpy(&addr.sin_addr, bytes.data(), 4);
    } else {
        // IPv6 端点无法表示为 sockaddr_in,填 0
        addr.sin_addr.s_addr = 0;
    }
    return addr;
}

// =====================================================================
// 构造 / 析构
// =====================================================================
C4DyuchiNETAsio::C4DyuchiNETAsio()
    : m_ref_count(1)
    , m_ioc()
    , m_work_guard(boost::asio::make_work_guard(m_ioc))
    , m_callback_strand(boost::asio::make_strand(m_ioc.get_executor()))
    , m_user_binded_port(0)
    , m_server_binded_port(0)
{
    StartWorkers();
}

C4DyuchiNETAsio::~C4DyuchiNETAsio()
{
    StopWorkers();
    if (m_timer_manager) {
        m_timer_manager->StopAll();
    }
    // 关闭 acceptor
    if (m_user_acceptor) {
        boost::system::error_code ec;
        m_user_acceptor->close(ec);
    }
    if (m_server_acceptor) {
        boost::system::error_code ec;
        m_server_acceptor->close(ec);
    }
}

// =====================================================================
// IUnknown
// =====================================================================
HRESULT __stdcall C4DyuchiNETAsio::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == nullptr) return E_POINTER;
    if (riid == IID_IUnknown || riid == IID_4DyuchiNET) {
        *ppv = static_cast<I4DyuchiNET*>(this);
        AddRef();
        return S_OK;
    }
    *ppv = nullptr;
    return E_NOINTERFACE;
}

ULONG __stdcall C4DyuchiNETAsio::AddRef()
{
    return static_cast<ULONG>(InterlockedIncrement(&m_ref_count));
}

ULONG __stdcall C4DyuchiNETAsio::Release()
{
    LONG ref = InterlockedDecrement(&m_ref_count);
    if (ref == 0) {
        delete this;
        return 0;
    }
    return static_cast<ULONG>(ref);
}

// =====================================================================
// I4DyuchiNET 完整实现 (16 个真实方法)
// =====================================================================
BOOL __stdcall C4DyuchiNETAsio::CreateNetwork(DESC_NETWORK* desc, DWORD, DWORD)
{
    if (desc == nullptr) return FALSE;
    m_desc = std::make_unique<DESC_NETWORK>(*desc);
    m_timer_manager = std::make_unique<CTimerManager>(m_ioc, desc->pEvent, desc->dwCustomDefineEventNum);
    m_timer_manager->StartAll();
    return TRUE;
}

void __stdcall C4DyuchiNETAsio::BreakMainThread()   { /* no-op */ }
void __stdcall C4DyuchiNETAsio::ResumeMainThread()  { /* no-op */ }

void __stdcall C4DyuchiNETAsio::SetUserInfo(DWORD idx, void* user)
{
    auto conn = m_registry.Get(idx);
    if (conn) conn->SetUserInfo(user);
}

void* __stdcall C4DyuchiNETAsio::GetUserInfo(DWORD idx)
{
    auto conn = m_registry.Get(idx);
    if (!conn) return nullptr;
    return conn->GetUserInfo();
}

void __stdcall C4DyuchiNETAsio::SetServerInfo(DWORD, void*) { /* no-op (AgentServer 不用) */ }
void* __stdcall C4DyuchiNETAsio::GetServerInfo(DWORD)      { return nullptr; }

sockaddr_in* __stdcall C4DyuchiNETAsio::GetServerAddress(DWORD idx)
{
    auto conn = m_registry.Get(idx);
    if (!conn) return nullptr;
    return conn->address_ptr();
}

sockaddr_in* __stdcall C4DyuchiNETAsio::GetUserAddress(DWORD idx)
{
    auto conn = m_registry.Get(idx);
    if (!conn) return nullptr;
    return conn->address_ptr();
}

BOOL __stdcall C4DyuchiNETAsio::GetServerAddress(DWORD, char*, WORD*) { return FALSE; }
BOOL __stdcall C4DyuchiNETAsio::GetUserAddress(DWORD, char*, WORD*)   { return FALSE; }

BOOL __stdcall C4DyuchiNETAsio::SendToServer(DWORD idx, char* msg, DWORD length, DWORD /*flag*/)
{
    return SendToConnection(idx, msg, length, /*is_user_path=*/false);
}

BOOL __stdcall C4DyuchiNETAsio::SendToUser(DWORD idx, char* msg, DWORD length, DWORD /*flag*/)
{
    return SendToConnection(idx, msg, length, /*is_user_path=*/true);
}

void __stdcall C4DyuchiNETAsio::CompulsiveDisconnectServer(DWORD idx)
{
    ForceCloseConnection(idx);
}

void __stdcall C4DyuchiNETAsio::CompulsiveDisconnectUser(DWORD idx)
{
    ForceCloseConnection(idx);
}

int __stdcall C4DyuchiNETAsio::GetServerMaxTransferRecvSize()  { return 65000; }
int __stdcall C4DyuchiNETAsio::GetServerMaxTransferSendSize()  { return 65000; }
int __stdcall C4DyuchiNETAsio::GetUserMaxTransferRecvSize()    { return 8192;  }
int __stdcall C4DyuchiNETAsio::GetUserMaxTransferSendSize()    { return 8192;  }

void __stdcall C4DyuchiNETAsio::BroadcastServer(char*, DWORD, DWORD) { /* no-op */ }
void __stdcall C4DyuchiNETAsio::BroadcastUser(char*, DWORD, DWORD)   { /* no-op */ }

DWORD __stdcall C4DyuchiNETAsio::GetConnectedServerNum() { return m_registry.GetServerCount(); }
DWORD __stdcall C4DyuchiNETAsio::GetConnectedUserNum()   { return m_registry.GetUserCount();   }

WORD __stdcall C4DyuchiNETAsio::GetBindedPortServerSide()
{
    return m_server_binded_port.load(std::memory_order_acquire);
}

WORD __stdcall C4DyuchiNETAsio::GetBindedPortUserSide()
{
    return m_user_binded_port.load(std::memory_order_acquire);
}

BOOL __stdcall C4DyuchiNETAsio::ConnectToServerWithUserSide(char*, WORD, CONNECTSUCCESSFUNC, CONNECTFAILFUNC, void*)
{
    // AgentServer 不用。返回 FALSE 以免误用
    return FALSE;
}

BOOL __stdcall C4DyuchiNETAsio::ConnectToServerWithServerSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt)
{
    if (szIP == nullptr) return FALSE;
    DoConnect(szIP, port, ConnectionRole::Server, pSucc, pFail, pExt);
    return TRUE;
}

BOOL __stdcall C4DyuchiNETAsio::StartServerWithUserSide(char* ip, WORD port)
{
    return StartAcceptor(ip, port, ConnectionRole::User, m_user_binded_port) ? TRUE : FALSE;
}

BOOL __stdcall C4DyuchiNETAsio::StartServerWithServerSide(char* ip, WORD port)
{
    return StartAcceptor(ip, port, ConnectionRole::Server, m_server_binded_port) ? TRUE : FALSE;
}

HANDLE __stdcall C4DyuchiNETAsio::GetCustomEventHandle(DWORD index)
{
    if (!m_timer_manager) return nullptr;
    return m_timer_manager->GetHandle(index);
}

BOOL __stdcall C4DyuchiNETAsio::PauseTimer(DWORD idx)
{
    if (!m_timer_manager) return FALSE;
    if (idx >= 3) return FALSE;
    m_timer_manager->Pause(idx);
    return TRUE;
}

BOOL __stdcall C4DyuchiNETAsio::ResumeTimer(DWORD idx)
{
    if (!m_timer_manager) return FALSE;
    if (idx >= 3) return FALSE;
    m_timer_manager->Resume(idx);
    return TRUE;
}

// 未使用的方法:WSABUF / PACKET_LIST 重载
BOOL __stdcall C4DyuchiNETAsio::SendToServer(DWORD, WSABUF*, DWORD, DWORD) { return TRUE; }
BOOL __stdcall C4DyuchiNETAsio::SendToUser(DWORD, WSABUF*, DWORD, DWORD)   { return TRUE; }
BOOL __stdcall C4DyuchiNETAsio::SendToServer(DWORD, PACKET_LIST*, DWORD)   { return TRUE; }
BOOL __stdcall C4DyuchiNETAsio::SendToUser(DWORD, PACKET_LIST*, DWORD)     { return TRUE; }

// =====================================================================
// 内部方法
// =====================================================================
void C4DyuchiNETAsio::StartWorkers()
{
    const DWORD n = std::max<DWORD>(2, std::thread::hardware_concurrency());
    m_workers.reserve(n);
    for (DWORD i = 0; i < n; ++i) {
        m_workers.emplace_back([this]() {
            try {
                m_ioc.run();
            } catch (...) {
                // 静默吞掉 worker 异常
            }
        });
    }
}

void C4DyuchiNETAsio::StopWorkers()
{
    m_work_guard.reset();
    m_ioc.stop();
    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();
}

bool C4DyuchiNETAsio::StartAcceptor(const char* ip, WORD port, ConnectionRole role, std::atomic<WORD>& out_port)
{
    auto& slot = (role == ConnectionRole::User) ? m_user_acceptor : m_server_acceptor;
    slot = std::make_unique<tcp::acceptor>(m_ioc);

    boost::system::error_code ec;
    boost::asio::ip::address addr;
    if (ip == nullptr || std::strcmp(ip, "") == 0 || std::strcmp(ip, "0.0.0.0") == 0) {
        addr = boost::asio::ip::address_v4::any();
    } else {
        try {
            addr = make_address(ip);
        } catch (...) {
            addr = boost::asio::ip::address_v4::any();
        }
    }
    tcp::endpoint endpoint(addr, port);
    slot->open(endpoint.protocol(), ec);
    if (ec) { OutputDebugStringA("[4DyuchiNET] StartAcceptor: open failed: "); OutputDebugStringA(ec.message().c_str()); return false; }
    slot->set_option(tcp::acceptor::reuse_address(true), ec);
    slot->bind(endpoint, ec);
    if (ec) { OutputDebugStringA("[4DyuchiNET] StartAcceptor: bind failed (reserved/in-use port?): "); OutputDebugStringA(ec.message().c_str()); return false; }
    slot->listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) { OutputDebugStringA("[4DyuchiNET] StartAcceptor: listen failed: "); OutputDebugStringA(ec.message().c_str()); return false; }

    // 记录实际绑定端口(bind 时可被系统调整 —— 这里直接用 port)
    out_port.store(slot->local_endpoint(ec).port(), std::memory_order_release);

    DoAccept(*slot, role);
    return true;
}

void C4DyuchiNETAsio::DoAccept(tcp::acceptor& acceptor, ConnectionRole role)
{
    auto socket = std::make_shared<tcp::socket>(m_ioc);
    acceptor.async_accept(*socket, [this, &acceptor, socket, role](const boost::system::error_code& ec) {
        if (ec) {
            // acceptor 已关闭
            return;
        }
        // 接受成功
        boost::system::error_code ec2;
        auto remote = socket->remote_endpoint(ec2);
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        if (!ec2) {
            addr = endpoint_to_sockaddr_in(remote);
        }

        // 创建 connection (shared_ptr)
        auto conn = std::make_shared<CConnection>(std::move(*socket), role, /*index*/0, m_registry, *this);
        conn->set_address(addr);
        const DWORD idx = m_registry.Register(conn);
        conn->set_connection_index(idx);

        // 触发 OnAccept 回调 (在 callback_strand 内)
        if (role == ConnectionRole::User) {
            InvokeOnAcceptUser(idx);
        } else {
            InvokeOnAcceptServer(idx);
        }

        // 启动 read 循环
        conn->Start();

        // 继续 accept
        DoAccept(acceptor, role);
    });
}

void C4DyuchiNETAsio::DoConnect(const char* ip, WORD port, ConnectionRole role,
                                CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt)
{
    auto socket = std::make_shared<tcp::socket>(m_ioc);
    tcp::endpoint endpoint;
    try {
        endpoint = tcp::endpoint(make_address(ip), port);
    } catch (...) {
        if (pFail) InvokeConnectFail(pExt, pFail);
        return;
    }
    socket->async_connect(endpoint, [this, socket, role, pSucc, pFail, pExt](const boost::system::error_code& ec) {
        if (ec) {
            if (pFail) InvokeConnectFail(pExt, pFail);
            return;
        }
        // 连接成功
        boost::system::error_code ec2;
        auto local = socket->local_endpoint(ec2);
        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        if (!ec2) addr = endpoint_to_sockaddr_in(local);

        auto conn = std::make_shared<CConnection>(std::move(*socket), role, 0, m_registry, *this);
        conn->set_address(addr);
        const DWORD idx = m_registry.Register(conn);
        conn->set_connection_index(idx);

        if (pSucc) InvokeConnectSuccess(idx, pExt, pSucc);
        conn->Start();
    });
}

BOOL C4DyuchiNETAsio::SendToConnection(DWORD idx, const char* msg, DWORD length, bool is_user_path)
{
    if (msg == nullptr && length > 0) return FALSE;
    auto conn = m_registry.Get(idx);
    if (!conn) return FALSE;
    if (is_user_path && conn->role() != ConnectionRole::User)   return FALSE;
    if (!is_user_path && conn->role() != ConnectionRole::Server) return FALSE;

    conn->PostSend(msg, static_cast<std::size_t>(length));
    return TRUE;
}

void C4DyuchiNETAsio::ForceCloseConnection(DWORD idx)
{
    auto conn = m_registry.Get(idx);
    if (!conn) return;
    conn->ForceClose();
}

// =====================================================================
// 用户回调派发 (在 callback_strand 内调用)
// =====================================================================
void C4DyuchiNETAsio::InvokeOnAcceptUser(DWORD idx)
{
    boost::asio::post(m_callback_strand, [this, idx]() {
        if (m_desc && m_desc->OnAcceptUser) {
            m_desc->OnAcceptUser(idx);
        }
    });
}

void C4DyuchiNETAsio::InvokeOnAcceptServer(DWORD idx)
{
    boost::asio::post(m_callback_strand, [this, idx]() {
        if (m_desc && m_desc->OnAcceptServer) {
            m_desc->OnAcceptServer(idx);
        }
    });
}

void C4DyuchiNETAsio::InvokeOnDisconnectUser(DWORD idx)
{
    boost::asio::post(m_callback_strand, [this, idx]() {
        if (m_desc && m_desc->OnDisconnectUser) {
            m_desc->OnDisconnectUser(idx);
        }
    });
}

void C4DyuchiNETAsio::InvokeOnDisconnectServer(DWORD idx)
{
    boost::asio::post(m_callback_strand, [this, idx]() {
        if (m_desc && m_desc->OnDisconnectServer) {
            m_desc->OnDisconnectServer(idx);
        }
    });
}

void C4DyuchiNETAsio::InvokeOnRecvFromUser(DWORD idx, const char* msg, DWORD length)
{
    // 拷贝数据到 post lambda: msg 指针可能来自 CConnection::m_payload_buf
    // 该 buffer 在 connection.strand 后续 do_read_header() 会被 assign(),
    // 即便两个 strand 没有显式顺序,拷贝确保 callback_strand 看到稳定内存
    std::vector<char> data;
    if (msg != nullptr && length > 0) {
        data.assign(msg, msg + length);
    }
    boost::asio::post(m_callback_strand, [this, idx, data = std::move(data), length]() mutable {
        if (m_desc && m_desc->OnRecvFromUserTCP) {
            m_desc->OnRecvFromUserTCP(idx, data.data(), length);
        }
    });
}

void C4DyuchiNETAsio::InvokeOnRecvFromServer(DWORD idx, const char* msg, DWORD length)
{
    std::vector<char> data;
    if (msg != nullptr && length > 0) {
        data.assign(msg, msg + length);
    }
    boost::asio::post(m_callback_strand, [this, idx, data = std::move(data), length]() mutable {
        if (m_desc && m_desc->OnRecvFromServerTCP) {
            m_desc->OnRecvFromServerTCP(idx, data.data(), length);
        }
    });
}

void C4DyuchiNETAsio::InvokeConnectSuccess(DWORD idx, void* ext, CONNECTSUCCESSFUNC pSucc)
{
    boost::asio::post(m_callback_strand, [pSucc, idx, ext]() {
        if (pSucc) pSucc(idx, ext);
    });
}

void C4DyuchiNETAsio::InvokeConnectFail(void* ext, CONNECTFAILFUNC pFail)
{
    boost::asio::post(m_callback_strand, [pFail, ext]() {
        if (pFail) pFail(ext);
    });
}

} // namespace asionet

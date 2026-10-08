// =====================================================================
// 4DyuchiNETAsioImpl.h
// C4DyuchiNETAsio - I4DyuchiNET 接口的真实 Boost.Asio 实现
// =====================================================================
//
// 唯一职责:
//   1. 实现 I4DyuchiNET 31 个方法(vtable 顺序与 stub 严格一致)
//   2. 持有 io_context + N 个 worker thread
//   3. 持有 callback_strand,统一派发所有 OnAccept/OnRecv/OnDisconnect
//   4. 持有 Registry 和 TimerManager
// =====================================================================

#pragma once

#ifndef _4DYUCHINETASIOIMPL_H_INCLUDED
#define _4DYUCHINETASIOIMPL_H_INCLUDED

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>

#include "inetwork.h"
#include "connection.h"
#include "connection_registry.h"
#include "timer_manager.h"

namespace asionet {

using tcp = boost::asio::ip::tcp;

class C4DyuchiNETAsio : public I4DyuchiNET
{
public:
    C4DyuchiNETAsio();
    ~C4DyuchiNETAsio();

    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    // I4DyuchiNET —— 31 个方法 (vtable 顺序与 stub 一致)
    BOOL            __stdcall CreateNetwork(DESC_NETWORK* desc, DWORD dwUserAcceptInterval, DWORD dwServerAcceptInterval) override;
    void            __stdcall BreakMainThread() override;
    void            __stdcall ResumeMainThread() override;
    void            __stdcall SetUserInfo(DWORD dwConnectionIndex, void* user) override;
    void*           __stdcall GetUserInfo(DWORD dwConnectionIndex) override;
    void            __stdcall SetServerInfo(DWORD dwConnectionIndex, void* server) override;
    void*           __stdcall GetServerInfo(DWORD dwConnectionIndex) override;
    sockaddr_in*    __stdcall GetServerAddress(DWORD dwConnectionIndex) override;
    sockaddr_in*    __stdcall GetUserAddress(DWORD dwConnectionIndex) override;
    BOOL            __stdcall GetServerAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) override;
    BOOL            __stdcall GetUserAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) override;
    BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) override;
    BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) override;
    void            __stdcall CompulsiveDisconnectServer(DWORD dwConnectionIndex) override;
    void            __stdcall CompulsiveDisconnectUser(DWORD dwConnectionIndex) override;
    int             __stdcall GetServerMaxTransferRecvSize() override;
    int             __stdcall GetServerMaxTransferSendSize() override;
    int             __stdcall GetUserMaxTransferRecvSize() override;
    int             __stdcall GetUserMaxTransferSendSize() override;
    void            __stdcall BroadcastServer(char* pMsg, DWORD len, DWORD flag) override;
    void            __stdcall BroadcastUser(char* pMsg, DWORD len, DWORD flag) override;
    DWORD           __stdcall GetConnectedServerNum() override;
    DWORD           __stdcall GetConnectedUserNum() override;
    WORD            __stdcall GetBindedPortServerSide() override;
    WORD            __stdcall GetBindedPortUserSide() override;
    BOOL            __stdcall ConnectToServerWithUserSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt) override;
    BOOL            __stdcall ConnectToServerWithServerSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt) override;
    BOOL            __stdcall StartServerWithUserSide(char* ip, WORD port) override;
    BOOL            __stdcall StartServerWithServerSide(char* ip, WORD port) override;
    HANDLE          __stdcall GetCustomEventHandle(DWORD index) override;
    BOOL            __stdcall PauseTimer(DWORD dwCustomEventIndex) override;
    BOOL            __stdcall ResumeTimer(DWORD dwCustomEventIndex) override;
    BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, WSABUF* pBuf, DWORD dwNum, DWORD flag) override;
    BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, WSABUF* pBuf, DWORD dwNum, DWORD flag) override;
    BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) override;
    BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) override;

    // 由 CConnection / CTimerManager 调用的内部 hook
    boost::asio::io_context& io() { return m_ioc; }
    boost::asio::strand<boost::asio::io_context::executor_type>& callback_strand() { return m_callback_strand; }
    bool        HasDesc() const { return m_desc != nullptr; }
    DESC_NETWORK& Desc() { return *m_desc; }

    // 触发用户回调 (内部用,在 callback_strand 内调用)
    void InvokeOnAcceptUser(DWORD idx);
    void InvokeOnAcceptServer(DWORD idx);
    void InvokeOnDisconnectUser(DWORD idx);
    void InvokeOnDisconnectServer(DWORD idx);
    void InvokeOnRecvFromUser(DWORD idx, const char* msg, DWORD length);
    void InvokeOnRecvFromServer(DWORD idx, const char* msg, DWORD length);

    // connect 回调
    void InvokeConnectSuccess(DWORD idx, void* ext, CONNECTSUCCESSFUNC pSucc);
    void InvokeConnectFail(void* ext, CONNECTFAILFUNC pFail);

private:
    // accept / connect 实现
    bool StartAcceptor(const char* ip, WORD port, ConnectionRole role, std::atomic<WORD>& out_port);
    void DoAccept(tcp::acceptor& acceptor, ConnectionRole role);
    void DoConnect(const char* ip, WORD port, ConnectionRole role,
                   CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt);

    // 内部:查找 connection 并 post send
    BOOL SendToConnection(DWORD idx, const char* msg, DWORD length, bool is_user_path);

    // 内部:触发 force-close
    void ForceCloseConnection(DWORD idx);

    // 启动 / 停止 io_context worker
    void StartWorkers();
    void StopWorkers();

    LONG    m_ref_count;
    boost::asio::io_context  m_ioc;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_work_guard;
    boost::asio::strand<boost::asio::io_context::executor_type>  m_callback_strand;
    std::vector<std::thread> m_workers;

    // acceptor 容器
    std::unique_ptr<tcp::acceptor>   m_user_acceptor;
    std::unique_ptr<tcp::acceptor>   m_server_acceptor;
    std::atomic<WORD>                m_user_binded_port;   // 0 = 未绑定
    std::atomic<WORD>                m_server_binded_port;

    // 描述符副本(CreateNetwork 时锁定)
    std::unique_ptr<DESC_NETWORK>    m_desc;
    CConnectionRegistry              m_registry;
    std::unique_ptr<CTimerManager>   m_timer_manager;
};

} // namespace asionet

#endif // _4DYUCHINETASIOIMPL_H_INCLUDED

// =====================================================================
// inetwork.h
// 4DyuchiNET COM 接口定义 (从 stub 复制,字节级一致)
// =====================================================================
//
// 这是 2001 年韩国商业第三方游戏网络库 4DyuchiNET 的接口定义。
// 我们仅复制接口契约,不复制实现,以便为 AgentServer/MapServer/ProxyServer
// 提供一个真实的 Boost.Asio 后端。
//
// vtable 顺序由本头文件的声明顺序决定 —— 必须与原 stub / AgentServer
// 的 inetwork.h 字节级一致,否则 COM 调用将走错方法。
// =====================================================================

#pragma once

#ifndef _INETWORK_H_INCLUDED
#define _INETWORK_H_INCLUDED

// -------------------------------------------------------------------------
// 平台前置宏
// -------------------------------------------------------------------------
// Asio 必须在 windows.h 之前包含,且需要 WIN32_LEAN_AND_MEAN 避免
// 与 Windows 套接字宏冲突。这里只声明前置条件,由使用方在包含本头文件
// 之前正确设置。
// -------------------------------------------------------------------------

#include <objbase.h>     // IUnknown, IClassFactory
#include <winsock2.h>    // sockaddr_in, WSABUF

// -------------------------------------------------------------------------
// 回调函数指针类型
// -------------------------------------------------------------------------
typedef void (__stdcall *ACCEPTFUNC)(DWORD);
typedef void (__stdcall *RECVFUNC)(DWORD dwConnectionIndex, char* pMsg, DWORD dwLength);
typedef void (__stdcall *CONNECTSUCCESSFUNC)(DWORD dwConnectionIndex, void* pVoid);
typedef void (__stdcall *CONNECTFAILFUNC)(void* pVoid);
typedef void (__stdcall *DISCONNECTFUNC)(DWORD dwConnectionIndex);
typedef void (__stdcall *EVENTFUNC)(DWORD dwEventIndex);

// -------------------------------------------------------------------------
// 描述结构
// -------------------------------------------------------------------------
struct CUSTOM_EVENT
{
    DWORD       dwPeriodicTime;   // 0 = 外部 SetEvent 触发;非 0 = 周期触发
    EVENTFUNC   pEventFunc;
};

struct DESC_NETWORK
{
    DWORD           dwMaxUserNum;
    DWORD           dwMaxServerNum;
    RECVFUNC        OnRecvFromUserTCP;
    RECVFUNC        OnRecvFromServerTCP;
    ACCEPTFUNC      OnAcceptUser;
    ACCEPTFUNC      OnAcceptServer;
    DISCONNECTFUNC  OnDisconnectUser;
    DISCONNECTFUNC  OnDisconnectServer;
    DWORD           dwServerMaxTransferSize;
    DWORD           dwUserMaxTransferSize;
    DWORD           dwServerBufferSizePerConnection;
    DWORD           dwUserBufferSizePerConnection;
    DWORD           dwMainMsgQueMaxBufferSize;
    DWORD           dwConnectNumAtSameTime;
    DWORD           dwFlag;
    DWORD           dwCustomDefineEventNum;
    CUSTOM_EVENT*   pEvent;
};

struct PACKET_LIST
{
    char*           pMsg;
    DWORD           dwLen;
    PACKET_LIST*    pNext;
    DWORD           dwFlag;
};

// -------------------------------------------------------------------------
// I4DyuchiNET 接口
// -------------------------------------------------------------------------
// vtable 顺序: 严格保持与 stub 相同的声明顺序
//   IUnknown(3) + 28 个 I4DyuchiNET 方法 = 31 个槽位
// -------------------------------------------------------------------------
interface I4DyuchiNET : public IUnknown
{
    virtual BOOL            __stdcall CreateNetwork(DESC_NETWORK* desc, DWORD dwUserAcceptInterval, DWORD dwServerAcceptInterval) = 0;
    virtual void            __stdcall BreakMainThread() = 0;
    virtual void            __stdcall ResumeMainThread() = 0;
    virtual void            __stdcall SetUserInfo(DWORD dwConnectionIndex, void* user) = 0;
    virtual void*           __stdcall GetUserInfo(DWORD dwConnectionIndex) = 0;
    virtual void            __stdcall SetServerInfo(DWORD dwConnectionIndex, void* server) = 0;
    virtual void*           __stdcall GetServerInfo(DWORD dwConnectionIndex) = 0;
    virtual sockaddr_in*    __stdcall GetServerAddress(DWORD dwConnectionIndex) = 0;
    virtual sockaddr_in*    __stdcall GetUserAddress(DWORD dwConnectionIndex) = 0;
    virtual BOOL            __stdcall GetServerAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) = 0;
    virtual BOOL            __stdcall GetUserAddress(DWORD dwConnectionIndex, char* pIP, WORD* pwPort) = 0;
    virtual BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, char* msg, DWORD length, DWORD flag) = 0;
    virtual void            __stdcall CompulsiveDisconnectServer(DWORD dwConnectionIndex) = 0;
    virtual void            __stdcall CompulsiveDisconnectUser(DWORD dwConnectionIndex) = 0;
    virtual int             __stdcall GetServerMaxTransferRecvSize() = 0;
    virtual int             __stdcall GetServerMaxTransferSendSize() = 0;
    virtual int             __stdcall GetUserMaxTransferRecvSize() = 0;
    virtual int             __stdcall GetUserMaxTransferSendSize() = 0;
    virtual void            __stdcall BroadcastServer(char* pMsg, DWORD len, DWORD flag) = 0;
    virtual void            __stdcall BroadcastUser(char* pMsg, DWORD len, DWORD flag) = 0;
    virtual DWORD           __stdcall GetConnectedServerNum() = 0;
    virtual DWORD           __stdcall GetConnectedUserNum() = 0;
    virtual WORD            __stdcall GetBindedPortServerSide() = 0;
    virtual WORD            __stdcall GetBindedPortUserSide() = 0;
    virtual BOOL            __stdcall ConnectToServerWithUserSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt) = 0;
    virtual BOOL            __stdcall ConnectToServerWithServerSide(char* szIP, WORD port, CONNECTSUCCESSFUNC pSucc, CONNECTFAILFUNC pFail, void* pExt) = 0;
    virtual BOOL            __stdcall StartServerWithUserSide(char* ip, WORD port) = 0;
    virtual BOOL            __stdcall StartServerWithServerSide(char* ip, WORD port) = 0;
    virtual HANDLE          __stdcall GetCustomEventHandle(DWORD index) = 0;
    virtual BOOL            __stdcall PauseTimer(DWORD dwCustomEventIndex) = 0;
    virtual BOOL            __stdcall ResumeTimer(DWORD dwCustomEventIndex) = 0;
    virtual BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, WSABUF* pBuf, DWORD dwNum, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, WSABUF* pBuf, DWORD dwNum, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToServer(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) = 0;
    virtual BOOL            __stdcall SendToUser(DWORD dwConnectionIndex, PACKET_LIST* pList, DWORD flag) = 0;
};

#endif // _INETWORK_H_INCLUDED

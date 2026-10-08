// =====================================================================
// Smoke.cpp - 4DyuchiNETAsio 烟测程序
// =====================================================================
//
// 独立于 AgentServer,使用本机 Boost.Asio 客户端连接 4DyuchiNETAsio 的
// user-side 监听端口,验证:
//   1. CoCreateInstance 成功
//   2. CreateNetwork + StartServerWithUserSide 成功
//   3. 客户端连接 -> OnAcceptUser 触发
//   4. 客户端发送数据 -> OnRecvFromUserTCP 触发,payload 完整
//   5. 客户端断开 -> OnDisconnectUser 触发
//   6. SendToUser 双向通信
//
// 用法: Smoke.exe [port]   (默认 60000)
// =====================================================================

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>
#include <objbase.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

// inetwork.h 与 4DyuchiNETAsio.dll 的头文件保持一致
#include "../include/inetwork.h"

// 在 Smoke.exe 中本地定义 GUID(与 4DyuchiNETAsio.dll 中相同)
#define INITGUID
#include <guiddef.h>
// {11C02A88-8BF9-4863-A7DE-1BF66D60CBA3}
DEFINE_GUID(CLSID_4DyuchiNET,
    0x11c02a88, 0x8bf9, 0x4863, 0xa7, 0xde, 0x1b, 0xf6, 0x6d, 0x60, 0xcb, 0xa3);
// {D41BD0F8-07BF-4dbc-8AD3-A3524CC43E5E}
DEFINE_GUID(IID_4DyuchiNET,
    0xd41bd0f8, 0x07bf, 0x4dbc, 0x8a, 0xd3, 0xa3, 0x52, 0x4c, 0xc4, 0x3e, 0x5e);

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "ws2_32.lib")

// -------------------------------------------------------------------------
// 回调收集
// -------------------------------------------------------------------------
enum class CallbackKind
{
    AcceptUser,
    AcceptServer,
    DisconnectUser,
    DisconnectServer,
    RecvFromUser,
    RecvFromServer,
};

struct CallbackRecord
{
    CallbackKind  kind;
    DWORD         connection_index;
    std::vector<char> payload;  // 仅 Recv 用
};

static std::mutex                  g_cb_mutex;
static std::vector<CallbackRecord> g_callbacks;

static void RecordCallback(CallbackKind k, DWORD idx, const char* msg, DWORD len)
{
    std::lock_guard<std::mutex> lk(g_cb_mutex);
    CallbackRecord r;
    r.kind = k;
    r.connection_index = idx;
    if (msg != nullptr && len > 0) {
        r.payload.assign(msg, msg + len);
    }
    g_callbacks.push_back(std::move(r));
}

static void __stdcall OnAcceptUser(DWORD idx)        { RecordCallback(CallbackKind::AcceptUser, idx, nullptr, 0); }
static void __stdcall OnAcceptServer(DWORD idx)      { RecordCallback(CallbackKind::AcceptServer, idx, nullptr, 0); }
static void __stdcall OnDisconnectUser(DWORD idx)    { RecordCallback(CallbackKind::DisconnectUser, idx, nullptr, 0); }
static void __stdcall OnDisconnectServer(DWORD idx)  { RecordCallback(CallbackKind::DisconnectServer, idx, nullptr, 0); }
static void __stdcall OnRecvFromUserTCP(DWORD idx, char* pMsg, DWORD dwLength)
{
    RecordCallback(CallbackKind::RecvFromUser, idx, pMsg, dwLength);
}
static void __stdcall OnRecvFromServerTCP(DWORD idx, char* pMsg, DWORD dwLength)
{
    RecordCallback(CallbackKind::RecvFromServer, idx, pMsg, dwLength);
}

// -------------------------------------------------------------------------
// 简单测试事件
// -------------------------------------------------------------------------
static void __stdcall TimerCallback0(DWORD) { std::printf("[event0] fired\n"); }
static void __stdcall TimerCallback2(DWORD) { std::printf("[event2] fired\n"); }

// -------------------------------------------------------------------------
// 帧格式与 4DyuchiNETAsio 协议一致
// -------------------------------------------------------------------------
static void SendFrame(boost::asio::ip::tcp::socket& s, const char* payload, std::size_t len)
{
    std::uint32_t hdr = static_cast<std::uint32_t>(len);
    boost::asio::write(s, boost::asio::buffer(&hdr, 4));
    if (len > 0) {
        boost::asio::write(s, boost::asio::buffer(payload, len));
    }
}

static bool RecvExact(boost::asio::ip::tcp::socket& s, char* dst, std::size_t n)
{
    std::size_t got = 0;
    while (got < n) {
        boost::system::error_code ec;
        std::size_t r = s.read_some(boost::asio::buffer(dst + got, n - got), ec);
        if (ec) return false;
        got += r;
    }
    return true;
}

static bool RecvFrame(boost::asio::ip::tcp::socket& s, std::vector<char>& out)
{
    std::uint32_t hdr = 0;
    if (!RecvExact(s, reinterpret_cast<char*>(&hdr), 4)) return false;
    out.assign(hdr, 0);
    if (hdr == 0) return true;
    return RecvExact(s, out.data(), hdr);
}

// -------------------------------------------------------------------------
// 主测试
// -------------------------------------------------------------------------
static int g_test_failures = 0;

#define EXPECT(cond, msg) do { \
    if (!(cond)) { std::printf("[FAIL] %s (line %d): %s\n", #cond, __LINE__, msg); ++g_test_failures; } \
    else        { std::printf("[ OK ] %s\n", msg); } \
} while(0)

static void ClearCallbacks()
{
    std::lock_guard<std::mutex> lk(g_cb_mutex);
    g_callbacks.clear();
}

static const std::vector<CallbackRecord>& SnapshotCallbacks()
{
    return g_callbacks;
}

int main(int argc, char* argv[])
{
    WORD port = 60000;
    if (argc >= 2) port = static_cast<WORD>(std::atoi(argv[1]));

    // 启动 winsock (Asio 在 Windows 下自动调用,但显式更稳)
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);

    // COM 初始化
    HRESULT hr_init = CoInitialize(nullptr);
    if (FAILED(hr_init)) {
        std::printf("CoInitialize failed: 0x%08X\n", hr_init);
        return 1;
    }

    // 创建 I4DyuchiNET
    I4DyuchiNET* pINet = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_4DyuchiNET, nullptr, CLSCTX_INPROC_SERVER,
                                  IID_4DyuchiNET, reinterpret_cast<void**>(&pINet));
    if (FAILED(hr) || pINet == nullptr) {
        std::printf("CoCreateInstance failed: 0x%08X\n", hr);
        CoUninitialize();
        return 1;
    }
    std::printf("[ OK ] CoCreateInstance succeeded\n");

    // CreateNetwork
    CUSTOM_EVENT ev[3];
    ev[0].dwPeriodicTime = 1000;
    ev[0].pEventFunc     = TimerCallback0;
    ev[1].dwPeriodicTime = 0;
    ev[1].pEventFunc     = nullptr;       // 不需要回调
    ev[2].dwPeriodicTime = 1000;
    ev[2].pEventFunc     = TimerCallback2;

    DESC_NETWORK desc;
    std::memset(&desc, 0, sizeof(desc));
    desc.OnAcceptUser         = OnAcceptUser;
    desc.OnAcceptServer       = OnAcceptServer;
    desc.OnDisconnectUser     = OnDisconnectUser;
    desc.OnDisconnectServer   = OnDisconnectServer;
    desc.OnRecvFromUserTCP    = OnRecvFromUserTCP;
    desc.OnRecvFromServerTCP  = OnRecvFromServerTCP;
    desc.dwMaxUserNum         = 16;
    desc.dwMaxServerNum       = 16;
    desc.dwServerMaxTransferSize  = 65000;
    desc.dwUserMaxTransferSize    = 8192;
    desc.dwServerBufferSizePerConnection = 256000;
    desc.dwUserBufferSizePerConnection   = 65000;
    desc.dwMainMsgQueMaxBufferSize       = 5120000;
    desc.dwConnectNumAtSameTime          = 16;
    desc.dwCustomDefineEventNum          = 3;
    desc.pEvent                          = ev;

    EXPECT(pINet->CreateNetwork(&desc, 10, 10) == TRUE, "CreateNetwork");

    // 启动 user 侧监听
    EXPECT(pINet->StartServerWithUserSide(const_cast<char*>("127.0.0.1"), port) == TRUE,
           "StartServerWithUserSide");

    // GetCustomEventHandle(1) -> 验证非 NULL
    HANDLE h = pINet->GetCustomEventHandle(1);
    EXPECT(h != nullptr, "GetCustomEventHandle(1) returns non-NULL");

    // Sleep 一小段时间,让 server 进入 accept
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // ---- 测试 1:客户端连接 + 发送 + 接收 + 关闭 ----
    ClearCallbacks();

    boost::asio::io_context client_ioc;
    boost::asio::ip::tcp::socket client(client_ioc);
    boost::system::error_code ec;
    client.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port), ec);
    EXPECT(!ec, "client.connect");

    // 等待 OnAcceptUser
    for (int i = 0; i < 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lk(g_cb_mutex);
            if (!g_callbacks.empty()) break;
        }
    }
    {
        std::lock_guard<std::mutex> lk(g_cb_mutex);
        bool found_accept = false;
        DWORD accept_idx = 0;
        for (auto& r : g_callbacks) {
            if (r.kind == CallbackKind::AcceptUser) { found_accept = true; accept_idx = r.connection_index; break; }
        }
        EXPECT(found_accept, "OnAcceptUser fired");
        if (found_accept) {
            std::printf("    accept idx = %u\n", accept_idx);
        }
    }

    // 发送一帧 4 字节 0xDEADBEEF
    const char payload[] = { '\xEF', '\xBE', '\xAD', '\xDE' };
    SendFrame(client, payload, 4);

    // 等待 OnRecvFromUserTCP
    for (int i = 0; i < 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lk(g_cb_mutex);
            bool found = false;
            for (auto& r : g_callbacks) {
                if (r.kind == CallbackKind::RecvFromUser) { found = true; break; }
            }
            if (found) break;
        }
    }
    {
        std::lock_guard<std::mutex> lk(g_cb_mutex);
        bool found_recv = false;
        for (auto& r : g_callbacks) {
            if (r.kind == CallbackKind::RecvFromUser) {
                found_recv = true;
                bool payload_ok = (r.payload.size() == 4) &&
                                  r.payload[0] == '\xEF' && r.payload[1] == '\xBE' &&
                                  r.payload[2] == '\xAD' && r.payload[3] == '\xDE';
                EXPECT(payload_ok, "OnRecvFromUserTCP payload matches");
                break;
            }
        }
        EXPECT(found_recv, "OnRecvFromUserTCP fired");
    }

    // 测试 SendToUser 反向发数据
    {
        std::lock_guard<std::mutex> lk(g_cb_mutex);
        // 找到 accept 的 idx
        DWORD accept_idx = 0;
        for (auto& r : g_callbacks) {
            if (r.kind == CallbackKind::AcceptUser) { accept_idx = r.connection_index; break; }
        }
        if (accept_idx != 0) {
            const char echo[] = "hello";
            BOOL sr = pINet->SendToUser(accept_idx, const_cast<char*>(echo), 5, 0);
            EXPECT(sr == TRUE, "SendToUser returns TRUE");
        }
    }
    // 客户端读一帧
    {
        std::vector<char> got;
        bool ok = RecvFrame(client, got);
        EXPECT(ok, "client received frame");
        EXPECT(got.size() == 5 && std::memcmp(got.data(), "hello", 5) == 0, "client got 'hello'");
    }

    // 测试 GetUserAddress 稳定性
    {
        std::lock_guard<std::mutex> lk(g_cb_mutex);
        DWORD accept_idx = 0;
        for (auto& r : g_callbacks) {
            if (r.kind == CallbackKind::AcceptUser) { accept_idx = r.connection_index; break; }
        }
        if (accept_idx != 0) {
            sockaddr_in* a1 = pINet->GetUserAddress(accept_idx);
            sockaddr_in* a2 = pINet->GetUserAddress(accept_idx);
            EXPECT(a1 != nullptr && a1 == a2, "GetUserAddress stable pointer");
        }
    }

    // 测试 SetUserInfo / GetUserInfo
    {
        std::lock_guard<std::mutex> lk(g_cb_mutex);
        DWORD accept_idx = 0;
        for (auto& r : g_callbacks) {
            if (r.kind == CallbackKind::AcceptUser) { accept_idx = r.connection_index; break; }
        }
        if (accept_idx != 0) {
            int marker = 0x12345678;
            pINet->SetUserInfo(accept_idx, &marker);
            void* p = pINet->GetUserInfo(accept_idx);
            EXPECT(p == &marker, "SetUserInfo / GetUserInfo roundtrip");
        }
    }

    // 关闭客户端
    client.close();

    // 等待 OnDisconnectUser
    for (int i = 0; i < 50; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::lock_guard<std::mutex> lk(g_cb_mutex);
            bool found = false;
            for (auto& r : g_callbacks) {
                if (r.kind == CallbackKind::DisconnectUser) { found = true; break; }
            }
            if (found) break;
        }
    }
    {
        std::lock_guard<std::mutex> lk(g_cb_mutex);
        bool found = false;
        for (auto& r : g_callbacks) {
            if (r.kind == CallbackKind::DisconnectUser) { found = true; break; }
        }
        EXPECT(found, "OnDisconnectUser fired");
    }

    // 测试 PauseTimer(0) / ResumeTimer(0)
    EXPECT(pINet->PauseTimer(0) == TRUE, "PauseTimer(0)");
    EXPECT(pINet->ResumeTimer(0) == TRUE, "ResumeTimer(0)");
    EXPECT(pINet->ResumeTimer(2) == TRUE, "ResumeTimer(2)");

    // 测试 SetEvent -> 触发外部事件回调
    if (h != nullptr) {
        // 先给一个 SetEvent
        SetEvent(h);
        // 等等 callback
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // 我们没有为 ev[1] 设置回调,这里只是确保不崩
    }

    // 释放
    pINet->Release();

    CoFreeUnusedLibraries();
    CoUninitialize();
    WSACleanup();

    std::printf("\n=========================\n");
    if (g_test_failures == 0) {
        std::printf("ALL TESTS PASSED\n");
        return 0;
    } else {
        std::printf("%d TEST(S) FAILED\n", g_test_failures);
        return 1;
    }
}

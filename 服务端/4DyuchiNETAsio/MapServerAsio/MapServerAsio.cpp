// =====================================================================
// MapServerAsio.cpp - 最小化骨架版 MapServer
// =====================================================================
//
// 目的:
//   1. 验证 I4DyuchiNET 接口(4DyuchiNETAsio 后端)能正确驱动 MapServer
//   2. 桥接 7 个 CUSTOM_EVENT 槽位(4DyuchiNETAsio 限 3 个,后 4 个由
//      MapServerTimerBridge 接管)
//   3. 连接 MockProxy,执行简化注册握手(PTCL=50 -> ACK=51)
//   4. 维持运行循环,响应 F1/F5/F6/F7 热键
//
// 范围(本最小化版本不做的事):
//   - 不加载 .tbl/.bin/.map/.toi2/.skb 数据文件(InitDRMapServerDatas)
//   - 不连 MySQL(Init_SQL)
//   - 不做 HSEL 解码(IdPassword.bin)
//   - 不实现 NPC 系统、技能系统、战斗系统
//   - 不接受 AgentServer 业务消息(只做"能联通 MockProxy"的最少流程)
//
// 协议(与 MockProxy 一致):
//   帧格式: [4 bytes: DWORD, little-endian, length N][N bytes: payload]
//   payload 第一字节 = PTCL ID
//
//   PTCL_MAP_REGISTER (50):
//     [4 bytes map_id LE] + [2 bytes cbNameLen LE] + [N bytes map_name]
//
//   PTCL_MAP_REGISTER_ACK (51):
//     [4 bytes map_id LE] + [2 bytes cbNameLen LE] + [N bytes map_name]
//
//   业务消息: 任意 PTCL,MockProxy 收到后转给已注册的 AgentServer(若有)
//
// 用法:MapServerAsio.exe [map_id] [map_name] [proxy_host] [proxy_port]
//      默认: map_id=1  map_name="MapA"  proxy_host="127.0.0.1"  proxy_port=3000
// =====================================================================

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <objbase.h>      // COM
#include <olectl.h>

// 强制 DEFINE_GUID 在本 TU 中生成 GUID 定义(参考 4DyuchiNETAsio.dll 的做法)
#define INITGUID
#include <guiddef.h>

// GUID 定义 (与 4DyuchiNETAsio/src/4DyuchiNETAsio.cpp 一致)
DEFINE_GUID(CLSID_4DyuchiNET,
    0x11c02a88, 0x8bf9, 0x4863, 0xa7, 0xde, 0x1b, 0xf6, 0x6d, 0x60, 0xcb, 0xa3);

DEFINE_GUID(IID_4DyuchiNET,
    0xd41bd0f8, 0x07bf, 0x4dbc, 0x8a, 0xd3, 0xa3, 0x52, 0x4c, 0xc4, 0x3e, 0x5e);

#include "../include/inetwork.h"

#include "MapServerTimerBridge.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "ole32.lib")

// =====================================================================
// 全局状态(最小化骨架版,大部分为桩)
// =====================================================================
static I4DyuchiNET*         g_pINet         = nullptr;
static HANDLE               hKeyEvent[4]    = { nullptr, nullptr, nullptr, nullptr };
static std::unique_ptr<MapServerTimerBridge> g_bridge;

static std::uint32_t        g_map_id        = 0;
static std::string          g_map_name;
static std::string          g_proxy_host    = "127.0.0.1";
static std::uint16_t        g_proxy_port    = 3000;

// Phase 3: 本 MapServer 对 AgentServer 的监听端口(0 = 不监听)
static std::uint16_t        g_listen_port   = 0;
static std::atomic<bool>    g_listen_ok     { false };

// MockProxy 连接的连接索引(由 I4DyuchiNET 分配);0 = 未连接
static std::atomic<DWORD>   g_proxy_conn_idx{ 0 };
// 重试连接用的扩展(任意指针)
static int                  g_connect_cookie = 0;

// 注册状态
static std::atomic<bool>    g_registered{ false };

// Phase 2: 业务消息
static std::uint32_t        g_business_interval_ms = 5000;   // 周期业务消息间隔(0=不发)
static std::uint32_t        g_auto_target_map_id   = 0;      // 注册后向该 map 发送一次性 map-to-map 业务消息(0=不发)
static std::atomic<bool>    g_auto_target_sent{ false };     // 一次性发送的闸
static std::atomic<bool>    g_auto_echo_enabled{ true };     // 收到 PTCL_AGENT_TO_MAP 是否自动回声
static std::atomic<std::uint64_t> g_business_counter{ 0 };   // 已发送业务消息计数
static std::atomic<std::uint64_t> g_echo_counter{ 0 };       // 已回声计数

// PTCL 业务消息常量
static constexpr std::uint8_t PTCL_MAP_TO_MAP_INTERNAL = 56; // map-to-map 直传
static constexpr std::uint8_t PTCL_DEMO_BUSINESS       = 100; // 业务负载(自定义)
static constexpr std::uint8_t ECHO_TAG                 = 0xEE; // 回声标识(防回声-回声-回声 死循环)
static std::atomic<std::uint64_t> g_echo_received{ 0 }; // 收到回声计数(命中 ECHO_TAG 后 +1)

// 周期定时器周期(取 4DyuchiNETAsio 的实际语义,不是原 MapServer 的精确值)
static constexpr DWORD kAwaitingProxyMs   = 1000;     // 1s
static constexpr DWORD kGameTimerMs       = 1000;     // 主循环周期(原 GTE_ROUNDING_TIME 不易获取,用 1s 占位)
static constexpr DWORD kCleanUpDeadMs     = 75000;    // 75s

// 日志
namespace {
std::mutex g_log_mu;

template <typename... Args>
void log_line(const char* fmt, Args... args) {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    char ts[32];
    std::snprintf(ts, sizeof(ts), "%02d:%02d:%02d.%03d",
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    std::lock_guard<std::mutex> lk(g_log_mu);
    std::printf("[%s] [MapServer] ", ts);
    std::printf(fmt, args...);
    std::printf("\n");
    std::fflush(stdout);
}
} // namespace

// =====================================================================
// I4DyuchiNET 回调
// =====================================================================

// MapServer 不接受 user 端连接(只跟 Proxy 通信),但接口需要
void __stdcall OnAcceptUser(DWORD dwConnectionIndex) {
    log_line("OnAcceptUser(idx=%u) - unexpected in MapServer skeleton", dwConnectionIndex);
}

void __stdcall OnAcceptServer(DWORD dwConnectionIndex) {
    log_line("OnAcceptServer(idx=%u)", dwConnectionIndex);
}

void __stdcall OnDisconnectUser(DWORD dwConnectionIndex) {
    log_line("OnDisconnectUser(idx=%u)", dwConnectionIndex);
}

void __stdcall OnDisconnectServer(DWORD dwConnectionIndex) {
    log_line("OnDisconnectServer(idx=%u)", dwConnectionIndex);
    if (dwConnectionIndex == g_proxy_conn_idx.load()) {
        log_line("  -> Proxy connection lost, will retry in AwaitingProxyServerConnect");
        g_proxy_conn_idx.store(0);
        g_registered.store(false);
    }
}

void __stdcall ReceivedMsgFromUser(DWORD /*dwConnectionIndex*/, char* /*pMsg*/, DWORD /*dwLength*/) {
    // MapServer 不接受 user 端
}

void __stdcall ReceivedMsgFromServer(DWORD dwConnectionIndex, char* pMsg, DWORD dwLength) {
    if (dwLength < 1 || pMsg == nullptr) {
        log_line("ReceivedMsgFromServer(idx=%u): empty/invalid frame", dwConnectionIndex);
        return;
    }
    const std::uint8_t bID = static_cast<std::uint8_t>(pMsg[0]);
    const char* body     = pMsg + 1;
    const std::size_t body_len = dwLength - 1;

    // -----------------------------------------------------------------
    // Phase 5a: 完整应用层握手
    // 协议来源:AgentServer\servertable.cpp OnConnectServerSuccess / BeginNegotiationWithNormalServer
    //   1) AgentServer(active) -> Map(passive):
    //        PTCL_NOTIFY_SERVER_UP (1) + 2 字节 wPort (AgentServer 自己的端口)
    //        PTCL_NOTIFY_SERVER_STATUS (3) + 4 字节 dwStatus
    //   2) Map(passive) -> AgentServer(active):
    //        PTCL_NOTIFY_SERVER_STATUS (3) + 4 字节 dwStatus
    //
    // 反直觉点:Map **不应回 PTCL=1** (NOTIFY_SERVER_UP)。
    //  原因:AgentServer 在 OnConnectServerSuccess 阶段(line 2967)已经把
    //   pServerData->dwConnectionIndex 绑定到这次连接;若 Map 后续发 PTCL=1,
    //   AgentServer 的 OnRecvServerUpMsg 会进入"is Already Connected"分支并
    //   CompulsiveDisconnectServer(实测已验证)。
    //  原始 MapServer 也不会回 PTCL=1,只回 PTCL=3。
    //
    // 后续:AgentServer 收到 Map 的 PTCL=3 -> OnRecvNegotiationMsgs 的
    //   PTCL_NOTIFY_SERVER_STATUS 分支(line 725)只调 SetServerStatus,不绑连接。
    //   该分支在非 PROXY 发送方过滤器(白名单)中合法。
    // -----------------------------------------------------------------
    if (bID == 1 /*PTCL_NOTIFY_SERVER_UP*/ && body_len >= 2) {
        std::uint16_t peer_wport = 0;
        std::memcpy(&peer_wport, body, 2);
        log_line("<- PTCL_NOTIFY_SERVER_UP (handshake step 1) from peer_wport=%u, conn_idx=%u",
                 static_cast<unsigned>(peer_wport), dwConnectionIndex);

        // 握手步骤 2:回 PTCL=3 (NOTIFY_SERVER_STATUS),带 STATUS_ACTIVATED=10
        // payload: [PTCL=3] + [4 bytes dwStatus LE]
        //   STATUS_ACTIVATED = 10 (AgentServer\servertable.h:63)
        std::vector<char> resp;
        resp.push_back(static_cast<char>(3));  // PTCL_NOTIFY_SERVER_STATUS
        std::uint32_t status = 10;            // STATUS_ACTIVATED
        for (int i = 0; i < 4; ++i) {
            resp.push_back(static_cast<char>((status >> (i * 8)) & 0xFF));
        }
        if (g_pINet->SendToServer(dwConnectionIndex, resp.data(),
                                  static_cast<DWORD>(resp.size()), 0)) {
            log_line("-> PTCL_NOTIFY_SERVER_STATUS (handshake step 2) status=STATUS_ACTIVATED(10) -> conn_idx=%u",
                     dwConnectionIndex);
        } else {
            log_line("ERROR: SendToServer(PTCL_NOTIFY_SERVER_STATUS reply) failed");
        }
        return;
    } else if (bID == 3 /*PTCL_NOTIFY_SERVER_STATUS*/) {
        // AgentServer 在 BeginNegotiationWithNormalServer 中紧随 PTCL=1 发出 PTCL=3
        // (5 字节: 1 byte bID + 4 bytes dwStatus)
        if (body_len >= 4) {
            std::uint32_t peer_status = 0;
            std::memcpy(&peer_status, body, 4);
            log_line("<- PTCL_NOTIFY_SERVER_STATUS from AgentServer (status=%u, conn_idx=%u)",
                     peer_status, dwConnectionIndex);
        } else {
            log_line("<- PTCL_NOTIFY_SERVER_STATUS from AgentServer (short frame, body_len=%zu)",
                     body_len);
        }
        return;
    } else if (bID == 51 /*PTCL_MAP_REGISTER_ACK*/ && body_len >= 6) {
        std::uint32_t echo_id = 0;
        std::memcpy(&echo_id, body, 4);
        std::uint16_t name_len = 0;
        std::memcpy(&name_len, body + 4, 2);
        std::string echo_name(body + 6, std::min<std::size_t>(name_len, body_len - 6));
        log_line("<- PTCL_MAP_REGISTER_ACK map_id=%u name=\"%s\" (len=%u)",
                 echo_id, echo_name.c_str(), name_len);
        g_registered.store(true);
    } else if (bID == 52 /*PTCL_AGENT_TO_MAP*/ && body_len >= 4) {
        // PTCL_AGENT_TO_MAP (52) + [4 bytes from_map_id] + [业务 payload]
        // 这里的 from_map_id 是"消息来源 map_id"(由 MockProxy route_map_to_map 包装)
        // target_map_id 字段在本骨架中冗余(始终是本 map),这里只取 from_map_id
        std::uint32_t from_map_id = 0;
        std::memcpy(&from_map_id, body, 4);
        const char* business = body + 4;
        const std::size_t business_len = body_len - 4;

        std::string hex_preview = "";
        for (size_t i = 0; i < std::min<std::size_t>(business_len, 16); ++i) {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%02X ", static_cast<unsigned char>(business[i]));
            hex_preview += buf;
        }
        log_line("<- PTCL_AGENT_TO_MAP from_map_id=%u, %zu bytes payload (preview=%s%s)",
                 from_map_id, business_len, hex_preview.c_str(),
                 business_len > 16 ? "..." : "");

        // 环回检测:如果业务 payload 是我们自己发出去的 echo 格式(PTCL=100 + ECHO_TAG),
        // 说明这是对端的回声,只记数,不再回声
        // echo 业务格式: [0x64=PTCL_DEMO_BUSINESS] [0xEE=ECHO_TAG] [8 bytes counter_be] [4 bytes from_map_id_be]
        if (g_auto_echo_enabled.load() && business_len >= 2
            && static_cast<std::uint8_t>(business[0]) == PTCL_DEMO_BUSINESS
            && static_cast<std::uint8_t>(business[1]) == ECHO_TAG) {
            const std::uint64_t recv_count = g_echo_received.fetch_add(1, std::memory_order_relaxed) + 1;
            log_line("   ECHO_TAG hit -> %s (echo_received=%llu)",
                     "loop break, no re-echo",
                     static_cast<unsigned long long>(recv_count));
            return;  // 不回声
        }

        // 自动回声:构造 PTCL_MAP_TO_MAP (56) + [4 bytes target=from_map_id] + [业务响应]
        if (g_auto_echo_enabled.load() && g_pINet != nullptr) {
            // 业务 payload: PTCL=100 + ECHO_TAG + [8 bytes echo_counter_be] + [4 bytes from_map_id_be]
            std::vector<char> resp;
            resp.push_back(static_cast<char>(PTCL_DEMO_BUSINESS));   // 0x64
            resp.push_back(static_cast<char>(ECHO_TAG));             // 0xEE 防止循环回声
            std::uint64_t counter = g_echo_counter.fetch_add(1, std::memory_order_relaxed) + 1;
            for (int i = 7; i >= 0; --i) {
                resp.push_back(static_cast<char>((counter >> (i * 8)) & 0xFF));
            }
            for (int i = 3; i >= 0; --i) {
                resp.push_back(static_cast<char>((from_map_id >> (i * 8)) & 0xFF));
            }
            // 拼成 PTCL_MAP_TO_MAP (56) + [4 bytes target_be] + business
            std::vector<char> frame;
            frame.push_back(static_cast<char>(PTCL_MAP_TO_MAP_INTERNAL));
            for (int i = 0; i < 4; ++i) {
                frame.push_back(static_cast<char>((from_map_id >> (i * 8)) & 0xFF));
            }
            frame.insert(frame.end(), resp.begin(), resp.end());

            const DWORD conn_idx = g_proxy_conn_idx.load();
            if (conn_idx != 0) {
                if (g_pINet->SendToServer(conn_idx, frame.data(),
                                          static_cast<DWORD>(frame.size()), 0)) {
                    log_line("-> PTCL_MAP_TO_MAP echo to map_id=%u (%zu bytes, counter=%llu)",
                             from_map_id, frame.size(),
                             static_cast<unsigned long long>(counter));
                } else {
                    log_line("ERROR: SendToServer(PTCL_MAP_TO_MAP) failed");
                }
            } else {
                log_line("WARN: proxy_conn==0, skip echo");
            }
        }
    } else if (bID == 55 /*PTCL_MAP_LIST_RESPONSE*/) {
        log_line("<- PTCL_MAP_LIST_RESPONSE (%zu bytes)", body_len);
    } else {
        log_line("<- RECV PTCL=%u len=%u (other)", bID, dwLength);
    }
}

// =====================================================================
// I4DyuchiNET 连接回调
// =====================================================================
void __stdcall OnConnectToProxySuccess(DWORD dwConnectionIndex, void* pExt) {
    int cookie = pExt ? *static_cast<int*>(pExt) : -1;
    log_line("ConnectToProxy SUCCESS idx=%u cookie=%d", dwConnectionIndex, cookie);
    g_proxy_conn_idx.store(dwConnectionIndex);
    g_registered.store(false);  // 还没注册

    // 发送 PTCL_MAP_REGISTER
    // payload: [PTCL=50] + [4 bytes map_id LE] + [2 bytes cbNameLen LE] + [name]
    //        + [2 bytes listen_port LE]   (Phase 3 新增)
    std::vector<char> p;
    p.push_back(static_cast<char>(50));  // PTCL_MAP_REGISTER
    std::uint32_t id_le = g_map_id;
    for (int i = 0; i < 4; ++i) {
        p.push_back(static_cast<char>((id_le >> (i * 8)) & 0xFF));
    }
    std::uint16_t name_len = static_cast<std::uint16_t>(g_map_name.size());
    for (int i = 0; i < 2; ++i) {
        p.push_back(static_cast<char>((name_len >> (i * 8)) & 0xFF));
    }
    p.insert(p.end(), g_map_name.begin(), g_map_name.end());
    // Phase 3: 追加 listen_port
    for (int i = 0; i < 2; ++i) {
        p.push_back(static_cast<char>((g_listen_port >> (i * 8)) & 0xFF));
    }

    if (g_pINet->SendToServer(dwConnectionIndex, p.data(), static_cast<DWORD>(p.size()), 0)) {
        log_line("-> PTCL_MAP_REGISTER map_id=%u name=\"%s\" port=%u (%zu bytes)",
                 g_map_id, g_map_name.c_str(), static_cast<unsigned>(g_listen_port), p.size());
    } else {
        log_line("ERROR: SendToServer(PTCL_MAP_REGISTER) failed");
    }
}

void __stdcall OnConnectToProxyFail(void* pExt) {
    int cookie = pExt ? *static_cast<int*>(pExt) : -1;
    log_line("ConnectToProxy FAIL cookie=%d - will retry next tick", cookie);
    g_proxy_conn_idx.store(0);
}

// =====================================================================
// CUSTOM_EVENT 周期定时器回调(由 4DyuchiNETAsio 在 1000ms 周期触发)
//
// 对应 4DyuchiNETAsio 的 3 个槽位:
//   槽 0: AwaitingProxyServerConnect  (原 ev[0], 1000ms)
//   槽 1: GameTimerProcess            (原 ev[1], 周期,本骨架用 1s 占位)
//   槽 2: CleanUpDeadConnections      (原 ev[2], 75000ms)
// =====================================================================

// connections[DRAGON_MAX_CONNECTIONS_+1] 在原 LowerLayers/servertable.h 中定义;
// 骨架不引入 LowerLayers,所以这里给一个简化版"未连接即重连"的逻辑
static std::atomic<std::uint64_t> g_tick_counter{ 0 };
static std::atomic<std::uint64_t> g_dead_check_counter{ 0 };

void __stdcall AwaitingProxyServerConnect(DWORD /*dwEventIndex*/) {
    g_tick_counter.fetch_add(1, std::memory_order_relaxed);
    if (g_proxy_conn_idx.load() != 0) {
        // 已连接,不重试
        return;
    }
    log_line("[tick #%llu] trying ConnectToServerWithServerSide(%s:%u)",
             static_cast<unsigned long long>(g_tick_counter.load()),
             g_proxy_host.c_str(), g_proxy_port);
    g_connect_cookie++;
    if (!g_pINet->ConnectToServerWithServerSide(
            const_cast<char*>(g_proxy_host.c_str()), g_proxy_port,
            OnConnectToProxySuccess, OnConnectToProxyFail,
            &g_connect_cookie)) {
        log_line("  -> ConnectToServerWithServerSide returned FALSE (sync fail)");
    }
}

void __stdcall GameTimerProcess(DWORD /*dwEventIndex*/) {
    // 真实 MapServer 在此推进 NPC/战斗/天气/经济等系统
    // 骨架版本仅记录 tick 计数
    g_tick_counter.fetch_add(1, std::memory_order_relaxed);
    if ((g_tick_counter.load() % 60) == 0) {
        log_line("GameTimerProcess tick (60s boundary, total=%llu)",
                 static_cast<unsigned long long>(g_tick_counter.load()));
    }
}

void __stdcall CleanUpDeadConnections(DWORD /*dwEventIndex*/) {
    // 真实 MapServer 在此遍历 connections[] 数组,关闭长时间无响应的用户
    // 骨架版本:每 75s 打印一次心跳
    auto n = g_dead_check_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    log_line("CleanUpDeadConnections check #%llu (skeleton - no connections to scan)",
             static_cast<unsigned long long>(n));
}

// =====================================================================
// Phase 2 业务消息:周期发送 PTCL=100 业务消息
// =====================================================================
static void send_business_message() {
    const DWORD conn_idx = g_proxy_conn_idx.load();
    if (conn_idx == 0 || g_pINet == nullptr) return;

    // payload: [PTCL=100] + [8 bytes counter_be] + [4 bytes map_id_be] + 8 bytes ascii
    std::vector<char> p;
    p.push_back(static_cast<char>(PTCL_DEMO_BUSINESS));
    std::uint64_t counter = g_business_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    for (int i = 7; i >= 0; --i) {
        p.push_back(static_cast<char>((counter >> (i * 8)) & 0xFF));
    }
    for (int i = 3; i >= 0; --i) {
        p.push_back(static_cast<char>((g_map_id >> (i * 8)) & 0xFF));
    }
    const char tag[8] = "MapBK!";
    p.insert(p.end(), tag, tag + 8);

    if (!g_pINet->SendToServer(conn_idx, p.data(), static_cast<DWORD>(p.size()), 0)) {
        log_line("WARN: SendToServer(PTCL_DEMO_BUSINESS) failed");
    } else {
        log_line("-> PTCL_DEMO_BUSINESS #%llu (map_id=%u, %zu bytes)",
                 static_cast<unsigned long long>(counter), g_map_id, p.size());
    }
}

static void send_map_to_map_once(std::uint32_t target_map_id) {
    const DWORD conn_idx = g_proxy_conn_idx.load();
    if (conn_idx == 0 || g_pINet == nullptr) {
        log_line("WARN: send_map_to_map_once: proxy_conn==0, skip");
        return;
    }
    // payload: PTCL_MAP_TO_MAP (56) + [4 bytes target_le] + business
    // business: PTCL=100 + [8 bytes counter_be] + [4 bytes from_map_id_be] + 8 bytes ascii
    std::vector<char> business;
    business.push_back(static_cast<char>(PTCL_DEMO_BUSINESS));
    std::uint64_t counter = g_business_counter.fetch_add(1, std::memory_order_relaxed) + 1;
    for (int i = 7; i >= 0; --i) {
        business.push_back(static_cast<char>((counter >> (i * 8)) & 0xFF));
    }
    for (int i = 3; i >= 0; --i) {
        business.push_back(static_cast<char>((g_map_id >> (i * 8)) & 0xFF));
    }
    const char tag[8] = "MapAtoB";
    business.insert(business.end(), tag, tag + 8);

    std::vector<char> frame;
    frame.push_back(static_cast<char>(PTCL_MAP_TO_MAP_INTERNAL));
    for (int i = 0; i < 4; ++i) {
        frame.push_back(static_cast<char>((target_map_id >> (i * 8)) & 0xFF));
    }
    frame.insert(frame.end(), business.begin(), business.end());

    if (!g_pINet->SendToServer(conn_idx, frame.data(),
                               static_cast<DWORD>(frame.size()), 0)) {
        log_line("WARN: SendToServer(PTCL_MAP_TO_MAP) failed");
    } else {
        log_line("-> PTCL_MAP_TO_MAP target_map_id=%u (%zu bytes, counter=%llu)",
                 target_map_id, frame.size(),
                 static_cast<unsigned long long>(counter));
    }
}

// =====================================================================
// CUSTOM_EVENT 外部事件回调(由 MapServerTimerBridge 在 SetEvent 后触发)
//
// 对应原 ev[3..6]:
//   ev[3] -> F1  ShowMapServerStatus
//   ev[4] -> F5  ReLoadGameServerDataByKeyInput
//   ev[5] -> F6  SaveNPCStatusByKeyInput
//   ev[6] -> F7  MakeMapDataFile
// =====================================================================

void __stdcall ShowMapServerStatus(DWORD dwEventIndex) {
    log_line("[F1] ShowMapServerStatus(event=%u) map_id=%u name=\"%s\" registered=%s "
             "proxy_conn=%u",
             dwEventIndex, g_map_id, g_map_name.c_str(),
             g_registered.load() ? "yes" : "no",
             g_proxy_conn_idx.load());
}

void __stdcall ReLoadGameServerDataByKeyInput(DWORD dwEventIndex) {
    log_line("[F5] ReLoadGameServerDataByKeyInput(event=%u) - skeleton: skip (no data files)",
             dwEventIndex);
}

void __stdcall SaveNPCStatusByKeyInput(DWORD dwEventIndex) {
    log_line("[F6] SaveNPCStatusByKeyInput(event=%u) - skeleton: skip (no NPC system)",
             dwEventIndex);
}

void __stdcall MakeMapDataFile(DWORD dwEventIndex) {
    log_line("[F7] MakeMapDataFile(event=%u) - skeleton: skip (no map data)",
             dwEventIndex);
}

// =====================================================================
// Init() - 初始化 I4DyuchiNET + 桥接器
// =====================================================================
static bool g_initialized = false;

bool Init() {
    if (g_initialized) return true;

    // ---- 1. 构建 DESC_NETWORK(声明 7 个事件;4DyuchiNETAsio 会截断到 3) ----
    CUSTOM_EVENT ev[7];
    ev[0].dwPeriodicTime = kAwaitingProxyMs;
    ev[0].pEventFunc     = AwaitingProxyServerConnect;
    ev[1].dwPeriodicTime = kGameTimerMs;
    ev[1].pEventFunc     = GameTimerProcess;
    ev[2].dwPeriodicTime = kCleanUpDeadMs;
    ev[2].pEventFunc     = CleanUpDeadConnections;
    // 后 4 个是外部 SetEvent 事件;4DyuchiNETAsio 不会处理,桥接器接管
    ev[3].dwPeriodicTime = 0;
    ev[3].pEventFunc     = ShowMapServerStatus;
    ev[4].dwPeriodicTime = 0;
    ev[4].pEventFunc     = ReLoadGameServerDataByKeyInput;
    ev[5].dwPeriodicTime = 0;
    ev[5].pEventFunc     = SaveNPCStatusByKeyInput;
    ev[6].dwPeriodicTime = 0;
    ev[6].pEventFunc     = MakeMapDataFile;

    DESC_NETWORK desc{};
    desc.dwMaxUserNum                    = 0;
    desc.dwMaxServerNum                  = 120;
    desc.OnRecvFromUserTCP               = ReceivedMsgFromUser;
    desc.OnRecvFromServerTCP             = ReceivedMsgFromServer;
    desc.OnAcceptUser                    = OnAcceptUser;
    desc.OnAcceptServer                  = OnAcceptServer;
    desc.OnDisconnectUser                = OnDisconnectUser;
    desc.OnDisconnectServer              = OnDisconnectServer;
    desc.dwServerMaxTransferSize         = 65000;
    desc.dwUserMaxTransferSize           = 0;
    desc.dwServerBufferSizePerConnection = 128000;
    desc.dwUserBufferSizePerConnection   = 0;
    desc.dwMainMsgQueMaxBufferSize       = 2560000;
    desc.dwConnectNumAtSameTime          = 200;
    desc.dwCustomDefineEventNum          = 7;   // 4DyuchiNETAsio 内部截断到 3
    desc.pEvent                          = ev;
    desc.dwFlag                          = 0;

    // ---- 2. COM init(4DyuchiNETAsio 注册为 Apartment) ----
    HRESULT hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        log_line("CoInitializeEx failed hr=0x%08lx", hr);
        return false;
    }

    // ---- 3. CoCreateInstance(CLSID_4DyuchiNET) ----
    hr = ::CoCreateInstance(CLSID_4DyuchiNET, nullptr, CLSCTX_INPROC_SERVER,
                            IID_4DyuchiNET, reinterpret_cast<void**>(&g_pINet));
    if (FAILED(hr) || !g_pINet) {
        log_line("CoCreateInstance(CLSID_4DyuchiNET) failed hr=0x%08lx", hr);
        log_line("  -> 请先 regsvr32 4DyuchiNETAsio.dll(或切回 4DyuchiNETStub.dll)");
        return false;
    }
    log_line("CoCreateInstance OK g_pINet=%p", g_pINet);

    // ---- 4. CreateNetwork ----
    if (!g_pINet->CreateNetwork(&desc, 10, 10)) {
        log_line("CreateNetwork returned FALSE");
        return false;
    }
    log_line("CreateNetwork OK (declared 7 events, 4DyuchiNETAsio keeps first 3)");

    // ---- 5. 暂停所有周期定时器(在 ReadyToConnect 之前) ----
    g_pINet->PauseTimer(0);
    g_pINet->PauseTimer(1);
    g_pINet->PauseTimer(2);
    g_pINet->PauseTimer(3);  // 4DyuchiNETAsio 中 index 3 越界,no-op
    log_line("PauseTimer(0..3) done");

    // ---- 6. 启动桥接器接管 4 个外部事件 ----
    g_bridge = std::make_unique<MapServerTimerBridge>();
    auto handles = g_bridge->Register({
        ShowMapServerStatus,
        ReLoadGameServerDataByKeyInput,
        SaveNPCStatusByKeyInput,
        MakeMapDataFile
    });
    hKeyEvent[0] = handles[0];
    hKeyEvent[1] = handles[1];
    hKeyEvent[2] = handles[2];
    hKeyEvent[3] = handles[3];
    g_bridge->Start();
    log_line("MapServerTimerBridge started (handles: %p %p %p %p)",
             hKeyEvent[0], hKeyEvent[1], hKeyEvent[2], hKeyEvent[3]);

    // ---- 7. StartServerWithServerSide: 让 MockProxy 知道我们的端口 ----
    // Phase 3: 如果 --port 指定了端口,启动 server-side 监听,让 AgentServer 可连
    if (g_listen_port != 0) {
        char bind_ip[32];
        std::snprintf(bind_ip, sizeof(bind_ip), "0.0.0.0");
        BOOL ok = g_pINet->StartServerWithServerSide(bind_ip, g_listen_port);
        if (ok) {
            g_listen_ok.store(true);
            log_line("StartServerWithServerSide OK - bound 0.0.0.0:%u (AgentServer/user will connect to this port)",
                     static_cast<unsigned>(g_listen_port));
        } else {
            log_line("WARN: StartServerWithServerSide(0.0.0.0, %u) FAILED - port may be in use",
                     static_cast<unsigned>(g_listen_port));
        }
    } else {
        log_line("(skeleton: no StartServerWithServerSide - we connect to Proxy only)");
    }

    g_initialized = true;
    return true;
}

// =====================================================================
// Shutdown
// =====================================================================
void Shutdown() {
    log_line("Shutting down...");
    if (g_bridge) {
        g_bridge->Stop();
        g_bridge.reset();
    }
    if (g_pINet) {
        g_pINet->Release();
        g_pINet = nullptr;
    }
    ::CoUninitialize();
    g_initialized = false;
    log_line("Shutdown complete");
}

// =====================================================================
// 控制台热键线程(替代原 ReadConsoleInput)
//   F1 -> SetEvent(hKeyEvent[0])
//   F5 -> SetEvent(hKeyEvent[1])
//   F6 -> SetEvent(hKeyEvent[2])
//   F7 -> SetEvent(hKeyEvent[3])
//   ESC -> exit
// =====================================================================
static std::atomic<bool> g_run_flag{ true };

void console_keyboard_thread() {
    HANDLE hIn = ::GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == nullptr || hIn == INVALID_HANDLE_VALUE) {
        log_line("no console input handle, keyboard thread exiting");
        return;
    }
    INPUT_RECORD ir;
    DWORD nRead = 0;
    while (g_run_flag.load()) {
        if (!::ReadConsoleInput(hIn, &ir, 1, &nRead) || nRead == 0) {
            break;
        }
        if (ir.EventType != KEY_EVENT || !ir.Event.KeyEvent.bKeyDown) continue;
        switch (ir.Event.KeyEvent.wVirtualKeyCode) {
            case VK_ESCAPE:
                log_line("ESC pressed -> exit");
                g_run_flag.store(false);
                return;
            case VK_F1:
                log_line("F1 pressed -> SetEvent(hKeyEvent[0])");
                ::SetEvent(hKeyEvent[0]);
                break;
            case VK_F5:
                log_line("F5 pressed -> SetEvent(hKeyEvent[1])");
                ::SetEvent(hKeyEvent[1]);
                break;
            case VK_F6:
                log_line("F6 pressed -> SetEvent(hKeyEvent[2])");
                ::SetEvent(hKeyEvent[2]);
                break;
            case VK_F7:
                log_line("F7 pressed -> SetEvent(hKeyEvent[3])");
                ::SetEvent(hKeyEvent[3]);
                break;
            default:
                break;
        }
    }
}

// =====================================================================
// main
// =====================================================================
// 用法:
//   MapServerAsio.exe [map_id=1] [name="MapA"] [host=127.0.0.1] [port=3000]
//                     [--target N] [--no-business] [--no-echo]
//                     [--business-interval MS]
//                     [--port N]
//   --target N              注册成功后向 map_id=N 发送一次性 PTCL=56 map-to-map 业务
//   --no-business           关闭周期业务消息发送
//   --no-echo               收到 PTCL_AGENT_TO_MAP 后不回声
//   --business-interval MS  周期业务消息间隔(毫秒),0=关闭
//   --port N                Phase 3: 本 MapServer 对 AgentServer 的监听端口(0=不监听)
// =====================================================================
int main(int argc, char* argv[]) {
    g_map_id     = 1;
    g_map_name   = "MapA";
    g_proxy_host = "127.0.0.1";
    g_proxy_port = 3000;
    g_listen_port = 0;  // Phase 3 默认不监听,只连 Proxy

    // 1) 先扫命名参数(出现在任何位置都行)
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--target" && i + 1 < argc) {
            g_auto_target_map_id = static_cast<std::uint32_t>(std::atoi(argv[++i]));
        } else if (a == "--no-business") {
            g_business_interval_ms = 0;
        } else if (a == "--no-echo") {
            g_auto_echo_enabled.store(false);
        } else if (a == "--business-interval" && i + 1 < argc) {
            int v = std::atoi(argv[++i]);
            if (v < 0) v = 0;
            g_business_interval_ms = static_cast<std::uint32_t>(v);
        } else if (a == "--port" && i + 1 < argc) {
            int v = std::atoi(argv[++i]);
            if (v < 0 || v > 65535) v = 0;
            g_listen_port = static_cast<std::uint16_t>(v);
        } else if (a == "--help" || a == "-h") {
            std::printf("Usage: %s [map_id=1] [name=MapA] [host=127.0.0.1] [port=3000]\n"
                        "       [--target N] [--no-business] [--no-echo]\n"
                        "       [--business-interval MS] [--port N]\n", argv[0]);
            return 0;
        }
    }
    // 2) 再扫位置参数(map_id/name/host/port)
    int pos = 1;
    if (pos < argc && argv[pos][0] != '-') g_map_id   = static_cast<std::uint32_t>(std::atoi(argv[pos++]));
    if (pos < argc && argv[pos][0] != '-') g_map_name = argv[pos++];
    if (pos < argc && argv[pos][0] != '-') g_proxy_host = argv[pos++];
    if (pos < argc && argv[pos][0] != '-') g_proxy_port = static_cast<std::uint16_t>(std::atoi(argv[pos++]));

    WSADATA wsa;
    if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    log_line("starting: map_id=%u name=\"%s\" proxy=%s:%u listen_port=%u",
             g_map_id, g_map_name.c_str(), g_proxy_host.c_str(), g_proxy_port,
             static_cast<unsigned>(g_listen_port));
    log_line("options: business_interval=%ums (0=off) echo=%s target_map=%u%s",
             g_business_interval_ms,
             g_auto_echo_enabled.load() ? "on" : "off",
             g_auto_target_map_id,
             g_auto_target_map_id == 0 ? "" : " (one-shot after register)");

    if (!Init()) {
        log_line("Init() failed");
        ::WSACleanup();
        return 1;
    }

    // 启动控制台热键线程
    std::thread key_thread(console_keyboard_thread);

    // 业务消息发送线程:周期调用 send_business_message()
    std::thread business_thread([&]() {
        log_line("business-thread: started (interval=%ums)", g_business_interval_ms);
        auto last = std::chrono::steady_clock::now();
        while (g_run_flag.load()) {
            // 短睡眠,既能让其他线程跑,也能快速响应退出
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            if (g_business_interval_ms == 0) continue;
            auto now = std::chrono::steady_clock::now();
            if (now - last >= std::chrono::milliseconds(g_business_interval_ms)) {
                last = now;
                if (g_registered.load()) {
                    send_business_message();
                }
            }
        }
        log_line("business-thread: exit");
    });

    log_line("MapServerAsio skeleton running. Hotkeys: F1/F5/F6/F7, ESC=exit");

    // 主循环:等待 1s 后 Resume 周期定时器(index 0/1/2)
    // 让 AwaitingProxyServerConnect 自动尝试连接 Proxy
    // (这是原 MapServer main.cpp 的标准行为)
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    g_pINet->ResumeTimer(0);
    g_pINet->ResumeTimer(1);
    g_pINet->ResumeTimer(2);
    // index 3 是外部事件,无需 Resume
    log_line("ResumeTimer(0..2) done - AwaitingProxyServerConnect will try to connect Proxy");

    // 主线程:简单 sleep + 状态打印 + 桥接器自检 + 一次性 map-to-map 触发
    auto last_log = std::chrono::steady_clock::now();
    auto last_bridge_test = std::chrono::steady_clock::now();
    int bridge_test_round = 0;
    while (g_run_flag.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        auto now = std::chrono::steady_clock::now();
        if (now - last_log >= std::chrono::seconds(15)) {
            last_log = now;
            log_line("heartbeat: registered=%s proxy_conn=%u tick=%llu bridge_test_round=%d "
                     "business=%llu echo_sent=%llu echo_recv=%llu",
                     g_registered.load() ? "yes" : "no",
                     g_proxy_conn_idx.load(),
                     static_cast<unsigned long long>(g_tick_counter.load()),
                     bridge_test_round,
                     static_cast<unsigned long long>(g_business_counter.load()),
                     static_cast<unsigned long long>(g_echo_counter.load()),
                     static_cast<unsigned long long>(g_echo_received.load()));
        }
        // 桥接器自检:每 20s 触发一次 hKeyEvent[0] (F1),验证 MapServerTimerBridge 工作
        if (now - last_bridge_test >= std::chrono::seconds(20)) {
            last_bridge_test = now;
            bridge_test_round++;
            log_line("self-test: SetEvent(hKeyEvent[0]) -> expect ShowMapServerStatus callback");
            ::SetEvent(hKeyEvent[0]);
        }
        // 一次性 map-to-map 触发:注册成功后 -> 向 g_auto_target_map_id 发一次 PTCL=56
        if (!g_auto_target_sent.load()
            && g_auto_target_map_id != 0
            && g_registered.load()
            && g_proxy_conn_idx.load() != 0) {
            // 多等一拍,让 REGISTER_ACK 走完
            static auto first_seen = std::chrono::steady_clock::time_point{};
            auto seen = now;
            if (first_seen == std::chrono::steady_clock::time_point{}) {
                first_seen = seen;
            }
            if (now - first_seen >= std::chrono::milliseconds(800)) {
                log_line("one-shot: sending PTCL=56 map-to-map to target_map_id=%u",
                         g_auto_target_map_id);
                send_map_to_map_once(g_auto_target_map_id);
                g_auto_target_sent.store(true);
            }
        }
    }

    log_line("main loop exit");
    g_run_flag.store(false);
    if (key_thread.joinable()) key_thread.join();
    if (business_thread.joinable()) business_thread.join();
    Shutdown();
    ::WSACleanup();
    return 0;
}

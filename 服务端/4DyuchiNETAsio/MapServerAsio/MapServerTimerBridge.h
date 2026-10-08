// =====================================================================
// MapServerTimerBridge.h
//
// 4DyuchiNETAsio 限制: kMaxCustomEvents = 3 (见 4DyuchiNETAsio/include/timer_manager.h)
//                    实际可监听外部事件的槽位只有 1 个(index 1, object_handle)
//
// MapServer 需求: 7 个 CUSTOM_EVENT 槽位
//   - index 0, 1, 2: 周期定时器 (AwaitingProxyServerConnect / GameTimerProcess / CleanUpDeadConnections)
//   - index 3, 4, 5, 6: 外部 SetEvent 触发 (F1/F5/F6/F7 热键)
//
// 4DyuchiNETAsio 只能承载前 3 个周期定时器(0,1,2)。后 4 个外部事件由本类桥接:
//
//   本类维护:
//     - 4 个 Windows HANDLE(可由 MapServer main.cpp 调 SetEvent 触发)
//     - 1 个独立的 boost::asio::io_context + worker thread
//     - 4 个 windows::object_handle(每个 async_wait 一个)
//
//   当 MapServer 调 SetEvent(hKeyEvent[i]) 时:
//     1. object_handle 触发 async_wait 完成
//     2. 桥接器在自己的 io_context 线程调原始的 EVENTFUNC(DWORD) 回调
//     3. 重新 async_wait 同一 HANDLE(自动复用,无需外部重置)
//
// 用法:
//     auto bridge = std::make_unique<MapServerTimerBridge>();
//     auto handles = bridge->Register({ShowMapServerStatus,
//                                       ReLoadGameServerDataByKeyInput,
//                                       SaveNPCStatusByKeyInput,
//                                       MakeMapDataFile});
//     hKeyEvent[0..3] = handles[0..3];
//     bridge->Start();    // 启动后台 io_context
//     ... 等待 ...
//     bridge->Stop();     // 退出时
// =====================================================================

#pragma once

#ifndef _MAPSERVER_TIMER_BRIDGE_H_INCLUDED
#define _MAPSERVER_TIMER_BRIDGE_H_INCLUDED

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#include <array>
#include <atomic>
#include <memory>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/windows/object_handle.hpp>

// 来自 4DyuchiNETAsio/include/inetwork.h
typedef void (__stdcall *EVENTFUNC)(DWORD);

class MapServerTimerBridge
{
public:
    MapServerTimerBridge();
    ~MapServerTimerBridge();

    MapServerTimerBridge(const MapServerTimerBridge&) = delete;
    MapServerTimerBridge& operator=(const MapServerTimerBridge&) = delete;

    // 注册 4 个外部事件,返回 HANDLE 数组(0..3)
    // - funcs[i]: 当 HANDLE[i] 被 SetEvent 触发时调用的回调
    // - 返回的 HANDLE 由本类持有(Stop 时统一 CloseHandle),调用方不得 Close
    std::array<HANDLE, 4> Register(const std::array<EVENTFUNC, 4>& funcs);

    // 启动后台 io_context 线程(在 Register 之后调用)
    void Start();

    // 停止后台线程并清理
    void Stop();

    // 已注册?
    bool IsRegistered() const { return m_registered; }

private:
    void OnAsyncEvent(size_t idx, const boost::system::error_code& ec);
    void ScheduleReWait(size_t idx);

    boost::asio::io_context                      m_ioc;
    std::thread                                 m_thread;
    std::atomic<bool>                           m_stopping{false};
    bool                                        m_registered = false;

    std::array<HANDLE, 4>                       m_handles{ nullptr, nullptr, nullptr, nullptr };
    std::array<EVENTFUNC, 4>                    m_funcs{ nullptr, nullptr, nullptr, nullptr };
    std::array<std::unique_ptr<boost::asio::windows::object_handle>, 4> m_object_handles;
};

#endif // _MAPSERVER_TIMER_BRIDGE_H_INCLUDED

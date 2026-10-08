// =====================================================================
// timer_manager.h
// CTimerManager - 管理 DESC_NETWORK::pEvent 中的 3 个 CustomEvent 槽位
// =====================================================================
//
// 槽位语义 (AgentServer.cpp:91-100):
//   ev[0].dwPeriodicTime = 1000 -> steady_timer 周期触发,callback = AwaitingProxyServerConnect
//   ev[1].dwPeriodicTime = 0    -> windows::object_handle 外部 SetEvent 触发,callback = ShowAgentServerStatus
//   ev[2].dwPeriodicTime = 1000 -> steady_timer 周期触发,callback = TimerForUserTable
//
// PauseTimer(0) / ResumeTimer(0/2):
//   - enabled[0] = false/true;steady_timer[0].cancel() / rearm
//   - index 1 不需要 rearm(object_handle 持续监听同一 HANDLE)
//
// 线程安全:
//   - enabled[] 是 atomic<bool>
//   - timer 操作用 post(io_context, ...) 在 worker 线程内执行
// =====================================================================

#pragma once

#ifndef _TIMER_MANAGER_H_INCLUDED
#define _TIMER_MANAGER_H_INCLUDED

#include <array>
#include <atomic>
#include <cstdint>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/windows/object_handle.hpp>

#include "inetwork.h"

namespace asionet {

constexpr DWORD kMaxCustomEvents = 3;

class CTimerManager
{
public:
    CTimerManager(boost::asio::io_context& ioc, const CUSTOM_EVENT* events, DWORD event_count);
    ~CTimerManager();

    CTimerManager(const CTimerManager&) = delete;
    CTimerManager& operator=(const CTimerManager&) = delete;

    // 返回 index 对应的 HANDLE (AgentServer 调用 GetCustomEventHandle(1) 拿到外部事件)
    HANDLE GetHandle(DWORD index) const;

    // 暂停/恢复
    void Pause(DWORD index);
    void Resume(DWORD index);

    // 启动所有 timer (CreateNetwork 完成后调用一次)
    void StartAll();

    // 停止所有 timer (Release 时调用)
    void StopAll();

private:
    struct Slot
    {
        DWORD                          period_ms;     // 0 = 外部事件;非 0 = 周期 ms
        EVENTFUNC                      callback;      // 用户回调
        std::atomic<bool>              enabled;
        // 周期 timer
        std::unique_ptr<boost::asio::steady_timer>     steady;
        // 外部事件 timer (仅 period == 0 时使用)
        std::unique_ptr<boost::asio::windows::object_handle> object_handle;
        HANDLE                         event_handle;  // 持有 object_handle 内部的 HANDLE
    };

    void ArmPeriodic(DWORD index);
    void ArmExternal(DWORD index);
    void FireCallback(DWORD index);

    boost::asio::io_context& m_ioc;
    std::array<Slot, kMaxCustomEvents> m_slots;
};

} // namespace asionet

#endif // _TIMER_MANAGER_H_INCLUDED

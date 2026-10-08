// =====================================================================
// timer_manager.cpp
// CTimerManager 实现
// =====================================================================
//
// 关键设计:
//   - 槽位 0, 2: period > 0, 使用 steady_timer 周期触发
//   - 槽位 1: period == 0, 使用 windows::object_handle 外部 SetEvent 触发
//
// PauseTimer(i):
//   - enabled[i] = false
//   - steady_timer[i].cancel()  (handler 中收到 operation_aborted,直接 return)
//
// ResumeTimer(i):
//   - enabled[i] = true
//   - 若 period > 0:rearm
//   - period == 0:不需 rearm (object_handle 持续监听同一 HANDLE)
//
// GetCustomEventHandle(i):
//   - period == 0 (index 1):返回底层 HANDLE
//   - period > 0 (index 0, 2):返回 NULL
// =====================================================================

#include "../include/timer_manager.h"

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/windows/object_handle.hpp>

namespace asionet {

CTimerManager::CTimerManager(boost::asio::io_context& ioc, const CUSTOM_EVENT* events, DWORD event_count)
    : m_ioc(ioc)
{
    for (DWORD i = 0; i < kMaxCustomEvents; ++i) {
        m_slots[i].period_ms    = 0;
        m_slots[i].callback     = nullptr;
        m_slots[i].enabled      = false;
        m_slots[i].steady.reset();
        m_slots[i].object_handle.reset();
        m_slots[i].event_handle = nullptr;
    }
    if (events == nullptr || event_count == 0) {
        return;
    }
    const DWORD n = (event_count < kMaxCustomEvents) ? event_count : kMaxCustomEvents;
    for (DWORD i = 0; i < n; ++i) {
        m_slots[i].period_ms = events[i].dwPeriodicTime;
        m_slots[i].callback  = events[i].pEventFunc;
    }
}

CTimerManager::~CTimerManager()
{
    // 先 cancel 所有 pending 操作
    for (DWORD i = 0; i < kMaxCustomEvents; ++i) {
        m_slots[i].enabled.store(false, std::memory_order_release);
        if (m_slots[i].steady) {
            boost::system::error_code ec;
            m_slots[i].steady->cancel(ec);
        }
        if (m_slots[i].object_handle) {
            boost::system::error_code ec;
            m_slots[i].object_handle->close(ec);
        }
        // object_handle 不会 ::CloseHandle,我们必须显式调用
        if (m_slots[i].event_handle != nullptr) {
            ::CloseHandle(m_slots[i].event_handle);
            m_slots[i].event_handle = nullptr;
        }
    }
}

HANDLE CTimerManager::GetHandle(DWORD index) const
{
    if (index >= kMaxCustomEvents) return nullptr;
    // 仅 period == 0 的槽位返回 HANDLE (AgentServer 调 GetCustomEventHandle(1))
    if (m_slots[index].period_ms != 0) return nullptr;
    return m_slots[index].event_handle;
}

void CTimerManager::Pause(DWORD index)
{
    if (index >= kMaxCustomEvents) return;
    m_slots[index].enabled.store(false, std::memory_order_release);

    // cancel 必须在 io_context 线程内执行
    boost::asio::post(m_ioc, [this, index]() {
        if (m_slots[index].steady) {
            boost::system::error_code ec;
            m_slots[index].steady->cancel(ec);
        }
        // object_handle 不需 cancel(handler 收到 ec 后会 rearm,但 enabled=false 会导致其 return)
    });
}

void CTimerManager::Resume(DWORD index)
{
    if (index >= kMaxCustomEvents) return;
    m_slots[index].enabled.store(true, std::memory_order_release);

    boost::asio::post(m_ioc, [this, index]() {
        if (m_slots[index].period_ms > 0) {
            ArmPeriodic(index);
        }
        // period == 0:object_handle 持续监听,无需 rearm
    });
}

void CTimerManager::StartAll()
{
    for (DWORD i = 0; i < kMaxCustomEvents; ++i) {
        if (m_slots[i].period_ms > 0) {
            m_slots[i].enabled.store(true, std::memory_order_release);
            boost::asio::post(m_ioc, [this, i]() { ArmPeriodic(i); });
        } else if (m_slots[i].period_ms == 0) {
            // 外部事件 (period == 0):同步创建 HANDLE,无论 callback 是否设置
            // (AgentServer 可能在没有事件回调时,仍需要 GetCustomEventHandle 拿 HANDLE 自己 SetEvent)
            if (m_slots[i].event_handle == nullptr) {
                HANDLE h = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
                if (h != nullptr) {
                    m_slots[i].event_handle = h;
                    m_slots[i].object_handle = std::make_unique<boost::asio::windows::object_handle>(m_ioc, h);
                }
            }
            m_slots[i].enabled.store(true, std::memory_order_release);
            boost::asio::post(m_ioc, [this, i]() { ArmExternal(i); });
        }
    }
}

void CTimerManager::StopAll()
{
    for (DWORD i = 0; i < kMaxCustomEvents; ++i) {
        m_slots[i].enabled.store(false, std::memory_order_release);
        if (m_slots[i].steady) {
            boost::system::error_code ec;
            m_slots[i].steady->cancel(ec);
        }
        if (m_slots[i].object_handle) {
            boost::system::error_code ec;
            m_slots[i].object_handle->close(ec);
        }
    }
}

void CTimerManager::ArmPeriodic(DWORD index)
{
    auto& slot = m_slots[index];
    if (!slot.steady) {
        slot.steady = std::make_unique<boost::asio::steady_timer>(m_ioc);
    }
    slot.steady->expires_after(std::chrono::milliseconds(slot.period_ms));

    EVENTFUNC cb = slot.callback;
    DWORD period = slot.period_ms;
    // 让 lambda 通过 this 间接访问 enabled —— 生命周期与 CTimerManager 一致
    std::atomic<bool>* enabled_ptr = &slot.enabled;

    slot.steady->async_wait([this, index, cb, period, enabled_ptr](const boost::system::error_code& ec) {
        if (ec) return;  // cancel / aborted -> 不触发回调
        if (!enabled_ptr->load(std::memory_order_acquire)) return;
        if (cb) cb(index);
        if (enabled_ptr->load(std::memory_order_acquire)) {
            ArmPeriodic(index);  // 递归 re-arm
        }
    });
}

void CTimerManager::ArmExternal(DWORD index)
{
    auto& slot = m_slots[index];
    if (!slot.object_handle) {
        // 创建自动重置事件,初始 nonsignaled
        HANDLE h = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (h == nullptr) return;
        slot.event_handle = h;
        slot.object_handle = std::make_unique<boost::asio::windows::object_handle>(m_ioc, h);
    }
    EVENTFUNC cb = slot.callback;
    std::atomic<bool>* enabled_ptr = &slot.enabled;

    slot.object_handle->async_wait([this, index, cb, enabled_ptr](const boost::system::error_code& ec) {
        if (ec) return;
        if (!enabled_ptr->load(std::memory_order_acquire)) return;
        if (cb) cb(index);
        // 重新监听同一 HANDLE
        if (enabled_ptr->load(std::memory_order_acquire)) {
            ArmExternal(index);
        }
    });
}

void CTimerManager::FireCallback(DWORD index)
{
    if (index >= kMaxCustomEvents) return;
    if (m_slots[index].callback) {
        m_slots[index].callback(index);
    }
}

} // namespace asionet

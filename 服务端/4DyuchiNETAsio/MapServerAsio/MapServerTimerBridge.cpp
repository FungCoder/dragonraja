// =====================================================================
// MapServerTimerBridge.cpp - 见头文件注释
// =====================================================================

#include "MapServerTimerBridge.h"

#include <cstdio>
#include <exception>

namespace {
void log_bridge(const char* fmt, ...) {
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    char ts[32];
    std::snprintf(ts, sizeof(ts), "%02d:%02d:%02d.%03d",
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    std::printf("[%s] [Bridge] ", ts);

    va_list ap;
    va_start(ap, fmt);
    std::vprintf(fmt, ap);
    va_end(ap);
    std::printf("\n");
    std::fflush(stdout);
}
} // namespace

MapServerTimerBridge::MapServerTimerBridge() = default;

MapServerTimerBridge::~MapServerTimerBridge() {
    Stop();
}

std::array<HANDLE, 4>
MapServerTimerBridge::Register(const std::array<EVENTFUNC, 4>& funcs) {
    if (m_registered) {
        log_bridge("ERROR: already registered");
        return m_handles;
    }
    for (size_t i = 0; i < 4; ++i) {
        // 手动重置事件,跨 SetEvent 调用之间保持 signaled 不被消费
        // 注:windows::object_handle 要求 HANDLE 必须满足 "signal 后 wait 会立即完成"
        //     FALSE(auto-reset)即可,async_wait 完成后下一次 wait 又会重新监听
        m_handles[i] = ::CreateEventW(nullptr, /*bManualReset*/ FALSE,
                                                 /*bInitialState*/ FALSE, nullptr);
        if (m_handles[i] == nullptr) {
            log_bridge("CreateEventW failed for slot %zu (gle=%lu)", i, ::GetLastError());
            // 清理已创建
            for (size_t j = 0; j < i; ++j) {
                ::CloseHandle(m_handles[j]);
                m_handles[j] = nullptr;
            }
            return m_handles;
        }
        m_funcs[i] = funcs[i];
        log_bridge("registered slot=%zu handle=%p func=%p",
                   i, m_handles[i], reinterpret_cast<void*>(funcs[i]));
    }
    m_registered = true;
    return m_handles;
}

void MapServerTimerBridge::Start() {
    if (!m_registered) {
        log_bridge("ERROR: Start() called before Register()");
        return;
    }
    if (m_thread.joinable()) {
        return;  // 已启动
    }
    m_stopping.store(false, std::memory_order_release);

    m_thread = std::thread([this]() {
        try {
            // 在工作线程内创建 object_handle 并发起首次 async_wait
            for (size_t i = 0; i < 4; ++i) {
                m_object_handles[i] =
                    std::make_unique<boost::asio::windows::object_handle>(m_ioc, m_handles[i]);
                ScheduleReWait(i);
            }
            log_bridge("io_context running (4 object_handles attached)");
            m_ioc.run();
            log_bridge("io_context returned");
        } catch (const std::exception& e) {
            log_bridge("worker exception: %s", e.what());
        } catch (...) {
            log_bridge("worker unknown exception");
        }
    });
}

void MapServerTimerBridge::Stop() {
    if (!m_stopping.exchange(true)) {
        // 第一次调用 Stop
        if (m_ioc.stopped()) {
            // 未启动
        } else {
            m_ioc.stop();
        }
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
    for (auto& oh : m_object_handles) {
        oh.reset();
    }
    for (auto& h : m_handles) {
        if (h != nullptr) {
            ::CloseHandle(h);
            h = nullptr;
        }
    }
    m_registered = false;
}

void MapServerTimerBridge::OnAsyncEvent(size_t idx, const boost::system::error_code& ec) {
    if (m_stopping.load(std::memory_order_acquire)) {
        return;
    }
    if (ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;  // Stop 触发的取消
        }
        log_bridge("slot=%zu async_wait error: %s", idx, ec.message().c_str());
        return;
    }
    // 调用原始回调
    if (m_funcs[idx] != nullptr) {
        log_bridge("slot=%zu fired, calling user func=%p",
                   idx, reinterpret_cast<void*>(m_funcs[idx]));
        m_funcs[idx](static_cast<DWORD>(idx));
    } else {
        log_bridge("slot=%zu fired, but no user func registered", idx);
    }
    // 重新监听同一 HANDLE
    ScheduleReWait(idx);
}

void MapServerTimerBridge::ScheduleReWait(size_t idx) {
    if (m_stopping.load(std::memory_order_acquire)) {
        return;
    }
    if (idx >= 4 || !m_object_handles[idx]) {
        return;
    }
    auto self = this;
    m_object_handles[idx]->async_wait(
        [self, idx](const boost::system::error_code& ec) {
            self->OnAsyncEvent(idx, ec);
        });
}

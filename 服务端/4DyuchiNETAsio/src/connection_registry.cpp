// =====================================================================
// connection_registry.cpp
// CConnectionRegistry - 全局连接表实现
// =====================================================================

#include "../include/connection_registry.h"

#include <cassert>

namespace asionet {

DWORD CConnectionRegistry::Register(std::shared_ptr<CConnection> conn)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    DWORD idx = ++m_next_index;
    if (idx == 0) {
        // 极端情况:wrap-around 跳过 0
        idx = ++m_next_index;
    }
    m_connections.emplace(idx, std::move(conn));
    return idx;
}

std::shared_ptr<CConnection> CConnectionRegistry::Unregister(DWORD index)
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_connections.find(index);
    if (it == m_connections.end()) return nullptr;
    auto conn = std::move(it->second);
    m_connections.erase(it);
    return conn;
}

std::shared_ptr<CConnection> CConnectionRegistry::Get(DWORD index) const
{
    std::lock_guard<std::mutex> lk(m_mutex);
    auto it = m_connections.find(index);
    if (it == m_connections.end()) return nullptr;
    return it->second;
}

DWORD CConnectionRegistry::GetUserCount() const
{
    std::lock_guard<std::mutex> lk(m_mutex);
    DWORD n = 0;
    for (auto& kv : m_connections) {
        if (kv.second->role() == ConnectionRole::User) ++n;
    }
    return n;
}

DWORD CConnectionRegistry::GetServerCount() const
{
    std::lock_guard<std::mutex> lk(m_mutex);
    DWORD n = 0;
    for (auto& kv : m_connections) {
        if (kv.second->role() == ConnectionRole::Server) ++n;
    }
    return n;
}

DWORD CConnectionRegistry::GetTotalCount() const
{
    std::lock_guard<std::mutex> lk(m_mutex);
    return static_cast<DWORD>(m_connections.size());
}

} // namespace asionet

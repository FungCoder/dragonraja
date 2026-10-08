// =====================================================================
// connection_registry.h
// CConnectionRegistry - 全局连接表
// =====================================================================
//
// 职责:
//   - 分配 / 回收 connection index (1 起递增,0 = 无连接)
//   - 提供按 index 的 O(1) 查找
//   - 提供稳定索引范围(永不重用)以便上层缓存 GetUserAddress() 指针
//
// 线程安全:
//   - 内部 m_mutex 保护所有读写
//   - 调用方需自行 short-lived lock(本类提供 const + 非 const 操作)
// =====================================================================

#pragma once

#ifndef _CONNECTION_REGISTRY_H_INCLUDED
#define _CONNECTION_REGISTRY_H_INCLUDED

#include <memory>
#include <mutex>
#include <unordered_map>

#include "connection.h"

namespace asionet {

class CConnectionRegistry
{
public:
    CConnectionRegistry() = default;
    ~CConnectionRegistry() = default;

    // 分配 index,创建并注册 connection
    DWORD Register(std::shared_ptr<CConnection> conn);

    // 移除并返回 connection(若存在)。返回 nullptr 表示未找到。
    std::shared_ptr<CConnection> Unregister(DWORD index);

    // 按 index 查找(返回 shared_ptr,可能为 nullptr)
    std::shared_ptr<CConnection> Get(DWORD index) const;

    // 当前 user / server 连接数
    DWORD GetUserCount() const;
    DWORD GetServerCount() const;
    DWORD GetTotalCount() const;

private:
    mutable std::mutex                                          m_mutex;
    std::unordered_map<DWORD, std::shared_ptr<CConnection>>     m_connections;
    DWORD                                                       m_next_index = 0; // 永不重用,仅递增;首值必须为 0
};

} // namespace asionet

#endif // _CONNECTION_REGISTRY_H_INCLUDED

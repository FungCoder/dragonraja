// =====================================================================
// MockProxy.cpp - Dragon Raja 路由中心桩(支持 AgentServer + MapServer)
// =====================================================================
//
// 目的(v2,从 460 行升级到 ~810 行):
//   从"AgentServer 握手桩"升级为"路由中心":
//     - 接受 AgentServer 连接,走 4 步 PTCL 握手(原行为,完全保留)
//     - 接受 N 个 MapServer 连接,走简化注册握手(新)
//     - 维护 map_id → connection 的路由表(新)
//     - 实现基本的双向消息转发(可扩展为完整业务路由)
//
// 监听端口:127.0.0.1:3000(PROXY 范围,避开 8.9 端口分类陷阱)
//
// 协议:
//   帧格式: [4 bytes: DWORD, little-endian, length N][N bytes: payload]
//   payload 第一字节 = PTCL ID
//
//   连接类型识别(基于首个 PTCL 消息):
//     - PTCL=1 (NOTIFY_SERVER_UP) + port 字段 ∈ [7000, 7999] → AgentServer 角色
//     - PTCL=50 (MAP_REGISTER)                              → MapServer 角色
//
// 实现原则:
//   - 不使用 I4DyuchiNET(绕开 8 定时器问题,纯 socket 协议)
//   - MockMapClient 通过 PTCL=50 注册;MockProxy 维护 map_registry
//   - AgentServer 与 MapServer 之间的消息转发用 PTCL=52/53 标记
//   - 任何协议解析失败都安全断开
//
// 用法:MockProxy.exe [port]   (默认 3000,即 PROXY 范围)
// =====================================================================

#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <winsock2.h>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <boost/asio.hpp>

#pragma comment(lib, "ws2_32.lib")

namespace ba = boost::asio;
using tcp = ba::ip::tcp;

std::uint16_t kLocalDbPort = 4000;
std::uint16_t kLocalMapPort = 5000;
std::uint16_t kLocalAgentPort = 7001;
std::string gInternalIp = "127.0.0.1";

// =====================================================================
// PTCL 协议常量
// =====================================================================
namespace ptcl {
    // ---- 原有 PTCL(AgentServer 侧) ----
    constexpr std::uint8_t PTCL_NONE                                = 0;
    constexpr std::uint8_t PTCL_NOTIFY_SERVER_UP                    = 1;
    constexpr std::uint8_t PTCL_NOTIFY_YOU_ARE_CERTIFIED            = 2;
    constexpr std::uint8_t PTCL_REQUEST_SET_SERVER_LIST             = 10;
    constexpr std::uint8_t PTCL_REQUEST_TO_CONNECT_SERVER_LIST      = 11;
    constexpr std::uint8_t PTCL_ORDER_SET_SERVER_LIST               = 20;
    constexpr std::uint8_t PTCL_ORDER_CONNECT_TO_SERVERS            = 21;
    constexpr std::uint8_t PTCL_ORDER_SET_DB_DEMON                  = 22;
    constexpr std::uint8_t PTCL_SERVER_CONNECTING_RESULT            = 31;
    constexpr std::uint8_t PTCL_DB_DEMON_SETTING_RESULT             = 32;
    constexpr std::uint8_t PTCL_PROXY_TO_ACCESS                     = 102;
    constexpr std::uint8_t PTCL_AGENT_TO_COMMIT                     = 103;
    constexpr std::uint8_t PTCL_REPORT_SERVER_DATAS                 = 33;
    constexpr std::uint8_t PTCL_REPORT_SERVER_DESTROY               = 34;

    constexpr std::uint32_t RESULT_DB_DEMON_SETTING_SUCCESSED       = 0;
    constexpr std::uint32_t RESULT_DB_DEMON_IS_NOT_ACTIVATED        = 1;
    constexpr std::uint32_t RESULT_DB_DEMON_IS_NOT_CONNECTED        = 2;
    constexpr std::uint32_t RESULT_DB_DEMON_IS_NOT_IN_LIST          = 3;

    // ---- PTCL constants (must match MapServer/AgentServer protocol.h) ----
    constexpr std::uint8_t PTCL_MAP_REGISTER            = 50;  // MapServer → MockProxy (NOTIFY_SERVER_UP with port 5000-6999)
    constexpr std::uint8_t PTCL_MAP_REGISTER_ACK        = 51;  // MockProxy → MapServer
    constexpr std::uint8_t PTCL_AGENT_TO_MAP            = 50;  // AgentServer → MapServer (same PTCL as MAP_REGISTER, distinguished by direction)
    constexpr std::uint8_t PTCL_MAP_TO_AGENT            = 70;  // MapServer → AgentServer (must match protocol.h PTCL_MAP_TO_AGENT=70)
    constexpr std::uint8_t PTCL_MAP_LIST_QUERY          = 54;  // AgentServer → MockProxy(查询地图)
    constexpr std::uint8_t PTCL_MAP_LIST_RESPONSE       = 55;  // MockProxy → AgentServer
    constexpr std::uint8_t PTCL_MAP_TO_MAP              = 81;  // MapServer → MapServer (must match protocol.h PTCL_MAP_TO_MAP=81)
}

// =====================================================================
// 帧编/解码(4 字节 LE 长度前缀)
// =====================================================================
namespace frame {
    constexpr std::size_t kHeaderSize = sizeof(std::uint32_t);
    constexpr std::size_t kMaxPayload = 65000;  // server role 上限

    inline void encode_header(char* buf, std::uint32_t length) {
        std::memcpy(buf, &length, sizeof(length));
    }

    inline std::uint32_t decode_header(const char* buf) {
        std::uint32_t length = 0;
        std::memcpy(&length, buf, sizeof(length));
        return length;
    }
}

// =====================================================================
// 日志
// =====================================================================
namespace {

std::mutex g_log_mu;

template <typename... Args>
void log_line(const char* fmt, Args... args) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    char ts[32];
    std::snprintf(ts, sizeof(ts), "%02d:%02d:%02d.%03d",
                  st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    std::lock_guard<std::mutex> lk(g_log_mu);
    std::printf("[%s] ", ts);
    std::printf(fmt, args...);
    std::printf("\n");
    std::fflush(stdout);
}

}  // namespace

// =====================================================================
// 前向声明
// =====================================================================
class Server;
class Session;

// =====================================================================
// MapInfo - 注册的 MapServer 信息
// =====================================================================
struct MapInfo {
    std::uint32_t       map_id = 0;
    std::string         map_name;
    std::uint16_t       listen_port = 0;    // MapServer 对 AgentServer 的监听端口(Phase 3 多端口)
    std::shared_ptr<Session> session;       // 引用计数保持连接活跃
    std::string         peer;
    std::chrono::steady_clock::time_point registered_at;
};

// =====================================================================
// Server - 监听 + 接受连接 + 路由注册
// 成员函数体中调用了 Session 的方法,因此函数体定义在 Session 类之后
// =====================================================================
class Server {
public:
    Server(ba::io_context& ioc, std::uint16_t port)
        : ioc_(ioc)
        , acceptor_(ioc, tcp::endpoint(ba::ip::make_address(gInternalIp), port))
        , port_(port)
        , next_id_(1)
    {
    }

    void start();

    // ---- 路由注册(由 Session 在状态切换时调用) ----
    void register_agent(std::shared_ptr<Session> s);
    void unregister_agent();
    bool has_agent();
    void register_map(std::uint32_t map_id, const std::string& map_name,
                      std::uint16_t listen_port, std::shared_ptr<Session> s);
    void unregister_map(std::uint32_t map_id);
    void set_dbdemon_port(std::uint16_t port);
    std::uint16_t dbdemon_port() const { return dbdemon_port_; }

    // 转发
    void route_to_agent(const std::vector<char>& payload);
    void route_to_map(std::uint32_t map_id, const std::vector<char>& payload);
    void route_map_to_map(std::uint32_t from_map_id, std::uint32_t target_map_id,
                          const std::vector<char>& business_payload);

    // ---- 供 Session 访问的 getter(无 Session 依赖,可保持内联) ----
    std::mutex& registry_mutex() { return registry_mu_; }
    const std::unordered_map<std::uint32_t, MapInfo>& maps() { return maps_; }
    size_t map_count() { return maps_.size(); }

    void log_map_list();

private:
    void do_accept();

    ba::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::uint16_t port_;
    std::atomic<std::uint32_t> next_id_;

    std::mutex registry_mu_;
    std::shared_ptr<Session> agent_;
    std::unordered_map<std::uint32_t, MapInfo> maps_;
    std::uint16_t dbdemon_port_ = 0;
};

// =====================================================================
// Session - 单连接状态机(支持 AgentServer + MapServer)
// Server 已在前面定义,本类内联方法体可直接调用 server_->xxx()
// =====================================================================
class Session : public std::enable_shared_from_this<Session> {
public:
    enum class ConnType : std::uint8_t {
        Unknown,
        AgentServer,
        MapServer,
        DBDemon
    };

    enum class State : int {
        kAwaitingFirstMessage,
        kAwaitingServerUp,
        kAwaitingSetServerListRequest,
        kAwaitingConnectListRequest,
        kAwaitingConnectingResult,
        kNegotiationDone,
        kMapAwaitingRegister,
        kMapRegistered,
        kClosed
    };

    Session(tcp::socket socket, std::uint32_t conn_id, Server* server)
        : socket_(std::move(socket))
        , conn_id_(conn_id)
        , server_(server)
        , state_(State::kAwaitingFirstMessage)
        , conn_type_(ConnType::Unknown)
    {
        auto ep = socket_.remote_endpoint();
        peer_ = ep.address().to_string() + ":" + std::to_string(ep.port());
    }

    void start() {
        log_line("[conn#%u] ACCEPT from %s (role=Unknown)", conn_id_, peer_.c_str());
        do_read_header();
    }

    ConnType conn_type() const { return conn_type_; }
    std::uint32_t map_id() const { return map_id_; }
    const std::string& peer() const { return peer_; }
    Server* server() const { return server_; }

    // 由 Server 调用,推送一个完整帧(已编码的 [length][payload])
    void enqueue_frame(const std::vector<char>& payload) {
        send_frame(payload);
    }

private:
    // ----- async read -----
    void do_read_header();
    void do_read_payload(std::uint32_t expected_len);

    // ----- write -----
    void send_frame(const std::vector<char>& payload);
    void do_write();

    // ----- 协议分发 -----
    void handle_frame();
    void handle_agent_frame(std::uint8_t bID, const char* body, std::size_t body_len);
    void handle_dbdemon_frame(std::uint8_t bID, const char* body, std::size_t body_len);
    std::vector<std::uint16_t> peer_ports(bool registered_only);
    void send_order_set_server_list();
    void send_order_connect_to_servers();
    // Phase 4:推送已注册 map 的 IP+port(让 AgentServer 主动 connect)
    void send_order_set_db_demon(std::uint16_t wDBDemonPort);
    void send_map_list_response();
    void on_map_register(const char* body, std::size_t body_len);
    void send_map_register_ack(std::uint32_t map_id);
    void handle_map_frame(std::uint8_t bID, const char* body, std::size_t body_len);

    // ----- 断开 -----
    void on_disconnect(const boost::system::error_code& ec);
    void close();

    // ----- members -----
    tcp::socket socket_;
    std::uint32_t conn_id_;
    std::string peer_;
    Server* server_;
    State state_;
    ConnType conn_type_;
    std::uint32_t map_id_ = 0;
    std::string map_name_;
    std::uint16_t dbdemon_port_ = 0;

    char header_buf_[frame::kHeaderSize]{};
    std::vector<char> payload_;

    std::mutex write_mu_;
    bool write_in_flight_ = false;
    std::deque<std::vector<char>> outbox_;
};

// =====================================================================
// Server 方法实现(在 Session 类之后定义,可使用 Session 完整类型)
// =====================================================================

void Server::start() {
    log_line("MockProxy v2 listening on %s:%u (AgentServer + MapServer)", gInternalIp.c_str(), port_);
    do_accept();
}

void Server::do_accept() {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket sock) {
            if (!ec) {
                auto id = next_id_.fetch_add(1, std::memory_order_relaxed);
                std::make_shared<Session>(std::move(sock), id, this)->start();
            } else {
                log_line("ACCEPT error: %s", ec.message().c_str());
            }
            do_accept();
        });
}

void Server::register_agent(std::shared_ptr<Session> s) {
    std::lock_guard<std::mutex> lk(registry_mu_);
    if (agent_ && agent_.get() != s.get()) {
        log_line("[SERVER] WARN: replacing existing AgentServer (old conn=%s, new conn=%s)",
                 agent_->peer().c_str(), s->peer().c_str());
    }
    agent_ = s;
    log_line("[SERVER] AgentServer registered: %s", s->peer().c_str());
}

void Server::unregister_agent() {
    std::lock_guard<std::mutex> lk(registry_mu_);
    if (agent_) {
        log_line("[SERVER] AgentServer unregistered: %s", agent_->peer().c_str());
        agent_.reset();
    }
}

bool Server::has_agent() {
    std::lock_guard<std::mutex> lk(registry_mu_);
    return static_cast<bool>(agent_);
}

void Server::set_dbdemon_port(std::uint16_t port) {
    std::lock_guard<std::mutex> lk(registry_mu_);
    dbdemon_port_ = port;
    log_line("[SERVER] DBDemon port set: %u", port);
}

void Server::register_map(std::uint32_t map_id, const std::string& map_name,
                          std::uint16_t listen_port, std::shared_ptr<Session> s) {
    std::lock_guard<std::mutex> lk(registry_mu_);
    MapInfo info;
    info.map_id = map_id;
    info.map_name = map_name;
    info.listen_port = listen_port;
    info.session = s;
    info.peer = s->peer();
    info.registered_at = std::chrono::steady_clock::now();
    maps_[map_id] = info;
    log_line("[SERVER] MapServer registered: map_id=%u name=\"%s\" port=%u peer=%s (total maps=%zu)",
             map_id, map_name.c_str(), listen_port, s->peer().c_str(), maps_.size());
}

void Server::unregister_map(std::uint32_t map_id) {
    std::lock_guard<std::mutex> lk(registry_mu_);
    auto it = maps_.find(map_id);
    if (it != maps_.end()) {
        log_line("[SERVER] MapServer unregistered: map_id=%u name=\"%s\" peer=%s",
                 map_id, it->second.map_name.c_str(), it->second.peer.c_str());
        maps_.erase(it);
    }
}

void Server::route_to_agent(const std::vector<char>& payload) {
    std::shared_ptr<Session> a;
    {
        std::lock_guard<std::mutex> lk(registry_mu_);
        a = agent_;
    }
    if (!a) {
        log_line("[SERVER] route_to_agent: no AgentServer connected, drop %zu bytes", payload.size());
        return;
    }
    a->enqueue_frame(payload);
}

void Server::route_to_map(std::uint32_t map_id, const std::vector<char>& payload) {
    std::shared_ptr<Session> m;
    {
        std::lock_guard<std::mutex> lk(registry_mu_);
        auto it = maps_.find(map_id);
        if (it != maps_.end()) {
            m = it->second.session;
        }
    }
    if (!m) {
        log_line("[SERVER] route_to_map(%u): map not found, drop %zu bytes", map_id, payload.size());
        return;
    }
    m->enqueue_frame(payload);
}

void Server::route_map_to_map(std::uint32_t from_map_id, std::uint32_t target_map_id,
                              const std::vector<char>& business_payload) {
    std::shared_ptr<Session> m;
    {
        std::lock_guard<std::mutex> lk(registry_mu_);
        auto it = maps_.find(target_map_id);
        if (it != maps_.end()) {
            m = it->second.session;
        }
    }
    if (!m) {
        log_line("[SERVER] route_map_to_map: target map_id=%u not found, drop %zu bytes",
                 target_map_id, business_payload.size());
        return;
    }
    // 构造 PTCL_AGENT_TO_MAP (52) + [4 bytes from_map_id] + [业务 payload]
    // 这样目标 map_session 收到时,可以识别是来自哪个 map_session 的业务消息
    std::vector<char> route;
    route.push_back(ptcl::PTCL_AGENT_TO_MAP);
    std::vector<char> from_id_bytes(4);
    std::memcpy(from_id_bytes.data(), &from_map_id, 4);
    route.insert(route.end(), from_id_bytes.begin(), from_id_bytes.end());
    route.insert(route.end(), business_payload.begin(), business_payload.end());
    log_line("[SERVER] MAP %u → MAP %u (%zu bytes payload)",
             from_map_id, target_map_id, business_payload.size());
    m->enqueue_frame(route);
}

void Server::log_map_list() {
    std::lock_guard<std::mutex> lk(registry_mu_);
    log_line("[SERVER] === Map Registry (total=%zu) ===", maps_.size());
    if (agent_) {
        log_line("[SERVER]   AgentServer: %s", agent_->peer().c_str());
    } else {
        log_line("[SERVER]   AgentServer: (none)");
    }
    for (auto& kv : maps_) {
        auto& info = kv.second;
        auto age = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - info.registered_at).count();
        log_line("[SERVER]   Map %u: name=\"%s\" port=%u peer=%s age=%llds",
                 info.map_id, info.map_name.c_str(), info.listen_port,
                 info.peer.c_str(), static_cast<long long>(age));
    }
    log_line("[SERVER] === End Map Registry ===");
}

// =====================================================================
// Session 方法实现
// =====================================================================

void Session::do_read_header() {
    auto self = shared_from_this();
    ba::async_read(socket_,
                   ba::buffer(header_buf_, frame::kHeaderSize),
                   [this, self](boost::system::error_code ec, std::size_t /*bytes*/) {
                       if (ec) {
                           on_disconnect(ec);
                           return;
                       }
                       const std::uint32_t len = frame::decode_header(header_buf_);
                       if (len == 0 || len > frame::kMaxPayload) {
                           log_line("[conn#%u] ERROR: bad frame length %u (max=%zu)",
                                    conn_id_, len, frame::kMaxPayload);
                           close();
                           return;
                       }
                       payload_.assign(len, 0);
                       do_read_payload(len);
                   });
}

void Session::do_read_payload(std::uint32_t expected_len) {
    auto self = shared_from_this();
    ba::async_read(socket_,
                   ba::buffer(payload_.data(), expected_len),
                   [this, self, expected_len](boost::system::error_code ec, std::size_t n) {
                       if (ec) {
                           on_disconnect(ec);
                           return;
                       }
                       payload_.resize(n);
                       handle_frame();
                       do_read_header();
                   });
}

void Session::send_frame(const std::vector<char>& payload) {
    bool need_to_start;
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        need_to_start = !write_in_flight_;
        if (need_to_start) {
            write_in_flight_ = true;
        }
        outbox_.push_back(payload);
    }
    if (need_to_start) {
        do_write();
    }
}

void Session::do_write() {
    auto buf = std::make_shared<std::vector<char>>();
    {
        std::lock_guard<std::mutex> lk(write_mu_);
        if (outbox_.empty()) {
            write_in_flight_ = false;
            return;
        }
        auto& front = outbox_.front();
        buf->resize(frame::kHeaderSize + front.size());
        frame::encode_header(buf->data(),
                             static_cast<std::uint32_t>(front.size()));
        if (!front.empty()) {
            std::memcpy(buf->data() + frame::kHeaderSize,
                        front.data(), front.size());
        }
        outbox_.pop_front();
    }
    auto self = shared_from_this();
    ba::async_write(socket_, ba::buffer(*buf),
        [this, self, buf](boost::system::error_code ec, std::size_t /*bytes*/) {
            if (ec) {
                on_disconnect(ec);
                return;
            }
            do_write();
        });
}

void Session::handle_frame() {
    if (payload_.empty()) {
        log_line("[conn#%u] WARN: empty frame, ignored", conn_id_);
        return;
    }
    const std::uint8_t bID = static_cast<std::uint8_t>(payload_[0]);
    const char* body = payload_.data() + 1;
    const std::size_t body_len = payload_.size() - 1;

    // 首次消息:判断连接类型
    if (state_ == State::kAwaitingFirstMessage) {
        if (bID == ptcl::PTCL_NOTIFY_SERVER_UP && body_len >= 2) {
            std::uint16_t port = 0;
            std::memcpy(&port, body, 2);
            if (port >= 7000 && port <= 7999) {
                conn_type_ = ConnType::AgentServer;
                state_ = State::kAwaitingSetServerListRequest;
                log_line("[conn#%u]   IDENTIFIED as AgentServer (port=%u)", conn_id_, port);
                server_->register_agent(shared_from_this());
                return;
            } else if (port >= 4000 && port <= 4999) {
                conn_type_ = ConnType::DBDemon;
                state_ = State::kAwaitingSetServerListRequest;
                dbdemon_port_ = port;
                server_->set_dbdemon_port(port);
                log_line("[conn#%u]   IDENTIFIED as DBDemon (port=%u)", conn_id_, port);
                return;
            } else if (port >= 5000 && port <= 6999) {
                // The original MapServer uses the same negotiation sequence as
                // AgentServer after notifying its port. It must receive
                // ORDER_SET_SERVER_LIST before it can bind its listen socket.
                conn_type_ = ConnType::MapServer;
                state_ = State::kAwaitingSetServerListRequest;
                // A classic MapServer identifies itself by port only. Use that
                // port as its stable routing ID so multiple map instances do
                // not overwrite one another in the registry.
                map_id_ = port;
                map_name_ = "LegacyMap-" + std::to_string(port);
                log_line("[conn#%u]   IDENTIFIED as MapServer (port=%u)", conn_id_, port);
                server_->register_map(map_id_, map_name_, port, shared_from_this());
                return;
            } else {
                log_line("[conn#%u]   UNKNOWN port=%u (not Agent 7000-7999), closing",
                         conn_id_, port);
                close();
                return;
            }
        } else if (bID == ptcl::PTCL_MAP_REGISTER && body_len >= 4) {
            conn_type_ = ConnType::MapServer;
            state_ = State::kMapRegistered;
            on_map_register(body, body_len);
            return;
        } else {
            log_line("[conn#%u]   UNKNOWN first message PTCL=%u, closing", conn_id_, bID);
            close();
            return;
        }
    }

    switch (conn_type_) {
    case ConnType::AgentServer:
        handle_agent_frame(bID, body, body_len);
        break;
    case ConnType::MapServer:
        handle_map_frame(bID, body, body_len);
        break;
    case ConnType::DBDemon:
        handle_dbdemon_frame(bID, body, body_len);
        break;
    default:
        log_line("[conn#%u]   unknown conn_type, ignored", conn_id_);
        break;
    }
}

void Session::handle_agent_frame(std::uint8_t bID, const char* body, std::size_t body_len) {
    log_line("[conn#%u] [AGENT] RECV PTCL=%u len=%zu (state=%d)",
             conn_id_, bID, body_len, static_cast<int>(state_));

    switch (state_) {
    case State::kAwaitingSetServerListRequest:
        if (bID == ptcl::PTCL_REQUEST_SET_SERVER_LIST) {
            log_line("[conn#%u]   Agent REQUEST_SET_SERVER_LIST -> reply ORDER_SET_SERVER_LIST (with maps)",
                     conn_id_);
            send_order_set_server_list();
            state_ = State::kAwaitingConnectListRequest;
        } else if (bID == ptcl::PTCL_MAP_LIST_QUERY) {
            log_line("[conn#%u]   Agent MAP_LIST_QUERY -> reply MAP_LIST_RESPONSE", conn_id_);
            send_map_list_response();
        } else {
            log_line("[conn#%u]   (ignored, waiting for PTCL_REQUEST_SET_SERVER_LIST=10)", conn_id_);
        }
        break;

    case State::kAwaitingConnectListRequest:
        if (bID == ptcl::PTCL_REQUEST_TO_CONNECT_SERVER_LIST) {
            log_line("[conn#%u]   Agent REQUEST_TO_CONNECT_SERVER_LIST -> reply ORDER_CONNECT_TO_SERVERS (with maps)",
                     conn_id_);
            send_order_connect_to_servers();
            state_ = State::kAwaitingConnectingResult;
        } else {
            log_line("[conn#%u]   (ignored, waiting for PTCL_REQUEST_TO_CONNECT_SERVER_LIST=11)", conn_id_);
        }
        break;

    case State::kAwaitingConnectingResult:
        if (bID == ptcl::PTCL_SERVER_CONNECTING_RESULT) {
            std::uint16_t dp = server_->dbdemon_port();
            log_line("[conn#%u]   Agent SERVER_CONNECTING_RESULT -> reply ORDER_SET_DB_DEMON (port=%u)",
                     conn_id_, dp);
            send_order_set_db_demon(dp);
            state_ = State::kNegotiationDone;
            log_line("[conn#%u] NEGOTIATION DONE. Agent should now bind user-side socket on 7000.",
                     conn_id_);
        } else {
            log_line("[conn#%u]   (ignored, waiting for PTCL_SERVER_CONNECTING_RESULT=31)", conn_id_);
        }
        break;

    case State::kNegotiationDone:
        if (bID == ptcl::PTCL_DB_DEMON_SETTING_RESULT && body_len >= 4) {
            std::uint32_t result = 0;
            std::memcpy(&result, body, 4);
            log_line("[conn#%u]   Agent DB_DEMON_SETTING_RESULT=%u (handshake complete)",
                     conn_id_, result);
        } else if (bID == ptcl::PTCL_MAP_LIST_QUERY) {
            send_map_list_response();
        } else if (bID == ptcl::PTCL_PROXY_TO_ACCESS) {
            // Original ProxyServer acknowledges a successful login before
            // AgentServer forwards CMD_ACCEPT_LOGIN to the client.
            if (body_len < 9) {
                log_line("[conn#%u] malformed login access frame", conn_id_);
                close();
                return;
            }
            std::uint16_t packet_size = 0;
            std::memcpy(&packet_size, body + 6, sizeof(packet_size));
            if (body_len != static_cast<std::size_t>(4 + 5 + packet_size)) {
                log_line("[conn#%u] invalid login access packet size", conn_id_);
                close();
                return;
            }
            std::vector<char> response = payload_;
            response[0] = ptcl::PTCL_AGENT_TO_COMMIT;
            send_frame(response);
            log_line("[conn#%u] login access committed", conn_id_);
        } else {
            log_line("[conn#%u]   post-negotiation PTCL=%u, ignored (no echo)", conn_id_, bID);
        }
        break;

    default:
        log_line("[conn#%u]   agent unhandled state=%d", conn_id_, static_cast<int>(state_));
        break;
    }
}

void Session::handle_dbdemon_frame(std::uint8_t bID, const char* body, std::size_t body_len) {
    log_line("[conn#%u] [DBDEMON] RECV PTCL=%u len=%zu (state=%d)",
             conn_id_, bID, body_len, static_cast<int>(state_));

    // DBDemon uses the same protocol as AgentServer for handshake
    // After NOTIFY_SERVER_UP, it expects the proxy to respond
    // For now, just acknowledge and keep the connection alive
    switch (state_) {
    case State::kAwaitingSetServerListRequest:
        if (bID == ptcl::PTCL_REQUEST_SET_SERVER_LIST) {
            log_line("[conn#%u]   DBDemon REQUEST_SET_SERVER_LIST -> reply empty list",
                     conn_id_);
            send_order_set_server_list();
            state_ = State::kAwaitingConnectListRequest;
        } else {
            log_line("[conn#%u]   DBDemon ignored PTCL=%u", conn_id_, bID);
        }
        break;

    case State::kAwaitingConnectListRequest:
        if (bID == ptcl::PTCL_REQUEST_TO_CONNECT_SERVER_LIST) {
            log_line("[conn#%u]   DBDemon REQUEST_TO_CONNECT -> reply empty list",
                     conn_id_);
            send_order_connect_to_servers();
            state_ = State::kAwaitingConnectingResult;
        } else {
            log_line("[conn#%u]   DBDemon ignored PTCL=%u", conn_id_, bID);
        }
        break;

    case State::kAwaitingConnectingResult:
        if (bID == ptcl::PTCL_SERVER_CONNECTING_RESULT) {
            log_line("[conn#%u]   DBDemon CONNECTING_RESULT -> reply ORDER_SET_DB_DEMON (port=%u)",
                     conn_id_, dbdemon_port_);
            send_order_set_db_demon(server_->dbdemon_port());
            state_ = State::kNegotiationDone;
            log_line("[conn#%u] DBDemon NEGOTIATION DONE.", conn_id_);
        } else {
            log_line("[conn#%u]   DBDemon ignored PTCL=%u", conn_id_, bID);
        }
        break;

    case State::kNegotiationDone:
        log_line("[conn#%u]   DBDemon post-negotiation PTCL=%u", conn_id_, bID);
        break;

    default:
        log_line("[conn#%u]   DBDemon unhandled state=%d", conn_id_, static_cast<int>(state_));
        break;
    }
}

std::vector<std::uint16_t> Session::peer_ports(bool registered_only) {
    std::vector<std::uint16_t> ports;
    const auto add_port = [&](std::uint16_t port) {
        if (port != 0 && !(conn_type_ == ConnType::MapServer && port == map_id_) &&
            std::find(ports.begin(), ports.end(), port) == ports.end()) {
            ports.push_back(port);
        }
    };

    if (!registered_only) {
        // 2026-09-30: 必须与 registered_only 分支一致地使用「注册表中的实际端口」。
        // 此前静态下发 kLocalDbPort(4000), 而 DBDemon 因 Windows 保留端口段改绑 4100 后,
        // AgentServer/MapServer 的服务器表里只有 4000, 收到 ORDER_CONNECT(4100) 时
        // GetServerData() 返回 NULL, 触发 2001 年原版空指针 bug 直接崩溃。
        if (conn_type_ != ConnType::DBDemon) {
            const auto db_port = server_->dbdemon_port();
            add_port(db_port ? db_port : kLocalDbPort);
        }
        if (conn_type_ != ConnType::AgentServer) add_port(kLocalAgentPort);
        add_port(kLocalMapPort);
    } else {
        if (conn_type_ != ConnType::DBDemon) add_port(server_->dbdemon_port());
        if (conn_type_ == ConnType::MapServer && server_->has_agent())
            add_port(kLocalAgentPort);
    }

    {
        std::lock_guard<std::mutex> lock(server_->registry_mutex());
        for (const auto& entry : server_->maps())
            add_port(entry.second.listen_port);
    }
    return ports;
}

void Session::send_order_set_server_list() {
    const auto ports = peer_ports(false);
    constexpr std::size_t kCinfoSize = 21 + 2 + 1;
    std::vector<char> packet(7 + ports.size() * kCinfoSize, 0);
    packet[0] = ptcl::PTCL_ORDER_SET_SERVER_LIST;
    const std::uint32_t set_number = 1;
    const auto count = static_cast<std::uint16_t>(ports.size());
    std::memcpy(packet.data() + 1, &set_number, sizeof(set_number));
    std::memcpy(packet.data() + 5, &count, sizeof(count));

    for (std::size_t index = 0; index < ports.size(); ++index) {
        char* info = packet.data() + 7 + index * kCinfoSize;
        std::memcpy(info, gInternalIp.c_str(), gInternalIp.size());
        std::memcpy(info + 21, &ports[index], sizeof(ports[index]));
    }
    log_line("[conn#%u] -> ORDER_SET_SERVER_LIST count=%u", conn_id_, count);
    for (auto port : ports)
        log_line("[conn#%u]   peer port=%u", conn_id_, port);
    send_frame(packet);
}

void Session::send_order_connect_to_servers() {
    const auto ports = peer_ports(true);
    std::vector<char> packet(3 + ports.size() * sizeof(std::uint16_t), 0);
    packet[0] = ptcl::PTCL_ORDER_CONNECT_TO_SERVERS;
    const auto count = static_cast<std::uint16_t>(ports.size());
    std::memcpy(packet.data() + 1, &count, sizeof(count));
    for (std::size_t index = 0; index < ports.size(); ++index)
        std::memcpy(packet.data() + 3 + index * sizeof(std::uint16_t),
                    &ports[index], sizeof(ports[index]));
    log_line("[conn#%u] -> ORDER_CONNECT_TO_SERVERS count=%u", conn_id_, count);
    for (auto port : ports)
        log_line("[conn#%u]   connect port=%u", conn_id_, port);
    send_frame(packet);
}

void Session::send_order_set_db_demon(std::uint16_t wDBDemonPort) {
    // PTCL_ORDER_SET_DB_DEMON (1) + wDBDemonPort(2)
    std::vector<char> p(3, 0);
    p[0] = ptcl::PTCL_ORDER_SET_DB_DEMON;
    std::memcpy(&p[1], &wDBDemonPort, 2);
    send_frame(p);
}

void Session::send_map_list_response() {
    // PTCL_MAP_LIST_RESPONSE (1) + dwCount(4)
    //   + N × (dwMapId(4) + cbNameLen(2) + name(N) + listen_port(2))   (Phase 3 加入 port)
    std::vector<MapInfo> maps_copy;
    {
        std::lock_guard<std::mutex> lk(server_->registry_mutex());
        maps_copy.reserve(server_->map_count());
        for (auto& kv : server_->maps()) {
            maps_copy.push_back(kv.second);
        }
    }

    std::vector<char> p;
    p.push_back(ptcl::PTCL_MAP_LIST_RESPONSE);
    std::uint32_t count = static_cast<std::uint32_t>(maps_copy.size());
    std::vector<char> count_bytes(4);
    std::memcpy(count_bytes.data(), &count, 4);
    p.insert(p.end(), count_bytes.begin(), count_bytes.end());

    for (auto& m : maps_copy) {
        std::uint32_t id = m.map_id;
        std::vector<char> id_bytes(4);
        std::memcpy(id_bytes.data(), &id, 4);
        p.insert(p.end(), id_bytes.begin(), id_bytes.end());

        std::uint16_t name_len = static_cast<std::uint16_t>(m.map_name.size());
        std::vector<char> len_bytes(2);
        std::memcpy(len_bytes.data(), &name_len, 2);
        p.insert(p.end(), len_bytes.begin(), len_bytes.end());

        p.insert(p.end(), m.map_name.begin(), m.map_name.end());

        // Phase 3:每条 map 后跟 listen_port(LE, 2 字节)
        std::uint16_t port = m.listen_port;
        std::vector<char> port_bytes(2);
        std::memcpy(port_bytes.data(), &port, 2);
        p.insert(p.end(), port_bytes.begin(), port_bytes.end());
    }

    log_line("[conn#%u]   -> MAP_LIST_RESPONSE count=%u", conn_id_, count);
    send_frame(p);
}

void Session::on_map_register(const char* body, std::size_t body_len) {
    // body: [4 bytes map_id] + [2 bytes cbNameLen] + [N bytes name]
    //      + [2 bytes listen_port LE]   (Phase 3 新增;若缺省则视为 0)
    if (body_len < 6) {
        log_line("[conn#%u]   MAP_REGISTER too short (%zu bytes), closing",
                 conn_id_, body_len);
        close();
        return;
    }

    std::uint32_t map_id = 0;
    std::memcpy(&map_id, body, 4);
    std::uint16_t name_len = 0;
    std::memcpy(&name_len, body + 4, 2);

    if (body_len < static_cast<std::size_t>(6 + name_len)) {
        log_line("[conn#%u]   MAP_REGISTER name truncated, closing", conn_id_);
        close();
        return;
    }

    std::string map_name(body + 6, name_len);
    map_id_ = map_id;
    map_name_ = map_name;

    // Phase 3:解析 listen_port(可选,向后兼容旧 map(无 port 字段))
    std::uint16_t listen_port = 0;
    const std::size_t port_offset = 6 + static_cast<std::size_t>(name_len);
    if (body_len >= port_offset + 2) {
        std::memcpy(&listen_port, body + port_offset, 2);
    } else {
        log_line("[conn#%u]   MAP_REGISTER no port field (legacy), using 0", conn_id_);
    }

    log_line("[conn#%u]   IDENTIFIED as MapServer map_id=%u name=\"%s\" port=%u (len=%u)",
             conn_id_, map_id, map_name.c_str(), listen_port, name_len);

    server_->register_map(map_id, map_name, listen_port, shared_from_this());
    send_map_register_ack(map_id);
}

void Session::send_map_register_ack(std::uint32_t map_id) {
    // PTCL_MAP_REGISTER_ACK (1) + map_id(4) + cbNameLen(2) + name
    std::vector<char> p;
    p.push_back(ptcl::PTCL_MAP_REGISTER_ACK);
    std::vector<char> id_bytes(4);
    std::memcpy(id_bytes.data(), &map_id, 4);
    p.insert(p.end(), id_bytes.begin(), id_bytes.end());

    std::uint16_t name_len = static_cast<std::uint16_t>(map_name_.size());
    std::vector<char> len_bytes(2);
    std::memcpy(len_bytes.data(), &name_len, 2);
    p.insert(p.end(), len_bytes.begin(), len_bytes.end());
    p.insert(p.end(), map_name_.begin(), map_name_.end());

    log_line("[conn#%u]   -> MAP_REGISTER_ACK map_id=%u name=\"%s\"",
             conn_id_, map_id, map_name_.c_str());
    send_frame(p);
}

void Session::handle_map_frame(std::uint8_t bID, const char* body, std::size_t body_len) {
    log_line("[conn#%u] [MAP %u] RECV PTCL=%u len=%zu (state=%d)",
             conn_id_, map_id_, bID, body_len, static_cast<int>(state_));

    if (state_ != State::kMapRegistered) {
        // Legacy MapServer uses the standard server negotiation sequence.
        handle_dbdemon_frame(bID, body, body_len);
        return;
    }

    if (bID != ptcl::PTCL_MAP_REGISTER) {
        // ---- 特殊:PTCL_MAP_TO_MAP(56) - map-to-map 直接路由 ----
        if (bID == ptcl::PTCL_MAP_TO_MAP && body_len >= 4) {
            std::uint32_t target_map_id = 0;
            std::memcpy(&target_map_id, body, 4);
            // 业务 payload = body + 4 .. end
            std::vector<char> business_payload(body + 4, body + body_len);
            log_line("[conn#%u]   MAP %u → MAP %u (PTCL=56, business_payload=%zu bytes)",
                     conn_id_, map_id_, target_map_id, business_payload.size());
            server_->route_map_to_map(map_id_, target_map_id, business_payload);
            return;
        }

        // ---- PTCL=36/37: MapServer status reports, just ACK ----
        if (bID == 36 || bID == 37) {
            log_line("[conn#%u]   MAP %u status report PTCL=%u (ACK'd)", conn_id_, map_id_, bID);
            return;
        }

        // ---- 常规:转给 AgentServer ----
        // 构造 PTCL_MAP_TO_AGENT (53) + [4 bytes map_id] + [原始 payload]
        std::vector<char> route;
        route.push_back(ptcl::PTCL_MAP_TO_AGENT);
        std::vector<char> id_bytes(4);
        std::memcpy(id_bytes.data(), &map_id_, 4);
        route.insert(route.end(), id_bytes.begin(), id_bytes.end());
        // 原始 payload(包括 PTCL 字节)跟在 map_id 后面
        route.insert(route.end(), payload_.begin(), payload_.end());

        std::string hex_preview = "";
        for (size_t i = 0; i < std::min<size_t>(body_len, 16); ++i) {
            char b[4];
            std::snprintf(b, sizeof(b), "%02X ", static_cast<unsigned char>(body[i]));
            hex_preview += b;
        }
        log_line("[conn#%u]   MAP %u → AGENT (PTCL=%u, %zu bytes, preview=%s%s)",
                 conn_id_, map_id_, bID, body_len, hex_preview.c_str(),
                 body_len > 16 ? "..." : "");

        server_->route_to_agent(route);
    }
}

void Session::on_disconnect(const boost::system::error_code& ec) {
    if (ec == ba::error::eof ||
        ec == ba::error::connection_reset ||
        ec == ba::error::operation_aborted) {
        log_line("[conn#%u] DISCONNECT (%s) (final state=%d, role=%s)",
                 conn_id_, ec.message().c_str(), static_cast<int>(state_),
                 conn_type_ == ConnType::AgentServer ? "Agent" :
                 conn_type_ == ConnType::MapServer ? "Map" : "Unknown");
    } else if (ec) {
        log_line("[conn#%u] DISCONNECT (ec=%d: %s)",
                 conn_id_, ec.value(), ec.message().c_str());
    } else {
        log_line("[conn#%u] DISCONNECT (clean)", conn_id_);
    }
    if (server_) {
        if (conn_type_ == ConnType::AgentServer) {
            server_->unregister_agent();
        } else if (conn_type_ == ConnType::MapServer) {
            server_->unregister_map(map_id_);
        }
    }
    close();
}

void Session::close() {
    state_ = State::kClosed;
    boost::system::error_code ec;
    socket_.close(ec);
}

// =====================================================================
// main
// =====================================================================
int main(int argc, char* argv[]) {
    std::uint16_t port = 3000;   // 默认 3000,即 PROXY 范围(3000-3999),避开端口分类陷阱
    if (argc >= 2) {
        int p = std::atoi(argv[1]);
        if (p > 0 && p < 65536) port = static_cast<std::uint16_t>(p);
    }

    if (argc >= 3) {
        boost::system::error_code error;
        const auto address = ba::ip::make_address_v4(argv[2], error);
        if (error || address.is_unspecified()) return 2;
        gInternalIp = address.to_string();
    }
    auto read_port = [&](int index, int minimum, int maximum, std::uint16_t& value) {
        if (argc <= index) return true;
        char* end = nullptr;
        const long parsed = std::strtol(argv[index], &end, 10);
        if (!end || *end || parsed < minimum || parsed > maximum) return false;
        value = static_cast<std::uint16_t>(parsed);
        return true;
    };
    if (!read_port(1, 3000, 3999, port) || !read_port(3, 7000, 7999, kLocalAgentPort) ||
        !read_port(4, 4000, 4999, kLocalDbPort) || !read_port(5, 5000, 6999, kLocalMapPort)) return 2;
    if (argc >= 7 && std::strcmp(argv[6], "--validate-config") == 0) {
        std::printf("Configuration valid: proxy=%u agent=%u database=%u map=%u ip=%s\n", port, kLocalAgentPort, kLocalDbPort, kLocalMapPort, gInternalIp.c_str());
        return 0;
    }

    // 启动 winsock(Boost.Asio 在 Windows 上需要 WSAStartup)
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    try {
        ba::io_context ioc(1);  // 1 个 io 线程足够(状态机不重)
        Server srv(ioc, port);
        srv.start();
        log_line("MockProxy v2 started (with MapServer support). Press Ctrl+C to stop.");

        // 定期打印路由表(每 30 秒)
        std::thread([&srv]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(30));
                srv.log_map_list();
            }
        }).detach();

        ioc.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Fatal: %s\n", e.what());
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}

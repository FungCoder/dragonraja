// =====================================================================
// MockMapClient.cpp - 模拟 MapServer 连接到 MockProxy
// =====================================================================
//
// 目的:
//   - 模拟一个 MapServer,执行简化注册握手(PTCL=50 -> ACK=51)
//   - 注册后定时发送"业务消息"(任意 PTCL,带可辨识的 payload)
//   - MockProxy 收到后会:
//       1. 维护到 maps_ 注册表
//       2. 把消息封装为 PTCL_MAP_TO_AGENT (53) + [map_id] + payload 转发给 AgentServer
//
// 协议(与 MockProxy 一致):
//   帧格式: [4 bytes: DWORD, little-endian, length N][N bytes: payload]
//   payload 第一字节 = PTCL ID
//
//   PTCL_MAP_REGISTER (50):
//     [4 bytes map_id] + [2 bytes cbNameLen] + [N bytes map_name]
//
//   PTCL_MAP_REGISTER_ACK (51):
//     [4 bytes map_id] + [2 bytes cbNameLen] + [N bytes map_name]
//
//   业务消息(任意):MockMapClient 周期性发送 PTCL=100 消息,
//                  携带 8 字节 [send_counter_be] 让对端可观测
//
// 用法:MockMapClient.exe [map_id] [map_name] [proxy_port] [interval_ms]
//      默认: map_id=1  map_name="TestMap"  proxy_port=3000  interval_ms=2000
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
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#pragma comment(lib, "ws2_32.lib")

namespace ba = boost::asio;
using tcp = ba::ip::tcp;

// =====================================================================
// PTCL 协议常量(与 MockProxy 对齐)
// =====================================================================
namespace ptcl {
    constexpr std::uint8_t PTCL_MAP_REGISTER            = 50;
    constexpr std::uint8_t PTCL_MAP_REGISTER_ACK        = 51;
    constexpr std::uint8_t PTCL_AGENT_TO_MAP            = 52;
    constexpr std::uint8_t PTCL_MAP_TO_AGENT            = 53;
    constexpr std::uint8_t PTCL_MAP_LIST_QUERY          = 54;
    constexpr std::uint8_t PTCL_MAP_LIST_RESPONSE       = 55;
    constexpr std::uint8_t PTCL_DEMO_BUSINESS_MSG       = 100;  // MockMapClient 自定义业务 PTCL
}

// =====================================================================
// 帧编/解码
// =====================================================================
namespace frame {
    constexpr std::size_t kHeaderSize = sizeof(std::uint32_t);

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
    std::printf("[%s] [MockMap] ", ts);
    std::printf(fmt, args...);
    std::printf("\n");
    std::fflush(stdout);
}
}  // namespace

// =====================================================================
// MockMapClient
// =====================================================================
class MockMapClient : public std::enable_shared_from_this<MockMapClient> {
public:
    MockMapClient(ba::io_context& ioc,
                  std::uint32_t map_id,
                  std::string map_name,
                  std::uint16_t proxy_port,
                  std::uint32_t interval_ms)
        : ioc_(ioc)
        , resolver_(ioc)
        , socket_(ioc)
        , map_id_(map_id)
        , map_name_(std::move(map_name))
        , proxy_port_(proxy_port)
        , interval_ms_(interval_ms)
        , registered_(false)
        , send_counter_(0)
        , stop_flag_(false)
    {
    }

    void start() {
        log_line("starting: map_id=%u name=\"%s\" proxy=127.0.0.1:%u interval=%ums",
                 map_id_, map_name_.c_str(), proxy_port_, interval_ms_);
        do_connect();
        // 业务发送线程(独立线程,简单清晰)
        sender_thread_ = std::thread([this]() { sender_loop(); });
    }

    void stop() {
        stop_flag_ = true;
        if (sender_thread_.joinable()) {
            sender_thread_.join();
        }
        boost::system::error_code ec;
        socket_.close(ec);
    }

private:
    void do_connect() {
        auto self = shared_from_this();
        resolver_.async_resolve("127.0.0.1", std::to_string(proxy_port_),
            [this, self](boost::system::error_code ec, tcp::resolver::results_type results) {
                if (ec) {
                    log_line("RESOLVE error: %s", ec.message().c_str());
                    return;
                }
                ba::async_connect(socket_, results,
                    [this, self](boost::system::error_code ec, const tcp::endpoint&) {
                        if (ec) {
                            log_line("CONNECT error: %s", ec.message().c_str());
                            return;
                        }
                        log_line("CONNECTED to MockProxy");
                        do_register();
                    });
            });
    }

    void do_register() {
        // PTCL_MAP_REGISTER (1) + map_id (4) + cbNameLen (2) + name
        std::vector<char> p;
        p.push_back(ptcl::PTCL_MAP_REGISTER);

        std::vector<char> id_bytes(4);
        std::memcpy(id_bytes.data(), &map_id_, 4);
        p.insert(p.end(), id_bytes.begin(), id_bytes.end());

        std::uint16_t name_len = static_cast<std::uint16_t>(map_name_.size());
        std::vector<char> len_bytes(2);
        std::memcpy(len_bytes.data(), &name_len, 2);
        p.insert(p.end(), len_bytes.begin(), len_bytes.end());

        p.insert(p.end(), map_name_.begin(), map_name_.end());

        log_line("-> MAP_REGISTER map_id=%u name=\"%s\"", map_id_, map_name_.c_str());
        send_frame(p);
        do_read_header();
    }

    void do_read_header() {
        auto self = shared_from_this();
        ba::async_read(socket_,
                       ba::buffer(header_buf_, frame::kHeaderSize),
                       [this, self](boost::system::error_code ec, std::size_t /*bytes*/) {
                           if (ec) {
                               log_line("READ header error: %s", ec.message().c_str());
                               return;
                           }
                           const std::uint32_t len = frame::decode_header(header_buf_);
                           if (len == 0 || len > 65000) {
                               log_line("ERROR: bad frame length %u", len);
                               return;
                           }
                           payload_.assign(len, 0);
                           do_read_payload(len);
                       });
    }

    void do_read_payload(std::uint32_t expected_len) {
        auto self = shared_from_this();
        ba::async_read(socket_,
                       ba::buffer(payload_.data(), expected_len),
                       [this, self, expected_len](boost::system::error_code ec, std::size_t n) {
                           if (ec) {
                               log_line("READ payload error: %s", ec.message().c_str());
                               return;
                           }
                           payload_.resize(n);
                           handle_frame();
                           do_read_header();
                       });
    }

    void handle_frame() {
        if (payload_.empty()) {
            log_line("WARN: empty frame");
            return;
        }
        const std::uint8_t bID = static_cast<std::uint8_t>(payload_[0]);
        const char* body = payload_.data() + 1;
        const std::size_t body_len = payload_.size() - 1;

        if (bID == ptcl::PTCL_MAP_REGISTER_ACK && body_len >= 6) {
            std::uint32_t echo_id = 0;
            std::memcpy(&echo_id, body, 4);
            std::uint16_t name_len = 0;
            std::memcpy(&name_len, body + 4, 2);
            std::string echo_name(body + 6, std::min<std::size_t>(name_len, body_len - 6));
            log_line("<- MAP_REGISTER_ACK map_id=%u name=\"%s\" (REGISTERED!)",
                     echo_id, echo_name.c_str());
            registered_ = true;
        } else if (bID == ptcl::PTCL_AGENT_TO_MAP && body_len >= 4) {
            // 来自 AgentServer 的路由消息: [4 bytes map_id] + [业务 payload]
            std::uint32_t target_map_id = 0;
            std::memcpy(&target_map_id, body, 4);
            log_line("<- AGENT_TO_MAP target_map_id=%u, %zu bytes (routed from agent)",
                     target_map_id, body_len - 4);
        } else {
            log_line("<- RECV PTCL=%u len=%zu (other)", bID, body_len);
        }
    }

    // ----- write ---------------------------------------------------------

    void send_frame(const std::vector<char>& payload) {
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

    void do_write() {
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
                    log_line("WRITE error: %s", ec.message().c_str());
                    return;
                }
                do_write();
            });
    }

    // ----- 业务发送线程(每 N ms 发一条) -----------------------------------

    void sender_loop() {
        log_line("sender loop started (interval=%ums)", interval_ms_);
        while (!stop_flag_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
            if (stop_flag_) break;
            if (!registered_) {
                log_line("(not registered yet, skip business send)");
                continue;
            }
            send_business_message();
        }
        log_line("sender loop stopped");
    }

    void send_business_message() {
        // PTCL_DEMO_BUSINESS_MSG (1) + send_counter_be (8) + map_id_be (4) + 16 bytes ascii
        std::vector<char> p;
        p.push_back(ptcl::PTCL_DEMO_BUSINESS_MSG);

        // send_counter (8 bytes, BE,便于对端直接读)
        std::uint64_t counter = send_counter_.fetch_add(1, std::memory_order_relaxed) + 1;
        for (int i = 7; i >= 0; --i) {
            p.push_back(static_cast<char>((counter >> (i * 8)) & 0xFF));
        }

        // map_id (4 bytes, BE)
        for (int i = 3; i >= 0; --i) {
            p.push_back(static_cast<char>((map_id_ >> (i * 8)) & 0xFF));
        }

        // 16 字节 ASCII 标记 "MockMapClient!"
        const char tag[16] = "MockMapClient!";
        p.insert(p.end(), tag, tag + 16);

        log_line("-> BUSINESS msg #%llu (PTCL=%u, %zu bytes)",
                 static_cast<unsigned long long>(counter),
                 ptcl::PTCL_DEMO_BUSINESS_MSG, p.size());
        send_frame(p);
    }

    // ----- members --------------------------------------------------------

    ba::io_context& ioc_;
    tcp::resolver resolver_;
    tcp::socket socket_;

    std::uint32_t map_id_;
    std::string map_name_;
    std::uint16_t proxy_port_;
    std::uint32_t interval_ms_;

    std::atomic<bool> registered_;
    std::atomic<std::uint64_t> send_counter_;
    std::atomic<bool> stop_flag_;
    std::thread sender_thread_;

    char header_buf_[frame::kHeaderSize]{};
    std::vector<char> payload_;

    std::mutex write_mu_;
    bool write_in_flight_ = false;
    std::deque<std::vector<char>> outbox_;
};

// =====================================================================
// main
// =====================================================================
int main(int argc, char* argv[]) {
    std::uint32_t map_id = 1;
    std::string map_name = "TestMap";
    std::uint16_t proxy_port = 3000;
    std::uint32_t interval_ms = 2000;

    if (argc >= 2) map_id = static_cast<std::uint32_t>(std::atoi(argv[1]));
    if (argc >= 3) map_name = argv[2];
    if (argc >= 4) proxy_port = static_cast<std::uint16_t>(std::atoi(argv[3]));
    if (argc >= 5) interval_ms = static_cast<std::uint32_t>(std::atoi(argv[4]));

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    try {
        ba::io_context ioc(1);
        auto client = std::make_shared<MockMapClient>(ioc, map_id, map_name, proxy_port, interval_ms);
        client->start();

        // 等 Ctrl+C
        std::printf("MockMapClient running. Press Ctrl+C to stop.\n");
        ioc.run();

        // 正常退出路径不会到这里(ioc.run() 只在 stop 时返回)
        client->stop();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Fatal: %s\n", e.what());
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}

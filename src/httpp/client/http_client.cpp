#include "httpp/http_client.hpp"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <future>
#include <limits>
#include <loopp/event_loop.hpp>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "httpp/client/http_client_connection.hpp"
#include "httpp/core/socket.hpp"
#include "httpp/http_message.hpp"

namespace httpp {

namespace {

/*
 * Simple URL parser to extract hostname and port.
 * Default port is 80 for HTTP and 443 for HTTPS.
 */
std::pair<std::string, uint16_t> parse_url(const std::string& url) {
    uint16_t port = 80;

    size_t protocol_end = url.find("://");
    size_t start = (protocol_end != std::string::npos) ? protocol_end + 3 : 0;

    size_t path_start = url.find('/', start);
    std::string host_part = (path_start != std::string::npos)
                                ? url.substr(start, path_start - start)
                                : url.substr(start);

    size_t port_pos = host_part.find(':');
    if (port_pos != std::string::npos) {
        std::string hostname = host_part.substr(0, port_pos);
        if (hostname.empty()) {
            throw std::runtime_error("Empty hostname in URL");
        }
        try {
            int port_int = std::stoi(host_part.substr(port_pos + 1));
            if (port_int <= 0 || port_int > std::numeric_limits<uint16_t>::max()) {
                throw std::runtime_error("Port out of range");
            }
            port = static_cast<uint16_t>(port_int);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Invalid port number");
        } catch (const std::out_of_range&) {
            throw std::runtime_error("Port number out of range");
        }
        return {hostname, port};
    }

    if (url.starts_with("https://")) {
        port = 443;
    }
    return {host_part, port};
}

}  // namespace

HttpClient::HttpClient() : event_loop_(loopp::EventLoop::create()) {
    is_running_ = true;
    event_loop_thread_ = std::thread([this]() { event_loop_->start(); });
}

HttpClient::~HttpClient() noexcept {
    stop();
}

std::future<HttpResponse> HttpClient::send_async(const HttpRequest& request) {
    auto promise = std::make_shared<std::promise<HttpResponse>>();
    auto future = promise->get_future();

    try {
        auto [hostname, port] = parse_url(request.url());

        auto socket = Socket::create_tcp_socket();
        auto addr = Socket::resolve_address(hostname, port);
        if (!socket.connect(addr)) {
            throw std::runtime_error("Failed to connect");
        }

        auto connection = HttpClientConnection::create(std::move(socket), event_loop_);
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_.insert(connection);
        }

        connection->on_disconnect([this](const auto& connection) {
            remove_connection(std::static_pointer_cast<HttpClientConnection>(connection));
        });

        if (!connection->start()) {
            throw std::runtime_error("Failed to start connection");
        }

        if (!connection->send_request(request, [promise](const HttpResponse& response) {
                promise->set_value(response);
            })) {
            throw std::runtime_error("Failed to send request");
        }
    } catch (...) {
        promise->set_exception(std::current_exception());
    }

    return future;
}

HttpResponse HttpClient::send_sync(const HttpRequest& request) {
    return send_async(request).get();
}

bool HttpClient::stop() {
    if (!is_running_.exchange(false)) {
        return true;
    }

    bool success = true;

    {
        std::lock_guard<std::mutex> lock(connections_mutex_);

        for (auto& connection : connections_) {
            success &= connection->close();
        }
    }

    success &= event_loop_->stop();

    if (event_loop_thread_.joinable()) {
        event_loop_thread_.join();
    }

    return success;
}

void HttpClient::remove_connection(const std::shared_ptr<HttpClientConnection>& connection) {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(connection);
}

}  // namespace httpp

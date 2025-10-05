#include "httpp/http_server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstdint>
#include <loopp/event_loop.hpp>
#include <memory>
#include <system_error>
#include <utility>

#include "httpp/core/socket.hpp"
#include "httpp/server/http_server_connection.hpp"

namespace httpp {

HttpServer::~HttpServer() noexcept {
    stop();
}

void HttpServer::on_request(const HttpServerConnection::RequestHandler& handler) {
    request_handler_ = handler;
}

void HttpServer::start(int port) {
    if (!socket_.set_reuse_addr()) {
        throw std::system_error(errno, std::system_category(), "Failed to set SO_REUSEADDR");
    }

    if (!socket_.set_nonblocking()) {
        throw std::system_error(errno, std::system_category(), "Failed to set non-blocking");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (!socket_.bind(addr)) {
        throw std::system_error(errno, std::system_category(), "Failed to bind socket");
    }

    if (!socket_.listen()) {
        throw std::system_error(errno, std::system_category(), "Failed to listen on socket");
    }

    auto callback = [this](int, loopp::EventType) {
        sockaddr_in addr{};
        int cfd = socket_.accept(addr);
        if (cfd == -1) {
            return;  // Either no pending connections or an error occurred
        }

        auto connection = add_connection(Socket(cfd));
    };

    // Register main socket for reading
    if (!event_loop_->add_fd(socket_.fd(), loopp::EventType::READ, callback)) {
        throw std::system_error(errno, std::system_category(), "Failed to add server socket to event loop");
    }

    is_running_ = true;
    event_loop_->start();
    is_running_ = false;
}

bool HttpServer::stop() {
    if (!is_running_.exchange(false)) {
        return true;
    }

    bool success = true;

    for (auto& connection : connections_) {
        success &= connection->close();
    }

    success &= event_loop_->stop();
    return success;
}

std::shared_ptr<HttpServerConnection> HttpServer::add_connection(Socket&& socket) {
    auto connection = HttpServerConnection::create(std::move(socket), event_loop_, request_handler_);
    connections_.insert(connection);

    connection->on_disconnect([this](const auto& connection) {
        remove_connection(std::static_pointer_cast<HttpServerConnection>(connection));
    });

    if (!connection->start()) {
        throw std::system_error(errno, std::system_category(), "Failed to start connection");
    }
    return connection;
}

void HttpServer::remove_connection(const std::shared_ptr<HttpServerConnection>& connection) {
    connections_.erase(connection);
}

}  // namespace httpp

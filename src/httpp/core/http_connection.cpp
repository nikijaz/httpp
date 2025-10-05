#include "httpp/core/http_connection.hpp"

#include <sys/types.h>

#include <cerrno>
#include <cstddef>
#include <loopp/event_loop.hpp>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

#include "httpp/core/socket.hpp"

namespace httpp {

HttpConnection::HttpConnection(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop) : socket_(std::move(socket)), loop_(std::move(loop)) {
    if (!socket_.set_nonblocking()) {
        throw std::system_error(errno, std::system_category(), "Failed to set connection socket to non-blocking");
    }
}

bool HttpConnection::start() {
    return loop_->add_fd(socket_.fd(), loopp::EventType::READ,
                         [this](int, loopp::EventType) { handle_read(); });
}

bool HttpConnection::write(const std::string& data) {
    write_buffer_.append(data);
    return loop_->add_fd(socket_.fd(), loopp::EventType::WRITE,
                         [this](int, loopp::EventType) { handle_write(); });
}

bool HttpConnection::disconnect() {
    bool success = close();
    handle_disconnect();
    return success;
}

void HttpConnection::on_disconnect(const DisconnectCallback& callback) {
    disconnect_callback_ = callback;
}

bool HttpConnection::close() {
    if (!loop_->remove_fd(socket_.fd(), loopp::EventType::READ)) {
        return false;
    }
    if (!loop_->remove_fd(socket_.fd(), loopp::EventType::WRITE)) {
        return false;
    }
    return true;
}

void HttpConnection::handle_disconnect() {
    if (disconnect_callback_) {
        disconnect_callback_(shared_from_this());
    }
}

void HttpConnection::handle_read() {
    static const size_t BUFFER_SIZE = 1024;

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = socket_.read(buffer, sizeof(buffer));

    // We received data
    if (bytes_read > 0) {
        feed_parser(std::string(buffer, static_cast<size_t>(bytes_read)));
    }

    // No data, server disconnected
    if (bytes_read == 0) {
        disconnect();
    }

    // An error occurred
    if (bytes_read < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        disconnect();
    }
}

void HttpConnection::handle_write() {
    ssize_t bytes_written = socket_.write(write_buffer_.data(), write_buffer_.size());

    // We sent data
    if (bytes_written > 0) {
        write_buffer_.erase(0, static_cast<size_t>(bytes_written));
    }

    // An error occurred
    if (bytes_written < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        disconnect();
    }

    // We finished writing all data
    if (write_buffer_.empty()) {
        if (!loop_->remove_fd(socket_.fd(), loopp::EventType::WRITE)) {
            // If remove fails, disconnect
            disconnect();
        }
    }
}

}  // namespace httpp

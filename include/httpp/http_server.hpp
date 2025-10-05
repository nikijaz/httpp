#pragma once

#include <atomic>
#include <loopp/event_loop.hpp>
#include <memory>
#include <unordered_set>

#include "httpp/core/socket.hpp"
#include "httpp/server/http_server_connection.hpp"

namespace httpp {

/*
 * HTTP server.
 * Supports handling multiple client connections and processing requests asynchronously.
 */
class HttpServer {
   public:
    HttpServer() = default;
    ~HttpServer() noexcept;

    [[nodiscard]] bool is_running() const noexcept { return is_running_; }

    /*
     * Set request handler (required before start).
     */
    void on_request(const HttpServerConnection::RequestHandler& handler);

    /*
     * Start the HTTP server on the specified port.
     * Blocks the calling thread.
     * Throws `std::system_error` on failure.
     */
    void start(int port);

    /*
     * Stop the HTTP server if it is running.
     * Closes all client connections and stops the event loop.
     * Returns true on success, false on failure (check errno for details).
     * If the server is not running, it's a no-op and returns true.
     */
    bool stop();

   private:
    std::atomic<bool> is_running_{false};
    Socket socket_{Socket::create_tcp_socket()};
    std::shared_ptr<loopp::EventLoop> event_loop_{loopp::EventLoop::create()};
    HttpServerConnection::RequestHandler request_handler_;
    /* A set of active connections connected to the server. */
    std::unordered_set<std::shared_ptr<HttpServerConnection>> connections_;

    /*
     * Adds a new connection to the server, keeping it alive.
     * Returns a shared_ptr to the new connection.
     * Throws `std::system_error` on failure.
     */
    std::shared_ptr<HttpServerConnection> add_connection(Socket&& socket);

    /*
     * Remove a connection from the active set.
     */
    void remove_connection(const std::shared_ptr<HttpServerConnection>& connection);
};

}  // namespace httpp

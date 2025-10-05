#pragma once

#include <atomic>
#include <future>
#include <loopp/event_loop.hpp>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "httpp/client/http_client_connection.hpp"
#include "httpp/http_message.hpp"

namespace httpp {

/*
 * HTTP client.
 * Supports sending HTTP requests asynchronously and synchronously (blocking).
 */
class HttpClient {
   public:
    HttpClient();
    ~HttpClient() noexcept;

    [[nodiscard]] bool is_running() const noexcept { return is_running_; }

    /*
     * Send request and get future for response.
     */
    std::future<HttpResponse> send_async(const HttpRequest& request);

    /*
     * Send request and wait for response (blocks).
     */
    HttpResponse send_sync(const HttpRequest& request);

    /*
     * Stop the client and close all connections.
     * Return true on success, false otherwise.
     * If the client is not running, it's a no-op and returns true.
     */
    bool stop();

   private:
    std::atomic<bool> is_running_{false};
    std::shared_ptr<loopp::EventLoop> event_loop_;
    std::thread event_loop_thread_;
    /* Set of active connections. */
    std::unordered_set<std::shared_ptr<HttpClientConnection>> connections_;
    /* Mutex to protect access to connections_. */
    std::mutex connections_mutex_;

    /*
     * Remove connection from the active set.
     */
    void remove_connection(const std::shared_ptr<HttpClientConnection>& connection);
};

}  // namespace httpp

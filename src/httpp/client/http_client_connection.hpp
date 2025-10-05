#pragma once

#include <functional>
#include <loopp/event_loop.hpp>
#include <memory>
#include <string>

#include "httpp/core/http_connection.hpp"
#include "httpp/core/socket.hpp"
#include "httpp/http_message.hpp"
#include "httpp/parser/http_response_parser.hpp"

namespace httpp {

/*
 * HTTP client connection.
 * Manages sending requests and receiving responses.
 */
class HttpClientConnection : public HttpConnection {
   public:
    using ResponseCallback = std::function<void(const HttpResponse&)>;

    static std::shared_ptr<HttpClientConnection> create(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop);

    ~HttpClientConnection() noexcept override = default;

    /*
     * Check if connection is available for sending a new request.
     */
    [[nodiscard]] bool is_available() const;

    /*
     * Send request and invoke callback when response is received.
     * Returns true on success, false on failure (check errno for details).
     */
    bool send_request(const HttpRequest& request, ResponseCallback callback);

   private:
    HttpClientConnection(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop);

    HttpResponseParser parser_;
    /* Callback to invoke when response is received. */
    ResponseCallback response_callback_;
    /* Whether a request has been sent and response is pending. */
    bool is_awaiting_response_{false};

    void feed_parser(const std::string& data) override;

    void handle_disconnect() override;
};

}  // namespace httpp

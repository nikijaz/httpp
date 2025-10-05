#pragma once

#include <functional>
#include <loopp/event_loop.hpp>
#include <memory>
#include <string>

#include "httpp/core/http_connection.hpp"
#include "httpp/core/socket.hpp"
#include "httpp/http_message.hpp"
#include "httpp/parser/http_request_parser.hpp"

namespace httpp {

/*
 * HTTP server connection.
 * Manages incoming requests and sending responses.
 */
class HttpServerConnection : public HttpConnection {
   public:
    using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

    static std::shared_ptr<HttpServerConnection> create(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop, RequestHandler handler);

    ~HttpServerConnection() noexcept override = default;

   private:
    HttpServerConnection(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop, RequestHandler handler);

    HttpRequestParser parser_;
    RequestHandler request_handler_;

    void send_response(const HttpResponse& response);

    void feed_parser(const std::string& data) override;
};

}  // namespace httpp

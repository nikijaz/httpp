#include "httpp/server/http_server_connection.hpp"

#include <cerrno>
#include <loopp/event_loop.hpp>
#include <memory>
#include <string>
#include <utility>

#include "httpp/core/http_connection.hpp"
#include "httpp/core/socket.hpp"
#include "httpp/http_message.hpp"
#include "httpp/parser/http_message_parser.hpp"

namespace httpp {

HttpServerConnection::HttpServerConnection(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop, RequestHandler handler)
    : HttpConnection(std::move(socket), std::move(loop)), request_handler_(std::move(handler)) {}

std::shared_ptr<HttpServerConnection> HttpServerConnection::create(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop, RequestHandler handler) {
    return std::shared_ptr<HttpServerConnection>(new HttpServerConnection(std::move(socket), std::move(loop), std::move(handler)));
}

void HttpServerConnection::send_response(const HttpResponse& response) {
    write(response.serialize());
}

void HttpServerConnection::feed_parser(const std::string& data) {
    parser_.append(data);

    auto request_opt = parser_.parse();
    if (parser_.state() == HttpMessageParser::ParseState::ERROR) {
        disconnect();
        return;
    }
    if (!request_opt.has_value()) {
        return;  // Need more data
    }

    const HttpRequest& request = request_opt.value();
    HttpResponse response = request_handler_(request);
    send_response(response);

    parser_.reset();

    if (!request.should_keep_alive()) {
        disconnect();
    }
}

}  // namespace httpp

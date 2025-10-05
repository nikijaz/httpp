#include "httpp/client/http_client_connection.hpp"

#include <cerrno>
#include <loopp/event_loop.hpp>
#include <memory>
#include <string>
#include <utility>

#include "httpp/core/http_connection.hpp"
#include "httpp/core/socket.hpp"
#include "httpp/http_message.hpp"
#include "httpp/http_types.hpp"
#include "httpp/parser/http_message_parser.hpp"

namespace httpp {

HttpClientConnection::HttpClientConnection(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop)
    : HttpConnection(std::move(socket), std::move(loop)) {}

std::shared_ptr<HttpClientConnection> HttpClientConnection::create(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop) {
    return std::shared_ptr<HttpClientConnection>(new HttpClientConnection(std::move(socket), std::move(loop)));
}

bool HttpClientConnection::send_request(const HttpRequest& request, ResponseCallback callback) {
    if (is_awaiting_response_) {
        errno = EALREADY;
        return false;
    }

    response_callback_ = std::move(callback);

    std::string request_data = request.serialize();
    if (!write(request_data)) {
        return false;
    }

    is_awaiting_response_ = true;
    return true;
}

bool HttpClientConnection::is_available() const {
    return !is_awaiting_response_;
}

void HttpClientConnection::feed_parser(const std::string& data) {
    parser_.append(data);

    auto response_opt = parser_.parse();
    if (parser_.state() == HttpMessageParser::ParseState::ERROR) {
        disconnect();
        return;
    }
    if (!response_opt.has_value()) {
        return;  // Need more data
    }

    const HttpResponse& response = response_opt.value();
    response_callback_(response);

    parser_.reset();
    is_awaiting_response_ = false;

    if (!response.should_keep_alive()) {
        disconnect();
    }
}

void HttpClientConnection::handle_disconnect() {
    HttpConnection::handle_disconnect();
    if (is_awaiting_response_) {
        response_callback_(
            HttpResponse::Builder()
                .status(HttpStatus::SERVICE_UNAVAILABLE)
                .header("Content-Type", "text/plain")
                .body("Connection closed before response was received.")
                .build());
        is_awaiting_response_ = false;
    }
}

}  // namespace httpp

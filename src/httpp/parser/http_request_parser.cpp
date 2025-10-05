#include "httpp/parser/http_request_parser.hpp"

#include <cstddef>
#include <optional>
#include <string>

#include "httpp/http_message.hpp"
#include "httpp/http_types.hpp"
#include "httpp/parser/http_message_parser.hpp"

namespace httpp {

std::optional<HttpRequest> HttpRequestParser::parse() {
    while (state_ != ParseState::COMPLETE && state_ != ParseState::ERROR) {
        switch (state_) {
            case ParseState::START_LINE: {
                if (!parse_start_line()) {
                    return std::nullopt;
                }

                state_ = ParseState::HEADERS;
                break;
            }

            case ParseState::HEADERS: {
                auto headers_opt = parse_headers();
                if (!headers_opt.has_value()) {
                    return std::nullopt;
                }

                for (const auto& [name, value] : headers_opt.value()) {
                    builder_.header(name, value);
                }

                state_ = ParseState::BODY;
                break;
            }

            case ParseState::BODY: {
                auto body_opt = parse_body(builder_.headers());
                if (!body_opt.has_value()) {
                    return std::nullopt;
                }

                builder_.body(body_opt.value());

                state_ = ParseState::COMPLETE;
                break;
            }

            case ParseState::COMPLETE:
            case ParseState::ERROR:
                break;
        }
    }

    return state_ == ParseState::COMPLETE ? std::optional(builder_.build()) : std::nullopt;
}

void HttpRequestParser::reset() {
    HttpMessageParser::reset();
    builder_ = HttpRequest::Builder();
}

bool HttpRequestParser::parse_start_line() {
    auto line_opt = extract_line();
    if (!line_opt.has_value()) {
        return false;  // Incomplete line
    }

    // <METHOD> <URL> <VERSION>\r\n
    const std::string& line = line_opt.value();
    if (line.size() > MAX_START_LINE_SIZE) {
        state_ = ParseState::ERROR;
        return false;  // Line too long
    }

    size_t first_space = line.find(' ');
    if (first_space == std::string::npos) {
        state_ = ParseState::ERROR;
        return false;  // Missing first space
    }

    size_t second_space = line.find(' ', first_space + 1);
    if (second_space == std::string::npos || second_space == first_space + 1) {
        state_ = ParseState::ERROR;
        return false;  // Either missing second space or no URL
    }

    std::string method_str = line.substr(0, first_space);
    std::string url = line.substr(first_space + 1, second_space - first_space - 1);
    std::string version_str = line.substr(second_space + 1);

    if (method_str.empty() || url.empty() || version_str.empty()) {
        state_ = ParseState::ERROR;
        return false;  // Malformed start line
    }

    auto method_opt = method_from_string(method_str);
    auto version_opt = version_from_string(version_str);

    if (!method_opt.has_value() || !version_opt.has_value()) {
        state_ = ParseState::ERROR;
        return false;  // Malformed start line
    }

    builder_.version(version_opt.value());
    builder_.method(method_opt.value());
    builder_.url(url);

    return true;
}

}  // namespace httpp

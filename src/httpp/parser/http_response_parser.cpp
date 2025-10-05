#include "httpp/parser/http_response_parser.hpp"

#include <cstddef>
#include <optional>
#include <string>

#include "httpp/http_message.hpp"
#include "httpp/http_types.hpp"
#include "httpp/parser/http_message_parser.hpp"

namespace httpp {

std::optional<HttpResponse> HttpResponseParser::parse() {
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

void HttpResponseParser::reset() {
    HttpMessageParser::reset();
    builder_ = HttpResponse::Builder();
}

bool HttpResponseParser::parse_start_line() {
    auto line_opt = extract_line();
    if (!line_opt.has_value()) {
        return false;  // Incomplete line
    }

    // <VERSION> <STATUS_CODE> [<REASON_PHRASE>]\r\n
    const std::string& line = line_opt.value();
    if (line.size() > MAX_START_LINE_SIZE) {
        state_ = ParseState::ERROR;
        return false;  // Line too long
    }

    size_t first_space = line.find(' ');
    if (first_space == std::string::npos || first_space == 0) {
        state_ = ParseState::ERROR;
        return false;  // Missing first space or empty version
    }

    size_t second_space = line.find(' ', first_space + 1);

    std::string version_str = line.substr(0, first_space);
    std::string status_str;
    if (second_space == std::string::npos) {
        // No reason phrase
        status_str = line.substr(first_space + 1);
    } else {
        // Has reason phrase (we can ignore it)
        status_str = line.substr(first_space + 1, second_space - first_space - 1);
    }

    if (status_str.empty()) {
        state_ = ParseState::ERROR;
        return false;  // Missing status code
    }

    auto version_opt = version_from_string(version_str);
    if (!version_opt.has_value()) {
        state_ = ParseState::ERROR;
        return false;  // Invalid version
    }

    int status_code;
    try {
        status_code = std::stoi(status_str);
    } catch (...) {
        state_ = ParseState::ERROR;
        return false;  // Invalid status code
    }

    if (status_code < 100 || status_code >= 600) {
        state_ = ParseState::ERROR;
        return false;  // Status code out of range
    }

    builder_.version(version_opt.value());
    builder_.status(HttpStatus(status_code));

    return true;
}

}  // namespace httpp

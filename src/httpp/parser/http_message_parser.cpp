#include "httpp/parser/http_message_parser.hpp"

#include <cctype>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace httpp {

namespace {

/*
 * Trim leading and trailing whitespace from a string.
 */
void trim(std::string& str) {
    size_t start = 0;
    size_t end = str.size();

    while (start < end && std::isspace(str[start])) {
        start++;
    }
    while (end > start && std::isspace(str[end - 1])) {
        end--;
    }

    if (start > 0 || end < str.size()) {
        str = str.substr(start, end - start);
    }
}

}  // namespace

void HttpMessageParser::append(const std::string& data) {
    if (buffer_.size() + data.size() > MAX_BUFFER_SIZE) {
        state_ = ParseState::ERROR;
        return;
    }
    buffer_ += data;
}

void HttpMessageParser::reset() {
    state_ = ParseState::START_LINE;
    buffer_.clear();
    parsed_header_count_ = 0;
}

std::optional<std::string> HttpMessageParser::extract_line() {
    size_t crlf_pos = buffer_.find(CRLF);
    if (crlf_pos == std::string::npos) {
        return std::nullopt;
    }

    std::string line = buffer_.substr(0, crlf_pos);
    buffer_.erase(0, crlf_pos + CRLF.size());
    return line;
}

std::optional<std::unordered_map<std::string, std::string>> HttpMessageParser::parse_headers() {
    std::unordered_map<std::string, std::string> headers{};

    while (true) {
        auto line_opt = extract_line();
        if (!line_opt.has_value()) {
            return std::nullopt;  // Need more data
        }

        const std::string& line = line_opt.value();
        if (line.empty()) {
            return headers;  // End of headers
        }

        if (parsed_header_count_ >= MAX_HEADER_COUNT) {
            state_ = ParseState::ERROR;
            return std::nullopt;  // Too many headers
        }

        auto header_opt = parse_header_line(line);
        if (!header_opt.has_value()) {
            state_ = ParseState::ERROR;
            return std::nullopt;  // Invalid header line
        }

        headers[header_opt->first] = header_opt->second;
        parsed_header_count_++;
    }
}

std::optional<std::pair<std::string, std::string>> HttpMessageParser::parse_header_line(const std::string& line) {
    if (line.size() > MAX_HEADER_SIZE) {
        state_ = ParseState::ERROR;
        return std::nullopt;  // Header line too long
    }

    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        state_ = ParseState::ERROR;
        return std::nullopt;  // Invalid header line
    }

    // <name>:<OWS><value><OWS>
    std::string name = line.substr(0, colon_pos);
    std::string value = line.substr(colon_pos + 1);
    trim(value);

    if (name.empty()) {
        state_ = ParseState::ERROR;
        return std::nullopt;  // Invalid header name
    }

    return std::make_pair(name, value);
}

std::optional<std::string> HttpMessageParser::parse_body(const std::unordered_map<std::string, std::string>& headers) {
    std::string transfer_encoding = headers.contains("transfer-encoding") ? headers.at("transfer-encoding") : "";
    for (auto& ch : transfer_encoding) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    if (transfer_encoding == "chunked") {
        return parse_chunked_body();
    }

    std::string content_length_str = headers.contains("content-length") ? headers.at("content-length") : "";
    size_t content_length;
    try {
        content_length = content_length_str.empty() ? 0 : std::stoul(content_length_str);
        if (content_length > MAX_BUFFER_SIZE) {
            state_ = ParseState::ERROR;
            return std::nullopt;  // Body too large
        }
    } catch (...) {
        state_ = ParseState::ERROR;
        return std::nullopt;  // Invalid Content-Length
    }

    if (content_length > 0) {
        return parse_fixed_body(content_length);
    }
    return "";  // No body
}

std::optional<std::string> HttpMessageParser::parse_fixed_body(size_t content_length) {
    if (buffer_.size() < content_length) {
        return std::nullopt;  // Need more data
    }

    auto body = buffer_.substr(0, content_length);
    buffer_.erase(0, content_length);
    return body;
}

std::optional<std::string> HttpMessageParser::parse_chunked_body() {
    std::string body;
    size_t current_pos = 0;

    while (current_pos < buffer_.size()) {
        size_t crlf_pos = buffer_.find(CRLF, current_pos);
        if (crlf_pos == std::string::npos) {
            return std::nullopt;  // Need more data for chunk size
        }

        std::string chunk_size_str = buffer_.substr(current_pos, crlf_pos - current_pos);
        size_t chunk_size;
        try {
            chunk_size = std::stoul(chunk_size_str, nullptr, 16);
        } catch (...) {
            state_ = ParseState::ERROR;
            return std::nullopt;  // Invalid chunk size
        }

        current_pos = crlf_pos + CRLF.size();  // Skip CRLF after chunk size

        if (chunk_size == 0) {
            // Final chunk, expect trailing CRLF
            if (current_pos + CRLF.size() <= buffer_.size() && buffer_.substr(current_pos, CRLF.size()) == CRLF) {
                current_pos += CRLF.size();
                buffer_.erase(0, current_pos);
                return body;
            } else {
                return std::nullopt;  // Need more data for final CRLF
            }
        }

        if (buffer_.size() < current_pos + chunk_size + CRLF.size()) {
            return std::nullopt;  // Not enough data for chunk
        }

        // Extract chunk data
        body += buffer_.substr(current_pos, chunk_size);
        current_pos += chunk_size;

        // Verify chunk ends with CRLF
        if (buffer_.substr(current_pos, CRLF.size()) != CRLF) {
            state_ = ParseState::ERROR;
            return std::nullopt;  // Invalid chunk
        }

        current_pos += CRLF.size();  // Skip CRLF after chunk data
    }

    // If we get here, we need more data
    return std::nullopt;
}

}  // namespace httpp

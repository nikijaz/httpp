#include "httpp/http_message.hpp"

#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>

#include "httpp/http_types.hpp"

namespace httpp {

namespace {

/*
 * Extract the path from a URL.
 * If no path is present, returns "/".
 */
std::string extract_path(const std::string& url) {
    static const std::string PROTOCOL_DELIMITER = "://";
    static const std::string DEFAULT_PATH = "/";

    size_t protocol_end = url.find(PROTOCOL_DELIMITER);
    size_t host_start = (protocol_end != std::string::npos) ? protocol_end + PROTOCOL_DELIMITER.size() : 0;
    size_t path_start = url.find('/', host_start);

    if (path_start != std::string::npos) {
        return url.substr(path_start);
    }
    return DEFAULT_PATH;
}

/*
 * Extract the host from a URL.
 * If no host is present, returns an empty string.
 */
std::string extract_host_header(const std::string& url) {
    static const std::string PROTOCOL_DELIMITER = "://";

    size_t protocol_end = url.find(PROTOCOL_DELIMITER);
    size_t host_start = (protocol_end != std::string::npos) ? protocol_end + PROTOCOL_DELIMITER.size() : 0;
    size_t path_start = url.find('/', host_start);
    std::string host = (path_start != std::string::npos) ? url.substr(host_start, path_start - host_start) : url.substr(host_start);

    return host;
}

}  // namespace

bool HttpMessage::should_keep_alive() const {
    static const std::string CONNECTION_HEADER = "connection";
    static const std::string KEEP_ALIVE = "keep-alive";
    static const std::string CLOSE = "close";

    auto it = headers_.find(CONNECTION_HEADER);
    switch (version_) {
        case HttpVersion::HTTP_1_0: {
            // Default to close unless "Connection: keep-alive" is specified
            if (it != headers_.end() && to_lower(it->second) == KEEP_ALIVE) {
                return true;
            }
            return false;
        }
        case HttpVersion::HTTP_1_1: {
            // Default to keep-alive unless "Connection: close" is specified
            if (it != headers_.end() && to_lower(it->second) == CLOSE) {
                return false;
            }
            return true;
        }
        default:
            return false;
    }
}

std::string HttpMessage::to_lower(const std::string& string) {
    std::string result = string;
    for (char& ch : result) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return result;
}

std::string HttpRequest::serialize() const {
    std::ostringstream result;

    // Start line
    result << to_string(method_) << " " << extract_path(url_) << " " << to_string(version_) << "\r\n";
    // User-provided headers
    for (const auto& [key, value] : headers_) {
        result << key << ": " << value << "\r\n";
    }
    // Required headers
    if (!headers_.contains("host")) {
        result << "host: " << extract_host_header(url_) << "\r\n";
    }
    if (!body_.empty() && !headers_.contains("content-length") && !headers_.contains("transfer-encoding")) {
        result << "content-length: " << body_.size() << "\r\n";
    }
    result << "\r\n";
    // Body
    result << body_;

    return result.str();
}

std::string HttpResponse::serialize() const {
    std::ostringstream result;

    // Start line
    result << to_string(version_) << " " << static_cast<int>(status_) << " " << get_reason_phrase(status_).value_or("") << "\r\n";
    // User-provided headers
    for (const auto& [key, value] : headers_) {
        result << key << ": " << value << "\r\n";
    }
    // Required headers
    if (!body_.empty() && !headers_.contains("content-length") && !headers_.contains("transfer-encoding")) {
        result << "content-length: " << body_.size() << "\r\n";
    }
    result << "\r\n";
    // Body
    result << body_;

    return result.str();
}

}  // namespace httpp

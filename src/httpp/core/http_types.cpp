#include "httpp/http_types.hpp"

#include <optional>
#include <string>
#include <unordered_map>

namespace httpp {

std::string to_string(HttpMethod method) {
    static const std::unordered_map<HttpMethod, std::string> METHOD_STR_MAP = {
        {HttpMethod::GET, "GET"},
        {HttpMethod::HEAD, "HEAD"},
        {HttpMethod::POST, "POST"},
        {HttpMethod::PUT, "PUT"},
        {HttpMethod::DELETE, "DELETE"},
        {HttpMethod::CONNECT, "CONNECT"},
        {HttpMethod::OPTIONS, "OPTIONS"},
        {HttpMethod::TRACE, "TRACE"},
        {HttpMethod::PATCH, "PATCH"}};

    return METHOD_STR_MAP.at(method);
}

std::optional<HttpMethod> method_from_string(const std::string& method_str) {
    static const std::unordered_map<std::string, HttpMethod> STR_METHOD_MAP = {
        {"GET", HttpMethod::GET},
        {"HEAD", HttpMethod::HEAD},
        {"POST", HttpMethod::POST},
        {"PUT", HttpMethod::PUT},
        {"DELETE", HttpMethod::DELETE},
        {"CONNECT", HttpMethod::CONNECT},
        {"OPTIONS", HttpMethod::OPTIONS},
        {"TRACE", HttpMethod::TRACE},
        {"PATCH", HttpMethod::PATCH}};

    return STR_METHOD_MAP.contains(method_str) ? std::optional(STR_METHOD_MAP.at(method_str)) : std::nullopt;
}

std::string to_string(HttpVersion version) {
    static const std::unordered_map<HttpVersion, std::string> VERSION_STR_MAP = {
        {HttpVersion::HTTP_1_0, "HTTP/1.0"},
        {HttpVersion::HTTP_1_1, "HTTP/1.1"}};

    return VERSION_STR_MAP.at(version);
}

std::optional<HttpVersion> version_from_string(const std::string& version_str) {
    static const std::unordered_map<std::string, HttpVersion> STR_VERSION_MAP = {
        {"HTTP/1.0", HttpVersion::HTTP_1_0},
        {"HTTP/1.1", HttpVersion::HTTP_1_1}};

    return STR_VERSION_MAP.contains(version_str) ? std::optional(STR_VERSION_MAP.at(version_str)) : std::nullopt;
}

std::optional<std::string> get_reason_phrase(HttpStatus status) {
    static const std::unordered_map<HttpStatus, std::string> REASON_PHRASES = {
        {HttpStatus::OK, "OK"},
        {HttpStatus::CREATED, "Created"},
        {HttpStatus::NO_CONTENT, "No Content"},

        {HttpStatus::MOVED_PERMANENTLY, "Moved Permanently"},
        {HttpStatus::FOUND, "Found"},
        {HttpStatus::NOT_MODIFIED, "Not Modified"},
        {HttpStatus::TEMPORARY_REDIRECT, "Temporary Redirect"},

        {HttpStatus::BAD_REQUEST, "Bad Request"},
        {HttpStatus::UNAUTHORIZED, "Unauthorized"},
        {HttpStatus::FORBIDDEN, "Forbidden"},
        {HttpStatus::NOT_FOUND, "Not Found"},
        {HttpStatus::METHOD_NOT_ALLOWED, "Method Not Allowed"},
        {HttpStatus::CONFLICT, "Conflict"},
        {HttpStatus::UNPROCESSABLE_ENTITY, "Unprocessable Entity"},
        {HttpStatus::TOO_MANY_REQUESTS, "Too Many Requests"},

        {HttpStatus::INTERNAL_SERVER_ERROR, "Internal Server Error"},
        {HttpStatus::BAD_GATEWAY, "Bad Gateway"},
        {HttpStatus::SERVICE_UNAVAILABLE, "Service Unavailable"},
        {HttpStatus::GATEWAY_TIMEOUT, "Gateway Timeout"}};

    return REASON_PHRASES.contains(status) ? std::optional(REASON_PHRASES.at(status)) : std::nullopt;
}

}  // namespace httpp

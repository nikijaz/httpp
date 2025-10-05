#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace httpp {

/*
 * Enumeration of supported HTTP methods.
 * If updating, also update the mapping in to_string and method_from_string functions.
 */
enum class HttpMethod : uint8_t {
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    PATCH,
    OPTIONS,
    CONNECT,
    TRACE
};

/*
 * Convert HttpMethod to its string representation.
 */
std::string to_string(HttpMethod method);

/*
 * Convert string representation of HTTP method to HttpMethod enum.
 * Returns std::nullopt if the method is not recognized.
 */
std::optional<HttpMethod> method_from_string(const std::string& method_str);

/*
 * Enumeration of supported HTTP versions.
 * If updating, also update the mapping in to_string and version_from_string functions.
 */
enum class HttpVersion : uint8_t {
    HTTP_1_0,
    HTTP_1_1
};

/*
 * Convert HttpVersion enum to its string representation.
 */
std::string to_string(HttpVersion version);

/*
 * Convert string representation of HTTP version to HttpVersion enum.
 * Returns std::nullopt if the version is not recognized.
 */
std::optional<HttpVersion> version_from_string(const std::string& version_str);

/*
 * Enumeration of most common HTTP status codes.
 * If updating, also update the mapping in get_reason_phrase function.
 */
enum class HttpStatus : uint16_t {
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,

    MOVED_PERMANENTLY = 301,
    FOUND = 302,
    NOT_MODIFIED = 304,
    TEMPORARY_REDIRECT = 307,

    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    CONFLICT = 409,
    UNPROCESSABLE_ENTITY = 422,
    TOO_MANY_REQUESTS = 429,

    INTERNAL_SERVER_ERROR = 500,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504
};

/*
 * Get the reason phrase associated with an HTTP status code.
 * Returns std::nullopt if the status code is not recognized.
 */
std::optional<std::string> get_reason_phrase(HttpStatus status);

}  // namespace httpp

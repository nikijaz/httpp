#pragma once

#include <string>
#include <unordered_map>
#include <utility>

#include "httpp/http_types.hpp"

namespace httpp {

/*
 * Base class for HTTP messages.
 */
class HttpMessage {
   public:
    virtual ~HttpMessage() noexcept = default;

    [[nodiscard]] HttpVersion version() const noexcept { return version_; }
    [[nodiscard]] const std::unordered_map<std::string, std::string>& headers() const noexcept { return headers_; }
    [[nodiscard]] const std::string& body() const noexcept { return body_; }

    /*
     * Determine if the connection should be kept alive.
     * Works based on the HTTP version and the "Connection" header.
     */
    bool should_keep_alive() const;

    /*
     * Serialize the HTTP message to a string.
     * Adds headers required by standard if not already present.
     */
    virtual std::string serialize() const = 0;

   protected:
    HttpMessage() = default;

    HttpVersion version_{HttpVersion::HTTP_1_1};
    /* Headers map. Keys must be stored in lowercase. */
    std::unordered_map<std::string, std::string> headers_{};
    std::string body_{};

    /*
     * Convert a string to lowercase.
     * Used for case-insensitive header name comparison.
     */
    static std::string to_lower(const std::string& string);
};

/*
 * HTTP request message.
 * For constructing instances, use HttpRequest::Builder.
 */
class HttpRequest : public HttpMessage {
   public:
    ~HttpRequest() noexcept override = default;

    [[nodiscard]] HttpMethod method() const noexcept { return method_; }
    [[nodiscard]] const std::string& url() const noexcept { return url_; }

    std::string serialize() const override;

    class Builder;

   protected:
    HttpMethod method_{HttpMethod::GET};
    std::string url_{};

   private:
    HttpRequest() = default;
};

/*
 * HTTP request builder.
 */
class HttpRequest::Builder {
   public:
    Builder() = default;

    [[nodiscard]] HttpVersion version() const noexcept { return request_.version_; };
    [[nodiscard]] HttpMethod method() const noexcept { return request_.method_; };
    [[nodiscard]] const std::string& url() const noexcept { return request_.url_; };
    [[nodiscard]] const std::unordered_map<std::string, std::string>& headers() const noexcept { return request_.headers_; };
    [[nodiscard]] const std::string& body() const noexcept { return request_.body_; };

    Builder& version(HttpVersion version) noexcept {
        request_.version_ = version;
        return *this;
    };
    Builder& method(HttpMethod method) noexcept {
        request_.method_ = method;
        return *this;
    };
    Builder& url(const std::string& url) noexcept {
        request_.url_ = url;
        return *this;
    };
    Builder& header(const std::string& name, const std::string& value) noexcept {
        request_.headers_[to_lower(name)] = value;
        return *this;
    };
    Builder& body(const std::string& body) noexcept {
        request_.body_ = body;
        return *this;
    };

    /*
     * Build the HTTP request.
     * Warning: After calling this method, the builder should not be used again.
     */
    HttpRequest&& build() { return std::move(request_); }

   private:
    /* HTTP request being built. */
    HttpRequest request_{};
};

/*
 * HTTP response message.
 * For constructing instances, use HttpResponse::Builder.
 */
class HttpResponse : public HttpMessage {
   public:
    ~HttpResponse() noexcept override = default;

    [[nodiscard]] HttpStatus status() const noexcept { return status_; }

    std::string serialize() const override;

    class Builder;

   protected:
    HttpStatus status_{HttpStatus::OK};

   private:
    HttpResponse() = default;
};

/*
 * HTTP response builder.
 */
class HttpResponse::Builder {
   public:
    Builder() = default;

    [[nodiscard]] HttpVersion version() const noexcept { return response_.version_; };
    [[nodiscard]] HttpStatus status() const noexcept { return response_.status_; };
    [[nodiscard]] const std::unordered_map<std::string, std::string>& headers() const noexcept { return response_.headers_; };
    [[nodiscard]] const std::string& body() const noexcept { return response_.body_; };

    Builder& version(HttpVersion version) noexcept {
        response_.version_ = version;
        return *this;
    };
    Builder& status(HttpStatus status) noexcept {
        response_.status_ = status;
        return *this;
    };
    Builder& header(const std::string& name, const std::string& value) noexcept {
        response_.headers_[to_lower(name)] = value;
        return *this;
    };
    Builder& body(const std::string& body) noexcept {
        response_.body_ = body;
        return *this;
    };

    /*
     * Build the HTTP response.
     * Warning: After calling this method, the builder should not be used again.
     */
    HttpResponse&& build() { return std::move(response_); };

   private:
    /* HTTP response being built. */
    HttpResponse response_;
};

}  // namespace httpp

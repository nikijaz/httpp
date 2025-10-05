#include <catch2/catch_test_macros.hpp>

#include "httpp/http_message.hpp"
#include "httpp/http_types.hpp"

TEST_CASE("HttpRequest serialization", "[http_message]") {
    auto request = httpp::HttpRequest::Builder()
                       .method(httpp::HttpMethod::GET)
                       .url("http://example.com/path?query=1")
                       .version(httpp::HttpVersion::HTTP_1_1)
                       .header("User-Agent", "httpp-test")
                       .build();

    std::string serialized = request.serialize();
    REQUIRE(serialized.contains("GET /path?query=1 HTTP/1.1"));
    REQUIRE(serialized.contains("user-agent: httpp-test"));
    REQUIRE(serialized.contains("host: example.com"));
}

TEST_CASE("HttpResponse serialization", "[http_message]") {
    auto response = httpp::HttpResponse::Builder()
                        .status(httpp::HttpStatus::OK)
                        .version(httpp::HttpVersion::HTTP_1_1)
                        .header("Content-Type", "text/plain")
                        .body("Hello, World!")
                        .build();

    std::string serialized = response.serialize();
    REQUIRE(serialized.contains("HTTP/1.1 200 OK"));
    REQUIRE(serialized.contains("content-type: text/plain"));
    REQUIRE(serialized.contains("content-length: 13"));
    REQUIRE(serialized.contains("Hello, World!"));
}

TEST_CASE("Keep-alive logic", "[http_message]") {
    SECTION("HTTP/1.0 defaults to close") {
        auto request = httpp::HttpRequest::Builder()
                           .method(httpp::HttpMethod::GET)
                           .url("/")
                           .version(httpp::HttpVersion::HTTP_1_0)
                           .build();
        REQUIRE_FALSE(request.should_keep_alive());
    }

    SECTION("HTTP/1.0 respects keep-alive header") {
        auto request = httpp::HttpRequest::Builder()
                           .method(httpp::HttpMethod::GET)
                           .url("/")
                           .version(httpp::HttpVersion::HTTP_1_0)
                           .header("Connection", "keep-alive")
                           .build();
        REQUIRE(request.should_keep_alive());
    }

    SECTION("HTTP/1.1 defaults to keep-alive") {
        auto request = httpp::HttpRequest::Builder()
                           .method(httpp::HttpMethod::GET)
                           .url("/")
                           .version(httpp::HttpVersion::HTTP_1_1)
                           .build();
        REQUIRE(request.should_keep_alive());
    }

    SECTION("HTTP/1.1 respects close header") {
        auto request = httpp::HttpRequest::Builder()
                           .method(httpp::HttpMethod::GET)
                           .url("/")
                           .version(httpp::HttpVersion::HTTP_1_1)
                           .header("Connection", "close")
                           .build();
        REQUIRE_FALSE(request.should_keep_alive());
    }
}

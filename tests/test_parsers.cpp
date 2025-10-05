#include <catch2/catch_test_macros.hpp>

#include "httpp/http_message.hpp"
#include "httpp/http_types.hpp"
#include "httpp/parser/http_message_parser.hpp"
#include "httpp/parser/http_request_parser.hpp"
#include "httpp/parser/http_response_parser.hpp"

TEST_CASE("Parse GET request", "[parser]") {
    httpp::HttpRequestParser parser;
    parser.append(
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "\r\n");

    auto request_opt = parser.parse();
    REQUIRE(request_opt.has_value());
    if (!request_opt.has_value()) return;

    const auto& request = request_opt.value();
    REQUIRE(request.method() == httpp::HttpMethod::GET);
    REQUIRE(request.url() == "/index.html");
    REQUIRE(request.version() == httpp::HttpVersion::HTTP_1_1);
    REQUIRE(request.headers().at("host") == "example.com");
}

TEST_CASE("Parse POST request with fixed-length body", "[parser]") {
    httpp::HttpRequestParser parser;
    parser.append(
        "POST /api/data HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello, World!");

    auto request_opt = parser.parse();
    REQUIRE(request_opt.has_value());
    if (!request_opt.has_value()) return;

    const auto& request = request_opt.value();
    REQUIRE(request.method() == httpp::HttpMethod::POST);
    REQUIRE(request.url() == "/api/data");
    REQUIRE(request.body() == "Hello, World!");
}

TEST_CASE("Parse request with chunked body", "[parser]") {
    httpp::HttpRequestParser parser;
    parser.append(
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "5\r\n"
        "Hello\r\n"
        "7\r\n"
        ", World\r\n"
        "0\r\n"
        "\r\n");

    auto request_opt = parser.parse();
    if (!request_opt.has_value()) return;
    REQUIRE(request_opt.value().body() == "Hello, World");
}

TEST_CASE("Parse malformed request", "[parser]") {
    httpp::HttpRequestParser parser;
    parser.append("INVALID REQUEST\r\n");

    auto request_opt = parser.parse();
    REQUIRE_FALSE(request_opt.has_value());
    REQUIRE(parser.state() == httpp::HttpMessageParser::ParseState::ERROR);
}

TEST_CASE("Parse 200 OK response", "[parser]") {
    httpp::HttpResponseParser parser;
    parser.append(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "OK");

    auto response_opt = parser.parse();
    REQUIRE(response_opt.has_value());
    if (!response_opt.has_value()) return;

    const auto& response = response_opt.value();
    REQUIRE(response.status() == httpp::HttpStatus::OK);
    REQUIRE(response.version() == httpp::HttpVersion::HTTP_1_1);
    REQUIRE(response.body() == "OK");
    REQUIRE(response.headers().at("content-type") == "text/plain");
}

TEST_CASE("Parse response with chunked body", "[parser]") {
    httpp::HttpResponseParser parser;
    parser.append(
        "HTTP/1.1 200 OK\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "4\r\n"
        "Test\r\n"
        "0\r\n"
        "\r\n");

    auto response_opt = parser.parse();
    REQUIRE(response_opt.has_value());
    if (!response_opt.has_value()) return;

    const auto& response = response_opt.value();
    REQUIRE(response.body() == "Test");
}

TEST_CASE("Parse malformed response", "[parser]") {
    httpp::HttpResponseParser parser;
    parser.append("INVALID RESPONSE\r\n");

    auto response_opt = parser.parse();
    REQUIRE_FALSE(response_opt.has_value());
    REQUIRE(parser.state() == httpp::HttpMessageParser::ParseState::ERROR);
}

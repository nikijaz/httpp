#include <catch2/catch_test_macros.hpp>
#include <thread>

#include "httpp/http_client.hpp"
#include "httpp/http_message.hpp"
#include "httpp/http_server.hpp"
#include "httpp/http_types.hpp"

TEST_CASE("Server handles GET request", "[integration]") {
    httpp::HttpServer server;

    server.on_request([](const httpp::HttpRequest& req) {
        REQUIRE(req.method() == httpp::HttpMethod::GET);
        REQUIRE(req.url() == "/test");

        return httpp::HttpResponse::Builder()
            .status(httpp::HttpStatus::OK)
            .header("Content-Type", "text/plain")
            .body("Test Response")
            .build();
    });

    std::thread server_thread([&server]() {
        server.start(8888);
    });

    while (!server.is_running()) {
        std::this_thread::yield();
    }

    httpp::HttpClient client;
    auto request = httpp::HttpRequest::Builder()
                       .method(httpp::HttpMethod::GET)
                       .url("http://localhost:8888/test")
                       .build();
    auto response = client.send_sync(request);

    REQUIRE(response.status() == httpp::HttpStatus::OK);
    REQUIRE(response.body() == "Test Response");

    server.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

TEST_CASE("Client handles POST request", "[integration]") {
    httpp::HttpServer server;

    server.on_request([](const httpp::HttpRequest& req) {
        REQUIRE(req.method() == httpp::HttpMethod::POST);
        REQUIRE(req.body() == "Test Data");

        return httpp::HttpResponse::Builder()
            .status(httpp::HttpStatus::CREATED)
            .body("Data Received")
            .build();
    });

    std::thread server_thread([&server]() {
        server.start(8888);
    });

    while (!server.is_running()) {
        std::this_thread::yield();
    }

    httpp::HttpClient client;
    auto request = httpp::HttpRequest::Builder()
                       .method(httpp::HttpMethod::POST)
                       .url("http://localhost:8888/api")
                       .body("Test Data")
                       .build();
    auto response = client.send_sync(request);

    REQUIRE(response.status() == httpp::HttpStatus::CREATED);
    REQUIRE(response.body() == "Data Received");

    server.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
}

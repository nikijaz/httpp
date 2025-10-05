#pragma once

#include <optional>

#include "httpp/http_message.hpp"
#include "httpp/parser/http_message_parser.hpp"

namespace httpp {

/*
 * State machine for parsing HTTP requests from a stream of data.
 */
class HttpRequestParser : public HttpMessageParser {
   public:
    HttpRequestParser() = default;
    ~HttpRequestParser() noexcept override = default;

    /*
     * Parse the HTTP request from the buffered data.
     * Returns std::nullopt if the request is incomplete or invalid.
     */
    std::optional<HttpRequest> parse();

    /*
     * Reset the parser to initial state.
     */
    void reset() override;

   private:
    /* Builder for constructing the HTTP request. */
    HttpRequest::Builder builder_;

    /*
     * Parse the start line of the HTTP request, filling the builder with parsed data.
     * Returns true if the start line is fully parsed, false otherwise.
     */
    bool parse_start_line();
};

}  // namespace httpp

#pragma once

#include <optional>

#include "httpp/http_message.hpp"
#include "httpp/parser/http_message_parser.hpp"

namespace httpp {

/*
 * State machine for parsing HTTP responses from a stream of data.
 */
class HttpResponseParser : public HttpMessageParser {
   public:
    HttpResponseParser() = default;
    ~HttpResponseParser() noexcept override = default;

    /*
     * Parse the HTTP response from the buffered data.
     * Returns std::nullopt if the response is incomplete or invalid.
     */
    std::optional<HttpResponse> parse();

    /*
     * Reset the parser to initial state.
     */
    void reset() override;

   private:
    /* Builder for constructing the HTTP response. */
    HttpResponse::Builder builder_;

    /*
     * Parse the start line of the HTTP response, filling the builder with parsed data.
     * Returns true if the start line is fully parsed, false otherwise.
     */
    bool parse_start_line();
};

}  // namespace httpp

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace httpp {

/*
 * Base class for parsing HTTP messages.
 * Provides common functionality for parsing headers and body.
 */
class HttpMessageParser {
   public:
    virtual ~HttpMessageParser() noexcept = default;

    enum class ParseState : uint8_t {
        START_LINE,
        HEADERS,
        BODY,
        COMPLETE,
        ERROR
    };

    [[nodiscard]] ParseState state() const noexcept { return state_; }

    /*
     * Append data to the internal buffer for parsing.
     * May set state to ERROR if buffer exceeds maximum size.
     */
    void append(const std::string& data);

    /*
     * Reset the parser to initial state.
     */
    virtual void reset();

   protected:
    HttpMessageParser() = default;

    static constexpr std::string_view CRLF = "\r\n";
    static constexpr size_t MAX_START_LINE_SIZE = 8192;

    /* Current state of the parser. */
    ParseState state_{ParseState::START_LINE};

    /*
     * Extract a single line from the buffer.
     * Returns std::nullopt if no line ending with CRLF is found.
     */
    std::optional<std::string> extract_line();

    /*
     * Parse headers from the buffer.
     * Returns std::nullopt if headers are incomplete or invalid.
     */
    std::optional<std::unordered_map<std::string, std::string>> parse_headers();

    /*
     * Parse the message body based on headers (Content-Length or Transfer-Encoding).
     * Returns std::nullopt if the body is incomplete or invalid.
     */
    std::optional<std::string> parse_body(const std::unordered_map<std::string, std::string>& headers);

   private:
    static constexpr size_t MAX_BUFFER_SIZE = 16l * 1024 * 1024;  // 16 MB
    static constexpr size_t MAX_HEADER_COUNT = 128;
    static constexpr size_t MAX_HEADER_SIZE = 8192;

    /* Buffer to accumulate incoming data. */
    std::string buffer_;
    size_t parsed_header_count_{0};

    /*
     * Parse a single header line into a name-value pair.
     * Returns std::nullopt if the line is invalid.
     */
    std::optional<std::pair<std::string, std::string>> parse_header_line(const std::string& line);

    /*
     * Parse a fixed-length body based on Content-Length header.
     * Returns std::nullopt if the body is incomplete or invalid.
     */
    std::optional<std::string> parse_fixed_body(size_t content_length);

    /*
     * Parse a chunked transfer-encoded body.
     * Returns std::nullopt if the body is incomplete or invalid.
     */
    std::optional<std::string> parse_chunked_body();
};

}  // namespace httpp

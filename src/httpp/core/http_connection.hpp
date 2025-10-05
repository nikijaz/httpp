#pragma once

#include <functional>
#include <loopp/event_loop.hpp>
#include <memory>
#include <string>

#include "httpp/core/socket.hpp"

namespace httpp {

/*
 * Base class for HTTP connections.
 * Manages read and write operations on the socket.
 * Must be instantiated as a std::shared_ptr.
 */
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
   public:
    using DisconnectCallback = std::function<void(std::shared_ptr<class HttpConnection> connection)>;

    virtual ~HttpConnection() noexcept = default;

    /*
     * Start polling for events on the connection socket.
     * Returns true on success, false on failure (check errno for details).
     */
    bool start();

    /*
     * Append data to the connection's write buffer.
     * Data will be sent to the socket when the socket is writable.
     * Returns true on success, false on failure (check errno for details).
     */
    bool write(const std::string& data);

    /*
     * Disconnect and close the connection, calling the disconnect callback if set.
     * Returns true on success, false on failure (check errno for details).
     */
    bool disconnect();

    /*
     * Set a callback to be called when the connection is disconnected.
     */
    void on_disconnect(const DisconnectCallback& callback);

    /*
     * Close the connection by removing it from the event loop.
     * Returns true on success, false on failure (check errno for details).
     * If the connection is already closed, it's a no-op and returns true.
     */
    bool close();

   protected:
    /* Must be instantiated as a std::shared_ptr. */
    HttpConnection(Socket&& socket, std::shared_ptr<loopp::EventLoop> loop);

    Socket socket_;
    std::shared_ptr<loopp::EventLoop> loop_;

    /*
     * Feed data to the HTTP parser.
     */
    virtual void feed_parser(const std::string& data) = 0;

    /*
     * Called when the connection is shut down.
     * Default implementation calls the disconnect callback if set.
     */
    virtual void handle_disconnect();

   private:
    std::string write_buffer_{};
    DisconnectCallback disconnect_callback_;

    /*
     * Called when socket is readable.
     */
    void handle_read();

    /*
     * Called when socket is writable.
     */
    void handle_write();
};

}  // namespace httpp

# httpp

A lightweight, modern C++ HTTP server and client library built on top of the
[loopp](https://github.com/nikijaz/loopp) library. Provides a clean,
asynchronous interface for building HTTP applications with thread-safe design.

## Usage

### HTTP Server

Create a server, register a request handler, then start listening. The handler
receives requests and returns responses synchronously while the server manages
connections asynchronously.

```cpp
httpp::HttpServer server;
server.on_request([](const httpp::HttpRequest& request) {
    return httpp::HttpResponse::Builder()
        .status(httpp::HttpStatus::OK)
        .header("Content-Type", "text/plain")
        .body("Hello, World!")
        .build();
});
server.start(8080); // Blocks until server.stop() called
```

### HTTP Client

Create a client and send requests either synchronously or asynchronously. The
client manages connections and event loop automatically in a background thread.

```cpp
httpp::HttpClient client;
auto request = httpp::HttpRequest::Builder()
    .method(httpp::HttpMethod::GET)
    .url("http://example.com/")
    .header("User-Agent", "httpp")
    .build();

// Synchronous (blocks until response received)
auto response = client.send_sync(request);

// Asynchronous (returns future immediately)
auto future = client.send_async(request);
auto response = future.get(); // Block when needed
```

## Installation

**CMake FetchContent:**

```cmake
include(FetchContent)  
FetchContent_Declare(httpp GIT_REPOSITORY https://github.com/nikijaz/httpp.git)  
FetchContent_MakeAvailable(httpp)  
target_link_libraries(your_target PRIVATE httpp)
```

**CMake Submodule:**

```cmake
add_subdirectory(path/to/httpp)  
target_link_libraries(your_target PRIVATE httpp)
```

## Development

0. Ensure **CMake 3.10+** and **C++23 compiler** are installed.
1. Clone with submodules and navigate to the repository:

    ```bash
    git clone --recursive https://github.com/nikijaz/httpp.git
    cd httpp/
    ```

2. Create a build directory and set up the project:

    ```bash
    mkdir build && cd build
    cmake ..
    ```

3. Make changes to the code.

4. Build the library and tests:

    ```bash
    make
    ```

5. Run the test suite:

    ```bash
    ctest
    ```

6. Check for any code quality issues:

    ```bash
    ../scripts/code-quality.sh
    ```

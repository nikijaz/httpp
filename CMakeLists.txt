# Set CMake version and project configuration
cmake_minimum_required(VERSION 3.10.0)
project(httpp LANGUAGES CXX)

# Set and force C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Enable compile_commands.json for IDE integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add loopp subdirectory
add_subdirectory(lib/loopp)

# Add source files
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp")
add_library(httpp ${SOURCES})

# Add header files
target_include_directories(httpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(httpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Link with loopp
target_link_libraries(httpp PUBLIC loopp)

# Set compiler warnings
target_compile_options(httpp PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wformat=2
    -Wundef
)

# Include tests only if this is the main project
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Add Catch2 subdirectory
    add_subdirectory(lib/Catch2 SYSTEM)

    # Add test source files
    file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")
    add_executable(httpp_test ${TEST_SOURCES})

    # Enable testing
    enable_testing()
    target_link_libraries(httpp_test PRIVATE httpp Catch2::Catch2WithMain)

    # Discover and register individual tests with CTest
    include(Catch)
    catch_discover_tests(httpp_test)
endif()

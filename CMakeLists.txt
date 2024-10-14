cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

add_executable(server ${SOURCE_FILES}
        src/request/http_request.h
        src/request/http_request_handler.h
        src/response/http_response.h
        src/url/abstract_url_action.h
        src/url/default_url_action.h
        src/url/url_handler.h
        src/url/not_found_url_action.h
        src/url/echo_url_action.h
        src/url/user_agent_url_action.h
        src/concurrent/thread_pool.h)

target_link_libraries(server PRIVATE Threads::Threads ZLIB::ZLIB)

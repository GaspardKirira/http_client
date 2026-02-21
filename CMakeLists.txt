cmake_minimum_required(VERSION 3.20)
project(http_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(http_client INTERFACE)
add_library(http_client::http_client ALIAS http_client)

target_include_directories(http_client INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

if (MSVC)
  target_compile_options(http_client INTERFACE /W4 /permissive-)
else()
  target_compile_options(http_client INTERFACE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
enable_testing()

add_executable(http_client_basic_test tests/test_basic.cpp)
target_link_libraries(http_client_basic_test PRIVATE http_client::http_client)
add_test(NAME http_client.basic COMMAND http_client_basic_test)

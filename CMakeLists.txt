cmake_minimum_required(VERSION 3.16)
project(crow_chat CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG        asio-1-32-0
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
  FetchContent_Populate(asio)
endif()
set(ASIO_INCLUDE_DIR "${asio_SOURCE_DIR}/asio/include" CACHE PATH "" FORCE)

set(CROW_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(CROW_BUILD_TESTS OFF CACHE INTERNAL "")
set(CROW_ENABLE_SSL OFF CACHE INTERNAL "")
set(CROW_ENABLE_COMPRESSION OFF CACHE INTERNAL "")

FetchContent_Declare(
  Crow
  GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
  GIT_TAG        v1.2.1
)
FetchContent_MakeAvailable(Crow)

file(COPY "${CMAKE_SOURCE_DIR}/templates" DESTINATION "${CMAKE_BINARY_DIR}")

add_executable(chat_server src/main.cpp)
target_link_libraries(chat_server PRIVATE Crow::Crow)

if(MSVC)
  target_compile_options(chat_server PRIVATE /utf-8)
endif()

file(TO_CMAKE_PATH "${CMAKE_BINARY_DIR}/templates" CHAT_TEMPLATES_MACRO)
target_compile_definitions(chat_server PRIVATE "CHAT_TEMPLATES_DIR=\"${CHAT_TEMPLATES_MACRO}\"")

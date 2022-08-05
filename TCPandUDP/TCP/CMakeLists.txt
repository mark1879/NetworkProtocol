cmake_minimum_required(VERSION 3.1)

project(TcpPractice C CXX)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

# message(">>> PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")
# message(">>> PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
# message(">>> CMAKE_RUNTIME_OUTPUT_DIRECTORY: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
# message(">>> CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
# message(">>> CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

include(CheckSymbolExists)
check_symbol_exists(epoll_create "sys/epoll.h" EPOLL_EXISTS)
if (EPOLL_EXISTS)
    set(EPOLL_ENABLE 1 CACHE INTERNAL "enable epoll")
else ()
    set(EPOLL_ENABLE "" CACHE INTERNAL "not enable epoll")
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(select/)
add_subdirectory(simple_tcp_client/)
add_subdirectory(poll/)
add_subdirectory(epoll/)
cmake_minimum_required(VERSION 3.10)

project(lab3 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main.cpp
    process_handler.cpp
    shared_memory.cpp
)

set(HEADERS
    process_handler.hpp
    shared_memory.hpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Crossplatform settings
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32)
    target_link_libraries(${PROJECT_NAME} kernel32)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE _POSIX_C_SOURCE=200809L)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

# Compilation warnings
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

add_definitions(-DLOG_TO_FILE)
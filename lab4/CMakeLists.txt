cmake_minimum_required(VERSION 3.10)

project(shserial)

set(CMAKE_CXX_STANDARD 20)

add_executable(sertest my_serial.hpp process_handler.hpp main.cpp)
add_executable(simulator my_serial.hpp simulator.cpp)

ADD_EXECUTABLE(test test.cpp)
IF (WIN32)
    TARGET_LINK_LIBRARIES(test ws2_32)
ENDIF(WIN32)

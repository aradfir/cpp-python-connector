cmake_minimum_required(VERSION 3.16)
project(socket_test)

set(CMAKE_CXX_STANDARD 14)

add_executable(socket_test main.cpp Tensor.cpp)

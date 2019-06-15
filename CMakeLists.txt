cmake_minimum_required(VERSION 3.10)
project(WikiPathfinder)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lcurl")

add_executable(WikiPathfinder main.cpp HTTPDownloader.h HTTPDownloader.cpp Queue.cpp)

target_link_libraries(WikiPathfinder curl)

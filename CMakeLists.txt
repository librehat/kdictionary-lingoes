cmake_minimum_required(VERSION 2.8.11)

project(kdictionary-lingoes)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Core REQUIRED)

set(kdictionary-lingoes_SRCS lingoes.cpp main.cpp)

add_executable(kdictionary-lingoes ${kdictionary-lingoes_SRCS})
target_link_libraries(kdictionary-lingoes Qt5::Core)

project(kdictionary-lingoes)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(kdictionary-lingoes_SRCS kdictionary-lingoes.cpp main.cpp)
qt4_automoc(${kdictionary-lingoes_SRCS})
add_executable(kdictionary-lingoes ${kdictionary-lingoes_SRCS})
target_link_libraries(kdictionary-lingoes ${QT_QTCORE_LIBRARY})

install(TARGETS kdictionary-lingoes RUNTIME DESTINATION bin)

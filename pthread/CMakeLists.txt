cmake_minimum_required(VERSION 3.10)

project(TP1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lgmp -lgmpxx")

set(SOURCES
    src/main.cpp
)

add_executable(TP1 ${SOURCES})

find_library(GMP_LIBRARIES NAMES gmp)

target_link_libraries(TP1 ${GMP_LIBRARIES})

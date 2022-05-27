cmake_minimum_required(VERSION 3.16)

project(Dalek)

set(CMAKE_CXX_STANDARD 17)

file(GLOB HEADS
        ${CMAKE_CURRENT_SOURCE_DIR}/base/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/http/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/reactor/*.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pson/*.hpp)


add_executable(Dalek Dalek.cc ${HEADS} )

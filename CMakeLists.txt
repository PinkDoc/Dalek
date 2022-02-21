cmake_minimum_required(VERSION 3.16)

project(Dalek)

set(CMAKE_CXX_STANDARD 17)

file(GLOB HEADS ${CMAKE_CURRENT_SOURCE_DIR}/base/*.h ${CMAKE_CURRENT_SOURCE_DIR}/http/*.h ${CMAKE_CURRENT_SOURCE_DIR}/reactor/*.h)

set(JSONXX ${CMAKE_CURRENT_SOURCE_DIR}/base/jsonxx.cc)

add_executable(Dalek Dalek.cc ${HEADS} ${JSONXX})

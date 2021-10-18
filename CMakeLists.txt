cmake_minimum_required(VERSION 3.5)

project(scgi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} server.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_link_libraries(scgi ev)
target_link_libraries(scgi Threads::Threads)

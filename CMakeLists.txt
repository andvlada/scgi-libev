cmake_minimum_required(VERSION 3.5)

project(scgi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp server.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_EXTENSIONS OFF
)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_link_libraries(scgi ev)

#find_package(PkgConfig REQUIRED)
#pkg_check_modules(lev REQUIRED IMPORTED_TARGET lev)

#target_link_libraries(scgi PRIVATE PkgConfig::lev)

SET (EXTRA_LIBS
   -lev
)

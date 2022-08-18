project(web_server)
cmake_minimum_required(VERSION 3.0)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_STANDARD 14)
set(CXX_FLAGS
    -Wall
    -Wextra
    # -Werror
    -Wshadow
    -Wno-unused-variable
    -rdynamic
    )
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-04 -DNDEBUG")

option(OBJECT_TESTS "something tset" ON)
enable_testing()

aux_source_directory(src/* SRC_FILES)
# add_library(webs ${SRC_FILES})
add_executable(web_server main.cpp ${SRC_FILES})
target_link_libraries(web_server pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


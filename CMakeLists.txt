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

# OPTION
# option(OBJECT_TESTS "something tset" ON)
option(USE_BUFFER "Use buffer implementation" ON)
option(USE_LOG "Use log implementation" ON)
option(USE_POOL "Use pool resource implementation" ON)
option(USE_TIMER "Use timer implementation" ON)
option(USE_SERVER "Use server implementation" ON)

if (USE_BUFFER)
    include_directories("${PROJECT_SOURCE_DIR}/src/buffer")
    add_subdirectory(src/buffer)
    set(EXTRA_LIBS ${EXTRA_LIBS} BUFFER)
endif ()
if (USE_LOG)
    include_directories("${PROJECT_SOURCE_DIR}/src/log")
    add_subdirectory(src/log)
    set(EXTRA_LIBS ${EXTRA_LIBS} LOG)
endif()
if (USE_POOL)
    include_directories("${PROJECT_SOURCE_DIR}/src/pool")
    add_subdirectory(src/pool)
    set(EXTRA_LIBS ${EXTRA_LIBS} POOL)
endif ()
if (USE_TIMER)
    include_directories("${PROJECT_SOURCE_DIR}/src/timer")
    add_subdirectory(src/timer)
    set(EXTRA_LIBS ${EXTRA_LIBS} TIMER)
endif()
if (USE_SERVER)
    include_directories("${PROJECT_SOURCE_DIR}/src/server")
    add_subdirectory(src/server)
    set(EXTRA_LIBS ${EXTRA_LIBS} SERVER)
endif()

# Other
set(EXTRA_LIBS ${EXTRA_LIBS} pthread)
set(EXTRA_LIBS ${EXTRA_LIBS} mysqlclient)

enable_testing()

aux_source_directory(src/* SRC_FILES)
# add_library(webs ${SRC_FILES})
add_executable(web_server main.cpp ${SRC_FILES})
target_link_libraries(web_server ${EXTRA_LIBS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)


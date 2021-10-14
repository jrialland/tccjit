cmake_minimum_required(VERSION 3.10)
project(tccjit)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -Wfatal-errors -ggdb ")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -Wfatal-errors -ffast-math -Ofast")
set(THREADS_PREFER_PTHREAD_FLAG ON)


include(./cmake/FileEmbed.cmake)

## pthread ##
find_package(Threads REQUIRED)

## tinycc ##
execute_process(COMMAND ./configure WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tinycc)
execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tinycc)


FileEmbedSetup()
FileEmbedAdd(${CMAKE_SOURCE_DIR}/deps/tinycc/libtcc1.a)

## source files ##
file(GLOB_RECURSE TCCJIT_SRC ${CMAKE_SOURCE_DIR}/src/*.cpp)

## generate a shared libary ##
add_library(tccjit SHARED ${TCCJIT_SRC}  ${CMAKE_SOURCE_DIR}/deps/tinycc/libtcc.c)
target_include_directories(tccjit PUBLIC ${CMAKE_SOURCE_DIR}/deps/tinycc)
target_link_libraries(tccjit file_embed ${CMAKE_DL_LIBS} Threads::Threads stdc++fs)

file(GLOB_RECURSE TESTS_SRC ${CMAKE_SOURCE_DIR}/tests/*.cpp)
add_executable(tests ${TESTS_SRC})
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/deps ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tests tccjit)

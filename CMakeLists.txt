cmake_minimum_required(VERSION 3.10)
project(tccjit)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(THREADS_PREFER_PTHREAD_FLAG ON)

## pthread ##

find_package(Threads REQUIRED)

## Boost ##
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS regex)

## tinycc ##
execute_process(COMMAND ./configure WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tinycc)
execute_process(COMMAND make WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/tinycc)

## source files ##
file(GLOB_RECURSE TCCJIT_SRC ${CMAKE_SOURCE_DIR}/src/*.cpp)

## generate a shared libary ##
add_library(tccjit SHARED ${TCCJIT_SRC} ${CMAKE_SOURCE_DIR}/deps/tinycc/libtcc.c)
target_include_directories(tccjit PRIVATE ${CMAKE_SOURCE_DIR}/deps/tinycc)
target_link_libraries(tccjit ${CMAKE_DL_LIBS} Threads::Threads)

file(GLOB_RECURSE TESTS_SRC ${CMAKE_SOURCE_DIR}/tests/*.cpp)
add_executable(tests ${TESTS_SRC})
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/deps ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tests tccjit)

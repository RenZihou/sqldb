cmake_minimum_required(VERSION 3.16)
project(sqldb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

find_package(Threads REQUIRED)

include_directories(include/antlr4-runtime)
file(GLOB_RECURSE antlr4-runtime-src "include/antlr4-runtime/*.cpp" "include/antlr4-runtime/*.h")
add_library(antlr4-runtime ${antlr4-runtime-src})

add_subdirectory(src)

add_executable(sqldb src/main.cpp)

target_link_libraries(antlr4-runtime Threads::Threads)
target_link_libraries(sqldb query parser system query parser index table pagefile util antlr4-runtime)  # TODO better depending chain

cmake_minimum_required(VERSION 3.16)
project(sqldb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

find_package(Threads REQUIRED)

include_directories(include/antlr4-runtime)
file(GLOB_RECURSE antlr4-runtime-src "include/antlr4-runtime/*.cpp" "include/antlr4-runtime/*.h")
add_library(antlr4-runtime ${antlr4-runtime-src})
add_subdirectory(src)

add_executable(sqldb src/main.cpp)

target_link_libraries(antlr4-runtime Threads::Threads)
target_link_libraries(sqldb query parser record pagefile util antlr4-runtime Threads::Threads)
